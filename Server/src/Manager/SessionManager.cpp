#include "LKZ/Core/Manager/SessionManager.h"
#include <algorithm>
#include <LKZ/Utility/Constants.h>
#include <iostream>

// Static members
std::vector<std::unique_ptr<Session>> SessionManager::m_sessions;
std::unordered_map<uint32_t, Session*> SessionManager::m_idToSession;
std::unordered_map<SOCKET, Session*> SessionManager::m_socketToSession;
std::unordered_map<uint64_t, Session*> SessionManager::m_addrToSession;
std::shared_mutex SessionManager::m_sessionMutex;

uint64_t SessionManager::AddrToKey(const sockaddr_in& addr)
{
    return (static_cast<uint64_t>(addr.sin_addr.s_addr) << 32) | addr.sin_port;
}

void SessionManager::Initialize()
{
    std::unique_lock lock(m_sessionMutex);

    m_sessions.clear();
    m_idToSession.clear();
    m_socketToSession.clear();
    m_addrToSession.clear();

    m_sessions.reserve(Constants::MAX_SESSION);

    m_idToSession.reserve(Constants::MAX_SESSION);
    m_socketToSession.reserve(Constants::MAX_SESSION * Constants::MAX_PLAYERS_PER_SESSION);
    m_addrToSession.reserve(Constants::MAX_SESSION * Constants::MAX_PLAYERS_PER_SESSION);
}

void SessionManager::CreateSession(uint32_t id, std::span<const uint32_t> authorizedIds)
{
    std::unique_lock lock(m_sessionMutex);

    auto session = std::make_unique<Session>(id, authorizedIds);
    Session* sessionPtr = session.get();

    m_sessions.emplace_back(std::move(session));
    m_idToSession[id] = sessionPtr;

    std::cout << "[SessionManager] Created session " << id << std::endl;
}

bool SessionManager::JoinSession(uint32_t sessionId, uint32_t clientId, SOCKET tcpSocket)
{
    std::unique_lock lock(m_sessionMutex);

    auto it = m_idToSession.find(sessionId);
    if (it == m_idToSession.end())
        return false;

    Session* session = it->second;

    if (session->players.size() >= Constants::MAX_PLAYERS_PER_SESSION)
        return false;

    auto authIt = std::find(session->authorizedClientIds.begin(),
        session->authorizedClientIds.end(),
        clientId);

    if (authIt == session->authorizedClientIds.end())
        return false;

    SessionPlayer newPlayer;
    newPlayer.id = clientId;
    newPlayer.tcpSocket = tcpSocket;
    newPlayer.isUdpReady = false;
    memset(&newPlayer.udpAddr, 0, sizeof(sockaddr_in));

    session->players.push_back(newPlayer);

    m_socketToSession[tcpSocket] = session;

    return true;
}

void SessionManager::SetClientUDPAddress(uint32_t sessionId,
    uint32_t clientId,
    const sockaddr_in& udpAddr)
{
    std::unique_lock lock(m_sessionMutex);

    auto it = m_idToSession.find(sessionId);
    if (it == m_idToSession.end())
        return;

    Session* session = it->second;

    for (auto& player : session->players)
    {
        if (player.id == clientId)
        {
            player.udpAddr = udpAddr;
            player.isUdpReady = true;

            m_addrToSession[AddrToKey(udpAddr)] = session;

            char ipStr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &udpAddr.sin_addr, ipStr, INET_ADDRSTRLEN);

            std::cout << "[SessionManager] UDP Linked: Client "
                << clientId << " -> "
                << ipStr << ":"
                << ntohs(udpAddr.sin_port)
                << std::endl;
            return;
        }
    }
}

void SessionManager::RemoveSession(uint32_t sessionId)
{
    std::unique_lock lock(m_sessionMutex);

    auto it = m_idToSession.find(sessionId);
    if (it == m_idToSession.end())
        return;

    Session* session = it->second;

    // Cleanup lookups
    for (const auto& player : session->players)
    {
        m_socketToSession.erase(player.tcpSocket);
        if (player.isUdpReady)
            m_addrToSession.erase(AddrToKey(player.udpAddr));
    }

    m_idToSession.erase(sessionId);

    // Remove from vector (swap & pop)
    for (size_t i = 0; i < m_sessions.size(); ++i)
    {
        if (m_sessions[i].get() == session)
        {
            m_sessions[i] = std::move(m_sessions.back());
            m_sessions.pop_back();
            break;
        }
    }
}

void SessionManager::RemoveClientFromSession(uint32_t sessionId, uint32_t clientId)
{
    std::unique_lock lock(m_sessionMutex);

    auto it = m_idToSession.find(sessionId);
    if (it == m_idToSession.end())
        return;

    Session* session = it->second;

    for (auto itPlayer = session->players.begin();
        itPlayer != session->players.end();
        ++itPlayer)
    {
        if (itPlayer->id == clientId)
        {
            m_socketToSession.erase(itPlayer->tcpSocket);

            if (itPlayer->isUdpReady)
                m_addrToSession.erase(AddrToKey(itPlayer->udpAddr));

            session->players.erase(itPlayer);
            break;
        }
    }
}

Session* SessionManager::GetSessionBySocket(SOCKET socket)
{
    if (socket == INVALID_SOCKET)
        return nullptr;

    std::shared_lock lock(m_sessionMutex);

    auto it = m_socketToSession.find(socket);
    return (it != m_socketToSession.end()) ? it->second : nullptr;
}

Session* SessionManager::GetSessionByAddress(const sockaddr_in& addr)
{
    std::shared_lock lock(m_sessionMutex);

    auto it = m_addrToSession.find(AddrToKey(addr));
    return (it != m_addrToSession.end()) ? it->second : nullptr;
}

Session* SessionManager::GetSession(uint32_t sessionId)
{
    std::shared_lock lock(m_sessionMutex);

    auto it = m_idToSession.find(sessionId);
    return (it != m_idToSession.end()) ? it->second : nullptr;
}

size_t SessionManager::GetSessionCount()
{
    std::shared_lock lock(m_sessionMutex);
    return m_sessions.size();
}
