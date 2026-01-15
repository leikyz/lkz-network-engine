#include "LKZ/Core/ECS/Manager/SystemManager.h"

void SystemManager::RegisterSystem(std::shared_ptr<ISystem> system)
{
    m_systems.push_back(system);
}

void SystemManager::Update(ComponentManager& components, float deltaTime)
{
    for (auto& system : m_systems)
    {
        system->Update(components, deltaTime);
    }
}
