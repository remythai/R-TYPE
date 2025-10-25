/*
** EPITECH PROJECT, 2025
** r-type-mirror
** File description:
** main.cpp
*/

#include <cstring>
#include <iostream>
#include <string>
#include "network/NetworkServer.hpp"

static void display_help(void)
{
    std::cout << "USAGE: ./r-type_server -p [port] -h [host] -g [game]\n";
}

static int check_args(int argc, char **argv, unsigned short &port, std::string &hostname, std::string &game)
{
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = static_cast<unsigned short>(std::stoi(argv[i + 1]));
            i++;
        }
        if (strcmp(argv[i], "-h") == 0 && i + 1 < argc) {
            hostname = argv[i + 1];
            i++;
        }
        if (strcmp(argv[i], "-g") == 0 && i + 1 < argc) {
            game = argv[i + 1];
            i++;
        }
    }
    if (hostname == std::string("") || port == 0 || game != "flappyByte" && game != "RType") {
        display_help();
        return 84;
    }
    return 0;
}

int main(int argc, char **argv)
{
    unsigned short port(0);
    std::string hostname;
    std::string game;

    if (check_args(argc, argv, port, hostname, game) == 84)
        return 84;
    rtype::NetworkServer server(port, hostname, game);
    server.run();
    return 0;
}
