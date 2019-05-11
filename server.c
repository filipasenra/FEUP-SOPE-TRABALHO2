#include "serverMessage.h"

// Server Program

int main(int argc, char *argv[])
{
    //Server <box offices> <password>
    if (argc != 3)
    {
        printf("./server <box offices> <password>\n");
        return RC_OTHER;
    }

    //Number of box offices
    int number_threads = strtol(argv[1], NULL, 10);
    if (number_threads <= 0 || number_threads > MAX_BANK_OFFICES)
        exit(1);

    sem_t *sem = sem_open(argv[2], O_CREAT, 0600, number_threads);
    if (sem == SEM_FAILED)
        exit(2);

    pthread_mutex_t mutex_array[number_threads];
    pthread_t thread_array[number_threads];
    for (int i = 0; i < number_threads; i++)
    {
        //create mutex
        pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
        mutex_array[i] = m;

        //create thread
        struct thread_arg arg;
        arg.sem = sem;
        arg.mutex = m;
        pthread_create(&thread_array[i], NULL, box_office, (void *)(&arg));
    }

    //Create accounts database
    dataBase_t dataBase;
    if (initializeDataBase(&dataBase))
        return RC_OTHER;

    //Create admin account
    bank_account_t account;
    creatAccount(&account, argv[2], 0, 0);
    addAccount(account, &dataBase);

    //CRIAR BOX OFFICES

    //Receive request and send answer to user
    tlv_request_t user_request;
    tlv_reply_t user_reply;
    if (sendReply(&user_request, &user_reply, &dataBase))
        return RC_USR_DOWN;

    //ESCREVER NO LOG

    return RC_OK;
}
