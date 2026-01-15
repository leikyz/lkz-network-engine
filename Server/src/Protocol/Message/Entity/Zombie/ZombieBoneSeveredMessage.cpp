#include "LKZ/Protocol/Message/Entity/Zombie/ZombieBoneSeveredMessage.h"
#include <cstdlib> 
#include <ctime>   
#include <LKZ/Core/ECS/Manager/EntityManager.h>

ZombieBoneSeveredMessage::ZombieBoneSeveredMessage() {};

ZombieBoneSeveredMessage::ZombieBoneSeveredMessage(uint16_t entityId, uint8_t boneID)
    : entityId(entityId), boneID(boneID)
{
}

uint8_t ZombieBoneSeveredMessage::getId() const
{
    return ID;
}

std::vector<uint8_t>& ZombieBoneSeveredMessage::serialize(Serializer& serializer) const
{
    serializer.reset();

    serializer.writeByte(ID);
    serializer.writeUInt16(entityId);
    serializer.writeByte(boneID);

    return serializer.getBuffer();
}

void ZombieBoneSeveredMessage::deserialize(Deserializer& deserializer)
{
    entityId = deserializer.readUInt16();
	boneID = deserializer.readByte();
}


void ZombieBoneSeveredMessage::process(const sockaddr_in& senderAddr)
{

    Client* client = ClientManager::getClientByAddress(senderAddr);
    if (!client) return;

    Lobby* lobby = LobbyManager::getLobby(client->lobbyId);
    if (!lobby) return;

    Serializer serializer;
    serialize(serializer);

    Engine::Instance().Server()->SendToMultiple(LobbyManager::getClientsInLobby(lobby->id), serializer.getBuffer(), getClassName(), client);
}