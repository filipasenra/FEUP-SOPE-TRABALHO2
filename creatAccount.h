#pragma once

#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int creatAccount(bank_account_t *account, char password[], int accound_id, int balance);
int getHash(char salt[SALT_LEN + 1], char password[], char hash[HASH_LEN + 1]);
int creatSalt(char salt[SALT_LEN + 1]);