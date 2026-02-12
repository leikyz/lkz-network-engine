#ifndef JOIN_SESSION_MESSAGE_H
#define JOIN_SESSION_MESSAGE_H

#include "LKZ/Protocol/Message/Message.h"
#include <vector>
#include <cstdint>

class JoinSessionMessage : public Message 
{
public:
    static constexpr uint8_t ID = 28;

    uint32_t sessionToken;
    uint32_t clientToken;

    JoinSessionMessage();

    uint8_t getId() const override { return ID; }
    std::vector<uint8_t>& serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer) override;
    void process(const sockaddr_in& senderAddr) override;
};

#endif