//User Program

#include "types.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int createAccount(req_create_account_t *create, char argv[]);

int requestMessageTLV(int argc, char *argv[], tlv_request_t *user_request)
{

    if (argc != 6)
    {
        printf("Usage: ./user <count_id> \"<password>\" <delay_mils> <type_operation> \"<arguments_of_operations>\"\n");
        return 1;
    }

    //Structs that we need
    tlv_request_t tlv_request;
    req_header_t red_header;
    req_value_t req_value;

    red_header.account_id = strtoul(argv[1], NULL, 10);
    red_header.op_delay_ms = strtoul(argv[3], NULL, 10);

    //password too long
    if (strlen(argv[2]) > MAX_PASSWORD_LEN + 1)
    {
        printf("Password too long");
        return RC_OTHER;
    }

    strcpy(red_header.password, argv[2]);

    //TYPE OF OPERATION
    //if type of is invalid

    int n_type = strtoul(argv[4], NULL, 10);

    switch (n_type)
    {
    case 0:
        tlv_request.type = OP_CREATE_ACCOUNT;
        if (red_header.account_id != 0)
            return RC_OP_NALLOW;
        break;

    case 1:
        tlv_request.type = OP_BALANCE;
        if (red_header.account_id == 0)
            return RC_OP_NALLOW;
        break;

    case 2:
        tlv_request.type = OP_TRANSFER;
        if (red_header.account_id == 0)
            return RC_OP_NALLOW;
        break;

    case 3:
        tlv_request.type = OP_SHUTDOWN;
        if (red_header.account_id != 0)
            return RC_OP_NALLOW;
        break;

    default:
        printf("Invalid Type");
        return RC_OTHER;
    }

    //==============================================

    if (tlv_request.type == OP_CREATE_ACCOUNT)
    {
        createAccount(&req_value.create, argv[5]);
    }

    return 0;
}

int createAccount(req_create_account_t *create, char argv[])
{
    char *token;
    int n = 0;

    /* get the first token */
    token = strtok(argv, " ");

    /* walk through other tokens */
    while (token != NULL)
    {
        if (n == 0)
            create->account_id = strtoul(token, NULL, 10);
        else if (n == 1)
            create->balance = strtoul(token, NULL, 10);
        else if (n == 2)
            strcpy(create->password, token);
        else
            break;

        token = strtok(NULL, " ");
        n++;
    }

    if (n != 3)
    {
        printf("Error in create Account arguments");
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    //Request to be sent to the fifo of the server
    tlv_request_t user_request;

    return 0;
}