#pragma once

#include <winsock2.h>
#include "LKZ/Core/Manager/ClientManager.h"
#include "LKZ/Core/Manager/SessionManager.h"
#include "LKZ/Core/Engine.h"
#include <Common/Codec/Serializer.h>
#include <Common/Codec/Deserializer.h>
struct Message
{
    Message() = default;

    constexpr virtual uint8_t getId() const = 0;
    virtual std::vector<uint8_t>& serialize(Serializer& serializer) const = 0;
    virtual void deserialize(Deserializer& deserializer) = 0;
    virtual void process(const sockaddr_in& senderAddr)  = 0;

    virtual ~Message() = default;

    virtual const char* getClassName() const
    {
        return typeid(*this).name() + 7; 
    }

};
