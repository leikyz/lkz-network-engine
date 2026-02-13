#pragma once
#ifndef PLAYER_SHOOTING_MESSAGE_H
#define PLAYER_SHOOTING_MESSAGE_H

#include "LKZ/Protocol/Message/Message.h"

struct PlayerShootingMessage : public Message
{
    static constexpr uint8_t ID = 18;

    PlayerShootingMessage();
    PlayerShootingMessage(uint16_t entityId);

    uint16_t entityId = 0;

    uint8_t getId() const override;

    std::vector<uint8_t>& serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer) override;
    void process(const sockaddr_in& senderAddr, SOCKET tcpSocket) override;
};

#endif // PLAYER_SHOOTING_MESSAGE_H
