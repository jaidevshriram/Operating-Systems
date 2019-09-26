#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include "helper.h"

int cd(char **command)
{
	command[1] = strtok(command[1], "\n");
	char *newpath = translate_home(command[1]);
	if(chdir(newpath)==-1)
	{
		perror("Shell");
		free(newpath);
		return -1;
	}
	free(newpath);
	return 0;
}