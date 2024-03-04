/*
 * File defines Server class which is used to create http server which includes debugging output
 * Authors: Chase Lamkin
 * Written March 2nd
 * Course COP4635
 */
#include <string.h>

//Port for server to listen on and buffer size of incoming requests
#define SERVER_PORT 60002
#define SERVER_BUFF_SIZE 75000
#define RETRY_INTERVAL 10

//Struct for client requests
typedef struct http_req {
    char* req_type;
    char* URL;
    char* resource_path;
    const char* resource_type;
    char* connection_type;
    char* content_length;
    char* content_body_ptr;
} http_req_t;

const char* get_ft(const char*);
