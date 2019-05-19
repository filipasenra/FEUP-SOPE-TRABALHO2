#pragma once

#include <stdlib.h>
#include "types.h"

typedef struct dataBase {
    bank_account_t dataBaseArray[MAX_BANK_ACCOUNTS];
    int last_element;
} dataBase_t;

/* data base constructor 
 * */
void init_database(dataBase_t *dataBase);

/* add account to database and increment last_element
 * return 0 on success, non-zero otherwise
 * */
int add_account(bank_account_t bank_account, dataBase_t *dataBase);

/* get account index with account id
 * return account index on success, -1 otherwise
 * */
int get_account(int account_id, dataBase_t *dataBase);

/* verify account credentials
 * return index on success, -1 otherwise
 * */
int log_in(dataBase_t *db, uint32_t account_id, char password[MAX_PASSWORD_LEN + 1]);