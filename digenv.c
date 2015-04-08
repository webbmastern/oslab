#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
struct command
{
    const char **argv;
};
/* Helper function that spawns processes */
int spawn_proc (int in, int out, struct command *cmd) {
    pid_t pid;
    if ((pid = fork ()) == 0) {
        if (in != 0) {
            dup2 (in, 0);
            close (in);
        }
        if (out != 1) {
            dup2 (out, 1);
            close (out);
        }
        return execvp (cmd->argv [0], (char * const *)cmd->argv);
    }
    return pid;
}
/* Helper function that forks pipes */
int fork_pipes (int n, struct command *cmd) {
    int i;
    int in, fd [2];
    for (i = 0; i < n - 1; ++i) {
        pipe (fd);
        spawn_proc (in, fd [1], cmd + i);
        close (fd [1]);
        in = fd [0];
    }
    dup2 (in, 0);
    return execvp (cmd [i].argv [0], (char * const *)cmd [i].argv);
}

int main (int argc, char ** argv) {
    int i;
    if (argc == 1) { /* There were no arguments */
        const char *printenv[] = { "printenv", 0};
        const char *sort[] = { "sort", 0 };
        const char *less[] = { "less", 0 };
        struct command cmd [] = { {printenv}, {sort}, {less} };
        return fork_pipes (3, cmd);
    }
    if (argc > 1) { /* I'd like an argument */

        if (strncmp(argv[1], "cd", 2) && strncmp(argv[1], "exit", 2)) {
            char *tmp;
            int len = 1;
            for( i=1; i<argc; i++)
            {
                len += strlen(argv[i]) + 2;
            }
            tmp = (char*) malloc(len);
            tmp[0] = '\0';
            int pos = 0;
            for( i=1; i<argc; i++)
            {
                pos += sprintf(tmp+pos, "%s%s", (i==1?"":"|"), argv[i]);
            }
            const char *printenv[] = { "printenv", 0};
            const char *grep[] = { "grep", "-E", tmp, NULL};
            const char *sort[] = { "sort", 0 };
            const char *less[] = { "less", 0 };
            struct command cmd [] = { {printenv}, {grep}, {sort}, {less} };
            return fork_pipes (4, cmd);
            free(tmp);
        } else if (! strncmp(argv[1], "cd", 2)) { /* change directory */
            printf("change directory to %s\n" , argv[2]);
            chdir(argv[2]);
        } else if (! strncmp(argv[1], "exit", 2)) { /* change directory */
            printf("exit\n");
            exit(0);
        }
    }
    exit(0);
}
