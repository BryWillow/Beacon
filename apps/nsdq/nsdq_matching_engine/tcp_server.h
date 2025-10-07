// ---------------------------------------------------------------------------
// File        : tcp_server.h
// @file        tcp_server.h
// Project     : Beacon
// Component   : Matching Engine
// Description : TCP server for order submissions and updates
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

#include <string>
#include <functional>
#include <thread>
#include <atomic>

namespace beacon::nsdq::matching_engine
{

    /**
     * @brief TCP server handling client connections for the matching engine
     */
    class TcpServer
    {
    public:
        using ClientMessageCallback = std::function<void(const std::string &)>;

        explicit TcpServer(unsigned short port, ClientMessageCallback callback);
        ~TcpServer();

        TcpServer(const TcpServer &) = delete;
        TcpServer &operator=(const TcpServer &) = delete;

        TcpServer(TcpServer &&) = delete;
        TcpServer &operator=(TcpServer &&) = delete;

        void start();
        void stop();

    private:
        void listenForConnectionRequests();
        void client_loop(int client_socket);

        unsigned short _port;
        ClientMessageCallback _callback;

        std::thread _accept_thread;
        std::atomic<bool> _running{false};
    };

} // namespace beacon::nsdq::matching_engine
