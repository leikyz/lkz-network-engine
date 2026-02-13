#ifndef ROTATE_ENTITY_MESSAGE_H
#define ROTATE_ENTITY_MESSAGE_H

#include "LKZ/Protocol/Message/Message.h"

struct RotateEntityMessage : public Message
{
    static constexpr uint8_t ID = 12;

    RotateEntityMessage();

    RotateEntityMessage(uint16_t entityId, float rotaY);

    uint16_t entityId;

    float rotaY;

    uint8_t getId() const override;

    std::vector<uint8_t>& serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer) override;
    void process(const sockaddr_in& senderAddr, SOCKET tcpSocket) override;
};

#endif // ROTATE_ENTITY_MESSAGE_H
