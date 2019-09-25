#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include "jobs.h"

int shell_pid, ischild;

void catch_ctrl_c(int temp)
{
	if(getpid() == shell_pid && ischild!=0)
    	kill(ischild, SIGINT);
	signal(SIGINT, catch_ctrl_c);
	fflush(stdout);
}

void catch_ctrl_z(int temp)
{
	if(getpid() == shell_pid && ischild!=0)
	{
		kill(ischild, SIGTSTP);
		kill(ischild, SIGINT);
		change_pid_status(ischild, 0);
	}
	fflush(stdout);
	signal(SIGTSTP, catch_ctrl_z);
}

void initialize_signal_handlers()
{
	signal(SIGINT, catch_ctrl_c);
	signal(SIGTSTP, catch_ctrl_z);
}

void set_shell_pid()
{
	shell_pid = getpid();
}

void set_child_pid(int child_pid)
{
	ischild = child_pid;
}