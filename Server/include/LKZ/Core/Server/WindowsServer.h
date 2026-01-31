#ifndef WINDOWS_SERVER_H
#define WINDOWS_SERVER_H

#include "INetworkInterface.h"
#include "LKZ/Core/Threading/ThreadManager.h"
#include "LKZ/Utility/Logger.h"
#include "LKZ/Utility/Constants.h"
#include "LKZ/Core/ECS/Manager/EntityManager.h"
#include "LKZ/Core/ECS/Manager/SystemManager.h"
#include "LKZ/Core/ECS/Manager/ComponentManager.h"
#include <vector>
#include <cstdint>
#include <memory>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <mswsock.h>
#include <span>
#pragma comment(lib, "ws2_32.lib")

enum class IO_OPERATION { RECEIVE_UDP, SEND_UDP, ACCEPT_TCP, RECEIVE_TCP };

// Base struct for I/O operations
struct BaseIo {
    OVERLAPPED overlapped{}; 
    IO_OPERATION opType;
};

// UDP receive I/O data structure
struct ReceiveUDPIoData : public BaseIo {
    WSABUF wsabuf{};
    sockaddr_in clientAddr{};
    int clientAddrLen = sizeof(sockaddr_in);
    std::vector<uint8_t> buffer;

    ReceiveUDPIoData(size_t bufferSize) {
        opType = IO_OPERATION::RECEIVE_UDP;
        buffer.resize(bufferSize); 
        wsabuf.buf = reinterpret_cast<char*>(buffer.data());
        wsabuf.len = static_cast<ULONG>(buffer.size());
    }
};

// UDP send I/O data structure
struct SendUDPIoData : public BaseIo {
    WSABUF wsabuf{};
    sockaddr_in target{};
    std::vector<uint8_t> data;

    SendUDPIoData(std::span<const uint8_t> buffer, const sockaddr_in& dest) {
        opType = IO_OPERATION::SEND_UDP;
        data.assign(buffer.begin(), buffer.end()); 
        target = dest;
        wsabuf.buf = reinterpret_cast<char*>(data.data());
        wsabuf.len = static_cast<ULONG>(data.size());
    }
};

// TCP receive I/O data structure
struct ReceiveTCPIoData : public BaseIo {
    SOCKET socket;
    uint8_t buffer[8192];
    WSABUF wsabuf;

    ReceiveTCPIoData(SOCKET s) : socket(s) {
        opType = IO_OPERATION::RECEIVE_TCP;
        wsabuf.buf = (CHAR*)buffer;
        wsabuf.len = sizeof(buffer);
    }
};

// Windows-specific server implementation using IOCP
class WindowsServer : public INetworkInterface 
{
public:
    explicit WindowsServer(int port, size_t bufferSize = Constants::NETWORK_BUFFER_SIZE);
    ~WindowsServer() override;

    void Start() override;
    void Send(const sockaddr_in& clientAddr, std::span<const uint8_t> data, const char* messageName) override;
    void SendToMultiple(std::span<const sockaddr_in> addresses, std::span<const uint8_t> data, const char* messageName, const sockaddr_in* excludedAddr = nullptr) override; 
    void Poll() override;
private:

    void InitIOCP();

	// Posts a receive operation to the IOCP for the given ioData
    void PostReceive(ReceiveUDPIoData* ioData);

	// Handles completed I/O operations from the IOCP


private:
    int port;
    SOCKET udpSocket = INVALID_SOCKET;
    SOCKET tcpSocket = INVALID_SOCKET; // HTTP 

    HANDLE completionPort = nullptr;

    std::vector<std::unique_ptr<ReceiveUDPIoData>> ioDataPool;
    size_t bufferSize;

    bool running = false;
};

#endif // WINDOWS_SERVER_H
