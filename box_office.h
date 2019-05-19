#pragma once

#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "communication.h"
#include "creatAccount.h"
#include "dataBase.h"
#include "queue.h"
#include "sope.h"
#include "types.h"

extern pthread_mutex_t q_mutex;
extern pthread_mutex_t db_mutex[MAX_BANK_ACCOUNTS];
extern queue_t queue;
extern dataBase_t db;
extern sem_t n_req;
extern sem_t b_off;
extern int number_threads;

void *box_office(void *arg);

int create_account(bank_account_t *account, char password[], int accound_id, int balance, tlv_reply_t *reply);
int check_balance(bank_account_t *bank_account, tlv_reply_t *user_reply);
int transfer(tlv_request_t user_request, tlv_reply_t *user_reply, int fd, int delay);
void shutdown(tlv_reply_t *user_reply);