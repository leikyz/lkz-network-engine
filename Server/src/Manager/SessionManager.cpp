#include "LKZ/Core/Manager/SessionManager.h"
#include <algorithm>
#include <LKZ/Utility/Constants.h>

// Static member definitions
std::unordered_map<int, size_t> SessionManager::m_idToIndex;
std::atomic<int> SessionManager::m_nextSessionId{ 1 };
std::shared_mutex SessionManager::m_sessionMutex;
std::vector<Session> SessionManager::m_sessions;

void SessionManager::Initialize()
{
    std::unique_lock lock(m_sessionMutex);

    m_idToIndex.clear();;
	m_sessions.clear();

	m_sessions.reserve(Constants::MAX_SESSION); 
}

void SessionManager::CreateSession()
{
    int sessionId = m_nextSessionId.fetch_add(1);

    std::unique_lock lock(m_sessionMutex);

	m_sessions.emplace_back(sessionId);
	m_idToIndex[sessionId] = m_sessions.size() - 1;
}

void SessionManager::AddClientToSession(uint32_t sessionId, sockaddr_in& clientAddress)
{
    std::unique_lock lock(m_sessionMutex);

    auto it = m_idToIndex.find(sessionId);
    if (it != m_idToIndex.end())
    {
		if (m_sessions[it->second].clientsAddress.size() >= Constants::MAX_PLAYERS_PER_SESSION || m_sessions.capacity() >= Constants::MAX_SESSION)
        {
            return; // Session is full
        }

		m_sessions[it->second].clientsAddress.emplace_back(clientAddress);
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
        std::erase_if(m_sessions[it->second].clientIds, [&](uint32_t id) {
            return id == clientId;
			});
    }
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
