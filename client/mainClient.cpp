/*
** EPITECH PROJECT, 2025
** r-type-mirror
** File description:
** mainClient.cpp
*/

#include "src/network/NetworkClient.hpp"
#include <iostream>
#include <string>

int main() {
    NetworkClient client("127.0.0.1", 8080);
    client.startReceiving();

    std::string username;
    std::cout << "Enter username: ";
    std::getline(std::cin, username);
    client.sendJoin(username);

    int packetCounter = 1;
    while (true) {
        std::string cmd;
        std::cout << ">>> ";
        std::getline(std::cin, cmd);

        if (cmd == "quit") break;
        else if (cmd == "ping") client.sendPing(packetCounter++);
        else if (!cmd.empty()) client.sendInput(1, static_cast<uint8_t>(cmd[0]), 1);
    }

    return 0;
}
