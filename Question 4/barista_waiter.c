#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "barista_waiter.h"

// Shared queue and its management variables
int order_queue[QUEUE_SIZE];
int head = 0;
int tail = 0;
int count = 0;

// Mutex and condition variables for synchronization
pthread_mutex_t mutex;
pthread_cond_t producer_cond;
pthread_cond_t consumer_cond;

// Barista thread function (Producer)
void *barista(void *arg) {
    int item = 0;
    while (1) {
        // Acquire the mutex before accessing shared resources
        pthread_mutex_lock(&mutex);

        // If the queue is full, wait for space to become available
        while (count == QUEUE_SIZE) {
            printf("Barista: Queue is full. Waiting for space...\n");
            pthread_cond_wait(&producer_cond, &mutex);
        }

        // Prepare a drink (simulate work)
        printf("Barista: Preparing drink %d...\n", item);
        sleep(BARISTA_TIME);

        // Add the drink to the queue
        order_queue[tail] = item;
        tail = (tail + 1) % QUEUE_SIZE;
        count++;
        printf("Barista: Placed drink %d in queue. Queue size: %d\n", item, count);

        // Signal the waiter that a new drink is available
        pthread_cond_signal(&consumer_cond);

        // Release the mutex
        pthread_mutex_unlock(&mutex);

        item++;
    }
    return NULL;
}

// Waiter thread function (Consumer)
void *waiter(void *arg) {
    while (1) {
        // Acquire the mutex before accessing shared resources
        pthread_mutex_lock(&mutex);

        // If the queue is empty, wait for a drink to become available
        while (count == 0) {
            printf("Waiter: Queue is empty. Waiting for drinks...\n");
            pthread_cond_wait(&consumer_cond, &mutex);
        }

        // Remove a drink from the queue
        int item = order_queue[head];
        head = (head + 1) % QUEUE_SIZE;
        count--;
        printf("Waiter: Serving drink %d. Queue size: %d\n", item, count);

        // Simulate serving time
        sleep(WAITER_TIME);

        // Signal the barista that space is available in the queue
        pthread_cond_signal(&producer_cond);

        // Release the mutex
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    pthread_t barista_tid, waiter_tid;

    // Initialize mutex and condition variables
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&producer_cond, NULL);
    pthread_cond_init(&consumer_cond, NULL);

    // Create threads
    pthread_create(&barista_tid, NULL, barista, NULL);
    pthread_create(&waiter_tid, NULL, waiter, NULL);

    // Join threads (they will run indefinitely in this simulation)
    pthread_join(barista_tid, NULL);
    pthread_join(waiter_tid, NULL);

    // Destroy mutex and condition variables
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&producer_cond);
    pthread_cond_destroy(&consumer_cond);

    return 0;
}
