#include "box_office.h"

int n_array = -1;

void *box_office(void *arg) {
    tlv_request_t request;
    tlv_reply_t reply;

    for (int n = 0; n < number_threads; n++) {
        if (thread_array[n] == pthread_self()) {
            n_array = n + 1;
            break;
        }
    }

    while (1) {
        sem_wait(&n_req);
        int value = -1;
        sem_getvalue(&b_off, &value);
        logSyncMechSem(*(int *)arg, n_array, SYNC_OP_SEM_WAIT,
                       SYNC_ROLE_CONSUMER, request.value.header.account_id,
                       value);

        pthread_mutex_lock(&q_mutex);
        logSyncMech(*(int *)arg, n_array, SYNC_OP_MUTEX_LOCK,
                    SYNC_ROLE_CONSUMER, request.value.header.account_id);

        request = front(queue);  // Gets the request that arrived first

        pop(&queue);  // Updates the queue and 'frees' the space ocupided by the
                      // request picked up by this thread

        pthread_mutex_unlock(&q_mutex);
        logSyncMech(*(int *)arg, n_array, SYNC_OP_MUTEX_UNLOCK,
                    SYNC_ROLE_CONSUMER, request.value.header.account_id);

        // Handles the request
        reply.value.header.account_id = request.value.header.account_id;
        reply.value.header.ret_code = RC_OK;
        reply.length = sizeof(rep_header_t);
        reply.type = request.type;

        int index;

        if ((index = log_in(&db, request.value.header.account_id, request.value.header.password)) != -1) {
            logSyncDelay(*(int *)arg, n_array, request.value.header.account_id, request.value.header.op_delay_ms * 1000);
            usleep(request.value.header.op_delay_ms * 1000);

            int op = (int)request.type;
            bank_account_t acc;

            switch (op) {
                case 0:  // CREATE
                    if (request.value.header.account_id != 0) { reply.value.header.ret_code = RC_OP_NALLOW; break; }

                    int tmp_index;
                    if ((tmp_index = get_account(request.value.create.account_id, &db, *(int *)arg, n_array, request.value.header.account_id)) != -1) {
                        reply.value.header.ret_code = RC_ID_IN_USE;
                        unlock_account(tmp_index); break;
                    }

                    create_account(&acc, request.value.create.password, request.value.create.account_id, request.value.create.balance, &reply, *(int *)arg);

                    if (add_account(acc, &db, *(int *)arg, n_array, request.value.header.account_id))) { reply.value.header.ret_code = RC_OTHER; break; }

                    break;
                case 1:  // CHECK BALANCE
                    if (request.value.header.account_id == 0) { reply.value.header.ret_code = RC_OP_NALLOW; break; }
                    acc = db.dataBaseArray[index];
                    check_balance(&acc, &reply);
                    break;
                case 2:  // TRANSFER
                    if (request.value.header.account_id == 0) { reply.value.header.ret_code = RC_OP_NALLOW; break; }
                    transfer(index, request, &reply, *(int *)arg, request.value.header.op_delay_ms * 1000);
                    break;
                case 3:  // SHUTDOWN
                    if (request.value.header.account_id != 0) { reply.value.header.ret_code = RC_OP_NALLOW; break; }
                    shutdown(&reply);
                    break;
                default:
                    break;
            }
            unlock_account(index);
        } else {
            reply.value.header.ret_code = RC_LOGIN_FAIL;
        }

        if (send_reply(request.value.header.pid, &reply) != RC_OK) { reply.value.header.ret_code = RC_USR_DOWN; logReply(STDOUT_FILENO, n_array, &reply); }

        logReply(*(int *)arg, n_array, &reply);

        sem_post(&b_off);
        sem_getvalue(&b_off, &value);
        logSyncMechSem(*(int *)arg, n_array, SYNC_OP_SEM_POST, SYNC_ROLE_CONSUMER, request.value.header.account_id, value);
    }

    return NULL;
}

int create_account(bank_account_t *account, char password[], int accound_id, int balance, tlv_reply_t *user_reply, int fd) {
    account->account_id = accound_id;
    account->balance = balance;

    creatSalt(account->salt);
    getHash(account->salt, password, account->hash);

    user_reply->type = OP_CREATE_ACCOUNT;
    user_reply->value.header.ret_code = RC_OK;

    logAccountCreation(fd, n_array, account);

    return 0;
}

int check_balance(bank_account_t *bank_account, tlv_reply_t *user_reply) {
    user_reply->type = OP_BALANCE;

    if (bank_account == NULL) { user_reply->value.header.ret_code = RC_ID_NOT_FOUND; return RC_ID_NOT_FOUND; }

    user_reply->value.balance.balance = bank_account->balance;
    user_reply->length += sizeof(rep_balance_t);
    user_reply->value.header.ret_code = RC_OK;

    return 0;
}

int transfer(int index_header, tlv_request_t user_request, tlv_reply_t *user_reply, int fd, int delay) {
    user_reply->type = OP_TRANSFER;

    int index = get_account(user_request.value.transfer.account_id, &db, *(int *)arg, n_array, request.value.header.account_id);

    if (index == -1) { user_reply->value.header.ret_code = RC_ID_NOT_FOUND; return RC_ID_NOT_FOUND; }

    logSyncDelay(fd, n_array, user_request.value.transfer.account_id, delay);
    usleep(delay);

    bank_account_t *bank_acc_dest = &(db.dataBaseArray[index]);

    bank_account_t *bank_acc_orig = &(db.dataBaseArray[index_header]);

    int amount = user_request.value.transfer.amount;

    user_reply->value.transfer.balance = bank_acc_orig->balance;
    user_reply->length += sizeof(rep_transfer_t);

    if ((bank_acc_orig->balance - amount) < MIN_BALANCE) {
        unlock_account(index);
        user_reply->value.header.ret_code = RC_NO_FUNDS;
        return RC_NO_FUNDS;
    }

    if ((bank_acc_dest->balance + amount) > MAX_BALANCE) {
        unlock_account(index);
        user_reply->value.header.ret_code = RC_TOO_HIGH;
        return RC_TOO_HIGH;
    }

    bank_acc_orig->balance -= amount;
    bank_acc_dest->balance += amount;

    unlock_account(index, fd, n_array, request.value.header.account_id));

    user_reply->value.transfer.balance = bank_acc_orig->balance;
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