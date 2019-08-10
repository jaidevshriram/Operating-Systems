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
    for(int i=0; i<=size/2; i++)
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

int calcprogress(float fraction)
{
    return floorl(fraction); 
}

int main(int argc, char *argv[])
{
    if( argc < 2 ) {
       sys_print("No File Specified\n");
        return 1;
    }

    //Size of the input
    int size = calcsize( argv[1] );

    //Open the file
    int fd = open(argv[1], O_RDONLY);

    if( fd < 0) {
        perror("Unavailable file");
        return 1;
    }

    char *filename = basename(argv[1]);
    char newfilepath[100] = "./Assignment/";
    strcat(newfilepath, filename);

    mkdir("./Assignment/", 600);

    //Open file to write too
    int fw = open(newfilepath, O_CREAT | O_WRONLY | O_APPEND, 600);

    int bytesread = 0, chunksize = 5000;

    //Set pointer to end of file initially
    lseek(fd, 0, 2);

    while (bytesread < size)
    {
        //If file is smaller than the chunksize
        if(chunksize > size) {
            
            lseek(fd, 0, 0);

            char *str = malloc(size);
            read(fd, str, size);
            
            strrev(str, size);
            
            write(fw, str, size);
            
            free(str);

            bytesread = size;
        }
        else {

            //If you are at start of the file
            if((size-bytesread) < chunksize) {
                char *str = malloc(size-bytesread);
                lseek(fd, 0, 0);
                read(fd, str, (size-bytesread));
                strrev(str, (size-bytesread));
                write(fw, str, (size-bytesread));
                free(str);

                bytesread = size;
            }

            //For cases where chunksize can be read
            else {
                char str[5000];
                movereadback(fd, chunksize);
                read(fd, str, chunksize);
                strrev(str, chunksize);
                write(fw, str, chunksize);
                movereadback(fd, chunksize);

                bytesread += chunksize;
            }
        }
    
        char output[100];
        sprintf(output,"\r%d percent done.\0", calcprogress((bytesread*100.0)/size));
        write(1, output, strlen(output));
    }

    close(fd);
    close(fw);

    return 0;
}