#pragma once

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "sope.h"
#include "types.h"

extern int server_stdw;

/**
 * @brief Sends a request to the server
 * 
 * @returns zero upon success, non-zero otherwise
*/
int send_request(tlv_request_t *request);

/**
 * @brief Gets a request from the user
 * 
 * @param user_request Where the request from the user is to be safed
 * @param fd_log File descriptor to the log file
 * @param fd_srv File descriptor to the FIFO of the server
 * 
 * @returns zero upon success, non-zero otherwise
*/
int get_request(tlv_request_t *user_request, int fd_log, int fd_srv);

/**
 * @brief Sends a request to the user
 * 
 * @returns zero upon success, non-zero otherwise
*/
int send_reply(pid_t pid, tlv_reply_t *user_reply);

/**
 * Struct to be used by the thread that gets the reply from the server
*/
typedef struct thread_arg{
    pid_t pid;
    tlv_reply_t reply;
    int completed;
} thread_arg_t;

/**
 * @brief Gets the reply from the server
*/
void *get_reply_thread(void *arg);
