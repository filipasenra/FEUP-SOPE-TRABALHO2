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

typedef struct box_office {
    pthread_mutex_t *q_mutex;
    tlv_request_t (* queue)[QUEUE_MAX];
    int *first;
    int *last;
    dataBase_t *db;
} __attribute__((packed)) box_office_t;

void * box_office(void *arg);

int get_operation();
int check_balance(bank_account_t bank_account, tlv_reply_t *user_reply);
int transfer();
void shutdown();
int log_in(dataBase_t *db, uint32_t account_id,
           char password[MAX_PASSWORD_LEN + 1]);