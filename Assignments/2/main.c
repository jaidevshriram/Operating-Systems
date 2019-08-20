#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>

// //Custom header files
// #include<changedir.h>

#define STDIN 0

char *username, hostname[100];
char pwd[1000];

char ** tokenize_input(char *input)
{
	char **tokenized_input = malloc(1000);

	char *input_part;

	int position = 0;

	input_part = strtok(input, " ");

	while(input_part)
	{
		tokenized_input[position++] = input_part;
		input_part = strtok(0, " ");
	}

	return tokenized_input;
}

void cd(char **command)
{
	command[1] = strtok(command[1], "\n");
	printf("%d code\n",chdir(command[1]));
}

void start_command_execution(char *input)
{
	char **tokenized_input = tokenize_input(input);

	char **command_list = malloc(1000);

	command_list[0] = "cd";
	command_list[1] = "pwd";
	command_list[2] = "dir";

	int command_count = 3;

	int commandfound = 0, i;

	for(i=0; !commandfound && i<command_count; i++)
	{
		if(strcmp(command_list[i], tokenized_input[0])==0)
			commandfound = 1;
	}

	if(commandfound == 0)
		printf("Command does not exist.\n");
	else
	{
		switch((i-1))
		{
			case 0: cd(tokenized_input);
		}
	}
	
}

char* input_is_triggered()
{

	unsigned long int size = 1000;
	char *input = malloc(size);

	if(input == NULL)
	{
		perror("Unable to allocate memory");
	}

	getline(&input, &size, stdin);

	start_command_execution(input);

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
		initialize();
		printf("\r\n%s@%s:%s$ ", username, hostname, pwd);

		if(!input_is_triggered())
			continue;

	}

	return 0;
}