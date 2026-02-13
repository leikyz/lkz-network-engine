#ifndef PROFILER_NETWORK_PERFORMANCE_MESSAGE_H
#define PROFILER_NETWORK_PERFORMANCE_MESSAGE_H

#include "LKZ/Protocol/Message/Message.h"

struct ProfilerNetworkPerformanceMessage : public Message
{
    static constexpr uint8_t ID = 24;

    ProfilerNetworkPerformanceMessage();

    uint8_t getId() const override;

	float deltaTime;
    float fps;

    std::vector<uint8_t>& serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer) override;
    void process(const sockaddr_in& senderAddr, SOCKET tcpSocket) override;
};

#endif // PROFILER_NETWORK_PERFORMANCE_MESSAGE_H

