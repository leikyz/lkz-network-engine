#include <LKZ/Core/ECS/System/PlayerSystem.h>
#include <LKZ/Core/ECS/Manager/ComponentManager.h>
#include <LKZ/Core/ECS/Manager/EntityManager.h>
#include <LKZ/Core/Engine.h>
#include <LKZ/Protocol/Message/Entity/MoveEntityMessage.h>
#include <LKZ/Protocol/Message/Entity/RotateEntityMessage.h>
#include <LKZ/Protocol/Message/Entity/LastEntityPositionMessage.h>
#include <LKZ/Simulation/Math/MathUtils.h>
#include <LKZ/Simulation/Math/Vector.h>
#include <LKZ/Utility/Logger.h>
#include <LKZ/Utility/Constants.h>
#include "LKZ/Core/ECS/Manager/NavMeshQueryManager.h"
#include "LKZ/Simulation/World.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"
#include <DetourCrowd.h>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <set>

constexpr float INERTIA_DAMPING = 8.0f;

void UpdateVelocity(Vector3& velocity, const PlayerInputData& input, float speed, float fixedDeltaTime)
{
    float yawRad = input.yaw * (Constants::PI / 180.0f);

    Vector3 forwardDir = { std::sin(yawRad), 0, std::cos(yawRad) };
    Vector3 rightDir = { std::cos(yawRad), 0, -std::sin(yawRad) };

    Vector3 targetDir = {
        (rightDir.x * input.inputX) + (forwardDir.x * input.inputY),
        0.0f,
        (rightDir.z * input.inputX) + (forwardDir.z * input.inputY)
    };

    float len = std::sqrt(targetDir.x * targetDir.x + targetDir.z * targetDir.z);
    if (len > 1.0f) {
        targetDir.x /= len;
        targetDir.z /= len;
    }

    Vector3 targetVel = { targetDir.x * speed, 0.0f, targetDir.z * speed };

    float blend = INERTIA_DAMPING * fixedDeltaTime;
    if (blend > 1.0f) blend = 1.0f;

    velocity.x += (targetVel.x - velocity.x) * blend;
    velocity.z += (targetVel.z - velocity.z) * blend;

    if (std::abs(velocity.x) < 0.01f && input.inputX == 0 && input.inputY == 0) velocity.x = 0;
    if (std::abs(velocity.z) < 0.01f && input.inputX == 0 && input.inputY == 0) velocity.z = 0;
}

void PlayerSystem::Update(ComponentManager& components, float fixedDeltaTime)
{
    static int tickCount = 0;
    static auto lastTickTime = std::chrono::high_resolution_clock::now();

    tickCount++;
    auto currentTime = std::chrono::high_resolution_clock::now();

    if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTickTime).count() >= 1000)
    {
        Logger::Log("[SERVER TICK] Simulation Frequency: " + std::to_string(tickCount) + " Hz", LogType::Info);

        // Reset
        tickCount = 0;
        lastTickTime = currentTime;
    }

    World& world = Engine::Instance().GetWorld();
    dtNavMeshQuery* navQuery = NavMeshQueryManager::GetThreadLocalQuery(world.getNavMesh());
    const dtQueryFilter* filter = world.getCrowd() ? world.getCrowd()->getFilter(0) : nullptr;

    const float extents[3] = { 2.0f, 4.0f, 2.0f };

    for (auto& [entity, inputComp] : components.playerInputs)
    {
        Session* session = EntityManager::Instance().GetSessionByEntity(entity);
        if (!session) continue;

        SessionPlayer* ownerPlayer = nullptr;
        for (auto& player : session->players)
        {
            if (player.entityId == entity)
            {
                ownerPlayer = &player;
                break;
            }
        }
        
        if (inputComp.inputQueue.size() > 1) {
            Logger::Log("Accumulation détectée : " + std::to_string(inputComp.inputQueue.size()) + " inputs", LogType::Warning);
        }

        if (!ownerPlayer || !ownerPlayer->isUdpReady) continue;

        auto& pos = components.positions[entity].position;
        Vector3& vel = inputComp.currentVelocity;

        std::sort(inputComp.inputQueue.begin(), inputComp.inputQueue.end(),
            [](const PlayerInputData& a, const PlayerInputData& b) {
                return a.sequenceId < b.sequenceId;
            });

        int lastProcessedSeq = -1;
        bool hasProcessed = false;

        dtPolyRef currentPolyRef = 0;
        float startPos[3] = { pos.x, pos.y, pos.z };

        if (navQuery && filter)
        {
            navQuery->findNearestPoly(startPos, extents, filter, &currentPolyRef, startPos);
            if (currentPolyRef != 0)
            {
                pos.y = startPos[1];
            }
        }

        for (const auto& input : inputComp.inputQueue)
        {
            if (input.sequenceId <= inputComp.lastExecutedSequenceId) continue;

            float currentSpeed = Constants::PLAYER_MOVE_SPEED;

            if (input.isAiming)
            {
                currentSpeed *= Constants::PLAYER_AIM_SPEED_MULTIPLICATOR;
            }
            else if (input.isArmed)
            {
                if (input.isRunning)
                    currentSpeed *= Constants::PLAYER_RUN_ARMED_SPEED_MULTIPLICATOR;
                else
                    currentSpeed *= Constants::PLAYER_WALK_ARMED_SPEED_MULTIPLICATOR;
            }
            else
            {
                if (input.isRunning)
                    currentSpeed *= Constants::PLAYER_RUN_SPEED_MULTIPLICATOR;
                else
                    currentSpeed *= Constants::PLAYER_WALK_SPEED_MULTIPLICATOR;
            }

            UpdateVelocity(vel, input, currentSpeed, fixedDeltaTime);

            if (navQuery && filter && currentPolyRef != 0)
            {
                float endPos[3];
                endPos[0] = startPos[0] + (vel.x * fixedDeltaTime);
                endPos[1] = startPos[1];
                endPos[2] = startPos[2] + (vel.z * fixedDeltaTime);

                float resultPos[3];
                dtPolyRef visitedPolys[16];
                int nVisited = 0;

                navQuery->moveAlongSurface(currentPolyRef, startPos, endPos, filter, resultPos, visitedPolys, &nVisited, 16);

                pos.x = resultPos[0];
                pos.y = resultPos[1];
                pos.z = resultPos[2];

                startPos[0] = resultPos[0];
                startPos[1] = resultPos[1];
                startPos[2] = resultPos[2];

                if (nVisited > 0)
                {
                    currentPolyRef = visitedPolys[nVisited - 1];
                }
            }
            else
            {
                pos.x += vel.x * fixedDeltaTime;
                pos.z += vel.z * fixedDeltaTime;
            }

            inputComp.lastExecutedSequenceId = input.sequenceId;
            lastProcessedSeq = input.sequenceId;
            hasProcessed = true;
        }

        inputComp.inputQueue.clear();

       /* Logger::Log("[PlayerSystem] Entity: " + std::to_string(entity) +
            " | Pos: (" + std::to_string(pos.x) + ", " +
            std::to_string(pos.y) + ", " +
            std::to_string(pos.z) + ")", LogType::Info);*/

        if (hasProcessed)
        {
            Serializer sMove;
            MoveEntityMessage moveMsg(entity, pos.x, pos.y, pos.z);
            moveMsg.serialize(sMove);
            const std::vector<uint8_t>& moveBuffer = sMove.getBuffer();
            const std::string& moveClassName = moveMsg.getClassName();

            for (const auto& player : session->players)
            {
                if (player.isUdpReady && player.entityId != entity)
                {
                    Engine::Instance().Server()->Send(player.udpAddr, moveBuffer, moveMsg.getClassName());
                }
            }

            LastEntityPositionMessage ackMsg(entity, pos.x, pos.y, pos.z, vel.x, vel.y, vel.z, lastProcessedSeq);
            Serializer sAck;
            ackMsg.serialize(sAck);
            Engine::Instance().Server()->Send(ownerPlayer->udpAddr, sAck.getBuffer(), ackMsg.getClassName());
        }
    }
}