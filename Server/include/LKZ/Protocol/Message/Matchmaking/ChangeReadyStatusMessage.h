#ifndef CHANGE_READY_STATUS_MESSAGE_H
#define CHANGE_READY_STATUS_MESSAGE_H

#include "LKZ/Protocol/Message/Message.h"

struct ChangeReadyStatusMessage : public Message
{
    static constexpr uint8_t ID = 6;

    ChangeReadyStatusMessage();

    bool isReady;
    uint8_t positionInLobby;

    uint8_t getId() const override;

    std::vector<uint8_t>& serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer) override;
    void process(const sockaddr_in& senderAddr, SOCKET tcpSocket) override;
};

#endif // CHANGE_READY_STATUS_MESSAGE_H

