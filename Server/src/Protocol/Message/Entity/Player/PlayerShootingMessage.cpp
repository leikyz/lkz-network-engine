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
    Serializer serializer;
    serialize(serializer);

    int lobbyId = ClientManager::getClientByAddress(senderAddr)->lobbyId;
    Lobby* lobby = LobbyManager::getLobby(lobbyId);

    Engine::Instance().Server()->SendToMultiple(
        LobbyManager::getClientsInLobby(lobby->id),
        serializer.getBuffer(),
        getClassName(),
        ClientManager::getClientByAddress(senderAddr));
}


