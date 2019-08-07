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
    // printf("%d\n", statbuf.st_size );
    return statbuf.st_size;
}

void strrev(char *str)
{
    for(int i=0; i<=strlen(str)/2; i++)
    {
        char temp = str[i];
        str[i] = str[strlen(str)-i-1];
        str[strlen(str)-i-1] = temp;
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
    int preseek = (size>=5000) ? 5000 : size;
    lseek(fd, -preseek, 2);

    //Go about the chunks of blocks
    for(int i=0; i<=size; i+=5000) {

        //Read 5000 or <5000 blocks?
        int readsize = (size-i) >= 5000 ? 5000 : (size-i);
        
        // printf("read: %d\n", readsize);

        //Create string for reversal
        char *c = malloc(readsize);
        
        read(fd, c, readsize);
        // printf("%s\n", c);

        //Reverse the string
        strrev(c);
        // printf("%s\n", c);

        write(fw, c, readsize);
        
        lseek(fd, -6000, 1);

        // printf("\r%f done", ((i*1.0)/size)*100);

        free(c);
    }

    return 0;
}