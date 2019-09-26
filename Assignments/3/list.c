#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/dir.h>
#include<pwd.h>
#include<grp.h>
#include<time.h>
#include "helper.h"

void permission_printer(struct stat statbuf, char output[])
{   
    strcpy(output, "");
    
    if(S_ISDIR(statbuf.st_mode))
        strcat(output, "d");
    else
        strcat(output, "-");

    statbuf.st_mode & S_IRUSR  ? strcat(output, "r") : strcat(output, "-");
    statbuf.st_mode & S_IWUSR  ? strcat(output, "w") : strcat(output, "-");
    statbuf.st_mode & S_IXUSR  ? strcat(output, "x") : strcat(output, "-");
    statbuf.st_mode & S_IRGRP  ? strcat(output, "r") : strcat(output, "-");
    statbuf.st_mode & S_IWGRP  ? strcat(output, "w") : strcat(output, "-");
    statbuf.st_mode & S_IXGRP  ? strcat(output, "x") : strcat(output, "-");
    statbuf.st_mode & S_IROTH  ? strcat(output, "r") : strcat(output, "-");
    statbuf.st_mode & S_IWOTH  ? strcat(output, "w") : strcat(output, "-");
    statbuf.st_mode & S_IXOTH  ? strcat(output, "x") : strcat(output, "-");

    strcat(output, "\0");
}

int ls(char **tokenized_input, char *input)
{
    int def = 1, hidden = 0, longv = 0, dir = 0, exit = 0;
    char *dirname = malloc(1000);

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

    dirname = translate_home(dirname);

    if(exit)
    {
        printf("Incorrect input format for ls. Please check flags\n");
        return -1;
    }

    if(dir == 0)
        strcpy(dirname, "./");

    struct stat statbuf;
    if(stat(dirname, &statbuf)!=0)
    {
        printf("File/Directory %s does not exist.\n", dirname);
        free(dirname);
        return -1;
    }

    if(S_ISDIR(statbuf.st_mode))
    {
        strcat(dirname, "/");

        DIR *directory_pointer = opendir(dirname);
        struct dirent *entry = readdir(directory_pointer);
        while(entry)
        {
            int display = 0;

            char updated_file_path[1000];
            strcpy(updated_file_path, dirname);

            strcat(updated_file_path, entry->d_name);

            struct stat filestat;
            if(stat(updated_file_path, &filestat)!=0)
            {
                printf("Unable to acces file %s\n", updated_file_path);
                entry = readdir(directory_pointer);
                continue;
            }

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
                    char perm[1000];
                    permission_printer(filestat, perm);

                    struct passwd *user = getpwuid(filestat.st_uid);
                    struct group *group = getgrgid(filestat.st_gid);

                    char time[1000];
                    strftime(time, 1000, "%b %d %H:%M", localtime(&filestat.st_mtime));

                    printf("%s %2d %s %s %10d %s %s\n", perm , filestat.st_nlink, user->pw_name , group->gr_name, filestat.st_size,  time, entry->d_name);
                }
            }
            
            entry = readdir(directory_pointer);
        }
    }
    else
    {
        printf("%s\n", dirname);
    }

    free(dirname);
    return 0;
}