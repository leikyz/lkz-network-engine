#include "LKZ/Core/Manager/MetricsManager.h"
#include "LKZ/Core/Server/WindowsServer.h" 
#include "LKZ/Core/Manager/SessionManager.h"
#include <iostream>
#include <LKZ/Protocol/Message/Telemetry/ServerMetricsMessage.h>

MetricsManager& MetricsManager::Instance()
{
    static MetricsManager instance;
    return instance;
}

void MetricsManager::TryBroadcastMetrics(WindowsServer* server)
{
    auto now = std::chrono::steady_clock::now();
    auto timeSinceLastSend = std::chrono::duration_cast<std::chrono::seconds>(now - lastSendTime).count();

    // Check if exactly 1 second (or more) has passed
    if (timeSinceLastSend >= 1)
    {
        Broadcast(server);
        lastSendTime = now; // Reset the timer
    }
}

void MetricsManager::Broadcast(WindowsServer* server)
{
    if (!server) return;

    // Extract and instantly reset the accumulators for the next second
    uint64_t bytesSentThisSec = currentMetrics.networkBytesSent.exchange(0, std::memory_order_relaxed);
    uint64_t bytesRecvThisSec = currentMetrics.networkBytesReceived.exchange(0, std::memory_order_relaxed);
    uint64_t ppsThisSec = currentMetrics.packetsPerSecond.exchange(0, std::memory_order_relaxed);

    // Read the standard variables
    long long simTime = currentMetrics.simulationTickTimeUs.load();
    int entityCount = currentMetrics.activeEntityCount.load();

    // Populate the Message
    ServerMetricsMessage msg;
    msg.simulationTickTimeUs = simTime;
    msg.activeEntityCount = entityCount;
    msg.txKbps = bytesSentThisSec / 1024; // Send as KB/s
    msg.rxKbps = bytesRecvThisSec / 1024; // Send as KB/s
    msg.packetsPerSecond = ppsThisSec;

    Serializer serializer;
    msg.serialize(serializer);
    const auto& buffer = serializer.getBuffer();

    std::vector<Session*> allSessions = SessionManager::GetAllSessions();
    for (Session* session : allSessions)
    {
        if (!session) continue;

        for (const SessionPlayer& player : session->players)
        {
            if (player.tcpSocket != INVALID_SOCKET)
            {
                server->SendReliable(player.tcpSocket, buffer);
            }
        }
    }

    // Console output for debugging (optional)
    /*
    std::cout << "[Metrics] 1-Second Update -> Sim: " << simTime
        << "us | Entities: " << entityCount
        << " | TX: " << msg.txKbps << " KB/s | RX: " << msg.rxKbps
        << " KB/s | PPS: " << msg.packetsPerSecond << "\n";
    */
}