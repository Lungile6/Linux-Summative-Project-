
#ifndef COMMON_H
#define COMMON_H

#define PORT 8080
#define MAX_CLIENTS 4
#define BUFFER_SIZE 1024
#define USERNAME_SIZE 50
#define QUESTION_SIZE 256
#define ANSWER_SIZE 50

typedef enum {
    AUTH_REQUEST,
    AUTH_SUCCESS,
    AUTH_FAILURE,
    QUESTION_REQUEST,
    QUESTION_DELIVERY,
    ANSWER_SUBMISSION,
    FEEDBACK_CORRECT,
    FEEDBACK_INCORRECT,
    ACTIVE_STUDENTS_UPDATE,
    EXAM_ENDED
} MessageType;

typedef struct {
    MessageType type;
    char payload[BUFFER_SIZE];
} Message;

#endif // COMMON_H
