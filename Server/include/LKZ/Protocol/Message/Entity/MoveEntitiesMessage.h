#ifndef MOVE_ENTITIES_MESSAGE_H
#define MOVE_ENTITIES_MESSAGE_H

#include "LKZ/Protocol/Message/Message.h"
#include <vector>
#include "LKZ/Simulation/Math/Vector.h"

struct MoveEntitiesMessage : public Message
{
    static constexpr uint8_t ID = 15;

    struct EntityUpdate
    {
        uint16_t entityId;
        float posX, posY, posZ;
    };

    std::vector<EntityUpdate> updates;

    MoveEntitiesMessage() = default;

    void addUpdate(uint16_t entityId, float x, float y, float z);

    uint8_t getId() const override;

    std::vector<uint8_t>& serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer) override;
    void process(const sockaddr_in& senderAddr, SOCKET tcpSocket) override;
};

#endif // MOVE_ENTITIES_MESSAGE_H
