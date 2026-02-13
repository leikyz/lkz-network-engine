#include "LKZ/Protocol/Message/Gameplay/ChangeWaveMessage.h"

ChangeWaveMessage::ChangeWaveMessage() {}

ChangeWaveMessage::ChangeWaveMessage(uint16_t wave)
    : wave(wave)
{
}

uint8_t ChangeWaveMessage::getId() const
{
    return ID;
}

std::vector<uint8_t>& ChangeWaveMessage::serialize(Serializer& serializer) const
{
	serializer.reset();

    serializer.writeByte(ID);
    serializer.writeUInt16(wave);

    return serializer.getBuffer();
}

void ChangeWaveMessage::deserialize(Deserializer& deserializer)
{
}

void ChangeWaveMessage::process(const sockaddr_in& senderAddr, SOCKET tcpSocket)
{
   
}


