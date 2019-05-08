#include "box_office.h"

void* box_office(void* arg) {
    // LOOP TO SOLVE REQUESTS
    struct thread_arg ta = *(struct thread_arg*)arg;
    pthread_mutex_t mutex = ta.mutex;
    sem_t* sem = ta.sem;

    while (1) {
        // WAIT
        sem_wait(sem);

        // LOCK
        pthread_mutex_lock(&mutex);

        // DO STUFF
        if (log_in()) {
            int op = get_operation();
            int ret_value = 0;

            switch (op) {
                case 0:  // CREATE
                    ret_value = create_account();
                    break;
                case 1:  // CHECK BALANCE
                    ret_value = create_account();
                    break;
                case 2:  // TRANSFER
                    ret_value = transfer();
                    break;
                case 3:
                    shutdown();
                    break;
                default:
                    break;
            }
        }

        // UNLOCK
        pthread_mutex_unlock(&mutex);

        // SIGNAL
        sem_post(sem);
    }
}