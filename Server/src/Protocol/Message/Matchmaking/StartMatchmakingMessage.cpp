#include "LKZ/Protocol/Message/Matchmaking/StartMatchmakingMessage.h"
#include "LKZ/Core/Manager/MatchmakingManager.h"

StartMatchmakingMessage::StartMatchmakingMessage() {}

uint8_t StartMatchmakingMessage::getId() const
{
    return ID;
}

std::vector<uint8_t>& StartMatchmakingMessage::serialize(Serializer& serializer) const
{
    serializer.writeByte(ID);
    return serializer.getBuffer();
}

void StartMatchmakingMessage::deserialize(Deserializer& deserializer)
{
    mapId = deserializer.readByte();
}

void StartMatchmakingMessage::process(const sockaddr_in& senderAddr)
{
    /*Serializer serializer;
    serialize(serializer);

	Client* client = ClientManager::getClientByAddress(senderAddr);
	client->matchmakingMapIdRequest = mapId;
	MatchmakingManager::AddPlayerToQueue(*client);

    Engine::Instance().Server()->Send(senderAddr, serializer.getBuffer(), getClassName());*/
}
