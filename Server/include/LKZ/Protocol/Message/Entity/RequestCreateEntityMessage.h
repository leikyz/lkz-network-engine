#ifndef REQUEST_CREATE_ENTITY_MESSAGE_H
#define REQUEST_CREATE_ENTITY_MESSAGE_H

#include "LKZ/Protocol/Message/Message.h"

struct RequestCreateEntityMessage : public Message
{
    static constexpr uint8_t ID = 14;

    RequestCreateEntityMessage();

    RequestCreateEntityMessage(int entityTypeId, uint16_t number = 0);

    uint8_t entitySuperTypeId;
    uint16_t number;

    uint8_t getId() const override;

    std::vector<uint8_t>& serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer) override;
    void process(const sockaddr_in& senderAddr, SOCKET tcpSocket) override;
};

#endif // REQUEST_CREATE_ENTITY_MESSAGE_H