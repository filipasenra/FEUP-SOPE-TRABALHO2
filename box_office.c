#include "box_office.h"

int n_array = -1;

void *box_office(void *arg) {
    tlv_request_t request;
    tlv_reply_t reply;
    int fd_log = *(int*)arg;

    for (int n = 0; n < number_threads; n++) {
        if (thread_array[n] == pthread_self()) {
            n_array = n + 1;
            break;
        }
    }

    while (1) {
        if(server_stdw && isEmpty(&queue)){
            return NULL;
        }

        sem_post(&n_req);
        int value = -1;
        sem_getvalue(&b_off, &value);
        logSyncMechSem(fd_log, n_array, SYNC_OP_SEM_WAIT, SYNC_ROLE_CONSUMER, request.value.header.account_id, value);

        logSyncMech(fd_log, n_array, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_CONSUMER, request.value.header.account_id);
        pthread_mutex_lock(&q_mutex);

		if(!isEmpty(queue)){
            request = front(queue);  // Gets the request that arrived first
            pop(&queue);  // Updates the queue and 'frees' the space ocupided by the request picked up by this thread}
        }
        
        logSyncMech(fd_log, n_array, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, request.value.header.account_id);
        pthread_mutex_unlock(&q_mutex);

        // Handles the request
        reply.value.header.account_id = request.value.header.account_id;
        reply.value.header.ret_code = RC_OK;
        reply.length = sizeof(rep_header_t);
        reply.type = request.type;

        int index;
		write(STDERR_FILENO, "REQ\n", 4);
        if ((index = log_in(&db, request.value.header.account_id, request.value.header.password)) != -1) {
            lock_account(index, fd_log, request);
            logSyncDelay(fd_log, n_array, request.value.header.account_id, request.value.header.op_delay_ms * 1000);
            usleep(request.value.header.op_delay_ms * 1000);
            int op = (int)request.type;
            bank_account_t acc;
            int new_index;

            switch (op) {
                case 0:  // CREATE
                    if (request.value.header.account_id != 0) { reply.value.header.ret_code = RC_OP_NALLOW; break; }
                    if (get_account(request.value.create.account_id, &db) != -1) { reply.value.header.ret_code = RC_ID_IN_USE; break; }
                    new_index = db.last_element;
                    lock_account(new_index, fd_log, request);
                    create_account(&acc, request.value.create.password, request.value.create.account_id, request.value.create.balance, &reply, fd_log);
                    if (add_account(acc, &db)) { reply.value.header.ret_code = RC_OTHER; unlock_account(new_index, fd_log, request); break; }
                    unlock_account(new_index, fd_log, request);
                    break;
                case 1:  // CHECK BALANCE
                    if (request.value.header.account_id == 0) { reply.value.header.ret_code = RC_OP_NALLOW; break; }
                    acc = db.dataBaseArray[index];
                    check_balance(&acc, &reply);
                    break;
                case 2:  // TRANSFER
                    if (request.value.header.account_id == 0) { reply.value.header.ret_code = RC_OP_NALLOW; break; }
                    transfer(index, request, &reply, fd_log, request.value.header.op_delay_ms * 1000);
                    break;
                case 3:  // SHUTDOWN
                    if (request.value.header.account_id != 0) { reply.value.header.ret_code = RC_OP_NALLOW; break; }
                    shutdown(&reply);
                    break;
            }
                                                                                                                                                            write(STDERR_FILENO, "FINISH\n", 7);

            unlock_account(index, fd_log, request);
        } else { reply.value.header.ret_code = RC_LOGIN_FAIL; }

        if (send_reply(request.value.header.pid, &reply) != RC_OK) { reply.value.header.ret_code = RC_USR_DOWN; }
        logReply(fd_log, n_array, &reply);
        
        logSyncMechSem(fd_log, n_array, SYNC_OP_SEM_POST, SYNC_ROLE_CONSUMER, request.value.header.account_id, value);
        sem_wait(&b_off);
        sem_getvalue(&b_off, &value);
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

    if (bank_account == NULL) {
        user_reply->value.header.ret_code = RC_ID_NOT_FOUND;
        return RC_ID_NOT_FOUND;
    }

    user_reply->value.balance.balance = bank_account->balance;
    user_reply->length += sizeof(rep_balance_t);
    user_reply->value.header.ret_code = RC_OK;

    return 0;
}

int transfer(int index_header, tlv_request_t user_request, tlv_reply_t *user_reply, int fd, int delay) {
    user_reply->type = OP_TRANSFER;

    int index = get_account(user_request.value.transfer.account_id, &db);
    if (index == -1) { user_reply->value.header.ret_code = RC_ID_NOT_FOUND; return RC_ID_NOT_FOUND; }
    lock_account(index, fd, user_request);
    logSyncDelay(fd, n_array, user_request.value.transfer.account_id, delay);
    usleep(delay);
    bank_account_t *bank_acc_dest = &(db.dataBaseArray[index]);
    bank_account_t *bank_acc_orig = &(db.dataBaseArray[index_header]);

    int amount = user_request.value.transfer.amount;

    user_reply->value.transfer.balance = bank_acc_orig->balance;
    user_reply->length += sizeof(rep_transfer_t);

    if ((bank_acc_orig->balance - amount) < MIN_BALANCE) {
        unlock_account(index, fd, user_request);
        user_reply->value.header.ret_code = RC_NO_FUNDS;
        return RC_NO_FUNDS;
    } else if ((bank_acc_dest->balance + amount) > MAX_BALANCE) {
        unlock_account(index, fd, user_request);
        user_reply->value.header.ret_code = RC_TOO_HIGH;
        return RC_TOO_HIGH;
    }

    bank_acc_orig->balance -= amount;
    bank_acc_dest->balance += amount;

    unlock_account(index, fd, user_request);

    user_reply->value.transfer.balance = bank_acc_orig->balance;
    user_reply->value.header.ret_code = RC_OK;

    return 0;
}

void shutdown(tlv_reply_t *user_reply) {
    int value = 1;
    sem_getvalue(&b_off, &value);
    fchmod(server_fifo, 0444);
    server_stdw = 1;

    user_reply->type = OP_SHUTDOWN;
    user_reply->value.shutdown.active_offices = number_threads - value;
    user_reply->length += sizeof(rep_shutdown_t);
    user_reply->value.header.ret_code = RC_OK;
}

void lock_account(int index, int fd, tlv_request_t user_request){
    logSyncMech(fd, n_array, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_ACCOUNT, user_request.value.transfer.account_id);
    pthread_mutex_lock(&(db_mutex[index]));
}

void unlock_account(int index, int fd, tlv_request_t user_request){
    logSyncMech(fd, n_array, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_ACCOUNT, user_request.value.transfer.account_id);
    pthread_mutex_unlock(&(db_mutex[index]));
}
