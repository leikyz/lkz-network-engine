#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include <unordered_map>
#include <vector>
#include <shared_mutex>
#include <memory>
#include <LKZ/Core/ECS/Entity.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <span>

struct SessionPlayer
{
    uint32_t id;
    SOCKET tcpSocket;
    sockaddr_in udpAddr;
    int entityId;
    bool isUdpReady = false;
};

struct Session
{
    uint32_t id;
    std::vector<uint32_t> authorizedClientIds;
    std::vector<SessionPlayer> players;

    Entity gameWaveEntity;
    int nextEntityId = 1;

    Session(uint32_t sessionId, std::span<const uint32_t> authIds)
        : id(sessionId),
        authorizedClientIds(authIds.begin(), authIds.end()),
        gameWaveEntity(0)
    {
        players.reserve(4);
    }

    SessionPlayer* GetPlayer(uint32_t clientId)
    {
        for (auto& p : players)
            if (p.id == clientId)
                return &p;
        return nullptr;
    }
};

class SessionManager
{
public:
    static void Initialize();
    static void CreateSession(uint32_t id, std::span<const uint32_t> authorizedIds);
    static void RemoveSession(uint32_t sessionId);

    static bool JoinSession(uint32_t sessionId, uint32_t clientId, SOCKET tcpSocket);
    static void SetClientUDPAddress(uint32_t sessionId, uint32_t clientId, const sockaddr_in& udpAddr);

    static Session* GetSessionBySocket(SOCKET socket);
    static Session* GetSessionByAddress(const sockaddr_in& addr);
    static Session* GetSession(uint32_t sessionId);

    static void RemoveClientFromSession(uint32_t sessionId, uint32_t clientId);
    static size_t GetSessionCount();

private:
    static std::vector<std::unique_ptr<Session>> m_sessions;

    static std::unordered_map<uint32_t, Session*> m_idToSession;
    static std::unordered_map<SOCKET, Session*> m_socketToSession;
    static std::unordered_map<uint64_t, Session*> m_addrToSession;

    static std::shared_mutex m_sessionMutex;

    static uint64_t AddrToKey(const sockaddr_in& addr);
};

#endif
