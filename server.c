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
    mkfifo("secure_srv", 0x666);
    fd = open("secure_srv", O_RDONLY);

    //CRIAR CONTA ADMIN
    //Need to finish, some things not to use in the end
    dataBase dataBase_t;
    bank_account_t account;
    account.account_id = 2;
    initializeDataBase(&dataBase_t);
    addElement(account, &dataBase_t);
    printf("accoun_id: %d\n", dataBase_t.dataBaseArray[0].account_id);

    //CRIAR BOX OFFICES

    //FECHAR FIFO

    return 0;
}