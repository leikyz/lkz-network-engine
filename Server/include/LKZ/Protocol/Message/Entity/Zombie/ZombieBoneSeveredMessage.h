#ifndef ZOMBIE_BONE_SEVERED_MESSAGE_H
#define ZOMBIE_BONE_SEVERED_MESSAGE_H

#include "LKZ/Protocol/Message/Message.h"

struct ZombieBoneSeveredMessage : public Message
{
    static constexpr uint8_t ID = 19;

    ZombieBoneSeveredMessage();

    ZombieBoneSeveredMessage(uint16_t entityId, uint8_t boneID);

    uint16_t entityId;
    uint8_t boneID;

    uint8_t getId() const override;

    std::vector<uint8_t>& serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer) override;
    void process(const sockaddr_in& senderAddr, SOCKET tcpSocket) override;
};

#endif // ZOMBIE_BONE_SEVERED_MESSAGE_H
