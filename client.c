#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int sock;
    struct sockaddr_in server;
    char buffer[512];
    char id[20];

    sock = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("Connection failed");
        return 1;
    }

    /* =========================
       SEND ID
    ========================= */
    printf("Enter Library ID: ");
    scanf("%s", id);

    send(sock, id, strlen(id) + 1, 0);

    /* =========================
       RECEIVE AUTH RESULT
    ========================= */
    int n = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (n <= 0) {
        printf("Server disconnected\n");
        close(sock);
        return 1;
    }

    buffer[n] = '\0';

    if (strcmp(buffer, "AUTH_OK") != 0) {
        printf("Authentication failed\n");
        close(sock);
        return 0;
    }

    printf("Authenticated successfully\n");

    /* =========================
       RECEIVE BOOK LIST
    ========================= */
    n = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (n <= 0) {
        printf("Server disconnected\n");
        close(sock);
        return 1;
    }

    buffer[n] = '\0';
    printf("%s\n", buffer);

    /* =========================
       SEND CHOICE
    ========================= */
    int choice;
    printf("Enter book number to reserve: ");
    scanf("%d", &choice);

    sprintf(buffer, "%d", choice);
    send(sock, buffer, strlen(buffer) + 1, 0);

    /* =========================
       RECEIVE RESPONSE
    ========================= */
    n = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (n <= 0) {
        printf("Server disconnected during reservation\n");
        close(sock);
        return 1;
    }

    buffer[n] = '\0';
    printf("Server: %s\n", buffer);

    printf("Session closed. Goodbye, %s\n", id);

    close(sock);
    return 0;
}
