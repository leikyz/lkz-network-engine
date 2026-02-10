#include "LKZ/Protocol/Message/Session/CreateSessionMessage.h"
#include <iostream>

CreateSessionMessage::CreateSessionMessage()
    : token(0), clientsCount(0) 
{
}

std::vector<uint8_t>& CreateSessionMessage::serialize(Serializer& serializer) const
{
    return serializer.getBuffer();
}

void CreateSessionMessage::deserialize(Deserializer& deserializer)
{
    token = deserializer.readInt();
    clientsCount = deserializer.readByte();

    for (int i = 0; i < clientsCount; i++)
    {
        clientIds.push_back(deserializer.readInt());
    }
}

void CreateSessionMessage::process(const sockaddr_in& senderAddr)
{
    std::cout << "Client found: " << static_cast<int>(clientsCount) << std::endl;

}
