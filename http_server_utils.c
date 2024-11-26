#include "http_server.h"
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static int request_count = 0;
static size_t total_bytes_received = 0;
static size_t total_bytes_sent = 0;

void read_http_client_message(
    int client_sock, http_client_message_t** msg, http_read_result_t* result)
{

    char buffer[1024] = { 0 };

    ssize_t bytes_read = recv(client_sock, buffer, sizeof(buffer) - 1, 0);

    if (bytes_read <= 0) {
        result->status_code = -1;
        result->bytes_read = 0;
        return;
    }

    *msg = malloc(sizeof(http_client_message_t));

    sscanf(buffer, "%s %s %s", (*msg)->method, (*msg)->url, (*msg)->version);

    result->status_code = 0;
    result->bytes_read = (size_t)bytes_read;
}

void handle_static_request(int client_sock, const char* file_path)
{
    char full_path[512] = "./static/";
    strncat(full_path, file_path, sizeof(full_path) - strlen(full_path) - 1);

    FILE* file = fopen(full_path, "rb");
    if (!file) {
        send(client_sock, "HTTP/1.1 404 Not Found\r\n\r\n", 26, 0);
        total_bytes_sent += 26;
        return;
    }

    const char* headers
        = "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\n\r\n";
    send(client_sock, headers, strlen(headers), 0);
    total_bytes_sent += strlen(headers);

    char buffer[1024];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        send(client_sock, buffer, bytes, 0);
        total_bytes_sent += bytes;
    }

    fclose(file);
}

void handle_stats_request(int client_sock)
{
    char response[1024];
    snprintf(response, sizeof(response),
        "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
        "<html><body><h1>Server Stats</h1>"
        "<p>Requests received: %d</p>"
        "<p>Bytes received: %zu</p>"
        "<p>Bytes sent: %zu</p></body></html>",
        request_count, total_bytes_received, total_bytes_sent);
    send(client_sock, response, strlen(response), 0);
    total_bytes_sent += strlen(response);
}

void handle_calc_request(int client_sock, const char* query_string)
{
    int a, b;
    if (sscanf(query_string, "a=%d&b=%d", &a, &b) == 2) {
        int sum = a + b;
        char response[1024];
        snprintf(response, sizeof(response),
            "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
            "<html><body><h1>Calculation Result</h1>"
            "<p>%d + %d = %d</p></body></html>",
            a, b, sum);
        send(client_sock, response, strlen(response), 0);
        total_bytes_sent += strlen(response);
    } else {
        send(client_sock, "HTTP/1.1 400 Fails", 26, 0);
        total_bytes_sent += 26;
    }
}

void* handle_client(void* client_sock)
{
    int sock = (intptr_t)client_sock;
    http_client_message_t* msg = NULL;
    http_read_result_t result = { 0 };

    read_http_client_message(sock, &msg, &result);
    if (result.status_code < 0) {
        close(sock);
        return NULL;
    }

    request_count++;
    total_bytes_received += result.bytes_read;

    if (strncmp(msg->url, "/static", 7) == 0) {
        handle_static_request(sock, msg->url + 8);
    } else if (strncmp(msg->url, "/stats", 6) == 0) {
        handle_stats_request(sock);
    } else if (strncmp(msg->url, "/calc", 5) == 0) {
        handle_calc_request(sock, msg->url + 6);
    } else {
        send(sock, "HTTP/1.1 404 Not Found\r\n\r\n", 26, 0);
        total_bytes_sent += 26;
    }

    close(sock);
    free(msg);

    return NULL;
}
