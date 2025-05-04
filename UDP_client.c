#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int guess;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    printf("Enter your guess (0 - 99): ");
    scanf("%d", &guess);

    // Stuur de gok naar de server
    sendto(sockfd, &guess, sizeof(int), 0,
           (const struct sockaddr *)&server_addr, sizeof(server_addr));

    // Wacht maximaal 8 seconden op antwoord
    fd_set readfds;
    struct timeval tv;
    tv.tv_sec = 8;
    tv.tv_usec = 0;
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);

    printf("Waiting for result...\n");

    if (select(sockfd + 1, &readfds, NULL, NULL, &tv) > 0) {
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, NULL, NULL);
        if (n >= 0) {
            buffer[n] = '\0';
            printf("Server: %s\n", buffer);
        } else {
            perror("recvfrom failed");
        }
    } else {
        printf("No response from server. Exiting.\n");
    }

    close(sockfd);
    return 0;
}
