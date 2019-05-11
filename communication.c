#include "communication.h"
#include "serverMessage.h"

int get_request(tlv_request_t *request) {
    int fdr;

    // Open FIFO
    if ((fdr = open(SERVER_FIFO_PATH, O_WRONLY)) < 0) return RC_OTHER;

    // Send request
    read(fdr, &request, sizeof(request));

    // Closing FIFOS
    if (close(fdr)) return RC_OTHER;

    return RC_OK;
}

int get_reply(tlv_reply_t reply) {
    int fda;
    clock_t initial = clock();

    // Create FIFO
    char *fifo_reply = malloc(sizeof(USER_FIFO_PATH_PREFIX) + sizeof(getpid()));
    sprintf(fifo_reply, "%s%d", USER_FIFO_PATH_PREFIX, getpid());
    if (mkfifo(fifo_reply, 0666)) return RC_OTHER;

    // Open FIFO
    if ((fda = open(fifo_reply, O_RDONLY)) < 0) return RC_OTHER;

    // Receiving reply
    while (read(fda, &reply, sizeof(reply))) {
        if (((clock() - initial) / CLOCKS_PER_SEC) == FIFO_TIMEOUT_SECS) {
            printf("Action took too long...\n");
            close(fda);
            return RC_SRV_TIMEOUT;
        }
    }

    // Closing FIFOS
    if (close(fda)) return RC_OTHER;

    // Free memory
    unlink(fifo_reply);
    free(fifo_reply);

    return RC_OK;
}

int send_request(tlv_request_t request) {
    int fdr;

    // Open FIFO
    if ((fdr = open(SERVER_FIFO_PATH, O_WRONLY)) < 0) return RC_OTHER;

    // Send request
    write(fdr, &request, sizeof(request));

    // Closing FIFOS
    if (close(fdr)) return RC_OTHER;

    return RC_OK;
}