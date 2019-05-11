#include "box_office.h"

void* box_office(void* arg) {
    // LOOP TO SOLVE REQUESTS
    struct thread_arg ta = *(struct thread_arg*)arg;
    pthread_mutex_t mutex = ta.mutex;
    sem_t* sem = ta.sem;

    pthread_mutex_t q_mutex = ta.q_mutex;
    int *first = ta.first;
    int *last = ta.last;

    while (1) {
        pthread_mutex_lock(&q_mutex);
        tlv_request_t request = ta.queue[*first];
        *first = (*first + 1) % QUEUE_MAX;
        pthread_mutex_unlock(&q_mutex);

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
}