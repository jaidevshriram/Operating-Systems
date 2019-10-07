#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<sys/ipc.h>
#include<sys/shm.h>

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

void normal_quicksort(int *arr, int low, int high)
{
    if(low>=high)
        return;
    
    int mid = partition(arr, low, high);
    normal_quicksort(arr, low, mid-1);
    normal_quicksort(arr, mid+1, high);
}

void main()
{
    int *arr = sharemem(10*4);
    
    for(int i=0; i<10; i++)
        arr[i] = 10-i;

    normal_quicksort(arr, 0, 9);

    for(int i=0; i<10; i++)
        printf("%d ", arr[i]);
}