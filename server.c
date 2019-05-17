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
#include <errno.h>

sem_t n_req;
sem_t b_off;
pthread_mutex_t q_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;
queue_t queue;
dataBase_t db;

void server_init(char *password, int number_threads, pthread_t thread_array[], bank_account_t *acc, int *fd_log, int *fd_srv);
void server_main_loop(int fd_log, int fd_srv);

// Server Program
int main(int argc, char *argv[])
{
    // ./server <box offices> <password>
    if (argc != 3)
    {
        printf("./server <box offices> <password>\n");
        return RC_OTHER;
    }

    int number_threads = strtol(argv[1], NULL, 10);
    pthread_t thread_array[number_threads];
    bank_account_t account;
    int fd_log;
    int fd_srv;

    if (number_threads <= 0 || number_threads > MAX_BANK_OFFICES)
        return RC_OTHER;

    server_init(argv[2], number_threads, thread_array, &account, &fd_log, &fd_srv);

    server_main_loop(fd_log, fd_srv);

    while (!isEmpty(queue))
        ;

    int value = 1;
    while (value != number_threads)
    {
        sem_getvalue(&b_off, &value);
    }


    freeDataBase(&db);
    close(fd_log);

    unlink(SERVER_FIFO_PATH);

    for (int i = 0; i < number_threads; i++)
        pthread_kill(thread_array[i], SIGTERM);

    return 0;
}

void server_init(char *password, int number_threads, pthread_t thread_array[], bank_account_t *account, int *fd_log, int *fd_srv)
{
    *fd_log = open(SERVER_LOGFILE, O_WRONLY | O_APPEND | O_CREAT, 0777);

    if (mkfifo(SERVER_FIFO_PATH, 0666) < 0)
    {

        if (errno == EEXIST)
            printf("FIFO '/tmp/requests' already exists\n");
        else
            printf("Can't create FIFO\n");
    }

    *fd_srv = open(SERVER_FIFO_PATH, O_RDONLY);

    if (*fd_srv < 0)
    {
        perror("server_init");
        return;
    }

    sem_init(&n_req, 0, 0);
    sem_init(&b_off, 0, number_threads);

    queueInitialize(&queue);

    if (initializeDataBase(&db))
        return;
    createAccount(account, password, 0, 0);
    addAccount(*account, &db);

    for (int i = 0; i < number_threads; i++)
        pthread_create(&thread_array[i], NULL, box_office, NULL);
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

            if (request.type == OP_SHUTDOWN)
            {
                fchmod(fd_srv, 0444);
                return;
            }
        }
    }
}