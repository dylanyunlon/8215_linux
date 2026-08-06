/*
* Copyright (c) 2016 AutoChips Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/




#include <dbus/dbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/time.h>

#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <syslog.h>
#include "policyEvent.h"
#include "audioPolicyFifoPrivate.h"
#include "fifo_client.h"



static  int gAppConnect[] =
{
    -1,
    -1,
    -1,
    -1,
    -1,
    -1
};


typedef int (*CMDCONNECT)(int , int );
typedef int (*CMDDISCONNECT)(int);
typedef int (*CMD_GET_SOURCE_ID)(char*);
typedef int (*CMD_GET_SINK_ID)(char*);
typedef int (*CMDFINDPULSEID)();

typedef int (*CMDSET_SYS_PRO)(int  , int );
typedef int (*CMDGET_SYS_PRO)(int );


static CMDCONNECT cmdConnect = NULL;
static CMDDISCONNECT cmdDisconnect = NULL;
static CMDFINDPULSEID cmdFindPulseId = NULL;
static CMD_GET_SOURCE_ID getSourceID = NULL;
static CMD_GET_SINK_ID getSinkID = NULL;
CMDSET_SYS_PRO cmdSetSysPro = NULL;
CMDGET_SYS_PRO cmdGetSysPro = NULL;


static int loadedSo = 0;
static int gAudioPolicyReady = 0;


//static void policy_aux_constructor(void) __attribute__((constructor));
//static void policy_aux_constructor(void)
//{
//    atc_stream_type_init();
//}

DBusConnection * connect_dbus(){
    DBusError err;
    DBusConnection * connection;
    int ret;

    //Step 1: connecting session bus

    dbus_error_init(&err);

    connection =dbus_bus_get(DBUS_BUS_SESSION, &err);
    if(dbus_error_is_set(&err)){
        printf("ConnectionErr : %s\n",err.message);
        dbus_error_free(&err);
    }
    if(connection == NULL) {
        printf("connection is NULL\n");
        return NULL;
    }

    return connection;
}

void send_method_call(DBusConnection * connection,void* arg1, void* arg2, char* methodName)
{
    DBusError err;
    DBusMessage * msg;
    DBusMessageIter    arg;
    DBusPendingCall * pending;
    dbus_bool_t * stat;
    dbus_uint32_t * level;
    char* strArg;

    dbus_error_init(&err);

    //Constructs a new message to invoke a method on a remote object.
    msg =dbus_message_new_method_call ("org.atc.PolicyServer",
                                       "/org/atc/PolicyObject",
                                       "org.atc.PolicyInterface",
                                       methodName);
   if(msg == NULL){
        printf("MessageNULL \n");
        return;
    }

    //Append arguments
    dbus_message_iter_init_append(msg, &arg);
    if(!dbus_message_iter_append_basic(&arg, DBUS_TYPE_UINT32,arg1)){ //dbus_message_iter_append_basic(&arg, DBUS_TYPE_STRING,&param)
        printf("Out of Memory at arg1!\n");
        exit(1);
    }

    if(!dbus_message_iter_append_basic(&arg, DBUS_TYPE_UINT32,arg2)){
        printf("Out of Memory at arg2!\n");
        exit(1);
    }

    //Queues amessage to send, as withdbus_connection_send() , but also returns aDBusPendingCall used to receive a reply to the message.
    //if(!dbus_connection_send_with_reply (connection, msg,&pending, -1)){
    if(!dbus_connection_send_with_reply (connection, msg,&pending, 1000)){
        printf("Out of Memory at dbus_connection_send_with_reply!\n");
        exit(1);
    }

    if(pending == NULL){
        printf("Pending CallNULL: connection is disconnected \n");
        dbus_message_unref(msg);
        return;
    }

    dbus_connection_flush(connection);
    dbus_message_unref(msg);

    //waiting a reply, block until we recieve a reply, Block until the pendingcall is completed.
    dbus_pending_call_block (pending);
    //get the reply message Gets the reply, or returns NULL if none has been received yet.
    msg =dbus_pending_call_steal_reply (pending);
    if (msg == NULL) {
        printf("ReplyNull\n");
         exit(1);
    }
    // free the pendingmessage handle
    dbus_pending_call_unref(pending);
    // read the parameters
    if(!dbus_message_iter_init(msg, &arg))
        printf("Error: fail, message has no return!\n");
    else if (dbus_message_iter_get_arg_type(&arg) == DBUS_TYPE_BOOLEAN) {
        dbus_message_iter_get_basic(&arg, &stat);
        //printf("Sucess, return is boolean: %d!\n", stat);
    }
    else if (dbus_message_iter_get_arg_type(&arg) == DBUS_TYPE_STRING) {
        dbus_message_iter_get_basic(&arg, &strArg);
        //printf("Argument is string:%s!\n", strArg);
    }
    else if (dbus_message_iter_get_arg_type(&arg) == DBUS_TYPE_UINT32) {
        dbus_uint32_t *tmp = (dbus_uint32_t *)arg2;
        dbus_message_iter_get_basic(&arg, tmp);
        //printf("Argument is string:%u!\n", *tmp);
    }

    dbus_message_unref(msg);
}


int setDbusAddresss()
{
    FILE* stream = fopen("/tmp/dbus-tmp-file","r");
    char buf[256] = {0};
    char* value;

    if (stream)
        fgets(buf, sizeof(buf),stream);
    else {
        printf("fopen fail: /tmp/dbus-tmp-file\n");
        fclose(stream);
        return -1;
    }

    int len = strlen(buf);
    buf[len-1] = '\0';

    //printf("setenv DBUS_SESSION_BUS_ADDRESS is %s\n", buf);
    setenv("DBUS_SESSION_BUS_ADDRESS",buf,1);

    value = getenv("DBUS_SESSION_BUS_ADDRESS");
    if (value == NULL) {
        printf("getenv DBUS_SESSION_BUS_ADDRESS is not success\n");
        fclose(stream);
        return -1;
    }

    fclose(stream);

    return 0;
}

DBusConnection * getConnection()
{
    if (setDbusAddresss() < 0)
        return NULL;

    return connect_dbus();
}



static void loadCommonAPILib()
{
    if(loadedSo)
        return ;

    void* handle = dlopen(COMMON_API_CLIENT_LIB_PATH, RTLD_LAZY | RTLD_GLOBAL);
    if(NULL == handle)
    {
        printf("policyEvent can not dlopen so\n");
        return;
    }

    /*************************************************************/
    void *func = NULL;
    func = dlsym(handle, "commandConnect");
    if(NULL == func)
    {
        printf("policyEvent can not dlsym func\n");
        return;
    }
    cmdConnect = (CMDCONNECT)func;

    /*************************************************************/
    func = NULL;
    func = dlsym(handle, "commandDisconnect");
    if(NULL == func)
    {
        printf("policyEvent can not dlsym func\n");
        return;
    }
    cmdDisconnect = (CMDDISCONNECT)func;

    /*************************************************************/

    func = NULL;
    func = dlsym(handle, "commandGetSourceID");
    if(NULL == func)
    {
        printf("policyEvent can not dlsym func\n");
        return;
    }
    getSourceID = (CMD_GET_SOURCE_ID)func;

    /*************************************************************/
    func = NULL;
    func = dlsym(handle, "commandGetSinkID");
    if(NULL == func)
    {
        printf("policyEvent can not dlsym func\n");
        return;
    }
    getSinkID = (CMD_GET_SINK_ID)func;


    /*************************************************************/

    func = NULL;
    func = dlsym(handle, "commandSetSystemProperty");
    if(NULL == func)
    {
        printf("policyEvent can not dlsym func\n");
        return;
    }
    cmdSetSysPro = (CMDSET_SYS_PRO)func;

    /*************************************************************/

    func = NULL;
    func = dlsym(handle, "commandGetSystemProperty");
    if(NULL == func)
    {
        printf("policyEvent can not dlsym func\n");
        return;
    }
    cmdGetSysPro = (CMDGET_SYS_PRO)func;

    loadedSo = 1;

    printf("policyEvent  loadedSo success\n");

    return ;

}


static int sendPolicyEventWith_AM(STREAM_HANDLE hStream, AUDIO_STATE eAudState)
{
    printf("policyEvent this is Audiomanager environment\n");

    if(0 == loadedSo)
        loadCommonAPILib();

    if(START == eAudState)
    {
        int srcID = getSourceID(APP_NAME[(int)hStream]);
        if(cmdConnect && getSinkID)
        {
            int connectID = -1;
            connectID = cmdConnect(srcID, getSinkID(AM_DRIVER_SINK_NAME));
            if(connectID < 0)
            {
                return -1;
            }
            gAppConnect[(int)hStream] = connectID;
            return 0;
        }
    }
    else
    {
        if(cmdDisconnect)
        {
            return cmdDisconnect(gAppConnect[(int)hStream]);
        }
    }
    return 0;

}


static int sendPolicyEventEX(STREAM_HANDLE hStream, AUDIO_STATE eAudState)
{
    printf("policyEvent this is no Audiomanager environment\n");

    DBusConnection* connection = getConnection();
    if (connection == NULL)
        return -1;

    send_method_call(connection,&hStream,&eAudState, "PolicyEvent");
    return 0;
}


static int hasAMTmpFile()
{
    return 1;

    /*if(0 == access(AM_TMP_FLAG_FILE, F_OK))
    {
        return 1;
    }
    return 0;*/
}

static int isAudioPolicyServerOK()
{
    if(0 == gAudioPolicyReady)
    {
        int count = 0;
        while(0 != access(AUDIO_POLICY_FIFO_SVR, F_OK))
        {
            usleep(10000); // Slepp 10ms
            count++;

            if(0 == (count % 100))
            {
                printf("AudioPolicy server isn't ready. Waiting...!\n");
            }
        }
        gAudioPolicyReady = 1;
    }
    return 1;
}

int sendPolicyEvent(STREAM_HANDLE hStream, AUDIO_STATE eAudState)
{
    printf("Note:AudioPolicy has been abandoned!!! sendPolicyEvent stream(%p) status(%d). \n\n", hStream, eAudState);
    return 0;

    printf("sendPolicyEvent stream(%p) status(%d)\n\n", hStream, eAudState);
    if(0 == isAudioPolicyServerOK())
    {
        printf("AudioPolicy server isn't exist!!!\n");  //cgx todo log
        return -1;
    }

    sendPolicyEventByFifo(hStream, eAudState);

    printf("sendPolicyEvent stream(%p) end\n\n", hStream);//cgx todo
    return 0;
}


int send_policy_event(STREAM_HANDLE hStream, AUDIO_STATE eAudState)
{
    return sendPolicyEvent(hStream, eAudState);
}

int sendPolicyEventNoBlock(STREAM_HANDLE hStream, AUDIO_STATE eAudState)
{
    printf("Note:AudioPolicy has been abandoned!!! no block STREAM_HANDLE is %p, aud status %d\n\n", hStream, eAudState);
    return 0;

    if(hasAMTmpFile())
    {
        sendPolicyEventWith_AM(hStream, eAudState);
        return 0;
    }
    sendPolicyEventEX(hStream, eAudState);
    return 0;
}

int send_policy_event_no_block(STREAM_HANDLE hStream, AUDIO_STATE eAudState)
{
    return sendPolicyEventNoBlock(hStream, eAudState);
}

int sendStreamTypeVol(int streamType, int volume)
{
    printf("Note:AudioPolicy has been abandoned!!! streamType is %d, volume %d\n\n", streamType, volume);
    return 0;

    if(0 == isAudioPolicyServerOK())
    {
        printf("no audioPolicy server in system!\n");
        return -1;
    }
    //return sendStreamTypeVolNoBlock(streamType, volume);
    return sendStreamTypeVolByFifo(streamType, volume);
}

int send_stream_type_vol(int streamType, int volume)
{
    if (volume < 0)
        {volume = 0;}

    return sendStreamTypeVol(streamType, volume);
}

int sendStreamTypeVolNoBlock(int streamType, int volume)
{
    printf("Note:AudioPolicy has been abandoned!!! sendStreamTypeVolNoBlock  streamType is %d, volume %d\n\n", streamType, volume);
    return 0;

    if(0 == loadedSo)
        loadCommonAPILib();

    //if (index > VOLUME_INDEX_MAX && MM_GAIN_TYPE != streamType)
     //   {index = VOLUME_INDEX_MAX;}

    if (volume < 0)
        {volume = 0;}

    if(MM_GAIN_TYPE == streamType)
    {   /*================== set MM's max volume ===================*/
        /*common api pass this index, only can bypass 16bits data   */
        /*      but the max volume is more than 0x20000,            */
        /*           so need to ignore LSI 4 bits                   */
        /*==========================================================*/
        volume = (volume & 0x0FFFFFF0) >> 4;
    }
    else
    {
        streamType = ((streamType & 0x0FF) | ((volume & 0x0FF0000) >> 8));
        volume = (volume & 0x0FFFF);
    }


    if(hasAMTmpFile())
    {

        printf("policyEvent this is Audiomanager environment\n");
        if(cmdSetSysPro)
            return cmdSetSysPro(streamType, volume);

        return -1;
    }

    printf("policyEvent this is no Audiomanager environment\n");
    DBusConnection* connection = getConnection();
    if (connection == NULL)
        return -1;

    send_method_call(connection,&streamType,&volume,"typeIndex");
    return 0;
}

int send_stream_type_vol_no_block(int streamType, int volume)
{
    sendStreamTypeVolNoBlock(streamType, volume);
}

int receiveStreamTypeVol(int streamType)
{
    int retIndex = -1;
    printf("Note:AudioPolicy has been abandoned!!! receiveStreamTypeVol %d return -1\n\n", streamType);
    return retIndex;

    if(0 == isAudioPolicyServerOK())
    {
        printf("no audioPolicy server in system!\n");
        return -1;
    }

    if(0 == loadedSo)
        loadCommonAPILib();

    if(hasAMTmpFile())
    {
        printf("policyEvent this is Audiomanager environment\n");
        if(cmdGetSysPro)
            return cmdGetSysPro(streamType);

        return -1;
    }

    printf("policyEvent this is no Audiomanager environment\n");
    DBusConnection* connection = getConnection();
    if (connection == NULL)
        return -1;

    send_method_call(connection,&streamType,&retIndex,"GetTypeIndex");
    printf("receiveStreamTypeVol streamType is %d, Index is %d\n\n", streamType, retIndex);
    return 0;
}

/*==========================================================================*/
/*                          share memory operation                          */
/*==========================================================================*/
static int gShmFd = -1;
static int* gShmAddress = NULL;

/*typedef struct POLICY_EVENT_DEBUG{
    int* pgShmFd;
    int** pShmAddress;

}POLICY_EVENT_DEBUG;

POLICY_EVENT_DEBUG gPolicyEventDebug = {
    .pgShmFd = &gShmFd,
    .pShmAddress = &gShmAddress

};*/



static int policy_event_shm_init()
{
    gShmFd = shm_open(AUDIO_POLICY_SHM_MEM_NAME, O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if(gShmFd < 0)
    {
        printf("policy_event_shm_init error ,open /audio_policy_shm fail");
        return -1;
    }

    gShmAddress = mmap(NULL, AUDIO_POLICY_SHM_MEM_LEN, PROT_READ, MAP_SHARED, gShmFd, 0);
    if(NULL == gShmAddress)
    {
        printf("policy_event_shm_init error ,gShmAddress NULL");
        return -1;
    }
    return 0;
}

static void policy_event_shm_unInit()
{
    munmap(gShmAddress, AUDIO_POLICY_SHM_MEM_LEN);
    gShmAddress = NULL;
    gShmFd -1;
}

int policy_event_get_streams()
{
    int ret = 0;
    if(policy_event_shm_init() < 0){return -1;}
    if(gShmAddress)
    {
        ret = *gShmAddress ;
    }
    policy_event_shm_unInit();
    return ret;
}

