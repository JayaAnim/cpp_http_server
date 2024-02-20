// Server side C program to demonstrate HTTP Server programming
#include "httpServer.h"


int main()
{
    Server* server = new Server();
    server->start_server();
    delete server;
    return 0;
}
