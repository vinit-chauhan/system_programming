#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main()
{
    printf("File descriptors:\n");
    printf("%d\n", STDIN_FILENO);
    printf("%d\n", STDOUT_FILENO);
    printf("%d\n", STDERR_FILENO);

    int fd;
    // If file does not exist, it will give an error
    if ((fd = open("data/lorem_ipsum.txt", O_RDONLY)) == -1)
    {
        write(STDERR_FILENO, "Error opening file.\n", 20);
    }
    else
    {
        char buffer[100];
        int bytes_read;
        do
        {
            bytes_read = read(fd, buffer, 100);
            if (bytes_read == -1)
            {
                write(STDERR_FILENO, "Error reading file.\n", 20);
            }
            else if (bytes_read > 0)
            {
                write(STDOUT_FILENO, buffer, bytes_read);
            }
        } while (bytes_read != 0 || bytes_read == -1);

        if (close(fd) == -1)
        {
            write(STDERR_FILENO, "Error closing file.\n", 20);
        }
    }

    // Print to stderr : run and redirect `2> error.txt`
    // write(STDERR_FILENO, "Hello World!\n", 13);

    return 0;
}