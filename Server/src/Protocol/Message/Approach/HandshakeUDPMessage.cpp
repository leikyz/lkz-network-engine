#include "LKZ/Protocol/Message/Approach/HandshakeUDPMessage.h"
#include <Common/ProfilerProtocol.h>
#include <LKZ/Protocol/Message/Profiler/ProfilerClientCreatedMessage.h>
#include <iostream>

HandshakeUDPMessage::HandshakeUDPMessage() {}

uint8_t HandshakeUDPMessage::getId() const
{
    return ID;
}

std::vector<uint8_t>& HandshakeUDPMessage::serialize(Serializer& serializer) const
{
    serializer.writeByte(ID);
    return serializer.getBuffer();
}

void HandshakeUDPMessage::deserialize(Deserializer& deserializer)
{
	sessionToken = deserializer.readUInt32();
	clientToken = deserializer.readUInt32();
}

void HandshakeUDPMessage::process(const sockaddr_in& senderAddr, SOCKET tcpSocket)
{
    Session* session = SessionManager::GetSession(sessionToken);

    if (!session)
    {
        std::cerr << "[HandshakeUDP] Erreur : Session introuvable pour le token " << sessionToken << std::endl;
        return; 
    }

    std::cout << "[HandshakeUDP] Linking UDP address for client " << clientToken << std::endl;

    SessionManager::SetClientUDPAddress(session->id, clientToken, senderAddr);

    Serializer serializer;
    serialize(serializer);

    Engine::Instance().Server()->Send(senderAddr, serializer.getBuffer(), getClassName());
}