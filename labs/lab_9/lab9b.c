#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 3

void*(func_1)(void* msg) {
    // print the message from the server.
    printf("server: %s", (char*)msg);
    // print the thread id
    printf(" to thread 1: %lu\n", pthread_self());

    return NULL;
}

void*(func_2)(void* msg) {
    // print the message from the server.
    printf("server: %s", (char*)msg);
    // print the thread id
    printf(" to thread 2: %lu\n", pthread_self());

    return NULL;
}

void*(func_3)(void* msg) {
    // print the message from the server.
    printf("server: %s", (char*)msg);
    // print the thread id
    printf(" to thread 3: %lu\n", pthread_self());

    return NULL;
}

int
main() {
    // create array to store thread ids
    pthread_t t_ids[NUM_THREADS];
    char* msg = "Welcome to COMP 8567";

    // create thread 1
    pthread_create(&t_ids[1], NULL, func_2, msg);
    // wait for thread 1 to finish
    pthread_join(t_ids[1], NULL);

    // do the same for thread 2 and 3
    pthread_create(&t_ids[2], NULL, func_3, msg);
    pthread_join(t_ids[2], NULL);
    pthread_create(&t_ids[0], NULL, func_1, msg);
    pthread_join(t_ids[0], NULL);

    // print main thread id
    printf("server: main thread is %lu\n", pthread_self());
    return 0;
}