#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[1024];
    int geheim_getal;

    // Maak socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket maken mislukt");
        exit(EXIT_FAILURE);
    }

    // Zet servergegevens
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket
    bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_fd, 1);

    printf("Wachten op verbinding...\n");
    client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
    printf("Verbinding geaccepteerd!\n");

    // Genereer geheim getal
    srand(time(NULL));
    geheim_getal = rand() % 100 + 1;
    printf("Geheim getal: %d\n", geheim_getal);

    // Ontvang gok van client
    recv(client_fd, buffer, sizeof(buffer), 0);
    int gok = atoi(buffer);
    printf("Client gokte: %d\n", gok);

    // Vergelijk
    const char *antwoord;
    if (gok == geheim_getal) {
        antwoord = "Perfect! Je hebt het goed!";
    } else {
        int verschil = abs(geheim_getal - gok);
        sprintf(buffer, "Niet juist. Je zat er %d vanaf.", verschil);
        antwoord = buffer;
    }

    // Stuur terug
    send(client_fd, antwoord, strlen(antwoord), 0);

    close(client_fd);
    close(server_fd);
    return 0;
}