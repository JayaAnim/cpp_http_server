#ifndef SERVER_H
#define SERVER_H
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define SERVER_PORT 8080
#define SERVER_BUFF_SIZE 32000

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
    int server_fd, client_fd;
    struct sockaddr_in addr;
    int addr_len;
public:
    Server();
    void start_server(); /* starts server by acceping one client connection at a time */
    http_req_t parse_req(char*); /* parses request buffer into http_req_t */
    bool handle_req(int, http_req_t*); /* routes individual request and sends appropriate response, returns true if successful */
    bool _send_file_res(int, const char*, const char*); /* sends file response and returns true if successful */
    bool _send_text_res(int, const char*); /* sends text response with the content res_text and returns true if successful */
    const char* _get_ft(const char*); /* returns content-header type for file */
    ~Server();    
};

#endif
