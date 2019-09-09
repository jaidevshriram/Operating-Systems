#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>

int shell_pid, ischild;

void catch_ctrl_c(int temp)
{
	// printf("\nCTRL C caught\n");
	// printf("this is being killed:%d\n", getpid());

	if(getpid() != shell_pid)
    	kill(getpid(), SIGKILL);
	// else
	// 	_exit(1);
}

void initialize_signal_handlers()
{
	signal(SIGINT, catch_ctrl_c);
}

void set_shell_pid()
{
	shell_pid = getpid();
}