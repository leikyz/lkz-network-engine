#pragma once
#include <vector>
#include <memory>
#include "LKZ/Core/ECS/System/ISystem.h"
#include "LKZ/Core/ECS/Manager/ComponentManager.h"

// Manager for all systems in the ECS architecture
class SystemManager
{
public:
    // Singleton instance access
    static SystemManager& Instance()
    {
        static SystemManager instance;
        return instance;
    }

	// Registers a new system
    void RegisterSystem(std::shared_ptr<ISystem> system);

	// Updates all registered systems
    void Update(ComponentManager& components, float deltaTime);

private:
    std::vector<std::shared_ptr<ISystem>> m_systems;

    // Private constructor to enforce singleton
    SystemManager() = default;

	// Delete copy constructor and assignment operator
    SystemManager(const SystemManager&) = delete;

	// Delete assignment operator
    SystemManager& operator=(const SystemManager&) = delete;
};
