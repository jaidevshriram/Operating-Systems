
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "fs.h"
#include "proc_stat.h"

long long int information[10000];
int proc_count = 5;

int main(int argc, char *argv[])
{
    if(argc > 1)
        proc_count = atoi(argv[1]);

    for(volatile int i=1; i<=proc_count; i++)
    {
        int pid = fork();

        if(pid == 0)
        {
            int count = 10000000;
            int index = 0;
            float test = 0;
            
            for(int j = 0; j < count * i; j++)
            {
                test += 5*0.01;
                
                if(j%2500000 == 0)
                {
                    struct proc_stat p;
                    getpinfo(&p, getpid());
                    information[index++] = p.current_queue;
                }   
            }

            printf(1, "%d: [", getpid());
            for(int k=0; k<index; k++)
            {
                if(k!=(index-1))
                    printf(1, "%d, ", information[k]);
                else
                    printf(1, "%d]\n", information[k]);
            }

            exit();
        }
    }

    for(volatile int i=1; i<=proc_count; i++)
    {
        wait();
    }
}