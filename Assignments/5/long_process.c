#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
    int pid = fork();
    if(pid == 0)
    {
        int count = 10000000;
        float test = 0;

        if(atoi(argv[1])==1)
            count = 500000000;
        
        for(volatile int i = 0; i<count; i++){
            test += 5*0.01;
        }
    }
    else if(pid < 0)
    {
        printf(1, "Forking Failed\n");
    }
    else
    {
        wait();
        printf(1, "PROCESS DONE\n");
    }

    exit();
}