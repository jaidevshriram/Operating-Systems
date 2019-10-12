# Automated Biryani Mess

This solution can be thought of as the producer-consumer problem on two layers. One relationship is between the robot and table, and the other between table, and person. Read more for explanation.

## Input Format

M = NUmber of Robot Chef
N = NUmber of Serving Table
K = Number of students

## Robot - Table Relationship

### Robot Half

The robot produces food and goes into wait state until it is signalled to wake up and make more food. 

<pre>
<code>

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
        robot_portions[index] += vessel_count*vessel_capacity[index];
        pthread_mutex_unlock(&robot_table_mutex[index]);
        robot_start(index);
        printf("\033[1;36mRobot %d finished preparations of %d vessels of capacity %d\033[0m\n", index, vessel_count, vessel_capacity[index]);
    }

    printf("\033[1;31m%d robot shutting down\033[0m\n", index);

    return NULL;
}

</code>
</pre>

As seen above, we use a mutex lock on the robot's container/food. This is because table's and the robot themselves will be accessing the container/vessel.

The while loop is exited only once all students are done, in which case we switch off the robot and exit. 

What happens after biryani is ready?

<pre>
<code>
void biryani_ready(int index)
{
    printf("Robot %d is waiting for all vessels to be emptied\n", index);
    while(robot_portions[index]!=0 && students_left!=0)
        pthread_cond_wait(&robot_cond[index], &robot_table_mutex[index]);

    if(robot_portions[index]==0 && students_left!=0)
        printf("All the vessels prepared by robot %d are emptied. Resuming cooking now\n", index);
}
</code>
</pre>

This function puts the robot into sleep state through the use of pthread conditional variable. Whenever a table gets the last portion of food in a vessel, it will signal on the same conditional variable, and robot resumes cooking.

### Table Half

The tables have the burden of simultaneously taking food from any available chef, as this is the point of concurrency. This is done using continuous polling using `pthread_mutex_lock()`. If lock cannot be obtained, it will try another robot chef. This way, multiple tables can get containers immediately, at the same time.

Portion of the table function:

<pre>
<code>
`
    void *table(void *ind)
    {
        int index = (int) ind;
        while(1 && students_left!=0)
        {
            .
            .
            .
                Ready to serve food code
            .
            .
            .

            if(table_portions[index]==0)
            {
                printf("\033[1;34mServing Container of table %d is empty, waiting for refill\n", index);
            }
            
            for(int i=0; i<number_of_robot_chef && table_portions[index]==0; i++)
            {
                // printf("robot %d lock attempt\n", i);

                int err = pthread_mutex_trylock(&robot_table_mutex[i]);
        
                //If lock was not obtained
                if(err != 0)
                    continue;
        
                if(robot_portions[i]==0)
                    pthread_cond_signal(&robot_cond[i]);
                else
                {
                    // printf("%d lock\n", index);
                    pthread_mutex_lock(&table_person_mutex[index]);
        
                    if(robot_portions[i]==0)
                        pthread_cond_signal(&robot_cond[i]);
                    else
                    {
                        printf("\033[1;34m%d obtained vessel from robot %d of capacity %d\033[0m\n", index, i, vessel_capacity[i]);
                        table_portions[index] += vessel_capacity[i];
                        robot_portions[i] -= vessel_capacity[i];
                        
                        if(robot_portions[i]==0)
                            pthread_cond_signal(&robot_cond[i]); 
                    }          
                    
                    pthread_mutex_unlock(&table_person_mutex[index]);
                    // printf("%d unlock\n", index);
                }

                pthread_mutex_unlock(&robot_table_mutex[i]);
            }
        }

        printf("\033[1;31m%d table closing\033[0m\n", index);
    }

<code>
</pre>

The code is pretty straightforward and illustrates obtaining the lock from the robot, and then obtaining lock for the table itself to ensure that no person has access to the table at the moment, since table_portion will be updated.

With these two locks, the robot vessel is emptied and if it was the last vessel:

<pre>
<code>
                    if(robot_portions[i]==0)
                        pthread_cond_signal(&robot_cond[i]); 
</code>
</pre>

this portion of code, sends signal to the robot, telling it to make more food.

## Table-Person Relationship

### Table Half

Once the table has food ready, it acts like a producer and is ready to create slots that people can use. This is done in this section:

<pre>
<code>
        while(table_portions[index]!=0 && students_left!=0)
        {
            // printf("table %d lock attempt\n", index);
            pthread_mutex_lock(&table_person_mutex[index]);
            // printf("table %d lock obtained\n", index);
            table_slots[index] = min(1 + rand()%10, table_portions[index]);
            ready_to_serve_table(table_slots[index], index);
            pthread_mutex_unlock(&table_person_mutex[index]);

            sleep(2);
            printf("Resetting table %d\n", index);
        }
</code>
</pre>

This code will generate number of slots available on the table, hence the use of lock to prevent person from accessing it at the same time.
Once the slots are ready, `ready_to_serve_table()` is called. 

<pre>
<code>
void ready_to_serve_table(int slots, int index)
{
    yellow();
    printf("\033[1;33m%d table is ready to serve %d slots\033[0m\n", index, table_slots[index]);

    while(table_slots[index]!=0 && students_left!=0)
        pthread_cond_wait(&table_cond[index], &table_person_mutex[index]);

    if(students_left!=0)
        printf("\033[1;31m%d table has run out of slots\033[0m\n", index);
}
</code>
</pre>

This function will put the table to sleep again, waking up only when no students are left or slots are over for that table. This uses pthread conditional variable once again, and signal is sent by the person.

### Person Half

Nearing the end of the solution :)

<pre>
<code>

void *person(void *ind)
{
    int index = (int) ind;
    int table;
    wait_for_slot(index, &table);
    student_in_slot(index, table);
}

</code>
</pre>

`wait_for_slot()` will poll for an open slot on all tables, this is implemented using mutex_lock since we could have race conditions when two people attempt to obtain the same slot. Once again, mutex_try_lock has been used. 

<pre>
<code>

void wait_for_slot(int index, int *table)
{
    person_enter(index);
    
    int slot_not_found = 1;

    while(slot_not_found)
    {   
        for(int i=0; i<number_of_serving_tables && slot_not_found; i++)
        {
            if(table_slots[i]==0)
            {
                pthread_cond_signal(&table_cond[i]);
                continue;
            }

            int err = pthread_mutex_trylock(&table_person_mutex[i]);
            // printf("%d lock\n", i);
            if(err!=0)
                continue;

            if(table_slots[i]==0)
                pthread_cond_signal(&table_cond[i]);
            else
            {
                table_portions[i] -= 1;
                table_slots[i]--;
                *table = i;
                students_left-=1;
                printf("\033[1;32m%d is in a slot on table %d\033[0m\n", index, *table);
                slot_not_found = 0;    
            }

            pthread_mutex_unlock(&table_person_mutex[i]);
            // printf("%d unlock\n", i);

            if(table_slots[i]==0)
                pthread_cond_signal(&table_cond[i]);
        }
    }

    while(table_slots[*table]!=0 && students_left!=0);
    printf("\033[1;32m%d is in a slot on table %d, has been served.\033[0m\n", index, *table);
}

</code>
</pre>

A lot of this code is just sending signals to wake a table up if all slots have been taken up. Once trylock is succesfull, the slot is obtained and the person waits until all slots are filled/students are not waiting. Then, it returns and `student_in_slot()` executes with the student eating for some time, implemented using `sleep()`.

## Doomsday Protocol

This is used to end the entire program. Once students_left becomes zero, it wakes up all sleeping tables, robots and they go to completion because no students are left. 

## Initialization and Main Function

Nothing special here, standard mutex, conditional variable , and thread initialization. In order to mimic delay between people coming in, a delay is used before creating each person's thread.