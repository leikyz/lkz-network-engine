#ifndef MOVE_ENTITY_MESSAGE_H
#define MOVE_ENTITY_MESSAGE_H

#include "LKZ/Protocol/Message/Message.h"

struct MoveEntityMessage : public Message
{
    static constexpr uint8_t ID = 11;

    MoveEntityMessage();

    MoveEntityMessage(uint16_t entityId, float posX, float posY, float posZ);

    uint16_t entityId;

    float posX;
    float posY;
	float posZ;

    uint8_t getId() const override;

    std::vector<uint8_t>& serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer) override;
    void process(const sockaddr_in& senderAddr, SOCKET tcpSocket) override;
};

#endif // MOVE_ENTITY_MESSAGE_H
