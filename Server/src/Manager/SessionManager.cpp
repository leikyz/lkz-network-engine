#include "LKZ/Core/Manager/SessionManager.h"
#include <algorithm>
#include <LKZ/Utility/Constants.h>
#include <iostream>

// Static member definitions
std::unordered_map<int, size_t> SessionManager::m_idToIndex;
std::shared_mutex SessionManager::m_sessionMutex;
std::vector<Session> SessionManager::m_sessions;

void SessionManager::Initialize()
{
    std::unique_lock lock(m_sessionMutex);

    m_idToIndex.clear();;
	m_sessions.clear();

	m_sessions.reserve(Constants::MAX_SESSION); 
}

void SessionManager::CreateSession(uint32_t id, std::span<const uint32_t> authorizedIds)
{
    std::unique_lock lock(m_sessionMutex);
    int sessionId = id;
	m_sessions.emplace_back(sessionId);

    std::cout << "[SessionManager] Created session with ID: " << sessionId << std::endl;
}

void SessionManager::AddClientToSession(uint32_t sessionId, sockaddr_in& clientAddress)
{
    std::unique_lock lock(m_sessionMutex);

    auto it = m_idToIndex.find(sessionId);
    if (it != m_idToIndex.end())
    {
		if (m_sessions[it->second].connectedAddresses.size() >= Constants::MAX_PLAYERS_PER_SESSION || m_sessions.capacity() >= Constants::MAX_SESSION)
        {
            return; // Session is full
        }

		m_sessions[it->second].connectedAddresses.emplace_back(clientAddress);
    }
}

void SessionManager::RemoveSession(uint32_t sessionId)
{
    std::unique_lock lock(m_sessionMutex);

    auto it = m_idToIndex.find(sessionId);
    if (it == m_idToIndex.end()) return;

    size_t indexToRemove = it->second;
    size_t lastIndex = m_sessions.size() - 1;

    if (indexToRemove != lastIndex)
    {
        std::swap(m_sessions[indexToRemove], m_sessions[lastIndex]);
        m_idToIndex[m_sessions[indexToRemove].id] = indexToRemove; // put the swapped session's id to the correct index
    }
    m_sessions.pop_back();
	m_idToIndex.erase(sessionId);
}


void SessionManager::RemoveClientFromSession(uint32_t sessionId, uint32_t clientId)
{
    std::unique_lock lock(m_sessionMutex);

    auto it = m_idToIndex.find(sessionId);
    if (it != m_idToIndex.end())
    {
        auto& session = m_sessions[it->second];

        auto itId = std::find(session.connectedIds.begin(), session.connectedIds.end(), clientId);

        if (itId != session.connectedIds.end())
        {
            size_t index = std::distance(session.connectedIds.begin(), itId);

            session.connectedIds.erase(itId);

            if (index < session.connectedAddresses.size()) {
                session.connectedAddresses.erase(session.connectedAddresses.begin() + index);
            }
        }
    }
}

bool SessionManager::JoinSession(uint32_t sessionId, uint32_t clientId)
{
    std::shared_lock lock(m_sessionMutex);

    auto it = m_idToIndex.find(sessionId);

    if (it == m_idToIndex.end())
    {
        return false;
    }

    auto& session = m_sessions[it->second];

    if (session.connectedIds.size() >= Constants::MAX_PLAYERS_PER_SESSION)
    {
        return false;
    }

    auto authIt = std::find(session.authorizedClientIds.begin(), session.authorizedClientIds.end(), clientId);
    if (authIt != session.authorizedClientIds.end())
    {
        session.connectedIds.push_back(clientId);
        return true;
    }

    return false; 
}

Session* SessionManager::GetSession(uint32_t sessionId)
{
	std::shared_lock lock(m_sessionMutex); // reading concurrente

    auto it = m_idToIndex.find(sessionId);
    if (it != m_idToIndex.end())
    {
        return &m_sessions[it->second];
    }
    return nullptr;
}

size_t SessionManager::GetSessionCount()
{
    std::shared_lock lock(m_sessionMutex); // reading concurrente
    return m_sessions.size();
}
