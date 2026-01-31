#include <LKZ/Core/ECS/System/WaveSystem.h>
#include <LKZ/Core/ECS/Manager/ComponentManager.h>
#include <LKZ/Core/ECS/Manager/EntityManager.h>
#include <LKZ/Core/Engine.h>
#include <LKZ/Core/Threading/CommandQueue.h>
#include <LKZ/Core/Threading/ThreadManager.h>
#include <LKZ/Protocol/Message/Entity/CreateEntityMessage.h>
#include <LKZ/Protocol/Message/Gameplay/ChangeWaveMessage.h>
#include <LKZ/Utility/Logger.h>
#include <LKZ/Utility/Constants.h>
#include "LKZ/Core/ECS/Manager/NavMeshQueryManager.h"
#include <DetourCrowd.h>

void WaveSystem::Update(ComponentManager& components, float fixedDeltaTime)
{
    /*for (auto& [entity, waveComponent] : components.waves)
    {
		Logger::Log("Zombie to spawn " + std::to_string(waveComponent.zombiesAlive), LogType::Debug);

        Lobby* lobby = LobbyManager::getLobby(waveComponent.lobbyId);

        if (!lobby || !lobby->inGame) continue;

        if (lobby->gameLoaded)
        {
            if (waveComponent.isIntermission)
            {
                waveComponent.stateTimer -= fixedDeltaTime;

                if (waveComponent.stateTimer <= 0.0f)
                {
                    waveComponent.currentWave++;
                    waveComponent.isIntermission = false;

                    int playerCount = lobby->getClientCount();
                    int baseZombies = 6;
                    float multiplier = 1.15f;

                    waveComponent.zombiesToSpawn = (int)((baseZombies * std::pow(multiplier, waveComponent.currentWave - 1)) * playerCount);

                    waveComponent.spawnTimer = 0.0f;

                    ChangeWaveMessage waveMsg;
                    waveMsg.wave = waveComponent.currentWave;

                    Serializer s;
                    waveMsg.serialize(s);
                    Engine::Instance().Server()->SendToMultiple(lobby->clients, s.getBuffer(), waveMsg.getClassName());
                }
                continue;
            }

            if (waveComponent.zombiesToSpawn <= 0 && waveComponent.zombiesAlive <= 0)
            {
                waveComponent.isIntermission = true;
                waveComponent.stateTimer = 10.0f; 
                Logger::Log("Wave Complete! Starting Intermission.", LogType::Info);
                return;
            }

            if (waveComponent.zombiesToSpawn > 0)
            {
                if (waveComponent.zombiesAlive < MAX_ZOMBIES_ON_MAP)
                {
                    waveComponent.spawnTimer -= fixedDeltaTime;

                    if (waveComponent.spawnTimer <= 0.0f)
                    {

                        SpawnZombie(lobby->id, (int)EntitySuperType::Zombie); 

                        waveComponent.zombiesToSpawn--;
                        waveComponent.zombiesAlive++;

                        waveComponent.spawnTimer = 2.0f;
                    }
                }
            }
        }
    }*/
}

void WaveSystem::SpawnZombie(int lobbyId, int entitySuperTypeId)
{
    //ThreadManager::GetPool("pathfinding")->EnqueueTask([=]() {

    //    World& world = Engine::Instance().GetWorld();

    //    Vector3 randomSpawnPoint;
    //    const Vector3 zombieSpawns[] = {
    //         Constants::FIRST_ZOMBIE_SPAWN_POSITION,
    //         Constants::SECOND_ZOMBIE_SPAWN_POSITION,
    //         Constants::THIRD_ZOMBIE_SPAWN_POSITION,
    //         Constants::FOURTH_ZOMBIE_SPAWN_POSITION
    //            };


    //    int randomIndex = std::rand() % 4;

    //    randomSpawnPoint = zombieSpawns[randomIndex];

    //    CommandQueue::Instance().Push([=]() {

    //        Lobby* lobby = LobbyManager::getLobby(lobbyId);
    //        if (!lobby) return; 

    //        auto& components = ComponentManager::Instance();
    //        auto& entityManager = EntityManager::Instance();
    //        INetworkInterface* server = Engine::Instance().Server();

    //        Entity entity = entityManager.CreateEntity(EntitySuperType(entitySuperTypeId), components, lobby);

    //        components.AddComponent(entity, PositionComponent{ randomSpawnPoint });
    //        components.AddComponent(entity, RotationComponent{ Vector3{ 0.0f, 0.0f, 0.0f } });

    //        float initialRepathDelay = ((rand() % 100) / 100.0f) * 2.0f;
    //        Vector3 initialTarget = { 0,0,0 }; 

    //        components.AddComponent(entity, AIComponent{
    //            initialTarget,
    //            initialRepathDelay,
    //            -1, 
    //            0.0f
    //            });

    //        dtCrowd* crowd = Engine::Instance().GetWorld().getCrowd();
    //        if (crowd)
    //        {
    //            dtCrowdAgentParams params;
    //            memset(&params, 0, sizeof(params));

    //            params.radius = Constants::AGENT_RADIUS;
    //            params.height = Constants::AGENT_HEIGHT;
    //            params.maxAcceleration = Constants::AGENT_MAX_ACCELERATION;
    //            params.maxSpeed = Constants::AGENT_MAX_SPEED;
    //            params.collisionQueryRange = params.radius * 8.0f;
    //            params.pathOptimizationRange = params.radius * 30.0f;
    //            params.queryFilterType = Constants::AGENT_QUERY_FILTER_TYPE;
    //            params.obstacleAvoidanceType = Constants::AGENT_OBSTACLE_AVOIDANCE_TYPE;
    //            params.separationWeight = Constants::AGENT_SEPARATION_WEIGHT;
    //            params.updateFlags = Constants::AGENT_UPDATE_FLAGS;
    //            params.userData = (void*)((uintptr_t)entity);

    //            float spawnPos[3] = { randomSpawnPoint.x, randomSpawnPoint.y, randomSpawnPoint.z };
    //            int agentIdx = crowd->addAgent(spawnPos, &params);

    //            if (agentIdx != -1) {
    //                components.ai[entity].crowdAgentIndex = agentIdx;
    //            }
    //        }

    //   /*     lobby->addEntity(&entity);*/

    //        CreateEntityMessage createEntityMsg;

    //        createEntityMsg.entityTypeId = 3 + rand() % 3;
    //        createEntityMsg.entityId = entity;
    //        createEntityMsg.posX = randomSpawnPoint.x;
    //        createEntityMsg.posY = randomSpawnPoint.y;
    //        createEntityMsg.posZ = randomSpawnPoint.z;

    //        Serializer serializer;
    //        createEntityMsg.serialize(serializer);

    //        server->SendToMultiple(
    //            LobbyManager::getClientsInLobby(lobby->id),
    //            serializer.getBuffer(),
    //            createEntityMsg.getClassName()
    //        );
    //        });
    //    });
}