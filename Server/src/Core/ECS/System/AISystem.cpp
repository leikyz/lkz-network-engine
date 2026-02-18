#include "LKZ/Core/ECS/System/AISystem.h"
#include "LKZ/Core/ECS/Manager/ComponentManager.h"
#include "LKZ/Simulation/World.h"
#include "LKZ/Simulation/Math/MathUtils.h"
#include "LKZ/Simulation/Math/Vector.h"
#include <cmath>
#include <LKZ/Core/Engine.h>
#include <LKZ/Utility/Logger.h>
#include <LKZ/Utility/Constants.h>
#include <LKZ/Core/ECS/Manager/EntityManager.h>
#include <LKZ/Protocol/Message/Entity/MoveEntitiesMessage.h>
#include "LKZ/Core/ECS/Manager/NavMeshQueryManager.h"

#include "DetourCrowd.h"
#include "DetourNavMeshQuery.h"

#include <float.h> 
#include <DetourCommon.h>
#include <stdlib.h> 
#include <chrono>

void AISystem::Update(ComponentManager& components, float deltaTime)
{
    // Global frame counter to distribute AI calculations
    static uint64_t globalFrameCount = 0;
    globalFrameCount++;

    World& world = Engine::Instance().GetWorld();
    dtNavMesh* navMesh = world.getNavMesh();
    dtCrowd* crowd = world.getCrowd();
    if (!navMesh || !crowd) return;

    dtNavMeshQuery* navQuery = NavMeshQueryManager::GetThreadLocalQuery(navMesh);
    if (!navQuery) return;

    const dtQueryFilter* filter = crowd->getFilter(Constants::AGENT_QUERY_FILTER_TYPE);
    std::unordered_map<Session*, MoveEntitiesMessage> sessionMessages;

    // --- CONFIGURATION DES DISTANCES ---
    const float AI_AGGRO_RANGE_SQ = 1000.0f * 1000.0f;
    const float STOP_DISTANCE_SQ = 1.5f * 1.5f;          // Distance d'arrêt (attaque)
    const float RESUME_CHASE_DISTANCE_SQ = 2.0f * 2.0f;  // Distance pour reprendre la course

    // DISTANCE DE COMBAT RAPPROCHÉ (4 mètres)
    // En dessous de cette distance, le zombie devient "frénétique" et update très vite
    const float CLOSE_COMBAT_RANGE_SQ = 4.0f * 4.0f;

    // SEUIL DE RÉACTION (6cm)
    // Si le joueur bouge de plus de 6cm, le zombie recalcule le chemin
    const float UPDATE_THRESHOLD_SQ = 0.25f * 0.25f;

    const int MAX_PATH_UPDATES_PER_TICK = 1000;
    int pathUpdatesThisTick = 0;

    for (auto& [entity, ai] : components.ai)
    {
        if (ai.crowdAgentIndex == -1) continue;

        const dtCrowdAgent* agent = crowd->getAgent(ai.crowdAgentIndex);
        if (!agent || !agent->active) continue;

        // LOAD BALANCING : On ne traite que 25% des entités par frame pour sauver le CPU
        if (entity % 4 == globalFrameCount % 4)
        {
            Vector3& position = components.positions[entity].position;

            // Optimization: Get Session inside the check to avoid lookup on skipped entities
            Session* session = EntityManager::Instance().GetSessionByEntity(entity);
            if (!session) continue;

            // --- 1. GESTION DU TIMER DE PATHFINDING ---
            // On décrémente 4x plus vite car on passe ici 1 frame sur 4
            ai.repathTimer -= (deltaTime * 4.0f);

            bool isIdle = (agent->targetState == DT_CROWDAGENT_TARGET_NONE) ||
                (agent->targetState == DT_CROWDAGENT_TARGET_FAILED);

            // Le timer est expiré OU on est idle
            bool timerExpired = (ai.repathTimer <= 0.0f);

            // On limite le nombre de calculs lourds par frame globale
            bool shouldAttemptPathing = (timerExpired || isIdle) && (pathUpdatesThisTick < MAX_PATH_UPDATES_PER_TICK);

            Entity nearestPlayerEntity = 0;
            float minDistanceSq = FLT_MAX;
            Vector3 targetPos;
            bool foundPlayer = false;

            // --- 2. RECHERCHE DU JOUEUR LE PLUS PROCHE ---
            for (auto& [playerEntity, input] : components.playerInputs)
            {
                if (components.positions.find(playerEntity) == components.positions.end()) continue;

                Session* playerSession = EntityManager::Instance().GetSessionByEntity(playerEntity);
                if (playerSession != session) continue;

                Vector3& playerPos = components.positions[playerEntity].position;
                float distSq = (playerPos - position).LengthSquared();

                if (distSq < minDistanceSq)
                {
                    minDistanceSq = distSq;
                    nearestPlayerEntity = playerEntity;
                    targetPos = playerPos;
                    foundPlayer = true;
                }
            }

            // --- 3. LOGIQUE DE MOUVEMENT ---
            if (foundPlayer && minDistanceSq < AI_AGGRO_RANGE_SQ)
            {
                // Est-ce qu'on est en corps à corps ? (Moins de 4m)
                bool isCloseCombat = minDistanceSq < CLOSE_COMBAT_RANGE_SQ;

                // FIX ANTI-SURPLACE : Si on est très proche et que le timer n'est pas encore reset,
                // mais qu'il est proche de la fin, on force le pathing pour suivre le joueur qui tourne autour.
                if (isCloseCombat && !shouldAttemptPathing && pathUpdatesThisTick < MAX_PATH_UPDATES_PER_TICK)
                {
                    // Si le timer est à moins de 0.05s, on force l'update
                    if (ai.repathTimer < 0.05f) shouldAttemptPathing = true;
                }

                if (minDistanceSq < STOP_DISTANCE_SQ)
                {
                    // On est arrivé au contact : STOP
                    if (ai.targetPosition.has_value() || !isIdle)
                    {
                        ai.targetPosition.reset();
                        crowd->resetMoveTarget(ai.crowdAgentIndex);
                        dtCrowdAgent* mutableAgent = crowd->getEditableAgent(ai.crowdAgentIndex);
                        // On coupe la vélocité pour éviter qu'il glisse
                        if (mutableAgent) { memset(mutableAgent->vel, 0, sizeof(float) * 3); }
                    }
                }
                else if (minDistanceSq > RESUME_CHASE_DISTANCE_SQ || ai.targetPosition.has_value())
                {
                    // EN CHASSE
                    if (shouldAttemptPathing)
                    {
                        pathUpdatesThisTick++;

                        // --- FIX TIMER ADAPTATIF ---
                        if (isCloseCombat)
                        {
                            // MODE FRÉNÉTIQUE : Update très rapide (0.05s à 0.10s)
                            // C'est ça qui empêche le zombie de faire du surplace quand tu tournes autour
                            ai.repathTimer = 0.05f + ((rand() % 5) / 100.0f);
                        }
                        else
                        {
                            // MODE NORMAL : Update standard (0.20s à 0.40s) pour économiser le CPU
                            ai.repathTimer = 0.2f + ((rand() % 20) / 100.0f);
                        }

                        const float extents[3] = { 2.0f, 4.0f, 2.0f };
                        dtPolyRef targetRef;
                        float nearestPt[3];

                        dtStatus status = navQuery->findNearestPoly(targetPos.data(), extents, filter, &targetRef, nearestPt, nullptr);

                        if (dtStatusSucceed(status) && targetRef)
                        {
                            float dx = agent->targetPos[0] - nearestPt[0];
                            float dy = agent->targetPos[1] - nearestPt[1];
                            float dz = agent->targetPos[2] - nearestPt[2];
                            float distToCurrentTargetSq = dx * dx + dy * dy + dz * dz;

                            // On vérifie si la cible a bougé de plus de 6cm (UPDATE_THRESHOLD_SQ)
                            if (agent->targetState != DT_CROWDAGENT_TARGET_VALID || distToCurrentTargetSq > UPDATE_THRESHOLD_SQ)
                            {
                                ai.targetPosition = targetPos;
                                crowd->requestMoveTarget(ai.crowdAgentIndex, targetRef, nearestPt);
                            }
                        }
                    }
                }
            }
            else
            {
                // Pas de joueur trouvé ou trop loin -> Reset
                if (ai.targetPosition.has_value()) {
                    ai.targetPosition.reset();
                    crowd->resetMoveTarget(ai.crowdAgentIndex);
                }
            }

            // --- 4. SYNCHRONISATION RÉSEAU ---

            // CORRECTION IMPORTANTE : Multiplier deltaTime par 4.0f
            // Sinon le temps réseau passe 4x trop lentement à cause du "if (entity % 4)"
            ai.timeSinceLastSend += (deltaTime * 4.0f);

            if (ai.timeSinceLastSend >= Constants::AI_MESSAGE_RATE)
            {
                Vector3 currentPos = { agent->npos[0], agent->npos[1], agent->npos[2] };
                float distSq = (currentPos - ai.lastSentPosition).LengthSquared();

                // Compression Delta : on envoie seulement si ça a bougé de 10cm
                if (distSq > 0.01f)
                {
                    ai.timeSinceLastSend = 0.0f; // Reset du timer réseau
                    ai.lastSentPosition = currentPos;

                    if (session) {
                        auto& msg = sessionMessages[session];
                        // On ajoute la position au paquet groupé
                        // Note : Assure-toi que le Yaw (Rotation) est géré soit ici, soit calculé côté client (LookAt)
                        msg.addUpdate(entity, currentPos.x, currentPos.y, currentPos.z);

                        // Si le paquet est plein (100 entités), on envoie tout de suite
                        if (msg.updates.size() >= 100)
                        {
                            Serializer s;
                            msg.serialize(s);
                            const std::vector<uint8_t>& buffer = s.getBuffer();

                            for (const auto& player : session->players)
                            {
                                if (player.isUdpReady)
                                {
                                    Engine::Instance().Server()->Send(player.udpAddr, buffer, msg.getClassName());
                                }
                            }
                            msg.updates.clear();
                        }
                    }
                }
            }
        }
    }

    // --- 5. ENVOI DES MESSAGES RESTANTS ---
    // On envoie les paquets qui n'étaient pas pleins (< 100 entités)
    for (auto& [session, msg] : sessionMessages)
    {
        if (!msg.updates.empty())
        {
            Serializer s;
            msg.serialize(s);
            const std::vector<uint8_t>& buffer = s.getBuffer();

            for (const auto& player : session->players)
            {
                if (player.isUdpReady)
                {
                    Engine::Instance().Server()->Send(player.udpAddr, buffer, msg.getClassName());
                }
            }
        }
    }
}