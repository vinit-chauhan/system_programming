#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int
main() {
    if (0) {
        printf("true\n");
    } else {
        printf("false\n");
    }

    int fd = open("data/temp_file.txt", O_CREAT | O_WRONLY, 0644);

    printf("Bytes written: %d\n", write(fd, "Hello", 5));
    printf("Bytes written: %d\n", write(fd, -1, 1));
    printf("Bytes written: %d\n", write(fd, " World\n", 7));

    int arr[] = {140725692692816, 7, 1, 0, 5, 6, 7};
    long arr2[] = {140725692692816, 7, 1, 0, 5, 6, 7};
    int buff[1025];
    long* ptr;
    int* ptr2;

    printf("Bytes written: %d\n", write(fd, arr, sizeof(arr)));
    int off = lseek(fd, -1 * sizeof(arr), SEEK_END);
    printf("offset: %d\n", off);
    printf("Bytes read: %d\n", read(fd, buff, 26));
    printf("%d\n", buff[0]);
    close(fd);
    ptr = (long*)arr;
    ptr2 = arr2;

    long num = ptr[0];

    printf("%d\n", arr[0]);
    printf("%ld\n", ptr[0]);
    printf("%ld\n", *ptr2);
    printf("%ld\n", *(ptr2 + 1));

    printf("%d\n", arr[1]);
    printf("%ld\n", ptr[1]);

    printf("ptr    :%ld\n", ptr);
    printf("ptr + 1:%ld\n", ptr + 1);

    printf("ptr2    :%ld\n", ptr2);
    printf("ptr2 + 1:%ld\n", ptr2 + 1);

    printf("arr    :%ld\n", arr);
    printf("arr + 1:%ld\n", arr + 1);

    printBits(sizeof(long), ptr);
    printBits(sizeof(long), ptr + 1);
    printBits(sizeof(long), ptr2);
    printBits(sizeof(int), ptr2);
    printBits(sizeof(int), ptr2 + 1);
    printBits(sizeof(int), arr);
    printBits(sizeof(int), arr + 1);

    printf("%ld\n", num);
    printf("%d", strerror());

    return 0;
}

// Assumes little endian
void
printBits(size_t const size, void const* const ptr) {
    unsigned char* b = (unsigned char*)ptr;
    unsigned char byte;
    int i, j;

    for (i = size - 1; i >= 0; i--) {
        for (j = 7; j >= 0; j--) {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
    }
    puts("");
}

// 0000000000000000000000000000000100000000000000000000000000000000