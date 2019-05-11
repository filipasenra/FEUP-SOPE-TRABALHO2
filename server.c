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

    pthread_t thread_array[number_threads];

    pthread_mutex_t q_mutex = PTHREAD_MUTEX_INITIALIZER;
    tlv_request_t queue[QUEUE_MAX];
    int first = 0;
    int last = -1;

    // CREATE DATABASE
    dataBase_t dataBase;
    if (initializeDataBase(&dataBase)) return RC_OTHER;

    for (int i = 0; i < number_threads; i++) {
        box_office_t arg;
        arg.first = &first;
        arg.last = &last;
        arg.queue = queue;
        arg.q_mutex = &q_mutex;
        arg.db = &dataBase

                     pthread_create(&thread_array[i], NULL, box_office,
                                    (void *)(&arg));
        logBankOfficeOpen(STDERR_FILENO, i, thread_array[i]);
    }

    // CREATE ADMIN
    bank_account_t account;
    creatAccount(&account, argv[2], 0, 0);
    addAccount(account, &dataBase);

    // OPEN FIFO
    if (mkfifo(SERVER_FIFO_PATH, 0666)) return RC_OTHER;
    if ((ff = open(SERVER_FIFO_PATH, O_WRONLY)) < 0) return RC_OTHER;

    tlv_request_t request;
    while (1) {
        if (get_request(&request)) return RC_OTHER;
        pthread_mutex_lock(&q_mutex);
        last = (last + 1) % QUEUE_MAX;
        queue[last] = request;
        pthread_mutex_unlock(&q_mutex);
    }

    // ESCREVER NO LOG

    return RC_OK;
}
