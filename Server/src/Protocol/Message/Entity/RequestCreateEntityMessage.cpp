#include "LKZ/Protocol/Message/Entity/RequestCreateEntityMessage.h"
#include <cstdlib>
#include <ctime>

#include "LKZ/Core/Threading/CommandQueue.h"
#include "LKZ/Core/Threading/ThreadManager.h"
#include "LKZ/Core/ECS/Manager/EntityManager.h"
#include "LKZ/Core/ECS/Manager/ComponentManager.h"
#include "LKZ/Core/ECS/Manager/NavMeshQueryManager.h"
#include "LKZ/Core/Manager/LobbyManager.h"
#include "LKZ/Core/Manager/ClientManager.h"
#include "LKZ/Core/Engine.h"
#include "LKZ/Protocol/Message/Entity/CreateEntityMessage.h"
#include "LKZ/Utility/Logger.h"
#include "LKZ/Utility/Constants.h"
#include <DetourCrowd.h>

// Include for Vector3
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

void RequestCreateEntityMessage::process(const sockaddr_in& senderAddr)
{
    Client* client = ClientManager::getClientByAddress(senderAddr);
    if (!client)
    {
        Logger::Log("RequestCreateEntityMessage: Client introuvable.", LogType::Warning);
        return;
    }

    Lobby* lobby = LobbyManager::getLobby(client->lobbyId);
    if (!lobby)
    {
        Logger::Log("RequestCreateEntityMessage: Lobby introuvable.", LogType::Warning);
        return;
    }

    int superTypeId = this->entitySuperTypeId;
    INetworkInterface* server = Engine::Instance().Server();

    // --- SPAWN PLAYER ---
    if (superTypeId == (int)EntitySuperType::Player)
    {
        CommandQueue::Instance().Push([=]() {
            auto& components = ComponentManager::Instance();
            Entity entity = EntityManager::Instance().CreateEntity(EntitySuperType(superTypeId), components, lobby);

            Logger::Log("Creating Player entity " + std::to_string(entity) +
                " for client " + client->ipAddress, LogType::Info);

            client->playerEntityId = entity;

            //// Calculate spawn position
            //float spawnX = 10.0f + rand() % 5;
            //float spawnY = 0.0f;
            //float spawnZ = 10.0f + rand() % 5;

            World& world = Engine::Instance().GetWorld();
            /*dtNavMeshQuery* simQuery = NavMeshQueryManager::GetThreadLocalQuery(world.getNavMesh());*/

			Vector3 playerSpawnPos = Constants::FIRST_PLAYER_SPAWN_POSITION;

            switch (client->positionInLobby)
            {
                case 1:
					playerSpawnPos = Constants::FIRST_PLAYER_SPAWN_POSITION;
				    break;
				case 2:
					playerSpawnPos = Constants::SECOND_PLAYER_SPAWN_POSITION;
                    break;
				case 3:
                    playerSpawnPos = Constants::THIRD_PLAYER_SPAWN_POSITION;
					break;
				case 4:
                    playerSpawnPos = Constants::FOURTH_PLAYER_SPAWN_POSITION;
					break;
            default:
                break;
            }

			std::cout << "Position in lobby: " << client->positionInLobby << std::endl;

            components.AddComponent(entity, PositionComponent{ playerSpawnPos });
            components.AddComponent(entity, RotationComponent{ Vector3{ 0.0f, 0.0f, 0.0f } });

            components.AddComponent(entity, PlayerInputComponent{ std::vector<PlayerInputData>() });

            CreateEntityMessage createEntityMsg;
            createEntityMsg.entityTypeId = (int)EntityType::Player1;
            createEntityMsg.entityId = entity;
            createEntityMsg.posX = playerSpawnPos.x;
            createEntityMsg.posY = playerSpawnPos.y;
            createEntityMsg.posZ = playerSpawnPos.z;

           /* lobby->addEntity(&entity);*/

            Serializer serializer;
            createEntityMsg.serialize(serializer);

            server->Send(senderAddr, serializer.getBuffer(), createEntityMsg.getClassName());

            createEntityMsg.entityTypeId = (int)EntityType::PlayerSynced1;
            createEntityMsg.serialize(serializer);

            server->SendToMultiple(LobbyManager::getClientsInLobby(lobby->id),
                serializer.getBuffer(),
                createEntityMsg.getClassName(),
                client);
            });
    }
    else // Zombie (or other AI)
    {
        // AI spawning is SLOW. Push to a worker thread first to find a valid point.
        ThreadManager::GetPool("pathfinding")->EnqueueTask([=]() {

            World& world = Engine::Instance().GetWorld();
            dtNavMeshQuery* simQuery = NavMeshQueryManager::GetThreadLocalQuery(world.getNavMesh());

            int randomChoice = (rand() % 4) + 1; 

            /*Vector3 randomSpawnPoint;

            if (randomChoice == 1) {
                randomSpawnPoint = Constants::FIRST_ZOMBIE_SPAWN_POSITION;
            }
            else if (randomChoice == 2) {
                randomSpawnPoint = Constants::SECOND_ZOMBIE_SPAWN_POSITION;
            }
            else if (randomChoice == 3) {
                randomSpawnPoint = Constants::THIRD_ZOMBIE_SPAWN_POSITION;
            }
            else {
                randomSpawnPoint = Constants::FOURTH_ZOMBIE_SPAWN_POSITION;
            }*/
            Vector3 randomSpawnPoint = world.getRandomNavMeshPoint(simQuery);
			/*Vector3 randomSpawnPoint = Constants::FIRST_PLAYER_SPAWN_POSITION;*/
            CommandQueue::Instance().Push([=]() {

                auto& components = ComponentManager::Instance();
                auto& world = Engine::Instance().GetWorld();
                Entity entity = EntityManager::Instance().CreateEntity(EntitySuperType(superTypeId), components, lobby);

                Logger::Log("Creating entity of type " + std::to_string(superTypeId) + " with ID " + std::to_string(entity), LogType::Info);

                // Initialize Position & Rotation
                components.AddComponent(entity, PositionComponent{ Vector3{ randomSpawnPoint.x, randomSpawnPoint.y, randomSpawnPoint.z } });
                components.AddComponent(entity, RotationComponent{ Vector3{ 0.0f, 0.0f, 0.0f } });

                Logger::Log("Spawned AI entity " + std::to_string(entity) +
                    " at (" + std::to_string(randomSpawnPoint.x) + ", " +
                    std::to_string(randomSpawnPoint.y) + ", " +
                    std::to_string(randomSpawnPoint.z) + ")", LogType::Info);



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
                    params.updateFlags = Constants::AGENT_UPDATE_FLAGS;

                    params.userData = &aiComp;

                    float spawnPos[3] = { randomSpawnPoint.x, randomSpawnPoint.y, randomSpawnPoint.z };
                    int agentIdx = crowd->addAgent(spawnPos, &params);

                    if (agentIdx != -1)
                    {
                        components.ai[entity].crowdAgentIndex = agentIdx;
                    }
                    else
                    {
                        Logger::Log("Failed to add agent to crowd (Max agents reached?)", LogType::Error);
                    }
                }

                CreateEntityMessage createEntityMsg;
                createEntityMsg.entityTypeId = 3 + rand() % 3;
                createEntityMsg.entityId = entity;
                createEntityMsg.posX = randomSpawnPoint.x;
                createEntityMsg.posY = randomSpawnPoint.y;
                createEntityMsg.posZ = randomSpawnPoint.z;

                /*lobby->addEntity(&entity);*/

                Serializer serializer;
                createEntityMsg.serialize(serializer);

                server->SendToMultiple(
                    LobbyManager::getClientsInLobby(lobby->id),
                    serializer.getBuffer(),
                    createEntityMsg.getClassName()
                );
                });
            });
    }
}