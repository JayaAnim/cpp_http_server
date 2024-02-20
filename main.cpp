#include "httpServer.h"


int main()
{
    //Create the server
    Server* server = new Server();
    //Start the server (accept requests in a loop)
    server->start_server();
    //Delete the server after done
    delete server;
    return 0;
}
