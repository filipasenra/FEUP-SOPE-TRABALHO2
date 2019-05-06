#include "type.h"
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

int box_office(sem_t sem, pthread_mutex_t mut){
    //LOOP TO SOLVE REQUESTS
    while(1){
        //WAIT
        
        //LOCK
        
        //DO STUFF

        //UNLOCK
        
        //SIGNAL

    }
}