#pragma once

#include <atomic>
#include <chrono>

// Forward declaration of your server class to avoid circular includes
class WindowsServer;

struct ServerMetrics
{
    // std::atomic ensures these can be read/written across threads safely
    std::atomic<long long> simulationTickTimeUs{ 0 };
    std::atomic<int> activeEntityCount{ 0 };
    std::atomic<uint64_t> networkBytesSent{ 0 };
    std::atomic<uint64_t> messagesPerSecond{ 0 };
    std::atomic<uint64_t> networkBytesReceived{ 0 };
};

class MetricsManager
{
public:
    static MetricsManager& Instance();
    ServerMetrics currentMetrics;

    void TryBroadcastMetrics(WindowsServer* server);

private:
    MetricsManager() = default;
    ~MetricsManager() = default;

    // Delete copy/move constructors to prevent accidental duplication
    MetricsManager(const MetricsManager&) = delete;
    MetricsManager& operator=(const MetricsManager&) = delete;

    // Timer tracking
    std::chrono::steady_clock::time_point lastSendTime = std::chrono::steady_clock::now();

    void Broadcast(WindowsServer* server);
};