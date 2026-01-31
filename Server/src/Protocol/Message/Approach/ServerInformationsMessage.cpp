#include "LKZ/Protocol/Message/Approach/ServerInformationsMessage.h"

ServerInformationsMessage::ServerInformationsMessage() : status(false), playersCount(0) {}

uint8_t ServerInformationsMessage::getId() const
{
    return ID;
}

std::vector<uint8_t>& ServerInformationsMessage::serialize(Serializer& serializer) const
{
	serializer.writeByte(ID);
    serializer.writeBool(status);
    serializer.writeInt(playersCount);

    return serializer.getBuffer();
}

void ServerInformationsMessage::deserialize(Deserializer& deserializer)
{
  /*  deserializer.readBool();
    deserializer.readInt();*/
}

void ServerInformationsMessage::process(const sockaddr_in& senderAddr)
{
    /*LobbyManager::createLobby();

    /*int lastLobbyId = LobbyManager::getLastLobbyId();*/

   /* status = true;
	playersCount = ClientManager::getClients().size();

    Serializer serializer;
    serialize(serializer);

	Client* client = ClientManager::getClientByAddress(senderAddr);

    Engine::Instance().Server()->Send(senderAddr, serializer.getBuffer(), getClassName());*/
}
