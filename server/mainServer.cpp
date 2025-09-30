#include "Network/NetworkServer.hpp"

int main()
{
    NetworkServer server(8080);
    server.run();
    return 0;
}