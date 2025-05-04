#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 12345
#define MAXBUF 1024

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

    printf("Waiting for guesses...\n");

    while (1) {
        int n = recvfrom(sockfd, buffer, MAXBUF, 0, (struct sockaddr *)&client_addr, &len);
        if (n < 0) {
            perror("recvfrom error or timeout");
            break;
        }

        buffer[n] = '\0';
        int guess = atoi(buffer);
        int diff = abs(guess - target);

        if (diff < closest_diff) {
            closest_diff = diff;
            closest_guess = guess;
            inet_ntop(AF_INET, &client_addr.sin_addr, winner_ip, INET_ADDRSTRLEN);
        }

        printf("Received guess %d from %s\n", guess, inet_ntoa(client_addr.sin_addr));

        if (guess == target) {
            // Winnaar gevonden, stuur feedback terug
            char message[64];
            snprintf(message, sizeof(message), "You won!");
            sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&client_addr, len);
            printf("Winner is %s with guess %d\n", winner_ip, closest_guess);
            break;
        } else {
            // Feedback sturen dat het niet juist is met verschil
            char feedback[64];
            snprintf(feedback, sizeof(feedback), "Not correct, you missed by %d.", diff);
            sendto(sockfd, feedback, strlen(feedback), 0, (struct sockaddr *)&client_addr, len);
        }
    }

    close(sockfd);
    return 0;
}
