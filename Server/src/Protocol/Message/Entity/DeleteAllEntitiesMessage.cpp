#include "LKZ/Protocol/Message/EntitY/DeleteAllEntitiesMessage.h"
#include <Common/ProfilerProtocol.h>
#include <LKZ/Protocol/Message/Profiler/ProfilerClientCreatedMessage.h>
#include <iostream>
#include <LKZ/Core/ECS/Manager/EntityManager.h>
#include "DetourCrowd.h"
DeleteAllEntitiesMessage::DeleteAllEntitiesMessage() {}

uint8_t DeleteAllEntitiesMessage::getId() const
{
    return ID;
}

std::vector<uint8_t>& DeleteAllEntitiesMessage::serialize(Serializer& serializer) const
{
    serializer.writeUInt16(3);
    serializer.writeByte(ID);
    return serializer.getBuffer();
}

void DeleteAllEntitiesMessage::deserialize(Deserializer& deserializer)
{
}

void DeleteAllEntitiesMessage::process(const sockaddr_in& senderAddr, SOCKET tcpSocket)
{
    Session* session = SessionManager::GetSessionBySocket(tcpSocket);

    if (session == nullptr)
    {
        std::cerr << "Failed to find session for entity deletion." << std::endl;
        return;
    }

    Serializer serializer;
    serialize(serializer);

    for (const auto& player : session->players)
    {
        if (player.tcpSocket != INVALID_SOCKET)
        {
            Engine::Instance().Server()->SendReliable(player.tcpSocket, serializer.getBuffer());
        }
    }

    EntityManager::Instance().DestroyEntitiesBySession(session);

    std::cout << "Cleaned up entities and freed simulation slots successfully." << std::endl;
}