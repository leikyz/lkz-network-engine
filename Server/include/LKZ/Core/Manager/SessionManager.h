#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include <unordered_map>
#include <vector>
#include <atomic>
#include <mutex>
#include <memory>
#include <string>
#include <LKZ/Core/ECS/Entity.h>
#include <WinSock2.h>
#include <shared_mutex>
#include <span>

struct SessionClient 
{
    uint32_t id;
    sockaddr_in address; 
    bool isConnected = false;
};
struct Session
{
    uint32_t id;

    std::vector<uint32_t> authorizedClientIds;
    std::vector<uint32_t> connectedIds;
    std::vector<sockaddr_in> connectedAddresses;
    std::vector<SOCKET> connectedTCPSocket;

    Entity gameWaveEntity;
    int nextEntityId = 1;

    Session(uint32_t sessionId, std::span<const uint32_t> authIds)
		: id(sessionId), authorizedClientIds(authIds.begin(), authIds.end()), gameWaveEntity(0)
    {
    }
};

class SessionManager
{
public:
    static void Initialize();
    static void CreateSession(uint32_t id, std::span<const uint32_t> authorizedIds);
    static void AddClientToSession(uint32_t sessionId, sockaddr_in& clientAddress);

	static void RemoveSession(uint32_t sessionId);
    static void RemoveClientFromSession(uint32_t sessionId, uint32_t clientId);
	static bool JoinSession(uint32_t sessionId, uint32_t clientId, SOCKET tcpSocket);

    static Session* GetSession(uint32_t sessionId);
    static size_t GetSessionCount();

private:
    static std::vector<Session> m_sessions;
    static std::unordered_map<int, size_t> m_idToIndex;
    static std::shared_mutex m_sessionMutex;
};

#endif