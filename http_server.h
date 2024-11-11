#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#include "http_server.h"

#define HTTP_BUFFER_SIZE 8192
#define MAX_HEADER_NAME_LENGTH 128
#define MAX_HEADER_VALUE_LENGTH 512

typedef struct {
    char method[16];
    char url[256];
    char version[16];
    char headers[10][2][MAX_HEADER_VALUE_LENGTH]; 
    int num_headers;
    char body[HTTP_BUFFER_SIZE];
    size_t body_length;
} http_client_message_t;

typedef struct {
    int status_code;
    size_t bytes_read;
    const char *error_message;
} http_read_result_t;

void read_http_client_message(int client_sock, http_client_message_t** msg, http_read_result_t* result);

#endif // HTTP_SERVER_H
