/*
** EPITECH PROJECT, 2025
** r-type-mirror
** File description:
** NetworkServer.hpp
*/

#pragma once
#include <asio.hpp>
#include <atomic>
#include <memory>
#include <unordered_map>
#include <vector>
#include <mutex>

class NetworkServer {
public:
    NetworkServer(unsigned short port);
    ~NetworkServer();

    void run();
    void stop();

    void broadcast(const std::string& message);

    void handleClientMessages(const std::string &message);

private:
    void doAccept();
    void handleClient(std::shared_ptr<asio::ip::tcp::socket> client, int clientId);

    asio::io_context _ioContext;
    asio::ip::tcp::acceptor _acceptor;

    std::vector<std::shared_ptr<asio::ip::tcp::socket>> _clients;
    std::unordered_map<int, std::shared_ptr<asio::ip::tcp::socket>> _clientMap;
    std::mutex _clientsMutex;
    std::atomic<int> _nextClientId{1};

    std::unordered_map<std::string, std::function<void()>> _clientMessagesGame;
    std::unordered_map<std::string, std::function<void()>> _clientMessagesMenu;

    bool _running;
};
