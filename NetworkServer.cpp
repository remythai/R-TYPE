/*
** EPITECH PROJECT, 2025
** r-type-mirror
** File description:
** NetworkServer.cpp
*/

#include "NetworkServer.hpp"
#include <thread>

NetworkServer::NetworkServer(unsigned short port)
    : _acceptor(_ioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)), _running(false) {}

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
            {
                std::lock_guard<std::mutex> lock(_clientsMutex);
                _clients.push_back(client);
                std::cout << "Client connected! Total clients: " << _clients.size() << std::endl;
            }

            std::thread(&NetworkServer::handleClient, this, client).detach();
        } else
            std::cerr << "Accept error: " << ec.message() << std::endl;

        if (_running)
            doAccept();
    });
}

void NetworkServer::handleClient(std::shared_ptr<asio::ip::tcp::socket> client)
{
    try {
        asio::streambuf buf;
        while (_running && client->is_open()) {
            asio::read_until(*client, buf, '\n');
            std::istream is(&buf);
            std::string line;
            std::getline(is, line);

            if (!line.empty())
                std::cout << "Received from client: " << line << std::endl;
        }
    } catch (std::exception& e) {
        std::cerr << "Client error: " << e.what() << std::endl;
    }

    std::lock_guard<std::mutex> lock(_clientsMutex);
    auto it = std::find(_clients.begin(), _clients.end(), client);
    if (it != _clients.end()) {
        _clients.erase(it);
        std::cout << "Client disconnected! Total clients: " << _clients.size() << std::endl;
    }
}

void NetworkServer::broadcast(const std::string& message)
{
    std::lock_guard<std::mutex> lock(_clientsMutex);
    for (auto& client : _clients)
        if (client->is_open())
            asio::write(*client, asio::buffer(message + "\n"));
}
