#ifndef I_NETWORK_INTERFACE_H
#define I_NETWORK_INTERFACE_H

#pragma once

#include <vector>
#include <cstdint>
#include <LKZ/Session/Client.h>

// Platform-specific includes for socket 
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>  
    #include <mswsock.h>  
    #pragma comment(lib, "ws2_32.lib")
#else
    //  Unix/Linux
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <sys/epoll.h>
#endif

class INetworkInterface 
{
public:
    virtual ~INetworkInterface() {}

    /**
    * @brief Starts the Windows server, initializes Winsock, creates a listening socket, and sets up IOCP.
    */
    virtual void Start() = 0;

    /**
    * @brief Sends a packet to a specified client address.
    *
    * \param clientAddr
    * \param buffer
    */
    virtual void Send(const sockaddr_in& client, uint8_t* data, size_t size, const char* messageName) = 0;

    /**
    * @brief Sends a packet to multiple clients, with an option to exclude a specific client.
    * @param clients Vector of pointers to Client objects to send the packet to.
    * @param buffer The data buffer to send.
    * @
    */
	virtual void SendToMultiple(const std::vector<Client*>& clients, const std::vector<uint8_t>& buffer, const char* messageName, const Client* excludedClient = nullptr) = 0;

    /**
	 * @brief Polls for completed I/O operations and processes them.
     * 
     */
    virtual void Poll() = 0;
};

#endif // I_NETWORK_INTERFACE_H