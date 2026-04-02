#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_CAPACITY 5
#define MAX_ITEMS 20   // for testing (program stops after 20 items)

/*
==========================================
Shared Buffer (Conveyor Belt)
------------------------------------------
- buffer[] stores luggage IDs
- count tracks current number of items
- in/out indexes implement circular queue
==========================================
*/

int buffer[MAX_CAPACITY];
int count = 0;
int in = 0;
int out = 0;

/* Counters for monitoring */
int produced_total = 0;
int consumed_total = 0;

/*
==========================================
Synchronization Primitives
==========================================
*/
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;

/*
==========================================
Producer Thread (Conveyor Loader)
------------------------------------------
- Produces luggage every 2 seconds
- Waits if buffer is full
==========================================
*/
void* producer(void* arg) {
    int id = 1;

    while (id <= MAX_ITEMS) {
        sleep(2);  // simulate loading time

        pthread_mutex_lock(&mutex);

        /* Wait if buffer is full */
        while (count == MAX_CAPACITY) {
            printf("Loader waiting: belt full\n");
            pthread_cond_wait(&not_full, &mutex);
        }

        /* Add luggage to buffer */
        buffer[in] = id;
        in = (in + 1) % MAX_CAPACITY;
        count++;
        produced_total++;

        printf("Loaded luggage ID %d | Belt size: %d\n", id, count);

        id++;

        /* Signal that buffer is not empty */
        pthread_cond_signal(&not_empty);

        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

/*
==========================================
Consumer Thread (Aircraft Loader)
------------------------------------------
- Consumes luggage every 4 seconds
- Waits if buffer is empty
==========================================
*/
void* consumer(void* arg) {
    while (1) {
        sleep(4);  // simulate aircraft loading time

        pthread_mutex_lock(&mutex);

        /* Wait if buffer is empty */
        while (count == 0) {
            if (consumed_total >= MAX_ITEMS) {
                pthread_mutex_unlock(&mutex);
                return NULL;
            }
            printf("Aircraft waiting: belt empty\n");
            pthread_cond_wait(&not_empty, &mutex);
        }

        /* Remove luggage */
        int item = buffer[out];
        out = (out + 1) % MAX_CAPACITY;
        count--;
        consumed_total++;

        printf("Dispatched luggage ID %d | Belt size: %d\n", item, count);

        /* Signal that buffer has space */
        pthread_cond_signal(&not_full);

        pthread_mutex_unlock(&mutex);

        if (consumed_total >= MAX_ITEMS)
            break;
    }

    return NULL;
}

/*
==========================================
Monitoring Thread
------------------------------------------
- Runs every 5 seconds
- Reads shared variables safely
==========================================
*/
void* monitor(void* arg) {
    while (1) {
        sleep(5);

        pthread_mutex_lock(&mutex);

        printf("\n[MONITOR REPORT]\n");
        printf("Total Loaded: %d\n", produced_total);
        printf("Total Dispatched: %d\n", consumed_total);
        printf("Current Belt Size: %d\n\n", count);

        pthread_mutex_unlock(&mutex);

        if (consumed_total >= MAX_ITEMS)
            break;
    }

    return NULL;
}

/*
==========================================
Main Function
==========================================
*/
int main() {
    pthread_t prod_thread, cons_thread, mon_thread;

    pthread_create(&prod_thread, NULL, producer, NULL);
    pthread_create(&cons_thread, NULL, consumer, NULL);
    pthread_create(&mon_thread, NULL, monitor, NULL);

    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);
    pthread_join(mon_thread, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&not_full);
    pthread_cond_destroy(&not_empty);

    printf("Simulation complete.\n");
    return 0;
}
