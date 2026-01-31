#include "LKZ/Protocol/Message/Entity/Player/PlayerAimTargetPositionMessage.h"
#include <LKZ/Core/ECS/Manager/ComponentManager.h>
#include <LKZ/Core/ECS/Manager/EntityManager.h>

PlayerAimTargetPositionMessage::PlayerAimTargetPositionMessage() {}

PlayerAimTargetPositionMessage::PlayerAimTargetPositionMessage(uint16_t entityId, float posX, float posY, float posZ)
    : entityId(entityId), posX(posX), posY(posY), posZ(posZ)
{
}

uint8_t PlayerAimTargetPositionMessage::getId() const
{
    return ID;
}

std::vector<uint8_t>& PlayerAimTargetPositionMessage::serialize(Serializer& serializer) const
{
    serializer.writeByte(ID);
    serializer.writeUInt16(entityId);
    serializer.writeFloat(posX);
    serializer.writeFloat(posY);
    serializer.writeFloat(posZ);

    return serializer.getBuffer();
}

void PlayerAimTargetPositionMessage::deserialize(Deserializer& deserializer)
{
    entityId = deserializer.readUInt16();
    posX = deserializer.readFloat();
    posY = deserializer.readFloat();
    posZ = deserializer.readFloat();
}

void PlayerAimTargetPositionMessage::process(const sockaddr_in& senderAddr)
{
    Serializer serializer;
    serialize(serializer);

    int sessionId = ClientManager::getClientByAddress(senderAddr)->lobbyId;
	Session* session = SessionManager::GetSession(sessionId);

    Engine::Instance().Server()->SendToMultiple(
        session->clientsAddress,
        serializer.getBuffer(),
        getClassName(),
        &senderAddr);
}


