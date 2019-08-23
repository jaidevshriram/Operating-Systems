#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include "helper.h"

int launch_command_bg(char **tokenized_input, int bg_pos)
{
    __pid_t pid, wpid;
    int status;

    tokenized_input[bg_pos] = NULL;

    pid = fork();
    if(pid == 0)
    {
        if(execvp(tokenized_input[0], tokenized_input) == -1)
        {
            perror("Shell");
            return -1;
        }
    }
    else if (pid < 0)
    {
        perror("Child process could not be created");
        return 0;
    }
    else
        add_pid_queue(pid);

    return 0;
}

int launch_command(char **tokenized_input)
{
    int is_background = 0, bg_char = -1;

    for(int i=0; tokenized_input[i]!=NULL; i++)
        if(strlen(tokenized_input[i])==1)
            if(strcmp(tokenized_input[i], "&")==0)
            {
                bg_char = i;
                is_background = 1;
            }

    if(is_background)
    {
        if(launch_command_bg(tokenized_input, bg_char)==-1)
            return -1;
        return 0;
    }

    __pid_t pid, wpid;
    int status;

    pid = fork();
    if(pid == 0)
    {
        if(execvp(tokenized_input[0], tokenized_input) == -1)
        {
            perror("Shell");
            return -1;
        }
    }
    else if (pid < 0)
    {
        perror("Child process could not be created");
        return 0;
    }
    else 
    {
        do
        {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 0;
}