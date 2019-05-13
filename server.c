#include "serverMessage.h"

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

    int number_threads = strtol(argv[1], NULL, 10);
    if (number_threads <= 0 || number_threads > MAX_BANK_OFFICES)
        return RC_OTHER;

    // CREATE DATABASE
    if (initializeDataBase(&db)) return RC_OTHER;

    pthread_t thread_array[number_threads];
    for (int i = 0; i < number_threads; i++) {
        pthread_create(&thread_array[i], NULL, box_office, NULL);
        logBankOfficeOpen(STDERR_FILENO, i, thread_array[i]);
    }

    // ADMIN ACC
    bank_account_t account;
    create_account(&account, argv[2], 0, 0);
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

    }

    return 0;
}
