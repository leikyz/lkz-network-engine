#ifndef UPDATE_LOBBY_MESSAGE_H
#define UPDATE_LOBBY_MESSAGE_H

#include "LKZ/Protocol/Message/Message.h"

class UpdateLobbyMessage : public Message
{
public:
    static constexpr uint8_t ID = 8;

    uint8_t updatedLobbyPos;
    uint8_t playersCount;
    std::vector<uint8_t> playersInLobby;

    UpdateLobbyMessage();

    uint8_t getId() const override { return ID; }

    std::vector<uint8_t>& serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer) override;
    void process(const sockaddr_in& senderAddr, SOCKET tcpSocket) override;

};

#endif // UPDATE_LOBBY_MESSAGE_H