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
#include <syslog.h>
#include "audioPolicyFifoPrivate.h"
#include "fifo_client.h"


static int sendServalMsgs(AudioPolicyMsg *msg, int n)
{
    int writefd = -1;

    if(NULL == msg || n <= 0)
    {
        printf("input error.  msg:%p, n:%d\n", msg, n);
        return -1;
    }

    writefd= open(AUDIO_POLICY_FIFO_SVR, O_WRONLY);

    if(writefd < 0)
    {
        printf("open fifo fail\n");
        return -1;
    }

    write(writefd, msg, n * sizeof(AudioPolicyMsg));

    close(writefd);
    return 0;
}

static int sendOneMsg(AudioPolicyMsg *msg)
{
    int writefd = -1;
    if(NULL == msg)
    {
        printf("input error.  msg NULL\n");//cgx todo log
        return -1;
    }

//    writefd= open(AUDIO_POLICY_FIFO_SVR, O_WRONLY|O_NONBLOCK);
    writefd= open(AUDIO_POLICY_FIFO_SVR, O_WRONLY);

    if(writefd < 0)
    {
        printf("open fifo fail\n");
        return -1;
    }

    if (write(writefd, msg, sizeof(AudioPolicyMsg)) < 0)
    {
        printf("Write FIFO Failed (%d)\n", errno);
        return -1;
    }

    close(writefd);
    return 0;
}


static int fillMsgPolicyEvent(AudioPolicyMsg *msg, int handle, int state)
{
    struct timezone tz;

    msg->pid = getpid();
    msg->ver = AUDIO_POLICY_MSG_VERSION;
    msg->needAck = 0;
    msg->msgType = 0;
    msg->msgLen = sizeof(*msg);

    msg->data.streamState.handle = handle;
    msg->data.streamState.state = state;

    gettimeofday(&msg->timeStamp, &tz);

    return 0;
}


static int fillMsgStreamTypeVol(AudioPolicyMsg *msg, int streamType, int volume)
{
    struct timezone tz;

    msg->pid = getpid();
    msg->ver = AUDIO_POLICY_MSG_VERSION;
    msg->needAck = 0;
    msg->msgType = 1;
    msg->msgLen = sizeof(*msg);

    msg->data.streamVol.streamType = streamType;
    msg->data.streamVol.volume = volume;

    gettimeofday(&msg->timeStamp, &tz);

    return 0;
}




static int policyAckMsgConfirm()
{
    return 0;
}


int sendPolicyEventByFifo(int handle, int state)
{
    AudioPolicyMsg msg ;
    memset(&msg, 0, sizeof(AudioPolicyMsg));

    fillMsgPolicyEvent(&msg, handle, state);
    sendOneMsg(&msg);

    policyAckMsgConfirm();
    return 0;
}


int sendStreamTypeVolByFifo(int streamType, int volume)
{
    AudioPolicyMsg msg ;
    memset(&msg, 0, sizeof(AudioPolicyMsg));

    fillMsgStreamTypeVol(&msg, streamType, volume);
    sendOneMsg(&msg);

    policyAckMsgConfirm();
    return 0;
}


#if 0
int main()
{
    char buffer[100];
    AudioPolicyMsg * msg = buffer;

    memset(buffer, 0, 100);

    fillMsgPolicyEvent(msg, 1, 0);
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

