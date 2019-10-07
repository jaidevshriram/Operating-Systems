#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>

typedef struct s{
    int id;
} s;

int var;

pthread_mutex_t mutex;

void* t(void* inp){
    sleep(2);
    s* inputs = (s*) inp;

    pthread_mutex_lock(&mutex);
        var = 2*inputs->id;

        for(int i=0;i<var;i++){
            printf("Thread: %d var: %d\n", inputs->id, var);
        }
        
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main(){
    pthread_t tid1;
    pthread_t tid2;

    s* thread1_input = (s*)malloc(sizeof(s));
    s* thread2_input = (s*)malloc(sizeof(s));

    thread1_input->id = 1;
    thread2_input->id = 2;

    pthread_mutex_init(&mutex, NULL);

    pthread_create(&tid1, NULL, t, (void*)(thread1_input));
    pthread_create(&tid2, NULL, t, (void*)(thread2_input));

    //join waits for the thread with id=tid to finish execution before proceeding
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    pthread_mutex_destroy(&mutex);
    return 0;
}