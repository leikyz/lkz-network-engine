#include "LKZ/Core/Server/WindowsServer.h"
#include <iostream>
#include <LKZ/Core/Manager/EventManager.h>


WindowsServer::WindowsServer(int port, size_t bufferSize)
    : port(port), bufferSize(bufferSize) {}

WindowsServer::~WindowsServer() 
{
    running = false;
    if (completionPort) CloseHandle(completionPort);
    if (udpSocket != INVALID_SOCKET) closesocket(udpSocket);
    if (tcpSocket != INVALID_SOCKET) closesocket(tcpSocket); 
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

    udpSocket = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, nullptr, 0, WSA_FLAG_OVERLAPPED);

    if (udpSocket == INVALID_SOCKET) {
        std::cerr << "[WindowsServer] UDP socket creation failed\n";
        return;
    }

    tcpSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);

    if (tcpSocket == INVALID_SOCKET) {
        std::cerr << "[HTTP] socket creation failed\n";
        return;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(udpSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "[WindowsServer] UDP bind failed\n";
        return;
    }

    sockaddr_in httpAddr{};
    httpAddr.sin_family = AF_INET;
    httpAddr.sin_addr.s_addr = INADDR_ANY;
    httpAddr.sin_port = htons(Constants::TCP_PORT); 

    if (bind(tcpSocket, (sockaddr*)&httpAddr, sizeof(httpAddr))) {
        std::cerr << "[WindowsServer] TCP bind failed\n";
        return;
    }

    listen(tcpSocket, SOMAXCONN);


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

    if (!CreateIoCompletionPort((HANDLE)udpSocket, completionPort, 0, 0)) 
    {
        std::cerr << "[WindowsServer] Associate UDP socket with IOCP failed\n";
        return;
    }

    if (!CreateIoCompletionPort((HANDLE)tcpSocket, completionPort, 0, 0))
    {
        std::cerr << "[WindowsServer] Associate TCP socket with IOCP failed\n";
        return;
    }

    // Preallocate IoData
    for (size_t i = 0; i < std::thread::hardware_concurrency() * 2; i++) 
    {
        auto ioData = std::make_unique<ReceiveUDPIoData>(bufferSize);
        PostReceive(ioData.get());
        ioDataPool.push_back(std::move(ioData));
    }
}
void WindowsServer::SendToMultiple(std::span<const sockaddr_in> addresses, std::span<const uint8_t> data, const char* messageName, const sockaddr_in* excludedAddr)
{
    for (const auto& addr : addresses)
    {
		// Verify if we need to exclude this address
        if (excludedAddr &&
            addr.sin_addr.s_addr == excludedAddr->sin_addr.s_addr &&
            addr.sin_port == excludedAddr->sin_port)
        {
			continue; // Skip this address
        }

        Send(addr, data, messageName);
    }
}

void WindowsServer::PostReceive(ReceiveUDPIoData* ioData) {
    DWORD flags = 0;
    ioData->clientAddrLen = sizeof(ioData->clientAddr);
    ZeroMemory(&ioData->overlapped, sizeof(OVERLAPPED));

    int ret = WSARecvFrom(udpSocket, &ioData->wsabuf, 1, nullptr, &flags,
                           (sockaddr*)&ioData->clientAddr, &ioData->clientAddrLen,
                           &ioData->overlapped, nullptr);

    if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        std::cerr << "[WindowsServer] WSARecvFrom failed\n";
    }
}

void WindowsServer::Poll() 
{
    DWORD bytesTransferred = 0;
    ULONG_PTR completionKey = 0;
    OVERLAPPED* overlapped = nullptr;

    BOOL success = GetQueuedCompletionStatus(
        completionPort, &bytesTransferred, &completionKey, &overlapped, INFINITE
    );

    if (!overlapped) return;

    BaseIo* base = CONTAINING_RECORD(overlapped, BaseIo, overlapped);

    switch (base->opType) 
    {
    case IO_OPERATION::RECEIVE_UDP: 
    {
        ReceiveUDPIoData* ioData = static_cast<ReceiveUDPIoData*>(base);

        if (success && bytesTransferred > 0)
            EventManager::processMessage(ioData->buffer, ioData->clientAddr);

        PostReceive(ioData); 
        break;
    }

    case IO_OPERATION::SEND_UDP: {
        delete static_cast<SendUDPIoData*>(base);
        break;
    }

    case IO_OPERATION::ACCEPT_TCP: {
        std::cout << "New TCP Connection" << std::endl;
        break;
    }

    case IO_OPERATION::RECEIVE_TCP: {
        std::cout << "New TCP Data received" << std::endl;
        break;
    }
    }
}

void WindowsServer::Send(
    const sockaddr_in& clientAddr,
    std::span<const uint8_t> buffer,
    const char* messageName
)
{
    auto* io = new SendUDPIoData{buffer, clientAddr};

    io->data.assign(buffer.begin(), buffer.end());

    io->wsabuf.buf = reinterpret_cast<CHAR*>(io->data.data());
    io->wsabuf.len = static_cast<ULONG>(io->data.size());
    io->target = clientAddr;

    ZeroMemory(&io->overlapped, sizeof(OVERLAPPED));

    int ret = WSASendTo(
        udpSocket,
        &io->wsabuf,
        1,
        nullptr,
        0,
        reinterpret_cast<sockaddr*>(&io->target),
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
