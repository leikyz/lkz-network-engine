#include "LKZ/Protocol/Message/Approach/ClientGameSceneLoadedMessage.h"
#include <LKZ/Core/ECS/Manager/ComponentManager.h>
#include <LKZ/Core/ECS/Manager/EntityManager.h>
#include <LKZ/Protocol/Message/Gameplay/ChangeWaveMessage.h>

ClientGameSceneLoadedMessage::ClientGameSceneLoadedMessage() {}

uint8_t ClientGameSceneLoadedMessage::getId() const
{
    return ID;
}

std::vector<uint8_t>& ClientGameSceneLoadedMessage::serialize(Serializer& serializer) const
{
	serializer.reset();

    serializer.writeByte(ID);
    return serializer.getBuffer();
}

void ClientGameSceneLoadedMessage::deserialize(Deserializer& deserializer)
{

}

void ClientGameSceneLoadedMessage::process(const sockaddr_in& senderAddr, SOCKET tcpSocket)
{
 //   Client* client = ClientManager::getClientByAddress(senderAddr);

	//Session* session = SessionManager::GetSession(client->lobbyId);

 //   if (!client)
 //   {
 //       Logger::Log("ClientGameSceneLoadedMessage: Client not found.", LogType::Warning);
 //       return;
	//}

 //   if (!lobby)
 //   {
 //       Logger::Log("ClientGameSceneLoadedMessage: Lobby not found.", LogType::Warning);
 //       return;
 //   }

 //   Serializer serializer;
 //   serialize(serializer);

 //   Engine::Instance().Server()->SendToMultiple(LobbyManager::getClientsInLobby(lobby->id), serializer.getBuffer(), getClassName());

	//lobby->gameLoaded = true;
	//// first wave start

 //   ChangeWaveMessage changeWaveMsg(1);
 //   changeWaveMsg.serialize(serializer);

 //   Engine::Instance().Server()->SendToMultiple(
 //       LobbyManager::getClientsInLobby(lobby->id),
 //       serializer.getBuffer(),
 //       changeWaveMsg.getClassName());
}
