#ifndef ZOMBIE_HIT_MESSAGE_H
#define ZOMBIE_HIT_MESSAGE_H

#include "LKZ/Protocol/Message/Message.h"

struct ZombieHitMessage : public Message
{
    static constexpr uint8_t ID = 21;

    ZombieHitMessage();

    ZombieHitMessage(uint16_t entityId, uint16_t from, float posX, float posY, float posZ);

    uint16_t zombieId;
    uint16_t from;

    float posX;
	float posY;
	float posZ;

    uint8_t getId() const override;

    std::vector<uint8_t>& serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer) override;
    void process(const sockaddr_in& senderAddr, SOCKET tcpSocket) override;
};

#endif // ZOMBIE_BONE_SEVERED_MESSAGE_H
