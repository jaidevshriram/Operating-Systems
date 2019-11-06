#include "types.h"
#include "stat.h"
#include "user.h"
#include "proc_stat.h"

int main(int argc, char *argv[])
{
    struct proc_stat a;
    getpinfo(&a, atoi(argv[1]));

    printf(1, "pid: %d\n", a.pid);
    printf(1, "runtime: %lf\n", a.runtime);
    printf(1, "num_run: %d\n", a.num_run);
    printf(1, "current_queue: %d\n", a.current_queue);
    for(int i=0; i<5; i++)
        printf(1, "ticks in queue %d is %d\n", i, a.ticks[i]);

    exit();
}