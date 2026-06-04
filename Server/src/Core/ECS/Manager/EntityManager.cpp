#include "LKZ/Core/ECS/Manager/EntityManager.h"
#include "LKZ/Core/ECS/Manager/ComponentManager.h"
#include "LKZ/Core/Engine.h"
#include "LKZ/Simulation/World.h"
#include <LKZ/Core/Manager/MetricsManager.h>
#include "LKZ/Core/ECS/Component/Component.h"
#include "DetourCrowd.h"
#include <iostream>
#include <vector>

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

    std::lock_guard<std::mutex> lock(m_destructionMutex);

    for (auto const& [entity, mappedSession] : m_entitySessionMap)
    {
        if (mappedSession == session)
        {
            EntitySuperType type = m_entityTypeMap[entity];
            // Filter for Zombie or Primitive
            if (type == EntitySuperType::Zombie || type == EntitySuperType::Primitive)
            {
                m_pendingDestructions.push_back(entity);
            }
        }
    }
}

void EntityManager::DestroyEntity(Entity entity)
{
    std::lock_guard<std::mutex> lock(m_destructionMutex);
    m_pendingDestructions.push_back(entity);
}

void EntityManager::ApplyPendingDestructions()
{
    std::lock_guard<std::mutex> lock(m_destructionMutex);

    if (m_pendingDestructions.empty()) return;

    auto& components = ComponentManager::Instance();
    World& world = Engine::Instance().GetWorld();
    dtCrowd* crowd = world.getCrowd();

    for (Entity entity : m_pendingDestructions)
    {
        if (m_entitySessionMap.find(entity) == m_entitySessionMap.end()) continue;

        // DetourCrowd Simulation Cleanup
        auto aiIt = components.ai.find(entity);
        if (aiIt != components.ai.end())
        {
            if (aiIt->second.crowdAgentIndex != -1)
            {
                if (crowd)
                {
                    crowd->removeAgent(aiIt->second.crowdAgentIndex);
                }
            }
            // Safely erase from the AI component map
            components.ai.erase(aiIt);
        }

        components.positions.erase(entity);
        components.rotations.erase(entity);
        components.playerInputs.erase(entity);

        MetricsManager::Instance().currentMetrics.activeEntityCount--;
        m_freeIDs.push(entity);
        m_entitySessionMap.erase(entity);
        m_entityTypeMap.erase(entity);
        m_lastSequenceIds.erase(entity);
    }

    m_pendingDestructions.clear();
}

Entity EntityManager::GetEntityById(uint16_t entityId, Session* lobby)
{
    for (const auto& [entity, mappedLobby] : m_entitySessionMap) {
        if (entity == entityId && mappedLobby == lobby) {
            return entity;
        }
    }
    return 0;
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

    return 0;
}