//#include "LKZ/Protocol/Message/Entity/Player/PlayerStateMessage.h"
//#include <LKZ/Core/ECS/Manager/ComponentManager.h>
//#include <LKZ/Core/ECS/Manager/EntityManager.h>
//
//PlayerStateMessage::PlayerStateMessage() {}
//
//PlayerStateMessage::PlayerStateMessage(uint16_t entityId, uint8_t flags)
//    : entityId(entityId), flags(flags)
//{
//}
//
//uint8_t PlayerStateMessage::getId() const
//{
//    return ID;
//}
//
//std::vector<uint8_t>& PlayerStateMessage::serialize(Serializer& serializer) const
//{
//    serializer.writeByte(ID);
//    serializer.writeUInt16(entityId);
//
//    uint8_t flagsByte = 0;
//    if (isArmed())  flagsByte |= 1;  // Bit 0
//    if (isAiming()) flagsByte |= 2;  // Bit 1
//    if (isRunning()) flagsByte |= 4; // Bit 2
//
//    serializer.writeByte(flagsByte);
//
//    return serializer.getBuffer();
//}
//
//void PlayerStateMessage::deserialize(Deserializer& deserializer)
//{
//    entityId = deserializer.readUInt16();
//    flags = deserializer.readByte();
//}
//
//void PlayerStateMessage::process(const sockaddr_in& senderAddr, SOCKET tcpSocket)
//{
//    auto* client = ClientManager::getClientByAddress(senderAddr);
//    if (!client) return;
//
//    Lobby* lobby = LobbyManager::getLobby(client->lobbyId);
//    if (!lobby) return;
//
//    Entity entity = entityId;
//    auto& components = ComponentManager::Instance();
//
//    if (components.playerState.find(entity) != components.playerState.end())
//    {
//        auto& state = components.playerState[entity];
//
//		state.isAiming = isAiming();
//		state.isArmed = isArmed();
//		state.isRunning = isRunning();
//    }
//
//    Serializer serializer;
//    serialize(serializer);
//
//    Engine::Instance().Server()->SendToMultiple(
//        lobby->clients,
//        serializer.getBuffer(),
//        getClassName(),
//        ClientManager::getClientByAddress(senderAddr));
//}
//
//
//