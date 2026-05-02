#ifndef SERVER_METRICS_MESSAGE_H
#define SERVER_METRICS_MESSAGE_H

#include "LKZ/Protocol/Message/Message.h"
#include <cstdint>

class ServerMetricsMessage : public Message
{
public:
    // Make sure ID 34 is available, or change it to fit your protocol
    static constexpr uint8_t ID = 33;

    uint64_t simulationTickTimeUs;
    uint32_t activeEntityCount;
    uint64_t txKbps;
    uint64_t rxKbps;
    uint64_t packetsPerSecond;

    ServerMetricsMessage();

    uint8_t getId() const override { return ID; }

    std::vector<uint8_t>& serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer) override;
    void process(const sockaddr_in& senderAddr, SOCKET tcpSocket) override;
};

#endif // SERVER_METRICS_MESSAGE_H