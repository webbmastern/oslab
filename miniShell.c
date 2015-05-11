#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFER_LEN 1024
#define BUFFERSIZE 1024


int mystrcmp(char const *, char const *);


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

int StartsWith(const char *a, const char *b)
{
   if(strncmp(a, b, strlen(b)) == 0) return 1;
   return 0;
}

int main() {
    char line[BUFFER_LEN];  
    char* argv[100];        
    char* path= "/bin/";    
    char progpath[20];      
    int argc;               
    size_t length;
    char *token;
    int i=0;
    int pid;
    char *tokenstr;
    char *search = " ";
    while(1) {
	i = 0;
        printf("miniShell>> ");                    

        if(!fgets(line, BUFFER_LEN, stdin)) { 
            break;                                
        }
        length = strlen(line);
        if (line[length - 1] == '\n') {
            line[length - 1] = '\0';
        }
        if(strcmp(line, "exit")==0) {           
            break;
        }
	if(StartsWith(line, "cd")) {           
            printf("change directory\n");
	    tokenstr = strtok(line, search);
	    tokenstr = strtok(NULL, search);
            chdir(tokenstr);
        }
                          
        token = strtok(line," ");
        
        while(token!=NULL) {
            argv[i]=token;
            token = strtok(NULL," ");
            i++;
        }
        argv[i]=NULL;                     

        argc=i;                           
        for(i=0; i<argc; i++) {
            printf("%s\n", argv[i]);      
        }
        strcpy(progpath, path);           
        strcat(progpath, argv[0]);            

        for(i=0; i<strlen(progpath); i++) {   
            if(progpath[i]=='\n') {
                progpath[i]='\0';
            }
        }
        pid= fork();              

        if(pid==0) {              
            execvp(progpath,argv);
            fprintf(stderr, "Child process could not do execvp\n");

        } else {                  
            wait(NULL);
            printf("Child exited\n");
        }

    }
return (0);
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
    char cwd[BUFFERSIZE];
    char * return_value;
    int other_return;
    strcpy(path,pth);
 
    if(pth[0] != '/')
    {  
        return_value = getcwd(cwd,sizeof(cwd));
        strcat(cwd,"/");
        strcat(cwd,path);
        other_return = chdir(cwd);
    } else { 
        other_return = chdir(pth);
    }
    printf("Spawned foreground process: %d\n", getpid());
    return 0;
}
