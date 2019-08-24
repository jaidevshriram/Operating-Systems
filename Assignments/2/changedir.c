#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include "helper.h"

void cd(char **command)
{
	command[1] = strtok(command[1], "\n");
	char *newpath = translate_home(command[1]);
	if(chdir(newpath)==-1)
		perror("Shell");
}