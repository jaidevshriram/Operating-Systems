#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include "helper.h"

char **tokenize_input_echo(char *input)
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

    // printf("%s\n", input_part);

    tokenized_input[0] = malloc(strlen(input_part));
    strcpy(tokenized_input[0], input_part);

    input+=(strlen(input_part)+1);

    printf("%s\n", input);

    tokenized_input[1] = malloc(strlen(str));
    strcpy(tokenized_input[1], str);

	return tokenized_input;
}

char *formattedecho(char *string);

int echo(char *command)
{
   	command = strtok(command, "\n");
    char **tokenized_input_echo = tokenize_input_echo(command);
	return 0;
}