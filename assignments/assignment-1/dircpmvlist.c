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

char *source_dir, *destination_dir, *relative_source_path;
char** extensions;
int extension_count = 0;

enum Options { OPTION_COPY = 0, OPTION_MOVE = 1, ERROR = -1 };

int option = ERROR;
int OUTPUT = STDOUT_FILENO;

// Prints the manual page for users
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

// Get command line arguments and check for corner cases
int
get_arguments(int argc, char* argv[]) {
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
    source_dir = malloc(sizeof(char) * strlen(argv[1]));
    strcpy(source_dir, argv[1]);

    // remove trailing '/'
    if (source_dir[strlen(source_dir) - 1] == '/') {
        source_dir[strlen(source_dir) - 1] = '\0';
    }

    destination_dir = malloc(sizeof(char) * strlen(argv[2]));
    strcpy(destination_dir, argv[2]);

    // remove trailing '/'
    if (destination_dir[strlen(destination_dir) - 1] == '/') {
        destination_dir[strlen(destination_dir) - 1] = '\0';
    }

    option = (strcmp(argv[3], "-cp") == 0)
                 ? OPTION_COPY
                 : (strcmp(argv[3], "-mv") == 0 ? OPTION_MOVE : ERROR);

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

// Check for file existence and its permissions
int
check_file_conditions() {
    // check if source directory exists
    if (access(source_dir, F_OK) == -1) {
        write(STDERR_FILENO, "[Error]: Source directory does not exist.\n", 42);
        return -1;
    }

    // check if destination already exist or not
    if (access(destination_dir, F_OK) == -1) {
        if (mkdir(destination_dir, 0777) == -1) {
            write(STDERR_FILENO, "[Error]: Can't create directory\n", 32);
            return -1;
        }
    }
    if (access(destination_dir, W_OK) == -1) {
        write(STDERR_FILENO,
              "[Error]: Destination directory is not writable.\n", 48);
        return -1;
    }

    char* last_dir;
    // last directory from the source
    for (int i = strlen(source_dir); i > 0; i--) {
        if (source_dir[i] == '/') {
            last_dir = source_dir + i;
            break;
        }
    }

    if (mkdir(strcat(destination_dir, last_dir), 0777) == -1) {
        write(STDERR_FILENO, "[Error]: Can't create directory\n", 32);
        return -1;
    }
    if (access(destination_dir, W_OK) == -1) {
        write(STDERR_FILENO,
              "[Error]: Destination directory is not writable.\n", 48);
        return -1;
    }

    // check if source file is readable
    if (access(source_dir, R_OK) == -1) {
        write(STDERR_FILENO, "[Error]: Source directory is not readable.\n",
              43);
        return -1;
    }

    // get path of home directory
    char* home_dir = getenv("HOME");
    // check if source_dir belongs to home directory hierarchy
    char* abs_source_dir = realpath(source_dir, NULL);
    if (strncmp(abs_source_dir, home_dir, strlen(home_dir)) != 0) {
        write(STDERR_FILENO,
              "[Error]: Source directory does not belong to home directory "
              "hierarchy.\n",
              72);
        return -1;
    }

    // check if destination_dir belongs to home directory hierarchy
    char* abs_destination_dir = realpath(destination_dir, NULL);
    if (strncmp(abs_destination_dir, home_dir, strlen(home_dir)) != 0) {
        write(STDERR_FILENO,
              "[Error]: Destination directory does not belong to home "
              "directory hierarchy.\n",
              77);
        return -1;
    }
}

//  Extract File name from the path
char*
extract_name(const char* path) {
    char* name = malloc(sizeof(char) * 256);
    int i = strlen(path) - 1;
    for (; i >= 0 && path[i] != '/'; i--)
        ;
    strcpy(name, path + i + 1);
    return name;
}

// copy file content to another file.
int
do_copy(const char* src, char* dest, mode_t mode) {
    int fd_src = open(src, O_RDONLY);
    int fd_dest = open(dest, O_CREAT | O_WRONLY, mode);

    char buf[BUFSIZ];
    int read_size = 0;

    do {
        if (read_size = read(fd_src, buf, BUFSIZ), read_size == -1) {
            return -1;
        }

        if (write(fd_dest, buf, read_size) == -1) {
            return -1;
        }
    } while (read_size > 0);

    return 0;
}

// function for working (copy/move) on a file
int
do_operation(const char* fpath, char* dest_path, const struct stat* sb) {
    printf("Working on: %s\n", fpath);
    if (do_copy(fpath, dest_path, sb->st_mode) == -1) {
        return -1;
    }

    if (option == OPTION_MOVE) {
        if (remove(fpath) == -1) {
            return -1;
        }
    }
    return 0;
}

int
callback_func(const char* fpath, const struct stat* sb, int typeflag,
              struct FTW* ftwbuf) {
    relative_source_path = (char*)(fpath + strlen(source_dir));
    char* dest_path = strcpy(malloc(sizeof(char) * 256), destination_dir);
    strcat(dest_path, relative_source_path);

    if (strlen(relative_source_path) == 0) {
        return 0;
    }

    // If its a file and extension is in the list of extensions, copy the file
    if (typeflag == FTW_F) {
        char ext[8];

        // Extract the extension form file path
        int i = strlen(dest_path) - 1;
        for (; i > 0 && dest_path[i] != '.'; i--)
            ;
        strcpy(ext, dest_path + i + 1);

        if (extension_count
            == 0) { // If no extensions are provided, copy all the files.
            if (do_operation(fpath, dest_path, sb) == -1) {
                write(STDERR_FILENO, "[Error]: Could not copy files.\n", 31);
            }
        } else { //  Check if the extension is in the list of extensions
            for (int i = 0; i < extension_count; i++) {
                if (strcmp(ext, extensions[i]) == 0) {
                    if (do_operation(fpath, dest_path, sb) == -1) {
                        write(STDERR_FILENO, "[Error]: Could not copy files.\n",
                              31);
                    }
                    break;
                }
            }
        }
    } else if (typeflag == FTW_D) { // If Its a directory, create the directory
        mkdir(dest_path, sb->st_mode);
    }

    return 0;
}

int
cleanup_callback_func(const char* fpath, const struct stat* sb, int typeflag,
                      struct FTW* ftwbuf) {
    rmdir(fpath);
    return 0;
}

int
del_callback_func(const char* fpath, const struct stat* sb, int typeflag,
                  struct FTW* ftwbuf) {
    if (typeflag == FTW_D || typeflag == FTW_DP) {
        rmdir(fpath);
    } else if (typeflag == FTW_F) {
        remove(fpath);
    }

    return 0;
}

int
main(int argc, char* argv[]) {

    // Check arguments and assign its values
    if (get_arguments(argc, argv) == -1) {
        exit(1);
    }

    // Check for file path related conditions
    if (check_file_conditions() == -1) {
        exit(1);
    }

    // Check the option and perform tasks accordingly
    switch (option) {
        case OPTION_COPY:
            printf("Copying following Files:\n");
            if (nftw(source_dir, callback_func, 64, FTW_PHYS) == -1) {
                write(STDERR_FILENO, "[Error]: Could not copy files.\n", 31);
                exit(1);
            }
            // Clean up the destination directory
            nftw(destination_dir, cleanup_callback_func, 64, FTW_DEPTH);
            break;
        case OPTION_MOVE:
            printf("Moving following Files:\n");
            if (nftw(source_dir, callback_func, 64, FTW_PHYS) == -1) {
                write(STDERR_FILENO, "[Error]: Could not move files.\n", 31);
                exit(1);
            }

            // Clean up the destination directory
            if (nftw(destination_dir, cleanup_callback_func, 64, FTW_DEPTH)
                == -1) {
                write(STDERR_FILENO, "[Error]: Could not move files.\n", 31);
                exit(1);
            }

            // remove source directory
            if (nftw(source_dir, del_callback_func, 64, FTW_DEPTH) == -1) {
                write(STDERR_FILENO, "[Error]: Could not move files.\n", 31);
                exit(1);
            }
            break;
        default: break;
    }

    printf("Operation Successful.\n");
    return 0;
}