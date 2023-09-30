// Name: Vinit Hemantbhai Chauhan
// Student Id: 110123359

#define _XOPEN_SOURCE 500

#include <fcntl.h>
#include <ftw.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *source_dir, *destination_dir;
char** extensions;
int extension_count = 0;

enum Options { OPTION_COPY = 0, OPTION_MOVE = 1, ERROR = -1 };

int option = ERROR;
int OUTPUT = STDOUT_FILENO;

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
init(int argc, char* argv[]) {
    // Check for number of arguments and throw error if not in range
    if (argc < 4) {
        write(STDERR_FILENO, "[Error]: Too few arguments.\n", 28);
        print_manual(OUTPUT);
        return -1;
    } else if (argc > 10) {
        write(STDERR_FILENO, "[Error]: Too many arguments.\n", 29);
        print_manual(OUTPUT);
        return -1;
    }

    // Parse command line arguments
    source_dir = argv[1];
    destination_dir = argv[2];

    option = (strcmp(argv[3], "-cp") == 0) ? OPTION_COPY : (strcmp(argv[3], "-mv") == 0 ? OPTION_MOVE : ERROR);

    // check if options are valid
    if (option == -1) {
        write(STDERR_FILENO, "[Error]: Invalid option.\n", 25);
        print_manual(OUTPUT);
        return -1;
    }

    // iterate through all extensions and store them in an array
    extension_count = argc - 4;
    extensions = malloc(extension_count * sizeof(char*));
    for (int i = 4; i < argc; i++) {
        extensions[i - 4] = argv[i];
    }

    return 0;
}

int
check_file_conditions() {
    // check if source directory exists
    if (access(source_dir, F_OK) == -1) {
        write(STDERR_FILENO, "[Error]: Source directory does not exist.\n", 42);
        return -1;
    }

    // check if destination already exist or not
    if (access(destination_dir, F_OK) == -1) {
        open(destination_dir, O_CREAT | O_RDWR, 0777);
    }
    if (access(destination_dir, W_OK) == -1) {
        write(STDERR_FILENO, "[Error]: Destination directory is not writable.\n", 48);
        return -1;
    }

    // check if source file is readable
    if (access(source_dir, R_OK) == -1) {
        write(STDERR_FILENO, "[Error]: Source directory is not readable.\n", 43);
    }

    // get path of home directory
    char* home_dir = getenv("HOME");
    // check if source_dir belongs to home directory hierarchy
    char* abs_source_dir = realpath(source_dir, NULL);
    if (strncmp(abs_source_dir, home_dir, strlen(home_dir)) != 0) {
        write(STDERR_FILENO, "[Error]: Source directory does not belong to home directory hierarchy.\n", 72);
        return -1;
    }

    // check if destination_dir belongs to home directory hierarchy
    char* abs_destination_dir = realpath(destination_dir, NULL);
    if (strncmp(abs_destination_dir, home_dir, strlen(home_dir)) != 0) {
        write(STDERR_FILENO, "[Error]: Destination directory does not belong to home directory hierarchy.\n", 77);
        return -1;
    }
}

int
copy_path(const char* fpath, const struct stat* sb, int typeflag, struct FTW* ftwbuf) {

    if (typeflag == FTW_F) {
        char ext[8];
        for (int i = strlen(fpath) - 1, j = 0; i > 0 && j < 8 && fpath[i] != '.'; i--, j++) {
            ext[j] = fpath[i];
        }

        for (int i = 0; i < extension_count; i++) {
            if (strcmp(ext, extensions[i]) == 0) {
                printf("Copying %s\n", fpath);
                // do_copy()
                break;
            }
        }
    } else if (typeflag == FTW_D) {
        // create the directory
        printf("Creating %s\n", fpath);
    }

    return 0;
}

int
copy_dir() {
    printf("Copying Files...\n");
    // copy directory and its content
    nftw(source_dir, copy_path, 10, FTW_PHYS);
    // print a list of copy files, maybe?
    return -1;
}

int
move_dir() {
    printf("-mv");
    // move directory and its content
    // print a list of moved files, maybe?
    return -1;
}

int
main(int argc, char* argv[]) {

    // Check arguments and assign its values
    if (init(argc, argv) == -1) {
        exit(1);
    }

    // Check for file path related conditions
    if (check_file_conditions() == -1) {
        exit(1);
    }

    // Check the option and perform tasks accordingly
    switch (option) {
        case OPTION_COPY:
            if (copy_dir() == -1) {
                exit(1);
            }
        case OPTION_MOVE:
            if (move_dir() == -1) {
                exit(1);
            }
        default: break;
    }

    return 0;
}