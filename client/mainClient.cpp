#include "Network/NetworkClient.hpp"
#include <iostream>

int main()
{
    NetworkClient client("127.0.0.1", 8080);

    while (true) {
        std::string input;
        std::cout << ">>> ";
        std::getline(std::cin, input);

        if (input == "quit")
            break;

        client.sendMessage(input + "\n");
        std::string response = client.receiveMessage();
        std::cout << "Message from server: " << response;
    }
    return 0;
}
