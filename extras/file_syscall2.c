#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int
main() {

    umask(0133);

    int fd;
    if ((fd = open("data/temp.txt", O_CREAT | O_RDWR, 0777)) == -1) {
        write(STDERR_FILENO, "Error opening file.\n", 20);
        exit(1);
    } else {
        if (write(fd, "Hello World!\n", 13) == -1) {
            write(STDERR_FILENO, "Error writing to file.\n", 23);
            exit(1);
        }
    }
    close(fd);

    fd = open("data/temp.txt", O_RDWR);

    char buffer[256];
    int read_size = read(fd, buffer, 20);
    if (read_size == -1) {
        write(STDERR_FILENO, "Error reading file.\n", 20);
        exit(1);
    } else if (read_size > 0) {
        write(STDOUT_FILENO, buffer, read_size);
    }

    // move cursor to more than current range 33rd byte
    int n = lseek(fd, 33000000, SEEK_SET);
    printf("n: %d\n", n);

    if (write(fd, "***!", 4) == -1) {
        write(STDERR_FILENO, "Error writing to file.\n", 23);
        exit(1);
    }

    // Print file contents.
    lseek(fd, 0, SEEK_SET);
    if (read(fd, buffer, 256) == -1) {
        write(STDERR_FILENO, "Error reading file.\n", 20);
        exit(1);
    } else {
        write(STDOUT_FILENO, buffer, read_size);
    }

    // Replace NULL with space.
    lseek(fd, 0, SEEK_SET);
    while (read(fd, buffer, 1) != 0) {
        lseek(fd, -1, SEEK_CUR);
        (buffer[0] == NULL) ? write(fd, " ", 1) : write(fd, buffer, 1);
    }

    close(fd);
    return 0;
}