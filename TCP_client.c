#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[1024];
    int gok;

    // Maak socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket maken mislukt");
        exit(EXIT_FAILURE);
    }

    // Zet servergegevens
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    // Verbind met server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect mislukt");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Vraag gebruiker om input
        printf("Raad het getal (tussen 1 en 100): ");
        if (scanf("%d", &gok) != 1) {
            printf("Ongeldige invoer.\n");
            // Clear stdin buffer
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            continue;
        }

        // Stuur gok naar server
        sprintf(buffer, "%d", gok);
        send(sockfd, buffer, strlen(buffer), 0);

        // Ontvang antwoord van server
        memset(buffer, 0, sizeof(buffer));
        ssize_t received = recv(sockfd, buffer, sizeof(buffer)-1, 0);
        if (received <= 0) {
            printf("Verbinding verloren met server.\n");
            break;
        }
        buffer[received] = '\0';
        printf("Antwoord van server: %s\n", buffer);

        if (strstr(buffer, "Perfect!") != NULL) {
            // Correct geraden, stop met raden
            break;
        }
    }

    close(sockfd);
    return 0;
}
