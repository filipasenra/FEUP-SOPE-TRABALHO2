#include "types.h"
#include <stdlib.h>

typedef struct {
    bank_account_t * dataBaseArray;
    int size;
    int last_element;
} dataBase;

int initializeDataBase(dataBase *dataBase);

int addElement(bank_account_t bank_account, dataBase *dataBase);