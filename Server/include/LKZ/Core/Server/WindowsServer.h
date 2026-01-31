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

enum class IO_OPERATION { RECEIVE, SEND };

// Base structure for I/O operations
struct BaseIo {
    OVERLAPPED overlapped{};
    IO_OPERATION opType;
};

// Structure for receive I/O operations
struct ReceiveIoData : public BaseIo {
    ReceiveIoData() { opType = IO_OPERATION::RECEIVE; }
    OVERLAPPED overlapped{};
    WSABUF wsabuf{};
    sockaddr_in clientAddr{};
    int clientAddrLen = sizeof(sockaddr_in);
    std::vector<uint8_t> buffer;

    ReceiveIoData(size_t bufferSize) : buffer(bufferSize) {
        wsabuf.buf = reinterpret_cast<char*>(buffer.data());
        wsabuf.len = static_cast<ULONG>(buffer.size());
    }
};

// Structure for send I/O operations
struct SendIoData : public BaseIo {
    SendIoData() { opType = IO_OPERATION::SEND; }
    OVERLAPPED overlapped{};
    WSABUF wsabuf{};
    sockaddr_in target{};
    std::vector<uint8_t> data;
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
    void PostReceive(ReceiveIoData* ioData);

	// Handles completed I/O operations from the IOCP


private:
    int port;
    SOCKET listenSocket = INVALID_SOCKET;
    HANDLE completionPort = nullptr;

    std::vector<std::unique_ptr<ReceiveIoData>> ioDataPool;
    size_t bufferSize;

    bool running = false;
};

#endif // WINDOWS_SERVER_H
