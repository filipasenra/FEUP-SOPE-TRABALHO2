#pragma once

#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "creatAccount.h"
#include "dataBase.h"
#include "sope.h"
#include "types.h"
#include "communication.h"

#include "queue.h"

typedef struct box_office {
    pthread_mutex_t *q_mutex;
    tlv_request_t* queue[QUEUE_MAX];
    int first;
    int last;
    dataBase_t *db;
} __attribute__((packed)) box_office_t;

extern pthread_mutex_t q_mutex;
extern pthread_mutex_t db_mutex;
extern queue_t queue;
extern dataBase_t db;
extern sem_t n_req;
extern sem_t b_off;

void * box_office(void *arg);
int get_operation();
int create_account(bank_account_t *account, char password[], int accound_id, int balance, tlv_reply_t *reply);
int check_balance(bank_account_t *bank_account, tlv_reply_t *user_reply);
int transfer(tlv_request_t user_request, tlv_reply_t *user_reply);
void shutdown(tlv_reply_t *user_reply);
int log_in(dataBase_t *db, uint32_t account_id, char password[MAX_PASSWORD_LEN + 1]);