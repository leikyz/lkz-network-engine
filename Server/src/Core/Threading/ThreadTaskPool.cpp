#include "LKZ/Core/Threading/ThreadTaskPool.h"
#include <chrono>
#include "LKZ/Core/ECS/Manager/NavMeshQueryManager.h"
#include "LKZ/Core/Engine.h"

#ifdef _WIN32
#define NOMINMAX 
#include <windows.h>
#include <timeapi.h> 

#pragma comment(lib, "winmm.lib") 
#endif

ThreadTaskPool::ThreadTaskPool(LoopHook hook, bool loopMode)
    : loopHook(hook), loopMode(loopMode)
{
}

ThreadTaskPool::~ThreadTaskPool()
{
    Stop();
}

void ThreadTaskPool::Start(int threadCount)
{
    if (running) return;
    running = true;
    stopRequested = false;

    for (int i = 0; i < threadCount; ++i)
    {
        if (loopMode) {
            workers.emplace_back(&ThreadTaskPool::SimulationThreadFunction, this);
        }
        else {
            workers.emplace_back(&ThreadTaskPool::WorkerThreadFunction, this);
        }
    }
}

void ThreadTaskPool::Stop()
{
    if (!running) return;

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        stopRequested = true;
    }
    condition.notify_all();

    for (auto& worker : workers) {
        if (worker.joinable()) worker.join();
    }
    workers.clear();
    running = false;
}

void ThreadTaskPool::EnqueueTask(std::function<void()> task)
{
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        tasks.push(std::move(task));
    }
    condition.notify_one();
}

void ThreadTaskPool::SetDeltaTime(float dt)
{
    deltaTime = dt;
}

void ThreadTaskPool::WorkerThreadFunction()
{
    while (true)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this]() { return stopRequested || !tasks.empty(); });

            if (stopRequested && tasks.empty())
                break;

            task = std::move(tasks.front());
            tasks.pop();
        }

        if (task) task();
    }

    NavMeshQueryManager::CleanupThreadQuery();
}

void ThreadTaskPool::SimulationThreadFunction()
{
#ifdef _WIN32
    timeBeginPeriod(1);
#endif

    const auto frameDuration = std::chrono::milliseconds(20);
    auto nextFrameStart = std::chrono::high_resolution_clock::now();

    while (!stopRequested)
    {
        float dt = 0.02f;
        if (loopHook) loopHook(dt);

        nextFrameStart += frameDuration;

        std::this_thread::sleep_until(nextFrameStart);
    }

#ifdef _WIN32
    timeEndPeriod(1);
#endif
    NavMeshQueryManager::CleanupThreadQuery();
}