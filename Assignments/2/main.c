#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

#define STDIN 0

char *username, hostname[100];
char pwd[1000];

char* input_is_triggered()
{

	unsigned long int size = 1000;
	char *input = malloc(size);

	if(input == NULL)
	{
		perror("Unable to allocate memory");
	}

	getline(&input, &size, stdin);

	return input;
}

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

int main(int argc, char const *argv[])
{

	initialize();

	while(1)
	{

		printf("\r\n<%s@%s:%s>  ", username, hostname, pwd);

		if(!input_is_triggered())
			continue;

	}

	return 0;
}