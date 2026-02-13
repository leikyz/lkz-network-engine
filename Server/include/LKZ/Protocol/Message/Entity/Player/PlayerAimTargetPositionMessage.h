#ifndef PLAYER_AIM_TARGET_POSITION_MESSAGE_H
#define PLAYER_AIM_TARGET_POSITION_MESSAGE_H

#include "LKZ/Protocol/Message/Message.h"

struct PlayerAimTargetPositionMessage : public Message
{
    static constexpr uint8_t ID = 17;

    PlayerAimTargetPositionMessage();
    PlayerAimTargetPositionMessage(uint16_t entityId, float posX, float posY, float posZ);

    uint16_t entityId = 0;
    float posX = 0.0f;
    float posY = 0.0f;
    float posZ = 0.0f;

    uint8_t getId() const override;

    std::vector<uint8_t>& serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer) override;
    void process(const sockaddr_in& senderAddr, SOCKET tcpSocket) override;
};

#endif // PLAYER_AIM_TARGET_POSITION_MESSAGE_H
