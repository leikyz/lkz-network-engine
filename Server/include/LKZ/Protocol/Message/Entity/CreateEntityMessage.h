#ifndef CREATE_ENTITY_MESSAGE_H
#define CREATE_ENTITY_MESSAGE_H

#include "LKZ/Protocol/Message/Message.h"

struct CreateEntityMessage : public Message
{
    static constexpr uint8_t ID = 9;

    CreateEntityMessage();

    CreateEntityMessage(uint16_t entityId, int entityTypeId, float posX, float posY, float posZ);

    uint16_t entityId;
    int entityTypeId;

    float posX;
    float posY;
    float posZ;

    uint8_t getId() const override;

    std::vector<uint8_t>& serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer) override;
    void process(const sockaddr_in& senderAddr, SOCKET tcpSocket) override;
};

#endif // CREATE_ENTITY_MESSAGE_H
