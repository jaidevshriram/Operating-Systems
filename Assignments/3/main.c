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

void start_redirect_handler(char *input)
{
	char **tokenized_input_redirect = tokenize_input(input, "<");
	
	char **tokenized_output_redirect;
	char **tokenized_input;

	int infile = 0;
	char *infile_name = malloc(1000);
	for(infile=0; tokenized_input_redirect[infile]!=NULL; infile++);

	if(infile<=1)
	{
		tokenized_output_redirect = tokenize_input(tokenized_input_redirect[0], ">");
		infile = 0;
	}
	else
	{
		infile = 1;
		char *temp = malloc(1000);
		char **extracted_file_from_tokens = tokenize_input(tokenized_input_redirect[1], " \r\t");
		
		strcpy(temp, extracted_file_from_tokens[0]);
		removewhitespace(temp, infile_name);
		
		free(temp);
		free(extracted_file_from_tokens);

		//Other initializations
		tokenized_output_redirect = tokenize_input(tokenized_input_redirect[1], ">");
	}

	int outfile = 0;
	char *outfile_name = malloc(1000);
	for(outfile = 0; tokenized_output_redirect[outfile]!=NULL; outfile++);

	if(outfile<=1)
		outfile = 0;
	else
	{
		outfile = 1;
		char *temp = malloc(1000);
		strcpy(temp, tokenized_output_redirect[1]);
		removewhitespace(temp, outfile_name);
		free(temp);
	}

	int tempin = dup(0);
	int tempout = dup(1);

	int fdin;

	if(infile)
		fdin = open(infile_name, O_RDONLY);
	else
		fdin = dup(tempin);

	//Initialize Simple Command list
	tokenized_input = tokenize_input(tokenized_output_redirect[0], "|");

	int ret, fdout = dup(tempout), countsimple = 0;

	for(countsimple = 0; tokenized_input[countsimple]!=NULL; countsimple++)
		printf("%d: %s", countsimple, tokenized_input[countsimple]);

	for (int i=0; tokenized_input[i]!=NULL; i++)
	{
		if(fdin == -1)
		{
			fprintf(stderr, "Unable to open input file %s\n", infile_name);
			return -1;
		}
		
		dup2(fdin, 0);
		close(fdin);

		if(i==0 && infile)
		{
			// fprintf(stderr, "2. %s is being executed\n", tokenized_input_redirect[0]);
			if(outfile)
				fdout = open(outfile_name, O_CREAT | O_WRONLY | O_APPEND, 0644);
			else
				fdout = dup(tempout);

			dup2(fdout, 1);
			close(fdout);

			start_command_execution(tokenized_input_redirect[0]);
			continue;
		}

		if(i == countsimple - 1)
		{
			if(outfile)
				fdout = open(outfile_name, O_CREAT | O_WRONLY | O_APPEND, 0644);
			else
				fdout = dup(tempout);
		}
		else
		{
			int fdpipe[2];
			pipe(fdpipe);
			fdout = fdpipe[1];
			fdin = fdpipe[0];
		}

		dup2(fdout, 1);
		close(fdout);

		// fprintf(stderr, "%s is being executed\n", tokenized_input[i]);
		start_command_execution(tokenized_input[i]);
	}

	dup2(tempin, 0);
	dup2(tempout, 1);
	
	close(tempin);
	close(tempout);
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