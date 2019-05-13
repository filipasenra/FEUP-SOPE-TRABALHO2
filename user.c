// User Program

#include "userMessage.h"

int main(int argc, char *argv[]){
    tlv_request_t user_request;
    tlv_reply_t user_reply;
    int fda;

    // CREATE FIFO REPLY
    char *fifo_reply = malloc(sizeof(USER_FIFO_PATH_PREFIX) + sizeof(getpid()));
    sprintf(fifo_reply, "%s%d", USER_FIFO_PATH_PREFIX, getpid());
    if (mkfifo(fifo_reply, 0666)) return RC_OTHER;

    // MAKE REQUEST
    if (requestMessageTLV(argc, argv, &user_request)) return RC_OTHER;

    // SEND REQUEST
    if (send_request(&user_request)) return RC_OTHER;

    fda = open(fifo_reply, O_RDONLY);

    // RECEIVE REPLY
    if (get_reply(&user_reply, fifo_reply, fda)) return RC_OTHER;

    // TODO: get_reply on separate thread and count time on main thread

    // READ REPLY
    printf("%d\n", user_reply.value.header.account_id);

    return RC_OK;
}