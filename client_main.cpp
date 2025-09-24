/*
** EPITECH PROJECT, 2025
** r-type-mirror
** File description:
** client_main.cpp
*/

#include "NetworkClient.hpp"
#include <iostream>

int main() {
    try {
        NetworkClient client("127.0.0.1", 12345);
        std::cout << "Connecté au serveuur" << std::endl;

        client.sendMessage("Hello from client!");

        std::string msg;
        while (true) {
            msg = client.receiveMessage();
            std::cout << "Server: " << msg << std::endl;
        }

    } catch (std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
