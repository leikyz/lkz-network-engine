#include "LKZ/Core/ECS/Manager/EntityManager.h"
#include <LKZ/Core/Manager/MetricsManager.h>
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
    m_entityTypeMap[id] = type;

    return id;
}
void EntityManager::DestroyEntitiesBySession(Session* session)
{
    if (!session) return;

    // Collect IDs first to avoid iterator invalidation while erasing
    std::vector<Entity> targets;

    for (auto const& [entity, mappedSession] : m_entitySessionMap)
    {
        if (mappedSession == session)
        {
            EntitySuperType type = m_entityTypeMap[entity];
            // Filter for Zombie (2) or Primitive (3)
            if (type == EntitySuperType::Zombie || type == EntitySuperType::Primitive)
            {
                targets.push_back(entity);
            }
        }
    }

    // Use the existing DestroyEntity logic for each target
    for (Entity entity : targets)
    {
        DestroyEntity(entity);
    }
}

void EntityManager::DestroyEntity(Entity entity)
{
    MetricsManager::Instance().currentMetrics.activeEntityCount--;
    m_freeIDs.push(entity);
    m_entitySessionMap.erase(entity);
    m_entityTypeMap.erase(entity); 
    m_lastSequenceIds.erase(entity); 
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

