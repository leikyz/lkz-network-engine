#pragma once

#include <vector>
#include <cstdint>
#include <algorithm> // Indispensable pour std::remove
#include <string>
#include "LKZ/Core/ECS/Entity.h"
#include "LKZ/Utility/Logger.h"

enum class LobbyState {
    WAITING,
    STARTING,
    IN_GAME,
    ENDING
};

struct Lobby
{
    LobbyState state = LobbyState::WAITING;
    static constexpr int MAX_PLAYER = 4;
    int id;
    uint8_t mapId;

	// Store only client IDs to reduce dependencies
    std::vector<uint32_t> clientIds;

    Entity gameWaveEntity;

    int nextEntityId = 1;
    bool inGame = false;
    bool gameLoaded = false;
    bool isStarting = false;
    float startTimer = 0.0f;
    const float TIME_BEFORE_FIRST_WAVE = 5.0f;

    Lobby(int lobbyId, uint8_t mapId) : id(lobbyId), mapId(mapId) {
        clientIds.reserve(MAX_PLAYER);
    }

    void addClient(uint32_t clientId)
    {
        clientIds.push_back(clientId);
        Logger::Log("Client ID " + std::to_string(clientId) + " added to lobby: " + std::to_string(id), LogType::Info);
    }

    void removeClient(uint32_t clientId)
    {
        clientIds.erase(std::remove(clientIds.begin(), clientIds.end(), clientId), clientIds.end());
        Logger::Log("Client ID " + std::to_string(clientId) + " removed from lobby: " + std::to_string(id), LogType::Info);
    }

    const std::vector<uint32_t>& getClientsIds() const { return clientIds; }
    int getClientCount() const { return static_cast<int>(clientIds.size()); }
};