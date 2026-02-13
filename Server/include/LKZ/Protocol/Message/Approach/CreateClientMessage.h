#ifndef CREATE_CLIENT_MESSAGE_H
#define CREATE_CLIENT_MESSAGE_H

#include "LKZ/Protocol/Message/Message.h"

struct CreateClientMessage : public Message
{
    static constexpr uint8_t ID = 1;

    CreateClientMessage();

    uint8_t getId() const override;

    std::vector<uint8_t>& serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer) override;
    void process(const sockaddr_in& senderAddr, SOCKET tcpSocket) override;
};

#endif // CREATE_CLIENT_MESSAGE_H
