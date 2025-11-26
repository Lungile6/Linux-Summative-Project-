#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdbool.h>

#include "common.h"

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char username[USERNAME_SIZE];
    bool authenticated = false;

    // Create socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }

    printf("Connected to server. Please authenticate.\n");

    // Authentication Loop
    // The client prompts the user for a username and sends it to the server
    // using an AUTH_REQUEST message. It then waits for an AUTH_SUCCESS or
    // AUTH_FAILURE response from the server. This loop continues until
    // authentication is successful or the server disconnects.
    while (!authenticated) {
        printf("Enter username: ");
        fgets(username, USERNAME_SIZE, stdin);
        username[strcspn(username, "\n")] = 0; // Remove newline character

        Message auth_request;
        auth_request.type = AUTH_REQUEST;
        strncpy(auth_request.payload, username, USERNAME_SIZE);
        send(sock, &auth_request, sizeof(Message), 0);

        Message response;
        ssize_t bytes_received = recv(sock, &response, sizeof(Message), 0);

        if (bytes_received <= 0) {
            printf("Server disconnected during authentication.\n");
            break;
        }

        if (response.type == AUTH_SUCCESS) {
            printf("Authentication successful! Welcome, %s.\n", username);
            authenticated = true;
        } else if (response.type == AUTH_FAILURE) {
            printf("Authentication failed: %s\n", response.payload);
        } else if (response.type == EXAM_ENDED) { // Server full scenario
            printf("Server message: %s\n", response.payload);
            goto cleanup;
        }
    }

    if (!authenticated) {
        goto cleanup;
    }

    // Exam Loop
    // After successful authentication, the client enters the exam session.
    // Message Exchange: The client continuously receives messages from the
    // server. It expects a QUESTION_DELIVERY message, then prompts the user
    // for an answer and sends it back as an ANSWER_SUBMISSION message.
    // It also displays FEEDBACK_CORRECT/INCORRECT messages and ACTIVE_STUDENTS_UPDATEs.
    // The loop breaks when an EXAM_ENDED message is received or the server disconnects.
    Message server_msg;
    while (true) {
        ssize_t bytes_received = recv(sock, &server_msg, sizeof(Message), 0);
        if (bytes_received <= 0) {
            printf("Server disconnected. Exam session ended.\n");
            break;
        }

        if (server_msg.type == QUESTION_DELIVERY) {
            printf("Exam Question: %s\n", server_msg.payload);
            printf("Your answer: ");
            char answer[ANSWER_SIZE];
            fgets(answer, ANSWER_SIZE, stdin);
            answer[strcspn(answer, "\n")] = 0; // Remove newline character

            Message answer_submission;
            answer_submission.type = ANSWER_SUBMISSION;
            strncpy(answer_submission.payload, answer, ANSWER_SIZE);
            send(sock, &answer_submission, sizeof(Message), 0);
        } else if (server_msg.type == FEEDBACK_CORRECT) {
            printf("%s\n", server_msg.payload);
        } else if (server_msg.type == FEEDBACK_INCORRECT) {
            printf("%s\n", server_msg.payload);
        } else if (server_msg.type == ACTIVE_STUDENTS_UPDATE) {
            printf("%s\n", server_msg.payload);
        } else if (server_msg.type == EXAM_ENDED) {
            printf("Exam session ended. Thank you, %s.\n", username);
            break;
        }
    }

cleanup:
    close(sock);
    return 0;
}
