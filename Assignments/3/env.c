#include<stdlib.h>
#include<stdio.h>

int set_env(char **tokenized_input, int count)
{
    if(count<=1 || count>=4)
    {
        printf("\nUsage: setenv var value");
        return -1;
    }

    if(count == 2)
    {
        if(setenv(tokenized_input[1], "", 1) == -1)
            perror("Shell: setenv failed");
    }
    else
    {
        if(setenv(tokenized_input[1], tokenized_input[2], 1) == -1)
            perror("Shell: setenv failed");
    }

    return 0;    
}

int unset_env(char **tokenized_input, int count)
{
    if(count!=2)
    {
        printf("\nUsage: unsetenv");
        return -1;
    }

    if(unsetenv(tokenized_input[1]) == -1)
        perror("Shell: unsetenv failed");
    
    return 0;    
}