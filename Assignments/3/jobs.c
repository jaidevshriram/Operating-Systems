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
    strcpy(pid_queue[pid_queue_count].name, name);
    pid_queue[pid_queue_count++].stat = stat;
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
            delete_pid_queue(pid_queue[i--].pid);
        }
        else if(exit_value!=0)
        {
            printf("[%d] exited with errors.\n", pid_queue[i].pid);
            delete_pid_queue(pid_queue[i--].pid);
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
        int pid = pid_queue[i].pid;

        char stat_file[100];
        sprintf(stat_file, "/proc/%d/stat", pid);

        FILE *fd = fopen(stat_file, "r");

        if(fd != NULL)
        {
            char p_prop[1000];
            sprintf(p_prop, "%d", pid);

            for(int j=0; fscanf(fd, "%s", p_prop) == 1 ; j++)
            {
                if(j == 2)
                {
                    char pid_stat[1000];

                    if(strcmp(p_prop, "t")==0 || strcmp(p_prop, "T")==0 || strcmp(p_prop, "Z")==0)
                        strcpy(pid_stat, "Stopped");
                    else if(strcmp(p_prop, "R")==0 || strcmp(p_prop, "S")==0)
                        strcpy(pid_stat, "Running");
                    else
                    {
                        printf("%d: %s with %s is being deleted\n", pid, p_prop, pid_queue[i].name);
                        delete_pid_queue(pid);
                        i--;
                        continue;
                    }

                    printf("[%d] %s %s [%d]\n", i+1, pid_stat, pid_queue[i].name, pid_queue[i].pid);
                    break;
                }
            }  
        }
        else
        {
            delete_pid_queue(pid);
            i--;
        }

        fclose(fd);
    }

    return 0;
}

int overkill()
{
    for(int i=0; i<pid_queue_count; i++)
        kill(pid_queue[i].pid, SIGKILL);
    start_pid_queue();
    return 0;
}

int kjobs(char **tokenized_input, int count)
{
    if(count!=3)
    {
        printf("Usage: kjobs <jobno> <signo>\n");
        return -1;
    }

    int pid_no = atoi(tokenized_input[1]);
    int signo = atoi(tokenized_input[2]);

    if(pid_no < 0 || signo < 0)
    {
        printf("Inavlid Usage of kjobs\n");
        return -1;
    }

    if(pid_no<=0 || pid_no>pid_queue_count)
    {
        printf("Job doesn't exist\n");
        return -1;
    }

    int pid = pid_queue[pid_no-1].pid;

    kill(pid, signo);
}

int fg(char **tokenized_input, int count)
{
    int error = 0;

    if(count<2)
    {
        printf("Shell: Too few arguments\nUsage: fg <jobno>\n");
        return -1;
    }

    int pid_no = atoi(tokenized_input[1]);

    if(pid_no<=0 || pid_no>pid_queue_count)
    {
        printf("Job doesn't exist\n");
        return -1;
    }

    int pid = pid_queue[pid_no-1].pid;

    if(pid == 0)
    {
        printf("\n Usage: fg <jobno>");
        return -1;
    }

    if(check_pid_exist(pid) == 1)
    {
        delete_pid_queue(pid);
        int status;

        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        
        tcsetpgrp(STDIN_FILENO, pid);
        kill(pid, SIGCONT);

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
    else
    {
        printf("\npid does not exist");
        return -1;
    }

    return error;
}

int bg(char **tokenized_input, int count)
{
    if(count<2)
    {
        printf("Shell: Too few arguments\nUsage: bg <jobno>\n");
        return -1;
    }
    
    int pid_no = atoi(tokenized_input[1]);

    if(pid_no<=0 || pid_no>pid_queue_count)
    {
        printf("Job doesn't exist\n");
        return -1;
    }

    int pid = pid_queue[pid_no-1].pid;

    if(pid == 0)
    {
        printf("\n Usage: bg <jobno>");
        return -1;
    }

    if(check_pid_exist(pid) == 1)
    {
		kill(pid, SIGCONT);
        change_pid_status(pid, 1);  
    }
    else
    {
        printf("\npid does not exist");
        return -1;
    }

    return 0;    
}