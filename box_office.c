#include "box_office.h"

void *box_office(void *arg)
{
    tlv_request_t request;
    tlv_reply_t reply;

    int fd = open(SERVER_LOGFILE, O_WRONLY | O_APPEND | O_CREAT, 0777);

    logBankOfficeOpen(fd, getpid(), pthread_self());

    while (1)
    {
        sem_wait(&n_req);

        // Locks the mutex
        pthread_mutex_lock(&q_mutex);
        logSyncMech(fd, getpid(), SYNC_OP_MUTEX_LOCK, SYNC_ROLE_CONSUMER, request.value.header.account_id);

        usleep(request.value.header.op_delay_ms * 1000);

        request = front(queue); // Gets the request that arrived first

        pop(&queue); // Updates the queue and 'frees' the space ocupided by the request picked up by this thread

        pthread_mutex_unlock(&q_mutex);
        logSyncMech(fd, getpid(), SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, request.value.header.account_id);

        pthread_mutex_lock(&db_mutex);
        logSyncMech(fd, getpid(), SYNC_OP_MUTEX_LOCK, SYNC_ROLE_ACCOUNT, request.value.header.account_id);

        // Handles the request
        if (log_in(&db, request.value.header.account_id, request.value.header.password))
        {
            int op = (int)request.type;
            bank_account_t acc;

            switch (op)
            {
            case 0: // CREATE
                create_account(&acc, request.value.create.password, request.value.create.account_id, request.value.create.balance, &reply);
                if (addAccount(acc, &db))
                    return (void *)RC_OTHER;

                break;
            case 1: // CHECK BALANCE
                acc = *accountExist(request.value.transfer.account_id, &db);
                check_balance(&acc, &reply);
                break;
            case 2: // TRANSFER
                usleep(request.value.header.op_delay_ms * 1000);
                transfer(request, &reply);
                break;
            case 3: // SHUTDOWN
                usleep(request.value.header.op_delay_ms * 1000);
                shutdown((int *)arg);
                break;
            default:
                break;
            }

            pthread_mutex_unlock(&db_mutex);
            logSyncMech(fd, getpid(), SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_ACCOUNT, request.value.header.account_id);

            if (send_reply(&request, &reply))
                return (void *)RC_OTHER;
        }
        else
        {
            pthread_mutex_unlock(&db_mutex);
            logSyncMech(fd, getpid(), SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_ACCOUNT, request.value.header.account_id);
        }

        sem_post(&b_off);
    }

    logBankOfficeClose(fd, getpid(), pthread_self());

    close(fd);

    return NULL;
}

int create_account(bank_account_t *account, char password[], int accound_id, int balance, tlv_reply_t *user_reply)
{
    // echo -n “<senha><sal>” | sha256sum
    // echo -n $salt | sha256sum
    user_reply->length = 0;
    user_reply->type = OP_CREATE_ACCOUNT;

    account->account_id = accound_id;
    user_reply->value.header.account_id = accound_id;
    user_reply->length += sizeof(rep_header_t);

    account->balance = balance;

    creatSalt(account->salt);

    getHash(account->salt, password, account->hash);

    user_reply->value.header.ret_code = RC_OK;

    return 0;
}

int check_balance(bank_account_t *bank_account, tlv_reply_t *user_reply)
{
    user_reply->length = 0;

    user_reply->type = OP_BALANCE;

    if (bank_account == NULL)
    {
        user_reply->value.header.ret_code = RC_ID_NOT_FOUND;
        user_reply->length += sizeof(rep_header_t);

        return RC_ID_NOT_FOUND;
    }

    user_reply->value.balance.balance = bank_account->balance;

    user_reply->length += sizeof(rep_balance_t);

    user_reply->value.header.account_id = bank_account->account_id;
    user_reply->value.header.ret_code = RC_OK;

    return 0;
}

int transfer(tlv_request_t user_request, tlv_reply_t *user_reply)
{
    user_reply->length = 0;

    user_reply->type = OP_TRANSFER;

    user_reply->value.header.account_id = user_request.value.header.account_id;

    // DOES DESTINATION ACCOUNT EXIST?
    bank_account_t *bank_account_destination =
        accountExist(user_request.value.transfer.account_id, &db);

    if (bank_account_destination == NULL)
    {
        user_reply->value.header.ret_code = RC_ID_NOT_FOUND;
        user_reply->length += sizeof(rep_header_t);

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
        user_reply->length += sizeof(rep_header_t);

        return RC_NO_FUNDS;
    }

    if ((bank_account_destination->balance + amount) > MAX_BALANCE)
    {
        user_reply->value.header.ret_code = RC_TOO_HIGH;
        user_reply->length += sizeof(rep_header_t);

        return RC_TOO_HIGH;
    }

    //========================================

    bank_account_origin->balance -= amount;
    bank_account_destination->balance += amount;

    user_reply->value.transfer.balance = bank_account_origin->balance;
    user_reply->length += sizeof(rep_header_t);
    user_reply->length += sizeof(rep_transfer_t);
    user_reply->value.header.ret_code = RC_OK;

    return 0;
}

void shutdown(int *closing)
{
    int fd = open(SERVER_FIFO_PATH, O_RDONLY);
    fchmod(fd, 0444);
    close(fd);
    *closing = 1;
}

int log_in(dataBase_t *db, uint32_t account_id, char password[MAX_PASSWORD_LEN + 1])
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