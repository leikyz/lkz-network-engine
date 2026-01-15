#pragma once
#include "LKZ/Core/ECS/System/ISystem.h"

// AI System to handle AI entity behavior
class AISystem : public ISystem 
{
public:
    void Update(ComponentManager& components, float deltaTime) override;
};
