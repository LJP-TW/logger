#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

void usage(void)
{
    printf(
        "usage: ./logger [-o file] [-p sopath] [--] cmd [cmd args ...]\n"
        "        -p: set the path to logger.so, default = ./logger.so\n"
        "        -o: print output to file, print to \"stderr\" if no file specified\n"
        "        --: separate the arguments for logger and for the command\n");
}

int main(int argc, char **argv, char **envp)
{
    char **tmp, **new_envp;
    char *env_ldpreload;
    char *env_file;
    char *sopath = "./logger.so";
    char *file;
    int opt, parsed, envp_cnt, file_fd;

    if (argc < 2) {
        goto NO_COMMAND;
    }

    file = NULL;
    parsed = 0;

    while (!parsed && (opt = getopt(argc, argv, "o:p:-")) != -1) {
        switch (opt) {
        case 'o':
            file = strdup(optarg);
            break;
        case 'p':
            sopath = strdup(optarg);
            break;
        case '-':
            parsed = 1;
            break;
        default:
            usage();
            return 1;
        }
    }

    /* Prepare argv */
    argv = &argv[optind];

    if (!argv[0]) {
        goto NO_COMMAND;
    }

    /* Open file */
    // Default: stderr
    file_fd = 2;
    
    if (file) {
        file_fd = creat(file, 00644);
    }

    if (file_fd == 2) {
        // Redirect fd 2 to another fd
        file_fd = dup(2);
    }

    /* Prepare envp */
    tmp = envp;
    envp_cnt = 0;
    while (*tmp++) {
        envp_cnt += 1;
    }

    new_envp = malloc(sizeof(char *) * (envp_cnt + 3));

    for (int i = 0; i < envp_cnt; ++i) {
        new_envp[i + 2] = envp[i];
    }

    new_envp[envp_cnt + 2] = NULL;

    env_ldpreload = malloc(sizeof(char) * 0x200);
    snprintf(env_ldpreload, 0x200, "LD_PRELOAD=%s", sopath);
    new_envp[0] = env_ldpreload;

    env_file = malloc(sizeof(char) * 0x200);
    snprintf(env_file, 0x200, "OUTPUT_FILE_FD=%d", file_fd);
    new_envp[1] = env_file;

    /* Run it! */
    execvpe(argv[0], argv, new_envp);
    /* Never reach */

NO_COMMAND:
    printf("no command given.\n");
    return 1;
}