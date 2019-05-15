#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
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
        pthread_create(&thread_array[i], NULL, box_office,
                       (void*)(&closing_server));
        logBankOfficeOpen(STDERR_FILENO, i, thread_array[i]);
    }

    // ADMIN ACC
    bank_account_t account;
    createAccount(&account, argv[2], 0, 0);
    addAccount(account, &db);

    tlv_request_t request;

    // REQUEST LOOP
    mkfifo(SERVER_FIFO_PATH, 0666);
    while (1) {
        if (get_request(&request)) return RC_OTHER;

        pthread_mutex_lock(&q_mutex);
        queue[last] = request;
        last = (last + 1) % QUEUE_MAX;
        pthread_mutex_unlock(&q_mutex);

        if (closing_server) break;
    }

    while (first != last);

    while(int i = 0; i < number_threads; i++){
        pthread_kill(thread_array[i], SIGTERM);
    }

    return 0;
}