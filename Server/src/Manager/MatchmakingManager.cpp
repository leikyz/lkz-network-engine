#include "LKZ/Core/Manager/MatchmakingManager.h"
#include "LKZ/Protocol/Message/Matchmaking/UpdateLobbyMessage.h"
#include "LKZ/Protocol/Message/Matchmaking/ChangeReadyStatusMessage.h"
#include "LKZ/Session/Client.h"
#include "LKZ/Core/Threading/ThreadManager.h"
#include <algorithm>
#include <iostream>

std::vector<Client> MatchmakingManager::waitingPlayers;

void MatchmakingManager::AddPlayerToQueue(Client& playerAddr)
{
    for (const auto& p : waitingPlayers) {
        if (p.address.sin_addr.s_addr == playerAddr.address.sin_addr.s_addr &&
            p.address.sin_port == playerAddr.address.sin_port)
            return;
    }

    waitingPlayers.push_back(playerAddr);

    auto matchmakingPool = ThreadManager::GetPool("matchmaking");
    if (matchmakingPool) {
        matchmakingPool->EnqueueTask([]() {
            MatchmakingManager::ProcessMatchmaking();
            });
    }
    else {
        std::cerr << "[MatchmakingManager] Matchmaking pool not found!\n";
    }
}


void MatchmakingManager::RemovePlayerFromQueue(const sockaddr_in& addr)
{
    waitingPlayers.erase(
        std::remove_if(waitingPlayers.begin(), waitingPlayers.end(),
            [&](const Client& c) {
                return c.address.sin_addr.s_addr == addr.sin_addr.s_addr &&
                    c.address.sin_port == addr.sin_port;
            }),
        waitingPlayers.end()
    );
}

void MatchmakingManager::ProcessMatchmaking()
{
    if (waitingPlayers.empty())
        return;

    std::vector<Client> playersToRemove;

    for (auto& p : waitingPlayers) {
        Client* client = ClientManager::getClientByAddress(p.address);
        if (!client) {
            playersToRemove.push_back(p);
            continue;
        }
        Lobby* lobby = LobbyManager::getAvailableLobby(p.matchmakingMapIdRequest);
        if (!lobby) {
            LobbyManager::createLobby(p.matchmakingMapIdRequest);
            int lastLobbyId = LobbyManager::getLastLobbyId();
            lobby = LobbyManager::getLobby(lastLobbyId);

        }

        if (!lobby) {
            std::cout << "[MATCHMAKING] Failed to get or create lobby for client" << std::endl;
            continue;
        }

        client->lobbyId = lobby->id;
        lobby->addClient(client->id);
        client->positionInLobby = static_cast<uint8_t>(lobby->clientIds.size());

		std::cout << "[MATCHMAKING] Client " << client->ipAddress << " added to lobby " << lobby->id << " at position " << static_cast<int>(client->positionInLobby) << std::endl;

        std::vector<uint8_t> allPositions;
        for (Client* c : LobbyManager::getClientsInLobby(lobby->id)) {
            if (!c) continue;
            allPositions.push_back(c->positionInLobby);
        }

        for (Client* c : LobbyManager::getClientsInLobby(lobby->id)) {
            if (!c) continue;

            if (c->isReady) {
                ChangeReadyStatusMessage changeReadyMsg;
                changeReadyMsg.isReady = false;
                changeReadyMsg.positionInLobby = c->positionInLobby;

                Serializer s;
                std::vector<uint8_t> buf = changeReadyMsg.serialize(s);
				Engine::Instance().Server()->Send(c->address, buf, changeReadyMsg.getClassName());

                c->isReady = false;
            }
        }

        for (Client* c : LobbyManager::getClientsInLobby(lobby->id)) {
            if (!c) continue;

            UpdateLobbyMessage updateLobbyMsg;
            updateLobbyMsg.updatedLobbyPos = c->positionInLobby;
            updateLobbyMsg.playersCount = static_cast<uint8_t>(allPositions.size());
            updateLobbyMsg.playersInLobby = allPositions;

            Serializer s;
            std::vector<uint8_t> buf = updateLobbyMsg.serialize(s);
			Engine::Instance().Server()->Send(c->address, buf, updateLobbyMsg.getClassName());
        }

        playersToRemove.push_back(p);
    }

    // Remove processed players
    for (const auto& p : playersToRemove) {
        waitingPlayers.erase(
            std::remove_if(waitingPlayers.begin(), waitingPlayers.end(),
                [&](const Client& c) {
                    return c.address.sin_addr.s_addr == p.address.sin_addr.s_addr &&
                        c.address.sin_port == p.address.sin_port;
                }),
            waitingPlayers.end()
        );
    }
}
