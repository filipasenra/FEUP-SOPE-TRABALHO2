#include "userMessage.h"

int prepareMainArgs(char *argv[], tlv_request_t *user_request)
{
    // Preparing values
    uint32_t account_id;
    uint32_t delay;

    account_id = strtoul(argv[1], NULL, 10);

    delay = strtoul(argv[3], NULL, 10);

    //Verify passord
    if (strlen(argv[2]) > MAX_PASSWORD_LEN + 1)
    {
        printf("Password too long\n");
        return RC_OTHER;
    }
    else if (strlen(argv[2]) < MIN_PASSWORD_LEN)
    {
        printf("Password too short\n");
        return RC_OTHER;
    }

    // Passing to the struct
    strcpy(user_request->value.header.password, argv[2]);
    user_request->value.header.account_id = account_id;
    user_request->value.header.op_delay_ms = delay;
    user_request->value.header.pid = getpid();
    user_request->length = sizeof(req_header_t);

    return RC_OK;
}

int prepareTypeOfOpArgs(char *argv[], tlv_request_t *user_request)
{
    // Preparing values
    uint32_t account_id = user_request->value.header.account_id;

    // TYPE OF OPERATION
    int n_type = strtoul(argv[4], NULL, 10);

    switch (n_type)
    {
    case 0:
        user_request->type = OP_CREATE_ACCOUNT;

        if (createAccountUser(&user_request->value.create, argv[5]) != 0)
            return RC_OTHER;

        user_request->length += sizeof(req_create_account_t);

        break;
    case 1:
        user_request->type = OP_BALANCE;
        break;

    case 2:
        user_request->type = OP_TRANSFER;

        int n;
        if ((n = transferOperation(
                 account_id, &user_request->value.transfer, argv[5])) != 0)
            return n;

        user_request->length += sizeof(req_transfer_t);

        break;
    case 3:

        user_request->type = OP_SHUTDOWN;
        
        break;
    default:

        printf("Invalid Type\n");
        return RC_OTHER;
    }

    return RC_OK;
}

int requestMessageTLV(int argc, char *argv[], tlv_request_t *user_request)
{
    // Checking the number of args
    if (argc != 6)
    {
        printf(
            "Usage: ./user <count_id> \"<password>\" <delay_mils> "
            "<type_operation> \"<arguments_of_operations>\"\n");
        return 1;
    }

    // Preparing main arguments request
    if (prepareMainArgs(argv, user_request))
        return RC_OTHER;

    // Preparing type of operation arguments request
    if (prepareTypeOfOpArgs(argv, user_request))
    {
        printf("Error in type of Op Arguments!\n");
        return RC_OTHER;
    }

    // Writing Log
    int fd = open(USER_LOGFILE, O_WRONLY | O_APPEND | O_CREAT, 0777);
    if (logRequest(fd, (int)getpid(), user_request) < 0)
    {
        printf("Failed to open and write into %s\n", USER_LOGFILE);
        return RC_OTHER;
    }
    
    return RC_OK;
}

int createAccountUser(req_create_account_t *create, char argv[])
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

int transferOperation(uint32_t idOrigin, req_transfer_t *transfer,
                      char argv[])
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
            if (idOrigin == strtoul(token, NULL, 10))
            {
                printf("Same Source and Destination account.\n");
                return RC_SAME_ID;
            }

            transfer->account_id = strtoul(token, NULL, 10);
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
        return RC_OTHER;
    }

    return 0;
}
