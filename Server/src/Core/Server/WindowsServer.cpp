#include "LKZ/Core/Server/WindowsServer.h"
#include <iostream>
#include <LKZ/Core/Manager/EventManager.h>
struct SendIoData {
    OVERLAPPED overlapped{};
    WSABUF wsabuf{};
    sockaddr_in target{};

    // On garde un pointeur vers les données. 
    // Si c'est un buffer partagé, il ne sera pas détruit tant que cet objet existe.
    std::unique_ptr<uint8_t[]> data;
};

WindowsServer::WindowsServer(int port, size_t bufferSize)
    : port(port), bufferSize(bufferSize) {}

WindowsServer::~WindowsServer() 
{
    running = false;
    if (completionPort) CloseHandle(completionPort);
    if (listenSocket != INVALID_SOCKET) closesocket(listenSocket);
    WSACleanup();
}

void WindowsServer::Start() 
{

    EventManager::BindEvents();

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[WindowsServer] WSAStartup failed\n";
        return;
    }

    listenSocket = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, nullptr, 0, WSA_FLAG_OVERLAPPED);

    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "[WindowsServer] socket creation failed\n";
        return;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "[WindowsServer] bind failed\n";
        return;
    }

    InitIOCP();
	running = true;

    std::cout << "[WindowsServer] Server started on port " << port << "\n";
}

void WindowsServer::InitIOCP() 
{
    completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
    if (!completionPort)
    {
        std::cerr << "[WindowsServer] CreateIoCompletionPort failed\n";
        return;
    }

    if (!CreateIoCompletionPort((HANDLE)listenSocket, completionPort, 0, 0)) 
    {
        std::cerr << "[WindowsServer] Associate socket with IOCP failed\n";
        return;
    }

    // Preallocate IoData
    for (size_t i = 0; i < std::thread::hardware_concurrency() * 2; i++) 
    {
        auto ioData = std::make_unique<IoData>(bufferSize);
        PostReceive(ioData.get());
        ioDataPool.push_back(std::move(ioData));
    }
}
void WindowsServer::SendToMultiple(const std::vector<Client*>& clients, const std::vector<uint8_t>& buffer, const char* messageName, const Client* excludedClient)
{
    for (const auto& clientPtr : clients)
    {
        if (!clientPtr) continue;

        if (excludedClient &&
            clientPtr->address.sin_addr.s_addr == excludedClient->address.sin_addr.s_addr &&
            clientPtr->address.sin_port == excludedClient->address.sin_port)
        {
            continue; // Skip excluded client
        }

        Send(clientPtr->address, buffer, bufferSize, messageName);
    }
}

void WindowsServer::PostReceive(IoData* ioData) {
    DWORD flags = 0;
    ioData->clientAddrLen = sizeof(ioData->clientAddr);
    ZeroMemory(&ioData->overlapped, sizeof(OVERLAPPED));

    int ret = WSARecvFrom(listenSocket, &ioData->wsabuf, 1, nullptr, &flags,
                           (sockaddr*)&ioData->clientAddr, &ioData->clientAddrLen,
                           &ioData->overlapped, nullptr);

    if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        std::cerr << "[WindowsServer] WSARecvFrom failed\n";
    }
}

void WindowsServer::Poll() 
{
    DWORD bytesTransferred;
    ULONG_PTR completionKey;
    OVERLAPPED* overlapped;

    BOOL success = GetQueuedCompletionStatus(
        completionPort,
        &bytesTransferred,
        &completionKey,
        &overlapped,
		INFINITE // Block indefinitely until an I/O operation completes
    );

    if (!overlapped) return;

    IoData* ioData = CONTAINING_RECORD(overlapped, IoData, overlapped);

    if (bytesTransferred > 0)
    {
        EventManager::processMessage(
            ioData->buffer.data(),
            bytesTransferred,
            ioData->clientAddr
        );
    }
    PostReceive(ioData);
 /*   NotifyThreadPool(ioData, bytesTransferred);*/
}

void WindowsServer::Send(const sockaddr_in& clientAddr, uint8_t* data, size_t size, const char* messageName)
{
    auto* io = new SendIoData{};

    io->data = std::make_unique<uint8_t[]>(size);
    memcpy(io->data.get(), data, size);

    io->wsabuf.buf = (CHAR*)io->data.get();
    io->wsabuf.len = (ULONG)size;
    io->target = clientAddr;
    ZeroMemory(&io->overlapped, sizeof(OVERLAPPED));

    int ret = WSASendTo(
        listenSocket,
        &io->wsabuf,
        1,
        nullptr,
        0,
        (sockaddr*)&io->target,
        sizeof(io->target),
        &io->overlapped,
        nullptr
    );

    if (ret == SOCKET_ERROR)
    {
        int err = WSAGetLastError();
        if (err != WSA_IO_PENDING)
        {
            std::cerr << "[WindowsServer] WSASendTo failed: " << err << "\n";
            delete io; 
        }
    }
}
