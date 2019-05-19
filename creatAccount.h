#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "types.h"


/**
 * @brief Creates an account
 * 
 * @param account Pointer to the new account
 * @param password Password to the new account
 * @param account_id Account_id of the new account
 * @param balance Inicial balance
 * 
 * @return Returns returns zero upon success, non-zero otherwise
*/
int createAccount(bank_account_t *account, char password[], int accound_id, int balance);


/**
 * @brief Creates a hash
 * 
 * @return Returns zero upon sucess, non-zero otherwise
*/
int getHash(char salt[SALT_LEN + 1], char password[], char hash[HASH_LEN + 1]);

/**
 * @brief Creates a salt
 * 
 * @return Returns zero upon sucess, non-zero otherwise
*/
int creatSalt(char salt[SALT_LEN + 1]);