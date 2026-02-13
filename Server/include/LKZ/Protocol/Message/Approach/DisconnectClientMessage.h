#ifndef DISCONNECT_CLIENT_MESSAGE_H
#define DISCONNECT_CLIENT_MESSAGE_H

#include "LKZ/Protocol/Message/Message.h"

struct DisconnectClientMessage : public Message
{
    static constexpr uint8_t ID = 3;

    DisconnectClientMessage();

    uint8_t getId() const override;

    std::vector<uint8_t>& serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer) override;
    void process(const sockaddr_in& senderAddr, SOCKET tcpSocket) override;
};

#endif // DISCONNECT_CLIENT_MESSAGE_H
