#include "LKZ/Core/Manager/ClientManager.h"
#include "LKZ/Core/Manager/LobbyManager.h"
#include "LKZ/Utility/Constants.h"
#include <iostream>
#include <span>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <unordered_map>
#include <vector>

std::shared_mutex ClientManager::m_clientsMutex; // For read-write locking
std::unordered_map<uint32_t, size_t> ClientManager::m_idToIndex;
std::unordered_map<ClientKey, size_t, ClientKeyHash> ClientManager::m_addressToIndex;
std::atomic<uint32_t> ClientManager::m_nextId{ 1 };
std::vector<Client> ClientManager::m_clients;

void ClientManager::Initialize()
{
    std::unique_lock lock(m_clientsMutex);

    m_clients.clear();
    m_idToIndex.clear();
    m_addressToIndex.clear();

    m_clients.reserve(Constants::MAX_CLIENTS);
    m_nextId.store(1);
}

void ClientManager::addClient(const sockaddr_in& clientAddr)
{
    std::unique_lock lock(m_clientsMutex);

    if (m_clients.size() >= Constants::MAX_CLIENTS)
    {
        std::cout << "[ClientManager] Maximum client capacity reached. Cannot add more clients.\n";
        return;
    }

    uint32_t id = m_nextId.fetch_add(1);

    m_clients.emplace_back(id, clientAddr);
    size_t index = m_clients.size() - 1;

    m_idToIndex[id] = index;
    m_addressToIndex[ClientKey{ clientAddr.sin_addr.s_addr, clientAddr.sin_port }] = index;
}

void ClientManager::removeClientById(uint32_t clientId)
{
    std::unique_lock lock(m_clientsMutex);

    auto it = m_idToIndex.find(clientId);
    if (it == m_idToIndex.end()) return;

    size_t indexToRemove = it->second;
    size_t lastIndex = m_clients.size() - 1;

	// client key to remove from address map
    ClientKey keyToRemove{
        m_clients[indexToRemove].address.sin_addr.s_addr,
        m_clients[indexToRemove].address.sin_port
    };

    if (indexToRemove != lastIndex)
    {
		// swap with last client
        std::swap(m_clients[indexToRemove], m_clients[lastIndex]);

		// update moved client's index in maps
        uint32_t movedClientId = m_clients[indexToRemove].id;
        ClientKey movedKey{
            m_clients[indexToRemove].address.sin_addr.s_addr,
            m_clients[indexToRemove].address.sin_port
        };

        m_idToIndex[movedClientId] = indexToRemove;
        m_addressToIndex[movedKey] = indexToRemove;
    }

	// remove last client
    m_clients.pop_back();
    m_idToIndex.erase(clientId);
    m_addressToIndex.erase(keyToRemove);

    std::cout << "[ClientManager] Client removed. Remaining: " << m_clients.size() << "\n";
}

Client* ClientManager::getClientByAddress(const sockaddr_in& clientAddr)
{
	std::shared_lock lock(m_clientsMutex); // reading concurrente

    auto it = m_addressToIndex.find(ClientKey{ clientAddr.sin_addr.s_addr, clientAddr.sin_port });
    if (it != m_addressToIndex.end()) {
        return &m_clients[it->second];
    }
    return nullptr;
}

Client* ClientManager::getClientById(uint32_t clientId)
{
    std::shared_lock lock(m_clientsMutex); // reading concurrente

    auto it = m_idToIndex.find(clientId);
    if (it != m_idToIndex.end()) {
        return &m_clients[it->second];
    }
    return nullptr;
}

// Need to optimize to avoid copying the entire vector
std::vector<Client> ClientManager::getClients()
{
    std::shared_lock lock(m_clientsMutex); 
    return m_clients; 
}
