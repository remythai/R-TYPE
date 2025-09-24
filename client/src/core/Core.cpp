/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** Core.cpp
*/

#include "Core.hpp"
#include "../graphics/Window.hpp"
#include <iostream>

CLIENT::Core::Core()
{
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
        CLIENT::Core core;
        core.run();
    } catch (const CLIENT::Core::CoreError &error) {
        std::cerr << "Core error: " << error.what() << std::endl;
        return 1;
    }
    return 0;
}