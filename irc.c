#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>

#define PORT 6667
#define BUFFER_SIZE 512

void send_message(int sockfd, const char *message) {
    send(sockfd, message, strlen(message), 0);
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char server[100], nickname[50], channel[50];

    // ask user for desired server address and their nick
    printf("Enter server address: ");
    scanf("%s", server);
    printf("Enter your nickname: ");
    scanf("%s", nickname);
    // create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // set up server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    struct hostent *host = gethostbyname(server);
    if (host == NULL) {
        fprintf(stderr, "Invalid server address: %s\n", server);
        return 1;
    }
    memcpy(&server_addr.sin_addr, host->h_addr, host->h_length);

    // connect to server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return 1;
    }

    // send NICK and USER commands
    char user_command[BUFFER_SIZE];
    snprintf(user_command, sizeof(user_command), "NICK %s\r\n", nickname);
    send_message(sockfd, user_command);
    snprintf(user_command, sizeof(user_command), "USER %s 0 * :My IRC Client\r\n", nickname);
    send_message(sockfd, user_command);

    // join the specified channel
    char join_command[BUFFER_SIZE];
    snprintf(join_command, sizeof(join_command), "JOIN %s\r\n", channel);
    send_message(sockfd, join_command);

    // main loop to handle user input and server messages
    fd_set read_fds;
    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);  // monitor the server socket
        FD_SET(STDIN_FILENO, &read_fds);  // monitor stdin for user input

        int max_fd = (sockfd > STDIN_FILENO) ? sockfd : STDIN_FILENO;

        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("Select error");
            break;
        }

        // handle server messages
        if (FD_ISSET(sockfd, &read_fds)) {
            int bytes_received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
            if (bytes_received <= 0) {
                perror("Receive failed");
                break;
            }
            buffer[bytes_received] = '\0';
            printf("%s", buffer);

            // respond to PING
            if (strncmp(buffer, "PING :", 6) == 0) {
                char pong_response[BUFFER_SIZE];
                snprintf(pong_response, sizeof(pong_response), "PONG %s", buffer + 5);
                send_message(sockfd, pong_response);
            }
        }

        // handle user input
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            char user_input[BUFFER_SIZE];
            if (fgets(user_input, sizeof(user_input), stdin) != NULL) {
                // remove newline character
                user_input[strcspn(user_input, "\n")] = '\0';

                // / for command
                if (user_input[0] == '/') {
                    send_message(sockfd, user_input + 1); // skip /
                    send_message(sockfd, "\r\n");
                } else {
                    // otherwise, treat it as a message to the channel
                    char chat_message[BUFFER_SIZE];
                    snprintf(chat_message, sizeof(chat_message), "PRIVMSG %s :%s\r\n", channel, user_input);
                    send_message(sockfd, chat_message);
                }
            }
        }
    }

    close(sockfd);
    return 0;
}
