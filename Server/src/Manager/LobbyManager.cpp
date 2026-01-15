#include "LKZ/Core/Manager/LobbyManager.h"
#include "LKZ/Core/Manager/ClientManager.h"
#include "LKZ/Session/Client.h"
#include <algorithm>

// Static member definitions
std::unordered_map<int, std::unique_ptr<Lobby>> LobbyManager::m_lobbies;
std::atomic<int> LobbyManager::m_nextLobbyId{ 1 };
std::mutex LobbyManager::m_lobbiesMutex;

void LobbyManager::createLobby(uint8_t mapId)
{
    int lobbyId = m_nextLobbyId.fetch_add(1);
    auto lobby = std::make_unique<Lobby>(lobbyId, mapId);

    std::lock_guard<std::mutex> lock(m_lobbiesMutex);
    m_lobbies[lobbyId] = std::move(lobby);
}

void LobbyManager::addClientToLobby(int lobbyId, uint32_t clientId)
{
    std::lock_guard<std::mutex> lock(m_lobbiesMutex);
    auto it = m_lobbies.find(lobbyId);
    if (it != m_lobbies.end())
    {
        it->second->addClient(clientId);
    }
}

void LobbyManager::removeClientFromLobby(int lobbyId, uint32_t clientId)
{
    std::lock_guard<std::mutex> lock(m_lobbiesMutex);
    auto it = m_lobbies.find(lobbyId);
    if (it != m_lobbies.end())
    {
        it->second->removeClient(clientId);
    }
}

void LobbyManager::removeClientFromAllLobbies(uint32_t clientId)
{
    std::lock_guard<std::mutex> lock(m_lobbiesMutex);
    for (auto& pair : m_lobbies)
    {
        pair.second->removeClient(clientId);
    }
}

std::vector<Client*> LobbyManager::getClientsInLobby(int lobbyId)
{
    std::vector<Client*> clientsInLobby;

    std::lock_guard<std::mutex> lock(m_lobbiesMutex); 

    auto it = m_lobbies.find(lobbyId);
    if (it != m_lobbies.end())
    {
        clientsInLobby.reserve(it->second->clientIds.size());

        for (uint32_t id : it->second->clientIds)
        {
            Client* c = ClientManager::getClientById(id);
            if (c)
            {
                clientsInLobby.push_back(c);
            }
        }
    }

    return clientsInLobby;
}

void LobbyManager::removeLobby(int lobbyId)
{
	// Extract the lobby to delete it safely outside the lock
    std::unique_ptr<Lobby> lobbyToDelete = nullptr;
    {
        std::lock_guard<std::mutex> lock(m_lobbiesMutex);
        auto node = m_lobbies.extract(lobbyId);
        if (!node.empty())
            lobbyToDelete = std::move(node.mapped());
    }
}

bool LobbyManager::IsEveryoneReadyInLobby(int lobbyId)
{
    std::lock_guard<std::mutex> lock(m_lobbiesMutex);
    auto it = m_lobbies.find(lobbyId);
    if (it == m_lobbies.end()) return false;

    Lobby* lobby = it->second.get();

    for (uint32_t cId : lobby->clientIds) 
    {
        auto c = ClientManager::getClientById(cId);
        if (!c || !c->isReady)
            return false;
    }
    return true;
}

Lobby* LobbyManager::getLobby(int lobbyId)
{
	// Get lobby with thread-safe access
    std::lock_guard<std::mutex> lock(m_lobbiesMutex); 
    auto it = m_lobbies.find(lobbyId);
    return (it != m_lobbies.end()) ? it->second.get() : nullptr;
}

int LobbyManager::getLobbyCount()
{
    std::lock_guard<std::mutex> lock(m_lobbiesMutex);
    return static_cast<int>(m_lobbies.size());
}

Lobby* LobbyManager::getAvailableLobby(uint8_t mapId)
{
    std::lock_guard<std::mutex> lock(m_lobbiesMutex);
    for (auto& pair : m_lobbies)
    {
        Lobby* l = pair.second.get();
        if (l && l->mapId == mapId && l->clientIds.size() < Lobby::MAX_PLAYER && !l->inGame)
        {
            return l;
        }
    }
    return nullptr;
}

std::vector<Lobby*> LobbyManager::getAllLobbies()
{
    std::lock_guard<std::mutex> lock(m_lobbiesMutex);
    std::vector<Lobby*> result;
    result.reserve(m_lobbies.size());
    for (auto& pair : m_lobbies)
    {
        result.push_back(pair.second.get());
    }
    return result;
}

Client* LobbyManager::getClientByEntityId(int entityId)
{
    std::lock_guard<std::mutex> lock(m_lobbiesMutex);
    for (auto& pair : m_lobbies)
    {
        for (uint32_t cId : pair.second->clientIds)
        {
            Client* c = ClientManager::getClientById(cId);
            if (c && c->playerEntityId == entityId)
            {
                return c;
            }
        }
    }
    return nullptr;
}