/*
** EPITECH PROJECT, 2025
** r-type-mirror
** File description:
** mainServer.cpp
*/

#include "src/network/NetworkServer.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>

int main(int argc, char const *argv[])
{
    unsigned short port = 8080;
    std::string hostname = "127.0.0.1";

    try {
        for (int i = 1; i < argc; ++i) {
            if (std::strcmp(argv[i], "-p") == 0) {
                if (i + 1 >= argc)
                    throw std::invalid_argument("Missing value for -p (port) flag");
                int portValue = std::stoi(argv[++i]);
                if (portValue < 1 || portValue > 65535)
                    throw std::out_of_range("Port must be between 1 and 65535");
                port = static_cast<unsigned short>(portValue);
            } else if (std::strcmp(argv[i], "-h") == 0) {
                if (i + 1 >= argc)
                    throw std::invalid_argument("Missing value for -h (hostname) flag");
                hostname = argv[++i];
                if (hostname.empty())
                    throw std::invalid_argument("Hostname cannot be empty");
            } else {
                throw std::invalid_argument(std::string("Unknown flag: ") + argv[i]);
            }
        }

        rtype::NetworkServer server(port, hostname);
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 84;
    }
    return 0;
}
