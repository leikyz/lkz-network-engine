#include "LKZ/Protocol/Message/Entity/Zombie/ZombieHitMessage.h"
#include <cstdlib> 
#include <ctime>   
#include <LKZ/Core/ECS/Manager/EntityManager.h>

ZombieHitMessage::ZombieHitMessage() {};

ZombieHitMessage::ZombieHitMessage(uint16_t zombieID, uint16_t from, float posX, float posY, float posZ)
	: zombieId(zombieID), from(from), posX(posX), posY(posY), posZ(posZ)
{
}

uint8_t ZombieHitMessage::getId() const
{
    return ID;
}

std::vector<uint8_t>& ZombieHitMessage::serialize(Serializer& serializer) const
{
    serializer.reset();

    serializer.writeByte(ID);
    serializer.writeUInt16(zombieId);
    serializer.writeUInt16(from);
    serializer.writeFloat(posX);
    serializer.writeFloat(posY);
    serializer.writeFloat(posZ);

    return serializer.getBuffer();
}

void ZombieHitMessage::deserialize(Deserializer& deserializer)
{
    zombieId = deserializer.readUInt16();
    from = deserializer.readUInt16();
	posX = deserializer.readFloat();
	posY = deserializer.readFloat();
	posZ = deserializer.readFloat();
}


void ZombieHitMessage::process(const sockaddr_in& senderAddr, SOCKET tcpSocket)
{
    Session* session = (tcpSocket != INVALID_SOCKET)
        ? SessionManager::GetSessionBySocket(tcpSocket)
		: SessionManager::GetSessionByAddress(senderAddr);

    if (!session) return;

    Serializer serializer;
    serialize(serializer);

    for (const auto& player : session->players)
    {
        Engine::Instance().Server()->Send(player.udpAddr, serializer.getBuffer(), getClassName());

    }

   /* Engine::Instance().Server()->SendToMultiple(session->clientsAddress, serializer.getBuffer(), getClassName(), &senderAddr);*/
}