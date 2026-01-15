#pragma once
#include "LKZ/Core/ECS/Entity.h"
#include <LKZ/Simulation/Math/Vector.h>
#include <optional>
#include <vector>

// Component definitions for the ECS architecture

enum class EntitySuperType : uint8_t
{
    GameManager = 0,
    Player = 1,
    Zombie = 2,
};

enum class EntityType : uint8_t
{
    Player1 = 1,
    PlayerSynced1 = 2,
    ZombieBase1 = 3,
    ZombieBase2 = 4,
	ZombieBase3 = 5
};

// Plyer input data structure
struct PlayerInputData
{
    int sequenceId;

    float inputX;
    float inputY;
    float yaw;

    bool isAiming;
    bool isRunning; 
    bool isArmed;   
};

// Component to store player input information
struct PlayerInputComponent
{
    std::vector<PlayerInputData> inputQueue;
    int lastExecutedSequenceId = -1;

    Vector3 currentVelocity = { 0.0f, 0.0f, 0.0f };
};

// Component to manage wave-based enemy spawning
struct WaveComponent 
{
    int lobbyId;

	bool canSpawn = false;

    int currentWave = 0;
    float stateTimer = 0.0f;
    bool isIntermission = true; 

    int zombiesToSpawn = 0;  
    int zombiesAlive = 0;     
    float spawnTimer = 0.0f;  
};

// Component to store position data (used by every Entity with a position)
struct PositionComponent
{
    Vector3 position;
};

// Component to store rotation data (used by every Entity with a rotation)
struct RotationComponent
{
    Vector3 rotation;
};

// Component to store AI-related data for entities (using recast navigation)
struct AIComponent
{
    std::optional<Vector3> targetPosition;
    float repathTimer;
    int crowdAgentIndex;
    float timeSinceLastSend = 0.0f;

    PositionComponent* posPtr = nullptr;
    RotationComponent* rotPtr = nullptr;

    Vector3 lastSentPosition = { 0, 0, 0 };
};