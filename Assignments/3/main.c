#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include<string.h>

//Custom header files
#include "changedir.h"
#include "helper.h"
#include "pwd.h"
#include "echo.h"
#include "list.h"
#include "system.h"
#include "pinfo.h"
#include "history.h"
#include "catchsig.h"

extern char *username, hostname[100], pwd[1000];

char **tokenize_input(char *input, char *delimiters)
{
	while(iswhitespace(*input))
		input++;

	char str[1000];
	strcpy(str, input);

	trimTrailing(str);

	char **tokenized_input = (char **) malloc(1000);

	char *input_part;

	int position = 0;

	input_part = strtok(str, delimiters);

	while(input_part)
	{
		tokenized_input[position++] = malloc(strlen(input_part));
		strcpy(tokenized_input[position-1], input_part);
		input_part = strtok(NULL, delimiters);
	}

	return tokenized_input;
}

void start_command_execution(char *input)
{
	char **tokenized_input = tokenize_input(input, "\r\n\t ");
	char command_list[100][1000];
	char command[100];

	strcpy(command, tokenized_input[0]);
	strcpy(command_list[0], "cd");
	strcpy(command_list[1], "pwd");
	strcpy(command_list[2], "echo");
	strcpy(command_list[3], "ls");
	strcpy(command_list[4], "pinfo");
	strcpy(command_list[5], "history");
	strcpy(command_list[6], "fg");
	strcpy(command_list[7], "quit");

	int command_count = 8;

	int command_found = 0, i;

	for(i=0; command_found == 0 && i<command_count; i++)
	{
		if(strcmp(command, command_list[i]) == 0)
			command_found = 1;
	}

	if(command_found == 0)
		launch_command(tokenized_input);
	else
	{
		switch((i-1))
		{
			case 0: cd(tokenized_input); break;
			case 1: print_pwd(); break;
			case 2: echo(input); break;
			case 3: ls(tokenized_input, input); break;
			case 4: pinfo(tokenized_input, count_tokens(input)); break;
			case 5: history(tokenized_input, count_tokens(input)); break;
			case 6: fg(tokenized_input, count_tokens(input)); break;
			case 7: _exit(0); break;
			default: launch_command(tokenized_input); break;
		}
	}

	add_history(input);
	free(tokenized_input);
}

void checkinput(char *input)
{
	char **tokenized_input = tokenize_input(input, "<");
	
	free(tokenized_input);
}

void start_redirect_handler(char *input)
{
	char *input_file = malloc(1000);
	int infile = checkinput(input, input_file);	

	free(input_file);
}

void start_command_chain(char *input)
{
	char **tokenized_input = tokenize_input(input, ";");
	for (int i=0; tokenized_input[i]!=NULL; i++)
		start_redirect_handler(tokenized_input[i]);
	free(tokenized_input);
}

int input_is_triggered()
{
	unsigned long int size = 1000;
	char *input = (char *) malloc(size);

	if(input == NULL)
	{
		perror("Unable to allocate memory");
	}

	getline(&input, &size, stdin);

	start_command_chain(input);
	free(input);
	return 1;
}

int main(int argc, char const *argv[])
{

	initialize();
	initialize_history();
	initialize_signal_handlers();
	set_shell_pid();

	while(1)
	{
		update();
		set_child_pid(0);
		initialize_signal_handlers();
		char prompt[1000];
		sprintf(prompt, "\r\n%s@%s:%s$ ", username, hostname, home_based(pwd));
		printf("%s", prompt);

		if(!input_is_triggered())
			continue;
	}

	return 0;
}