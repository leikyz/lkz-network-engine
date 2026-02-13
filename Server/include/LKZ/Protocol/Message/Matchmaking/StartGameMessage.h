#ifndef START_GAME_MESSAGE_H
#define START_GAME_MESSAGE_H

#include "LKZ/Protocol/Message/Message.h"

struct StartGameMessage : public Message
{
    static constexpr uint8_t ID = 25;

    StartGameMessage();

    uint8_t mapId;

    uint8_t getId() const override;

    std::vector<uint8_t>& serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer) override;
    void process(const sockaddr_in& senderAddr, SOCKET tcpSocket) override;
};

#endif // START_GAME_MESSAGE_H

