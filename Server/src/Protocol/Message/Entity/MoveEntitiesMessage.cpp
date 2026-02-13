#include "LKZ/Protocol/Message/Entity/MoveEntitiesMessage.h"

void MoveEntitiesMessage::addUpdate(uint16_t entityId, float x, float y, float z)
{
    updates.push_back({ entityId, x, y, z });
}

uint8_t MoveEntitiesMessage::getId() const
{
    return ID;
}

std::vector<uint8_t>& MoveEntitiesMessage::serialize(Serializer& serializer) const
{
    serializer.writeByte(ID);
    serializer.writeUInt16(static_cast<uint16_t>(updates.size()));

    for (auto& u : updates)
    {
        serializer.writeUInt16(u.entityId);
        serializer.writeFloat(u.posX);
        serializer.writeFloat(u.posY);
        serializer.writeFloat(u.posZ);
    }

    return serializer.getBuffer();
}

void MoveEntitiesMessage::deserialize(Deserializer& deserializer)
{
}

void MoveEntitiesMessage::process(const sockaddr_in& senderAddr, SOCKET tcpSocket)
{
}
