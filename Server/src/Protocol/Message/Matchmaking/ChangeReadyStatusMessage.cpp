#include "LKZ/Protocol/Message/Matchmaking/ChangeReadyStatusMessage.h"
#include "LKZ/Protocol/Message/Matchmaking/StartGameMessage.h"
#include <LKZ/Core/ECS/Manager/EntityManager.h>
ChangeReadyStatusMessage::ChangeReadyStatusMessage() {}

uint8_t ChangeReadyStatusMessage::getId() const
{
    return ID;
}

std::vector<uint8_t>& ChangeReadyStatusMessage::serialize(Serializer& serializer) const
{
    serializer.writeByte(ID);
    serializer.writeBool(isReady);
	serializer.writeByte(positionInLobby);
    return serializer.getBuffer();
}


void ChangeReadyStatusMessage::deserialize(Deserializer& deserializer)
{
	isReady = deserializer.readBool();
}

void ChangeReadyStatusMessage::process(const sockaddr_in& senderAddr)
{
	Client* currentClient = ClientManager::getClientByAddress(senderAddr);

	Lobby* lobby = LobbyManager::getLobby(currentClient->lobbyId);

    if (currentClient && lobby)
    {
        currentClient->isReady = isReady;
		positionInLobby = currentClient->positionInLobby;

        Serializer serializer;
        serialize(serializer);

        Engine::Instance().Server()->SendToMultiple(LobbyManager::getClientsInLobby(lobby->id), serializer.getBuffer(), getClassName());

        if (LobbyManager::IsEveryoneReadyInLobby(lobby->id))
        {
            StartGameMessage startGameMsg;
			startGameMsg.mapId = lobby->mapId;
            Serializer s;
            std::vector<uint8_t> buf = startGameMsg.serialize(s);

            Engine::Instance().Server()->SendToMultiple(LobbyManager::getClientsInLobby(lobby->id), buf, getClassName());

			lobby->inGame = true;

            ComponentManager& components = ComponentManager::Instance();

            Entity entity = EntityManager::Instance().CreateEntity(EntitySuperType(EntitySuperType::GameManager), components, lobby);
            lobby->gameWaveEntity = entity;
            components.AddComponent(entity, WaveComponent{ lobby->id });
        }
	}


}
