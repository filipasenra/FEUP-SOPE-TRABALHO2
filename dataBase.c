#include "dataBase.h"

int initializeDataBase(dataBase_t *dataBase)
{
    dataBase->size = 20;

    if ((dataBase->dataBaseArray =
             calloc(sizeof(bank_account_t), dataBase->size)) == NULL)
        return RC_OTHER;

    dataBase->last_element = -1;

    return RC_OK;
}

int addAccount(bank_account_t bank_account, dataBase_t *dataBase)
{
    dataBase->last_element++;

    if (dataBase->size <= dataBase->last_element)
    {
        if ((dataBase->dataBaseArray = realloc(dataBase->dataBaseArray, dataBase->size + 20)) == NULL)
            return RC_OTHER;
    }

    dataBase->dataBaseArray[dataBase->last_element] = bank_account;

    return RC_OK;
}

bank_account_t * accountExist(int account_id, dataBase_t *dataBase)
{
    for (int i = 0; i < dataBase->size; i++)
    {
        bank_account_t acc = dataBase->dataBaseArray[i];
        if (acc.account_id == account_id)
        {
            return &dataBase->dataBaseArray[i];
        }
    }

    return NULL;
}

int freeDataBase(dataBase_t *dataBase){

    free(dataBase->dataBaseArray);

    return 0;
}

int repeatedAccount(uint32_t account_id, dataBase_t dataBase) {
    for (int i = 0; i < dataBase.size; i++) {
        if (account_id == dataBase.dataBaseArray[i].account_id)
            return RC_ID_IN_USE;
    }

    return RC_OK;
}