#include "types.h"
#include "stat.h"
#include "user.h"
#include "proc_stat.h"

volatile int information[10000000];
int main(int argc, char *argv[])
{
    for(int i=1; i<=15; i++)
    {
        int pid = fork();

        if(pid == 0)
        {
            int count = 10000000, index = 0;
            float test = 0;
            
            for(volatile int j = 0; j < count * i; j++)
            {
                test += 5*0.01;
                
                if(j%5000000 == 0)
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
                    printf(1, "%d]\n", information[index]);
            }

        }
    }

    for(int i=1; i<=15; i++)
    {
        wait();
    }
}