/*
** EPITECH PROJECT, 2025
** r-type-mirror
** File description:
** NetworkServer.cpp
*/

#include "NetworkServer.hpp"
#include <thread>
#include <iostream>

NetworkServer::NetworkServer(unsigned short port)
    : _acceptor(_ioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)), _running(false)
    {
        _clientMessagesGame = {
        {"UP",    [this]() { broadcast("Player moved up"); }},
        {"DOWN",  [this]() { broadcast("Player moved down"); }},
        {"LEFT",  [this]() { broadcast("Player moved left"); }},
        {"RIGHT", [this]() { broadcast("Player moved right"); }},
        {"SPACE", [this]() { broadcast("Player shot"); }},
        {"ESC",   [this]() { broadcast("Pause menu opened"); }},
        };

        _clientMessagesMenu = {
            {"ENTER", [this]() { broadcast("Menu: validate choice"); }},
            {"A",     [this]() { broadcast("Menu: pressed A"); }},
            {"B",     [this]() { broadcast("Menu: pressed B"); }},
            {"C",     [this]() { broadcast("Menu: pressed C"); }},
            {"D",     [this]() { broadcast("Menu: pressed D"); }},
            {"E",     [this]() { broadcast("Menu: pressed E"); }},
            {"F",     [this]() { broadcast("Menu: pressed F"); }},
            {"G",     [this]() { broadcast("Menu: pressed G"); }},
            {"H",     [this]() { broadcast("Menu: pressed H"); }},
            {"I",     [this]() { broadcast("Menu: pressed I"); }},
            {"J",     [this]() { broadcast("Menu: pressed J"); }},
            {"K",     [this]() { broadcast("Menu: pressed K"); }},
            {"L",     [this]() { broadcast("Menu: pressed L"); }},
            {"M",     [this]() { broadcast("Menu: pressed M"); }},
            {"N",     [this]() { broadcast("Menu: pressed N"); }},
            {"O",     [this]() { broadcast("Menu: pressed O"); }},
            {"P",     [this]() { broadcast("Menu: pressed P"); }},
            {"Q",     [this]() { broadcast("Menu: pressed Q"); }},
            {"R",     [this]() { broadcast("Menu: pressed R"); }},
            {"S",     [this]() { broadcast("Menu: pressed S"); }},
            {"T",     [this]() { broadcast("Menu: pressed T"); }},
            {"U",     [this]() { broadcast("Menu: pressed U"); }},
            {"V",     [this]() { broadcast("Menu: pressed V"); }},
            {"W",     [this]() { broadcast("Menu: pressed W"); }},
            {"X",     [this]() { broadcast("Menu: pressed X"); }},
            {"Y",     [this]() { broadcast("Menu: pressed Y"); }},
            {"Z",     [this]() { broadcast("Menu: pressed Z"); }},
        };
    }

NetworkServer::~NetworkServer() {
    stop();
}

void NetworkServer::run()
{
    _running = true;
    doAccept();

    std::cout << "Server running..." << std::endl;
    _ioContext.run();
}

void NetworkServer::stop()
{
    _running = false;
    _ioContext.stop();

    std::lock_guard<std::mutex> lock(_clientsMutex);
    for (auto& client : _clients)
        if (client->is_open())
            client->close();
    _clients.clear();
    std::cout << "Server stopped." << std::endl;
}

void NetworkServer::doAccept()
{
    auto client = std::make_shared<asio::ip::tcp::socket>(_ioContext);
    _acceptor.async_accept(*client, [this, client](std::error_code ec)
    {
        if (!ec) {
            int clientId = _nextClientId++;
            {
                std::lock_guard<std::mutex> lock(_clientsMutex);
                _clients.push_back(client);
                _clientMap[clientId] = client;
                std::cout << "Client " << clientId
                    << " connected, Total clients: " << _clients.size() << std::endl;
            }

            std::thread(&NetworkServer::handleClient, this, client, clientId).detach();
        } else
            std::cerr << "Accept error: " << ec.message() << std::endl;

        if (_running)
            doAccept();
    });
}

void NetworkServer::handleClient(std::shared_ptr<asio::ip::tcp::socket> client, int clientId)
{
    try {
        asio::streambuf buf;
        while (_running && client->is_open()) {
            asio::read_until(*client, buf, '\n');
            std::istream is(&buf);
            std::string line;
            std::getline(is, line);

            if (!line.empty()) {
                std::cout << "Received from client " << clientId << ": " << line << std::endl;
                handleClientMessages(line);
            }
        }
    } catch (std::exception& e) {
        std::cerr << "Client " << clientId << " error: " << e.what() << std::endl;
    }

    std::lock_guard<std::mutex> lock(_clientsMutex);
    auto it = std::find(_clients.begin(), _clients.end(), client);
    if (it != _clients.end()) {
        _clients.erase(it);
        _clientMap.erase(clientId);
        std::cout << "Client " << clientId << " disconnected! Total clients: " << _clients.size() << std::endl;
    }
}

void NetworkServer::handleClientMessages(const std::string &message)
{
    auto itGame = _clientMessagesGame.find(message);
    if (itGame != _clientMessagesGame.end()) {
        itGame->second();
        return;
    }

    auto itMenu = _clientMessagesMenu.find(message);
    if (itMenu != _clientMessagesMenu.end()) {
        itMenu->second();
        return;
    }

    broadcast("Unknown command: " + message);
};

void NetworkServer::broadcast(const std::string& message)
{
    std::lock_guard<std::mutex> lock(_clientsMutex);
    for (auto& client : _clients)
        if (client->is_open())
            asio::write(*client, asio::buffer(message + "\n"));
}
