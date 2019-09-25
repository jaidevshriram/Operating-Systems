#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/wait.h>

struct job
{
    int pid;
    int stat;
    char name[100];
};

struct job pid_queue[1000];
int pid_queue_count;

void start_pid_queue()
{
    pid_queue_count = 0;
}

void add_pid_queue(int pid, char name[], int stat)
{
    pid_queue[pid_queue_count].pid = pid;
    strcpy(pid_queue[pid_queue_count++].name, name);
    pid_queue[pid_queue_count].stat = stat;
}

void delete_pid_queue(int pid)
{
    int j=0; struct job new_pid_queue[1000];
    for(int i=0; i<pid_queue_count; i++)
        if(pid_queue[i].pid!=pid)
            new_pid_queue[j++] = pid_queue[i];
        else
            pid_queue_count --;

    for(int i=0; i<j; i++)
        pid_queue[i] = new_pid_queue[i];
}

void check_pid_status()
{
    for(int i=0; i<pid_queue_count; i++)
    {
        __pid_t status;
        __pid_t exit_value = waitpid(pid_queue[i].pid, &status, WNOHANG);
        if(exit_value == pid_queue[i].pid)
        {
            printf("[%d] exited normally\n", pid_queue[i].pid);
            delete_pid_queue(pid_queue[i].pid);
        }
        else if(exit_value!=0)
        {
            printf("[%d] exited with errors.\n", pid_queue[i].pid);
            delete_pid_queue(pid_queue[i].pid);
        }
    }
}

void change_pid_status(int pid, int newstat)
{
    for(int i=0; i<pid_queue_count; i++)
        if(pid_queue[i].pid!=pid)
            pid_queue[i].stat = newstat;
}

int check_pid_exist(int pid)
{
    for(int i=0; i<pid_queue_count; i++)
    {
        if(pid_queue[i].pid == pid)
            return 1;
    }

    return -1;
}

int jobs()
{
    for(int i=0; i<pid_queue_count; i++)
    {
        printf("[%d] %s %s [%d]\n", i, pid_queue[i].stat == 0 ? "Stopped" : "Running", pid_queue[i].name, pid_queue[i].pid);
    }

    return 0;
}