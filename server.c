#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include "box_office.h"
#include "communication.h"
#include "creatAccount.h"
#include "dataBase.h"
#include "sope.h"
#include "types.h"

pthread_mutex_t q_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;
tlv_request_t queue[QUEUE_MAX];
int first = 0;
int last = 0;
dataBase_t db;

// Server Program
int main(int argc, char* argv[]) {
    // Server <box offices> <password>
    if (argc != 3) {
        printf("./server <box offices> <password>\n");
        return RC_OTHER;
    }
    int closing_server = 0;
    int number_threads = strtol(argv[1], NULL, 10);
    if (number_threads <= 0 || number_threads > MAX_BANK_OFFICES)
        return RC_OTHER;

    // CREATE DATABASE
    if (initializeDataBase(&db)) return RC_OTHER;

    pthread_t thread_array[number_threads];
    for (int i = 0; i < number_threads; i++) {
        pthread_create(&thread_array[i], NULL, box_office, NULL);
    }

    // ADMIN ACC
    bank_account_t account;
    createAccount(&account, argv[2], 0, 0);
    addAccount(account, &db);

    tlv_request_t request;

    //OPENING LOG
    int fd = open(SERVER_LOGFILE, O_WRONLY | O_APPEND | O_CREAT, 0777);

    // REQUEST LOOP
    mkfifo(SERVER_FIFO_PATH, 0666);
    while (1) {
        if (get_request(&request)) return RC_OTHER;

        pthread_mutex_lock(&q_mutex);
        logSyncMech(fd, getpid(), SYNC_OP_MUTEX_LOCK, SYNC_ROLE_ACCOUNT, request.value.header.account_id);

        queue[last] = request;
        last = (last + 1) % QUEUE_MAX;
        
        pthread_mutex_unlock(&q_mutex);
        logSyncMech(fd, getpid(), SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_ACCOUNT, request.value.header.account_id);

        if (closing_server) break;
    }

    while (first != last);

    for(int i = 0; i < number_threads; i++){
        pthread_kill(thread_array[i], SIGTERM);
    }
    unlink(SERVER_FIFO_PATH);

    close(fd);

    return 0;
}