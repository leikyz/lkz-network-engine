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
#pragma comment(lib, "ws2_32.lib")

 /**
 * @brief Structure to hold data for each I/O operation.
 */
struct IoData 
{
    OVERLAPPED overlapped{};
    WSABUF wsabuf{};
    sockaddr_in clientAddr{};
    int clientAddrLen = sizeof(sockaddr_in);
    std::vector<uint8_t> buffer;

    IoData(size_t bufferSize) : buffer(bufferSize) {
        wsabuf.buf = reinterpret_cast<char*>(buffer.data());
        wsabuf.len = static_cast<ULONG>(buffer.size());
    }
};

 /**
 * @brief Windows-specific server implementation using IOCP for high-performance networking.
 */
class WindowsServer : public INetworkInterface 
{
public:
    explicit WindowsServer(int port, size_t bufferSize = Constants::NETWORK_BUFFER_SIZE);
    ~WindowsServer() override;

    void Start() override;
    void Send(const sockaddr_in& clientAddr, uint8_t* data, size_t size, const char* messageName) override;
    void SendToMultiple(const std::vector<Client*>& clients,const std::vector<uint8_t>& buffer, const char* messageName, const Client* excludedClient = nullptr) override;
    void Poll() override;

private:

     /**
	 * @brief Initializes the IO Completion Port and starts worker threads.
     */
    void InitIOCP();

    /**
	 * @brief Accepts a new client connection and associates it with the IOCP.
     */
    void PostReceive(IoData* ioData);

    /**
	 * @brief Notifies the thread pool of a completed I/O operation.
     */
    void NotifyThreadPool(IoData* ioData, DWORD bytesTransferred);

private:
    int port;
    SOCKET listenSocket = INVALID_SOCKET;
    HANDLE completionPort = nullptr;

    std::vector<std::unique_ptr<IoData>> ioDataPool;
    size_t bufferSize;

    bool running = false;
};

#endif // WINDOWS_SERVER_H
