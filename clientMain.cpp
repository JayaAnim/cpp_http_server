#include "httpClient.h"


int main(int argc, char** argv)
{
    //Create the server
    Client* client; 

    if (argc == 3 || argc == 4) {
        client = new Client(argv[1], argv[2]);
    }
    else {
        client = new Client();
    }

    //Start the server (accept requests in a loop)
    if (argc == 4) {
        client->req_file(argv[3]);
    }
    else {
        client->start_server();
    }
    //Delete the server after done
    delete client;
    return 0;
}
