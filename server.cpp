#include "server.h"

const char* NOT_FOUND_MSG = "HTTP/1.1 404 Not Found\nContent-Type: text/html\n\n<!DOCTYPE html>\n<html>\n<head>\n<title>404 Not Found</title>\n</head>\n<body>\n<h1>404 Not Found</h1>\n<p>The requested resource was not found.</p>\n</body>\n</html>";

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

void Server::start_server() {
    while(1) {
        fprintf(stdout, "\n====================----- Waiting For New Connection -----====================\n\n");

        if ((client_fd = accept(server_fd, (struct sockaddr *)&addr, (socklen_t*)&addr_len)) < 0) {
            fprintf(stderr, "Server failed to accept incoming connection from %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
            perror("[ERROR] could not accept client request");
            exit(EXIT_FAILURE);
        }
            
        fprintf(stdout, "===--- Server accepted incoming connection from %s:%d\n\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

        int bytes_recv;
        char client_buff[SERVER_BUFF_SIZE];
        http_req_t req;

        do {

            memset(client_buff, 0, SERVER_BUFF_SIZE); 

            bytes_recv = read(client_fd, client_buff, SERVER_BUFF_SIZE);

            //If bytes received are invalid, end connection
            if (bytes_recv < 0) {
                fprintf(stdout, "Server failed to read bytes from %s:%d\n\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
                break;
            }

            fprintf(stdout, "--- Client request is\n\n"); 
            //Print the client request
            fprintf(stdout, "%s\n", client_buff);
            
            req = parse_req(client_buff);
            
            fprintf(stdout, "--- Sending response to client\n\n");

            if (handle_req(client_fd, &req)) {
                fprintf(stdout, "--- Response sent successfully to client\n\n");
            }
            else {
                break;
            }

        } while(bytes_recv > 0 && strcmp(req.connection_type, "close") != 0);

        fprintf(stdout, "===--- Server ending connection with %s:%d\n\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        close(client_fd);

    }

    close(server_fd);
}

http_req_t Server::parse_req(char* client_buff) {
    char* body_ptr = strrchr(client_buff, '\r');

    http_req_t req;
    req.resource_type = NULL;
    req.resource_path = NULL;
    req.content_length = NULL;
    req.content_body_ptr = body_ptr;

    char* req_type = strtok(client_buff, " \n");
    char* URL = strtok(NULL, " \n");

    //If request type is GET, parse the resource type and resource path
    if (strcmp(req_type, "GET") == 0) {
        //Get the resource type (filetype)
        req.resource_type = _get_ft(URL);
        //If resource type is not NULL (it is a file) remove starting / from the path
        if (req.resource_type != NULL) {
            req.resource_path = &URL[1];
        }
    }
    //If request is POST get the content-length
    else if (strcmp(req_type, "POST") == 0) {
        //Get the connection type (i.e close or keep alive)
        char* content_length = strtok(NULL, " \n");
        while (strcmp(content_length, "Content-Length:") != 0) {
            content_length = strtok(NULL, " \n");
        }
        content_length = strtok(NULL, " \n");
    }

    //Get the connection type (i.e close or keep alive)
    char* connection_type = strtok(NULL, " \n");
    while (strcmp(connection_type, "Connection:") != 0) {
        connection_type = strtok(NULL, " \n");
    }
    connection_type = strtok(NULL, " \n");

    req.URL = URL;
    req.req_type = req_type;
    req.connection_type = connection_type;
    return req;
}

bool Server::handle_req(int client_fd, http_req_t* req) {
    //If request type is GET
    if (strcmp(req->req_type, "GET") == 0) {

        if (strcmp(req->URL, "/") == 0) {
            return _send_file_res(client_fd, "index.html", "text/html");
        }

        //If no direct match for url path, send resource_path to _send_file_res (if resource_path is NULL, the function will send a 404 page)
        else {
            return _send_file_res(client_fd, req->resource_path, req->resource_type);
        }

    }
    else if (strcmp(req->req_type, "POST") == 0) {
        return _send_text_res(client_fd, req->content_body_ptr);
    }
    else {
        fprintf(stdout, "--- The URL %s does not match any resources, sending 404 response\n\n", req->URL);
        return _send_file_res(client_fd, "404.html", "text/html");
    }
}

bool Server::_send_file_res(int client_fd, const char* filepath, const char* filetype) {
    FILE* file = NULL;
    unsigned long file_len;
    char* res;
    char status_code[64];

    if (filepath != NULL) { 
        file = fopen(filepath, "rb");
    }

    //If file not found, or filepath not provided, set file to 404.html, ensure filetype is text/html, and set status code to 404
    if (file == NULL) {
        fclose(file);

        strcpy(status_code, "404 Not Found");
        file = fopen("404.html", "rb");
        filetype = "text/html";
    }
    //If file is found, but is 404.html, set status code to 404, and ensure filetype is text/html
    else if (strcmp(filepath, "404.html") == 0) {
        fclose(file);

        strcpy(status_code, "404 Not Found");
        filetype = "text/html";
    }
    //If file is found but filetype is not specified, assume error occurred and repeat steps from first if statement
    else if (filetype == NULL) {
        fclose(file);

        strcpy(status_code, "404 Not Found");
        file = fopen("404.html", "rb");
        filetype = "text/html";
    }
    //If file is found and filetype is specified set status code to 200 OK
    else {
        strcpy(status_code, "200 OK");
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

    dprintf(client_fd, "HTTP/1.1 %s\nContent-Length: %ld\nContent-Type: %s\n\n", status_code, file_len, filetype);

    unsigned long bytes_sent = write(client_fd, res, file_len);

    free(res);

    return (bytes_sent == file_len);
}

bool Server::_send_text_res(int client_fd, const char* res_text) {
    const char* status_code = "200 OK";
    unsigned long text_len = strlen(res_text);

    dprintf(client_fd, "HTTP/1.1 %s\nContent-Length: %ld\nContent-Type: text/plain\n\n", status_code, text_len);

    unsigned long bytes_sent = write(client_fd, res_text, text_len);

    return bytes_sent == text_len;
}

const char* Server::_get_ft(const char* filepath) {
    const char* ext = strrchr(filepath, '.');
    if (ext != NULL) {
        if (strcmp(ext, ".html") == 0) {
            return "text/html";
        } else if (strcmp(ext, ".txt") == 0) {
            return "text/plain";
        } else if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) {
            return "image/jpeg";
        } else if (strcmp(ext, ".png") == 0) {
            return "image/png";
        } else if (strcmp(ext, ".gif") == 0) {
            return "image/gif";
        } else if (strcmp(ext, ".pdf") == 0) {
            return "application/pdf";
        }
    }
    return NULL;
}

Server::~Server() {
    close(server_fd);
    close(client_fd);
}
