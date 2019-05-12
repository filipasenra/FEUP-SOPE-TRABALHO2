#pragma once

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "sope.h"
#include "types.h"
#include "serverMessage.h"

int send_request(tlv_request_t *request);
int get_request(tlv_request_t *user_request, int fdr);
int send_reply(tlv_request_t *user_request, tlv_reply_t *user_reply);
int get_reply(tlv_reply_t *reply, char *fifo_reply, int fda);
