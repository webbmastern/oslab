#include<sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
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


#define BUFFER_LEN 1024

int main() {
    char line[BUFFER_LEN];  //get command line
    char* argv[100];        //user command
    char* path= "/bin/";    //set path at bin
    char progpath[20];      //full file path
    int argc;               //arg count

    while(1) {

        printf("miniShell>> ");                    //print shell prompt

        if(!fgets(line, BUFFER_LEN, stdin)) { //get command and put it in line
            break;                                //if user hits CTRL+D break
        }
        size_t length = strlen(line);
        if (line[length - 1] == '\n') {
            line[length - 1] = '\0';
        }
        if(strcmp(line, "exit")==0) {           //check if command is exit
            break;
        }

        char *token;                  //split command into separate strings
        token = strtok(line," ");
        int i=0;
        while(token!=NULL) {
            argv[i]=token;
            token = strtok(NULL," ");
            i++;
        }
        argv[i]=NULL;                     //set last value to NULL for execvp

        argc=i;                           //get arg count
        for(i=0; i<argc; i++) {
            printf("%s\n", argv[i]);      //print command/args
        }
        strcpy(progpath, path);           //copy /bin/ to file path
        strcat(progpath, argv[0]);            //add program to path

        for(i=0; i<strlen(progpath); i++) {   //delete newline
            if(progpath[i]=='\n') {
                progpath[i]='\0';
            }
        }
        int pid= fork();              //fork child

        if(pid==0) {              //Child
            execvp(progpath,argv);
            fprintf(stderr, "Child process could not do execvp\n");

        } else {                   //Parent
            wait(NULL);
            printf("Child exited\n");
        }

    }
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
