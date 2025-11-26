#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>

#include "common.h"

// Structure to hold client information
typedef struct {
    int socket;
    char username[USERNAME_SIZE];
    bool authenticated;
    pthread_t thread_id;
} ClientSession;

ClientSession clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex to protect access to the clients array and active_clients count
int active_clients = 0;

// Function to broadcast active students list to all connected clients
// This function is responsible for informing all currently authenticated clients
// about the list of other active and authenticated students. It acquires a mutex
// to ensure thread-safe access to the shared `clients` array and `active_clients` count.
void broadcast_active_students() {
    char active_students_list[BUFFER_SIZE];
    strcpy(active_students_list, "Active Students: ");

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket != 0 && clients[i].authenticated) {
            strcat(active_students_list, clients[i].username);
            strcat(active_students_list, " ");
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    Message msg;
    msg.type = ACTIVE_STUDENTS_UPDATE;
    strncpy(msg.payload, active_students_list, BUFFER_SIZE);

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket != 0 && clients[i].authenticated) {
            send(clients[i].socket, &msg, sizeof(Message), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Function to handle each client connection
void *handle_client(void *arg) {
    ClientSession *client = (ClientSession *)arg;
    Message msg;
    ssize_t bytes_received;

    printf("Client connected: Socket %d\n", client->socket);

    // Authentication Loop
    // This loop handles the initial authentication of a connecting client.
    // It continuously receives messages until the client is successfully authenticated
    // or disconnects. Unauthenticated clients are prevented from proceeding to the exam.
    while (!client->authenticated) {
        bytes_received = recv(client->socket, &msg, sizeof(Message), 0);
        if (bytes_received <= 0) {
            printf("Client disconnected during authentication: Socket %d\n", client->socket);
            goto cleanup;
        }

        if (msg.type == AUTH_REQUEST) {
            // Authentication check: In a real-world scenario, this would involve
            // validating credentials against a secure database. For this simulation,
            // we have hardcoded a few valid usernames (student1 to student4).
            if (strcmp(msg.payload, "student1") == 0 || strcmp(msg.payload, "student2") == 0 ||
                strcmp(msg.payload, "student3") == 0 || strcmp(msg.payload, "student4") == 0) {
                client->authenticated = true;
                strncpy(client->username, msg.payload, USERNAME_SIZE);
                msg.type = AUTH_SUCCESS;
                send(client->socket, &msg, sizeof(Message), 0);
                printf("Client %s authenticated. Socket %d\n", client->username, client->socket);

                // Concurrency: After successful authentication, the server increments
                // the `active_clients` count and broadcasts the updated list to all clients.
                // A mutex (`clients_mutex`) is used to protect `active_clients` and the
                // `clients` array from race conditions during concurrent access.
                pthread_mutex_lock(&clients_mutex);
                active_clients++;
                pthread_mutex_unlock(&clients_mutex);
                broadcast_active_students();

            } else {
                msg.type = AUTH_FAILURE;
                send(client->socket, &msg, sizeof(Message), 0);
                printf("Authentication failed for user: %s. Socket %d\n", msg.payload, client->socket);
            }
        } else {
            msg.type = AUTH_FAILURE; // Deny access if not an auth request
            send(client->socket, &msg, sizeof(Message), 0);
        }
    }

    // Exam Loop
    // After successful authentication, the client enters the exam session.
    // The server sends a question and then enters a loop to receive answers
    // and provide real-time feedback.
    // Message Exchange: The server sends a QUESTION_DELIVERY message with the
    // exam question. It then waits for an ANSWER_SUBMISSION message from the client.
    // Upon receiving an answer, it checks for correctness and sends back
    // FEEDBACK_CORRECT or FEEDBACK_INCORRECT messages.
    // For simplicity, a single question is used in this example.
    Message question_msg;
    question_msg.type = QUESTION_DELIVERY;
    strncpy(question_msg.payload, "What is 2+2? (a)3 (b)4 (c)5", QUESTION_SIZE);
    send(client->socket, &question_msg, sizeof(Message), 0);

    while (true) {
        bytes_received = recv(client->socket, &msg, sizeof(Message), 0);
        if (bytes_received <= 0) {
            printf("Client disconnected: %s (Socket %d)\n", client->username, client->socket);
            goto cleanup;
        }

        if (client->authenticated) {
            if (msg.type == ANSWER_SUBMISSION) {
                printf("Answer received from %s: %s\n", client->username, msg.payload);
                if (strcmp(msg.payload, "b") == 0) { // Correct answer
                    msg.type = FEEDBACK_CORRECT;
                    strncpy(msg.payload, "Server: Correct!", BUFFER_SIZE);
                } else {
                    msg.type = FEEDBACK_INCORRECT;
                    strncpy(msg.payload, "Server: Incorrect.", BUFFER_SIZE);
                }
                send(client->socket, &msg, sizeof(Message), 0);
            } else {
                // If authenticated but sending wrong message type, ignore or send error
                printf("Invalid message type from authenticated client %s: %d\n", client->username, msg.type);
            }
        } else {
            // Should not happen if authentication loop works, but for safety
            msg.type = AUTH_FAILURE;
            strncpy(msg.payload, "You are not authenticated.", BUFFER_SIZE);
            send(client->socket, &msg, sizeof(Message), 0);
        }
    }

cleanup:
    close(client->socket);
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket == client->socket) {
            clients[i].socket = 0;
            clients[i].authenticated = false;
            memset(clients[i].username, 0, USERNAME_SIZE);
            active_clients--;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    broadcast_active_students();
    free(client); // Free the dynamically allocated client session
    pthread_exit(NULL);
}


int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Initialize client sessions
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].socket = 0;
        clients[i].authenticated = false;
    }

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the specified port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    // Main server loop to accept new client connections
    // Concurrency: Each new client connection is handled in a separate thread.
    // This allows the server to manage multiple students simultaneously without
    // blocking the main thread from accepting new connections.
    // A mutex is used to protect the `clients` array when adding new clients.
    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue;
        }

        pthread_mutex_lock(&clients_mutex);
        int client_idx = -1;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket == 0) {
                client_idx = i;
                break;
            }
        }

        if (client_idx == -1) {
            printf("Max clients reached. Rejecting new connection.\n");
            Message busy_msg;
            busy_msg.type = EXAM_ENDED; // Using EXAM_ENDED to signify server full
            strncpy(busy_msg.payload, "Server is full. Please try again later.", BUFFER_SIZE);
            send(new_socket, &busy_msg, sizeof(Message), 0);
            close(new_socket);
            pthread_mutex_unlock(&clients_mutex);
            continue;
        }

        // Concurrency: Dynamically allocate a ClientSession for the new client
        // and pass it to the `handle_client` thread. The thread is detached
        // so its resources are automatically reclaimed upon termination.
        ClientSession *new_client = (ClientSession *)malloc(sizeof(ClientSession));
        new_client->socket = new_socket;
        new_client->authenticated = false;
        memset(new_client->username, 0, USERNAME_SIZE);
        clients[client_idx] = *new_client; // Copy the new_client data into the array

        if (pthread_create(&clients[client_idx].thread_id, NULL, handle_client, (void *)&clients[client_idx]) != 0) {
            perror("pthread_create failed");
            // If thread creation fails, clean up the client slot
            clients[client_idx].socket = 0;
            free(new_client);
            close(new_socket);
        } else {
            // Detach the thread so its resources are automatically released when it terminates
            pthread_detach(clients[client_idx].thread_id);
        }

        pthread_mutex_unlock(&clients_mutex);
    }

    close(server_fd);
    return 0;
}
