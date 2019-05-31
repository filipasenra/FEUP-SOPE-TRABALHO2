#include "serverHandler.h"
#include "sope.h"
#include "box_office.h"

int server_fifo;
int server_stdw;

int checkArg(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("./server <box offices> <password>\n");
        return 1;
    }

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

    return 0;
}

int server_init(char *password, int number_threads, pthread_t thread_array[], bank_account_t *account, int *fd_log, int *fd_srv)
{
    if ((*fd_log = open(SERVER_LOGFILE, O_WRONLY | O_APPEND | O_CREAT, 0777)) < 0)
    {
        perror("LOGFILE");
        return 1;
    }
	unlink(SERVER_FIFO_PATH);
    if (mkfifo(SERVER_FIFO_PATH, 0666) < 0)
    {
        perror("MKFIFO");
        return 1;
    }

    if ((*fd_srv = open(SERVER_FIFO_PATH, O_RDONLY)) < 0)
    {
        perror("SERVERFIFO");
        return 1;
    }
    server_fifo = *fd_srv;

    sem_init(&n_req, 0, 0);
    logSyncMechSem(*fd_log, 0, SYNC_OP_SEM_INIT, SYNC_ROLE_PRODUCER, 0, 0);

    sem_init(&b_off, 0, number_threads);
    logSyncMechSem(*fd_log, 0, SYNC_OP_SEM_INIT, SYNC_ROLE_PRODUCER, 0, number_threads);

    queueInitialize(&queue);
    init_database(&db);

    createAccount(account, password, 0, 0);
    if (add_account(*account, &db)) return 2;

    logAccountCreation(*fd_log, 0, account);

    for (int i = 0; i < MAX_BANK_ACCOUNTS; i++) { 
        pthread_mutex_init(&db_mutex[i], NULL);
    }

    for (int i = 0; i < number_threads; i++) {
        pthread_create(&thread_array[i], NULL, box_office, fd_log);
        logBankOfficeOpen(*fd_log, 0, thread_array[i]);
    }

    fsync(*fd_log);

    return 0;
}

void server_main_loop(int fd_log, int fd_srv)
{
    tlv_request_t request;
    int value = 0;

    while (1){
        if (get_request(&request, fd_log, fd_srv)) return;

        if (request.length) {
            sem_wait(&b_off);
            sem_getvalue(&b_off, &value);
            logSyncMechSem(fd_log, 0, SYNC_OP_SEM_WAIT, SYNC_ROLE_PRODUCER, request.value.header.account_id, value);

            pthread_mutex_lock(&q_mutex);
            logSyncMech(fd_log, 0, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_PRODUCER, request.value.header.account_id);
            
            push(&queue, request);
            
            pthread_mutex_unlock(&q_mutex);
            logSyncMech(fd_log, 0, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_PRODUCER, request.value.header.account_id);

            sem_post(&n_req);
            sem_getvalue(&n_req, &value);
            logSyncMechSem(fd_log, 0, SYNC_OP_SEM_POST, SYNC_ROLE_PRODUCER, request.value.header.account_id, value);

            request.length = 0;
        } else if(server_stdw && isEmpty(queue)){
    		write(STDOUT_FILENO, "srv_stdw\n", 9);
            return;
        }
    }
}

void closingServer(int fd_log, pthread_t thread_array[number_threads])
{
    for (int i = 0; i < number_threads; i++)
    {
        pthread_join(thread_array[i], NULL);
        logBankOfficeClose(fd_log, 0, thread_array[i]);
    }

    close(fd_log);

    unlink(SERVER_FIFO_PATH);

    pthread_mutex_destroy(&q_mutex);
    for (int i = 0; i < MAX_BANK_ACCOUNTS; i++)
        pthread_mutex_destroy(&db_mutex[i]);
    sem_destroy(&n_req);
    sem_destroy(&b_off);
}
