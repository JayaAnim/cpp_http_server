#include "httpServer.h"


int main(int argc, char** argv)
{
    //Create the server
    Server* server;
    if (argc == 3) {
        server = new Server(argv[1], argv[2]);
    }
    else {
        server = new Server();
    }

    //Start the server (accept requests in a loop)
    server->start_server();
    //Delete the server after done
    delete server;
    return 0;
}
