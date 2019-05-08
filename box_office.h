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

struct thread_arg{
    sem_t *sem;
    pthread_mutex_t mutex;
};

void* box_office(void* arg);

int log_in();
int get_operation();
int create_account();
int check_balance();
int transfer();
