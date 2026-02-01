#include "LKZ/Core/Server/WindowsServer.h"
#include <iostream>
#include <LKZ/Core/Manager/EventManager.h>


WindowsServer::WindowsServer(int port)
    : port(port) {}

WindowsServer::~WindowsServer() 
{
    running = false;
    if (completionPort) CloseHandle(completionPort);
    if (udpSocket != INVALID_SOCKET) closesocket(udpSocket);
    if (tcpSocket != INVALID_SOCKET) closesocket(tcpSocket); 
    WSACleanup();
}

#pragma region Initialization



void WindowsServer::Start() 
{
	EventManager::BindEvents(); // Initialize event handlers

	// Initialize Winsock
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[WindowsServer] WSAStartup failed\n";
        return;
    }

	// Create UDP socket
    udpSocket = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, nullptr, 0, WSA_FLAG_OVERLAPPED);

    if (udpSocket == INVALID_SOCKET) {
        std::cerr << "[WindowsServer] UDP socket creation failed\n";
        return;
    }

	// Create TCP socket for HTTP
    tcpSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);

    if (tcpSocket == INVALID_SOCKET) {
        std::cerr << "[HTTP] socket creation failed\n";
        return;
    }

	// Bind UDP socket
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(udpSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "[WindowsServer] UDP bind failed\n";
        return;
    }

	// Bind TCP socket
    sockaddr_in httpAddr{};
    httpAddr.sin_family = AF_INET;
    httpAddr.sin_addr.s_addr = INADDR_ANY;
    httpAddr.sin_port = htons(Constants::TCP_PORT); 

    if (bind(tcpSocket, (sockaddr*)&httpAddr, sizeof(httpAddr))) {
        std::cerr << "[WindowsServer] TCP bind failed\n";
        return;
    }

    InitIOCP();

    std::cout << "[WindowsServer] Server started on port " << port << "\n";
}

void WindowsServer::InitIOCP() 
{
    // Create a new IOCP port with default concurrency
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

	// Preallocate IoData (only for UDP receives)
    for (size_t i = 0; i < Constants::MAX_PENDING_UDP_RECEIVES; i++)
    {
        auto ioData = std::make_unique<ReceiveUDPIoData>(Constants::NETWORK_BUFFER_SIZE);
        PostReceive(ioData.get());
        receiveUDPPool.push_back(std::move(ioData));
    }

	// Preallocate Send IoData pool
    for (size_t i = 0; i < Constants::MAX_PENDING_UDP_RECEIVES; i++) {
        
        auto io = std::make_unique<SendUDPIoData>(std::vector<uint8_t>(Constants::NETWORK_BUFFER_SIZE), sockaddr_in{});
        sendUDPPool.push_back(std::move(io));
        availableSends.push(sendUDPPool.back().get());
    }
  
    listen(tcpSocket, SOMAXCONN); // Maximum pending connections
    PostAccept(); 

    running = true;
}
#pragma endregion

#pragma region Send

void WindowsServer::Send(const sockaddr_in& clientAddr, std::span<const uint8_t> buffer, const char* messageName)
{
    // Borrow a pre-allocated buffer from the pool.
    // We use a mutex to ensure that multiple threads don't grab the same buffer.
    SendUDPIoData* io = nullptr;
    {
        std::lock_guard<std::mutex> lock(sendPoolMutex);
        if (!availableSends.empty()) {
            io = availableSends.front();
            availableSends.pop();
        }
    }

    // If the pool is empty, it means all buffers are currently busy in the Windows kernel.
    // We drop the message to avoid blocking the server or leaking memory.
    if (!io) {
        std::cerr << "[Network] Send Pool empty! Message dropped: " << messageName << "\n";
        return;
    }

    // We copy the data from the source (span) to our fixed-size pool buffer.
    // std::min prevents buffer overflows if the message is larger than our pre-allocated size (1472).
    size_t copySize = (std::min)(buffer.size(), io->data.size());
    memcpy(io->data.data(), buffer.data(), copySize);

    // Update the WSABUF structure to tell Windows exactly how many bytes to send.
    io->wsabuf.buf = reinterpret_cast<CHAR*>(io->data.data());
    io->wsabuf.len = static_cast<ULONG>(copySize);
    io->target = clientAddr;

    // Reset the OVERLAPPED structure.
    io->ResetOverlapped();

    // Initiate the asynchronous send.
    // WSASendTo hands the buffer to the OS and returns immediately.
    // The actual sending happens in the background (hardware level).
    int ret = WSASendTo(
        udpSocket,
        &io->wsabuf,
        1,              // Sending 1 buffer
        nullptr,        // Bytes sent (ignored for async)
        0,              
        reinterpret_cast<sockaddr*>(&io->target),
        sizeof(io->target),
        &io->overlapped, // Connection to IOCP
        nullptr         // No completion routine
    );

    if (ret == SOCKET_ERROR) {
        int err = WSAGetLastError();
        // WSA_IO_PENDING is normal; anything else is a real failure.
        if (err != WSA_IO_PENDING) {
            std::cerr << "[UDP] WSASendTo failed with error: " << err << "\n";

            // If the call failed instantly, the buffer was never "queued" in the kernel.
            // We must return it to the pool manually, otherwise it's lost forever.
            std::lock_guard<std::mutex> lock(sendPoolMutex);
            availableSends.push(io);
        }
    }
}

// Sends data to multiple addresses, optionally excluding one address
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

#pragma endregion

#pragma region I/O Dispatching

void WindowsServer::PostAccept() {
    // Pre-create a "placeholder" socket for the next incoming connection.
    // AcceptEx requires us to provide the socket handle before the connection actually occurs.
    SOCKET clientSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);

    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "[TCP] Failed to create placeholder socket for AcceptEx\n";
        return;
    }

    // We pass 'tcpSocket' (the listening socket) to the constructor to identify the source.
    auto* io = new AcceptIoData(tcpSocket);
    io->acceptSocket = clientSocket;

    DWORD bytesReceived = 0;

    // This is a high-performance Windows-specific function that combines 
    // accepting a connection and (optionally) receiving the first block of data.
    BOOL result = AcceptEx(
        tcpSocket,           
        io->acceptSocket,    
        io->addressBuffer,   
        0,                   // Number of bytes of data to receive (0 = just accept connection)
        sizeof(sockaddr_in) + 16, // Reserved space for local address info
        sizeof(sockaddr_in) + 16, // Reserved space for remote address info
        &bytesReceived,      // Receives data length immediately (if any)
        &io->overlapped      // The link to our IOCP
    );

    // Check for immediate completion or pending status.
    if (!result) {
        int err = WSAGetLastError();
        // Windows will notify our IOCP Poll() when someone connects.
        if (err != ERROR_IO_PENDING) {
            std::cerr << "[TCP] AcceptEx failed with error: " << err << "\n";
            closesocket(clientSocket);
            delete io;
        }
    }
}

void WindowsServer::PostReceive(BaseIo* baseIo)
{
    // The switch acts as a traffic controller, directing the pointer to 
    // the correct WinSock function based on the operation type.
    switch (baseIo->opType)
    {
    case IO_OPERATION::RECEIVE_UDP:
    {
        // Cast the base pointer back to the UDP-specific structure
        auto* io = static_cast<ReceiveUDPIoData*>(baseIo);
        pendingReceives++;
        // Reset the address length and the OVERLAPPED structure
        // This is mandatory before every new asynchronous call to avoid kernel errors
        io->clientAddrLen = sizeof(io->clientAddr);
        io->ResetOverlapped();

        DWORD flags = 0;

        // Register an asynchronous receive request for UDP (connectionless)
        // Windows will fill 'io->clientAddr' with the sender's info automatically
        int ret = WSARecvFrom(
            udpSocket,
            &io->wsabuf,
            1,              // Number of WSABUF structures
            nullptr,        // Number of bytes received (output only if sync)
            &flags,
            reinterpret_cast<sockaddr*>(&io->clientAddr),
            &io->clientAddrLen,
            &io->overlapped, // The link to the Completion Port
            nullptr         // No completion routine (We use IOCP instead)
        );

        // Check if the operation was successfully initiated
        if (ret == SOCKET_ERROR)
        {
            int err = WSAGetLastError();
            // WSA_IO_PENDING is the expected "success" code for async I/O
            if (err != WSA_IO_PENDING)
            {
                pendingReceives--;
                std::cerr << "[UDP] WSARecvFrom failed: " << err << "\n";
            }
                
        }
        break;
    }

    case IO_OPERATION::RECEIVE_TCP:
    {
        // Cast the base pointer back to the TCP-specific structure
        auto* io = static_cast<ReceiveTCPIoData*>(baseIo);

        // Prepare the OVERLAPPED structure for a new operation
        io->ResetOverlapped();

        DWORD flags = 0;

        // Register an asynchronous receive request for an established TCP connection
        int ret = WSARecv(
            io->socket,
            &io->wsabuf,
            1,
            nullptr,
            &flags,
            &io->overlapped,
            nullptr
        );

        if (ret == SOCKET_ERROR)
        {
            int err = WSAGetLastError();
            if (err != WSA_IO_PENDING)
                std::cerr << "[TCP] WSARecv failed: " << err << "\n";
        }
        break;
    }

    default:
        // If we land here, a logic error occurred
        break;
    }
}
#pragma endregion

void WindowsServer::Poll()
{
    DWORD bytesTransferred = 0;
    ULONG_PTR completionKey = 0;
    OVERLAPPED* overlapped = nullptr;

    // Block the thread until an I/O operation completes on the Completion Port
    BOOL success = GetQueuedCompletionStatus(
        completionPort, &bytesTransferred, &completionKey, &overlapped, INFINITE
    );

    // If overlapped is null, it means an unrecoverable error occurred in the IOCP itself
    if (!overlapped) return;

    // Magic Macro: Retrieve the pointer to the base structure by calculating 
    // the offset of the 'overlapped' member within the BaseIo struct
    BaseIo* base = CONTAINING_RECORD(overlapped, BaseIo, overlapped);

    switch (base->opType)
    {
    case IO_OPERATION::RECEIVE_UDP:
    {
        ReceiveUDPIoData* ioData = static_cast<ReceiveUDPIoData*>(base);

  /*      std::cout << "[UDP] Received " << bytesTransferred << std::endl;*/
        // Success and bytesTransferred > 0 means we actually received data
        if (success && bytesTransferred > 0)
            EventManager::processMessage(ioData->buffer, ioData->clientAddr);
        pendingReceives--;
        std::cout << "Buffers currently held by Windows: " << pendingReceives.load() << std::endl;

         Sleep(1);
        // Recycle the buffer: Immediately repost the receive request to the Windows Kernel
        // so we don't miss the next incoming UDP packet
        PostReceive(ioData);
        break;
    }

    case IO_OPERATION::SEND_UDP:
    {
        SendUDPIoData* io = static_cast<SendUDPIoData*>(base);

        // The send operation is complete. Return the buffer to the pool so it can be reused
        // for future outgoing messages. Locked to ensure thread-safety.
        std::lock_guard<std::mutex> lock(sendPoolMutex);
        availableSends.push(io);
        break;
    }

    case IO_OPERATION::ACCEPT_TCP:
    {
        AcceptIoData* acceptData = static_cast<AcceptIoData*>(base);

        if (success) {
            // Extract the local and remote sockaddr structures from the binary buffer filled by AcceptEx
            sockaddr_in* localAddr = nullptr;
            int localAddrLen = 0;
            sockaddr_in* remoteAddr = nullptr;
            int remoteAddrLen = 0;

            GetAcceptExSockaddrs(
                acceptData->addressBuffer,
                0, // Bytes received (usually 0 if we only accept connection without initial data)
                sizeof(sockaddr_in) + 16,
                sizeof(sockaddr_in) + 16,
                (LPSOCKADDR*)&localAddr, &localAddrLen,
                (LPSOCKADDR*)&remoteAddr, &remoteAddrLen
            );

            // The new client socket must be associated with the IOCP completion port
            CreateIoCompletionPort((HANDLE)acceptData->acceptSocket, completionPort, 0, 0);

            // Transition from "Accepting" to "Receiving": 
            // Create a dedicated TCP receive context for this specific client
            ReceiveTCPIoData* tcpRev = new ReceiveTCPIoData(acceptData->acceptSocket, *remoteAddr);

            // Prime the first receive operation for this new TCP connection
            PostReceive(tcpRev);

            std::cout << "[TCP] Go Server connected! Remote IP: " << std::endl;
        }

        // Repost AcceptEx to the listen socket so we can handle the next incoming connection
        PostAccept();

        // Clean up the temporary Accept context
        delete acceptData;
        break;
    }

    case IO_OPERATION::RECEIVE_TCP:
    {
        ReceiveTCPIoData* ioData = static_cast<ReceiveTCPIoData*>(base);

        if (success && bytesTransferred > 0) {
            EventManager::processMessage(ioData->buffer, ioData->clientAddr);

            // Re-prime the TCP receive: wait for more data on the same socket
            PostReceive(ioData);
        }
        else {
            // bytesTransferred == 0 or !success indicates the remote client (Go Server) disconnected
            std::cout << "[TCP] Connection closed by remote host." << std::endl;
            closesocket(ioData->socket);
            delete ioData; // Cleanup resources for this session
        }
        break;
    }
    }
}