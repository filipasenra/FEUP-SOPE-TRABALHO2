#include "communication.h"

int send_request(tlv_request_t *user_request) {
    int fdr;
    if ((fdr = open(SERVER_FIFO_PATH, O_WRONLY)) < 0) return RC_OTHER;

    if (write(fdr, user_request, sizeof(tlv_request_t)) <= 0) {
        perror("send_request\n");
        return RC_OTHER;
    }

    if (close(fdr) != 0) return RC_OTHER;

    return RC_OK;
}

int get_request(tlv_request_t *user_request) {
    int fdr;
    if ((fdr = open(SERVER_FIFO_PATH, O_RDONLY)) < 0) return RC_OTHER;

    if (read(fdr, user_request, sizeof(tlv_request_t)) <= 0) {
        perror("get_request");
        return RC_OTHER;
    }

    if (close(fdr) != 0) return RC_OTHER;

    return RC_OK;
}

int send_reply(tlv_request_t *user_request, tlv_reply_t *user_reply) {
    int fda;
    char *fifo_send = malloc(sizeof(USER_FIFO_PATH_PREFIX) +
                             sizeof(user_request->value.header.pid));
    sprintf(fifo_send, "%s%d", USER_FIFO_PATH_PREFIX,
            user_request->value.header.pid);

    if ((fda = open(fifo_send, O_WRONLY)) < 0) return RC_OTHER;

    if (write(fda, user_reply, sizeof(tlv_reply_t)) <= 0) {
        perror("send_reply");
        return RC_OTHER;
    }

    if (close(fda) != 0) return RC_OTHER;

    return RC_OK;
}

int get_reply(tlv_reply_t *user_reply, char *fifo_reply, int fda) {
    int n;
    while ((n = read(fda, user_reply, sizeof(tlv_reply_t))) == 0)
        ;

    if (n < 0) {
        perror("get_reply");
        return -1;
    }

    // Closing FIFOS
    if (close(fda) != 0) return RC_OTHER;

    // Free memory
    unlink(fifo_reply);
    free(fifo_reply);

    return RC_OK;
}

void *get_reply_thread(void *arg) {
    int fda;
    thread_arg_t *thread_arg = (thread_arg_t *)arg;

    char *fifo_reply = malloc(sizeof(USER_FIFO_PATH_PREFIX) + sizeof(pid_t));
    sprintf(fifo_reply, "%s%d", USER_FIFO_PATH_PREFIX, reply->pid);

    if ((fda = open())) {
        *(thread_arg->completed) = 1;
        return RC_SRV_DOWN;
    }

    if (read(fda, thread_arg->reply, sizeof(tlv_reply_t))) {
        perror("get_reply");
        *(thread_arg->completed) = 1;
        return RC_OTHER;
    }

    // Closing FIFOS
    if (close(fda) != 0) {
        *(thread_arg->completed) = 1;
        return RC_OTHER;
    }

    unlink(fifo_reply);
    free(fifo_reply);

    *(thread_arg->completed) = 1;
    return RC_OK;
}