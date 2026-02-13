#include "LKZ/Protocol/Message/Matchmaking/StartGameMessage.h"

StartGameMessage::StartGameMessage() {}

uint8_t StartGameMessage::getId() const
{
    return ID;
}

std::vector<uint8_t>& StartGameMessage::serialize(Serializer& serializer) const
{
    serializer.writeUInt16(3);
    serializer.writeByte(ID);

    return serializer.getBuffer();
}

void StartGameMessage::deserialize(Deserializer& deserializer)
{
}

void StartGameMessage::process(const sockaddr_in& senderAddr, SOCKET tcpSocket)
{
}
