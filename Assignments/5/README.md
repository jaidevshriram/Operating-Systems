# xv6 Operation System

## Task 1

All modified files for task-1 have been placed in the folder Modified_Files_Task_1
There is a command `test_task_1` which takes a command as input and outputs the waiting, running time along with status code.

Usage: `test_task_1 <command> [args]`

## Task 2

### FCFS

This just chooses the process with lowest process creation time in the process table. 

### Priority Based

This chooses processes with lowest priority first. Using user program `set_priority` users can change the priority of a process during run time to the specified one. This changes the process being run as well since processes yield at every tick.

### MLFQ

Clearly multilevel feedback queue is the better of all the scheduling algorithms implemented. Priority and FCFS suffer from prolonged waiting time depending on the priority or arrival time of a process. Since MLFQ is based on the tick usage of the process, it is fair to all processes. Further, this implementation prevents starvation due to upgradation of processes if it doesn't get control of the CPU after some time