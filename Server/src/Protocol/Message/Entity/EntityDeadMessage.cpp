#include "LKZ/Protocol/Message/Entity/EntityDeadMessage.h"
#include <cstdlib> 
#include <ctime>   
#include <LKZ/Core/ECS/Manager/EntityManager.h>
#include <LKZ/Core/ECS/Component/Component.h>
#include <LKZ/Core/ECS/Manager/EntityManager.h>
#include <iostream>

EntityDeadMessage::EntityDeadMessage() {};

EntityDeadMessage::EntityDeadMessage(uint16_t entityId)
    : entityId(entityId)
{
}

uint8_t EntityDeadMessage::getId() const
{
    return ID;
}

std::vector<uint8_t>& EntityDeadMessage::serialize(Serializer& serializer) const
{
    serializer.reset();

    serializer.writeUInt16(5);
    serializer.writeByte(ID);
    serializer.writeUInt16(entityId);

    return serializer.getBuffer();
}

void EntityDeadMessage::deserialize(Deserializer& deserializer)
{
	entityId = deserializer.readUInt16();
}


void EntityDeadMessage::process(const sockaddr_in& senderAddr, SOCKET tcpSocket)
{
    Session* session = SessionManager::GetSessionBySocket(tcpSocket);

    if (!session)
        return;

    ComponentManager& components = ComponentManager::Instance();

	/*WaveComponent* waveComponent = components.GetComponent<WaveComponent>(EntityManager::Instance().GetEntityById(lobby->gameWaveEntity, lobby));

    waveComponent->zombiesAlive--;*/

	EntityManager::Instance().DestroyEntity(entityId);

    std::cout << "entity destroyed" << std::endl;

    Serializer serializer;
    serialize(serializer);

    for (const auto& player : session->players)
    {
        Engine::Instance().Server()->SendReliable(player.tcpSocket, serializer.getBuffer());

    }

}