#include "httpServer.h"

Server::Server() {
    addr.sin_family = AF_INET;             
    addr.sin_port = htons(SERVER_PORT);        
    addr.sin_addr.s_addr = INADDR_ANY;

    addr_len = sizeof(addr);

    memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("[ERROR] could not create socket");
        exit(EXIT_FAILURE);
    }
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("[ERROR] could not bind address for server");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0) {
        perror("[ERROR] could not set server to passive mode (10 requests at a time)");
        exit(EXIT_FAILURE);
    }
}

    
Server::Server(char* addr_str, char* port_str) {
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

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("[ERROR] could not create socket");
        exit(EXIT_FAILURE);
    }
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("[ERROR] could not bind address for server");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0) {
        perror("[ERROR] could not set server to passive mode (10 requests at a time)");
        exit(EXIT_FAILURE);
    }

}

void Server::start_server() {
    fprintf(stdout, "\n\n+++++++++++++++++++++++++----- Server Started At IP %s And Port %d -----+++++++++++++++++++++++++\n\n", inet_ntoa(addr.sin_addr), SERVER_PORT);

    while(1) {
        fprintf(stdout, "====================----- Waiting For New Connection -----====================\n\n");

        if ((client_fd = accept(server_fd, (struct sockaddr *)&addr, (socklen_t*)&addr_len)) < 0) {
            //Skip to next client
            fprintf(stderr, "!!!Server failed to accept incoming connection, moving to next client in queue\n\n");
            close(client_fd);
            continue;
        }
        else {
            fprintf(stdout, "===--- Server accepted incoming connection\n\n");
        }

        int bytes_recv;
        char client_buff[SERVER_BUFF_SIZE];
        http_req_t req;

        do {

            memset(client_buff, 0, SERVER_BUFF_SIZE); 

            bytes_recv = read(client_fd, client_buff, SERVER_BUFF_SIZE);

            fprintf(stdout, "\n====================----- Server received new request -----====================\n\n");

            //If bytes received are invalid, end connection
            if (bytes_recv <= 0) {
                fprintf(stdout, "!!!Server failed to read bytes, moving to next client in queue\n\n");
                break;
            }

            fprintf(stdout, "===--- Client request is\n\n"); 
            //Print the client request
            fprintf(stdout, "%s\n", client_buff);

            req = parse_req(client_buff);

            fprintf(stdout, "===--- Sending response to client\n\n");

            if (handle_req(client_fd, &req)) {
                fprintf(stdout, "===--- Response sent successfully to client\n\n");
            }
            else {
                break;
            }

            

        } while(bytes_recv > 0 && strcmp(req.connection_type, "keep-alive") == 0);

        fprintf(stdout, "===--- Server ending connection with client\n\n");
        close(client_fd);

    }

    close(server_fd);
}

http_req_t Server::parse_req(char* client_buff) {
    //Set everything to NULL for error handling abilities
    http_req_t req;
    req.req_type = NULL;
    req.URL = NULL;
    req.resource_path = NULL;
    req.resource_type = NULL;
    req.connection_type = NULL;
    req.content_length = NULL;
    req.content_body_ptr = NULL;

    if (client_buff == NULL) {
        return req;
    }

    //Set ptr to start of req body
    char* body_ptr = strstr(client_buff, "\r\n\r\n");
    if (body_ptr == NULL) {
        return req;
    }
    body_ptr += strlen("\r\n\r\n");
    req.content_body_ptr = body_ptr;

    //Get req_type
    char* req_type = strtok(client_buff, " \r\n");
    if (req_type == NULL) {
        return req;
    }
    req.req_type = req_type;

    char* URL = strtok(NULL, " \r\n");
    if (URL == NULL) {
        return req;
    }
    req.URL = URL;

    //If request type is GET, parse the resource type and resource path
    if (strcmp(req_type, "GET") == 0) {

        //Get the resource type (filetype)
        req.resource_type = get_ft(URL);

        //If resource type is not NULL (it is a file) remove starting / from the path
        if (req.resource_type != NULL && strlen(req.URL) > 1) {
            req.resource_path = &URL[1];
        }
    }
    //If request is POST get the content-length
    else if (strcmp(req_type, "POST") == 0) {
        //Get the connection type (i.e close or keep alive)
        char* content_length = strtok(NULL, " \r\n");
        while (content_length != NULL && strcmp(content_length, "Content-Length:") != 0) {
            content_length = strtok(NULL, " \r\n");
        }

        content_length = strtok(NULL, " \r\n");
        if (content_length == NULL) {
            return req;
        }
        req.content_length = content_length;
    }

    //Get the connection type (i.e close or keep alive)
    char* connection_type = strtok(NULL, " \r\n");
    while (connection_type != NULL && strcmp(connection_type, "Connection:") != 0) {
        connection_type = strtok(NULL, " \r\n");
    }

    connection_type = strtok(NULL, " \r\n");
    if (connection_type == NULL) {
        return req;
    }
    req.connection_type = connection_type;

    return req;
}

bool Server::handle_req(int client_fd, http_req_t* req) {
    if (req->req_type == NULL) {
        fprintf(stdout, "!!!Attempted to respond to empty request object, moving onto next client in queue\n\n");
        return false;
    }

    //If request type is GET
    if (strcmp(req->req_type, "GET") == 0) {

        if (strcmp(req->URL, "/") == 0) {
            return _send_file_res(client_fd, "index.html", "text/html");
        }

        //If no direct match for url path, send resource_path to _send_file_res (if resource_path is NULL, the function will send a 404 message)
        else {
            return _send_file_res(client_fd, req->resource_path, req->resource_type);
        }

    }
    //If request type is POST send back the body
    else if (strcmp(req->req_type, "POST") == 0) {
        return _send_text_res(client_fd, req->content_body_ptr);
    }
    else {
        return _not_found(client_fd);
    }
}

bool Server::_send_file_res(int client_fd, const char* filepath, const char* filetype) {
    FILE* file = NULL;
    unsigned long file_len;
    char* res;

    if (filepath != NULL) { 
        file = fopen(filepath, "rb");
    }

    //If file is not found or file is null throw a 404 error
    if (file == NULL) {
        return _not_found(client_fd);
    }
    //If file is found but filetype is not specified, assume error occurred and send 404 error
    else if (filetype == NULL) {
        //close file since it is not NULL
        fclose(file);

        return _not_found(client_fd);
    }

    // Get the length of the file
    fseek(file, 0, SEEK_END);
    file_len = ftell(file);
    rewind(file);

    // Allocate memory for the file content
    res = (char *)malloc(file_len * sizeof(char));

    // Read file content into buffer
    fread(res, 1, file_len, file);

    fclose(file);

    //Send header
    dprintf(client_fd, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\r\n", filetype, file_len);

    //Write body
    unsigned long bytes_sent = write(client_fd, res, file_len);

    //Print to console what was just sent
    fprintf(stdout, "===---Received a resource GET request. The response Content-Length is %ld, the status code is 200 OK, and the resource being sent is %s\n\n", file_len, filepath);

    free(res);

    return (bytes_sent == file_len);
}

bool Server::_send_text_res(int client_fd, const char* res_text) {
    // response text cannot be NULL, if it is send 500 internal server error to client
    if (res_text == NULL) {
        return _intern_err(client_fd);
    }

    //Send header
    dprintf(client_fd, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %ld\r\n\r\n", strlen(res_text));

    //Send body
    unsigned long bytes_sent = write(client_fd, res_text, strlen(res_text));

    //Print to console what was just sent
    fprintf(stdout, "===---Received a text POST request. The response status code is 200 OK, and the text being sent is %s\n\n", res_text);

    return (bytes_sent == strlen(res_text));
}

bool Server::_not_found(int client_fd) {

    const char* err_msg = "Not Found\n";

    //Send header
    dprintf(client_fd, "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: %ld\r\n\r\n", strlen(err_msg));

    //Send body
    unsigned long bytes_sent = write(client_fd, err_msg, strlen(err_msg));

    //Print to console that 404 Not Found was just sent
    fprintf(stdout, "===--The reponse status code is 404 Not Found\n\n");

    return (bytes_sent == strlen(err_msg));
}

bool Server::_intern_err(int client_fd) {
    
    const char* err_msg = "500 Internal Error\n";

    //Send header
    dprintf(client_fd, "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\nContent-Length: %ld\r\n\r\n", strlen(err_msg));

    //Send body
    unsigned long bytes_sent = write(client_fd, err_msg, strlen(err_msg));

    //Print to console that 500 Internal Server Error was just sent
    fprintf(stdout, "===--The response status code is 500 Internal Server Error\n\n");

    return (bytes_sent == strlen(err_msg));
}

Server::~Server() {
    close(server_fd);
    close(client_fd);
}
