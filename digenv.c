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

/*static _Noreturn void err_syserr(char *fmt, ...)
{
    int errnum = errno;
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    if (errnum != 0)
        fprintf(stderr, "(%d: %s)\n", errnum, strerror(errnum));
    exit(EXIT_FAILURE);
}*/

/* Helper function that spawns processes */
static int spawn_proc(int in, int out, struct command *cmd)
{
    pid_t pid;
    if ((pid = fork()) == 0)
    {
        if (in != 0)
        {
            if (dup2(in, 0) < 0)
                /*err_syserr("dup2() failed on stdin for %s: ", cmd->argv[0]);*/
                ;
            close(in);
        }
        if (out != 1)
        {
            if (dup2(out, 1) < 0)
                /*err_syserr("dup2() failed on stdout for %s: ", cmd->argv[0]);*/
                close(out);
        }
        fprintf(stderr, "%d: executing %s\n", (int)getpid(), cmd->argv[0]);
        execvp(cmd->argv[0], cmd->argv);
        /*  err_syserr("failed to execute %s: ", cmd->argv[0]);*/
    }
    else if (pid < 0)	{
        /* err_syserr("fork failed: "); */
    }
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
    if (dup2(in, 0) < 0)	{
        /*   err_syserr("dup2() failed on stdin for %s: ", cmd[i].argv[0]);*/
    }
    fprintf(stderr, "%d: executing %s\n", (int)getpid(), cmd[i].argv[0]);
    execvp(cmd[i].argv[0], cmd[i].argv);
    /* err_syserr("failed to execute %s: ", cmd[i].argv[0]);*/
}


/*Remove zoombie processes*/
/*Return if background process terminated*/
/*
http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/signal.h.html
*/
void Janitor(int status)	{

    if(status==SIGCHLD)	{	/*Child process terminated, stopped, or continued*/

        int a = 1;

        while(a)	{

            pid_t pid_my1 = waitpid(-1, &status, WNOHANG);
            /*WNOHANG = return immediately if no child has exited*/
            /*Wait*/
            /*http://linux.die.net/man/2/waitpid*/

            if(0<pid_my1)	{	/*Still things to clean up*/


                if(pid_my1!=foreground)	{ /*Don't stop me since it's the foregound process*/

                    /*http://linux.die.net/man/3/wait*/
                    if(WIFEXITED(status))	{	/*Child process terminated*/
                        printf("%d terminated", pid_my1);
                    }
                }
            }
            else {	/*All work done, for now*/
                a = 0;
            }
        }
    }
}


int main(int argc, char **argv)
{

    /*TODO Define isSignal, should be set by a macro at compilation time*/

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


    /*TODO we should need to do a loop since we should be able to run the program
    multiple times.
    checkenv should also be incorporated as a built in command in the new labversion
    */


    int isBackground = 0;

    sigset_t my_sig;
    sigemptyset(&my_sig); /*empty and initialising a signal set*/
    sigaddset(&my_sig, SIGCHLD);	/*Adds signal to a signal set (my_sig)*/

    pid_t pid_temp;

    /*TODO
    Determine if a command should be executed as a background process
    The input should be checked if ending with a &-sign and the variable isBackground should be set to 1*/


    /*TODO insert the input array here*/
    /*char input;

    int max = 80;
    int b;
    for (b = 0; b<max; b++)	{
    	if (NULL = input[b])	{
    		b = max;
    	}
    	if (0 == strcmp("&", input[b])	{
    		isBackground = 1;*/
    /*input[i] = NULL;*/ /*Maybe it should be removed FIXME*/
    /*}
    }*/




    /*TODO store the time forground process started*/

    int fd[2];
    if (isBackground == 1)	{	//If backgroundprocess

        pipe(fd);  /*(two new file descriptors)*/

        /*FIXME pid_temp = fork_pipes(2, .....);*/
        pid_temp = fork();
    }

    else if (isBackground == 0)	{	//If foreground process

        int isSignal = 0;	/*FIXME*/
        if (1 == isSignal)	{	/*If using signaldetection*/

            /*http://pubs.opengroup.org/onlinepubs/7908799/xsh/sigprocmask.html*/
            sigprocmask(SIG_BLOCK, &my_sig, NULL);
        }

        /*FIXME pid_temp = fork_pipes(2, .....);*/

        pid_temp = fork();
        foreground = pid_temp;	/*Set pid for foreground process*/

    }


    if (0<pid_temp)	{
        /*Parent process*/
    }

    else if (0>pid_temp)	{
        /*Error*/
    }

    else	{
        /*Child process*/
    }


    if (1 == isBackground)	{	//Backgroundprocess


        close(fd[0]);
        close(fd[1]);

    }

    else if (0 == isBackground)	{	//Foregroundprocess



        /*Write here, Emil*/
        int status = 0;
        waitpid(pid_temp, &status, 0);

        /*Foregroundprocess terminated*/
        /*TODO How long time was the total execution time*/



        int isSignal = 0;	/*FIXME*/
        if (1 == isSignal)	{	/*If using signaldetection*/

            int a = sigprocmask(SIG_UNBLOCK, &my_sig, NULL);
            /*http://man7.org/linux/man-pages/man2/sigprocmask.2.html*/

            if (0 == a)	{
                /*Sigprocmask was successfull*/
            }
            else	{
                /*Sigprocmask was not successfull, return=-1*/
            }
            Janitor(SIGCHLD);
        }

    }




    return(0);
}




