// ---------------------------------------------------------------------------
// @file        tcp_server.h
// Project     : Beacon
// Component   : HFT Networking
// Description : Generic TCP server for low-latency pipelines
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

#include <atomic>
#include <functional>
#include <thread>
#include <vector>
#include <string>
#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

namespace beacon::hft::networking
{

    /**
     * @class TcpServer
     * @brief Generic TCP server handling multiple client connections.
     *
     * Hot-path notes:
     * - Accepts client connections asynchronously.
     * - Each client handled in a separate thread.
     * - Uses std::atomic for stop flag; safe for HFT low-latency pipelines.
     */
    class TcpServer
    {
    public:
        using ClientMessageCallback = std::function<void(const std::string &)>;

        /**
         * @brief Construct the TCP server.
         * @param port TCP port to listen on
         * @param callback Callback invoked on message from any client
         * @param backlog Maximum number of pending connections
         */
        TcpServer(unsigned short port, ClientMessageCallback callback, size_t backlog = 5)
            : _port(port), _callback(std::move(callback)), _backlog(backlog)
        {
        }

        ~TcpServer() { stop(); }

        // Non-copyable, non-movable
        TcpServer(const TcpServer &) = delete;
        TcpServer &operator=(const TcpServer &) = delete;
        TcpServer(TcpServer &&) = delete;
        TcpServer &operator=(TcpServer &&) = delete;

        /**
         * @brief Start listening and accepting client connections
         */
        void start()
        {
            _running = true;
            _accept_thread = std::thread(&TcpServer::listenLoop, this);
        }

        /**
         * @brief Stop the server and join all threads
         */
        void stop()
        {
            _running = false;

            if (_accept_thread.joinable())
                _accept_thread.join();

            for (auto &t : _client_threads)
            {
                if (t.joinable())
                    t.join();
            }
            _client_threads.clear();
        }

    private:
        void listenLoop()
        {
            int server_fd = socket(AF_INET, SOCK_STREAM, 0);
            if (server_fd < 0)
            {
                std::cerr << "TCP server: failed to create socket\n";
                return;
            }

            int enable = 1;
            setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

            sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = INADDR_ANY;
            addr.sin_port = htons(_port);

            if (bind(server_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
            {
                std::cerr << "TCP server: bind failed\n";
                close(server_fd);
                return;
            }

            if (listen(server_fd, static_cast<int>(_backlog)) < 0)
            {
                std::cerr << "TCP server: listen failed\n";
                close(server_fd);
                return;
            }

            while (_running)
            {
                int client_fd = accept(server_fd, nullptr, nullptr);
                if (client_fd >= 0)
                {
                    _client_threads.emplace_back(&TcpServer::clientLoop, this, client_fd);
                }
            }

            close(server_fd);
        }

        void clientLoop(int client_fd)
        {
            constexpr size_t BUFFER_SIZE = 1024;
            char buffer[BUFFER_SIZE];

            while (_running)
            {
                ssize_t bytes = read(client_fd, buffer, BUFFER_SIZE);
                if (bytes <= 0)
                    break;

                std::string message(buffer, static_cast<size_t>(bytes));
                _callback(message);
            }

            close(client_fd);
        }

        unsigned short _port;            ///< Listening TCP port
        size_t _backlog;                 ///< Max pending connections
        ClientMessageCallback _callback; ///< Hot-path callback for messages

        std::atomic<bool> _running{false};        ///< Stop flag
        std::thread _accept_thread;               ///< Thread for accepting connections
        std::vector<std::thread> _client_threads; ///< Threads for each client
    };

} // namespace beacon::hft::networking
