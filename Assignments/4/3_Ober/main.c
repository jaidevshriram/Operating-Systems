#include<stdio.h>
#include<pthread.h>
#include<semaphore.h>
#include<time.h>

#define SIZE 1000

typedef struct cab_type {
    int state;
    int count;
    pthread_mutex_t mutex;
} Cab;

// 0:: Wait State, 1:: onRidePremier, 2:: onRidePoolFull, 3:: onRidePoolOne

typedef struct rider_type {
    int cab_type;
    int max_wait_time;
    int ride_time;
} Rider;

sem_t cab_sem;

int number_of_cabs, number_of_riders, number_of_servers;

Cab cabs[SIZE];
Rider riders[SIZE];

pthread_t rider_tid[SIZE];
pthread_t server_tid[SIZE];

void reset_cab(Cab *cab)
{
    cab->state = 0;
    cab->count = 0;
}

void rider_enter(int index)
{
    printf("\033[1;32mPerson %d is ready to ride\033[0m\n", index);
}

void rider_leave(int index)
{
    printf("\033[1;31mPerson %d has left the cab\033[0m\n");
}

void rider_in_cab(int index, int cab)
{
    printf("\033[1;33mPerson %d is in cab %d\033[0m\n", index, cab);
}

void wait_premier(int index)
{
    /* Calculate relative interval as current time plus maxwait time */

    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
    {
        /* handle error */
        return -1;
    }

    ts.tv_sec += 10;
    while ((s = sem_timedwait(&full, &ts)) == -1 && errno == EINTR)
                continue;       /* Restart if interrupted by handler */
    /* Check what happened */
    if (s == -1)
    {
        if (errno == ETIMEDOUT)
            printf("sem_timedwait() timed out\n");
        else
            perror("sem_timedwait");
    } else
            printf("sem_timedwait() succeeded\n");
    int cab_number = -1;
    while(cab_number == -1) 
    {
        for(int i=0; i<number_of_cabs; i++)
            if(cabs[i].state == 0)
            {
                cabs[i].state = 1;
                cab_number = i;
                break;
            }
    }

}

void *rider(void *ind)
{
    int index = (int) ind;
    riders[index].cab_type = rand()%2;
    riders[index].max_wait_time = 3 + rand()%5;
    riders[index].ride_time = 5 + rand()%3;

    rider_enter(index);
    
    switch(riders[index].cab_type)
    {
        case 0: wait_premier(index); break;
        case 1: wait_pool(index); break;
        default: {
             printf("Person %d requested non-existent cab\n", index);
             exit(0);
        }
    }

    make_payment(index);
    rider_leave(index);
}

void *payment_server(void *ind)
{

}

void start_day()
{
    for(int i=0; i<number_of_servers; i++)
        pthread_create(&server_tid[i], NULL, payment_server, (void*)i);
    
    sleep(5);

    for(int i=0; i<number_of_riders; i++)
    {
        sleep(rand()%2);
        pthread_create(&rider_tid[i], NULL, rider, (void*)i);
    }
}

void init_semaphore_cab()
{
    sem_init(&cab_sem, 0, number_of_cabs);
    
    for(int i=0; i<number_of_cabs; i++)
        pthread_mutex_init(&(cabs[i].mutex), NULL);
}

int main()
{
    srand(time(NULL));
    scanf("%d %d %d", &number_of_cabs, &number_of_riders, &number_of_servers);

    init_semaphore_cab();
    start_day();

    return 0;
}