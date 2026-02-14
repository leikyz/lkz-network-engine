#pragma once
#include "LKZ/Core/Server/INetworkInterface.h"
#include "LKZ/Core/Server/ProfilerServer.h" // Ajout
#include "LKZ/Core/Threading/ThreadManager.h"
#include "LKZ/Simulation/World.h"
#include <chrono>
#include <memory>

class Engine
{
public:
    static Engine& Instance(INetworkInterface* netInterface = nullptr)
    {
        static Engine instance(netInterface);
        return instance;
    }

    void Initialize();
    void Run();

    float GetDeltaTime() const { return deltaTime; }
    World& GetWorld() const { return *world; }
	bool IsRunning() const { return isRunning; }
    void SetWorld(World* newWorld) { world = newWorld; }

    static INetworkInterface* Server();

    // Accès au profiler
    ProfilerServer* GetProfiler() { return profiler.get(); }

private:
    Engine(INetworkInterface* netInterface);
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    static INetworkInterface* network;

    // Pointeur intelligent vers le profiler
    std::unique_ptr<ProfilerServer> profiler;
    bool isRunning;
    World* world;
    std::chrono::steady_clock::time_point lastFrame;
    float deltaTime = 0.0f;
};