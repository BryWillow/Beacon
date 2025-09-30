// ---------------------------------------------------------------------------
// File        : tcp_server.cpp
// Project     : Beacon
// Component   : Matching Engine
// Description : TCP server implementation
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#include "tcp_server.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

namespace beacon::nsdq::matching_engine
{

    TcpServer::TcpServer(unsigned short port, ClientMessageCallback callback)
        : _port(port), _callback(std::move(callback)) {}

    TcpServer::~TcpServer()
    {
        stop();
    }

    void TcpServer::start()
    {
        _running = true;
        _accept_thread = std::thread(&TcpServer::accept_loop, this);
    }

    void TcpServer::stop()
    {
        _running = false;
        if (_accept_thread.joinable())
        {
            _accept_thread.join();
        }
    }

    void TcpServer::accept_loop()
    {
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0)
        {
            std::cerr << "Failed to create socket\n";
            return;
        }

        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(_port);

        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        if (bind(server_fd, reinterpret_cast<sockaddr *>(&address), sizeof(address)) < 0)
        {
            std::cerr << "Bind failed\n";
            close(server_fd);
            return;
        }

        if (listen(server_fd, 5) < 0)
        {
            std::cerr << "Listen failed\n";
            close(server_fd);
            return;
        }

        while (_running)
        {
            int client_socket = accept(server_fd, nullptr, nullptr);
            if (client_socket >= 0)
            {
                std::thread(&TcpServer::client_loop, this, client_socket).detach();
            }
        }

        close(server_fd);
    }

    void TcpServer::client_loop(int client_socket)
    {
        constexpr size_t buffer_size = 1024;
        char buffer[buffer_size];

        while (_running)
        {
            ssize_t bytes_read = read(client_socket, buffer, buffer_size);
            if (bytes_read <= 0)
            {
                break;
            }

            std::string message(buffer, static_cast<size_t>(bytes_read));
            _callback(message);
        }

        close(client_socket);
    }

} // namespace beacon::nsdq::matching_engine
