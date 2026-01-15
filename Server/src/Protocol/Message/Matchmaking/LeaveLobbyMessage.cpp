#include "LKZ/Protocol/Message/Matchmaking/LeaveLobbyMessage.h"
#include "LKZ/Protocol/Message/Matchmaking/UpdateLobbyMessage.h"
#include "LKZ/Protocol/Message/Matchmaking/ChangeReadyStatusMessage.h"

LeaveLobbyMessage::LeaveLobbyMessage() {}

uint8_t LeaveLobbyMessage::getId() const
{
    return ID;
}

std::vector<uint8_t>& LeaveLobbyMessage::serialize(Serializer& serializer) const
{
    serializer.writeByte(ID);
	serializer.writeByte(positionInLobby);
    return serializer.getBuffer();
}


void LeaveLobbyMessage::deserialize(Deserializer& deserializer)
{
}

void LeaveLobbyMessage::process(const sockaddr_in& senderAddr)
{
    Client* currentClient = ClientManager::getClientByAddress(senderAddr);
    Lobby* lobby = LobbyManager::getLobby(currentClient->lobbyId);
    uint8_t removedPosition = currentClient->positionInLobby;
    if (lobby)
    {
        for (Client* c : LobbyManager::getClientsInLobby(lobby->id))
        {
            if (!c) continue;

            if (c->isReady)
            {
                c->isReady = false;
                ChangeReadyStatusMessage changeReadyMsg;
                changeReadyMsg.isReady = c->isReady;
                changeReadyMsg.positionInLobby = c->positionInLobby;
                Serializer s;
                std::vector<uint8_t> buf = changeReadyMsg.serialize(s);
                Engine::Instance().Server()->SendToMultiple(LobbyManager::getClientsInLobby(lobby->id), buf, getClassName());
              
            }
        }

        lobby->removeClient(currentClient->id);

        if (lobby->clientIds.empty())
        {
            LobbyManager::removeLobby(lobby->id);
        }
        else
        {
            std::vector<uint8_t> allPositions;

            for (Client* c : LobbyManager::getClientsInLobby(lobby->id))
            {
                if (!c) continue;
                if (c->positionInLobby > removedPosition)
                {
                    c->positionInLobby--; 
                }
                allPositions.push_back(c->positionInLobby);
              
            }        

         

            for (Client* c : LobbyManager::getClientsInLobby(lobby->id))
            {
                if (!c) continue;


                UpdateLobbyMessage updateLobbyMsg;
                updateLobbyMsg.updatedLobbyPos = c->positionInLobby;
                updateLobbyMsg.playersCount = static_cast<byte>(allPositions.size());
                updateLobbyMsg.playersInLobby = allPositions;

                Serializer s;
                std::vector<uint8_t> buf = updateLobbyMsg.serialize(s);
				Engine::Instance().Server()->Send(c->address, buf, getClassName());
            }
        }

        currentClient->lobbyId = -1;
        currentClient->positionInLobby = 0;
        currentClient->matchmakingMapIdRequest = 0;   
    }
}
