// User Program

#include "userMessage.h"

int main(int argc, char *argv[]) {
    tlv_request_t user_request;
    tlv_reply_t user_reply;
    pthread_t t;
    int comp = 0;

    // CREATE FIFO REPLY
    char *fifo_reply = malloc(sizeof(USER_FIFO_PATH_PREFIX) + sizeof(pid_t));
    sprintf(fifo_reply, "%s%d", USER_FIFO_PATH_PREFIX, getpid());
    if (mkfifo(fifo_reply, 0666)) return RC_OTHER;

    // MAKE REQUEST
    if (requestMessageTLV(argc, argv, &user_request)) return RC_OTHER;
    
    // SEND REQUEST
    if (send_request(&user_request)) return RC_OTHER;

    // START TIME
    clock_t initial = clock();

    // RECEIVE REPLY
    thread_arg_t thread_arg;
    thread_arg.pid = getpid();
    thread_arg.reply = &user_reply;
    thread_arg.completed = &comp;

    pthread_create(&t, NULL, get_reply_thread, (void *)(&thread_arg));

    while (1) {
        if (((clock() - initial) / CLOCKS_PER_SEC) == FIFO_TIMEOUT_SECS) {
            printf("Action took too long...\n");
            pthread_kill(t, SIGTERM);
            return RC_SRV_TIMEOUT;
        }
        if (comp) break;
    }

    // READ REPLY
    printf("account id %d\n", thread_arg.reply->value.header.account_id);

    return RC_OK;
}