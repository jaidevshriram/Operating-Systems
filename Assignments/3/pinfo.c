#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include "helper.h"

#define PROP_SIZE 1000

int pinfo(char **tokenized_input, int token_count)
{
    pid_t pid = getpid();

    if(token_count>=2)
        pid = atoi(tokenized_input[1]);

    if(pid == 0)
    {
        printf("\nInvalid PID");
        return -1;
    }

    char stat_file[100];
    sprintf(stat_file, "/proc/%d/stat", pid);

    FILE *fd = fopen(stat_file, "r");

    if(fd != NULL)
    {
        char p_prop[PROP_SIZE];
        sprintf(p_prop, "%d", pid);
        printf("\npid -- %s", p_prop);

        for(int i=0; fscanf(fd, "%s", p_prop) == 1 ; i++)
        {
            if(i == 2)
                printf("\nProcess Status -- %s", p_prop);
            if(i == 22)
                printf("\nMemory -- %s", p_prop);
        }

        sprintf(stat_file, "/proc/%d/exe", pid);
        
        char exe_path[100];
        int exe_path_end = readlink(stat_file, exe_path, 1000);
        if(exe_path_end != -1)
        {
            exe_path[exe_path_end] = '\0';
            printf("\nExecutable path -- %s", exe_path);
        }
        else
            printf("\n Error finding path for PID %d", pid);

        
        fclose(fd);    
    }
    else
    {
        printf("\nError looking up PID %d", pid);
        return -1;
    }

    return 0;
}