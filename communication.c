#include "communication.h"

int send_request(tlv_request_t *user_request)
{       
    int fdr;
    if ((fdr = open(SERVER_FIFO_PATH, O_WRONLY)) < 0)
    {
        perror("send_request");
        return RC_OTHER;
    }

    if (write(fdr, user_request, sizeof(op_type_t) + sizeof(uint32_t) + user_request->length) <= 0)
    {
        perror("send_request\n");
        return RC_OTHER;
    }
    if (close(fdr) != 0)
        return RC_OTHER;

    return RC_OK;
}

int get_request(tlv_request_t *user_request, int fd_log, int fd_srv)
{   
    int n = 0;
    while ((n = read(fd_srv, &(user_request->type), sizeof(enum op_type))) == 0)
        ;

    if (n < 0)
    {
        perror("get_request");
        return RC_OTHER;
    }

    if (read(fd_srv, &(user_request->length), sizeof(uint32_t)) <= 0)
    {
        perror("get_request");
        return RC_OTHER;
    }

    if (read(fd_srv, &(user_request->value), user_request->length) <= 0)
    {
        perror("get_request");
        return RC_OTHER;
    }

    if (logRequest(fd_log, pthread_self(), user_request) < 0)
    {
        printf("Failed to open and write into %s\n", SERVER_LOGFILE);
        return RC_OTHER;
    }

    return RC_OK;
}

int send_reply(tlv_request_t *user_request, tlv_reply_t *user_reply)
{
    int fda;
    char *fifo_send = malloc(sizeof(USER_FIFO_PATH_PREFIX) +
                             sizeof(user_request->value.header.pid));
    sprintf(fifo_send, "%s%d", USER_FIFO_PATH_PREFIX,
            user_request->value.header.pid);

    if ((fda = open(fifo_send, O_WRONLY | O_NONBLOCK)) < 0)
        return RC_OTHER;

    if (write(fda, user_reply, sizeof(op_type_t) + sizeof(uint32_t) + user_request->length) <= 0)
    {
        perror("send_reply");
        return RC_OTHER;
    }

    if (close(fda) != 0)
        return RC_OTHER;

    free(fifo_send);

    return RC_OK;
}

void *get_reply_thread(void *arg)
{
    int fda;
    thread_arg_t *thread_arg = (thread_arg_t *)arg;

    char *fifo_reply = malloc(sizeof(USER_FIFO_PATH_PREFIX) + sizeof(pid_t));
    sprintf(fifo_reply, "%s%d", USER_FIFO_PATH_PREFIX, thread_arg->pid);

    fda = open(fifo_reply, O_RDONLY);

    if (read(fda, &(thread_arg->reply.type), sizeof(enum op_type)) != sizeof(enum op_type))
    {
        perror("get_reply_thread type");
        return (void *)RC_OTHER;
    }

    if (read(fda, &(thread_arg->reply.length), sizeof(uint32_t)) != sizeof(uint32_t))
    {
        perror("get_reply_thread lenght");
        return (void *)RC_OTHER;
    }

    if (read(fda, &(thread_arg->reply.value), thread_arg->reply.length) != thread_arg->reply.length)
    {
        perror("get_reply_thread value");
        return (void *)RC_OTHER;
    }

    // Closing FIFOS
    if (close(fda) != 0)
    {
        thread_arg->completed = 1;
        return (void *)RC_OTHER;
    }

    unlink(fifo_reply);
    free(fifo_reply);

    thread_arg->completed = 1;
    return (void *)RC_OK;
}
