#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

void cd(char **command)
{
	command[1] = strtok(command[1], "\n");
	// command [1] = translate_home(command[1]);
	chdir(command[1]);
}