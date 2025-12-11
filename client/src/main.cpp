/*
** EPITECH PROJECT, 2025
** r-type-mirror
** File description:
** main.cpp - VERSION AVEC MODE TEST
*/

#include <iostream>
#include <string>

#include "./core/Core.hpp"
#include "macros.hpp"

/**
 * @brief Displays usage instructions for the program.
 */
static void display_help(void)
{
    std::cout << "USAGE: ./r-type_client [-test [-map FILE]] [-editor] -p PORT -h HOSTNAME\n";
    std::cout << "Options:\n";
    std::cout << "  -test                Launch in standalone test mode (no server)\n";
    std::cout << "  -map FILE            Map file to load in test mode (default: map_level1.json)\n";
    std::cout << "  -editor              Launch the map editor instead of the game\n";
    std::cout << "  -p PORT              Server port\n";
    std::cout << "  -h HOSTNAME          Server hostname\n";
    std::cout << "\nExamples:\n";
    std::cout << "  ./r-type_client -test\n";
    std::cout << "  ./r-type_client -test -map stress_10k.json\n";
    std::cout << "  ./r-type_client -h 127.0.0.1 -p 8080\n";
    std::cout << "  ./r-type_client -editor\n";
}

/**
 * @brief Checks command-line arguments for validity.
 */
static int check_args(int argc, char **argv, bool &isEditor, bool &isTest)
{
    isEditor = false;
    isTest = false;
    int requiredArgs = NB_ARGS;

    // Vérifier les flags spéciaux en premier
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-test") {
            isTest = true;
            return EPITECH_SUCCESS; // Mode test n'a pas besoin d'hostname/port
        }
        if (std::string(argv[i]) == "-editor") {
            isEditor = true;
            requiredArgs++;
            break;
        }
    }

    // Validation normale uniquement si pas en mode test
    if (argc < requiredArgs || argv[0] == nullptr) {
        display_help();
        return EPITECH_FAILURE;
    }

    bool hasPort = false;
    bool hasHostname = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-p" && i + 1 < argc)
            hasPort = true;
        if (std::string(argv[i]) == "-h" && i + 1 < argc)
            hasHostname = true;
    }

    if (!hasPort || !hasHostname) {
        display_help();
        return EPITECH_FAILURE;
    }

    return EPITECH_SUCCESS;
}

/**
 * @brief Program entry point.
 */
int main(int argc, char **argv)
{
    bool isEditor = false;
    bool isTest = false;

    if (check_args(argc, argv, isEditor, isTest) == EPITECH_FAILURE)
        return EPITECH_FAILURE;

    try {
        if (isTest) {
            std::cout << "=================================================\n";
            std::cout << "   R-TYPE CLIENT - STANDALONE TEST MODE\n";
            std::cout << "=================================================\n";
            CLIENT::Core core(argv, true); // true = test mode
            core.run();
        } else if (isEditor) {
            CLIENT::Core::launchMapEditor();
        } else {
            return execute_rtypeClient(argv);
        }
    } catch (const std::exception &error) {
        std::cerr << "Error: " << error.what() << std::endl;
        return EPITECH_FAILURE;
    }

    return EPITECH_SUCCESS;
}

/**
 * @brief Executes the R-Type client in normal network mode.
 */
int execute_rtypeClient(char** argv)
{
    try {
        CLIENT::Core core(argv, false); // false = mode normal avec réseau
        core.run();
    } catch (const CLIENT::Core::CoreError& error) {
        std::cerr << "Core error: " << error.what() << std::endl;
        return 1;
    } catch (const std::exception& error) {
        std::cerr << "Unexpected error: " << error.what() << std::endl;
        return 1;
    }

    return 0;
}