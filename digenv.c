#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

pid_t foreground = -1;

struct command
{
    char * const *argv;
};

static _Noreturn void err_syserr(char *fmt, ...)
{
    int errnum = errno;
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    if (errnum != 0)
        fprintf(stderr, "(%d: %s)\n", errnum, strerror(errnum));
    exit(EXIT_FAILURE);
}

/* Helper function that spawns processes */
static int spawn_proc(int in, int out, struct command *cmd)
{
    pid_t pid;
    if ((pid = fork()) == 0)
    {
        if (in != 0)
        {
            if (dup2(in, 0) < 0)
                err_syserr("dup2() failed on stdin for %s: ", cmd->argv[0]);
            ;
            close(in);
        }
        if (out != 1)
        {
            if (dup2(out, 1) < 0)
                err_syserr("dup2() failed on stdout for %s: ", cmd->argv[0]);
            close(out);
        }
        fprintf(stderr, "%d: executing %s\n", (int)getpid(), cmd->argv[0]);
        execvp(cmd->argv[0], cmd->argv);
        err_syserr("failed to execute %s: ", cmd->argv[0]);
    }
    else if (pid < 0)
        err_syserr("fork failed: ");
    return pid;
}

/* Helper function that forks pipes */
static void fork_pipes(int n, struct command *cmd)
{
    int i;
    int in = 0;
    int fd[2];
    for (i = 0; i < n - 1; ++i)
    {
        pipe(fd);
        spawn_proc(in, fd[1], cmd + i);
        close(fd[1]);
        in = fd[0];
    }
    if (dup2(in, 0) < 0)
        err_syserr("dup2() failed on stdin for %s: ", cmd[i].argv[0]);
    fprintf(stderr, "%d: executing %s\n", (int)getpid(), cmd[i].argv[0]);
    execvp(cmd[i].argv[0], cmd[i].argv);
    err_syserr("failed to execute %s: ", cmd[i].argv[0]);
}

int main(int argc, char **argv)
{
    int i;
    if (argc == 1)   /* There were no arguments */
    {
        char *printenv[] = { "printenv", 0};
        char *sort[] = { "sort", 0 };
        char *less[] = { "less", 0 };
        struct command cmd[] = { {printenv}, {sort}, {less} };
        fork_pipes(3, cmd);
    }
    else
    {
        if (strcmp(argv[1], "cd") == 0)      /* change directory */
        {
            printf("change directory to %s\n", argv[2]);
            chdir(argv[2]);
        }
        else if (strcmp(argv[1], "exit") == 0)
        {
            printf("exit\n");
            exit(0);
        }
        else
        {
            char *tmp;
            int len = 1;
            for (i = 1; i < argc; i++)
            {
                len += strlen(argv[i]) + 2;
            }
            tmp = (char *) malloc(len);
            tmp[0] = '\0';
            int pos = 0;
            for (i = 1; i < argc; i++)
            {
                pos += sprintf(tmp + pos, "%s%s", (i == 1 ? "" : "|"), argv[i]);
            }
            char *printenv[] = { "printenv", 0};
            char *grep[] = { "grep", "-E", tmp, NULL};
            char *sort[] = { "sort", 0 };
            char *less[] = { "less", 0 };
            struct command cmd[] = { {printenv}, {grep}, {sort}, {less} };
            fork_pipes(4, cmd);
            free(tmp);
        }
    }
    return(0);
}

/*Remove zoombie processes*/
/*Return if background process terminated*/
/*
http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/signal.h.html
*/
void Janitor(int signal)	{

	if(signal==SIGCHLD)	{	/*Child process terminated, stopped, or continued*/

		int a = 1;

		while(a)	{

			pid_t pid_my1 = waitpid(-1, &signal, WNOHANG);
			/*WNOHANG = return immediately if no child has exited*/
			/*http://linux.die.net/man/2/waitpid*/

			if(0<pid_my1)	{	/*Still things to clean up*/
			

				if(pid_my1!=foreground)	{ /*Don't stop me since it's the foregound process*/

					/*http://linux.die.net/man/3/wait*/
					if(WIFEEXITED(signal))	{	/*Child process terminated*/
						/*FIXME*/
					}
				}
			}
			else {	/*All work done, for now*/
				a = 0;
			}

		}

	}

}

