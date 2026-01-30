#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include <unordered_map>
#include <vector>
#include <functional>
#include <string>
#include <winsock2.h>

// Manager for handling incoming events/messages

class EventManager
{
public:
	// Type definition for message handler functions
    using MessageHandler = void(*)(const uint8_t* data, const sockaddr_in&);

    static void BindEvents();
    static void processMessage(const uint8_t* data, size_t bytesTransfered, const sockaddr_in& senderAddr);

private:
	// Array of message handlers indexed by message ID (0-255) can handle up to 256 different message types
    static MessageHandler messageHandlers[256];

    template<typename T>
	// Generic message handler to process messages of type T
    static void handleMessage(const std::vector<uint8_t>& buffer, const sockaddr_in& senderAddr);

    template<typename T>
	// Registers a message handler for a specific message ID
    static void registerHandler(uint8_t id);
};

#endif // EVENT_MANAGER_H