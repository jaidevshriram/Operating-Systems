#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/dir.h>
#include "helper.h"

void permission_printer(struct stat statbuf, char output[])
{   
    if(S_ISDIR(statbuf.st_mode))
        strcat(output, "d");
    else
        strcat(output, "-");

    statbuf.st_mode & S_IRUSR == 1 ? strcat(output, "r") : strcat(output, "-");
    statbuf.st_mode & S_IWUSR == 1 ? strcat(output, "w") : strcat(output, "-");
    statbuf.st_mode & S_IXUSR == 1 ? strcat(output, "x") : strcat(output, "-");
    statbuf.st_mode & S_IRGRP == 1 ? strcat(output, "r") : strcat(output, "-");
    statbuf.st_mode & S_IWGRP == 1 ? strcat(output, "w") : strcat(output, "-");
    statbuf.st_mode & S_IXGRP == 1 ? strcat(output, "x") : strcat(output, "-");
    statbuf.st_mode & S_IROTH == 1 ? strcat(output, "r") : strcat(output, "-");
    statbuf.st_mode & S_IWOTH == 1 ? strcat(output, "w") : strcat(output, "-");
    statbuf.st_mode & S_IXOTH == 1 ? strcat(output, "x") : strcat(output, "-");
}

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
                    if(tokenized_input[1][i]!='a' && tokenized_input[1][i]!='l')
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
                // printf("%s\n", tokenized_input[i]);
                if(tokenized_input[i][0]=='-')
                {
                    for(int j=1; j<strlen(tokenized_input[i]); j++)
                    {
                        if(tokenized_input[i][j]!='a' && tokenized_input[i][j]!='l')
                            exit = 1;
                        else if (tokenized_input[i][j]=='a')
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

    if(dir == 0)
        strcpy(dirname, ".");

    struct stat statbuf;
    if(stat(dirname, &statbuf)!=0)
    {
        printf("File/Directory %s does not exist.\n", dirname);
        return -1;
    }

    if(S_ISDIR(statbuf.st_mode))
    {
        DIR *directory_pointer = opendir(dirname);
        struct dirent *entry = readdir(directory_pointer);
        while(entry)
        {
            int display = 0;

            struct stat filestat;
            stat(entry->d_name, &filestat);

            if(hidden == 1 && entry->d_name[0]=='.')
                display = 1;
            else if(entry->d_name[0]!='.')
                display = 1;

            if(display)
            {
                if(!longv)
                    printf("%s\n", entry->d_name);
                else
                {
                    char perm[100];
                    permission_printer(filestat, perm);
                    printf("%s %2d %s %s %10d %s %s\n", perm , filestat.st_nlink, filestat.st_uid, filestat.st_gid, filestat.st_atime, entry->d_name);
                }
            }
            
            entry = readdir(directory_pointer);
        }
    }
    else
    {
        printf("%s\n", dirname);
    }
}