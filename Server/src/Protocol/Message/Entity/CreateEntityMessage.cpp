#include "LKZ/Protocol/Message/Entity/CreateEntityMessage.h"
#include <cstdlib> 
#include <ctime>   
#include <LKZ/Core/ECS/Manager/EntityManager.h>

CreateEntityMessage::CreateEntityMessage() {};

CreateEntityMessage::CreateEntityMessage(uint16_t entityId, int entityTypeId, float posX, float posY, float posZ)
    : entityId(entityId), entityTypeId(entityTypeId), posX(posX), posY(posY), posZ(posZ)
{
}

uint8_t CreateEntityMessage::getId() const
{
    return ID;
}

std::vector<uint8_t>& CreateEntityMessage::serialize(Serializer& serializer) const
{
    serializer.reset();

    serializer.writeByte(ID);
    serializer.writeUInt16(entityId);
    serializer.writeInt(entityTypeId);
    serializer.writeFloat(posX);
    serializer.writeFloat(posY);
    serializer.writeFloat(posZ);

    return serializer.getBuffer();
}

void CreateEntityMessage::deserialize(Deserializer& deserializer)
{
}


void CreateEntityMessage::process(const sockaddr_in& senderAddr, SOCKET tcpSocket)
{
    
    
}