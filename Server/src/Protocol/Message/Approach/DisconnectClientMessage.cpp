#include "LKZ/Protocol/Message/Approach/DisconnectClientMessage.h"
#include "LKZ/Protocol/Message/Matchmaking/LeaveLobbyMessage.h"
#include "LKZ/Core/Manager/MatchmakingManager.h"


DisconnectClientMessage::DisconnectClientMessage() {}

uint8_t DisconnectClientMessage::getId() const
{
    return ID;
}

std::vector<uint8_t>& DisconnectClientMessage::serialize(Serializer& serializer) const
{
    serializer.writeByte(ID);
    return serializer.getBuffer();
}

void DisconnectClientMessage::deserialize(Deserializer& deserializer)
{

}

void DisconnectClientMessage::process(const sockaddr_in& senderAddr)
{
    MatchmakingManager::RemovePlayerFromQueue(senderAddr);

    Client* currentClient = ClientManager::getClientByAddress(senderAddr);
    if (!currentClient)
    {
        Logger::Log("Disconnect: Client not found", LogType::Warning);
        return;
    }

    uint32_t clientId = currentClient->id;
    int lobbyId = currentClient->lobbyId;
    uint8_t removedPosition = currentClient->positionInLobby;

    if (lobbyId != -1)
    {
        Lobby* lobby = LobbyManager::getLobby(lobbyId);
        if (lobby)
        {
            lobby->removeClient(clientId);

            if (lobby->getClientCount() == 0)
            {
                LobbyManager::removeLobby(lobbyId);
            }
            else
            {
                std::vector<Client*> remainingClients = LobbyManager::getClientsInLobby(lobbyId);

                for (Client* c : remainingClients)
                {
                    if (c->positionInLobby > removedPosition)
                    {
                        c->positionInLobby--;
                    }
                }

                LeaveLobbyMessage leaveLobbyMsg;
                leaveLobbyMsg.positionInLobby = removedPosition;

                Serializer serializer;
                std::vector<uint8_t> buffer = leaveLobbyMsg.serialize(serializer);

                for (Client* c : remainingClients)
                {
                    Engine::Instance().Server()->Send(c->address, buffer, "LeaveLobbyMessage");
                }
            }
        }
    }
    ClientManager::removeClient(senderAddr);
}



