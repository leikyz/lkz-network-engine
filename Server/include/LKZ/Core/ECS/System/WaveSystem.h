#pragma once
#include "LKZ/Core/ECS/System/ISystem.h"

// Wave System to handle wave-based enemy spawning

class WaveSystem : public ISystem 
{
public:
    void Update(ComponentManager& components, float deltaTime) override;
private:
	// Spawns a zombie entity in the specified lobby with the given entity super type ID
    void SpawnZombie(int lobbyId, int entitySuperTypeId);
};
