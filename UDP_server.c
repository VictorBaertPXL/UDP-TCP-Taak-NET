#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <math.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100  // genoeg voor veel clients

typedef struct {
    struct sockaddr_in addr;
    int guess;
} ClientGuess;

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len;
    char buffer[BUFFER_SIZE];
    int number_to_guess;
    ClientGuess clients[MAX_CLIENTS];
    int client_count = 0;

    srand(time(0));
    number_to_guess = rand() % 100;
    printf("Number to guess: %d\n", number_to_guess);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    addr_len = sizeof(client_addr);
    printf("Server started. Waiting for first guess on port %d...\n", PORT);

    // Wacht op eerste gok
    while (1) {
        fd_set readfds;
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100000; // 100ms poll
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        if (select(sockfd + 1, &readfds, NULL, NULL, &tv) > 0) {
            int guess;
            recvfrom(sockfd, &guess, sizeof(int), 0, (struct sockaddr *)&client_addr, &addr_len);

            printf("First guess: %d from %s:%d\n", guess,
                   inet_ntoa(client_addr.sin_addr),
                   ntohs(client_addr.sin_port));

            clients[client_count].addr = client_addr;
            clients[client_count].guess = guess;
            client_count++;
            break;
        }
    }

    // Start 8 seconden timer om andere gokken te verzamelen
    time_t start_time = time(NULL);
    while (time(NULL) - start_time < 8) {
        fd_set readfds;
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100000; // 100ms poll
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        if (select(sockfd + 1, &readfds, NULL, NULL, &tv) > 0) {
            int guess;
            recvfrom(sockfd, &guess, sizeof(int), 0, (struct sockaddr *)&client_addr, &addr_len);

            // Check of client al geregistreerd is
            int duplicate = 0;
            for (int i = 0; i < client_count; i++) {
                if (clients[i].addr.sin_addr.s_addr == client_addr.sin_addr.s_addr &&
                    clients[i].addr.sin_port == client_addr.sin_port) {
                    duplicate = 1;
                    break;
                }
            }

            if (!duplicate && client_count < MAX_CLIENTS) {
                clients[client_count].addr = client_addr;
                clients[client_count].guess = guess;
                client_count++;
                printf("Received guess: %d from %s:%d\n", guess,
                       inet_ntoa(client_addr.sin_addr),
                       ntohs(client_addr.sin_port));
            }
        }
    }

    // Bepaal winnaar (dichtst bij)
    int closest_diff = 999;
    int winner_index = -1;

    for (int i = 0; i < client_count; i++) {
        int diff = abs(clients[i].guess - number_to_guess);
        if (diff < closest_diff) {
            closest_diff = diff;
            winner_index = i;
        }
    }

    // Verstuur feedback naar alle clients
    char response[BUFFER_SIZE];
    for (int i = 0; i < client_count; i++) {
        if (i == winner_index) {
            snprintf(response, sizeof(response),
                     "You won! You guessed %d, the number was %d.",
                     clients[i].guess, number_to_guess);
        } else {
            snprintf(response, sizeof(response),
                     "You lost. You guessed %d, the number was %d. Winner guessed %d.",
                     clients[i].guess, number_to_guess, clients[winner_index].guess);
        }

        sendto(sockfd, response, strlen(response), 0,
               (const struct sockaddr *)&clients[i].addr, sizeof(clients[i].addr));
    }

    printf("Game over. Server shutting down.\n");
    close(sockfd);
    return 0;
}
