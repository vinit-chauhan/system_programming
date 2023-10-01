#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int
main() {

    int fd = open("data/temp.txt", O_RDONLY);
    char buffer[256];
    while (read(fd, buffer, 256) != 0) {
        write(STDOUT_FILENO, buffer, 256);
    }
    int n = lseek(fd, 0, SEEK_CUR);
    printf("n: %d\n", n);
    return 0;
}