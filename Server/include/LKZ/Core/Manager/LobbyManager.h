#ifndef LOBBY_MANAGER_H
#define LOBBY_MANAGER_H

#include <unordered_map>
#include <vector>
#include <atomic>
#include <mutex>
#include <memory>
#include <string>
#include "LKZ/Session/Lobby.h"

struct Client;

class LobbyManager
{
public:
    static void createLobby(uint8_t mapId);
    static void addClientToLobby(int lobbyId, uint32_t clientId);
    static void removeClientFromLobby(int lobbyId, uint32_t clientId);
    static void removeClientFromAllLobbies(uint32_t clientId); 


    static Lobby* getLobby(int lobbyId);
    static void removeLobby(int lobbyId);

    static int getLastLobbyId() { return m_nextLobbyId.load() - 1; }
    static int getLobbyCount();
    static std::vector<Client*> getClientsInLobby(int lobbyId);
    static Client* getClientByEntityId(int entityId);
    static Lobby* getAvailableLobby(uint8_t mapId);
    static std::vector<Lobby*> getAllLobbies();
    static bool IsEveryoneReadyInLobby(int lobbyId);

private:
    static std::unordered_map<int, std::unique_ptr<Lobby>> m_lobbies;
    static std::atomic<int> m_nextLobbyId;
    static std::mutex m_lobbiesMutex;
};

#endif