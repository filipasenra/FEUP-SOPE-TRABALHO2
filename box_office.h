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
extern pthread_t *thread_array;

/**
 * @brief Processes the next request on the queue 
 * 
 * @param arg a pointer to the log file descriptor
 * 
*/
void *box_office(void *arg);

/**
 * @brief Creates a bank account
 * 
 * @param account Pointer to the created bank account
 * @param password C-String of the password associated with the account to be created
 * @param accound_id Account id of the account to be created
 * @param balance Inicial balance of the account
 * @param reply Pointer to the struct that will serve as the reply to the user
 * 
 * @return returns zero upon sucess, non-zero otherwise
 *  
*/
int create_account(bank_account_t *account, char password[], int accound_id, int balance, tlv_reply_t *user_reply, int fd);

/**
 * @brief Checks the balance of an account
 * 
 * @param bank_account Pointer to account to be checked the balance
 * @param user_reply Pointer to the struct that will serve as the reply to the user
 * 
 * @return returns zero upon sucess, non-zero otherwise
*/
int check_balance(bank_account_t *bank_account, tlv_reply_t *user_reply);

/**
 * @brief Handles a transfer
 * 
 * @param user_request Request given by the user
 * @param user_reply Pointer to the struct that will serve as the reply to the user
 * 
 *  @return returns zero upon sucess, non-zero otherwise
 */
int transfer(tlv_request_t user_request, tlv_reply_t *user_reply, int fd, int delay);

/**
 * @brief Handles a shutdown
 * 
 * @param user_reply Pointer to the struct that will serve as the reply to the user
 * 
*/
void shutdown(tlv_reply_t *user_reply);

void lock_account(int index, int fd, tlv_request_t user_request);
void unlock_account(int index, int fd, tlv_request_t user_request);