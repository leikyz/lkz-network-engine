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
#include <LKZ/Core/ECS/System/WaveSystem.h>
#include <Common/ProfilerProtocol.h>



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
    systemManager.RegisterSystem(std::make_shared<WaveSystem>());
    systemManager.RegisterSystem(std::make_shared<AISystem>());

    ThreadManager::CreatePool("io", 1, [server](float) { server->Poll(); }, false);
    ThreadManager::CreatePool("profiler", 1, [&](float) { engine.GetProfiler()->Poll(); }, true);
    ThreadManager::CreatePool("pathfinding", 2);
    ThreadManager::CreatePool("simulation", 1, [&](float)
        {
            auto& engine = Engine::Instance();
            auto& components = ComponentManager::Instance();

            CommandQueue::Instance().ProcessAllCommands();
          
            if (world)
                world->UpdateCrowd(Constants::FIXED_DELTA_TIME);

            systemManager.Update(components, Constants::FIXED_DELTA_TIME);

        }, true);


    engine.Run();
    
    ThreadManager::StopAll();
    delete world;
    delete server;

    return 0;
}
