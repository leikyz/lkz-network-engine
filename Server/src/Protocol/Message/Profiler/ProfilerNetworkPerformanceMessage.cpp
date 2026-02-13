#include "LKZ/Protocol/Message/Profiler/ProfilerNetworkPerformanceMessage.h"

ProfilerNetworkPerformanceMessage::ProfilerNetworkPerformanceMessage() {}

uint8_t ProfilerNetworkPerformanceMessage::getId() const
{
    return ID;
}

std::vector<uint8_t>& ProfilerNetworkPerformanceMessage::serialize(Serializer& serializer) const
{
    serializer.reset();

    serializer.writeByte(ID);
	serializer.writeFloat(deltaTime);
	serializer.writeFloat(fps);

    return serializer.getBuffer();
}


void ProfilerNetworkPerformanceMessage::deserialize(Deserializer& deserializer)
{
}

void ProfilerNetworkPerformanceMessage::process(const sockaddr_in& senderAddr, SOCKET tcpSocket)
{

}
