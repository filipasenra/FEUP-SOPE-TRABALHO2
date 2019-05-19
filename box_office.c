#include "box_office.h"

void *box_office(void *arg) {
    tlv_request_t request;
    tlv_reply_t reply;

    logBankOfficeOpen(*(int *)arg, getpid(), pthread_self());

    while (1) {
        sem_wait(&n_req);

        pthread_mutex_lock(&q_mutex);
        logSyncMech(*(int *)arg, pthread_self(), SYNC_OP_MUTEX_LOCK, SYNC_ROLE_CONSUMER, request.value.header.account_id);

        request = front(queue);  // Gets the request that arrived first

        pop(&queue);  // Updates the queue and 'frees' the space ocupided by the request picked up by this thread

        pthread_mutex_unlock(&q_mutex);
        logSyncMech(*(int *)arg, pthread_self(), SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, request.value.header.account_id);

        // Handles the request
        reply.value.header.account_id = request.value.header.account_id;
        reply.value.header.ret_code = RC_OK;
        reply.length = sizeof(rep_header_t);

        logSyncDelay(*(int *)arg, pthread_self(), request.value.header.account_id, request.value.header.op_delay_ms * 1000);
        usleep(request.value.header.op_delay_ms * 1000);

        int index;

        if ((index = log_in(&db, request.value.header.account_id, request.value.header.password)) != -1) {

            pthread_mutex_lock(&db_mutex[index]);
            logSyncMech(*(int *)arg, pthread_self(), SYNC_OP_MUTEX_LOCK, SYNC_ROLE_ACCOUNT, request.value.header.account_id);

            int op = (int)request.type;
            bank_account_t acc;

            switch (op) {
                case 0:  // CREATE
                    if (request.value.header.account_id != 0) {
                        reply.value.header.ret_code = RC_OP_NALLOW;
                        break;
                    }
                    if (get_account(request.value.create.account_id, &db) != -1) {
                        reply.value.header.ret_code = RC_ID_IN_USE;
                        break;
                    }

                    create_account(&acc, request.value.create.password, request.value.create.account_id, request.value.create.balance, &reply);

                    if (add_account(acc, &db)) {
                        reply.value.header.ret_code = RC_OTHER;
                        break;
                    }

                    break;
                case 1:  // CHECK BALANCE
                    if (request.value.header.account_id == 0) {
                        reply.value.header.ret_code = RC_OP_NALLOW;
                        break;
                    }

                    acc = db.dataBaseArray[index];
                    check_balance(&acc, &reply);
                    break;
                case 2:  // TRANSFER
                    if (request.value.header.account_id == 0) {
                        reply.value.header.ret_code = RC_OP_NALLOW;
                        break;
                    }
                    transfer(request, &reply, *(int *)arg, request.value.header.op_delay_ms * 1000);
                    break;
                case 3:  // SHUTDOWN
                    if (request.value.header.account_id != 0) {
                        reply.value.header.ret_code = RC_OP_NALLOW;
                        break;
                    }
                    shutdown(&reply);
                    break;
                default:
                    break;
            }

            pthread_mutex_unlock(&db_mutex[index]);
            logSyncMech(*(int *)arg, pthread_self(), SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_ACCOUNT, request.value.header.account_id);
        } else {
            reply.value.header.ret_code = RC_LOGIN_FAIL;
        }

        if (send_reply(&request, &reply) != RC_OK) {
            /* sem_post(&b_off);
             return (void *)RC_OTHER;*/

            // needs message of error to slog
        }

        sem_post(&b_off);
    }

    return NULL;
}

int create_account(bank_account_t *account, char password[], int accound_id,
                   int balance, tlv_reply_t *user_reply) {
    // echo -n “<senha><sal>” | sha256sum
    // echo -n $salt | sha256sum

    //verificação do tamanho já é feita no user!

    account->account_id = accound_id;
    account->balance = balance;

    creatSalt(account->salt);
    getHash(account->salt, password, account->hash);

    user_reply->type = OP_CREATE_ACCOUNT;
    user_reply->value.header.ret_code = RC_OK;

    int fd = open(SERVER_LOGFILE, O_WRONLY | O_APPEND | O_CREAT, 0777);
    logAccountCreation(fd, pthread_self(), account);

    close(fd);

    return 0;
}

int check_balance(bank_account_t *bank_account, tlv_reply_t *user_reply) {
    user_reply->type = OP_BALANCE;

    if (bank_account == NULL) {
        user_reply->value.header.ret_code = RC_ID_NOT_FOUND;
        return RC_ID_NOT_FOUND;
    }

    user_reply->value.balance.balance = bank_account->balance;
    user_reply->length += sizeof(rep_balance_t);
    user_reply->value.header.ret_code = RC_OK;

    return 0;
}

int transfer(tlv_request_t user_request, tlv_reply_t *user_reply, int fd, uint32_t delay) {
    user_reply->type = OP_TRANSFER;

    logSyncDelay(fd, pthread_self(), user_request.value.transfer.account_id, delay);
    usleep(delay);


    int index = get_account(user_request.value.transfer.account_id, &db);
    if (index == -1) {
        user_reply->value.header.ret_code = RC_ID_NOT_FOUND;
        return RC_ID_NOT_FOUND;
    }

    pthread_mutex_lock(&db_mutex[index]);
    logSyncMech(fd, pthread_self(), SYNC_OP_MUTEX_LOCK, SYNC_ROLE_ACCOUNT, user_request.value.transfer.account_id);

    bank_account_t *bank_acc_dest = &(db.dataBaseArray[index]);

    index = get_account(user_request.value.header.account_id, &db);
    bank_account_t *bank_acc_orig = &(db.dataBaseArray[index]);

    int amount = user_request.value.transfer.amount;

    if ((bank_acc_orig->balance - amount) < MIN_BALANCE) {
        user_reply->value.header.ret_code = RC_NO_FUNDS;
        return RC_NO_FUNDS;
    }

    if ((bank_acc_dest->balance + amount) > MAX_BALANCE) {
        user_reply->value.header.ret_code = RC_TOO_HIGH;
        return RC_TOO_HIGH;
    }

    bank_acc_orig->balance -= amount;
    bank_acc_dest->balance += amount;

    pthread_mutex_unlock(&db_mutex[index]);
    logSyncMech(fd, pthread_self(), SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_ACCOUNT, user_request.value.transfer.account_id);

    user_reply->value.transfer.balance = bank_acc_orig->balance;
    user_reply->length += sizeof(rep_transfer_t);
    user_reply->value.header.ret_code = RC_OK;

    return 0;
}

void shutdown(tlv_reply_t *user_reply) {
    int value = 1;
    sem_getvalue(&b_off, &value);

    user_reply->type = OP_SHUTDOWN;
    user_reply->value.shutdown.active_offices = number_threads - value;
    user_reply->length += sizeof(rep_shutdown_t);
    user_reply->value.header.ret_code = RC_OK;
}