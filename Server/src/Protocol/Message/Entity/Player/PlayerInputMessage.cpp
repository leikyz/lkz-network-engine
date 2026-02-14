#include "LKZ/Protocol/Message/Entity/Player/PlayerInputMessage.h"
#include <LKZ/Core/ECS/Manager/ComponentManager.h>
#include <LKZ/Core/ECS/Manager/EntityManager.h>
#include <LKZ/Core/Manager/ClientManager.h>
#include <LKZ/Core/Engine.h>
#include <LKZ/Utility/Constants.h>
#include "LKZ/Core/Threading/CommandQueue.h" // Required for the fix

PlayerInputMessage::PlayerInputMessage() {}

PlayerInputMessage::PlayerInputMessage(uint16_t entityId, float inputX, float inputY, float yaw, bool isSprinting, bool isAiming, bool isArmed, int sequenceId)
    : entityId(entityId), inputX(inputX), inputY(inputY), yaw(yaw), isSprinting(isSprinting), isAiming(isAiming), isArmed(isArmed), sequenceId(sequenceId)
{
}

uint8_t PlayerInputMessage::getId() const
{
    return ID;
}

std::vector<uint8_t>& PlayerInputMessage::serialize(Serializer& serializer) const
{
    serializer.writeByte(ID);
    serializer.writeUInt16(entityId);
    serializer.writeFloat(inputX);
    serializer.writeFloat(inputY);
    serializer.writeFloat(yaw);
    serializer.writeBool(isSprinting);
    serializer.writeBool(isAiming);
    serializer.writeBool(isArmed);
    serializer.writeInt(sequenceId);

    return serializer.getBuffer();
}

void PlayerInputMessage::deserialize(Deserializer& deserializer)
{
    entityId = deserializer.readUInt16();
    inputX = deserializer.readFloat();
    inputY = deserializer.readFloat();
    yaw = deserializer.readFloat();
    isSprinting = deserializer.readBool();
    isAiming = deserializer.readBool();
    isArmed = deserializer.readBool();
    sequenceId = deserializer.readInt();
}

void PlayerInputMessage::process(const sockaddr_in& senderAddr, SOCKET tcpSocket)
{
    Session* session = SessionManager::GetSessionByAddress(senderAddr);
    if (!session) return;

    uint32_t targetEntityId = this->entityId;
    float currentYaw = this->yaw;

    PlayerInputData packet;
    packet.inputX = this->inputX;
    packet.inputY = this->inputY;
    packet.yaw = this->yaw;
    packet.isAiming = this->isAiming;
    packet.isRunning = this->isSprinting;
    packet.isArmed = this->isArmed;
    packet.sequenceId = this->sequenceId;

    CommandQueue::Instance().Push([targetEntityId, packet, currentYaw]()
        {
            auto& components = ComponentManager::Instance();

            if (components.playerInputs.find(targetEntityId) != components.playerInputs.end())
            {
                components.playerInputs[targetEntityId].inputQueue.push_back(packet);
            }

            if (components.rotations.find(targetEntityId) != components.rotations.end())
            {
                components.rotations[targetEntityId].rotation.y = currentYaw;
            }
        });

    Serializer serializer;
    serialize(serializer); 

    const std::vector<uint8_t>& buffer = serializer.getBuffer();
    const std::string& className = getClassName();

    for (const auto& player : session->players)
    {
        if (player.isUdpReady && player.entityId != entityId)
        {

            Engine::Instance().Server()->Send(player.udpAddr, buffer, getClassName());
        }
    }
}