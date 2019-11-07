#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
    int pid = fork();
    if(pid == 0)
    {
        unsigned int count = 100000000;
        float test = 0;
        
        for(volatile unsigned int i = 0; i<count; i++){
            for(volatile  unsigned int j = 0; j<5; j++){
                test += 5*0.01*2.00345 - 1 + 0.25;
            }
        }
        exit();
    }
    else if(pid < 0)
    {
        printf(1, "Forking Failed\n");
    }
    else
    {
        int status = -1;
        // status = wait();
        printf(1, "PROCESS DONE: %d with %d\n", pid, status);
    }

    exit();
}