#define _XOPEN_SOURCE 500

#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <pwd.h>

#define BUFFER_LEN 1024

#ifdef SIGDET
#if SIGDET == 1
int isSignal = 1;		/*Termination detected by signals*/
#endif
#endif

#ifndef SIGDET
int isSignal = 0;
#endif

pid_t foreground = -1;

struct command
{
    char * const *argv;
};
/* Error handling function */
void err_syserr(char *fmt, ...)
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
/* Helper function that determines the beginning of a string */
int StartsWith(const char *a, const char *b)
{
    if(strncmp(a, b, strlen(b)) == 0) return 1;
    return 0;
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

        if(pipe(fd)==-1)	{
            err_syserr("Failed creating pipe");
        }

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

/*Remove zoombie processes*/
/*Return if background process terminated*/
/*
http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/signal.h.html
*/
void Janitor(int status)	{
    if(status==SIGCHLD)	{	/*Child process terminated, stopped, or continued*/
        int a = 1;
        while(a) {
            pid_t pid_my1 = waitpid(-1, &status, WNOHANG);
            /*WNOHANG = return immediately if no child has exited*/
            /*Wait*/
            /*http://linux.die.net/man/2/waitpid*/

            if(0<pid_my1) { /*Still things to clean up*/
                if(pid_my1!=foreground)	{ /*Don't stop me since it's the foregound process*/

                    /*http://linux.die.net/man/3/wait*/
                    if(WIFSIGNALED(status) || WIFEXITED(status))	{	/*Child process terminated*/
                        printf("\n\n\n\n%d terminated \n\n\n\n\n", pid_my1);
                    }
                }
            }
            else {	/*All work done, for now*/
                a = 0;
            }
        }
    }
}
/* helper function that removes spaces from a string */
void RemoveSpaces(char* source) {
    char* i = source;
    char* j = source;
    while(*j != 0) {
        *i = *j++;
        if(*i != ' ')
            i++;
    }
    *i = 0;
}

/* helper function that determines whether a file exists */
int file_exist (char *filename) {
    struct stat   buffer;
    return (stat (filename, &buffer) == 0);
}

/* Signal handler */
void sighandler(int signo, siginfo_t *si, void *vp) {
    int return_value;
    return_value = write(2, "Received SIGINT\n", 16);
    ++return_value; /* use the value to please the compiler */
}
int main(int argc, char *argv[]) {
    char line[BUFFER_LEN];
    char line2[BUFFER_LEN];
    char* argv2[100];
    int argc2 = 0;
    size_t length;
    char *token;
    char *token3;
    int i=0;
    char *tokenstr;
    char *search = " ";
    int isBackground = 0;
    int built_in_command = 0;
    int fd[2];
    char *printenv[] = { "printenv", 0};
    char *sort[] = { "sort", 0 };
    char *pager_cmd[] = { "less", 0 };
    char *grep[4];
    long time;
    int status = 0;
    int max = 80;
    int b;
    int pos = 0;
    char *tmp;
    char *new_str;
    int len = 1;
    int k;
    struct passwd *pw;
    const char *homedir;
    struct command cmd[3];
    struct command cmd2[4];
    struct timeval time_start;
    struct timeval time_end;
    sigset_t my_sig;
    pid_t pid_temp;
    char * pagerValue;
    int ret;
    char * pathValue;
    char * pathValue2;
    int breakloop=0;
    struct sigaction sa, osa;
    pid_temp = 0; /* To please the compiler */
    sa.sa_sigaction = sighandler;
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGINT, &sa, &osa);
    /* get the PATH environment to find if less is installed */
    pathValue = getenv("PATH");
    if (! pathValue) {
        printf ("'%s' is not set.\n", "PATH");
    }
    else {
        printf ("'%s' is set to %s.\n", "PATH", pathValue);
    }
    pathValue2 = strdup(pathValue);
    token3 = strtok(pathValue2, ":");
    ret = 1;
    while( token3 != NULL ) {
        if((new_str = malloc(strlen(token3)+strlen("/less")+1)) != NULL) {
            new_str[0] = '\0';
            strcat(new_str,token3);
            strcat(new_str,"/less");
            if (file_exist (new_str)) {
                /* Found less */
                ret=0;
                breakloop = 1;
            }
            free(new_str);
            if (breakloop) {
                break;
            }
        } else {
            printf("malloc failed!\n");
        }
        token3 = strtok(NULL, ":");
    }
    free(pathValue2);
    while(1) {
        i = 0;

        Janitor(SIGCHLD);

        printf("miniShell>> ");
        memset(line, 0, sizeof line); /*Reset*/
        if(!fgets(line, BUFFER_LEN, stdin)) {
            break;
        }
        Janitor(SIGCHLD);
        strncpy(line2, line, BUFFER_LEN);
        RemoveSpaces(line2);
        if (StartsWith(line2, "\n"))	{
            continue;
        }
        length = strlen(line);
        if (line[length - 1] == '\n') {
            line[length - 1] = '\0';
        }
        if(strcmp(line, "exit")==0) {
            break;
        }
        if(StartsWith(line, "cd")) {
            built_in_command=1;
            if(strstr(line, " ") == NULL) {
                pw = getpwuid(getuid());
                homedir = pw->pw_dir;

                if (chdir(homedir)==-1)	{	/*Change to home directory*/
                    perror("Failed changing to homedirectory\n");
                }
            } else {
                tokenstr = strtok(line, search);
                tokenstr = strtok(NULL, search);

                if (chdir(tokenstr)==-1)	{
                    perror("Failed changing directory\n");
                }

            }
        }
        token = strtok(line," ");
        while(token!=NULL) {
            argv2[i]=token;
            token = strtok(NULL," ");
            i++;
        }
        if(StartsWith(line, "checkEnv")) {
            built_in_command=1;


            pagerValue = getenv ("PAGER");
            if (! pagerValue) {
                if (ret == 0) {
                    pager_cmd[0]="less";
                } else {
                    pager_cmd[0]="more";
                }
            }
            else {
                pager_cmd[0]=pagerValue;
            }


            if(i==1) {
                cmd[0].argv= printenv;
                cmd[1].argv= sort;
                cmd[2].argv= pager_cmd;
                fork_pipes(3, cmd);


            }
            else {

                for (k = 1; k < i; k++)
                {
                    len += strlen(argv2[k]) + 2;
                }
                tmp = (char *) malloc(len);
                tmp[0] = '\0';
                for (k = 1; k < i; k++)
                {
                    pos += sprintf(tmp + pos, "%s%s", (k == 1 ? "" : "|"), argv2[k]);
                }
                grep[0]="grep";
                grep[1]="-E";
                grep[2]= tmp;
                grep[3]= NULL;
                cmd2[0].argv= printenv;
                cmd2[1].argv= grep;
                cmd2[2].argv= sort;
                cmd2[3].argv= pager_cmd;
                fork_pipes(4, cmd2);
                free(tmp);
            }
        }
        if(0==built_in_command)	{	/*Not a built in command, so let execute it*/

            argv2[i]=NULL;
            argc=i;
            for(i=0; i<argc2; i++) {
                printf("%s\n", argv2[i]);
            }

            isBackground = 0;


            for (b = 0; b<max; b++)	{

                if ('&'==line[b])	{
                    isBackground = 1;

                }
            }
            if (isBackground == 1)	{	/*If backgroundprocess*/

                if (pipe(fd)==-1)	{  /*(two new file descriptors)*/
                    perror("Failed crating pipe\n");
                }

                pid_temp = fork();
            }
            else if (isBackground == 0)	{	/*If foreground process*/
                gettimeofday(&time_start, NULL);

                if (1 == isSignal)	{	/*If using signaldetection*/
                    sigemptyset(&my_sig); /*empty and initialising a signal set*/
                    sigaddset(&my_sig, SIGCHLD);	/*Adds signal to a signal set (my_sig)*/
                    /*http://pubs.opengroup.org/onlinepubs/7908799/xsh/sigprocmask.html*/
                    sigprocmask(SIG_BLOCK, &my_sig, NULL);
                }

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
                if (1 == isBackground)	{	/*Backgroundprocess*/
                    dup2(fd[STDIN_FILENO], STDIN_FILENO);
                    close(fd[0]);
                    close(fd[1]);
                }
                /*http://www.lehman.cuny.edu/cgi-bin/man-cgi?execvp+2*/
                if (execvp(argv2[0],argv2) < 0)	{
                    printf("We are sorry to inform you that something went wrong %d \n", errno);
                }
            }
            if (0 == isBackground) {	/*Foregroundprocess*/
                waitpid(foreground, &status, 0);	/*Waiting*/
                /*Foregroundprocess terminated*/

                gettimeofday(&time_end, NULL);
                time = (time_end.tv_sec-time_start.tv_sec)*1000000 + time_end.tv_usec-time_start.tv_usec;
                printf("Execution time %ld ms\n", time);	/*Print out the execution time*/

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
            else if (1==isBackground)	{
                close(fd[0]);
                close(fd[1]);
            }
        }

        built_in_command = 0;	/*Reset*/
        memset(line, 0, sizeof line); /*Reset*/
    }
    return (0);
}
