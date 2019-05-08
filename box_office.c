#include "types.h"
#include "sope.h"
#include "types.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "sope.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include "box_office.h"

void* box_office(void* arg){
    //LOOP TO SOLVE REQUESTS
    struct thread_arg ta = *(struct thread_arg*)arg;
    pthread_mutex_t mutex = ta.mutex;
    sem_t *sem = ta.sem;

    
    while(1){
        //WAIT
        sem_wait(sem);
        
        //LOCK
        pthread_mutex_lock(&mutex);
        
        //DO STUFF

        //UNLOCK
        pthread_mutex_unlock(&mutex);
        
        //SIGNAL
        sem_post(sem);

    }
}