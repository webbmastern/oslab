#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv, char **envp)
{
    int i;
    FILE *fp = fopen("digenv.dat", "ab+");
    for(i=0; envp[i]!= NULL; i++) {
        printf("%2d:%s\n",i, envp[i]);
        fprintf(fp, "%s\n", envp[i]);
    }
    fclose(fp);
}
