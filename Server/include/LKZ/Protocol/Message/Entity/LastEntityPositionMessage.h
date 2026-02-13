#ifndef LAST_ENTITY_POSITION_MESSAGE_H
#define LAST_ENTITY_POSITION_MESSAGE_H

#include "LKZ/Protocol/Message/Message.h"

struct LastEntityPositionMessage : public Message
{
    static constexpr uint8_t ID = 13;

    LastEntityPositionMessage();

    LastEntityPositionMessage(uint16_t entityId, float posX, float posY, float posZ, float velocityX, float velocityY, float velocityZ, uint32_t lastProcessedInput);

    uint16_t entityId;

    float posX;
    float posY;
    float posZ;

	float velocityX;
	float velocityY;
	float velocityZ;

    uint32_t lastProcessedInput;

    uint8_t getId() const override;

    std::vector<uint8_t>& serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer) override;
    void process(const sockaddr_in& senderAddr, SOCKET tcpSocket) override;
};

#endif // LAST_ENTITY_POSITION_MESSAGE_H
