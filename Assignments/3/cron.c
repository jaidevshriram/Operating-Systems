#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "system.h"
#include<time.h>

int cronjob(char **tokenized_input, int count)
{
    if(count!=7)
    {
        printf("Incorrect Number of arguments. Usage: cronjob -c <command> -t <period> -p <duration>\n");
        return -1;
    }

    int exit = 0, t=-1, p=-1;
    char name[200]="err";

    if(tokenized_input[1][0]=='-' && exit==0)
    {
        if(strlen(tokenized_input[1]) <=1)
            exit = 1;

        for(int i=1; i<strlen(tokenized_input[1]); i++)
        {
            if(tokenized_input[1][i]!='c' && tokenized_input[1][i]!='t' && tokenized_input[1][i]!='p')
                exit = 1;
            else if (tokenized_input[1][i]=='c')
                strcpy(name, tokenized_input[2]);
            else if (tokenized_input[1][i]=='p')
                p = atoi(tokenized_input[2]);
            else if (tokenized_input[1][i]=='t')
                t = atoi(tokenized_input[2]);
        }
    }

    if(tokenized_input[3][0]=='-' && exit==0)
    {
        if(strlen(tokenized_input[3]) <=1)
            exit = 1;

        for(int i=1; i<strlen(tokenized_input[3]); i++)
        {
            if(tokenized_input[3][i]!='c' && tokenized_input[3][i]!='t' && tokenized_input[3][i]!='p')
                exit = 1;
            else if (tokenized_input[3][i]=='c')
                strcpy(name, tokenized_input[4]);
            else if (tokenized_input[3][i]=='p')
                p = atoi(tokenized_input[4]);
            else if (tokenized_input[3][i]=='t')
                t = atoi(tokenized_input[4]);
        }
    }

    if(tokenized_input[5][0]=='-' && exit==0)
    {
        if(strlen(tokenized_input[5]) <=1)
            exit = 1;

        for(int i=1; i<strlen(tokenized_input[5]); i++)
        {
            if(tokenized_input[5][i]!='c' && tokenized_input[5][i]!='t' && tokenized_input[5][i]!='p')
                exit = 1;
            else if (tokenized_input[5][i]=='c')
                strcpy(name, tokenized_input[6]);
            else if (tokenized_input[5][i]=='p')
                p = atoi(tokenized_input[6]);
            else if (tokenized_input[5][i]=='t')
                t = atoi(tokenized_input[6]);
        }
    }

    for(int i=0; name[i]; i++)
        if(name[i]=='&')
            exit = 1;

    if(exit==1 || strcmp(name, "err")==0 || p==-1 || t==-1)
    {
        printf("Error in command format\n");
        return -1;
    }

    __pid_t pid, wpid;
    int status;

    pid = fork();
    if(pid == 0)
    {
        setpgid(0,0);
        initialize_signal_handlers();
        int run_count = p/t;

        for(int i=0; i<run_count; i++)
        {
            sleep(t);
            int err = start_command_execution(name);
            if(err == -1)
            {
                printf("Command has an error\n Cronjob killed\n");
                return -1;
            }
        }

        return 0;
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