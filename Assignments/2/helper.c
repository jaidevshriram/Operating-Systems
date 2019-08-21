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

char *translate_home(char *path)
{
    if(path[0]=='~' && strlen(path)!=1)
    {
        path++;
        updatepwd();
        char newpath[1000] = "/home/";
        strcat(newpath, getenv("USER"));
        strcat(newpath, path);
        path = newpath;
    }
    else if(path[0]=='~')
    {
        char newpath[1000] = "/home/";
        strcat(newpath, getenv("USER"));
        path = newpath;
    }

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

	char **tokenized_input = (char **) malloc(1000);

	char *input_part;

	int position = 0;

	input_part = strtok(str, " ");

	while(input_part)
	{
		tokenized_input[position++] = malloc(strlen(input_part));
        tokens++;
		strcpy(tokenized_input[position-1], input_part);
		input_part = strtok(NULL, " ");
	}

	return tokens;
}