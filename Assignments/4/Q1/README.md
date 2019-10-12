# 1. Concurrent QuickSort

This program implements quicksort using three methods:

1. Normal
2. Multi - Process
3. Multi- Threaded

## Normal Quicksort

<pre>
<code>
void normal_quicksort(int *arr, int left, int right)
{
    if(left>=right)
        return;
    
    if(right-left+1<=5)
    {
        for(int i=left+1; i<=right; i++)
        {
            int key = arr[i];
            int j = i-1;

            while(j>=0 && arr[j]>key)
            {
                arr[j+1] = arr[j];
                j--;
            }
            arr[j+1] = key;
        }
        return;
    }
    
    
    int mid = partition(arr, left, right);
    normal_quicksort(arr, left, mid-1);
    normal_quicksort(arr, mid+1, right);
}
</code>
</pre>

This is the regular quicksort implemented with insertion sort for subarrays of size <= 5. The partition function will decide the pivot position.

## Multi - Process Quicksort

<pre>
<code>

void process_quicksort(int *arr, int left, int right)
{
    if(left>=right)
        return;

    if(right-left+1<=5)
    {
        for(int i=left+1; i<=right; i++)
        {
            int key = arr[i];
            int j = i-1;

            while(j>=0 && arr[j]>key)
            {
                arr[j+1] = arr[j];
                j--;
            }
            arr[j+1] = key;
        }
        return;
    }
    
    int mid = partition(arr, left, right);

    int pid_1 = fork();

    if(pid_1 == 0)
    {
        normal_quicksort(arr, left, mid-1);
        _exit(0);
    }
    else if(pid_1 > 0)
    {
        int pid_2 = fork();
        if(pid_2 == 0)
        {
            normal_quicksort(arr, mid+1, right);
            _exit(0);
        }
        else
        {
            int status;
            waitpid(pid_1, &status, 0);
            waitpid(pid_2, &status, 0);
        }
    }
}

</code>
</pre>

Multi-Process Quicksort uses seperate processes for each half of the problem at every step. We ensure that program ends correctly by exiting only once each sub-half of the problem is completed. To prevent the child processes from doing extra things, we use <code>_exit(0)</code> for the child process to kill itself.

The processes should all use the same memory segment, hence array is created using a shared memory segment, defined below:

<pre>
<code>
int *sharemem(size_t size){
    key_t mem_key = IPC_PRIVATE;
    int shm_id = shmget(mem_key, size, IPC_CREAT | 0666);

    if (shm_id < 0) {
        printf("shmget error\n");
        exit(1);
    }

    return (int*)shmat(shm_id, NULL, 0);
}
</code>
<pre>

## Multi-Threaded Quicksort

Multi-Threaded Quicksort uses seperate threads for each half of the problem. Since threads use the same data, there is no concern of memory sharing. The code for this is:

<pre>
<code>

void *threaded_quicksort(void *temp_args)
{
    struct args *arg = (struct args *) temp_args;

    int left = arg->l;
    int right = arg->r;
    int *arr = arg->arr;

    if(left>=right)
        return NULL;
    
    if(right-left+1<=5)
    {
        for(int i=left+1; i<=right; i++)
        {
            int key = arr[i];
            int j = i-1;

            while(j>=0 && arr[j]>key)
            {
                arr[j+1] = arr[j];
                j--;
            }
            arr[j+1] = key;
        }
        return NULL;
    }
    
    int mid = partition(arr, left, right);

    pthread_t thread_id_1;
    struct args arguments_1 = { left, mid-1, arr };
    pthread_create(&thread_id_1, NULL, threaded_quicksort, &arguments_1);

    pthread_t thread_id_2;
    struct args arguments_2 = { mid+1, right, arr};
    pthread_create(&thread_id_2, NULL, threaded_quicksort, &arguments_1);

    pthread_join(thread_id_1, NULL);

    pthread_join(thread_id_2, NULL);

    normal_quicksort(arr, mid+1, right);
}

</code>
</pre>

This is done using the POSIX pthreads library. Each argument gets typecast to void * type, hence we typecast it back to stuct type for our usage. 

For each half of the problem, we create a new thread using `pthread_create()` and wait for both 'child' threads to finish execution using `pthread_join()`. 

## Time Analysis of both

Contrary to the expected output, normal quicksort < process < thread. Despite concurrency, the overhead in creating these processes/threads, slows down the entire program. One sample running time on an array of size 10 was:

0.000022 0.000205 0.001143
Normal   Process  Thread