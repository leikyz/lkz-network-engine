#include "LKZ/Protocol/Message/Entity/EntityDeadMessage.h"
#include <cstdlib> 
#include <ctime>   
#include <LKZ/Core/ECS/Manager/EntityManager.h>
#include <LKZ/Core/ECS/Component/Component.h>
#include <LKZ/Core/ECS/Manager/EntityManager.h>

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

    serializer.writeByte(ID);
    serializer.writeUInt16(entityId);

    return serializer.getBuffer();
}

void EntityDeadMessage::deserialize(Deserializer& deserializer)
{
	entityId = deserializer.readUInt16();
}


void EntityDeadMessage::process(const sockaddr_in& senderAddr)
{
    /*Client* client = ClientManager::getClientByAddress(senderAddr);
    Session* session = SessionManager::GetSession(client->lobbyId);

    if (!client || !session)
        return;

    ComponentManager& components = ComponentManager::Instance();

	WaveComponent* waveComponent = components.GetComponent<WaveComponent>(EntityManager::Instance().GetEntityById(lobby->gameWaveEntity, lobby));

    waveComponent->zombiesAlive--;

	EntityManager::Instance().DestroyEntity(entityId);

    Serializer serializer;
    serialize(serializer);

    Engine::Instance().Server()->SendToMultiple(
        session->clientsAddress,
        serializer.getBuffer(),
        getClassName()
    ); */

}