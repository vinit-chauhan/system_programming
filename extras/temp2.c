#include <fcntl.h>
#include <stdio.h>

int
main() {

    int fd = open("data/temp_file.txt", O_CREAT | O_RDWR, 0644);

    char buff[256] = {0};

    printf("Written: %d\n", write(fd, "Hello", 5));

    close(fd);
    fd = open("data/temp_file.txt", O_RDWR, 0644);

    printf("Seek: %d\n", lseek(fd, 5, SEEK_END));
    printf("Written: %d\n", write(fd, "World", 6));

    close(fd);
    fd = open("data/temp_file.txt", O_RDONLY, 0644);

    printf("Seek: %d\n", lseek(fd, 0, SEEK_CUR));
    printf("Seek: %d\n", lseek(fd, -5, SEEK_END));
    printf("Seek: %d\n", lseek(fd, 0, SEEK_CUR));
    printf("Read: %d\n", read(fd, buff, 3));

    printf("%s\n", buff);
    printf("Read: %d\n", read(fd, buff, 5));
    printf("%s\n", buff);

    close(fd);
    return 0;
}
