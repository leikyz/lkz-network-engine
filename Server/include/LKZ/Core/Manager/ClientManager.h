#ifndef CLIENT_MANAGER_H
#define CLIENT_MANAGER_H

#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include "LKZ/Session/Client.h"

// Manager for connected clients



class ClientManager 
{
public:
    static void addClient(const sockaddr_in& clientAddr);
    static Client* getClientByAddress(const sockaddr_in& clientAddr);
    static Client* getClientById(const uint32_t clientId);
    static std::vector<Client*> getClients();
    static void removeClient(const sockaddr_in& clientAddr);

private:
	// Private constructor to prevent instantiation
    static std::unordered_map<std::string, std::unique_ptr<Client>> m_clients;

	// Mutex for thread-safe access to the clients map
    static std::mutex m_clientsMutex;

	// Next unique client ID
	static std::atomic<uint32_t> m_nextId; // Thread-safe with atomic

	// Generates a unique key for a client based on their address
    static std::string getClientKey(const sockaddr_in& clientAddr);
};

#endif // CLIENT_MANAGER_H
