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
    std::cout << "USAGE: ./r-type_server -p [port] -h [host]\n";
}

static int check_args(
    int argc, char **argv, unsigned short &port, std::string &hostname)
{
    if (argc != 5) {
        display_help();
        return 84;
    }
    for (int i = 0; i < 5; i++) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = static_cast<unsigned short>(std::stoi(argv[i + 1]));
            i++;
        }
        if (strcmp(argv[i], "-h") == 0 && i + 1 < argc) {
            hostname = argv[i + 1];
            i++;
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    unsigned short port;
    std::string hostname;

    if (check_args(argc, argv, port, hostname) == 84)
        return 84;
    rtype::NetworkServer server(port, hostname);
    server.run();
    return 0;
}
