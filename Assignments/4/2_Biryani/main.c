#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include "colours.c"

#define SIZE 1000
#define MIN(A,B) A<B ? A : B

int number_of_robot_chef;
int vessel_capacity[SIZE];
int number_of_serving_tables;
int number_of_students;
int students_left;

pthread_mutex_t robot_table_mutex[SIZE];
pthread_mutex_t table_person_mutex[SIZE];
pthread_cond_t robot_cond[SIZE];
pthread_cond_t table_cond[SIZE];
pthread_t robot_t[SIZE];
pthread_t table_t[SIZE];
pthread_t person_t[SIZE];

int robot_portions[SIZE];
int table_portions[SIZE];
int table_slots[SIZE];

void person_enter(int i)
{
    green();
    printf("Person %d has entered Kadamba!\n", i);
    reset();
}

void person_leave(int i)
{
    red();
    printf("Person %d has left the mess\n", i);
    reset();
}

void robot_start(int i)
{
    cyan();
    printf("Robot %d has started preparations....\n", i);
    reset();

    int s = 2 + rand()%4;
    sleep(s);

    cyan();
    printf("Robot %d finished preparations\n", i);
    reset();
}

void biryani_ready(int index)
{
    while(robot_portions[index]!=0)
        pthread_cond_wait(&robot_cond[index], &robot_table_mutex[index]);
}

void *robot(void *ind)
{
    int index = ((int) ind);
    // printf("%d \n", index);
    vessel_capacity[index] = 5 + rand()%10;
    while(1 && students_left!=0)
    {
        pthread_mutex_lock(&robot_table_mutex[index]);
        biryani_ready(index);
        int vessel_count = 1 + rand()%9;
        robot_start(index);
        robot_portions[index] += vessel_count*vessel_capacity[index];
        pthread_mutex_unlock(&robot_table_mutex[index]);
    }

    red();
    printf("%d robot shutting down\n", index);
    reset();

    return NULL;
}

void ready_to_serve_table(int slots, int index)
{
    yellow();
    printf("%d table is ready to serve %d slots\n", index, slots);
    reset();

    while(table_slots[index]!=0)
        pthread_cond_wait(&table_cond[index], &table_person_mutex[index]);

    printf("%d table has run out of slots\n", index);
}

void *table(void *ind)
{
    int index = (int) ind;
    while(1 && students_left!=0)
    {
        sleep(1);

        while(table_portions[index]!=0)
        {
            pthread_mutex_lock(&table_person_mutex[index]);
            printf("table s%d lock obtained\n", index);
            table_slots[index] = (MIN(1 + rand()%10, table_portions[index]));
            pthread_mutex_unlock(&table_person_mutex[index]);

            ready_to_serve_table(table_slots[index], index);
            printf("Reset table %d\n", index);
        }
        
        for(int i=0; i<number_of_robot_chef && table_portions[index]==0; i++)
        {
            int err = pthread_mutex_trylock(&robot_table_mutex[i]);
    
            //If lock was not obtained
            if(err != 0)
                continue;
    
            if(robot_portions[i]==0)
                pthread_cond_signal(&robot_cond[i]);
            else
            {
                pthread_mutex_lock(&table_person_mutex[index]);
     
                if(robot_portions[i]==0)
                    pthread_cond_signal(&robot_cond[i]);
                else
                {
                    blue();
                    printf("%d obtained vessel from robot %d of capacity %d\n", index, i, vessel_capacity[i]);
                    reset();
                    table_portions[index] += vessel_capacity[i];
                    robot_portions[i] -= vessel_capacity[i];
                    
                    if(robot_portions[i]==0)
                        pthread_cond_signal(&robot_cond[i]); 
                }          
                
                pthread_mutex_unlock(&table_person_mutex[index]);
            }

            pthread_mutex_unlock(&robot_table_mutex[i]);
        }
    }

    red();
    printf("%d table closing\n", index);
    reset();
}

void wait_for_slot(int index, int *table)
{
    person_enter(index);
    
    int slot_not_found = 1;

    while(slot_not_found)
    {   
        for(int i=0; i<number_of_serving_tables; i++)
        {
            if(table_slots[i]==0)
                pthread_cond_signal(&table_cond[i]);
            else
            {
                int err = pthread_mutex_trylock(&table_person_mutex[i]);
                if(err!=0)
                    continue;
                else
                {
                    table_slots[i] --;
                    *table = i;
                    slot_not_found = 0;    
                }
                pthread_mutex_unlock(&table_person_mutex[i]);
            }
        }
    }
}

void student_in_slot(int index, int table)
{
    table_portions[index] -= 1;
    green();
    printf("%d is in a slot on table %d, eating happily :)))\n", index, table);
    reset();

    sleep(0.5);
    red();
    printf("%d is done eating. Has left the mess :(\n", index);
    reset();
}

void *person(void *ind)
{
    int index = (int) ind;
    int table;
    wait_for_slot(index, &table);
    student_in_slot(index, table);
    students_left-=1;
}

void mutex_cond_initialise()
{
    for(int i=0; i<SIZE; i++)
    {
        pthread_mutex_init(&robot_table_mutex[i], NULL);
        pthread_mutex_init(&table_person_mutex[i], NULL);
        pthread_cond_init(&robot_cond[i], NULL);
        pthread_cond_init(&table_cond[i], NULL);
    }
}

void *doomsday(void *args)
{
    int end = 1;
    while(end)
    {
        if(students_left == 0)
        {
            end = 0;
            for(int i=0; i<number_of_serving_tables; i++)
                pthread_cond_signal(&table_cond[i]);
            for(int i=0; i<number_of_robot_chef; i++)
                pthread_cond_signal(&table_cond[i]);
        }
    }
    sleep(10);
    printf("Cleanup protocol complete\n");
}

void init_threads()
{
    //Create threads for all persons
    for(int i=0; i<number_of_students; i++)
        pthread_create(&person_t[i], NULL, person, (void *)i);

    sleep(10);

    //Create threads for all robots
    for(int i=0; i<number_of_robot_chef; i++)
    {
        // printf("%d is sent for creation\n", i);
        while(pthread_create(&robot_t[i], NULL, robot, (void *)i)!=0)
        {
            printf("%d thread failed\n", i);
        };
    }

    sleep(2);

    //Create threads for all tables
    for(int i=0; i<number_of_serving_tables; i++)
        pthread_create(&table_t[i], NULL, table, (void *)i);
    
    pthread_t tid_end;
    pthread_create(&tid_end, NULL, doomsday, NULL);

    pthread_exit(NULL);
}

int main()
{
    //Initialize seed for random variables
    srand(time(NULL));

    //Taking input for P, V, M, K
    scanf("%d %d %d", &number_of_robot_chef,&number_of_serving_tables, &number_of_students);

    students_left = number_of_students;

    mutex_cond_initialise();
    init_threads();
}