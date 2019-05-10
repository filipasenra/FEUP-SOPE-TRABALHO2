// User Program

#include "userMessage.h"

int main(int argc, char *argv[])
{
    tlv_request_t user_request;
    tlv_reply_t user_reply;
    /*pthread_t t;
    int thread_end = 0;
    int timeout = 0;

    //Send request
    if (requestMessageTLV(argc, argv, &user_request))
        return RC_OTHER;

    //Sending request to secure_srv
    if(pthread_create(&t, NULL, send_req, (void*)(&user_request)))   return RC_OTHER;
    while(!thread_end){
        //check clock for timeout
        //If timeout exit
    }*/

    if (setCommunication(&user_request, &user_reply))
        return RC_OTHER;
    //Recieve reply
    //pthread_t t;
    //if(pthread_create(&t, NULL, get_reply, (void*)(&user_reply)))   return RC_OTHER;
    /*while(!thread_end){
        //check clock for timeout
    }*/

    //Reading reply
    //If timeout exit

    return RC_OK;
}