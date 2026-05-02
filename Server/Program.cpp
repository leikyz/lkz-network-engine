#include "LKZ/Core/Server/WindowsServer.h"
#include "LKZ/Core/Manager/MatchmakingManager.h"
#include "LKZ/Core/Engine.h"
#include "LKZ/Core/Threading/ThreadManager.h"
#include "LKZ/Simulation/Navmesh/NavMeshLoader.h" 
#include "LKZ/Simulation/World.h"
#include <iostream>
#include <thread>
#include <DetourCrowd.h>
#include <LKZ/Core/ECS/System/PlayerSystem.h>
#include "LKZ/Core/Threading/CommandQueue.h"
#include "LKZ/Utility/Logger.h"
#include <string>
#include <LKZ/Core/ECS/System/AISystem.h>
#include <LKZ/Core/ECS/System/PlayerSystem.h>
#include <Common/ProfilerProtocol.h>
#include <LKZ/Core/Manager/MetricsManager.h>

int main()
{
#ifdef _WIN32
    WindowsServer* server = new WindowsServer(Constants::UDP_PORT);
#else
    LinuxServer server = new LinuxServer();
#endif

    Engine& engine = Engine::Instance(server);
    engine.Initialize();

    ComponentManager& componentManager = ComponentManager::Instance();
    EntityManager& entityManager = EntityManager::Instance();
    SystemManager& systemManager = SystemManager::Instance();

    ThreadManager::CreatePool("logger", 1);

    World* world = new World();
    engine.SetWorld(world);
    world->initialize();

    systemManager.RegisterSystem(std::make_shared<PlayerSystem>());
    systemManager.RegisterSystem(std::make_shared<AISystem>());

    ThreadManager::CreatePool("io", 1);

    ThreadManager::GetPool("io")->EnqueueTask([server]()
        {
            while (true)
            {
                server->Poll();
            }
        });

    ThreadManager::CreatePool("profiler", 1, [&](float)
        {
            engine.GetProfiler()->Poll();

            // This is called constantly, but the internal 1-second check inside 
            MetricsManager::Instance().TryBroadcastMetrics(server);

        }, true);

    ThreadManager::CreatePool("pathfinding", 4);

    ThreadManager::CreatePool("simulation", 1, [&](float)
        {
            // Record the start time
            auto startTime = std::chrono::high_resolution_clock::now();

            auto& engine = Engine::Instance();
            auto& components = ComponentManager::Instance();

            CommandQueue::Instance().ProcessAllCommands();

            if (world)
                world->UpdateCrowd(Constants::FIXED_DELTA_TIME);

            systemManager.Update(components, Constants::FIXED_DELTA_TIME);

            auto endTime = std::chrono::high_resolution_clock::now();

            // Calculate the duration in microseconds
            auto durationUs = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();

            MetricsManager::Instance().currentMetrics.simulationTickTimeUs.store(durationUs);

        }, true);

    engine.Run();

    ThreadManager::StopAll();
    delete world;
    delete server;

    return 0;
}