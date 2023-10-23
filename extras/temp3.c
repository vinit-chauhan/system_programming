#include <fcntl.h>
#include <stdio.h>

int
main() {
    char buffer;

    int fd = open("data/temp.txt", O_RDONLY);

    printf("%d\n", sizeof(buffer));
    int bytes_read = read(fd, buffer, sizeof(buffer) - 1);

    printf("bytes_read: %d\n", bytes_read);
    printf("buffer: %s\n", buffer);

    return 0;
}