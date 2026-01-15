#include "LKZ/Core/Manager/ClientManager.h"
#include "LKZ/Core/Manager/LobbyManager.h"

std::unordered_map<std::string, std::unique_ptr<Client>> ClientManager::m_clients;
std::mutex ClientManager::m_clientsMutex;
std::atomic<uint32_t> ClientManager::m_nextId{ 1 }; // For avoiding multithreading issues


void ClientManager::addClient(const sockaddr_in& clientAddr)
{
    std::string key = getClientKey(clientAddr);
    uint32_t id = m_nextId.fetch_add(1);

    auto client = std::make_unique<Client>(id, clientAddr, key);

	// Thread-safe insertion into the clients map
    std::lock_guard<std::mutex> lock(m_clientsMutex);
    m_clients.try_emplace(key, std::move(client));
}

void ClientManager::removeClient(const sockaddr_in& clientAddr)
{
    std::string key = getClientKey(clientAddr);
    std::unique_ptr<Client> clientToDelete;

	// Lock the mutex only for the duration of the map modification
    {
        std::lock_guard<std::mutex> lock(m_clientsMutex);

       /* LobbyManager::getLobby*/
		// Extract the node to remove it safely
        auto node = m_clients.extract(key);

        if (!node.empty())
            clientToDelete = std::move(node.mapped());
    }
}

Client* ClientManager::getClientByAddress(const sockaddr_in& clientAddr)
{
    std::lock_guard<std::mutex> lock(m_clientsMutex);

    std::string key = getClientKey(clientAddr);
    auto it = m_clients.find(key);

    if (it != m_clients.end())
    {
        return it->second.get();
    }

    return nullptr;
}


// Need to rework this to be more efficient

Client* ClientManager::getClientById(const uint32_t clientId)
{
    std::lock_guard<std::mutex> lock(m_clientsMutex);

    for (auto& pair : m_clients)
    {
        if (pair.second->id == clientId)
        {
            return pair.second.get(); 
        }
    }

    return nullptr; 
}


std::vector<Client*> ClientManager::getClients()
{
    std::lock_guard<std::mutex> lock(m_clientsMutex);

    std::vector<Client*> clientList;

    clientList.reserve(m_clients.size());

    for (auto& pair : m_clients)
    {
        clientList.push_back(pair.second.get());
    }

    return clientList;
}

std::string ClientManager::getClientKey(const sockaddr_in& clientAddr)
{
	// Convert IP address to string
    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(clientAddr.sin_addr), ipStr, INET_ADDRSTRLEN);
    return std::string(ipStr) + ":" + std::to_string(ntohs(clientAddr.sin_port));
}

