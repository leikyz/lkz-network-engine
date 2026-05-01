#ifndef PING_MESSAGE_H
#define PING_MESSAGE_H

#include "LKZ/Protocol/Message/Message.h"
#include <cstdint>

class PingMessage : public Message
{
public:
    static constexpr uint8_t ID = 32;

    uint64_t clientTimestamp;

    PingMessage();
    PingMessage(uint64_t timestamp);

    uint8_t getId() const override;

    std::vector<uint8_t>& serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer) override;
    void process(const sockaddr_in& senderAddr, SOCKET tcpSocket) override;
};

#endif // PING_MESSAGE_H