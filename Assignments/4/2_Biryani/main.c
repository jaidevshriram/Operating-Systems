#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include "colours.c"

#define SIZE 1000 

int number_of_robot_chef;
int vessel_capacity;
int number_of_serving_tables;
int number_of_students;

pthread_mutex_t robot_table_mutex[SIZE];
pthread_mutex_t table_person_mutex[SIZE];
pthread_cond_t robot_cond[SIZE];
pthread_cond_t table_cond[SIZE];
pthread_t robot_t[SIZE];
pthread_t table_t[SIZE];
pthread_t person_t[SIZE];

int robot_portions[SIZE];
int table_portions[SIZE];

void robot((void *)ind)
{
    int index = (int *) ind;
    pthread_mutex_lock(&robot_table_mutex[index]);
    while(robot_portions[index]!=0)
        pthread_cond_wait(&robot_cond[index], &robot_table_mutex[index]);
    int vessel_count = 1 + rand()%9;
    robot_start(index);
    robot_portions[index] += vessel_count*vessel_capacity;
    pthread_mutex_unlock(&robot_table_mutex[index]);
}

void table((void *)ind)
{
    int index = (int *) ind;
    pthread_mutex_lock(&robot_table_mutex[index]);
    while(table_portions[index]!=0)
        pthread_cond_wait(&table_cond[index], &robot_table_mutex[index]);
       
}

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
    printf("Robot %d has started preparations....", i);
    int s = 2 + rand()%4;
    sleep(100*s);
    printf("Finished\n");
    reset();
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

void init_threads()
{
    //Create threads for all robots
    for(int i=0; i<number_of_robot_chef; i++)
        pthread_create(&robot_t[i], NULL, robot, i);

    //Create threads for all tables
    for(int i=0; i<number_of_serving_tables; i++)
        pthread_create(&table_t[i], NULL, table, i);

    //Create threads for all persons
    for(int i=0; i<number_of_students; i++)
        pthread_create(&person_t[i], NULL, person, i)

}

int main()
{
    //Initialize seed for random variables
    srand(time(NULL));

    //Taking input for P, V, M, K
    scanf("%d %d %d %d", &number_of_robot_chef, &vessel_capacity, &number_of_serving_tables, &number_of_students);

    mutex_cond_initialise();
    init_threads();
}
