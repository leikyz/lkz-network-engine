#pragma once
#include "LKZ/Session/Lobby.h"
#include "ComponentManager.h"
#include <queue>
#include <unordered_map>

// Manager for entity creation, destruction, and tracking
class EntityManager 
{
public:
    static EntityManager& Instance() 
    {
        static EntityManager instance;
        return instance;
    }

    Entity nextID = 1; // start at 1, 0 is invalid

    void SetLastSequenceId(Entity entity, uint32_t sequenceId);

    uint32_t GetLastSequenceId(Entity entity) const;

    Entity CreateEntity(EntitySuperType type, ComponentManager& components, Lobby* lobby);

    void DestroyEntity(Entity entity);

    Lobby* GetLobbyByEntity(Entity entity);

    Entity GetEntityById(uint16_t entityId, Lobby* lobby);

private:
	// Private constructor for singleton pattern
    EntityManager() = default;

	// Delete copy constructor and assignment operator
    EntityManager(const EntityManager&) = delete;

	// Delete assignment operator
    EntityManager& operator=(const EntityManager&) = delete;

	// Maps to track last sequence IDs and entity-lobby associations
    std::unordered_map<Entity, uint32_t> m_lastSequenceIds;

	// Queue to manage reusable entity IDs
    std::queue<Entity> m_freeIDs;

	// Map to associate entities with their respective lobbies
    std::unordered_map<Entity, Lobby*> m_entityLobbyMap;
};

