#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "box_office.h"
#include "communication.h"
#include "creatAccount.h"
#include "dataBase.h"
#include "sope.h"
#include "types.h"

int replyMessageTLV(tlv_request_t *user_request, tlv_reply_t *user_reply,
                    dataBase_t *dataBase);

int findAccount(int id, bank_account_t *account, dataBase_t *dataBase);