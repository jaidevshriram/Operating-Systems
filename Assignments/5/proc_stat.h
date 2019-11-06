//Information about process
struct proc_stat {
  int pid;                      // PID of each Process
  float runtime;                // Runtime of the process
  int num_run;                  // Number of times, process is executed
  int current_queue;            // Currently Assigned Queue
  int ticks[5];                 // Number of ticks each process has received at each of the 5 priority queue
};
