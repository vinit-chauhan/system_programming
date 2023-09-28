#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void
print_manual(int output_fd) {
    char page[] = "Usage: dircpmvlist SOURCE DEST OPTIONS [EXTENSIONS]   \n\
    \n\
Options:    \n\
    -cp: Copy files from SOURCE to DEST  \n\
    -mv: Move files from SOURCE to DEST  \n\
    \n\
Extensions: (Upto 6 extensions)     \n\
    List of extensions to copy/move \n\
    ex. -cp .txt .pdf .docx \n\
    \n\
Example:   \n\
    $ dircpmvlist /home/user/Downloads /home/user/Documents -cp .txt .pdf .docx \n\
    $ dircpmvlist /home/user/Downloads /home/user/Documents -mv .txt .pdf .docx \n";

    write(output_fd, page, sizeof(page));
}

int
main(int argc, char* argv[]) {
    char *source_dir, *destination_dir, *options;
    char** extensions;
    int OUTPUT = STDOUT_FILENO;

    // Check for number of arguments and throw error if not in range
    if (argc < 4) {
        write(STDERR_FILENO, "[Error]: Too few arguments.\n", 28);
        print_manual(OUTPUT);
        exit(001);
    } else if (argc > 10) {
        write(STDERR_FILENO, "[Error]: Too many arguments.\n", 29);
        print_manual(OUTPUT);
        exit(002);
    }

    // Parse command line arguments
    source_dir = argv[1];
    destination_dir = argv[2];
    options = argv[3];

    // iterate through all extensions and store them in an array
    extensions = malloc((argc - 4) * sizeof(char*));
    for (int i = 4; i < argc; i++) {
        extensions[i - 4] = argv[i];
    }

    printf("Source: %s\n", source_dir);
    printf("Destination: %s\n", destination_dir);
    printf("Options: %s\n", options);
    printf("Extensions: ");
    for (int i = 0; i < argc - 4; i++) {
        printf("%s ", extensions[i]);
    }
}