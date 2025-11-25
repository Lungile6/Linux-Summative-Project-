#ifndef BARISTA_WAITER_H
#define BARISTA_WAITER_H

#include <pthread.h>

// Constants
#define QUEUE_SIZE 8
#define BARISTA_TIME 4  // seconds
#define WAITER_TIME 3   // seconds

// Shared queue and its management variables
extern int order_queue[QUEUE_SIZE];
extern int head;
extern int tail;
extern int count;

// Mutex and condition variables for synchronization
extern pthread_mutex_t mutex;
extern pthread_cond_t producer_cond;
extern pthread_cond_t consumer_cond;

// Function prototypes
void *barista(void *arg);
void *waiter(void *arg);

#endif // BARISTA_WAITER_H
