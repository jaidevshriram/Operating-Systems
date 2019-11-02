#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
    int wtime, rtime, status;
    status = wtime = rtime = -1;
    int pid = fork();
    if(pid == 0)
    {
        exec(argv[1], argv);
        printf(1, "Invalid Command\n");
    }
    else if(pid < 0)
    {
        printf(1, "Forking Failed\n");
    }
    else
    {
        status = waitx(&wtime, &rtime);
    }

    printf(1, "Wait Time: %d and Run TIme: %d and Status: %d\n", wtime, rtime, status);
    exit();
}