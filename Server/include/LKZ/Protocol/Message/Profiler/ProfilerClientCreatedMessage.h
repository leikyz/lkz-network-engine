#ifndef PROFILER_CLIENT_CREATED_MESSAGE_H
#define PROFILER_CLIENT_CREATED_MESSAGE_H

#include "LKZ/Protocol/Message/Message.h"

struct ProfilerClientCreatedMessage : public Message
{
    static constexpr uint8_t ID = 23;

    ProfilerClientCreatedMessage();

    uint8_t getId() const override;

    std::vector<uint8_t>& serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer) override;
    void process(const sockaddr_in& senderAddr, SOCKET tcpSocket) override;
};

#endif // PROFILER_CLIENT_CREATED_MESSAGE_H

