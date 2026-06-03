#include "LKZ/Protocol/Message/Entity/Player/PlayerShootingMessage.h"
#include <LKZ/Core/ECS/Manager/ComponentManager.h>
#include <LKZ/Core/ECS/Manager/EntityManager.h>
#include <iostream>

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
    serializer.writeUInt16(5);
    serializer.writeByte(ID);
    serializer.writeUInt16(entityId);

    return serializer.getBuffer();
}

void PlayerShootingMessage::deserialize(Deserializer& deserializer)
{
    entityId = deserializer.readUInt16();
}

void PlayerShootingMessage::process(const sockaddr_in& senderAddr, SOCKET tcpSocket)
{
    Serializer serializer;
    serialize(serializer);

    Session* session = SessionManager::GetSessionBySocket(tcpSocket);

    if (!session)
    {
        return;
    }

    for (const auto& player : session->players)
    {
        if (player.tcpSocket == tcpSocket)
            continue; // Skip the sender

/*		std::cout << "Sending PlayerShootingMessage to player with TCP socket: " << player.tcpSocket << std::endl; */ 

        Engine::Instance().Server()->SendReliable(player.tcpSocket, serializer.getBuffer());
    }
}


