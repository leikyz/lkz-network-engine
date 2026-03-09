#include "LKZ/Protocol/Message/Session/CreateSessionMessage.h"
#include <iostream>
#include <LKZ/Core/ECS/Manager/EntityManager.h>

CreateSessionMessage::CreateSessionMessage()
    : token(0), clientsCount(0) 
{
}

std::vector<uint8_t>& CreateSessionMessage::serialize(Serializer& serializer) const
{
    return serializer.getBuffer();
}

void CreateSessionMessage::deserialize(Deserializer& deserializer)
{
    token = deserializer.readInt();
    clientsCount = deserializer.readByte();

    for (int i = 0; i < clientsCount; i++)
    {
        clientIds.push_back(deserializer.readInt());
    }
}

void CreateSessionMessage::process(const sockaddr_in& senderAddr, SOCKET tcpSocket)
{
	Session* session = SessionManager::CreateSession(token, clientIds);
    ComponentManager& components = ComponentManager::Instance();

    Entity entity = EntityManager::Instance().CreateEntity(EntitySuperType(EntitySuperType::GameManager), components, session);
    session->sessionManager = entity;

}
