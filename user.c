//User Program

#include "types.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int createAccount(req_create_account_t *create, char argv[]);
int transferOperation(req_transfer_t *transfer, char argv[]);

int requestMessageTLV(int argc, char *argv[], tlv_request_t *user_request)
{

    if (argc != 6)
    {
        printf("Usage: ./user <count_id> \"<password>\" <delay_mils> <type_operation> \"<arguments_of_operations>\"\n");
        return 1;
    }

    req_header_t red_header;
    req_value_t req_value;

    user_request->value.header.account_id = strtoul(argv[1], NULL, 10);
    user_request->value.header.op_delay_ms = strtoul(argv[3], NULL, 10);

    //password too long
    if (strlen(argv[2]) > MAX_PASSWORD_LEN + 1)
    {
        printf("Password too long");
        return RC_OTHER;
    }

    strcpy(user_request->value.header.password, argv[2]);

    //TYPE OF OPERATION
    //if type of is invalid

    int n_type = strtoul(argv[4], NULL, 10);

    switch (n_type)
    {
    case 0:
        user_request->type = OP_CREATE_ACCOUNT;
        if (user_request->value.header.account_id != 0)
            return RC_OP_NALLOW;

        createAccount(&user_request->value.create, argv[5]);

        break;

    case 1:
        user_request->type = OP_BALANCE;
        if (user_request->value.header.account_id == 0)
            return RC_OP_NALLOW;
        break;

    case 2:
        user_request->type = OP_TRANSFER;
        if (user_request->value.header.account_id == 0)
            return RC_OP_NALLOW;

        transferOperation(&user_request->value.transfer, argv[5]);

        break;

    case 3:
        user_request->type = OP_SHUTDOWN;
        if (user_request->value.header.account_id != 0)
            return RC_OP_NALLOW;
        break;

    default:
        printf("Invalid Type");
        return RC_OTHER;
    }

    //==============================================

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
        {
            create->account_id = strtoul(token, NULL, 10);
        }
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
        printf("Error in create Account arguments\n");
        return -1;
    }

    return 0;
}

int transferOperation(req_transfer_t *transfer, char argv[])
{
    char *token;
    int n = 0;

    /* get the first token */
    token = strtok(argv, " ");

    /* walk through other tokens */
    while (token != NULL)
    {
        if (n == 0)
        {
            transfer->account_id = strtoul(token, NULL, 10);
            printf("heelo%d\n", transfer->account_id);
        }
        else if (n == 1)
            transfer->amount = strtoul(token, NULL, 10);
        else
            break;

        token = strtok(NULL, " ");
        n++;
    }

    if (n != 2)
    {
        printf("Error in transfer Operation arguments\n");
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    //Request to be sent to the fifo of the server
    tlv_request_t user_request;

    requestMessageTLV(argc, argv, &user_request);


    //Testing Area
    printf("Account_id: %d\n", user_request.value.header.account_id);
    printf("Op_delay_ms: %d\n", user_request.value.header.op_delay_ms);
    printf("Password: %s\n", user_request.value.header.password);
    printf("Type: %d\n", user_request.type);

    if (user_request.type == OP_CREATE_ACCOUNT)
    {
        printf("Create Accound_id: %d\n", user_request.value.create.account_id);
        printf("Create Balance: %d\n", user_request.value.create.balance);
        printf("Create password: %s\n", user_request.value.create.password);
    }
    else if (user_request.type == OP_TRANSFER)
    {
        printf("Create Accound_id: %d\n", user_request.value.transfer.account_id);
        printf("Create Balance: %d\n", user_request.value.transfer.amount);
    }

    return 0;
}