#include "LKZ/Core/ECS/Manager/EntityManager.h"
#include "LKZ/Core/ECS/Component/Component.h."
#include <iostream>

Entity EntityManager::CreateEntity(EntitySuperType type, ComponentManager& components, Session* session)
{
    Entity id;
    if (!m_freeIDs.empty()) {
        id = m_freeIDs.front();
        m_freeIDs.pop();
    }
    else {
        id = nextID++;
    }

    m_entitySessionMap[id] = session;

    return id;
}

void EntityManager::DestroyEntity(Entity entity)
{

    m_freeIDs.push(entity);
    m_entitySessionMap.erase(entity);
}

Entity EntityManager::GetEntityById(uint16_t entityId, Session* lobby)
{

    for (const auto& [entity, mappedLobby] : m_entitySessionMap) {
        if (entity == entityId && mappedLobby == lobby) {
            return entity;
        }
    }
    return 0; // or some invalid entity value
}

Session* EntityManager::GetSessionByEntity(Entity entity)
{

    auto it = m_entitySessionMap.find(entity);
    if (it != m_entitySessionMap.end()) {
        return it->second;
    }
    return nullptr;
}

void EntityManager::SetLastSequenceId(Entity entity, uint32_t sequenceId) {

    m_lastSequenceIds[entity] = sequenceId;
}

uint32_t EntityManager::GetLastSequenceId(Entity entity) const {

    auto it = m_lastSequenceIds.find(entity);
    if (it != m_lastSequenceIds.end())
        return it->second;

    return 0; // default if not found
}

