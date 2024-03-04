/*
 * File defines Server class which is used to create http server which includes debugging output
 * Authors: Chase Lamkin
 * Written March 2nd
 * Course COP4635
 */
#include "httpClient.h"

Client::Client() {

    //Uses default PORT and 0.0.0.0 if constructor without args is called
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("[ERROR] could not create client socket");
        exit(EXIT_FAILURE);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    addr_len = sizeof(addr);

    memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));
}

Client::Client(char* addr_str, char* port_str) {

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("[ERROR] could not create client socket");
        exit(EXIT_FAILURE);
    }

    // Convert address and port strings to appropriate types
    struct in_addr ip_addr;
    if (inet_pton(AF_INET, addr_str, &ip_addr) <= 0) {
        perror("[ERROR] invalid address");
        exit(EXIT_FAILURE);
    }

    int i_port = atoi(port_str); // Convert port string to integer
    if (i_port <= 0 || i_port > 65535) {
        perror("[ERROR] invalid port");
        exit(EXIT_FAILURE);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(i_port);
    addr.sin_addr = ip_addr;

    addr_len = sizeof(addr);

    memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));
}

void Client::req_file(char* file_name) {
    fprintf(stdout, "\n\n+++++++++++++++++++++++++----- Client Started At IP %s And Port %d -----+++++++++++++++++++++++++\n\n", inet_ntoa(addr.sin_addr), SERVER_PORT);
    //If bytes_sent = 0, new connection will be established
    int bytes_sent = 0;

    fprintf(stdout, "====================----- Attempting New Connection -----====================\n\n");

    if (connect(client_fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        fprintf(stdout, "!!!Client failed to establish connection, ending connection\n\n");
        close(client_fd);
        return;
    }
    else {
        fprintf(stdout, "===--- Established client connection\n\n"); 
    }

    int bytes_recv;
    char server_buff[SERVER_BUFF_SIZE];

    memset(server_buff, 0, SERVER_BUFF_SIZE); 

    //Generate request string after asking user for input
    http_req_t* req = (http_req_t*)malloc(sizeof(http_req_t));
    req->URL = NULL;
    req->req_type = strdup("GET");
    req->resource_type = get_ft(file_name);
    req->resource_path = strdup(file_name);
    req->URL = (char*)malloc(sizeof(char) * (strlen(file_name) + 2));
    strcat(req->URL, "/");
    strcat(req->URL, file_name);
    req->content_length = strdup("0");
    req->connection_type = strdup("close");
    req->content_body_ptr = NULL;
    char* req_str = _gen_req_str(req);

    fprintf(stdout, "===--- Sending request to server, request is\n\n");
    fprintf(stdout, "%s\n\n", req_str);

    //Send request to server
    bytes_sent = send(client_fd, req_str, strlen(req_str), 0);

    //Free request string and request object after sending them to server
    free(req_str);
    _free_req(req);
    
    //Check on status of sent req
    if (bytes_sent <= 0) {
        fprintf(stdout, "!!!Client failed to send bytes, ending connection\n\n");
        close(client_fd);
        return;
    }
    else if (bytes_sent != (int)strlen(req_str)) {
        fprintf(stdout, "!!!Client failed to send full request\n\n");
    }
    else {
        fprintf(stdout, "===--- Request sent successfully to server\n\n");
    }


    //Get server response
    bytes_recv = recv(client_fd, server_buff, sizeof(server_buff), 0);

    //adds \0 at start of body so header can now be printed and its length found
    char* body_ptr = _get_body_ptr(server_buff);
    int hdr_len = strlen(server_buff) + 1;

    fprintf(stdout, "===--- Server response header is\n\n");
    fprintf(stdout, "%s\n\n", server_buff);

    //adds \0 at end of content type so header can no longer be printed
    int content_len = _get_content_len(server_buff);
    char* content_type = _get_content_type(server_buff);


    if (strcmp(content_type, "text/html") == 0 || strcmp(content_type, "text/plain") == 0) {
        fprintf(stdout, "===--- Server response body (ASCII format) is\n\n");
        disp_txt(body_ptr, strlen(body_ptr)); 

        //Total received bytes from http body, used to compare against expected
        int total_recv = strlen(body_ptr);

        //While bytes are still missing from body request them (or until end of stream)
        while (total_recv < content_len) {

            memset(server_buff, 0, SERVER_BUFF_SIZE); 
            total_recv += recv(client_fd, server_buff, sizeof(server_buff), 0);

            disp_txt(body_ptr, strlen(body_ptr)); 
            
        }
            
        fprintf(stdout, "\n\n");

    }
    else {
        fprintf(stdout, "===--- Server response body (base2 format) is\n\n");

        //Total received bytes from body (does not use strlen since content is binary)
        int total_recv = bytes_recv - hdr_len;

        //Print binary data based on calculated body length (not from strlen due to binary)
        disp_base2(body_ptr, total_recv);

        //While bytes are still missing from body request them (or until end of stream)
        while (total_recv < content_len) {

            memset(server_buff, 0, SERVER_BUFF_SIZE); 
            bytes_recv = recv(client_fd, server_buff, sizeof(server_buff), 0);

            //All bytes now received are parts missing from the body, so print all
            disp_base2(server_buff, bytes_recv);

            total_recv += bytes_recv;


            
        }
            
        fprintf(stdout, "\n\n");
    }


    fprintf(stdout, "===--- Client ending connection with server");
    close(client_fd);

}

void Client::start_server() {
    fprintf(stdout, "\n\n+++++++++++++++++++++++++----- Client Started At IP %s And Port %d -----+++++++++++++++++++++++++\n\n", inet_ntoa(addr.sin_addr), SERVER_PORT);
    //If bytes_sent = 0, new connection will be established
    int bytes_sent = 0;

    fprintf(stdout, "====================----- Attempting New Connection -----====================\n\n");

    if (connect(client_fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        fprintf(stdout, "!!!Client failed to establish connection, ending connection\n\n");
        close(client_fd);
        return;
    }
    else {
        fprintf(stdout, "===--- Established client connection\n\n"); 
    }

    int bytes_recv;
    char server_buff[SERVER_BUFF_SIZE];

    memset(server_buff, 0, SERVER_BUFF_SIZE); 

    //Generate request string after asking user for input
    http_req_t* req = create_req();
    char* req_str = _gen_req_str(req);

    fprintf(stdout, "===--- Sending request to server, request is\n\n");
    fprintf(stdout, "%s\n\n", req_str);

    //Send request to server
    bytes_sent = send(client_fd, req_str, strlen(req_str), 0);

    //Free request string and request object after sending them to server
    free(req_str);
    _free_req(req);
    
    //Check on status of sent req
    if (bytes_sent <= 0) {
        fprintf(stdout, "!!!Client failed to send bytes, ending connection\n\n");
        close(client_fd);
        return;
    }
    else if (bytes_sent != (int)strlen(req_str)) {
        fprintf(stdout, "!!!Client failed to send full request\n\n");
    }
    else {
        fprintf(stdout, "===--- Request sent successfully to server\n\n");
    }


    //Get server response
    bytes_recv = recv(client_fd, server_buff, sizeof(server_buff), 0);

    //adds \0 at start of body so header can now be printed and its length found
    char* body_ptr = _get_body_ptr(server_buff);
    int hdr_len = strlen(server_buff) + 1;

    fprintf(stdout, "===--- Server response header is\n\n");
    fprintf(stdout, "%s\n\n", server_buff);

    //adds \0 at end of content type so header can no longer be printed
    int content_len = _get_content_len(server_buff);
    char* content_type = _get_content_type(server_buff);


    if (strcmp(content_type, "text/html") == 0 || strcmp(content_type, "text/plain") == 0) {
        fprintf(stdout, "===--- Server response body (ASCII format) is\n\n");
        disp_txt(body_ptr, strlen(body_ptr)); 

        //Total received bytes from http body, used to compare against expected
        int total_recv = strlen(body_ptr);

        //While bytes are still missing from body request them (or until end of stream)
        while (total_recv < content_len) {

            memset(server_buff, 0, SERVER_BUFF_SIZE); 
            total_recv += recv(client_fd, server_buff, sizeof(server_buff), 0);

            disp_txt(body_ptr, strlen(body_ptr)); 
            
        }
            
        fprintf(stdout, "\n\n");

    }
    else {
        fprintf(stdout, "===--- Server response body (base2 format) is\n\n");

        //Total received bytes from body (does not use strlen since content is binary)
        int total_recv = bytes_recv - hdr_len;

        //Print binary data based on calculated body length (not from strlen due to binary)
        disp_base2(body_ptr, total_recv);

        //While bytes are still missing from body request them (or until end of stream)
        while (total_recv < content_len) {

            memset(server_buff, 0, SERVER_BUFF_SIZE); 
            bytes_recv = recv(client_fd, server_buff, sizeof(server_buff), 0);

            //All bytes now received are parts missing from the body, so print all
            disp_base2(server_buff, bytes_recv);

            total_recv += bytes_recv;


            
        }
            
        fprintf(stdout, "\n\n");
    }


    fprintf(stdout, "===--- Client ending connection with server");
    close(client_fd);
}

http_req_t* Client::create_req() {
    http_req_t* req = (http_req_t*)malloc(sizeof(http_req_t));
    req->req_type = NULL;
    req->URL = NULL;
    req->resource_path = NULL;
    req->resource_type = NULL;
    req->connection_type = NULL;
    req->content_length = NULL;
    req->content_body_ptr = NULL;

    char user_res[100];

    //Get the request method (GET or POST)
    printf("(Makin a GET request means to request data, while making a POST request will give you the option to send data)\n");
    printf("Enter g to make a GET request or enter p to make a POST request: ");
    while(1) {
        fgets(user_res, sizeof(user_res), stdin);
        _clean_newline(user_res);

        if (strcmp(user_res, "p") == 0 || strcmp(user_res, "P") == 0) {
            req->req_type = strdup("POST");
            break;
        }
        else if (strcmp(user_res, "g") == 0 || strcmp(user_res, "G") == 0) {
            req->req_type = strdup("GET");
            break;
        }
        else {
            printf("Invalid input, please enter g to make a GET request or enter p to make a POST request: ");
        }
    }
    printf("\n");

    //Get the relative URL path (not including leading /)
    printf("Enter relative URL path to send request, do not include leading \"/\" and input can be blank (For example: input of file.html becomes the URL /file.html): ");
    fgets(user_res, sizeof(user_res), stdin);
    _clean_newline(user_res);
    req->resource_path = strdup(user_res);
    printf("\n");

    //Prepend / to relative url path
    req->URL = (char*)malloc(sizeof(char) * (strlen(req->resource_path) + 2));
    strcat(req->URL, "/");
    strcat(req->URL, req->resource_path);

    if (strlen(req->resource_path) > 1) {
        req->resource_type = get_ft(req->resource_path);
    }

    req->connection_type = strdup("close");

    //If the method is POST ask user to add a body
    if (strcmp(req->req_type, "POST") == 0) {
        printf("You've opted to make a POST request. Please enter the body of your request: ");
        fgets(user_res, sizeof(user_res), stdin);

        //Check if user enters nothing
        if (user_res[0] == '\n') {
            req->content_body_ptr = strdup(""); 
            req->content_length = strdup("0"); 
        }
        else {
            _clean_newline(user_res);

            req->content_body_ptr = strdup(user_res);
            size_t len = strlen(req->content_body_ptr);
            req->content_length = (char*)malloc(len + 1);
            snprintf(req->content_length, len + 1, "%zu", len);
        }

    }
    else {
        req->content_body_ptr = NULL;
        req->content_length = strdup("0");
    }

    return req;
}

void Client::disp_txt(char* buff, int size) {

    //Diplsay buffer
    for (int i = 0; i < size; ++i) {
        putchar(buff[i]);
    }

}

void Client::disp_base2(char* buff, int size) {
    
    //Display all bits from a buffer
    for (int i = 0; i < size; ++i) {
        for (int j = 7; j >= 0; --j) {
            fprintf(stdout, "%d", (buff[i] >> j) & 1);
        }
        fprintf(stdout, " ");
    }
    fprintf(stdout, "\n");

}
    
char* Client::_get_body_ptr(char* buff) {
    char* body_ptr = strstr(buff, "\r\n\r\n");
    //Insert a \0 at the end of \r\n\r\n to allow for printing the header
    body_ptr[3] = '\0';
    body_ptr += strlen("\r\n\r\n");

    return body_ptr;
}

int Client::_get_content_len(char* buff) {
    int res = 0;
    //Index to parse content length
    int i = 0;

    char* cl_ptr = strstr(buff, "Content-Length: ");
    cl_ptr += strlen("Content-Length: ");

    //Keep appending each digit to res then return it
    while (isdigit(cl_ptr[i])) {

        int digit = (cl_ptr[i] - '0');
        res = (res * 10) + digit;
        ++i;

    }
    
    return res; 
}


char* Client::_get_content_type(char* buff) {
    //Find pointer to end of Content-Type
    char* ct_ptr = strstr(buff, "Content-Type: ");
    ct_ptr += strlen("Content-Type: ");

    //Insert \0 where \r is to create c string
    char* ct_ptr_end = strchr(ct_ptr, '\r');
    *ct_ptr_end = '\0';

    return ct_ptr;
}

char* Client::_gen_req_str(http_req_t* req) {

    //Go through each member of http_req_t to create http string
    char* req_str = (char*)malloc(SERVER_BUFF_SIZE * sizeof(char));
    memset(req_str, 0, SERVER_BUFF_SIZE); 

    strcat(req_str, req->req_type);
    strcat(req_str, " ");

    strcat(req_str, req->URL);
    strcat(req_str, " ");

    strcat(req_str, "HTTP/1.1\r\n");

    char addr_str[100];
    inet_ntop(AF_INET, &(addr.sin_addr), addr_str, sizeof(addr_str));
    strcat(req_str, "Host: ");
    strcat(req_str, addr_str);
    strcat(req_str, ":");

    char port_str[100];
    snprintf(port_str, 100, "%d", ntohs(addr.sin_port));
    strcat(req_str, port_str);
    strcat(req_str, "\r\n");

    strcat(req_str, "User-Agent: httpClient\r\n");
    strcat(req_str, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8\r\n");
    strcat(req_str, "Accept-Language: en-US,en;q=0.5\r\n");

    if (strcmp(req->req_type, "POST") == 0) {
        strcat(req_str, "Content-Type: text/plain\r\n");
        strcat(req_str, "Content-Length: ");
        strcat(req_str, req->content_length);
        strcat(req_str, "\r\n");
    }

    strcat(req_str, "Connection: ");
    strcat(req_str, req->connection_type);
    strcat(req_str, "\r\n\r\n");

    if (strcmp(req->req_type, "POST") == 0) {
        strcat(req_str, req->content_body_ptr);
    }
    
    return req_str;
}

void Client::_free_req(http_req_t* req) {
    if (req->req_type != NULL) {
        free(req->req_type);
    }
    if (req->URL != NULL) {
        free(req->URL);
    }
    if (req->resource_path != NULL) {
        free(req->resource_path);
    }
    if (req->connection_type != NULL) {
        free(req->connection_type);
    }
    if (req->content_length != NULL) {
        free(req->content_length);
    }
    if (req->content_body_ptr != NULL) {
        free(req->content_body_ptr);
    }

    free(req);
}

void Client::_clean_newline(char* buff) {
    // Find first \0
    char* eos = strchr(buff, '\0');

    // If end of string doesnt equal beginning of string
    if (eos != buff) {

        // If there is a preceding \n, from user input, replace it 
        if (*(eos - 1) == '\n') {
            *(eos - 1) = '\0';
        }
    }
}

Client::~Client() {
    //Close client file descriptor
    close(client_fd);
}
