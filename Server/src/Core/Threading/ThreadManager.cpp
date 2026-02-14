#include "LKZ/Core/Threading/ThreadManager.h"
#include "LKZ/Core/ECS/Manager/NavMeshQueryManager.h"
#include <iostream>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#include <timeapi.h>
#pragma comment(lib, "winmm.lib") 
#endif


void ThreadManager::CreatePool(const std::string& name, int threads, ThreadTaskPool::LoopHook hook, bool isLoop)
{
    std::lock_guard<std::mutex> lock(managerMutex);

    if (pools.contains(name)) {
        std::cerr << "[ThreadManager] Pool '" << name << "' already exists.\n";
        return;
    }

    auto pool = std::make_shared<ThreadTaskPool>(hook, isLoop);

    if (isLoop && threads > 1) {
        std::cout << "[ThreadManager] Warning: Loop pool '" << name << "' forced to 1 thread.\n";
        threads = 1;
    }

    std::cout << "[ThreadManager] Created pool '" << name << "' with " << threads << " thread(s).\n";

    pool->Start(threads); 
    pools[name] = pool;
}

ThreadManager::PoolPtr ThreadManager::GetPool(const std::string& name)
{
    std::lock_guard<std::mutex> lock(managerMutex);
    auto it = pools.find(name);
    if (it != pools.end()) return it->second;
    return nullptr;
}

void ThreadManager::StopAll()
{
    std::lock_guard<std::mutex> lock(managerMutex);
    for (auto& [name, pool] : pools) {
        pool->Stop();
    }
    pools.clear();
}

void ThreadManager::SetGlobalDeltaTime(float dt)
{
    std::lock_guard<std::mutex> lock(managerMutex);
    for (auto& [name, pool] : pools) {
        pool->SetDeltaTime(dt);
    }
}

