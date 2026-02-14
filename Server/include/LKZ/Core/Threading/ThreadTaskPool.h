#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <atomic>
#include <string>
#include <memory>
#include <unordered_map>

class ThreadTaskPool
{
public:
    using LoopHook = std::function<void(float)>;

    ThreadTaskPool(LoopHook hook = nullptr, bool loopMode = false);
    ~ThreadTaskPool();

    void Start(int threadCount = 1);
    void Stop();
    void EnqueueTask(std::function<void()> task);
    void SetDeltaTime(float dt);

private:
    void WorkerThreadFunction(); 
    void SimulationThreadFunction(); 

private:
    std::vector<std::thread> workers; 
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;

    std::atomic<bool> running{ false };
    std::atomic<bool> stopRequested{ false };

    bool loopMode = false;
    LoopHook loopHook;
    std::atomic<float> deltaTime = 0.02f;
};

