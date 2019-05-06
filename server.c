#include "box_office.c"

// Server Program

int main(int argc, char *argv[]) {
    //server <BOX OFFICES> <NAME>
    int number_threads = strtol(argv[1]);
    if(number_threads <= 0 || number_threads > MAX_BANK_OFFICES)    exit(1);


    sem_t* sem = sem_open(argv[2], O_CREAT, 0600, number_threads);
    if(sem == SEM_FAILED)   exit(2);

    pthread_mutex_t mutex_array[number_threads];
    pthread_t thread_array[number_threads];
    for(int i = 0; i < number_threads; i++){
        //criar mutex
        pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
        mutex_array[i] = m;
        
        //criar thread
        struct thread_arg arg; arg.sem = sem; arg.mutex = m;
        pthread_create(&thread_array[i], NULL, box_office,(void*)(&thread_arg) )
    }



    //OPEN FIFO TO READ
    //CRIAR CONTA ADMIN
    //CRIAR BOX OFFICES


    //FECHAR FIFO

    return 0;
}