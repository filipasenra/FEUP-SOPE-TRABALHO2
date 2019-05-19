#pragma once

#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "queue.h"
#include "dataBase.h"

extern pthread_mutex_t q_mutex;
extern pthread_mutex_t db_mutex[MAX_BANK_ACCOUNTS];
extern queue_t queue;
extern dataBase_t db;
extern sem_t n_req;
extern sem_t b_off;
extern int number_threads;

/**
 * @Brief Inicializes the Server
 * 
 * @return Returns zero upon success, non-zero otherwise
*/
int server_init(char *password, int number_threads, pthread_t thread_array[], bank_account_t *acc, int *fd_log, int *fd_srv);

/**
 * @Brief Main Loop of the Server
 * 
*/
void server_main_loop(int fd_log, int fd_srv);

/**
 * @Brief Closes the Server
*/
void closingServer(int fd_log, pthread_t thread_array[number_threads]);

/**
 * @Brief Checks the arguments given by the user of the program
*/
int checkArg(int argc, char *argv[]);

