#include "serverMessage.h"
#include "box_office.h"

// Server Program

int main(int argc, char *argv[])
{
    // Server <box offices> <password>
    if (argc != 3)
    {
        printf("./server <box offices> <password>\n");
        return RC_OTHER;
    }

    // Number of box offices
    int number_threads = strtol(argv[1], NULL, 10);
    if (number_threads <= 0 || number_threads > MAX_BANK_OFFICES)
        return RC_OTHER;

    pthread_t thread_array[number_threads];

    pthread_mutex_t q_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&q_mutex);
    tlv_request_t queue[QUEUE_MAX];
    int first = 0;
    int last = -1;

    // CREATE DATABASE
    dataBase_t dataBase;
    if (initializeDataBase(&dataBase))
        return RC_OTHER;

    for (int i = 0; i < number_threads; i++)
    {
        box_office_t arg;
        arg.first = &first;
        arg.last = &last;
        arg.queue = &queue;
        arg.q_mutex = &q_mutex;
        arg.db = &dataBase;

        if (pthread_create(&thread_array[i], NULL, box_office, (void *)(&arg)))
        {
            perror("pthread_create failed.\n");
            break;
        }
        logBankOfficeOpen(STDERR_FILENO, i, thread_array[i]);
    }

    // Create admin account
    bank_account_t account;
    create_account(&account, argv[2], 0, 0);
    addAccount(account, &dataBase);

    //CRIAR BOX OFFICES
    //TODO

    // Set Communication
    tlv_request_t user_request;
    tlv_reply_t user_reply;


    //Create FIFO to receive request
    int fdr;
    mkfifo(SERVER_FIFO_PATH, 0666);

    // Receiving request
    while (1)
    {
        pthread_mutex_unlock(&q_mutex);
        
        if (get_request(&user_request, fdr))
            return RC_OTHER;

        printf("after unlock\n");

        pthread_mutex_lock(&q_mutex);
        last = (last + 1) % QUEUE_MAX;
        queue[last] = user_request;
    }

    // Make operation requested
    //TODO

    // Preparing reply
    if (replyMessageTLV(&user_request, &user_reply, &dataBase))
        return RC_USR_DOWN;

    // Sending reply
    if (send_reply(&user_request, &user_reply))
        return RC_USR_DOWN;

    // ESCREVER NO LOG

    return 0;
}
