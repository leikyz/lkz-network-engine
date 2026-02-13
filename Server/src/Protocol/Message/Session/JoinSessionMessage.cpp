#include "LKZ/Protocol/Message/Session/JoinSessionMessage.h"
#include <iostream>
#include <LKZ/Protocol/Message/Matchmaking/StartGameMessage.h>

JoinSessionMessage::JoinSessionMessage()
    : sessionToken(0), clientToken(0)
{
}

std::vector<uint8_t>& JoinSessionMessage::serialize(Serializer& serializer) const
{
    serializer.writeUInt16(3);
	serializer.writeByte(ID);
  
    return serializer.getBuffer();
}

void JoinSessionMessage::deserialize(Deserializer& deserializer)
{
    sessionToken = deserializer.readUInt32(); 
	clientToken = deserializer.readUInt32();  
}

void JoinSessionMessage::process(const sockaddr_in& senderAddr, SOCKET tcpSocket)
{
	Session* session = SessionManager::GetSession(sessionToken);

	if (!session) {
        std::cerr << "[JoinSessionMessage] Session not found for token: " << sessionToken << std::endl;
        return;
    }

    if (!SessionManager::JoinSession(sessionToken, clientToken, tcpSocket)) {
        std::cerr << "[JoinSessionMessage] Failed to join session with token: " << sessionToken << " for client token: " << clientToken << std::endl;
        return;
    }
    else
    {
        std::cout << "[JoinSessionMessage] Client with token: " << clientToken << std::endl;

		Serializer s;
        serialize(s);

		Engine::Instance().Server()->SendReliable(tcpSocket, s.getBuffer());

    }

    if(session->connectedIds.size() == session->authorizedClientIds.size())
    {
        std::cout << "[JoinSessionMessage] All clients joined session " << std::endl;

		for (const auto& socket : session->connectedTCPSocket)
        {
			std::cout << "[JoinSessionMessage] Sending StartGameMessage to socket: " << socket << std::endl;
			StartGameMessage startGameMsg;
            Serializer s;
            startGameMsg.serialize(s);
            Engine::Instance().Server()->SendReliable(socket, s.getBuffer());
        }
	}
}