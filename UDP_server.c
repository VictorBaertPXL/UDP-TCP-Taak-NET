#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 12345
#define MAXBUF 1024
#define TIMEOUT_INITIAL 8
#define TIMEOUT_FINAL 16

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t len = sizeof(client_addr);
    char buffer[MAXBUF];
    int closest_guess = -1, closest_diff = 9999;
    char winner_ip[INET_ADDRSTRLEN];

    srand(time(NULL));
    int target = rand() % 100;

    printf("Generated number: %d\n", target);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Set timeout for receiving
    struct timeval tv = {TIMEOUT_INITIAL, 0};
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    printf("Waiting for guesses...\n");

    while (1) {
        int n = recvfrom(sockfd, buffer, MAXBUF, 0, (struct sockaddr *)&client_addr, &len);
        if (n < 0) break;

        buffer[n] = '\0';
        int guess = atoi(buffer);
        int diff = abs(guess - target);

        if (diff < closest_diff) {
            closest_diff = diff;
            closest_guess = guess;
            inet_ntop(AF_INET, &client_addr.sin_addr, winner_ip, INET_ADDRSTRLEN);
        }

        printf("Received %d from %s\n", guess, inet_ntoa(client_addr.sin_addr));
    }

    // Final timeout period
    tv.tv_sec = TIMEOUT_FINAL;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    while (1) {
        int n = recvfrom(sockfd, buffer, MAXBUF, 0, (struct sockaddr *)&client_addr, &len);
        if (n < 0) break;
    }

    // Send result to clients
    char message[64];
    snprintf(message, sizeof(message), "You won !");
    inet_pton(AF_INET, winner_ip, &client_addr.sin_addr);
    sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&client_addr, len);

    printf("Winner is %s with guess %d (target was %d)\n", winner_ip, closest_guess, target);
    close(sockfd);
    return 0;
}