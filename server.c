#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define MAX_CLIENTS 5
#define MAX_BOOKS 5

/* ==========================================
   Shared Data
========================================== */

// Predefined valid IDs
char *valid_ids[] = {"1001", "1002", "1003", "1004", "1005"};
int num_ids = 5;

// Book list
char *books[MAX_BOOKS] = {
    "Book_A", "Book_B", "Book_C", "Book_D", "Book_E"
};

// 0 = available, 1 = reserved
int book_status[MAX_BOOKS] = {0};

// Active users
char active_users[MAX_CLIENTS][10];
int active_count = 0;

/* Mutex */
pthread_mutex_t lock;

/* ==========================================
   Utility Functions
========================================== */

int is_valid_id(char *id) {
    for (int i = 0; i < num_ids; i++) {
        if (strcmp(id, valid_ids[i]) == 0)
            return 1;
    }
    return 0;
}

void add_active_user(char *id) {
    if (active_count < MAX_CLIENTS) {
        strcpy(active_users[active_count++], id);
    }
}

void remove_active_user(char *id) {
    for (int i = 0; i < active_count; i++) {
        if (strcmp(active_users[i], id) == 0) {
            for (int j = i; j < active_count - 1; j++)
                strcpy(active_users[j], active_users[j+1]);
            active_count--;
            break;
        }
    }
}

void send_book_list(int sock) {
    char buffer[512] = "Available books:\n";

    for (int i = 0; i < MAX_BOOKS; i++) {
        char line[100];
        sprintf(line, "%d. %s [%s]\n",
            i,
            books[i],
            book_status[i] ? "RESERVED" : "AVAILABLE");
        strcat(buffer, line);
    }

    send(sock, buffer, strlen(buffer), 0);
}

void print_status() {
    printf("\n--- SERVER STATUS ---\n");

    printf("Active users:\n");
    for (int i = 0; i < active_count; i++)
        printf(" - %s\n", active_users[i]);

    printf("Books:\n");
    for (int i = 0; i < MAX_BOOKS; i++)
        printf(" %s : %s\n",
            books[i],
            book_status[i] ? "RESERVED" : "AVAILABLE");

    printf("----------------------\n\n");
}

/* ==========================================
   Client Thread Handler
========================================== */

void *handle_client(void *arg) {
    int client_sock = *(int *)arg;
    free(arg);

    char buffer[256];
    char user_id[20];

    /* =========================
       RECEIVE USER ID
    ========================= */
    int n = recv(client_sock, buffer, sizeof(buffer) - 1, 0);

    if (n <= 0) {
        printf("Client disconnected before sending ID\n");
        close(client_sock);
        return NULL;
    }

    buffer[n] = '\0';
    strcpy(user_id, buffer);

    printf("Received ID: %s\n", user_id);

    pthread_mutex_lock(&lock);

    if (!is_valid_id(user_id)) {
        send(client_sock, "AUTH_FAIL", 9, 0);
        pthread_mutex_unlock(&lock);
        close(client_sock);
        return NULL;
    }

    send(client_sock, "AUTH_OK", 7, 0);

    add_active_user(user_id);
    print_status();

    pthread_mutex_unlock(&lock);

    /* =========================
       SEND BOOK LIST
    ========================= */
    send_book_list(client_sock);

    /* =========================
       RECEIVE BOOK CHOICE
    ========================= */
    n = recv(client_sock, buffer, sizeof(buffer) - 1, 0);

    if (n <= 0) {
        printf("Client disconnected during reservation\n");

        pthread_mutex_lock(&lock);
        remove_active_user(user_id);
        pthread_mutex_unlock(&lock);

        close(client_sock);
        return NULL;
    }

    buffer[n] = '\0';
    int choice = atoi(buffer);

    pthread_mutex_lock(&lock);

    if (choice >= 0 && choice < MAX_BOOKS) {
        if (book_status[choice] == 0) {
            book_status[choice] = 1;
            send(client_sock, "RESERVED", 8, 0);
            printf("User %s reserved %s\n", user_id, books[choice]);
        } else {
            send(client_sock, "ALREADY_RESERVED", 16, 0);
            printf("User %s attempted reserved book %s\n", user_id, books[choice]);
        }
    } else {
        send(client_sock, "INVALID_CHOICE", 14, 0);
    }

    remove_active_user(user_id);
    print_status();

    pthread_mutex_unlock(&lock);

    close(client_sock);
    return NULL;
}

/* ==========================================
   Main Server
========================================== */

int main() {
    int server_fd, *client_sock;
    struct sockaddr_in server, client;
    socklen_t c;

    pthread_mutex_init(&lock, NULL);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        perror("Socket failed");
        exit(1);
    }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    listen(server_fd, MAX_CLIENTS);

    printf("Server started on port %d...\n", PORT);

    c = sizeof(struct sockaddr_in);

    while (1) {
        client_sock = malloc(sizeof(int));
        *client_sock = accept(server_fd, (struct sockaddr*)&client, &c);

        if (*client_sock < 0) {
            perror("Accept failed");
            free(client_sock);
            continue;
        }

        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, client_sock);
        pthread_detach(tid);
    }

    close(server_fd);
    pthread_mutex_destroy(&lock);

    return 0;
}
