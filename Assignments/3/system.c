#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<signal.h>
#include "helper.h"
#include "catchsig.h"
#include "jobs.h"

extern int shell_pid;

int launch_command_bg(char **tokenized_input, int bg_pos)
{
    __pid_t pid, wpid;
    int status;

    tokenized_input[bg_pos] = NULL;

    pid = fork();
    if(pid == 0)
    {
        setpgid(0,0);
        initialize_signal_handlers();
        if(execvp(tokenized_input[0], tokenized_input) == -1)
        {
            perror("Shell");
            return -1;
        }
    }
    else if (pid < 0)
    {
        perror("Child process could not be created");
        return -1;
    }
    else
    {
        printf("\n[+] %d", pid);
        add_pid_queue(pid, tokenized_input[0], 1);
    }

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
        {
            printf("\nBackground proccess couldn't be initiated");
            return -1;
        }
        return 0;
    }

    __pid_t pid, wpid;
    int status, error = 0;

    pid = fork();
    if(pid == 0)
    {
        initialize_signal_handlers();
        setpgid(0,0);
        if(execvp(tokenized_input[0], tokenized_input) == -1)
        {
            perror("Process couldn't be started");
            // printf("%s\n", tokenized_input[0]);
            exit(-1);
        }
    }
    else if (pid < 0)
    {
        perror("Child process could not be created");
        error = -1;
    }
    else 
    {
        set_child_pid(pid);

        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        
        tcsetpgrp(STDIN_FILENO, pid);

        waitpid(pid, &status, WUNTRACED);

        if(WIFEXITED(status)==0)
            error = -1;

        if(WIFSTOPPED(status)!=0)
        {
            add_pid_queue(pid, tokenized_input[0], 0);
            printf("[%d] Stopped\n", pid);
            error = -1;
        }
         
        tcsetpgrp(STDIN_FILENO, getpgrp());

        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);
    }

    return error;
}