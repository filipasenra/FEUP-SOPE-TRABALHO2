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
                   request.value.header.password) != 0)
        {
            reply.type = request.type;
            reply.value.header.account_id = request.value.header.account_id;
            reply.value.header.ret_code = RC_LOGIN_FAIL;

            *return_value = RC_LOGIN_FAIL;
            return return_value;
        }

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

            acc = *accountExist(request.value.header.account_id, db);
            check_balance(acc, &reply);

            break;

        case 2: // TRANSFER

            transfer(request, &reply, db);

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

    return NULL;
}

int check_balance(bank_account_t bank_account, tlv_reply_t *user_reply)
{

    user_reply->length = 0;

    user_reply->type = OP_BALANCE;
    user_reply->length += sizeof(user_reply->type);

    user_reply->value.balance.balance = bank_account.balance;
    user_reply->length += sizeof(user_reply->value.balance);

    user_reply->value.header.account_id = bank_account.account_id;
    user_reply->value.header.ret_code = RC_OK;

    return 0;
}

int transfer(tlv_request_t user_request, tlv_reply_t *user_reply, dataBase_t *db)
{

    user_reply->length = 0;

    user_reply->type = OP_TRANSFER;
    user_reply->length += sizeof(user_reply->type);

    user_reply->value.header.account_id = user_request.value.header.account_id;

    //DOES DESTINATION ACCOUNT EXIST?
    bank_account_t *bank_account_destination = accountExist(user_request.value.transfer.account_id, db);

    if (bank_account_destination == NULL)
    {
        user_reply->value.header.ret_code = RC_ID_NOT_FOUND;
        user_reply->length += sizeof(user_reply->value.header);

        return RC_ID_NOT_FOUND;
    }

    //====================================

    bank_account_t *bank_account_origin = accountExist(user_request.value.header.account_id, db);

    int amount = user_request.value.transfer.amount;

    //ARE THE FINAL BALANCES WITHIN THE LIMITES?
    if ((bank_account_origin->balance - amount) < MIN_BALANCE)
    {
        user_reply->value.header.ret_code = RC_NO_FUNDS;
        user_reply->length += sizeof(user_reply->value.header);

        return RC_NO_FUNDS;
    }

    if ((bank_account_destination->balance + amount) > MAX_BALANCE)
    {
        user_reply->value.header.ret_code = RC_TOO_HIGH;
        user_reply->length += sizeof(user_reply->value.header);

        return RC_TOO_HIGH;
    }

    //========================================

    bank_account_origin->balance -= amount;
    bank_account_destination += amount;

    user_reply->value.transfer.balance = bank_account_origin->balance;
    user_reply->length += sizeof(user_reply->value.transfer);

    return 0;
}

void shutdown() { pthread_exit(NULL); }

int log_in(dataBase_t *db, uint32_t account_id, char password[MAX_PASSWORD_LEN + 1])
{
    bank_account_t *acc = accountExist(account_id, db);

    if (acc == NULL)
        return 1;

    char hash[HASH_LEN + 1];

    if (acc->account_id == account_id)
    {
        getHash(acc->salt, password, hash);
        if (acc->hash == hash)
            return 0;
    }

    return 1;
}