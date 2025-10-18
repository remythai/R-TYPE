#include <iostream>
#include <string>
#include "./core/Core.hpp"
#include "macros.hpp"

static void display_help(void)
{
    std::cout << "USAGE: ./r-type_client [-editor] -p PORT -h HOSTNAME\n";
    std::cout << "Options:\n";
    std::cout << "  -editor              Launch the map editor instead of the game\n";
    std::cout << "  -p PORT              Server port\n";
    std::cout << "  -h HOSTNAME          Server hostname\n";
}

static int check_args(int argc, char **argv, bool &isEditor)
{
    isEditor = false;
    int requiredArgs = NB_ARGS;
    
    // Check if -editor flag is present
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-editor") {
            isEditor = true;
            requiredArgs++;
            break;
        }
    }
    
    // Need at least 5 args (program name + -p PORT -h HOSTNAME)
    // or 6 args if -editor is present
    if (argc < requiredArgs || argv[0] == nullptr) {
        display_help();
        return EPITECH_FAILURE;
    }
    
    // Check that -p and -h are present
    bool hasPort = false, hasHostname = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-p" && i + 1 < argc) hasPort = true;
        if (std::string(argv[i]) == "-h" && i + 1 < argc) hasHostname = true;
    }
    
    if (!hasPort || !hasHostname) {
        display_help();
        return EPITECH_FAILURE;
    }
    
    return EPITECH_SUCCESS;
}

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