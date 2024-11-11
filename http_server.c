#include "http_server.h"

void read_http_client_message(int client_sock, http_client_message_t** msg, http_read_result_t* result) {
    char buffer[HTTP_BUFFER_SIZE];
    ssize_t bytes_received;
    size_t total_bytes_received = 0;
    int header_index = 0;
    int in_body = 0;
    
    result->status_code = 0;
    result->bytes_read = 0;
    result->error_message = NULL;

    *msg = (http_client_message_t*)malloc(sizeof(http_client_message_t));
    if (*msg == NULL) {
        result->status_code = 1;
        result->error_message = "Allocation failed";
        return;
    }

    memset(*msg, 0, sizeof(http_client_message_t));

    while ((bytes_received = recv(client_sock, buffer, sizeof(buffer), 0)) > 0) {
        total_bytes_received += bytes_received;

        for (size_t i = 0; i < bytes_received; i++) {
            if (!in_body) {
                if (buffer[i] == '\r' && i + 1 < bytes_received && buffer[i + 1] == '\n') {
                    if (header_index == 0) 
                    {
                        in_body = 1;
                    } else {
                        char *line = &buffer[i + 2];
                        char *colon_pos = strchr(line, ':');
                        if (colon_pos) {
                            *colon_pos = '\0';
                            strncpy((*msg)->headers[header_index][0], line, MAX_HEADER_NAME_LENGTH);
                            strncpy((*msg)->headers[header_index][1], colon_pos + 1, MAX_HEADER_VALUE_LENGTH);
                            header_index++;
                        }
                    }

                    i++;
                }
            }

            if (header_index == 0 && buffer[i] != '\r') {
                char *line_start = &buffer[i];
                char *space_pos = strchr(line_start, ' ');
                if (!space_pos) break;
                *space_pos = '\0';
                strncpy((*msg)->method, line_start, sizeof((*msg)->method));

                line_start = space_pos + 1;
                space_pos = strchr(line_start, ' ');
                if (!space_pos) break;
                *space_pos = '\0';
                strncpy((*msg)->url, line_start, sizeof((*msg)->url));

                line_start = space_pos + 1;
                strncpy((*msg)->version, line_start, sizeof((*msg)->version));
            }

            if (in_body) {
                size_t remaining_space = HTTP_BUFFER_SIZE - (*msg)->body_length;
                if (remaining_space > 0) {
                    (*msg)->body[(*msg)->body_length++] = buffer[i];
                }
            }
        }

        if (bytes_received < sizeof(buffer)) {
            break;
        }
    }

    result->bytes_read = total_bytes_received;

    if (bytes_received <= 0) {
        result->status_code = 2;
        result->error_message = "Failed to read data";
    }
}
