#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int OUTPUT = STDOUT_FILENO;

void
user_manual() {
    write(OUTPUT,
          "USAGE: prctree [root_process] [process_id1] [process_id2]… "
          "[process_id(n)] [OPTION] \n",
          85);
    write(OUTPUT, "OPTIONS: \n", 10);
    write(OUTPUT,
          "    - dn additionally lists the PIDs of all the non-direct "
          "descendants of process_id1 (only)\n",
          94);
    write(OUTPUT,
          "    - id additionally lists the PIDs of all the immediate "
          "descendants of process_id1\n",
          86);
    write(OUTPUT,
          "    - lp additionally lists the PIDs of all the sibling processes "
          "of process_id1\n",
          82);
    write(OUTPUT,
          "    - sz additionally Lists the PIDs of all sibling processes of "
          "process_id1 that are defunct\n",
          95);
    write(OUTPUT,
          "    - gp additionally lists the PIDs of all the grandchildren of "
          "process_id1\n",
          78);
    write(OUTPUT,
          "    - zz additionally prints the status of process_id1(Defunct / "
          "Not Defunct)\n",
          79);
    write(OUTPUT,
          "    - zc additionally lists the PIDs of all the direct descendants "
          "of process_id1 that are currently in the defunct state\n",
          123);
    write(
        OUTPUT,
        "    - zx additionally lists the PIDs of the direct descendants of "
        "process_id1..process_id[n] that are currently in the defunct state\n",
        134);
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
get_stat(int* stat_fd, int stat_type) {
    char buf[64];
    int rbyte;

    if ((rbyte = read(*stat_fd, buf, 64)) == -1) {
        printf("rbyte: %d, fd: %d\n", rbyte, *stat_fd);
        perror("read");
        return NULL;
    }

    return get_token_from_index(buf, stat_type);
}

char*
get_stat_from_pid(int pid, int stat_type) {
    char buf[64];
    int rbyte;

    char stat_path[64];
    sprintf(stat_path, "/proc/%d/stat", pid);

    int stat_fd = open(stat_path, O_RDONLY);
    if (stat_fd == -1) {
        return 0;
    }

    if ((rbyte = read(stat_fd, buf, 64)) == -1) {
        printf("rbyte: %d, fd: %d\n", rbyte, stat_fd);
        perror("read");
        return NULL;
    }

    close(stat_fd);

    return get_token_from_index(buf, stat_type);
}

int
is_child(int pid, int ppid) {

    char stat_path[64];
    sprintf(stat_path, "/proc/%d/stat", pid);

    int stat_fd = open(stat_path, O_RDONLY);
    if (stat_fd == -1) {
        return 0;
    }

    int cur_ppid = atoi(get_stat(&stat_fd, PPID));
    close(stat_fd);

    if (cur_ppid == ppid) {
        return 1;
    } else if (cur_ppid < ppid) {
        return 0;
    } else {
        return is_child(cur_ppid, ppid);
    }
}

int
main(int argc, char* argv[]) {

    // check if the number of arguments are correct
    if (argc > 8 || argc < 2) {
        user_manual();
        return 0;
    }

    int* pids;
    int pid_count = 0, root_pid = atoi(argv[1]);
    pids = malloc(sizeof(int) * (argc - 2));

    for (int i = 2, j = 0; i < argc; i++) {
        int cur_pid = atoi(argv[i]);
        if (is_child(cur_pid, root_pid)) {
            pids[j] = cur_pid;
            j++;
            pid_count++;
        }
    }

    char* option = argv[argc - 1];
    printf("option:%s, %d\n", option, strcmp(option, "-id"));

    // print pid and ppid of the all the processes
    for (int i = 0; i < pid_count; i++) {
        printf("pid: %s, ppid: %s\n", get_stat_from_pid(pids[i], PID),
               get_stat_from_pid(pids[i], PPID));
    }

    if (strcmp(option, "-dn") == 0) {

    } else if (strcmp(option, "-id") == 0) {
        int pid = pids[0];
        char cmd[1024], buffer[1024];

        sprintf(cmd,
                "pstree -p %d | grep -Eo '([0-9]\+)' | grep -v \"%d\" | sed "
                "'s/[()]//g'",
                pid, pid);

        FILE* fp = popen(cmd, "r");
        if (fp == NULL) {
            perror("popen");
            exit(1);
        }

        while (fgets(buffer, 1024, fp) != NULL) {
            buffer[strlen(buffer) - 1] = '\0';
            printf("%s ", buffer);
        }
        printf("\n");

        pclose(fp);

        // "- lp" additionally lists the PIDs of all the sibling processes of process_id1
    } else if (strcmp(option, "-lp") == 0) {
        int ppid = atoi(get_stat_from_pid(pids[0], PPID));
        char cmd[1024], buffer[1024];

        sprintf(cmd, "pgrep -P %d -d \" \" | grep -v \"%d\" | sed 's/[()]//g'",
                ppid, ppid);

        printf("cmd: %s\n", cmd);

        FILE* fp = popen(cmd, "r");
        if (fp == NULL) {
            perror("popen");
            exit(1);
        }

        while (fgets(buffer, 1024, fp) != NULL) {
            buffer[strlen(buffer) - 1] = '\0';
            printf("%s ", buffer);
        }
        printf("\n");
        pclose(fp);

        //  pgrep -P PPID_HERE -d " "

    } else if (strcmp(option, "-sz") == 0) {

    } else if (strcmp(option, "-gp") == 0) {

    } else if (strcmp(option, "-zz") == 0) {

    } else if (strcmp(option, "-zc") == 0) {

    } else if (strcmp(option, "-zx") == 0) {

    } else {
        user_manual();
        return 0;
    }

    // ========================= TEST 1 =========================
    // int p_fd = 64434, z_fd = 64435;

    // printf("stat pid: %s, ppid: %s, mode: %s\n", get_stat(p_fd, PID),
    //        get_stat(p_fd, PPID), get_stat(p_fd, STATE));

    // printf("stat zombie pid: %s, ppid: %s, mode: %s\n", get_stat(z_fd, PID),
    //        get_stat(z_fd, PPID), get_stat(z_fd, STATE));

    return 0;
}

//      - dn additionally lists the PIDs of all the non-direct descendants of process_id1 (only)
//              pgrep -P PPID -d " "
//      - id additionally lists the PIDs of all the immediate descendants of process_id1
//              pgrep -P PPID -d " "
//      - sz additionally Lists the PIDs of all sibling processes of process_id1 that are defunct
//              pgrep -P PPID_HERE -r Z -d " "
//      - gp additionally lists the PIDs of all the grandchildren of process_id1
//              pgrep -P LOOP_ALL_CHILD -d " "
//      - zz additionally prints the status of process_id1(Defunct / Not Defunct)
//              use stat file to get the status of the process
//      - zc additionally lists the PIDs of all the direct descendants of process_id1 that are currently in the defunct state
//              pgrep -P PID -r Z -d " "
//      - zx additionally lists the PIDs of the direct descendants of process_id1..process_id[n] that are currently in the defunct state
//              pgrep -P LOOP_PIDS -r Z -d " "             :: loop through all the pids

// list all child of a process : pgrep -P 167161 -d " "