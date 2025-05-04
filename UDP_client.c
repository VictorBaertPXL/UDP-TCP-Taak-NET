#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 12345
#define MAXBUF 1024

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    char buffer[MAXBUF];
    char input[16];

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &servaddr.sin_addr);

    while (1) {
        // Send guess
        printf("Enter your guess (0-99): ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0;

        sendto(sockfd, input, strlen(input), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));

        // Set timeout
        struct timeval tv = {2, 0}; // 2 seconds timeout
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        int len = sizeof(servaddr);
        int n = recvfrom(sockfd, buffer, MAXBUF, 0, (struct sockaddr *)&servaddr, &len);

        if (n < 0) {
            printf("No response from server. Try again.\n");
        } else {
            buffer[n] = '\0';
            printf("%s\n", buffer);
            if (strcmp(buffer, "You won!") == 0) {
                break; // Stop als de client wint
            }
        }
    }

    close(sockfd);
    return 0;
}