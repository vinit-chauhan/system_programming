#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void*
func(void* p) {
    // sleep(1);
    printf("Message from thread %d\n", pthread_self());
    pthread_exit(NULL);
    return NULL;
}

main() {
    pthread_t t1, t2, t3, t4; // declare thread
    pthread_create(&t1, NULL, func, NULL);
    pthread_create(&t2, NULL, func, NULL);
    pthread_create(&t3, NULL, func, NULL);
    pthread_create(&t4, NULL, func, NULL);
    // sleep(1);
    printf("From the main function\n");
}
