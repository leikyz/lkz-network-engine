#ifndef START_MATCHMAKING_MESSAGE_H
#define START_MATCHMAKING_MESSAGE_H

#include "LKZ/Protocol/Message/Message.h"

struct StartMatchmakingMessage : public Message
{
    static constexpr uint8_t ID = 4;

    uint8_t mapId;

    StartMatchmakingMessage();

    uint8_t getId() const override;

    std::vector<uint8_t>& serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer) override;
    void process(const sockaddr_in& senderAddr, SOCKET tcpSocket) override;
};

#endif // DISCONNECT_CLIENT_MESSAGE_H
