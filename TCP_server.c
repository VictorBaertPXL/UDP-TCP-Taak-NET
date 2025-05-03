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

    // Optie instellen om poort snel te hergebruiken
    int optval = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("setsockopt(SO_REUSEADDR) mislukt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Zet servergegevens
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind mislukt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 1) < 0) {
        perror("Listen mislukt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Wachten op verbinding...\n");
    client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) {
        perror("Accept mislukt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Verbinding geaccepteerd!\n");

    // Genereer geheim getal
    srand(time(NULL));
    geheim_getal = rand() % 100 + 1;
    printf("Geheim getal: %d\n", geheim_getal);

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        ssize_t received = recv(client_fd, buffer, sizeof(buffer)-1, 0);
        if (received <= 0) {
            printf("Client verbinding verbroken of fout bij ontvangen\n");
            break;
        }

        buffer[received] = '\0';
        int gok = atoi(buffer);
        printf("Client gokte: %d\n", gok);

        const char *antwoord;
        if (gok == geheim_getal)
        {
            antwoord = "Perfect! Je hebt het goed!";
            send(client_fd, antwoord, strlen(antwoord), 0);
            break; // juiste gok, stop de loop
        }
        else
        {
            int verschil = abs(geheim_getal - gok);
            sprintf(buffer, "Niet juist. Je zat er %d vanaf.", verschil);
            antwoord = buffer;
            send(client_fd, antwoord, strlen(antwoord), 0);
        }
    }

    close(client_fd);
    close(server_fd);
    return 0;
}
