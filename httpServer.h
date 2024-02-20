#ifndef HTTPSERVER_H
#define HTTPSERVER_H
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

//Port for server to listen on and buffer size of incoming requests
#define SERVER_PORT 60002
#define SERVER_BUFF_SIZE 32000

//Struct to parse incoming requests
typedef struct http_req {
    char* req_type;
    char* URL;
    char* resource_path;
    const char* resource_type;
    char* connection_type;
    char* content_length;
    char* content_body_ptr;
} http_req_t;

class Server {
private:
    //Address, file descriptors (server and client) and address length, for server configurations
    int server_fd, client_fd;
    struct sockaddr_in addr;
    int addr_len;
public:
    Server(); /* configures server, will halt program if any configurations (listen and bind) fail, as they will prevent the server from working. So far, has only thrown issues if the port is busy */
    void start_server(); /* starts server by acceping one client connection at a time */
    http_req_t parse_req(char*); /* parses request buffer into http_req_t */
    bool handle_req(int, http_req_t*); /* routes individual request and sends appropriate response, returns true if successful */
    bool _send_file_res(int, const char*, const char*); /* sends file response and returns true if successful */
    bool _send_text_res(int, const char*); /* sends text response with the content res_text and returns true if successful */
    bool _not_found(int); /* sends 404 not found to client and returns true if successful */
    bool _intern_err(int client_fd); /* sends 500 internal server error to client and returns true if successful (server error does not mean the server will crash, I have never encountered it, it is just incase a header is malformed and doesn't parse properly) */
    const char* _get_ft(const char*); /* returns content-header type for file */
    ~Server();    
};

#endif
