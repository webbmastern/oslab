#include<sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include<dirent.h>
#include<error.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
int mystrcmp(char const *, char const *);

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
    else if (pid < 0)	{
         err_syserr("fork failed: "); 
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
           err_syserr("dup2() failed on stdin for %s: ", cmd[i].argv[0]);
    }
    fprintf(stderr, "%d: executing %s\n", (int)getpid(), cmd[i].argv[0]);
    execvp(cmd[i].argv[0], cmd[i].argv);
     err_syserr("failed to execute %s: ", cmd[i].argv[0]);
}

#define BUFFERSIZE 200
int main() {

    char *args[80];
    char buffer[BUFFERSIZE];
    char *prompt = "OS";
    char *a = ">";

    char *tok;
    tok = strtok (buffer," ");


    while(buffer != NULL) {
        bzero(buffer, BUFFERSIZE);
        printf("%s%s",prompt,a);
        fgets(buffer, BUFFERSIZE, stdin);
        if(mystrcmp(buffer,"cd") == 0) {
            tok = strchr(buffer,' ')+1; //use something more powerful
            *strchr(tok, '\n')='\0';
            cd(tok);
        } 
	else if(mystrcmp(buffer,"exit") == 0) {
	   	return 0;
	}	
	else {
            system("ls"); //for testing the CWD/PWD
	    printf("Spawned foreground process: %d\n", getpid());
        }
    }
    return 0;
}

int mystrcmp(char const *p, char const *q)
{
    int i = 0;
    for(i = 0; q[i]; i++)
    {
        if(p[i] != q[i])
            return -1;
    }
    return 0;
}

int cd(char *pth) {
    char path[BUFFERSIZE];
    strcpy(path,pth);

    char *token;

    char cwd[BUFFERSIZE];
    if(pth[0] != '/')
    {   // true for the dir in cwd
        getcwd(cwd,sizeof(cwd));
        strcat(cwd,"/");
        strcat(cwd,path);
        chdir(cwd);
    } else { //true for dir w.r.t. /
        chdir(pth);
    }
    printf("Spawned foreground process: %d\n", getpid());
    return 0;
}
