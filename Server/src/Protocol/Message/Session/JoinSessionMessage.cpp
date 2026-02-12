#include "LKZ/Protocol/Message/Session/JoinSessionMessage.h"
#include <iostream>

JoinSessionMessage::JoinSessionMessage()
    : sessionToken(0), clientToken(0)
{
}

std::vector<uint8_t>& JoinSessionMessage::serialize(Serializer& serializer) const
{
    return serializer.getBuffer();
}

void JoinSessionMessage::deserialize(Deserializer& deserializer)
{
    sessionToken = deserializer.readUInt32(); 
	clientToken = deserializer.readUInt32();  
}

void JoinSessionMessage::process(const sockaddr_in& senderAddr)
{
	Session* session = SessionManager::GetSession(sessionToken);

	if (!session) {
        std::cerr << "[JoinSessionMessage] Session not found for token: " << sessionToken << std::endl;
        return;
    }

    if (!SessionManager::JoinSession(sessionToken, clientToken)) {
        std::cerr << "[JoinSessionMessage] Failed to join session with token: " << sessionToken << " for client token: " << clientToken << std::endl;
        return;
    }
    else
    {
        std::cout << "[JoinSessionMessage] Client with token: " << clientToken << std::endl;
    }
}