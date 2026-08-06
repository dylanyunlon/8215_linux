#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "audio_bt_sco_fifo_private.h"

static int sendServalMsgs(AudioBtScoMsg *msg, int n)
{
    int writefd = -1;

    if(NULL == msg || n <= 0)
    {
        printf("input error.  msg:%p, n:%d\n", msg, n);
        return -1;
    }

    writefd= open(AUDIO_BT_SCO_FIFO_SVR, O_WRONLY);

    if(writefd < 0)
    {
        printf("open fifo fail\n");
        return -1;
    }

    write(writefd, msg, n * sizeof(AudioBtScoMsg));

    close(writefd);
    return 0;
}

static int sendOneMsg(AudioBtScoMsg *msg)
{
    int writefd = -1;
    if(NULL == msg)
    {
        printf("input error.  msg NULL\n");
        return -1;
    }

    writefd= open(AUDIO_BT_SCO_FIFO_SVR, O_WRONLY);

    if(writefd < 0)
    {
        printf("open fifo fail\n");
        return -1;
    }

    write(writefd, msg, sizeof(AudioBtScoMsg));

    close(writefd);
    return 0;
}

static int fillMsgBTScoEvent(AudioBtScoMsg *msg, int start)
{
    struct timezone tz;

    if(NULL == msg)
    {
        printf("input error.  msg NULL\n");
    }

    msg->pid = getpid();
    msg->ver = AUDIO_BT_SCO_MSG_VERSION;
    msg->msgType = 0;
    msg->msgLen = sizeof(*msg);

    msg->data.start = start;
    msg->data.rate = 16000;

    gettimeofday(&msg->timeStamp, &tz);

    return 0;
}

static int fillMsgBTScoEvent8k(AudioBtScoMsg *msg, int start)
{
    struct timezone tz;

    if(NULL == msg)
    {
        printf("input error.  msg NULL\n");
    }

    msg->pid = getpid();
    msg->ver = AUDIO_BT_SCO_MSG_VERSION;
    msg->msgType = 0;
    msg->msgLen = sizeof(*msg);

    msg->data.start = start;
    msg->data.rate = 8000;

    gettimeofday(&msg->timeStamp, &tz);

    return 0;
}


int ackMsgConfirm()
{
    return 0;
}

int btsco_start(int samplerate)
{

    printf("btsco_start,samplerate:%d \n", samplerate);

    //return 0;

    char buffer[100];
    AudioBtScoMsg * msg = buffer;

    memset(buffer, 0, 100);

    if(8000 == samplerate)
    {
        fillMsgBTScoEvent8k(msg, 0);
    }
    else if(16000 == samplerate)
    {
        fillMsgBTScoEvent(msg, 0);
    }
    else
    {
        printf("btsco_start wrong samplerate:%d\n", samplerate);
        return -1;
    }

    sendOneMsg(msg);
    ackMsgConfirm();


    return 0;

}

int btsco_stop(void)
{
    printf("btsco_stop\n");
    //return 0;


    char buffer[100];
    AudioBtScoMsg * msg = buffer;

    memset(buffer, 0, 100);
    fillMsgBTScoEvent(msg, 1);

    sendOneMsg(msg);
    ackMsgConfirm();
    return 0;

}


#if 0
int main(int argc, char** argv)
{
    char buffer[100];
    AudioBtScoMsg * msg = buffer;
    int start = 1;

    memset(buffer, 0, 100);
    if(argc > 1)
    {
        start = argv[1][0] - '0';
    }

    fillMsgBTScoEvent(msg, start);
    sendOneMsg(msg);
    ackMsgConfirm();

    /*
    fillMsgPolicyEvent(msg, 1, 0);
    fillMsgPolicyEvent(msg, 2, 0);
    sendServalMsgs(msg, 2);
    */
    return 0;
}
#endif


