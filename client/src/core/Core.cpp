/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** Core.cpp
*/

#include "Core.hpp"
#include "../graphics/Window.hpp"
#include <iostream>
#include <memory>

CLIENT::Core::Core(char **argv)
{
    for (int i = 1; i <= 3; ++i) {
        std::string arg = argv[i];
        if (arg == "-p") {
            try {
                _port = std::stoi(argv[i + 1]);
            } catch (const std::invalid_argument &) {
                throw CoreError("Invalid port: not a number");
            } catch (const std::out_of_range &) {
                throw CoreError("Invalid port: number out of range");
            }
        } else if (arg == "-h")
            _hostname = argv[i + 1];
    }

    if (_port == 0 || _hostname.empty())
        throw CoreError("Missing -p or -h argument");
    
    _networkClient = std::make_unique<NetworkClient>(_hostname, _port);
    std::cout << "aujourd'hui je suis\n";
}

CLIENT::Core::~Core()
{
    std::cout << "je t'ai regardé dormir\n";
}

void CLIENT::Core::run()
{

    CLIENT::Window window("R-Type Client", WINDOW_WIDTH, WINDOW_HEIGHT);

    while (window.isOpen()) {
        window.pollEvents();

        window.clear();

        window.display();
    }
}

int execute_rtypeClient(char **argv)
{
    try {
        CLIENT::Core core(argv);
        core.run();
    } catch (const CLIENT::Core::CoreError &error) {
        std::cerr << "Core error: " << error.what() << std::endl;
        return 1;
    } catch (const CLIENT::NetworkClient::NetworkError &error) {
        std::cerr << "Network error: " << error.what() << std::endl;
        return 1;
    }

    return 0;
}