#include "LKZ/Protocol/Message/Matchmaking/UpdateLobbyMessage.h"

UpdateLobbyMessage::UpdateLobbyMessage()  
{
}

std::vector<uint8_t>& UpdateLobbyMessage::serialize(Serializer& serializer) const
{
    serializer.writeByte(ID);
    serializer.writeByte(updatedLobbyPos);
    serializer.writeByte(playersCount);

    for (uint8_t index : playersInLobby)
    {
        serializer.writeByte(index);
    }

    return serializer.getBuffer();
}

void UpdateLobbyMessage::deserialize(Deserializer& deserializer)
{
}

void UpdateLobbyMessage::process(const sockaddr_in& senderAddr, SOCKET tcpSocket)
{
}
