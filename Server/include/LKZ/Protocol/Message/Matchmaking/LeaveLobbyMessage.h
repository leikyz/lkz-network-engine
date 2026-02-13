#ifndef LEAVE_LOBBY_MESSAGE_H
#define LEAVE_LOBBY_MESSAGE_H

#include "LKZ/Protocol/Message/Message.h"

struct LeaveLobbyMessage : public Message
{
    static constexpr uint8_t ID = 7;

    LeaveLobbyMessage();

    uint8_t positionInLobby;

    uint8_t getId() const override;

    std::vector<uint8_t>& serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer) override;
    void process(const sockaddr_in& senderAddr, SOCKET tcpSocket) override;
};

#endif // LEAVE_LOBBY_MESSAGE_H

