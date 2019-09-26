#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include "helper.h"

extern char pwd[100];

int print_pwd()
{
    printf("%s\n", pwd);
    return 0;
}