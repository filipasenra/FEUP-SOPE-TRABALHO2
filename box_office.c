#include "box_office.h"

void *box_office(void *arg) {
    // LOOP TO SOLVE REQUESTS
    struct thread_arg ta = *(struct thread_arg *)arg;
    pthread_mutex_t mutex = ta.mutex;
    sem_t *sem = ta.sem;

    pthread_mutex_t q_mutex = ta.q_mutex;
    int *first = ta.first;
    int *last = ta.last;
    dataBase_t *db = ta.db;

    tlv_request_t request;
    tlv_reply_t reply;
    
    while (1) {
        pthread_mutex_lock(&q_mutex);
        request = ta.queue[*first];
        *first = (*first + 1) % QUEUE_MAX;
        pthread_mutex_unlock(&q_mutex);

        if (log_in(db, request.value.header.account_id, request.value.header.password)) {
            int op = (int)request.type;
            int ret_value = 0;
            bank_account_t acc;


            switch (op) {
                case 0:  // CREATE
                    if(ret_value = create_account(&acc, request.value.create.password , request.value.create.account_id, request.value.create.balance)) return RC_OTHER;
                    if(addAccount(acc, db)) return RC_OTHER;
                    break;
                case 1:  // CHECK BALANCE
                    ret_value = check_balance();
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

            if (send_reply()) return RC_OTHER;
        }
    }
}

int check_balance() {}

int transfer() {}

void shutdown() { pthread_exit(NULL); }

int log_in(dataBase_t *db, uint32_t account_id,
           char password[MAX_PASSWORD_LEN + 1]) {
    bank_account_t acc;
    char hash[HASH_LEN + 1];

    for (int i = 0; i < db->size; i++) {
        acc = db->dataBaseArray[i];
        if (acc.account_id == account_id) {
            getHash(acc.salt, password, hash);
            if (acc.hash == hash) return 1;
        }
    }
    return 0;
}