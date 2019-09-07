#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include "helper.h"

extern char pwd[100];

void print_pwd()
{
    printf("%s\n", pwd);
}