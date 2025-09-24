/*
** EPITECH PROJECT, 2025
** r-type-mirror
** File description:
** server_main.cpp
*/

#include "NetworkServer.hpp"
#include <iostream>

int main() {
    try {
        NetworkServer server(12345);
        std::cout << "Server run on 12345..." << std::endl;
        server.run();
    } catch (std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
