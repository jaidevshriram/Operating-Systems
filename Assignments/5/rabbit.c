#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
    for(int i=0; i<3; i++)
    {
        int pid = fork();

        if(pid == 0)
        {

            int count = 900000000;
            float test = 0;
            
            for(volatile int i = 0; i<count; i++){
                test += 5*0.01;
            }

            exit();
        }
    }
}