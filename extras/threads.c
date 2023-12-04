#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void*
handler(void* arg) {
    arg = "Hello from thread";
    printf("Done thread: %lu\n", pthread_self());
    // return arg;
    pthread_exit(arg);
}

void*
handler_int(void* arg) {
    int* temp = malloc(sizeof(int));
    arg = temp;
    *temp = 10;
    printf("In handler_int: %d\n", *(int*)arg);
    pthread_exit(arg);
}

int
main() {
    pthread_t tid;
    int* ret_int = malloc(sizeof(int));
    *ret_int = 0;
    char* ret = "\0";

    // pthread_create(&tid, NULL, &handler, ret);
    pthread_create(&tid, NULL, &handler_int, &ret_int);

    // printf("ret: %s\n", ret);
    // pthread_join(tid, NULL);
    // pthread_join(tid, &ret_val);
    pthread_join(tid, &ret_int);

    // printf("ret: %s\n", ret);
    printf("ret_int: %d\n", *ret_int);
    printf("Hello from main\n");

    return 0;
}