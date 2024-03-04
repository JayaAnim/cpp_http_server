/*
 * File defines Server class which is used to create http server which includes debugging output
 * Authors: Chase Lamkin
 * Written March 2nd
 * Course COP4635
 */
#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>
#include "globals.h"

class Client {
private:
    //Address, file descriptor (client) and address length, for server configurations
    int client_fd;
    struct sockaddr_in addr;
    int addr_len;
public:
    Client(); /* configures client connection */
    Client(char*, char*); /* configures client connection based on input */
    void start_server(); /* creates client-server socket connection and allows user to make 1 request before exiting */
    void req_file(char*); /* same as start_server but only allows GET request for a resource */

    http_req_t* create_req(); /* creates request object (same used by httpServer) by asking user for information on request to be made */
    void disp_txt(char*, int); /* prints char buffer based on size input */
    void disp_base2(char*, int); /* prints char buffer as base2 based on size input */

    char* _get_body_ptr(char*); /* returns ptr to start of http body (i.e /r/n/r/n[pointer here]) */
    int _get_content_len(char*); /* returns value for Content-Length in http response */
    char* _get_content_type(char*); /* returns content type in http response */
    char* _gen_req_str(http_req_t*);  /* generates request string to be send via send() based on http_req_t struct */
    void _free_req(http_req_t*); /* frees http_req_t strcut */
    void _clean_newline(char*); /* replaces user input newline with \0 */

    ~Client();    
};

#endif
