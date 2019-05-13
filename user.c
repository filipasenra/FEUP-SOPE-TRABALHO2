// User Program

#include "userMessage.h"

int main(int argc, char *argv[])
{
    tlv_request_t user_request;
    tlv_reply_t user_reply;
    int fda;

    // CREATE FIFO REPLY
    char *fifo_reply = malloc(sizeof(USER_FIFO_PATH_PREFIX) + sizeof(getpid()));
    sprintf(fifo_reply, "%s%d", USER_FIFO_PATH_PREFIX, getpid());
    if (mkfifo(fifo_reply, 0666))
        return RC_OTHER;

    // MAKE REQUEST
    if (requestMessageTLV(argc, argv, &user_request))
        return RC_OTHER;

    // SEND REQUEST
    if (send_request(&user_request))
        return RC_OTHER;

    //começar o tempo

    clock_t initial = clock();
    while (1)
    {
        if (((clock() - initial) / CLOCKS_PER_SEC) == FIFO_TIMEOUT_SECS)
        {
            printf("Action took too long...\n");
            close(fda);
            return RC_SRV_TIMEOUT;
        }

        fda = open(fifo_reply, O_RDONLY | O_NONBLOCK);
        printf("%d\n", fda);
        if(fda >= 0)
            break;
    }

    // RECEIVE REPLY
    if (get_reply(&user_reply, fifo_reply, fda))
        return RC_OTHER;

    // TODO: get_reply on separate thread and count time on main thread

    // READ REPLY
    printf("%d\n", user_reply.value.header.account_id);

    return RC_OK;
}