#pragma once

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "sope.h"
#include "types.h"

int setCommunication(tlv_request_t *user_request, tlv_reply_t *user_reply);

int sendReply(tlv_request_t *user_request, tlv_reply_t *user_reply);