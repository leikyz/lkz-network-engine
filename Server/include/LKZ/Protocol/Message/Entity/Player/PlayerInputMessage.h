#ifndef PLAYER_INPUT_MESSAGE_H
#define PLAYER_INPUT_MESSAGE_H

#include "LKZ/Protocol/Message/Message.h"

struct PlayerInputMessage : public Message
{
    static constexpr uint8_t ID = 10;

    PlayerInputMessage();
    PlayerInputMessage(uint16_t entityId, float inputX, float inputY, float yaw, bool isSprinting, bool isAiming, bool isArmed, int sequenceId);

    uint16_t entityId = 0;
    float inputX = 0.0f;
    float inputY = 0.0f;
    float yaw = 0.0f;

    bool isSprinting;
	bool isAiming;
	bool isArmed;

    int sequenceId = 0;

    uint8_t getId() const override;

    std::vector<uint8_t>& serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer) override;
    void process(const sockaddr_in& senderAddr, SOCKET tcpSocket) override;
};

#endif // PLAYER_INPUT_MESSAGE_H
