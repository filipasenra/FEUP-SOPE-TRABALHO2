#include "serverMessage.h"

// Server Program

int main(int argc, char *argv[]) {
    // Server <box offices> <password>
    if (argc != 3) {
        printf("./server <box offices> <password>\n");
        return RC_OTHER;
    }

    // Number of box offices
    int number_threads = strtol(argv[1], NULL, 10);
    if (number_threads <= 0 || number_threads > MAX_BANK_OFFICES)
        return RC_OTHER;

    sem_t *sem = sem_open(argv[2], O_CREAT, 0600, number_threads);
    if (sem == SEM_FAILED) return RC_OTHER;

    pthread_mutex_t mutex_array[number_threads];
    pthread_t thread_array[number_threads];

    pthread_mutex_t q_mutex = PTHREAD_MUTEX_INITIALIZER;
    tlv_request_t queue[20];
    int first = 0;
    int last = -1;

    for (int i = 0; i < number_threads; i++) {
        // create mutex
        pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
        mutex_array[i] = m;

        // create thread
        struct thread_arg arg;
        arg.sem = sem;
        arg.mutex = &m;
        arg.first = &first;
        arg.last = &last;
        arg.queue = queue;
        arg.q_mutex = &q_mutex;

        pthread_create(&thread_array[i], NULL, box_office, (void *)(&arg));
        logBankOfficeOpen(STDERR_FILENO, i, thread_array[i]);
    }

    // Create accounts database
    dataBase_t dataBase;
    if (initializeDataBase(&dataBase)) return RC_OTHER;

    // Create admin account
    bank_account_t account;
    creatAccount(&account, argv[2], 0, 0);
    addAccount(account, &dataBase);

    // OPEN FIFO
    if (mkfifo(SERVER_FIFO_PATH, 0666)) return RC_OTHER;
    if ((ff = open(SERVER_FIFO_PATH, O_WRONLY)) < 0) return RC_OTHER;
    if (close(ff)) return RC_OTHER;

    tlv_request_t request;
    tlv_reply_t reply;

    while (1) {
        if (get_request(&request)) return RC_OTHER;
        if (log_in()) {
            pthread_mutex_lock(&q_mutex);
            last = (last + 1) % 20;
            queue[last] = request;
            pthread_mutex_unlock(&q_mutex);
        }
    }

    // ESCREVER NO LOG

    return RC_OK;
}
