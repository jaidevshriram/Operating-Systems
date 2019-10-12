#include<stdio.h>
#include<pthread.h>
#include<semaphore.h>
#include<stdlib.h>
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
    int status;
} Rider;

sem_t cab_sem;
sem_t server_sem;

int number_of_cabs, number_of_riders, number_of_servers, riders_left;

Cab cabs[SIZE];
Rider riders[SIZE];

pthread_t rider_tid[SIZE];
pthread_t server_tid[SIZE];

int payment_rider[SIZE];
pthread_cond_t payment_cond_rider[SIZE];
pthread_mutex_t payment_mutex_rider[SIZE];

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
    printf("\033[1;31mPerson %d has left the cab\033[0m\n", index);
}

void rider_in_cab(int index, int cab)
{
    printf("\033[1;33mPerson %d is in cab %d\033[0m\n", index, cab);
    printf("DEBUG: cab type:%s actual:%s\n", riders[index].cab_type == 0 ? "Premier" : "Pool", cabs[cab].state >= 2 ? "Pool" : "Premier");
}

void ride_cab(int index, int cab_no)
{
    rider_in_cab(index, cab_no);
    sleep(riders[index].ride_time);
    if(riders[index].cab_type == 0)
    {
        reset_cab(&cabs[cab_no]);
        sem_post(&cab_sem);
    }
    else
    {
        pthread_mutex_lock(&cabs[cab_no].mutex);
        cabs[cab_no].count--;
        if(cabs[cab_no].count == 0)
        {
            cabs[cab_no].state = 0;
            sem_post(&cab_sem);
        }
        pthread_mutex_unlock(&cabs[cab_no].mutex);
    }
}

void wait_premier(int index)
{
    /* Calculate relative interval as current time plus maxwait time */

    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
    {
        return NULL;
    }

    ts.tv_sec += riders[index].max_wait_time;
    int err = sem_timedwait(&cab_sem, &ts);

    if (err == -1)
    {
        printf("\033[1;31mPerson %d hates Ober. Waiting time exceeded max wait time of %d seconds.\033[0m\n", index, riders[index].max_wait_time);
        return;
    }

    int cab_number = -1;
    while(cab_number == -1) 
    {
        for(int i=0; i<number_of_cabs; i++)
            if(cabs[i].state == 0)
            {
                pthread_mutex_lock(&cabs[i].mutex);
                cabs[i].state = 1;
                cabs[i].count = 1;
                pthread_mutex_unlock(&cabs[i].mutex);
                cab_number = i;
                riders[index].status = 1;
                break;
            }
    }

    ride_cab(index, cab_number);
}

void wait_pool(int index)
{
    int cab_number = -1;

    for(int i=0; i<number_of_cabs; i++)
        if(cabs[i].state == 2)
        {
            pthread_mutex_lock(&cabs[i].mutex);
            cabs[i].state = 3;
            cabs[i].count = 2;
            pthread_mutex_unlock(&cabs[i].mutex);
            cab_number = i;
            break;
        }

    if(cab_number == -1)
    {
        /* Calculate relative interval as current time plus maxwait time */

        struct timespec ts;
        if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
        {
            return NULL;
        }

        ts.tv_sec += riders[index].max_wait_time;
        int err = sem_timedwait(&cab_sem, &ts);

        if (err == -1)
        {
            printf("\033[1;31mPerson %d hates Ober. Waiting time exceeded max wait time of %d seconds.\033[0m\n", index, riders[index].max_wait_time);
            return;
        } 

        while(cab_number == -1)
        {
            for(int i=0; i<number_of_cabs; i++)
                if(cabs[i].state == 0)
                {
                    pthread_mutex_lock(&cabs[i].mutex);
                    cabs[i].state = 2;
                    cabs[i].count++;
                    pthread_mutex_unlock(&cabs[i].mutex);
                    cab_number = i;
                    riders[index].status = 1;
                    break;
                }
                else if(cabs[i].state == 2)
                {
                    pthread_mutex_lock(&cabs[i].mutex);
                    cabs[i].state = 3;
                    cabs[i].count++;
                    pthread_mutex_unlock(&cabs[i].mutex);
                    cab_number = i;
                    riders[index].status = 1;
                    break;
                }
        }
    
    }
    else
    {
        riders[index].status = 1;
    }
    

    ride_cab(index, cab_number);
}

void make_payment(int index)
{
    sem_wait(&server_sem);
    int payment_not_done = 1;
    while(payment_not_done)
    {
        for(int i=0; i<number_of_servers; i++)
        {
            if(payment_rider[i]==-1)
            {
                pthread_mutex_lock(&payment_mutex_rider[i]);
                payment_rider[i] = index;
                pthread_mutex_unlock(&payment_mutex_rider[i]);
                pthread_cond_signal(&payment_cond_rider[i]);
                payment_not_done = 0;
                break;
            }
        }
    }
    sem_post(&server_sem);
}

void *rider(void *ind)
{
    int index = (int) ind;
    riders[index].cab_type = rand()%2;
    riders[index].max_wait_time = 3 + rand()%5;
    riders[index].ride_time = 5 + rand()%3;
    riders[index].status = 0;

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

    if(riders[index].status == 1)
    {
        rider_leave(index);
        make_payment(index);
    }

    riders_left-=1;
}

void *payment_server(void *ind)
{
    int index = (int) ind;
    while(riders_left!=0)
    {
        while(payment_rider[index]==-1)
            pthread_cond_wait(&payment_cond_rider[index], &payment_mutex_rider[index]);
        if(riders_left!=0)
        {
            sleep(2);
            printf("Payment Server %d recieved payment from rider %d\n", index, payment_rider[index]);
        }
        payment_rider[index]=-1;
    }
}

void *doomsday(void *args)
{
    while(riders_left!=0);

    for(int i=0; i<number_of_servers; i++)
        pthread_cond_signal(&payment_cond_rider[i]);
    printf("Simulation Ends. Hallelujah\n");
    exit(0);
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

    pthread_t tid;
    pthread_create(&tid, NULL, doomsday, NULL);

    pthread_exit(NULL);
}

void init_semaphore_cab()
{
    sem_init(&cab_sem, 0, number_of_cabs);
    sem_init(&server_sem, 0, number_of_servers);
    
    for(int i=0; i<number_of_cabs; i++)
        pthread_mutex_init(&(cabs[i].mutex), NULL);

    for(int i=0; i<number_of_servers; i++)
    {
        payment_rider[i] = -1;
        pthread_mutex_init(&payment_mutex_rider[i], NULL);
        pthread_cond_init(&payment_cond_rider[i], NULL);
    }
}

int main()
{
    srand(time(NULL));
    scanf("%d %d %d", &number_of_cabs, &number_of_riders, &number_of_servers);

    riders_left = number_of_riders;

    init_semaphore_cab();
    start_day();

    return 0;
}