#include "LKZ/Protocol/Message/Approach/ClientGameSceneLoadedMessage.h"
#include <LKZ/Core/ECS/Manager/ComponentManager.h>
#include <LKZ/Core/ECS/Manager/EntityManager.h>
#include <LKZ/Protocol/Message/Gameplay/ChangeWaveMessage.h>
#include <iostream>

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
	std::cout << "scene loader received from client" << std::endl;
	Session* session = SessionManager::GetSessionBySocket(tcpSocket);
	session->isInGame = true;

 
}
