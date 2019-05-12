#pragma once

#include <stdlib.h>
#include "types.h"

typedef struct dataBase {
    bank_account_t *dataBaseArray;
    int size;
    int last_element;
} dataBase_t;

int initializeDataBase(dataBase_t *dataBase);

int addAccount(bank_account_t bank_account, dataBase_t *dataBase);