#include    <stdio.h>
#include    <sys/stat.h>
#include    <fcntl.h> 
#include    <unistd.h>
#include    <stdlib.h>
#include    <string.h>
#include    <math.h>
#include    <libgen.h>

//Prints to screen
void sys_print(char *a)
{
    write(1, a, strlen(a));
}

//Calculates size of the file
int calcsize(char *file)
{
    struct stat statbuf;
    stat(file, &statbuf);
    return statbuf.st_size;
}

//Reverses the string
void strrev(char *str, int size)
{
    for(int i=0; i<size/2; i++)
    {
        char temp = str[i];
        str[i] = str[size-i-1];
        str[size-i-1] = temp;
    }
}

//Moves the read file pointer back by the offset
void movereadback(int fd, int offset) 
{
    lseek(fd, -offset, 1);
}

void checkdir(char *dirname)
{
    if( access(dirname, F_OK) !=0 )
        sys_print("Diretory is created: No\n");
    else
        sys_print("Directory is created: Yes\n");
}

int checkreverse(char *oldfile, char *newfile)
{
    int fd1 = open(oldfile, O_RDONLY);
    int fd2 = open(newfile, O_RDONLY);

    if( fd1==-1 || fd2==-1 ) {
        perror("One or more files do not exist");
        return -1;
    }

    if( calcsize(oldfile) != calcsize(newfile) ) {
        perror("Files are not reversed versions of the other\n");
        sys_print("Sizes are different\n");
        return -1;
    }
    else {
        
        int bytesread = 0, chunksize = 5000, size=calcsize(oldfile);

        //Set pointer to end of file initially
        lseek(fd2, 0, 2);

        sys_print("Comparing files ...\n");

        while (bytesread < size)
        {
            //If file is smaller than the chunksize
            if(chunksize > size) {
                
                lseek(fd2, 0, 0);

                char *str = malloc(size);
                read(fd2, str, size); 
                strrev(str, size);

                char *str2 = malloc(size);
                read(fd1, str2, size);

                if(strcmp(str, str2)!=0) {
                    sys_print("Files are not reversed versions of the other\n");
                    return 2;
                }
                free(str);
                free(str2);
                bytesread = size;
            }
            else {

                //If you are at start of the file
                if((size-bytesread) < chunksize) {

                    lseek(fd2, 0, 0);

                    char *str = malloc(size-bytesread+1);
                    read(fd2, str, (size-bytesread));
                    strrev(str, (size-bytesread));

                    char *str2 = malloc((size-bytesread)+1);
                    read(fd1, str2, (size-bytesread));

                    str[0] = str2 [0] = '\0';

                    if(strcmp(str, str2)!=0) {
                        sys_print("Files are not reversed versions of the other\n");
                        return 3;
                    }

                    free(str);
                    free(str2);

                    bytesread = size;
                }

                //For cases where chunksize can be read
                else {
                    char str[5001];
                    movereadback(fd2, chunksize);
                    read(fd2, str, chunksize);
                    // sys_print(str);
                    // sys_print("\n");
                    strrev(str, chunksize);

                    char str2[5001];
                    read(fd1, str2, chunksize);

                    str[0] = str2 [0] = '\0';

                    if(strcmp(str, str2)!=0) {
                        sys_print("Files are not reversed versions of the other\n");
                        // printf("%d bytes read so far succesffully\n", bytesread);
                        return 4;
                    }

                    movereadback(fd2, chunksize);

                    bytesread += chunksize;
                }
            }
        }
    }

    sys_print("File contents are reversed in newfile: Yes\n");    

    return 0;
}

void permdisplay(char *type, char *permtype, char *filename, int hasperm)
{
    char buffer[100];
    if(hasperm)
        sprintf(buffer, "%s has %s permission on %s:%s\n\0", type, permtype, filename, "yes");
    else
        sprintf(buffer, "%s has %s permission on %s:%s\n\0", type, permtype, filename, "no");
    sys_print(buffer);
}

int perm(char *filetype, char *file)
{
    struct stat filestat;
    if(stat(file, &filestat) == -1) {
        perror("File/Directory does not exist");
        return -1;
    }

    permdisplay("User", "read", file, filestat.st_mode & S_IRUSR);
    permdisplay("User", "write", file, filestat.st_mode & S_IWUSR);
    permdisplay("User", "execute", file, filestat.st_mode & S_IXUSR);

    sys_print("\n");

    permdisplay("Group", "read", file, filestat.st_mode & S_IRGRP);
    permdisplay("Group", "write", file, filestat.st_mode & S_IWGRP);
    permdisplay("Group", "execute", file, filestat.st_mode & S_IXGRP);

    sys_print("\n");
    
    permdisplay("Other", "read", file, filestat.st_mode & S_IROTH);
    permdisplay("Other", "write", file, filestat.st_mode & S_IWOTH);
    permdisplay("Other", "execute", file, filestat.st_mode & S_IXOTH);

    sys_print("\n-----\n");
}

int main(int argc, char *argv[])
{
    if( argc < 4 ) {
        sys_print("Missing command line arguments\n");
        sys_print("Usage: ./a.out newfile oldfile directory");
        return 1;
    }

    checkdir(argv[3]);
    checkreverse(argv[1], argv[2]);
    perm("newfile", argv[1]);
    perm("oldfile", argv[2]);
    perm("directory", argv[3]);

    return 0;
}