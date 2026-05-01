#include "LKZ/Core/Manager/MetricsManager.h"
#include "LKZ/Core/Server/WindowsServer.h" // Update this path if necessary
#include <iostream>

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
    // 1. Read the atomic variables safely using .load()
    long long simTime = currentMetrics.simulationTickTimeUs.load();
    int entityCount = currentMetrics.activeEntityCount.load();
    int bytesSent = currentMetrics.networkBytesSent.load();

    // 2. Here is where you format this data into your specific network packet
    // auto packet = ProfilerProtocol::CreateMetricsPacket(simTime, pathTime, entityCount, bytesSent);

    // 3. Send the packet to the clients
    /*
    if (server)
    {
        server->BroadcastToAll(packet);
    }
    */

    // Console output for debugging
   /* std::cout << "[Metrics] 1-Second Update -> Sim: " << simTime
        << "us | Path: " << pathTime
        << "us | Entities: " << entityCount << "\n";*/
}