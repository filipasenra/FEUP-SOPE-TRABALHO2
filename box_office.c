#include "box_office.h"

#include "communication.h"

void *box_office(void *arg)
{
    // LOOP TO SOLVE REQUESTS
    box_office_t ta = *(box_office_t *)arg;

    pthread_mutex_t mutex = *ta.q_mutex;
    int *first = ta.first;
    int *last = ta.last;
    dataBase_t *db = ta.db;

    tlv_request_t request;
    tlv_reply_t reply;

    int *return_value = malloc(sizeof(int));

    while (1)
    {
        pthread_mutex_lock(&mutex);
        request = *ta.queue[*first];
        *first = (*first + 1) % QUEUE_MAX;
        pthread_mutex_unlock(&mutex);

        if (log_in(db, request.value.header.account_id,
                   request.value.header.password))
        {
            int op = (int)request.type;
            bank_account_t acc;

            switch (op)
            {
            case 0: // CREATE
                if (create_account(&acc, request.value.create.password,
                                   request.value.create.account_id,
                                   request.value.create.balance))
                {

                    *return_value = RC_OTHER;
                    return return_value;
                }

                if (addAccount(acc, db))
                {
                    *return_value = RC_OTHER;
                    return return_value;
                }

                break;

            case 1: // CHECK BALANCE
                check_balance();
                break;
            case 2: // TRANSFER
                transfer();
                break;
            case 3:
                shutdown();
                break;
            default:
                break;
            }

            if (send_reply(&request, &reply))
            {
                *return_value = RC_OTHER;
                return return_value;
            }
        }
    }

    return NULL;
}

int check_balance() {}

int transfer() {}

void shutdown() { pthread_exit(NULL); }

int log_in(dataBase_t *db, uint32_t account_id,
           char password[MAX_PASSWORD_LEN + 1])
{
    bank_account_t acc;
    char hash[HASH_LEN + 1];

    for (int i = 0; i < db->size; i++)
    {
        acc = db->dataBaseArray[i];
        if (acc.account_id == account_id)
        {
            getHash(acc.salt, password, hash);
            if (acc.hash == hash)
                return 1;
        }
    }
    return 0;
}