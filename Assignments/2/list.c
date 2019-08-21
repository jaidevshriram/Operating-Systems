#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/dir.h>
#include "helper.h"

int ls(char **tokenized_input, char *input)
{
    int def = 1, hidden = 0, longv = 0, dir = 0, exit = 0;
    char dirname[100];

    switch(count_tokens(input))
    {
        case 1: break;
        case 2: {
            if(tokenized_input[1][0]=='-')
            {
                for(int i=1; i<strlen(tokenized_input[1]); i++)
                {
                    if(tokenized_input[1][i]!='a' || tokenized_input[1][i]=='l')
                        exit = 1;
                    else if (tokenized_input[1][i]=='a')
                        hidden = 1;
                    else
                        longv = 1;
                }
            }
            else
            {
                dir = 1;
                strcpy(dirname, tokenized_input[1]);
            }
            break;
        }
        case 3: {
            for(int i=1; i<3; i++)
            {
                if(tokenized_input[i][0]=='-')
                {
                    for(int j=1; j<strlen(tokenized_input[1]); j++)
                    {
                        if(tokenized_input[i][j]!='a' || tokenized_input[i][j]=='l')
                            exit = 1;
                        else if (tokenized_input[1][j]=='a')
                            hidden = 1;
                        else
                            longv = 1;
                    }
                }
                else {
                    if(dir==1)
                        exit = 1;
                    else
                    {
                        dir = 1;
                        strcpy(dirname, tokenized_input[i]);
                    }
                }
            }

            break;
        }

        case 4: {
            exit = 1;
            break;
        }
    }

    if(exit)
    {
        printf("Incorrect input format for ls. Please check flags\n");
        return -1;
    }

    if(dir)
    {
        struct stat statbuf;
        if(stat(dirname, &statbuf)!=0)
        {
            printf("File/Directory %s does not exist.\n", tokenized_input[1]);
            return -1;
        }

        DIR *directory_pointer = opendir(dirname);
        struct dirent *entry = readdir(directory_pointer);

        while(entry)
        {
            printf("%s\n", entry->d_name);
            entry = readdir(directory_pointer);
        }
    }

}