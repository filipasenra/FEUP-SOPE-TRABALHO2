#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

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
pthread_mutex_t db_mutex[MAX_BANK_ACCOUNTS];
queue_t queue;
dataBase_t db;
int number_threads = 0;

int server_init(char *password, int number_threads, pthread_t thread_array[], bank_account_t *acc, int *fd_log, int *fd_srv);
void server_main_loop(int fd_log, int fd_srv);

// Server Program
int main(int argc, char *argv[])
{
    // ./server <box offices> <password>
    if (argc != 3)
    {
        printf("./server <box offices> <password>\n");
        return 1;
    }

    number_threads = strtol(argv[1], NULL, 10);
    pthread_t thread_array[number_threads];
    bank_account_t account;
    int fd_log;
    int fd_srv;

    if (number_threads <= 0 || number_threads > MAX_BANK_OFFICES)
        return 1;

    if (strlen(argv[2]) > MAX_PASSWORD_LEN + 1)
    {
        printf("Password too long\n");
        return 1;
    }
    else if (strlen(argv[2]) < MIN_PASSWORD_LEN)
    {
        printf("Password too short\n");
        return 1;
    }

    if (server_init(argv[2], number_threads, thread_array, &account, &fd_log, &fd_srv))
        return 2;

    server_main_loop(fd_log, fd_srv);

    while (!isEmpty(queue))
        ;

    int value = -1;
    while (value < number_threads)
    {
        sem_getvalue(&b_off, &value);
    }

    for (int i = 0; i < number_threads; i++)
    {
        logBankOfficeClose(fd_log, getpid(), thread_array[i]);
        pthread_cancel(thread_array[i]);
    }

    close(fd_log);

    unlink(SERVER_FIFO_PATH);

    pthread_mutex_destroy(&q_mutex);
    for (int i = 0; i < MAX_BANK_ACCOUNTS; i++)
        pthread_mutex_destroy(&db_mutex[i]);
    sem_destroy(&n_req);
    sem_destroy(&b_off);

    return 0;
}

int server_init(char *password, int number_threads, pthread_t thread_array[], bank_account_t *account, int *fd_log, int *fd_srv)
{
    if ((*fd_log = open(SERVER_LOGFILE, O_WRONLY | O_APPEND | O_CREAT, 0777)) < 0)
    {
        perror("server_init");
        return 1;
    }

    if (mkfifo(SERVER_FIFO_PATH, 0666) < 0)
    {
        perror("server_init");
        return 1;
    }

    if ((*fd_srv = open(SERVER_FIFO_PATH, O_RDONLY)) < 0)
    {
        perror("server_init");
        return 1;
    }

    sem_init(&n_req, 0, 0);
    sem_init(&b_off, 0, number_threads);

    queueInitialize(&queue);
    init_database(&db);

    createAccount(account, password, 0, 0);
    if (add_account(*account, &db))
        return 2;

    
    for (int i = 0; i < MAX_BANK_ACCOUNTS; i++)
    {
        pthread_mutex_init(& db_mutex[i], NULL);
    }

    for (int i = 0; i < number_threads; i++)
        pthread_create(&thread_array[i], NULL, box_office, fd_log);

    return 0;
}

void server_main_loop(int fd_log, int fd_srv)
{
    tlv_request_t request;
    int value = 0;

    while (1)
    {
        if (get_request(&request, fd_log, fd_srv))
            return;

        if (request.length)
        {
            pthread_mutex_lock(&q_mutex);
            logSyncMech(fd_log, getpid(), SYNC_OP_MUTEX_LOCK, SYNC_ROLE_PRODUCER, request.value.header.account_id);

            sem_wait(&b_off);
            sem_getvalue(&b_off, &value);
            logSyncMechSem(fd_log, getpid(), SYNC_OP_SEM_WAIT, SYNC_ROLE_PRODUCER, request.value.header.account_id, value);

            push(&queue, request);

            sem_post(&n_req);
            sem_getvalue(&n_req, &value);
            logSyncMechSem(fd_log, getpid(), SYNC_OP_SEM_POST, SYNC_ROLE_PRODUCER, request.value.header.account_id, value);

            pthread_mutex_unlock(&q_mutex);
            logSyncMech(fd_log, getpid(), SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_PRODUCER, request.value.header.account_id);

            request.length = 0;

            if (request.type == OP_SHUTDOWN && request.value.header.account_id == 0)
            {
                if (log_in(&db, 0, request.value.header.password) == 0)
                {
                    logDelay(fd_log, getpid(), request.value.header.op_delay_ms * 1000);
                    usleep(request.value.header.op_delay_ms * 1000);
                    fchmod(fd_srv, 0444);
                    return;
                }
            }
        }
    }
}