#include "communication.h"

int sendReply(tlv_request_t *user_request, tlv_reply_t *user_reply) {
    //Create FIFO to receive request
    int fdr;
    mkfifo(SERVER_FIFO_PATH, 0666);

    //Open FIFO to receive request
    if ((fdr = open(SERVER_FIFO_PATH, O_RDONLY)) < 0)
        return RC_OTHER;

    //Receiving request
    read(fdr, &user_request, sizeof(user_reply));

    //Preparing reply


    //Open FIFO to send reply
    int fda;
    char *fifo_send = malloc(sizeof(USER_FIFO_PATH_PREFIX) + sizeof(user_request->value.header.pid));
    if ((fda = open(user_request->value.header.pid, O_WRONLY)) < 0)
        return RC_OTHER;

    //Sending reply
    

}

int setCommunication(tlv_request_t *user_request, tlv_reply_t *user_reply) {
    //Create FIFO to receive reply
    char *fifo_receive = malloc(sizeof(USER_FIFO_PATH_PREFIX) + sizeof(getpid()));
    sprintf(fifo_receive, "%s%d", USER_FIFO_PATH_PREFIX, getpid());
    mkfifo(fifo_receive, 0666);

    //Open FIFO to send request
    int fdr;
    if ((fdr = open(SERVER_FIFO_PATH, O_WRONLY)) < 0)
        return RC_OTHER;

    //Sending request
    write(fdr, &user_request, sizeof(user_request));

    //Open FIFO to receive answer
    int fda;
    if ((fda = open(fifo_receive, O_RDONLY)) < 0)
        return RC_OTHER;
    
    //Receiving reply
    clock_t initial = clock();
    while(read(fda, &user_reply, sizeof(user_reply))) {
        if (((clock()-initial)/CLOCKS_PER_SEC) == FIFO_TIMEOUT_SECS) {
            printf("Action took too long...\n");
            close(fda);
            close(fdr);
            return RC_SRV_TIMEOUT;
        }
    }

    //Closing FIFOS
    if (close(fdr) || close(fda))
        return RC_OTHER;

    //Free memory
    unlink(fifo_receive);
    free(fifo_receive);

    return RC_OK;
}