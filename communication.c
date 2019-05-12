#include "communication.h"
#include "serverMessage.h"

int send_request(tlv_request_t *request)
{
    int fdr;
    clock_t initial = clock();

    // Open FIFO
    if ((fdr = open(SERVER_FIFO_PATH, O_WRONLY)) < 0)
        return RC_OTHER;

    // Send request
    write(fdr, &request, sizeof(request));

    // Closing FIFOS
    if (close(fdr))
        return RC_OTHER;

    return RC_OK;
}

int get_request(tlv_request_t *user_request)
{
    //Create FIFO to receive request
    int fdr;
    mkfifo(SERVER_FIFO_PATH, 0666);

    // Open FIFO to receive request
    if ((fdr = open(SERVER_FIFO_PATH, O_RDONLY)) < 0)
        return RC_OTHER;

    // Receiving request
    read(fdr, user_request, sizeof(user_request));

    return RC_OK;
}

int send_reply(tlv_request_t *user_request, tlv_reply_t *user_reply)
{
    // Open FIFO to send reply
    int fda;
    char *fifo_send = malloc(sizeof(USER_FIFO_PATH_PREFIX) + sizeof(user_request->value.header.pid));
    if ((fda = open(user_request->value.header.pid, O_WRONLY)) < 0)
        return RC_OTHER;

    // Sending reply
    write(fda, user_reply, sizeof(user_reply));

    // Free memory
    free(fifo_send);

    return RC_OK;
}

int get_reply(tlv_reply_t *reply)
{
    int fda;
    clock_t initial = clock();

    // Create FIFO
    char *fifo_reply = malloc(sizeof(USER_FIFO_PATH_PREFIX) + sizeof(getpid()));
    sprintf(fifo_reply, "%s%d", USER_FIFO_PATH_PREFIX, getpid());
    if (mkfifo(fifo_reply, 0666))
        return NULL;

    // Open FIFO
    if ((fda = open(fifo_reply, O_RDONLY)) < 0)
        return RC_OTHER;

    // Receiving reply
    while (read(fda, &reply, sizeof(reply)))
    {
        if (((clock() - initial) / CLOCKS_PER_SEC) == FIFO_TIMEOUT_SECS)
        {
            printf("Action took too long...\n");
            close(fda);
            return RC_SRV_TIMEOUT;
        }
    }

    // Closing FIFOS
    if (close(fda))
        return RC_OTHER;

    // Free memory
    unlink(fifo_reply);
    free(fifo_reply);

    return RC_OK;
}