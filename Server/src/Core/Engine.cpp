#include "LKZ/Core/Engine.h"
#include "LKZ/Utility/Constants.h"
#include <iostream>
#include <thread>
#include <vector>
#include <Common/ProfilerProtocol.h>
#include <LKZ/Protocol/Message/Profiler/ProfilerNetworkPerformanceMessage.h>

INetworkInterface* Engine::network = nullptr;

Engine::Engine(INetworkInterface* netInterface)
{
    network = netInterface;
    // Initialisation du serveur profiler sur le port 5001
    profiler = std::make_unique<ProfilerServer>(ProfilerProtocol::PROFILER_PORT);
}

void Engine::Initialize()
{
    std::cout << "\033[34m";
    std::cout << R"(
                                                       _    _  __ ____           
                                                      | |  | |/ /|_  /           
                                                      | |__| ' <  / /            
                                           _  _ ___ __|____|_|\_\/___|  ___ _  __
                                          | \| | __|_   _\ \    / / _ \| _ \ |/ /
                                          | .` | _|  | |  \ \/\/ / (_) |   / ' < 
                                          |_|\_|___| |_|   \_/\_/ \___/|_|_\_|\_\
                                         
    )" << std::endl;
    std::cout << "\033[0m";
    std::cout << "[Main] Starting engine...\n";

    lastFrame = std::chrono::steady_clock::now();

    network->Start();

    if (profiler) {
        std::cout << "[Profiler] Starting Profiler...\n";
        profiler->Start();
    }
}

void Engine::Run()
{
    const float targetFrameTime = 1.0f / 60.0f;
    float accumulator = 0.0f;

    float profilerTimer = 0.0f;
    int framesCounted = 0;
    float accumulatedDeltaTime = 0.0f;

    lastFrame = std::chrono::steady_clock::now();

    while (true)
    {
        auto now = std::chrono::steady_clock::now();
        deltaTime = std::chrono::duration<float>(now - lastFrame).count();
        lastFrame = now;

        accumulator += deltaTime;

        profilerTimer += deltaTime;
        accumulatedDeltaTime += deltaTime;
        framesCounted++;

        ThreadManager::SetGlobalDeltaTime(deltaTime);

        while (accumulator >= Constants::FIXED_DELTA_TIME)
        {
            ThreadManager::SetGlobalDeltaTime(Constants::FIXED_DELTA_TIME);
            accumulator -= Constants::FIXED_DELTA_TIME;

        }

        if (profiler && profilerTimer >= 0.1f)
        {
            float avgDeltaTime = accumulatedDeltaTime / framesCounted;
            float avgFps = (avgDeltaTime > 0.0f) ? (1.0f / avgDeltaTime) : 0.0f;

            ProfilerNetworkPerformanceMessage msg;
            Serializer s;

            msg.deltaTime = avgDeltaTime;
            msg.fps = avgFps;

            msg.serialize(s);
            profiler->Broadcast(s.getBuffer());

            profilerTimer = 0.0f;
            framesCounted = 0;
            accumulatedDeltaTime = 0.0f;
        }

        network->Poll();

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

INetworkInterface* Engine::Server()
{
    return network;
}