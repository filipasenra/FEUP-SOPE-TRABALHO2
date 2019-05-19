
#include <errno.h>
#include "box_office.h"
#include "communication.h"
#include "creatAccount.h"
#include "dataBase.h"
#include "queue.h"
#include "sope.h"
#include "types.h"
#include "serverHandler.h"

/**
 * To be shared with the threads
*/

sem_t n_req; ///< Semaphore that represents the number of request to be treated
sem_t b_off; ///< Semaphore that represents the number of free threads

pthread_mutex_t q_mutex = PTHREAD_MUTEX_INITIALIZER; ///< Mutex that authorizes the access to the queue
pthread_mutex_t db_mutex[MAX_BANK_ACCOUNTS];         ///< Array of mutex that authorizes the access to the Accounts

queue_t queue; ///< Queue with the requestes to be treated

dataBase_t db; ///< Data Base with the accounts of the bank

int number_threads = 0; ///< Number of threads


// Server Program
int main(int argc, char *argv[])
{
    number_threads = strtol(argv[1], NULL, 10);
    pthread_t thread_array[number_threads];
    bank_account_t account;
    int fd_log;
    int fd_srv;

    if (checkArg(argc, argv) != 0)
        return 1;

    if (server_init(argv[2], number_threads, thread_array, &account, &fd_log, &fd_srv))
        return 2;

    server_main_loop(fd_log, fd_srv);

    closingServer(fd_log, thread_array);

    return 0;
}