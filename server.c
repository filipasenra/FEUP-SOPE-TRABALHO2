#include "types.h"
#include "sope.h"
#include "types.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "sope.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include "dataBase.h"
#include "box_office.h"
#include "creatAccount.h"

// Server Program

int main(int argc, char *argv[])
{
    //server <BOX OFFICES> <NAME>
    if (argc != 3)
    {
        printf("./server <BOX OFFICES> <NAME>\n");
        return 1;
    }

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
        //criar mutex
        pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
        mutex_array[i] = m;

        //criar thread
        struct thread_arg arg;
        arg.sem = sem;
        arg.mutex = m;
        pthread_create(&thread_array[i], NULL, box_office, (void *)(&arg));
    }

    //OPEN FIFO TO READ
    int fd;
    mkfifo(SERVER_FIFO_PATH, 0666);
    fd = open(SERVER_FIFO_PATH, O_RDONLY);

    //CRIAR CONTA ADMIN
    dataBase_t dataBase;
    bank_account_t account;
    creatAccount(&account, argv[2], 0, 0);
    initializeDataBase(&dataBase);
    addAccount(account, &dataBase);

    //CRIAR BOX OFFICES

    //FECHAR FIFO

    return 0;
}
