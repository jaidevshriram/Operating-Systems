#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
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

extern char *username, hostname[100], pwd[1000];

char **tokenize_input(char *input)
{
	while(iswhitespace(*input))
		input++;

	char str[1000];
	strcpy(str, input);

	trimTrailing(str);

	char **tokenized_input = (char **) malloc(1000);

	char *input_part;

	int position = 0;

	input_part = strtok(str, " ");

	while(input_part)
	{
		tokenized_input[position++] = malloc(strlen(input_part));
		strcpy(tokenized_input[position-1], input_part);
		input_part = strtok(NULL, " ");
	}

	return tokenized_input;
}

char **tokenize_input_semicolin(char *input)
{
	while(iswhitespace(*input))
		input++;

	char str[1000];
	strcpy(str, input);

	trimTrailing(str);

	char **tokenized_input = (char **) malloc(1000);

	char *input_part;

	int position = 0;

	input_part = strtok(str, ";");

	while(input_part)
	{
		tokenized_input[position++] = malloc(strlen(input_part));
		strcpy(tokenized_input[position-1], input_part);
		input_part = strtok(NULL, ";");
	}

	return tokenized_input;
}


void start_command_execution(char *input)
{
	char **tokenized_input = tokenize_input(input);
	char command_list[100][1000];
	char command[100];

	strcpy(command, tokenized_input[0]);
	strcpy(command_list[0], "cd");
	strcpy(command_list[1], "pwd");
	strcpy(command_list[2], "echo");
	strcpy(command_list[3], "ls");
	strcpy(command_list[4], "pinfo");
	strcpy(command_list[5], "history");

	int command_count = 6;

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
			default: launch_command(tokenized_input); break;
		}
	}

	add_history(input);
}

void start_command_chain(char *input)
{
	char **tokenized_input = tokenize_input_semicolin(input);
	for (int i=0; tokenized_input[i]!=NULL; i++)
		start_command_execution(tokenized_input[i]);
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

	return 1;
}

int main(int argc, char const *argv[])
{

	initialize();
	initialize_history();

	while(1)
	{
		update();
		char prompt[1000];
		sprintf(prompt, "\r\n%s@%s:%s$ ", username, hostname, home_based(pwd));
		write(1, prompt, strlen(prompt));

		if(!input_is_triggered())
			continue;

	}

	return 0;
}