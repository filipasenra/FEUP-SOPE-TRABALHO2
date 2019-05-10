#include "communication.h"

int setCommunication(tlv_request_t *user_request, tlv_reply_t *user_reply) {
    //Create FIFO to receive answer
    char *fifo_receive = malloc(sizeof(USER_FIFO_PATH_PREFIX) + sizeof(getpid()));
    sprintf(fifo_receive, "%s%d", USER_FIFO_PATH_PREFIX, getpid());
    mkfifo(fifo_receive, 0444);

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
    
    //Receiving answer
    clock_t initial = clock();
    while(read(fda, &user_reply, sizeof(user_reply))) {
        if (((clock()-initial)/CLOCKS_PER_SEC) == FIFO_TIMEOUT_SECS) {
            printf("Action took too long...\n");
            break;
        }
    }

    if (close(fdr) || close(fda))
        return RC_OTHER;

    return RC_OK;
}