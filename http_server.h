#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <stddef.h>

typedef struct {
    char method[8];
    char url[256];
    char version[16];
} http_client_message_t;

typedef struct {
    int status_code;
    size_t bytes_read;
} http_read_result_t;

void read_http_client_message(
    int client_sock, http_client_message_t** msg, http_read_result_t* result);
void handle_static_request(int client_sock, const char* file_path);
void handle_stats_request(int client_sock);
void handle_calc_request(int client_sock, const char* query_string);
void* handle_client(void* client_sock);

#endif
