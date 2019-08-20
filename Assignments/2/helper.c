#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

#define STDIN 0

char *username, hostname[100], pwd[1000];

void updatepwd()
{
	getcwd(pwd, 1000);
}

void initialize() 
{
	username = getenv("USER");
	gethostname(hostname, 100);
	updatepwd();
}
