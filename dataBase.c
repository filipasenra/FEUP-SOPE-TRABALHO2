#include "dataBase.h"
#include "creatAccount.h"

void init_database(dataBase_t *dataBase) { dataBase->last_element = -1; }

int add_account(bank_account_t bank_account, dataBase_t *dataBase, int fd, int n_array, tlv_request_t user_request)
{
    if (dataBase->last_element == MAX_BANK_ACCOUNTS) return 1;

    int index = dataBase->last_element++;
    logSyncMech(fd, n_array, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_ACCOUNT, user_request.value.transfer.account_id);
    pthread_mutex_lock(&(db_mutex[index]));
    dataBase->dataBaseArray[dataBase->last_element] = bank_account;
    logSyncMech(fd, n_array, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_ACCOUNT, user_request.value.transfer.account_id);
    pthread_mutex_unlock(&(db_mutex[index]));

    return 0;
}

int get_account(int account_id, dataBase_t *dataBase, int fd, int n_array, tlv_request_t user_request)
{
    for (int i = 0; i <= dataBase->last_element; i++)
    {
        bank_account_t acc = dataBase->dataBaseArray[i];
        if (acc.account_id == account_id)
        {
            logSyncMech(fd, n_array, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_ACCOUNT, user_request.value.transfer.account_id);
            pthread_mutex_lock(&(db_mutex[i]));
            return i;
        }
    }

    return -1;
}

int unlock_account(int index, int fd, int n_array, tlv_request_t user_request){
    logSyncMech(fd, n_array, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_ACCOUNT, user_request.value.transfer.account_id);
    pthread_mutex_unlock(&(db_mutex[index]));
    return index;
}

int log_in(dataBase_t *db, uint32_t account_id, char password[MAX_PASSWORD_LEN + 1])
{
    int index;
    if ((index = get_account(account_id, db)) == -1)
        return -1;

    bank_account_t acc = db->dataBaseArray[index];
    char hash[HASH_LEN + 1];
    getHash(acc.salt, password, hash);

    if (strcmp(acc.hash, hash) == 0)
        return index;

    return -1;
}