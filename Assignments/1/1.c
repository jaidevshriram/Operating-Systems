#include <stdio.h>
#include <sys/stat.h>

void sys_print(char a*)
{
    printf("%s\n", a);
}

int main(int argc, char *argv[])
{
    if( argc <=2 ) {
       sys_print("No File Specified\n");
        return 1;
    }

    struct stat statbuf;

    stat(argv[1], &statbuf);

    printf("%d is size of file\n", statbuf.st_size);

    return 0;
}