#include "LKZ/Protocol/Message/Approach/CreateClientMessage.h"
#include <Common/ProfilerProtocol.h>
#include <LKZ/Protocol/Message/Profiler/ProfilerClientCreatedMessage.h>
#include <iostream>

CreateClientMessage::CreateClientMessage() {}

uint8_t CreateClientMessage::getId() const
{
    return ID;
}

std::vector<uint8_t>& CreateClientMessage::serialize(Serializer& serializer) const
{
    serializer.writeByte(ID);
    return serializer.getBuffer();
}

void CreateClientMessage::deserialize(Deserializer& deserializer)
{
   
}

void CreateClientMessage::process(const sockaddr_in& senderAddr, SOCKET tcpSocket)
{
   /* ClientManager::addClient(senderAddr);

    std::cout << "[CreateClientMessage] New client connected: ";*/

	//Client* client = ClientManager::getClientByAddress(senderAddr);

 //   Serializer serializer;
 //   serialize(serializer);

 //   Engine::Instance().Server()->Send(senderAddr, serializer.getBuffer(), getClassName());

 //   ProfilerClientCreatedMessage profilerMsg;
	//profilerMsg.serialize(serializer);

	//Engine::Instance().GetProfiler()->Broadcast(serializer.getBuffer());
}
