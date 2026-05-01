#ifndef CREATE_SESSION_MESSAGE_H
#define CREATE_SESSION_MESSAGE_H

#include "LKZ/Protocol/Message/Message.h"

class CreateSessionMessage : public Message
{
public:
    static constexpr uint8_t ID = 33;

    uint16_t animatedEntitiesCount;
    uint16_t primitiveEntitiesCount;
    uint8_t clientsCount;
    std::vector<uint32_t> clientIds;

    CreateSessionMessage();

    uint8_t getId() const override { return ID; }

    std::vector<uint8_t>& serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer) override;
    void process(const sockaddr_in& senderAddr, SOCKET tcpSocket) override;

};

#endif // CREATE_SESSION_MESSAGE_H