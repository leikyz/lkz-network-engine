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

    // Distance and logic constants
    const float AI_AGGRO_RANGE_SQ = 1000.0f * 1000.0f;
    const float STOP_DISTANCE_SQ = 1.5f * 1.5f;
    const float RESUME_CHASE_DISTANCE_SQ = 2.0f * 2.0f;
    const float UPDATE_THRESHOLD_SQ = 0.7f * 0.7f;
    const int MAX_PATH_UPDATES_PER_TICK = 1000;
    int pathUpdatesThisTick = 0;

    for (auto& [entity, ai] : components.ai)
    {
        if (ai.crowdAgentIndex == -1) continue;

        const dtCrowdAgent* agent = crowd->getAgent(ai.crowdAgentIndex);
        if (!agent || !agent->active) continue;

        // This distributes the CPU load by only updating 25% of entities per frame.
        if (entity % 4 == globalFrameCount % 4)
        {
            Vector3& position = components.positions[entity].position;
            Session* session = EntityManager::Instance().GetSessionByEntity(entity);
            if (!session) continue;

            // Adjust timer decrement because we only enter this block every 4 frames
            ai.repathTimer -= (deltaTime * 4.0f);

            bool isIdle = (agent->targetState == DT_CROWDAGENT_TARGET_NONE) ||
                (agent->targetState == DT_CROWDAGENT_TARGET_FAILED);
            bool timerExpired = (ai.repathTimer <= 0.0f);
            bool shouldAttemptPathing = (timerExpired || isIdle) && (pathUpdatesThisTick < MAX_PATH_UPDATES_PER_TICK);

            Entity nearestPlayerEntity = 0;
            float minDistanceSq = FLT_MAX;
            Vector3 targetPos;
            bool foundPlayer = false;

            // Search for the nearest player within the same lobby
            for (auto& [playerEntity, input] : components.playerInputs)
            {
                if (components.positions.find(playerEntity) == components.positions.end()) continue;
                Session* playerSession = EntityManager::Instance().GetSessionByEntity(playerEntity);
                if (playerSession != session) continue;

                Vector3& playerPos = components.positions[playerEntity].position;
                float distSq = (playerPos - position).LengthSquared();

               /* if (distSq < minDistanceSq)
                {*/
                    minDistanceSq = distSq;
                    nearestPlayerEntity = playerEntity;
                    targetPos = playerPos;
                    foundPlayer = true;
               /* }*/
            }

            // Search first player created
            //for (auto& [playerEntity, input] : components.playerInputs)
            //{
            //    if (components.positions.find(playerEntity) == components.positions.end()) continue;

            //    Lobby* playerLobby = EntityManager::Instance().GetLobbyByEntity(playerEntity);
            //    if (playerLobby != lobby) continue;

            //    Vector3& playerPos = components.positions[playerEntity].position;

            //    nearestPlayerEntity = playerEntity;
            //    targetPos = playerPos;
            //    foundPlayer = true;
            //    break;
            //}


            // MOVEMENT LOGIC (Updated only during the entity's turn)
            if (foundPlayer && minDistanceSq < AI_AGGRO_RANGE_SQ)
            {
                if (minDistanceSq < STOP_DISTANCE_SQ)
                {
                    // Stop distance reached
                    if (ai.targetPosition.has_value() || !isIdle)
                    {
                        ai.targetPosition.reset();
                        crowd->resetMoveTarget(ai.crowdAgentIndex);
                        dtCrowdAgent* mutableAgent = crowd->getEditableAgent(ai.crowdAgentIndex);
                        // Force velocity to zero to prevent sliding
                        if (mutableAgent) { memset(mutableAgent->vel, 0, sizeof(float) * 3); }
                    }
                }
                else if (minDistanceSq > RESUME_CHASE_DISTANCE_SQ || ai.targetPosition.has_value())
                {
                    // Chasing or updating trajectory
                    if (shouldAttemptPathing)
                    {
                        pathUpdatesThisTick++;
                        ai.repathTimer = 0.2f + ((rand() % 10) / 100.0f); // Randomized repath delay

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

                            // Only request a new path if the player moved significantly
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
                // No valid target or player out of range: reset objective
                if (ai.targetPosition.has_value()) {
                    ai.targetPosition.reset();
                    crowd->resetMoveTarget(ai.crowdAgentIndex);
                }
            }
        } 

        // --- NETWORK SYNCHRONIZATION (Always active for fluidity) ---
        ai.timeSinceLastSend += deltaTime;
        if (ai.timeSinceLastSend >= Constants::AI_MESSAGE_RATE)
        {
            Vector3 currentPos = { agent->npos[0], agent->npos[1], agent->npos[2] };
            float distSq = (currentPos - ai.lastSentPosition).LengthSquared();

            // Delta compression: only send update if the entity moved more than 10cm
            if (distSq > 0.01f)
            {
                ai.timeSinceLastSend = 0.0f;
                ai.lastSentPosition = currentPos;

                Session* session = EntityManager::Instance().GetSessionByEntity(entity);
                if (session) {
                    auto& msg = sessionMessages[session];
                    msg.addUpdate(entity, currentPos.x, currentPos.y, currentPos.z);

					// Send in batches of 100 updates to avoid large packets
                    if (msg.updates.size() >= 100)
                    {
                        Serializer s;
                        msg.serialize(s);
                        const std::vector<uint8_t>& buffer = s.getBuffer();
                        const std::string& className = msg.getClassName();

       
                        for (const auto& player : session->players)
                        {
                            if (player.isUdpReady)
                            {
                                Engine::Instance().Server()->Send(player.udpAddr, buffer, msg.getClassName());
                            }
                        }

                        // 4. On vide les mises à jour pour recommencer à accumuler
                        msg.updates.clear();
                    }
                }
            }
        }
    }

    // Broadcast messages grouped by Lobby
    for (auto& [session, msg] : sessionMessages)
    {
        if (!msg.updates.empty())
        {
            // 1. OPTIMISATION : On sérialise UNE SEULE FOIS pour tout le monde
            Serializer s;
            msg.serialize(s);

            // On récupère le buffer (évite de copier le vector à chaque appel)
            const std::vector<uint8_t>& buffer = s.getBuffer();

            // 2. On parcourt les joueurs de la session
            for (const auto& player : session->players)
            {
                // 3. SÉCURITÉ : On n'envoie qu'à ceux qui ont fait le Handshake UDP
                if (player.isUdpReady)
                {
                    // Envoi direct (Coût faible, c'est juste un appel système sendto)
                    Engine::Instance().Server()->Send(player.udpAddr, buffer, msg.getClassName());
                }
            }
        }
    }
}