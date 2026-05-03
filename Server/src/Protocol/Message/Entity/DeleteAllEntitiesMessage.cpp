#include "LKZ/Protocol/Message/EntitY/DeleteAllEntitiesMessage.h"
#include <Common/ProfilerProtocol.h>
#include <LKZ/Protocol/Message/Profiler/ProfilerClientCreatedMessage.h>
#include <iostream>
#include <LKZ/Core/ECS/Manager/EntityManager.h>

DeleteAllEntitiesMessage::DeleteAllEntitiesMessage() {}

uint8_t DeleteAllEntitiesMessage::getId() const
{
    return ID;
}

std::vector<uint8_t>& DeleteAllEntitiesMessage::serialize(Serializer& serializer) const
{
    serializer.writeByte(ID);
    return serializer.getBuffer();
}

void DeleteAllEntitiesMessage::deserialize(Deserializer& deserializer)
{
}

void DeleteAllEntitiesMessage::process(const sockaddr_in& senderAddr, SOCKET tcpSocket)
{
    Session* session = SessionManager::GetSessionBySocket(tcpSocket);

    if (session != nullptr)
    {
        // Batch destroy all Zombies (2) and Primitives (3) for this session
        EntityManager::Instance().DestroyEntitiesBySession(session);

        std::cout << "Cleaned up entities" << std::endl;
    }
    else
    {
        std::cerr << "Failed to find session for entity deletion." << std::endl;
    }
}