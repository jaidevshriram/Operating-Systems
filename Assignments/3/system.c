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
        // __pid_t exit_value = waitpid(pid_queue[i].pid, &status, WNOHANG);
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
        if(execvp(tokenized_input[0], tokenized_input) == -1)
        {
            perror("Process couldn't be started");
            // printf("%s\n", tokenized_input[0]);
            exit(10);
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
        do
        {
            wpid = waitpid(pid, &status, 0);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));

        if ( WIFEXITED(status) ) 
            if(WEXITSTATUS(status)!=0)
                error = -1; 
    }

    return error;
}

int fg(char **tokenized_input, int count)
{
    if(count<2)
    {
        printf("Shell: Too few arguments\nUsage: fg <pid>\n");
        return -1;
    }

    int pid = atoi(tokenized_input[1]);
    if(pid == 0)
    {
        printf("\n Usage: fg <pid>");
    }

    if(check_pid_exist(pid) == 1)
    {
        kill(pid, SIGCONT);
        delete_pid_queue(pid);
        int status;
		waitpid(-1,&status,WUNTRACED);
    }
    else
        printf("\npid does not exist");
}

int bg(char **tokenized_input, int count)
{
    if(count<2)
    {
        printf("Shell: Too few arguments\nUsage: fg <pid>\n");
        return -1;
    }

    int pid = atoi(tokenized_input[1]);
    if(pid == 0)
    {
        printf("\n Usage: fg <pid>");
    }

    if(check_pid_exist(pid) == 1)
    {
		kill(pid, SIGCONT);
        change_pid_status(pid, 1);  
    }
    else
        printf("\npid does not exist");
    
}