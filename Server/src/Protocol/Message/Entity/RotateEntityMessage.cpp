#include "LKZ/Protocol/Message/Entity/RotateEntityMessage.h"
#include <LKZ/Core/ECS/Manager/ComponentManager.h>

RotateEntityMessage::RotateEntityMessage() {};

RotateEntityMessage::RotateEntityMessage(uint16_t entityId, float yawY)
{

    this->entityId = entityId;
    this->rotaY = yawY;
}

uint8_t RotateEntityMessage::getId() const
{
    return ID;
}

std::vector<uint8_t>& RotateEntityMessage::serialize(Serializer& serializer) const
{
    serializer.writeByte(ID);
    serializer.writeUInt16(entityId);
    serializer.writeFloat(rotaY);

    return serializer.getBuffer();
}

void RotateEntityMessage::deserialize(Deserializer& deserializer)
{
   /* entityId = deserializer.readUInt16();
    rotaY = deserializer.readFloat();*/
}


void RotateEntityMessage::process(const sockaddr_in& senderAddr, SOCKET tcpSocket)
{
 /*   auto* client = ClientManager::getClientByAddress(senderAddr);
    if (!client) return;

    Lobby* lobby = LobbyManager::getLobby(client->lobbyId);
    if (!lobby) return;

    Entity entity = entityId;
    auto& components = ComponentManager::Instance();

    if (components.rotations.find(entity) == components.rotations.end())
    {
        components.rotations[entity] = RotationComponent{ 0.0f, rotaY, 0.0f };
    }
    else
    {
        components.rotations[entity].rotation.y = rotaY;
    }

    Serializer serializer;
    serialize(serializer);

    Engine::Instance().Server()->SendToMultiple(
        lobby->clients,
        serializer.getBuffer(),
        getClassName(),
        ClientManager::getClientByAddress(senderAddr)
    );*/

}
