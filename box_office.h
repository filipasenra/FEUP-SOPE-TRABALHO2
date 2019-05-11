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

typedef struct box_office{
    pthread_mutex_t *q_mutex;
    tlv_reply_t queue[QUEUE_MAX];
    int *first;
    int *last;
    dataBase_t *db;
} __attribute__((packed)) box_office_t;

void* box_office(void* arg);

int log_in();
int get_operation();
int create_account();
int check_balance();
int transfer();
void shutdown();