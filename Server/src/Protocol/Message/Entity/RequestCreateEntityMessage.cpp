#include "LKZ/Protocol/Message/Entity/RequestCreateEntityMessage.h"
#include <cstdlib>
#include <ctime>

#include "LKZ/Core/Threading/CommandQueue.h"
#include "LKZ/Core/Threading/ThreadManager.h"
#include "LKZ/Core/ECS/Manager/EntityManager.h"
#include "LKZ/Core/ECS/Manager/ComponentManager.h"
#include "LKZ/Core/ECS/Manager/NavMeshQueryManager.h"
#include "LKZ/Core/Manager/ClientManager.h"
#include "LKZ/Core/Engine.h"
#include "LKZ/Protocol/Message/Entity/CreateEntityMessage.h"
#include "LKZ/Utility/Logger.h"
#include "LKZ/Utility/Constants.h"
#include <DetourCrowd.h>
#include "LKZ/Simulation/Math/Vector.h" 
#include <iostream>

RequestCreateEntityMessage::RequestCreateEntityMessage() {}

RequestCreateEntityMessage::RequestCreateEntityMessage(int entityTypeId)
    : entitySuperTypeId(entityTypeId)
{
}

uint8_t RequestCreateEntityMessage::getId() const
{
    return ID;
}

std::vector<uint8_t>& RequestCreateEntityMessage::serialize(Serializer& serializer) const
{
    serializer.reset();
    return serializer.getBuffer();
}

void RequestCreateEntityMessage::deserialize(Deserializer& deserializer)
{
    entitySuperTypeId = deserializer.readByte();
}

void RequestCreateEntityMessage::process(const sockaddr_in& senderAddr, SOCKET tcpSocket)
{
    Session* session = (tcpSocket != INVALID_SOCKET)
        ? SessionManager::GetSessionBySocket(tcpSocket)
        : SessionManager::GetSessionByAddress(senderAddr);

    std::cout << "[RequestCreateEntityMessage] Processing request from " 
              << (tcpSocket != INVALID_SOCKET ? "TCP socket" : "UDP address") 
              << ": " << senderAddr.sin_addr.s_addr << ":" << ntohs(senderAddr.sin_port) 
		<< " for entity super type ID: " << (int)entitySuperTypeId << std::endl;

    if (!session)
    {
        Logger::Log("RequestCreateEntityMessage: Session introuvable.", LogType::Warning);
        return;
    }

    SessionPlayer* player = nullptr;
    if (tcpSocket != INVALID_SOCKET)
    {
        for (auto& p : session->players)
        {
            if (p.tcpSocket == tcpSocket)
            {
                player = &p;
                break;
            }
        }
    }

    int superTypeId = this->entitySuperTypeId;
    INetworkInterface* server = Engine::Instance().Server();

    // --- SPAWN PLAYER ---
    if (superTypeId == (int)EntitySuperType::Player)
    {
        CommandQueue::Instance().Push([=]() {
            auto& components = ComponentManager::Instance();
            Entity entity = EntityManager::Instance().CreateEntity(EntitySuperType(superTypeId), components, session);

            if (player)
            {
                player->entityId = entity;
            }

            Vector3 playerSpawnPos = Constants::FIRST_PLAYER_SPAWN_POSITION;

            int positionIndex = 1;
            for (size_t i = 0; i < session->players.size(); ++i)
            {

                if (session->players[i].id == player->id)
                {

                    positionIndex = (int)i + 1;
                    break;
                }
            }

            switch (positionIndex)
            {
            case 1: playerSpawnPos = Constants::FIRST_PLAYER_SPAWN_POSITION; break;
            case 2: playerSpawnPos = Constants::SECOND_PLAYER_SPAWN_POSITION; break;
            case 3: playerSpawnPos = Constants::THIRD_PLAYER_SPAWN_POSITION; break;
            case 4: playerSpawnPos = Constants::FOURTH_PLAYER_SPAWN_POSITION; break;
            }

            components.AddComponent(entity, PositionComponent{ playerSpawnPos });
            components.AddComponent(entity, RotationComponent{ Vector3{ 0.0f, 0.0f, 0.0f } });
            components.AddComponent(entity, PlayerInputComponent{ std::vector<PlayerInputData>() });

            // Message pour le client local (Propriétaire)
            CreateEntityMessage createEntityMsg;
            createEntityMsg.entityTypeId = (int)EntityType::Player1;
            createEntityMsg.entityId = entity;
            createEntityMsg.posX = playerSpawnPos.x;
            createEntityMsg.posY = playerSpawnPos.y;
            createEntityMsg.posZ = playerSpawnPos.z;

            Serializer serializer;
            createEntityMsg.serialize(serializer);

            server->SendReliable(tcpSocket, serializer.getBuffer());

            createEntityMsg.entityTypeId = (int)EntityType::PlayerSynced1;
            createEntityMsg.serialize(serializer);

            const std::vector<uint8_t>& buffer = serializer.getBuffer();

            for (const auto& p : session->players)
            {
                if (!player)
                    continue;

                if (p.id != player->id && p.tcpSocket != INVALID_SOCKET)
                {
                   

                    server->SendReliable(p.tcpSocket, buffer);
                }
            }
            });
    }
    else
    {
        ThreadManager::GetPool("pathfinding")->EnqueueTask([=]() {
            auto& world = Engine::Instance().GetWorld();
            dtNavMeshQuery* simQuery = NavMeshQueryManager::GetThreadLocalQuery(world.getNavMesh());
            Vector3 randomSpawnPoint = world.getRandomNavMeshPoint(simQuery);

            CommandQueue::Instance().Push([=]() {
                auto& components = ComponentManager::Instance();
                auto& world = Engine::Instance().GetWorld();
                Entity entity = EntityManager::Instance().CreateEntity(EntitySuperType(superTypeId), components, session);

                components.AddComponent(entity, PositionComponent{ randomSpawnPoint });
                components.AddComponent(entity, RotationComponent{ Vector3{ 0.0f, 0.0f, 0.0f } });

                float initialRepathDelay = ((rand() % 100) / 100.0f) * 2.0f;
                Vector3 initialTarget = world.getRandomNavMeshPoint(simQuery);
                if (initialTarget.x == 0 && initialTarget.z == 0) initialTarget = { 10.0f, 0.0f, 10.0f };

                components.AddComponent(entity, AIComponent{
                    initialTarget,
                    initialRepathDelay,
                    -1,
                    0.0f,
                    });

                auto& aiComp = components.ai[entity];
                aiComp.posPtr = &components.positions[entity];
                aiComp.rotPtr = &components.rotations[entity];

                dtCrowd* crowd = world.getCrowd();
                if (crowd)
                {
                    dtCrowdAgentParams params;
                    memset(&params, 0, sizeof(params));
                    params.radius = Constants::AGENT_RADIUS;
                    params.height = Constants::AGENT_HEIGHT;
                    params.maxAcceleration = Constants::AGENT_MAX_ACCELERATION;
                    params.maxSpeed = Constants::AGENT_MAX_SPEED;
                    params.collisionQueryRange = params.radius * 8.0f;
                    params.pathOptimizationRange = params.radius * 30.0f;
                    params.queryFilterType = Constants::AGENT_QUERY_FILTER_TYPE;
                    params.obstacleAvoidanceType = Constants::AGENT_OBSTACLE_AVOIDANCE_TYPE;
                    params.separationWeight = Constants::AGENT_SEPARATION_WEIGHT;
                    params.updateFlags = Constants::AGENT_UPDATE_FLAGS & ~DT_CROWD_OBSTACLE_AVOIDANCE;
                    params.userData = &aiComp;

                    float spawnPos[3] = { randomSpawnPoint.x, randomSpawnPoint.y, randomSpawnPoint.z };
                    int agentIdx = crowd->addAgent(spawnPos, &params);
                    if (agentIdx != -1) components.ai[entity].crowdAgentIndex = agentIdx;
                }

                CreateEntityMessage createEntityMsg;
                createEntityMsg.entityTypeId = 3 + rand() % 3;
                createEntityMsg.entityId = entity;
                createEntityMsg.posX = randomSpawnPoint.x;
                createEntityMsg.posY = randomSpawnPoint.y;
                createEntityMsg.posZ = randomSpawnPoint.z;

                Serializer serializer;
                createEntityMsg.serialize(serializer);

                const std::vector<uint8_t>& buffer = serializer.getBuffer();

                for (const auto& p : session->players)
                {
                    if (p.tcpSocket != INVALID_SOCKET)
                    {
                        server->SendReliable(p.tcpSocket, buffer);
                    }
                }
                });
            });
    }
}