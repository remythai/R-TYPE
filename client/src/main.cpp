#include <iostream>
#include "./core/Core.hpp"
#include "macros.hpp"

static void display_help(void)
{
    std::cout << "USAGE: ./r-type_client\n";
}

static int check_args(int argc, char **argv)
{
    if (argc != NB_ARGS || argv[0] == nullptr ||
        argv[1] == nullptr || argv[2] == nullptr ||
        argv[3] == nullptr || argv[4] == nullptr) {
            display_help();
            return EPITECH_FAILURE;
        }
    if ((std::string(argv[1]) != "-p" && std::string(argv[1]) != "-h") ||
        (std::string(argv[3]) != "-p" && std::string(argv[3]) != "-h")) {
        display_help();
        return EPITECH_FAILURE;
    }
    return EPITECH_SUCCESS;
}

int main(int argc, char **argv)
{
    if (check_args(argc, argv) == 84)
        return EPITECH_FAILURE;
    return execute_rtypeClient(argv);
}
