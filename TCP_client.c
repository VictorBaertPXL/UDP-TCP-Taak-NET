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
    connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    // Vraag gebruiker om input
    printf("Raad het getal (tussen 1 en 100): ");
    scanf("%d", &gok);

    // Stuur gok naar server
    sprintf(buffer, "%d", gok);
    send(sockfd, buffer, strlen(buffer), 0);

    // Ontvang antwoord van server
    memset(buffer, 0, sizeof(buffer));
    recv(sockfd, buffer, sizeof(buffer), 0);
    printf("Antwoord van server: %s\n", buffer);

    close(sockfd);
    return 0;
}