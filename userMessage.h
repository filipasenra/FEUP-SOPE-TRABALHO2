#pragma once

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include "communication.h"
#include "serverMessage.h"
#include "sope.h"
#include "types.h"

/**
 * @brief Puts into the struct create the information of the account to be
 * created
 *
 * @param create Pointer to a struct req_create_account_t
 * @param argv C-string with the information of the account (to be created)
 *
 * @returns zero upon sucess, non-zero otherwise
 */
int createAccount(req_create_account_t *create, char argv[]);

/**
 * @brief Puts into the struct transfer the information of the transfer
 *
 * @param transfer Pointer to a struct req_transfer_t
 * @param argv C-string with the information of the transfer
 *
 * @returns zero upon sucess, non-zero otherwise
 */
int transferOperation(uint32_t idOrigin, req_transfer_t *transfer, char argv[]);

/**
 * @brief Puts into the struct user_request the information of the request of
 * the user
 *
 * @param argc Size of the array argv
 * @param argv Paraments given by the user
 * @param user_request Pointer to a struct tlv_request_t
 *
 * @return zero upon sucess, non-zero otherwise
 */
int requestMessageTLV(int argc, char *argv[], tlv_request_t *user_request);

int prepareMainArgs(char *argv[], tlv_request_t *user_request);

int prepareTypeOfOpArgs(char *argv[], tlv_request_t *user_request);
