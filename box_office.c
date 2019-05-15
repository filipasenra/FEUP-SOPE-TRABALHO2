#include "box_office.h"

void *box_office(void *arg)
{
    tlv_request_t request;
    tlv_reply_t reply;

    while (1)
    {
        pthread_mutex_lock(&q_mutex);
        request = queue[first];
        if (request.length == 0)
        {
            pthread_mutex_unlock(&q_mutex);
            continue;
        }
        first = (first + 1) % QUEUE_MAX;
        request.length = 0;
        pthread_mutex_unlock(&q_mutex);

        pthread_mutex_lock(&db_mutex);

        if (log_in(&db, request.value.header.account_id,
                   request.value.header.password))
        {
            int op = (int)request.type;
            bank_account_t acc;

            switch (op)
            {
            case 0: // CREATE
                create_account(&acc, request.value.create.password,
                               request.value.create.account_id,
                               request.value.create.balance);
                if (addAccount(acc, &db))
                    return (void *)RC_OTHER;
                printf("CREATE - ACCOUNT - %d", request.value.create.account_id);
                break;
            case 1: // CHECK BALANCE

                acc = *accountExist(request.value.transfer.account_id, &db);
                check_balance(&acc, &reply);
                break;
            case 2: // TRANSFER
                transfer(request, &reply);
                break;
            case 3: // SHUTDOWN
                shutdown();
                break;
            default:
                break;
            }
            pthread_mutex_unlock(&db_mutex);
            
            if (send_reply(&request, &reply))
                return (void *)RC_OTHER;
        }
        else
            pthread_mutex_unlock(&db_mutex);
    }

    return NULL;
}

int check_balance(bank_account_t *bank_account, tlv_reply_t *user_reply)
{
    user_reply->length = 0;

    user_reply->type = OP_BALANCE;
    user_reply->length += sizeof(user_reply->type);

    if (bank_account == NULL)
    {
        user_reply->value.header.ret_code = RC_ID_NOT_FOUND;
        user_reply->length += sizeof(user_reply->value.header);

        return RC_ID_NOT_FOUND;
    }

    user_reply->value.balance.balance = bank_account->balance;
    user_reply->length += sizeof(user_reply->value.balance);

    user_reply->value.header.account_id = bank_account->account_id;
    user_reply->value.header.ret_code = RC_OK;

    return 0;
}

int transfer(tlv_request_t user_request, tlv_reply_t *user_reply)
{
    user_reply->length = 0;

    user_reply->type = OP_TRANSFER;
    user_reply->length += sizeof(user_reply->type);

    user_reply->value.header.account_id = user_request.value.header.account_id;

    // DOES DESTINATION ACCOUNT EXIST?
    bank_account_t *bank_account_destination =
        accountExist(user_request.value.transfer.account_id, &db);

    if (bank_account_destination == NULL)
    {
        user_reply->value.header.ret_code = RC_ID_NOT_FOUND;
        user_reply->length += sizeof(user_reply->value.header);

        return RC_ID_NOT_FOUND;
    }

    //====================================

    bank_account_t *bank_account_origin =
        accountExist(user_request.value.header.account_id, &db);

    int amount = user_request.value.transfer.amount;

    // ARE THE FINAL BALANCES WITHIN THE LIMITES?
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
    bank_account_destination->balance += amount;

    user_reply->value.transfer.balance = bank_account_origin->balance;
    user_reply->length += sizeof(user_reply->value.transfer);

    return 0;
}

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

    return 1;
}