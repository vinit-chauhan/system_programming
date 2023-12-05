#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define THREAD_COUNT 10 // Number of threads to create
int num = 10;           // Global variable
pthread_mutex_t mutex;  // Mutex to protect the global variable 'num'

void*
decrement(void* arg) {

    if (pthread_detach(pthread_self()) == 0) {
        printf("Detached Thread: num: %d\n", num);
    } else {
        // Acquire the mutex lock before updating 'num'
        pthread_mutex_lock(&mutex);
        // Update 'num'
        num--;
        // Release the mutex lock after updating 'num'
        pthread_mutex_unlock(&mutex);
        printf("Joinable Thread: num: %d\n", num);
    }

    pthread_exit(NULL);
}

int
main(void) {
    pthread_attr_t attr;
    pthread_t threads[THREAD_COUNT] = {0};

    // Initialize the mutex
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        printf("\nMutex initialization failed\n");
        return 1;
    }

    for (int i = 0; i < 10; i++) {
        // Initialize the attribute object
        pthread_attr_init(&attr);

        // Set the attribute object to create joinable or detached threads
        int isDetached = i % 2 ? 0 : 1;

        if (isDetached) {
            // Set the attribute object to create detached threads
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        } else {
            // Set the attribute object to create detached threads
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
            printf("Creating joinable thread\n");
        }

        pthread_t tid;
        // Create the thread
        pthread_create(&tid, &attr, decrement, NULL);
        threads[i] = tid;
    }

    // Wait for all joinable threads to finish
    for (int i = 0; i < 10; i++) {
        pthread_join(threads[i], NULL);
    }

    // Destroy the attribute object once it's no longer needed
    pthread_attr_destroy(&attr);

    printf("Final Value of num: %d\n", num);

    // Destroy the mutex once it's no longer needed
    pthread_mutex_destroy(&mutex);

    return 0;
}