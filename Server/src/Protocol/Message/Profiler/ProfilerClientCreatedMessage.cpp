#include "LKZ/Protocol/Message/Profiler/ProfilerClientCreatedMessage.h"

ProfilerClientCreatedMessage::ProfilerClientCreatedMessage() {}

uint8_t ProfilerClientCreatedMessage::getId() const
{
    return ID;
}

std::vector<uint8_t>& ProfilerClientCreatedMessage::serialize(Serializer& serializer) const
{
	serializer.reset();

    serializer.writeByte(ID);

    return serializer.getBuffer();
}


void ProfilerClientCreatedMessage::deserialize(Deserializer& deserializer)
{
}

void ProfilerClientCreatedMessage::process(const sockaddr_in& senderAddr, SOCKET tcpSocket)
{
 
}
