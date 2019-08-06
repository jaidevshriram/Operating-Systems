#include    <stdio.h>
#include    <sys/stat.h>
#include    <fcntl.h> 
#include    <unistd.h>
#include    <stdlib.h>

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

    int fw = open("foo.txt", O_CREAT | O_WRONLY | O_APPEND);

    if( fd < 0 || fw < 0) {
        perror("Unavailable file");
        return 1;
    }

    // Place input file pointer at end of file
    lseek(fd, 0, 2);

    for(int i=1; i<=size; i++) {
        char c[1];
        read(fd, c, 1);
        write(fw, c, 1);
        lseek(fd, -2, 1);
        
        write(1, "\rtest", 5);
    }

    return 0;
}