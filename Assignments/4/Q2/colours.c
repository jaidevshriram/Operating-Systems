#include<stdio.h>

#define RED "[1;31m"
#define GREEN "[1;32m"
#define BLUE "[1;34m"
#define YELLOW "[01;33m"
#define CYAN "[1;36m"
#define RESET "[0m"

void red () {
  printf("\033[1;31m");
}

void yellow() {
  printf("\033[1;33m");
}

void green() {
    printf("\033[1;32m");
}

void cyan() {
    printf("\033[1;36m");
}

void blue() {
    printf("\033[1;34m");
}

void reset () {
  printf("\033[0m");
}