#include "serverMessage.h"

int replyMessageTLV(tlv_request_t *user_request, tlv_reply_t *user_reply, dataBase_t *dataBase) {
    //Preparing values
    uint32_t length = 0;
    uint32_t account_id;
    bank_account_t account;
    
    //Account id
    account_id = user_request->value.header.account_id;
    length += sizeof(account_id);

    //Type of operation
    user_reply->type = user_request->type;
    length += sizeof(user_reply->type);
    //Passing values
    switch (user_reply->type)
    {
    case 0:
        if (account_id != 0)
            user_reply->value.header.ret_code = RC_OP_NALLOW;
        break;
    case 1:
        if (account_id == 0)
            user_reply->value.header.ret_code = RC_OP_NALLOW;
        else if (findAccount(account_id, &account, dataBase))
                user_reply->value.header.ret_code = RC_ID_NOT_FOUND;
        else {
            user_reply->value.balance.balance = account.balance;
            length += sizeof(user_reply->value.balance.balance);
        }
        break;
    case 2:
    //TODO
        break;
    case 3:
    //TODO
        break;
    case 4:
    //TODO
        break;
    
    default:
    //TODO
        break;
    }


    //TO BE CONTINUED
    user_reply->length = length;
    if (user_reply->value.header.ret_code != RC_OP_NALLOW)
        user_reply->value.header.ret_code = RC_OK;

    return RC_OK;
}

int findAccount(int id, bank_account_t *account, dataBase_t *dataBase) {
    for (int i = 0; i < dataBase->size; i++)
        if (dataBase->dataBaseArray[i].account_id == id) {
            *account = dataBase->dataBaseArray[i];
            return RC_OTHER;
        }
    
    RC_OK;
}