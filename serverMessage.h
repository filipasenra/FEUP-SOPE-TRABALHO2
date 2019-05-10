#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "sope.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include "types.h"
#include "sope.h"
#include "types.h"
#include "dataBase.h"
#include "box_office.h"
#include "creatAccount.h"
#include "communication.h"

int replyMessageTLV(tlv_request_t *user_request, tlv_reply_t *user_reply);