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

pid_t foreground = -1;

int mystrcmp(char const *, char const *);

struct command
{
    char * const *argv;
};

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

int EndsWith(const char *a, const char *b)
{

	


} 


/* Helper function that spawns processes */
/*static int spawn_proc(int in, int out, struct command *cmd)
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
}*/

/* Helper function that forks pipes */
/*static void fork_pipes(int n, struct command *cmd)
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
}*/


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

	int isSignal = 0;
	int isBackground = 0;

	#ifdef SIGDET
		#if SIGDET == 1
			isSignal = 1;		/*Termination detected by signals*/
		#endif
	#endif

	
    while(1) {
	i = 0;
	int built_in_command = 0;

	struct timeval time_start;
    struct timeval time_end;


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

			built_in_command=1;
           
			printf("change directory\n");
		    tokenstr = strtok(line, search);
		    tokenstr = strtok(NULL, search);
            chdir(tokenstr);
		/*TODO maybe have a check whether extra argument exist, if not go to home directory*/

        }
                          
        token = strtok(line," ");
        
        while(token!=NULL) {
            argv[i]=token;
            token = strtok(NULL," ");
            i++;
        }

		if(StartsWith(line, "checkEnv")) {
			
			built_in_command=1;
			if (0==i)	{

				char *printenv[] = { "printenv", 0};
        		char *sort[] = { "sort", 0 };
        		char *less[] = { "less", 0 };
        		struct command cmd[] = { {printenv}, {sort}, {less} };

       			fork();
			}
			else	{

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
		        fork();
		        free(tmp);
			}
		}


 
		if(0==built_in_command)	{	/*Not a built in command, so let execute it*/
	
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

		

			isBackground = 0;

			sigset_t my_sig;
			
			pid_t pid_temp;
	
			
			if (0==strcmp(argv[i-1], "&"))	{

				isBackground = 1;
			}



			int fd[2];
			if (isBackground == 1)	{	//If backgroundprocess

				pipe(fd);  /*(two new file descriptors)*/

				/*FIXME pid_temp = fork_pipes(2, .....);*/
				pid_temp = fork();
			}

			else if (isBackground == 0)	{	//If foreground process

				gettimeofday(&time_start, NULL);

				int isSignal = 0;	/*FIXME*/
				if (1 == isSignal)	{	/*If using signaldetection*/

					sigemptyset(&my_sig); /*empty and initialising a signal set*/
					sigaddset(&my_sig, SIGCHLD);	/*Adds signal to a signal set (my_sig)*/
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
			
				if (1 == isBackground)	{	//Backgroundprocess

					dup2(fd[STDIN_FILENO], STDIN_FILENO);

					close(fd[0]);
					close(fd[1]);

				}
				execvp(argv[0],argv);		
			}

			if (0 == isBackground)	{	//Foregroundprocess



				/*Write here, Emil*/
				int status = 0;
				waitpid(pid_temp, &status, 0);

				/*Foregroundprocess terminated*/

				gettimeofday(&time_end, NULL);

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

				/*TODO Print how long time was the total execution time*/

			}

			else if (1==isBackground)	{

				close(fd[0]);
				close(fd[1]);
			}
		}




       /* pid= fork();              

        if(pid==0) {              
            execvp(progpath,argv);
            fprintf(stderr, "Child process could not do execvp\n");

        } else {                  
            wait(NULL);
            printf("Child exited\n");
        }*/

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
