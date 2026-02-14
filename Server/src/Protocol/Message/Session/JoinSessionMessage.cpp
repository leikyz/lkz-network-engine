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

    char ipStr[INET_ADDRSTRLEN];
    // Convertit l'IP binaire en string (ex: "192.168.1.50")
    inet_ntop(AF_INET, &(senderAddr.sin_addr), ipStr, INET_ADDRSTRLEN);
    // Convertit le port réseau en entier (ntohs = Network To Host Short)
    int port = ntohs(senderAddr.sin_port);

    std::cout << "[JoinSessionMessage] Processing request from: " << ipStr << ":" << port << std::endl;

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

    if(session->players.size() == session->authorizedClientIds.size())
    {
        std::cout << "[JoinSessionMessage] All clients joined session " << std::endl;

		for (const auto& player : session->players)
        {
			std::cout << "[JoinSessionMessage] Sending StartGameMessage to socket: " << player.tcpSocket << std::endl;
			StartGameMessage startGameMsg;
            Serializer s;
            startGameMsg.serialize(s);
            Engine::Instance().Server()->SendReliable(player.tcpSocket, s.getBuffer());
        }
	}
}