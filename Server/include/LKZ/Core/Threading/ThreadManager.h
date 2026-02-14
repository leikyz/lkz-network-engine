#pragma once
#include "ThreadTaskPool.h"
#include <unordered_map>
#include <memory>
#include <string>
#include <mutex>

class ThreadManager
{
public:
    using PoolPtr = std::shared_ptr<ThreadTaskPool>;

    static void CreatePool(const std::string& name, int threads, ThreadTaskPool::LoopHook hook = nullptr, bool isLoop = false);
    static PoolPtr GetPool(const std::string& name);
    static void StopAll();
    static void SetGlobalDeltaTime(float dt);

private:
    static inline std::unordered_map<std::string, PoolPtr> pools;
    static inline std::mutex managerMutex;
};
