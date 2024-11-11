#include "http_server.h"
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define DEFAULT_PORT 80

void *handle_client(void *arg) {
    int client_sock = (intptr_t)arg;
    http_client_message_t *msg = NULL;
    http_read_result_t result;

    read_http_client_message(client_sock, &msg, &result);

    if (result.status_code != 0) {
        fprintf(stderr, "Error reading client message: %s\n", result.error_message);
        close(client_sock);
        return NULL;
    }

    printf("Received Request:\n");
    printf("Method: %s\n", msg->method);
    printf("URL:%s\n", msg->url);
    printf("Version: %s\n", msg->version);
    printf("Headers:\n");
    for (int i = 0; i < msg->num_headers; i++) {
        printf("  %s: %s\n", msg->headers[i][0], msg->headers[i][1]);
    }

    if (strcmp(msg->url, "/stats") == 0) {
        char response[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>Stats</h1><p>Request handled</p>";
        send(client_sock, response, sizeof(response) - 1, 0);
    } else {
        char response[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello, World!";
        send(client_sock, response, sizeof(response) - 1, 0);
    }
    free(msg);
    close(client_sock);
    return NULL;
}

int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;  
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pthread_t thread_id;

    if (argc > 2) {
        fprintf(stderr, "Usage: %s [-p port]\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (argc == 2 && strcmp(argv[1], "-p") == 0) {
        if (argc == 4) {
            port = atoi(argv[3]);
        } else {
            fprintf(stderr, "Usage: %s [-p port]\n", argv[0]);
            return EXIT_FAILURE;
        }
    }

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("Socket creation failed");
        return EXIT_FAILURE;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_sock);
        return EXIT_FAILURE;
    }

    if (listen(server_sock, 5) == -1) {
        perror("Listen failed");
        close(server_sock);
        return EXIT_FAILURE;
    }
    printf("Server listening on port %d...\n", port);

    while (1) {
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
        if (client_sock == -1) {
            perror("Accept failed");
            continue;
        }
        if (pthread_create(&thread_id, NULL, handle_client, (void *)(intptr_t)client_sock) != 0) {
            perror("Thread creation failed");
            close(client_sock);
        }

        pthread_detach(thread_id);
    }
    close(server_sock);
    return EXIT_SUCCESS;
}
