// User Program

#include "userMessage.h"
#include "communication.h"

int main(int argc, char *argv[]) {
    //Request to be sent to the fifo of the server
    tlv_request_t user_request;

    if (requestMessageTLV(argc, argv, &user_request))
        return RC_OTHER;

    // Testing Area
    /*printf("Account_id: %d\n", user_request.value.header.account_id);
    printf("Op_delay_ms: %d\n", user_request.value.header.op_delay_ms);
    printf("Password: %s\n", user_request.value.header.password);
    printf("Type: %d\n", user_request.type);
    printf("Length: %d\n", user_request.length);
    printf("Pid: %d\n", user_request.value.header.pid);

    if (user_request.type == OP_CREATE_ACCOUNT) {
        printf("Create Accound_id: %d\n", user_request.value.create.account_id);
        printf("Create Balance: %d\n", user_request.value.create.balance);
        printf("Create password: %s\n", user_request.value.create.password);
    } else if (user_request.type == OP_TRANSFER) {
        printf("Transfer Accound_id: %d\n", user_request.value.transfer.account_id);
        printf("Transfer Balance: %d\n", user_request.value.transfer.amount);
    }*/

    /*
     * MISSING PUTTING THIS INFORMATION ON THE FIFO "secure_srv" AND
     * RETRIVING THE RESPONSE OF THE SERVER IN THE FIFO "secure_<pid_of_this_process>"
     * */

    /*
     * MISSING DOING SOMETHING WITH THE REPLY
     * */

    //Reply sent by the server
    tlv_reply_t user_reply;
    
    if (setCommunication(&user_request, &user_reply))
        return RC_OTHER;

    return RC_OK;
}