#ifndef DELETE_ALL_ENTITIES_MESSAGE_H
#define DELETE_ALL_ENTITIES_MESSAGE_H

#include "LKZ/Protocol/Message/Message.h"

struct DeleteAllEntitiesMessage : public Message
{
    static constexpr uint8_t ID = 34;

    DeleteAllEntitiesMessage(); 

    uint8_t getId() const override;

    std::vector<uint8_t>& serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer) override;
    void process(const sockaddr_in& senderAddr, SOCKET tcpSocket) override;
};

#endif // DELETE_ALL_ENTITIES_MESSAGE_H
