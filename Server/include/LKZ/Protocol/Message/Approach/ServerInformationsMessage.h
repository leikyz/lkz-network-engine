#ifndef SERVER_INFORMATIONS_MESSAGES_H
#define SERVER_INFORMATIONS_MESSAGES_H

#include "LKZ/Protocol/Message/Message.h"

struct ServerInformationsMessage : public Message
{
    static constexpr uint8_t ID = 2;

    ServerInformationsMessage();

    bool status;
    int playersCount;

    uint8_t getId() const override;

    std::vector<uint8_t>& serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer) override;
    void process(const sockaddr_in& senderAddr, SOCKET tcpSocket) override;
};

#endif // SERVER_INFORMATIONS_MESSAGES_H
