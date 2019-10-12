# Ober Cab Services

Thanks to semaphores, this program is able to efficiently wake up riders when cabs become available. There are threads for each payment server, and rider. The cabs are implemented using an array of structs as they are indefintely available.

# Cabs

The function `ride_cab()` starts the ride for person 'index' and goes to sleep to mimix ride time. Once ride is done, it obtains mutex lock if ride was pool, to change status to waitcab/onridepoolOne. 

<pre>
<code>
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
</code>
</pre>

There is a semaphore `cab_sem` which is initialized to the number of cabs. Using sem_wait and sem_post, riders can know if a cab is available instead of infintely polling. We use mutex on each cab to ensure correctness in case of pool rides.

# Riders

The riders have randomized values for type of cab (0=premier, 1=pool), maxwaittime, and rideTime. 

<pre>
<code>

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

</code>
</pre>

This is the code that every rider thread will execute. WHen the cab type is premier/pool, it calls the respective wait function.

### Wait_Premier()

This function waits for premier cab to become available.

<pre>
<code>

void wait_premier(int index)
{
    /* Calculate relative interval as current time plus maxwait time */

    .
    .
    .
        Sem_timed_wait()
    .
    .
    .

    int cab_number = -1;
    while(cab_number == -1) 
    {
        for(int i=0; i < number_of_cabs; i++)
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


</code>
</pre>

`Sem_timed_wait()` ensures you get cab within the maxWaitTime if possible. Once the cab is obtained, we assign a particular cab number to the person, and start the ride using call to `ride_cab()`.


### Wait_Pool()

This function will wait for pool cab to become available, giving preference for the pool ride that has one person.

It runs through the array of cabs to see if any of this type exists, if not, it gets into the cab which is in waiting state.

<pre>
<code>
void wait_pool(int index)
{
    int cab_number = -1;

    for(int i=0; i< number_of_cabs; i++)
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
        .
        .
        .
            Sem_timed_wait()
        .
        .
        .

        while(cab_number == -1)
        {
            for(int i=0; i < number_of_cabs; i++)
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
</code>
</pre>

## Payment_Sever

Each server is a thread to mimic real life usage. 

<pre>
<code>


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

</code>
</pre>

This code makes the payment server go to sleep when noone is making a payment request. It also resets availability information using array payment_rider[index]. That array contains the information on what rider is paying to what server.

Riders make payment using this function:

<pre>
<code>

void make_payment(int index)
{
    sem_wait(&server_sem);
    int payment_not_done = 1;
    while(payment_not_done)
    {
        for(int i=0; i< number_of_servers; i++)
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
</code>
</pre>

The rider knows a server is available using another semaphore `server_sem`. It uses a lock to assign itself that server, and then the server is awoken using `pthread_cond_signal` and payment goes through.

## Doomsday Protocol

This thread ends all threads in process once all riders have finished their ride/wait.

## Initialization and Main

Nothing special here, threads, semaphore, mutex's and conditional variables are initialized.