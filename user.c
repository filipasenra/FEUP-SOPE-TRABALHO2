// User Program

#include "userMessage.h"
#include "communication.h"

int main(int argc, char *argv[]) {
    tlv_request_t user_request;
    tlv_reply_t user_reply;

    //Send request
    if (requestMessageTLV(argc, argv, &user_request))   return RC_OTHER;

    //Sending request to secure_srv

    //Recieve reply
    pthread_t t;
    if(pthread_create(&t, NULL, get_reply, (void*)(&user_reply)))   return RC_OTHER;
    //if (setCommunication(&user_request, &user_reply))   return RC_OTHER;

    //Reading reply

    return RC_OK;
}