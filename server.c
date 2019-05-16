#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "box_office.h"
#include "communication.h"
#include "creatAccount.h"
#include "dataBase.h"
#include "queue.h"
#include "sope.h"
#include "types.h"

sem_t n_req;
sem_t b_off;
pthread_mutex_t q_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;
queue_t queue;
dataBase_t db;

// Server Program
int main(int argc, char* argv[]) {

    // Server <box offices> <password>
    if (argc != 3) {
        printf("./server <box offices> <password>\n");
        return RC_OTHER;
    }

    int number_threads = strtol(argv[1], NULL, 10);
    if (number_threads <= 0 || number_threads > MAX_BANK_OFFICES)
        return RC_OTHER;

    
    // OPENING LOG
    int fd = open(SERVER_LOGFILE, O_WRONLY | O_APPEND | O_CREAT, 0777);

    //Variable that indicates the server is closing
    int closing_server = 0;

    //Initializing the semaphores
    sem_init(&n_req, 0, 0);
    sem_init(&b_off, 0, number_threads);

    // CREATE DATABASE
    if (initializeDataBase(&db)) return RC_OTHER;

    //Creating the threads
    pthread_t thread_array[number_threads];
    for (int i = 0; i < number_threads; i++) {
        pthread_create(&thread_array[i], NULL, box_office, NULL);
    }

    // ADMIN ACC
    bank_account_t account;
    createAccount(&account, argv[2], 0, 0);
    addAccount(account, &db);

    // INCIALIZING QUEUE
    queueInicialize(&queue);

    tlv_request_t request;

    // REQUEST LOOP
    mkfifo(SERVER_FIFO_PATH, 0666);
    while (1) {
        if (get_request(&request)) return RC_OTHER;

        if (request.length) {
            pthread_mutex_lock(&q_mutex);
            logSyncMech(fd, getpid(), SYNC_OP_MUTEX_LOCK, SYNC_ROLE_PRODUCER, request.value.header.account_id);

            sem_wait(&b_off);
            int value = 0;
            sem_getvalue(&b_off, &value);
            logSyncMechSem(fd, getpid(), SYNC_OP_SEM_WAIT, SYNC_ROLE_PRODUCER, request.value.header.account_id, value);

            push(&queue, request);

            sem_post(&n_req);
            sem_getvalue(&n_req, &value);
            logSyncMechSem(fd, getpid(), SYNC_OP_SEM_POST, SYNC_ROLE_PRODUCER, request.value.header.account_id, value);
            

            pthread_mutex_unlock(&q_mutex);
            logSyncMech(fd, getpid(), SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_PRODUCER, request.value.header.account_id);

            request.length = 0;
        }

        if (closing_server) break;
    }

    while (queue.first != queue.last)
        ;

    for (int i = 0; i < number_threads; i++) {
        pthread_kill(thread_array[i], SIGTERM);
    }

    unlink(SERVER_FIFO_PATH);
    freeDataBase(&db);

    close(fd);

    return 0;
}