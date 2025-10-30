/**
 * @file main.cpp
 * @brief Entry point for the R-Type client application.
 *
 * This program launches either the R-Type game client or the map editor
 * depending on command-line arguments. It validates required parameters
 * such as server hostname and port.
 */

#include <iostream>
#include <string>

#include "./core/Core.hpp"
#include "macros.hpp"

/**
 * @brief Displays usage instructions for the program.
 *
 * @details
 * Prints the correct command-line syntax and the available options:
 * -editor, -p PORT, -h HOSTNAME
 */
static void display_help(void)
{
    std::cout << "USAGE: ./r-type_client [-editor] -p PORT -h HOSTNAME\n";
    std::cout << "Options:\n";
    std::cout << "  -editor              Launch the map editor instead of the game\n";
    std::cout << "  -p PORT              Server port\n";
    std::cout << "  -h HOSTNAME          Server hostname\n";
}

/**
 * @brief Checks command-line arguments for validity.
 *
 * @param argc Argument count.
 * @param argv Argument vector.
 * @param[out] isEditor Set to true if -editor flag is provided, false otherwise.
 * @return EPITECH_SUCCESS if arguments are valid, EPITECH_FAILURE otherwise.
 *
 * @details
 * - Ensures that the required number of arguments is provided.
 * - Detects the optional -editor flag.
 * - Verifies that both server port (-p) and hostname (-h) are provided.
 */
static int check_args(int argc, char **argv, bool &isEditor)
{
    isEditor = false;
    int requiredArgs = NB_ARGS;

    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-editor") {
            isEditor = true;
            requiredArgs++;
            break;
        }
    }

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
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 * @return EPITECH_SUCCESS on successful execution, EPITECH_FAILURE otherwise.
 *
 * @details
 * - Validates command-line arguments using check_args().
 * - Launches the map editor if -editor flag is provided.
 * - Otherwise, executes the R-Type client using execute_rtypeClient().
 * - Catches and reports exceptions thrown during execution.
 */
int main(int argc, char **argv)
{
    bool isEditor = false;

    if (check_args(argc, argv, isEditor) == EPITECH_FAILURE)
        return EPITECH_FAILURE;

    try {
        if (isEditor) {
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
