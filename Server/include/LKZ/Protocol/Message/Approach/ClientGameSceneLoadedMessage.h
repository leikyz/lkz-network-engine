#ifndef CLIENT_GAME_SCENE_LOADED_MESSAGE_H
#define CLIENT_GAME_SCENE_LOADED_MESSAGE_H

#include "LKZ/Protocol/Message/Message.h"

struct ClientGameSceneLoadedMessage : public Message
{
    static constexpr uint8_t ID = 22;

    ClientGameSceneLoadedMessage();

    uint8_t getId() const override;

    std::vector<uint8_t>& serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer) override;
    void process(const sockaddr_in& senderAddr, SOCKET tcpSocket) override;
};

#endif // CLIENT_GAME_SCENE_LOADED_MESSAGE_H
