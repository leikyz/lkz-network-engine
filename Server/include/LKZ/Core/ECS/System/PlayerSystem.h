#pragma once
#include "LKZ/Core/ECS/System/ISystem.h"

// Player System to handle player entity behavior

class PlayerSystem : public ISystem 
{
public:
    void Update(ComponentManager& components, float deltaTime) override;
};
