#include <pthread.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http_server.h"

int main(int argc, char *argv[]) {
    int port = 8080;
    if (argc == 3 && strcmp(argv[1], "-p") == 0) 
    {
        port = atoi(argv[2]);
    } else if (argc != 1) {
        fprintf(stderr, "Usage: %s [-p <port>]\n", argv[0]);

        exit(EXIT_FAILURE);
    }

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Socket creation failed.");

        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_sock);

        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, 10) < 0) {
        perror("Listen failed");
        close(server_sock);

        exit(EXIT_FAILURE);
    }
    printf("Server running on port %d\n", port);

    while (1) {
        int client_sock = accept(server_sock, NULL, NULL);

        if (client_sock < 0) {
            perror("Accept failed");

            continue;
        }

        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, (void *)(intptr_t)client_sock);
        pthread_detach(thread);
    }

    close(server_sock);
    return 0;
}
