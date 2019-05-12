// User Program

#include "userMessage.h"

int main(int argc, char *argv[])
{
    tlv_request_t user_request;
    tlv_reply_t user_reply;
    int fda;

    // Preparing request
    if (requestMessageTLV(argc, argv, &user_request))
        return RC_OTHER;

    // Create FIFO to receive reply
    char *fifo_reply = malloc(sizeof(USER_FIFO_PATH_PREFIX) + sizeof(getpid()));
    sprintf(fifo_reply, "%s%d", USER_FIFO_PATH_PREFIX, getpid());
    if (mkfifo(fifo_reply, 0666))
        return RC_OTHER;

    // Open FIFO to receive reply
    if ((fda = open(fifo_reply, O_RDONLY)) < 0)
        return RC_OTHER;

    // Sending request to secure_srv
    if (send_request(&user_request))
        return RC_OTHER;

    // Recieving reply
    if (get_reply(&user_reply, fifo_reply, fda))
        return RC_OTHER;

    // Reading reply

    return RC_OK;
}