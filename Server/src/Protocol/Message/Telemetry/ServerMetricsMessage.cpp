#include "LKZ/Protocol/Message/Telemetry/ServerMetricsMessage.h" // Update path if necessary

ServerMetricsMessage::ServerMetricsMessage()
    : simulationTickTimeUs(0), activeEntityCount(0), txKbps(0), rxKbps(0), packetsPerSecond(0)
{
}

std::vector<uint8_t>& ServerMetricsMessage::serialize(Serializer& serializer) const
{
    uint16_t messageSize = 39;

    serializer.writeUInt16(messageSize);
    serializer.writeByte(ID);
    serializer.writeUInt64(simulationTickTimeUs);
    serializer.writeUInt32(activeEntityCount);
    serializer.writeUInt64(txKbps);
    serializer.writeUInt64(rxKbps);
    serializer.writeUInt64(packetsPerSecond);

    return serializer.getBuffer();
}

void ServerMetricsMessage::deserialize(Deserializer& deserializer)
{
}

void ServerMetricsMessage::process(const sockaddr_in& senderAddr, SOCKET tcpSocket)
{
}