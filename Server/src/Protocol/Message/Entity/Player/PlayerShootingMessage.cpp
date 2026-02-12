#include "LKZ/Protocol/Message/Entity/Player/PlayerShootingMessage.h"
#include <LKZ/Core/ECS/Manager/ComponentManager.h>
#include <LKZ/Core/ECS/Manager/EntityManager.h>

PlayerShootingMessage::PlayerShootingMessage() {}

PlayerShootingMessage::PlayerShootingMessage(uint16_t entityId)
    : entityId(entityId)
{
}

uint8_t PlayerShootingMessage::getId() const
{
    return ID;
}

std::vector<uint8_t>& PlayerShootingMessage::serialize(Serializer& serializer) const
{
    serializer.writeByte(ID);
    serializer.writeUInt16(entityId);

    return serializer.getBuffer();
}

void PlayerShootingMessage::deserialize(Deserializer& deserializer)
{
    entityId = deserializer.readUInt16();
}

void PlayerShootingMessage::process(const sockaddr_in& senderAddr)
{
   /* Serializer serializer;
    serialize(serializer);

    int sessionId = ClientManager::getClientByAddress(senderAddr)->lobbyId;
    Session* session = SessionManager::GetSession(sessionId);

    Engine::Instance().Server()->SendToMultiple(
        session->clientsAddress,
        serializer.getBuffer(),
        getClassName(),
        &senderAddr);*/
}


