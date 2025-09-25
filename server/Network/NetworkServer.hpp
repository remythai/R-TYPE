/*
** EPITECH PROJECT, 2025
** r-type-mirror
** File description:
** NetworkServer.hpp
*/

#pragma once
#include <asio.hpp>
#include <memory>
#include <vector>
#include <mutex>
#include <iostream>

class NetworkServer {
public:
    NetworkServer(unsigned short port);
    ~NetworkServer();

    void run();
    void stop();

    void broadcast(const std::string& message);

private:
    void doAccept();
    void handleClient(std::shared_ptr<asio::ip::tcp::socket> client);

    asio::io_context _ioContext;
    asio::ip::tcp::acceptor _acceptor;

    std::vector<std::shared_ptr<asio::ip::tcp::socket>> _clients;
    std::mutex _clientsMutex;

    bool _running;
};
