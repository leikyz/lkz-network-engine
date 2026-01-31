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

struct Session
{
    uint32_t id;
    std::vector<uint32_t> clientIds;
	std::vector<sockaddr_in> clientsAddress; // For broadcasting purposes
    Entity gameWaveEntity;
    int nextEntityId = 1;
};

class SessionManager
{
public:
    static void Initialize();
    static void CreateSession();
    static void AddClientToSession(uint32_t sessionId, sockaddr_in& clientAddress);

	static void RemoveSession(uint32_t sessionId);
    static void RemoveClientFromSession(uint32_t sessionId, uint32_t clientId);


    static Session* GetSession(uint32_t sessionId);
    static int GetLastSessionId() { return m_nextSessionId.load() - 1; }
    static size_t GetSessionCount();

private:
    static std::vector<Session> m_sessions;
    static std::unordered_map<int, size_t> m_idToIndex;

    static std::atomic<int> m_nextSessionId;
    static std::shared_mutex m_sessionMutex;
};

#endif