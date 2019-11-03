#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
    int pid = fork();
    if(pid == 0)
    {
        float test = 0;
        for(volatile int i = 0; i<300000000; i++){
            test += 5*0.01;
        }
    }
    else if(pid < 0)
    {
        printf(1, "Forking Failed\n");
    }
    else
    {
        // wait();
    }

    exit();
}