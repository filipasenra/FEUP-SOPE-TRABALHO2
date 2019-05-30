#pragma once

#include <stdlib.h>
#include "types.h"
#include <string.h>
#include "creatAccount.h"

extern pthread_mutex_t db_mutex[MAX_BANK_ACCOUNTS];

/**
 * Represents a dataBase
*/
typedef struct dataBase {
    bank_account_t dataBaseArray[MAX_BANK_ACCOUNTS];
    int last_element;
} dataBase_t;


/**
 * @Brief Inicializes a Data Base
*/
void init_database(dataBase_t *dataBase);

/**
 * @Brief Adds an account to a Data Base
 * 
 * @return Returns zero upon sucess, non-zero otherwise
*/
int add_account(bank_account_t bank_account, dataBase_t *dataBase, int fd, int n_array, tlv_request_t user_request);

/**
 * @brief Gets the index of the account with the account_id given
 * 
 * @return Returns index upon sucess, -1 otherwise
*/
int get_account(int account_id, dataBase_t *dataBase, int fd, int n_array, tlv_request_t user_request);

/**
 * @brief Unlocks mutex of given account
 * @return Returns index
*/
int unlock_account(int index, int fd, int n_array, tlv_request_t user_request){


/**
 * @brief Log In of an account
 * 
 * @param db Pointer to the data base
 * @param account_id Account id to be loged in
 * @þaram password Password given by the user
 * 
 * @return Returns zero upon sucess (meaning the user has loged in), non-zero otherwise
*/
int log_in(dataBase_t *db, uint32_t account_id, char password[MAX_PASSWORD_LEN + 1]);