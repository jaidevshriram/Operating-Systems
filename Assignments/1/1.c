#include    <stdio.h>
#include    <sys/stat.h>
#include    <fcntl.h> 
#include    <unistd.h>
#include    <stdlib.h>
#include    <string.h>

void sys_print(char *a)
{
    printf("%s\n", a);
}

int calcsize(char *file)
{
    struct stat statbuf;
    stat(file, &statbuf);
    return statbuf.st_size;
}

void strrev(char *str, int size)
{
    for(int i=0; i<=size/2; i++)
    {
        char temp = str[i];
        str[i] = str[size-i-1];
        str[size-i-1] = temp;
    }
}

int main(int argc, char *argv[])
{
    if( argc < 2 ) {
       sys_print("No File Specified\n");
        return 1;
    }

    //Size of the input
    int size = calcsize( argv[1] );
    int chunksize = 5000;

    //Open the file
    int fd = open(argv[1], O_RDONLY);

    //Open file to write too
    int fw = open("foo.txt", O_CREAT | O_WRONLY | O_APPEND);

    if( fd < 0 || fw < 0) {
        perror("Unavailable file");
        return 1;
    }

    // Place input file pointer at end of file
    int preseek = (size>=chunksize) ? chunksize : size;
    int bytesread = 0;
    
    lseek(fd, -preseek, 2);

    //Go about the chunks of blocks
    for(int i=0; i<=size; i+=chunksize) {

        //Read chunksize or <chunksize blocks?
        int readsize = (size-i) >= chunksize ? chunksize : (size-i);

        //Create string for reversal
        char c[5000];
        
        
        read(fd, c, readsize);
        bytesread += readsize;

        //Reverse the string
        // strrev(c, readsize);

        write(fw, c, readsize);
        
        lseek(fd, (size-bytesread) >= chunksize ? -2*chunksize : , 1)

        printf("\r%f done", ((i*1.0)/size)*100);
    }

    close(fd);
    close(fw);

    return 0;
}