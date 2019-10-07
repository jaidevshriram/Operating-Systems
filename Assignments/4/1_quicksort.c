#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<unistd.h>
#include<wait.h>
#include<pthread.h>
#include<time.h> 

struct args {
    int l, r, *arr;
};

int n;

int getrandom(int range)
{
    srand(time(0));
    int random = rand() % (range);
    return random;    
}

int *sharemem(size_t size){
    key_t mem_key = IPC_PRIVATE;
    int shm_id = shmget(mem_key, size, IPC_CREAT | 0666);

    if (shm_id < 0) {
        printf("shmget error\n");
        exit(1);
    }

    return (int*)shmat(shm_id, NULL, 0);
}

int partition(int *arr, int left, int right)
{
    int random = left + getrandom(right-left);

    int temp = arr[random];
    arr[random] = arr[right];
    arr[right] = temp;

    int pivot = arr[right];
    int i =  left-1;

    for(int j=left; j<right; j++)
    {
        if(arr[j]<=pivot)
        {
            i++;
            int temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }

    temp = arr[i+1];
    arr[i+1] = arr[right];
    arr[right] = temp;

    return i+1;
}

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

void start_process_quicksort(int a[], int n)
{
   int *arr = sharemem(n*4);
   for(int i=0; i<n; i++)
        arr[i] = a[i];
    process_quicksort(arr, 0, n-1);
    // for(int i=0; i<n i++)
    //     printf("%d ", arr[i]);
}

void start_threaded_quicksort(int a[], int n)
{
    int *arr = sharemem(10*4);
    for(int i=0; i<n; i++)
        arr[i] = a[i];

    pthread_t thread_id;
    struct args arguments = { 0, n-1, arr };
    pthread_create(&thread_id, NULL, threaded_quicksort, &arguments);
    pthread_join(thread_id, NULL);

    // for(int i=0; i<n; i++)
    //     printf("%d ", arr[i]);
}

void main()
{
    scanf("%d", &n);

    int arr[1000]; 

    for(int i=0; i<n; i++)
        arr[i] = 10-i;

    clock_t time_normal;
    time_normal = clock();
    normal_quicksort(arr, 0, 9);
    time_normal = clock() - time_normal;

    clock_t time_process;
    time_process = clock();
    start_process_quicksort(arr, 10);
    time_process = clock() - time_process;

    clock_t time_thread;
    time_thread = clock();
    start_threaded_quicksort(arr, 10);
    time_thread = clock() - time_process;

    printf("%f %f %f\n", ((double)time_normal)/CLOCKS_PER_SEC, ((double)time_process)/CLOCKS_PER_SEC, ((double)time_thread)/CLOCKS_PER_SEC);
    if(time_normal < time_process && time_normal < time_thread)
        printf("Normal execution took lesser time than process and thread\n");
}