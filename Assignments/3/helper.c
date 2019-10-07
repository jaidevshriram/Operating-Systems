#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include "jobs.h"

#define STDIN 0

char *username, hostname[100], pwd[1000], initial_pwd[1000];

void updatepwd()
{
	getcwd(pwd, 1000);
}

void initialize() 
{
	username = getenv("USER");
	gethostname(hostname, 100);
    getcwd(initial_pwd, 100);
	updatepwd();
    start_pid_queue();
}

void update()
{
	username = getenv("USER");
	gethostname(hostname, 100);
	updatepwd();
    check_pid_status();
}

char *translate_home(char *path)
{
    char *newpath = malloc(1000);
    strcpy(newpath, path);

    if(path[0]=='~' && strlen(path)!=1)
    {
        path++;
        updatepwd();
        strcpy(newpath, initial_pwd);
        strcat(newpath, path);
    }
    else if(path[0]=='~')
    {
        strcpy(newpath, initial_pwd);
    }

    return newpath;
}

char *home_based(char *path)
{
    char *newpath = malloc(1000);
    if(strlen(path) >= strlen(initial_pwd))
    {
        if(strncmp(path, initial_pwd, strlen(initial_pwd)) == 0)
        {
            char *temp = malloc(1000);

            int j = 0;
            for(int i=strlen(initial_pwd); path[i]; i++)
                temp[j++] = path[i];
            temp[j]='\0';         
            
            strcpy(newpath, "~");
            strcat(newpath, temp);

            free(temp);
            return newpath;
        }
        else
            return path;
    }
    // else
        return path;
}

int iswhitespace(char c)
{
    if(c == ' ' || c == '\t' || c == '\n')
        return 1;
    else
        return 0; 
}

void trimTrailing(char * str)
{
    int index, i;

    /* Set default index */
    index = -1;

    /* Find last index of non-white space character */
    i = 0;
    while(str[i] != '\0')
    {
        if(str[i] != ' ' && str[i] != '\t' && str[i] != '\n')
        {
            index= i;
        }

        i++;
    }

    /* Mark next character to last non-white space character as NULL */
    str[index + 1] = '\0';
}

int custom_strcmp(const char a[], const char b[])
{
    int ca = strlen(a);
    int cb = strlen(b);

    if(ca!=cb)
    {
        printf("%d %d\n", ca, cb);
        return -1;
    }
    else
    {
        for(int i=0; i<ca; i++)
            if(a[i]!=b[i])
            {
                printf("%c %c\n", a[i], b[i]);
                return -1;
            }
            else
            {
                printf("%c %c %d\n", a[i], b[i], i);
            }
            
    }

    printf("end");

    return 0;
}

int count_tokens(char *input)
{

    int tokens = 0;

	while(iswhitespace(*input))
		input++;

	char str[1000];
	strcpy(str, input);

	trimTrailing(str);

	char *input_part;

	int position = 0;

	input_part = strtok(str, " ");

	while(input_part)
	{
        tokens++;
		input_part = strtok(NULL, " ");
	}

	return tokens;
}

void removewhitespace(char *old, char *new)
{
    while(iswhitespace(*old))
		old++;
	strcpy(new, old);
	trimTrailing(new);
}

int countfreq(char pattern[], char string[])
{
    int patlength = strlen(pattern);
    int strlength = strlen(string);

    int count = 0;

    for(int i=0; i<=strlength - patlength; i++)
    {
        int j;
        for(int j=0; j<patlength; j++)
            if(string[i+j]!=pattern[j])
                break;
        
        if(j==patlength)
            count++;
    }

    return count;
}