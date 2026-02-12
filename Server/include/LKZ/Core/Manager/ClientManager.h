//#ifndef CLIENT_MANAGER_H
//#define CLIENT_MANAGER_H
//
//#include <unordered_map>
//#include <memory>
//#include <string>
//#include <vector>
//#include <mutex>
//#include <winsock2.h>
//#include <ws2tcpip.h>
//#include <stdio.h>
//#include "LKZ/Session/Client.h"
//#include <shared_mutex>
//#include <span>
//
//// Manager for connected clients
//
//struct Client 
//{
//    uint32_t id;
//    int lobbyId;
//    sockaddr_in address;
//
//    Client(uint32_t _id, const sockaddr_in& _address, int _lobbyId = -1)
//        : id(_id), address(_address), lobbyId(_lobbyId)
//    {
//    }
//};
//
//struct ClientKey 
//{
//    uint32_t ip;   
//    uint16_t port; 
//
//    bool operator==(const ClientKey& other) const {
//        return ip == other.ip && port == other.port;
//    }
//};
//
//struct ClientKeyHash 
//{
//    size_t operator()(const ClientKey& k) const {
//        return std::hash<uint64_t>()((uint64_t(k.ip) << 16) | k.port);
//    }
//};
//
//class ClientManager 
//{
//public:
//	static void Initialize();
//    static void addClient(const sockaddr_in& clientAddr);
//    static Client* getClientByAddress(const sockaddr_in& clientAddr);
//    static Client* getClientById(const uint32_t clientId);
//    std::vector<Client> getClients();
//    static void removeClientById(uint32_t clientId);
//
//private:
//	// Private constructor to prevent instantiation
//    static std::vector<Client> m_clients;
//
//	// Map from client ID to index in the clients vector for quick lookup
//    static std::unordered_map<uint32_t, size_t> m_idToIndex;
//    
//	// Map from client address (IP + port) to index in the clients vector for quick lookup
//    static std::unordered_map<ClientKey, size_t, ClientKeyHash> m_addressToIndex;
//
//	// Mutex for thread-safe access to the clients map
//    static std::shared_mutex m_clientsMutex;
//	// Next unique client ID
//	static std::atomic<uint32_t> m_nextId; // Thread-safe with atomic
//
//	// Generates a unique key for a client based on their address
//    static std::string getClientKey(const sockaddr_in& clientAddr);
//};
//
//#endif // CLIENT_MANAGER_H
