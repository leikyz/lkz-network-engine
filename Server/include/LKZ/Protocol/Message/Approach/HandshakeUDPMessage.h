#ifndef HANDSHAKE_UDP_MESSAGE_H
#define HANDSHAKE_UDP_MESSAGE_H

#include "LKZ/Protocol/Message/Message.h"

struct HandshakeUDPMessage : public Message
{
    static constexpr uint8_t ID = 30;

    HandshakeUDPMessage();

    uint32_t sessionToken;
    uint32_t clientToken;

    uint8_t getId() const override;

    std::vector<uint8_t>& serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer) override;
    void process(const sockaddr_in& senderAddr, SOCKET tcpSocket) override;
};

#endif // HANDSHAKE_UDP_MESSAGE_H
