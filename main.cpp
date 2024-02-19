// Server side C program to demonstrate HTTP Server programming
#include <stdio.h>
#include "server.h"


int main()
{
    Server* server = new Server();
    server->start_server();
    delete server;
    return 0;
}
