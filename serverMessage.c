#include "serverMessage.h"

int replyMessageTLV(tlv_request_t *user_request, tlv_reply_t *user_reply) {
    //Preparing values
    uint32_t length = 0;
    uint32_t account_id;
    uint32_t type;
    uint32_t ret_code;

    account_id = user_request->value.header.account_id;
    length += sizeof(account_id);




    //TO BE CONTINUED


    return RC_OK;
}