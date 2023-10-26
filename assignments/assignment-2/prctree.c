#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int OUTPUT = STDOUT_FILENO;

void
user_manual() {
    char page[] =
        "USAGE: prctree [root_process] [process_id1] [process_id2]… \n\
                  [process_id(n)] [OPTION] \n\
                  OPTIONS: \n\
                  -dn: ";
    write(OUTPUT, page, sizeof(page));
}

enum StatType { PID = 0, STATE = 2, PPID = 3 };

int
char_to_int(char c) {
    return (int)(c - '0');
}

char*
next_token(char* str, int* start) {
    if (*start >= strlen(str)) {
        return NULL;
    }
    int end = *start + 1;
    char* tmp;
    while (str[end] != ' ') {
        end++;
    }

    tmp = malloc(sizeof(char) * end - *start + 1);

    memcpy(tmp, str + *start, end - *start);
    *start = end + 1;

    return tmp;
}

char*
get_token_from_index(char* str, int index) {
    int i = 0, cur_index = 0;
    char* token;
    while (cur_index < index) {
        token = next_token(str, &i);
        cur_index++;
    }
    next_token(str, &i);
}

char*
get_stat(int stat_fd, int stat_type) {
    char buf[64];
    int rbyte;

    if ((rbyte = read(stat_fd, buf, 64)) == -1) {
        perror("read");
        return NULL;
    }

    lseek(stat_fd, 0, SEEK_SET);

    return get_token_from_index(buf, stat_type);
}

int
main() {

    int pid = 27803;
    int zombie_process = 27805;

    char *stat_pid, *stat_z_pid;

    int p_fd = open("/proc/27803/stat", O_RDONLY);
    // lseek(p_fd, 0, SEEK_SET);
    int z_fd = open("/proc/27805/stat", O_RDONLY);
    // lseek(z_fd, 0, SEEK_SET);

    printf("stat pid: %s, ppid: %s, mode: %s\n", get_stat(p_fd, PID),
           get_stat(p_fd, PPID), get_stat(p_fd, STATE));

    printf("stat zombie pid: %s, ppid: %s, mode: %s\n", get_stat(z_fd, PID),
           get_stat(z_fd, PPID), get_stat(z_fd, STATE));

    return 0;
}
