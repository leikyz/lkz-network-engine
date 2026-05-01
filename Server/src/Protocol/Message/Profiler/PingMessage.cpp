#include "LKZ/Protocol/Message/Profiler/PingMessage.h" // Update this path
#include <iostream>
// #include "LKZ/Core/Engine.h" // Uncomment to access your server instance

PingMessage::PingMessage() : clientTimestamp(0) {}

PingMessage::PingMessage(uint64_t timestamp) : clientTimestamp(timestamp) {}

uint8_t PingMessage::getId() const
{
    return ID;
}

std::vector<uint8_t>& PingMessage::serialize(Serializer& serializer) const
{
    serializer.writeUInt16(11); // size
    serializer.writeByte(ID);
    serializer.writeUInt64(clientTimestamp);
    return serializer.getBuffer();
}

void PingMessage::deserialize(Deserializer& deserializer)
{
    clientTimestamp = deserializer.readUInt64();
}

void PingMessage::process(const sockaddr_in& senderAddr, SOCKET tcpSocket)
{
    
    std::cout << "[Network] Ping received. Bouncing back timestamp: " << clientTimestamp << "\n";

    Serializer responseSerializer;

    this->serialize(responseSerializer);

    Engine::Instance().Server()->SendReliable(tcpSocket, responseSerializer.getBuffer());
    
}