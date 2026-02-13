#ifndef ENTITY_DEAD_MESSAGE_H
#define ENTITY_DEAD_MESSAGE_H

#include "LKZ/Protocol/Message/Message.h"

struct EntityDeadMessage : public Message
{
    static constexpr uint8_t ID = 20;

    EntityDeadMessage();

    EntityDeadMessage(uint16_t entityId);

    uint16_t entityId;

    uint8_t getId() const override;

    std::vector<uint8_t>& serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer) override;
    void process(const sockaddr_in& senderAddr, SOCKET tcpSocket) override;
};

#endif // ENTITY_DEAD_MESSAGE_H
