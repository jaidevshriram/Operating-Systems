void start_pid_queue();
void add_pid_queue(int pid, char name[], int stat);
void check_pid_status();
int check_pid_exist(int pid);
void change_pid_status(int pid, int newstat);
void delete_pid_queue(int pid);
int jobs();
int overkill();