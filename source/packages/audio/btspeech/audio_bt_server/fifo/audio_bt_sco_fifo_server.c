#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <sys/time.h>
#include "audio_bt_sco_fifo_private.h"
#include "audio_bt_com_priv.h"


#include "policyEvent.h"
#include <syslog.h>

#define BT_SCO_FIFO_TAG "btScoFifo"

static pthread_mutex_t stop_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct MsgServer{
    int readfd;

    int epollFd;
    int pipe[2];
    int stop;
    pthread_t stopThread;
    pthread_t selfThread;

    AudioBtScoMsg msg;
}MsgServer ;
static MsgServer gMsgServer;


void BTScoFifoMsgPrint(AudioBtScoMsg * msg)
{
    printf(BT_SCO_FIFO_TAG, "================================================\n");
    printf(BT_SCO_FIFO_TAG, "|                    one msg                   |\n");
    printf(BT_SCO_FIFO_TAG, "================================================\n");
    printf(BT_SCO_FIFO_TAG, "AudioPolicyMsg:\n"
            "|-ver:%d\n"
            "|-pid:%d\n"
            "|-needAck:%d\n"
            "|-msgType:%d\n"
            "|-msgLen:%d\n"
            "'-data:\n"
            "   |-start.start:%d\n"
            "   |-start.rate:%d\n"
            , msg->ver
            , msg->pid
            , msg->needAck
            , msg->msgType
            , msg->msgLen
            , msg->data.start
            , msg->data.rate
            );
    printf(BT_SCO_FIFO_TAG, "------------------------------------------------\n");
    return;
}

static int readOneMsg(void)
{
    if(gMsgServer.readfd < 0)
    {
        printf(BT_SCO_FIFO_TAG, "readfd does not init !\n");
        return -1;
    }

    int n = read(gMsgServer.readfd, &gMsgServer.msg, sizeof(AudioBtScoMsg));
    printf(BT_SCO_FIFO_TAG, "readOneMsg n = %d\n", n );

    if(n < 0)
    {
        printf(BT_SCO_FIFO_TAG, "errno = %d, %s\n", errno, strerror(errno));
    }

    return n;
}

static int readPipe(void)
{
    if(gMsgServer.pipe[0] < 0)
    {
        printf(BT_SCO_FIFO_TAG, "pipe does not init !\n");
        return -1;
    }

    char stop = 0;
    int n = read(gMsgServer.pipe[0], (void*)&stop, sizeof(char));
    printf(BT_SCO_FIFO_TAG, "readPipe n = %d\n", n );

    if(n < 0)
    {
        printf(BT_SCO_FIFO_TAG, "errno = %d, %s\n", errno, strerror(errno));
    }
    if(1 == stop)
    {
        gMsgServer.stop = 1;
    }

    return n;
}

static int watchFifo(void)
{
    struct epoll_event eventCtl;


    eventCtl.data.fd = gMsgServer.readfd;
    eventCtl.events = EPOLLIN | EPOLLET;
    return epoll_ctl(gMsgServer.epollFd, EPOLL_CTL_ADD, gMsgServer.readfd, &eventCtl);
}

static int watchPipe(void)
{
    struct epoll_event eventCtl;

    eventCtl.data.fd = gMsgServer.pipe[0];
    eventCtl.events = EPOLLIN | EPOLLET;
    return epoll_ctl(gMsgServer.epollFd, EPOLL_CTL_ADD, gMsgServer.pipe[0], &eventCtl);
}

static int watchInput(void)
{

    if(gMsgServer.epollFd >= 0){
        close(gMsgServer.epollFd);
    }

    gMsgServer.epollFd = epoll_create(2);
    if(gMsgServer.epollFd < 0){
        printf(BT_SCO_FIFO_TAG, "epoll_create failed, errno = %d, %s\n", errno, strerror(errno));
        return -1;
    }

    if(watchFifo() < 0){
        printf(BT_SCO_FIFO_TAG, "watchFifo failed, errno = %d, %s\n", errno, strerror(errno));
        close(gMsgServer.epollFd);
        gMsgServer.epollFd = -1;
        return -1;
    }

    if(watchPipe() < 0){
        printf(BT_SCO_FIFO_TAG, "watchPipe failed, errno = %d, %s\n", errno, strerror(errno));
        close(gMsgServer.epollFd);
        gMsgServer.epollFd = -1;
        return -1;
    }

    return 0;
}

static int handleRecieveMsg(AudioBtScoMsg* msg)
{
        if (START == msg->data.start) {
            start_audio_bt(msg->data.rate);
        } else {
            stop_audio_bt();
        }


    return 0;
}

static int handleMsgEvent(void)
{
    int n = 0;

    do
    {
        n = readOneMsg();

        if(sizeof(AudioBtScoMsg) == n)
        {
            BTScoFifoMsgPrint(&gMsgServer.msg);
            handleRecieveMsg(&gMsgServer.msg);
        }

    }while(n > 0);
    return n;
}
static int handlePipeEvent(void)
{
    readPipe();
    return 0;
}

static int waitAndHandle(void)
{
    struct epoll_event events[10];
    int fdCnt = 0;
    int i = 0;

    if(gMsgServer.epollFd < 0)
    {
        printf(BT_SCO_FIFO_TAG, "errno = %d, %s\n", errno, strerror(errno));
        return -1;
    }

    fdCnt = epoll_wait(gMsgServer.epollFd, events, 10,  5000);

    if (fdCnt < 0) {
        if (errno == EINTR) {
            printf(BT_SCO_FIFO_TAG, "epoll_wait interrupted by signal\n");
            return 0;
        } else {
            printf(BT_SCO_FIFO_TAG, "epoll_wait error, errno = %d, %s\n", errno, strerror(errno));
            return 0;
        }
    }

    for(i = 0 ; i < fdCnt; i ++)
    {
        printf(BT_SCO_FIFO_TAG, "  epoll_wait() return \n");
        if(gMsgServer.readfd >= 0
           && gMsgServer.readfd == events[i].data.fd)
        {
            printf(BT_SCO_FIFO_TAG, "handleMsgEvent()\n");

            handleMsgEvent();
        }
        if(gMsgServer.pipe[0] >= 0
           && gMsgServer.pipe[0] == events[i].data.fd)
        {
            printf(BT_SCO_FIFO_TAG, "handlePipeEvent()\n");
            handlePipeEvent();
        }
        fflush(stdout);

    }

    return 0;
}


static int readThreadLoop(void)
{
    int error_count = 0;
    int max_errors = 20;
    //watchInput();

    if (setpriority(PRIO_PROCESS, 0, BT_SERVER_THREAD_NICE_PRIORITY)) {
        printf(BT_SCO_FIFO_TAG, "setpriority with nice(%d) fail!\n", BT_SERVER_THREAD_NICE_PRIORITY);
    }
#if 0
    while(1)
    {
        waitAndHandle();

        if(gMsgServer.stop)
        {
            printf(BT_SCO_FIFO_TAG, "recevie a stop command !\n");
            break;
        }
    }
#endif

    while(1)
    {
        if (gMsgServer.epollFd < 0) {
            printf(BT_SCO_FIFO_TAG, "Reinitializing epoll...\n");
            if (watchInput() < 0) {
                error_count++;
                if (error_count > max_errors) {
                    printf(BT_SCO_FIFO_TAG, "Too many errors, exiting\n");
                    break;
                }
                sleep(1);
                continue;
            }
            error_count = 0;
        }

        int result = waitAndHandle();
        
        if (result < 0) {
            error_count++;
            if (error_count > max_errors) {
                printf(BT_SCO_FIFO_TAG, "Too many consecutive errors, reinitializing...\n");
                close(gMsgServer.epollFd);
                gMsgServer.epollFd = -1;
                sleep(1);
                continue;
            }
        } else {
            error_count = 0;
        }

        if(gMsgServer.stop)
        {
            printf(BT_SCO_FIFO_TAG, "received a stop command !\n");
            break;
        }
    }

    return (void*)0;
}

static void initGlobal(void)
{
    gMsgServer.readfd = -1;
    gMsgServer.epollFd = -1;
    gMsgServer.pipe[0] = -1;
    gMsgServer.pipe[1] = -1;
    gMsgServer.stop = 0;
    memset(&gMsgServer.msg, 0, sizeof(gMsgServer.msg));

}

static int initFifo(void)
{

    printf(BT_SCO_FIFO_TAG, "mkfifo\n");
    if(mkfifo(AUDIO_BT_SCO_FIFO_SVR, O_RDWR) < 0)
    {
        if(EEXIST != errno)
        {
            printf(BT_SCO_FIFO_TAG, "can't create %s\n", AUDIO_BT_SCO_FIFO_SVR);
        }
        else
        {
            printf(BT_SCO_FIFO_TAG, "already exist \n");
        }
    }

    //gMsgServer.readfd = open(AUDIO_POLICY_FIFO_SVR, O_RDONLY);
    /*=====================================================*/
    /* if use O_RDONLY, open will block until the fifo fd  */
    /*                  be open by write mode              */
    /*=====================================================*/
    gMsgServer.readfd = open(AUDIO_BT_SCO_FIFO_SVR, O_RDWR | O_NONBLOCK);

    if(gMsgServer.readfd < 0)
    {
        printf(BT_SCO_FIFO_TAG, "open fifo fail\n");
        return -1;
    }
    return 0;
}

static int sendStop2Pipe(void)
{
    pthread_mutex_lock(&stop_mutex);
    if(gMsgServer.pipe[1] < 0)
    {
        printf(BT_SCO_FIFO_TAG, "pipe 1 does not init\n");
        pthread_mutex_unlock(&stop_mutex);
        return -1;
    }
    int c = 1;
    write(gMsgServer.pipe[1], &c, 1);
    pthread_mutex_unlock(&stop_mutex);
    return 0;
}

static void* pipeStopThread(void* arg)
{
    printf(BT_SCO_FIFO_TAG, "in thread pipe\n");
    int c = 0;
    while(1)
    {
        c = getchar();
        printf(BT_SCO_FIFO_TAG, "getchar() = %c, %d\n", c, c);
        if('s' == c)
        {
            sendStop2Pipe();
            break;
        }
    }

    return (void*)0;
}

static int initPipe(void)
{
    int ret = pipe(gMsgServer.pipe);
    if(ret < 0)
    {
        printf(BT_SCO_FIFO_TAG, "errno = %d, %s\n", errno, strerror(errno));
        return -1;
    }
    printf(BT_SCO_FIFO_TAG, "start thread pipe\n");

    //need change this code to adaptor audio policy quit procedure
    //pthread_create(&gMsgServer.stopThread, NULL, pipeStopThread , NULL);

    return 0;
}



static int init(void)
{
    //setbuf(stdout, NULL);
    printf(BT_SCO_FIFO_TAG, "initGlobal\n");
    initGlobal();

    printf(BT_SCO_FIFO_TAG, "initFifo\n");
    if(initFifo() < 0){
        return -1;
    }

    printf(BT_SCO_FIFO_TAG, "initPipe\n");
    if(initPipe() < 0){
        return -1;
    }

    return 0;
}
static void uninit(void)
{
    if(gMsgServer.readfd >= 0){
        close(gMsgServer.readfd);
    }
    if(gMsgServer.epollFd >= 0){
        close(gMsgServer.epollFd);
    }
    if(gMsgServer.pipe[0] >= 0){
        close(gMsgServer.pipe[0]);
    }
    if(gMsgServer.pipe[1] >= 0){
        close(gMsgServer.pipe[1]);
    }


    unlink(AUDIO_BT_SCO_FIFO_SVR);
    return;
}


static void* fifoThread(void* arg)
{
    if(init() < 0){
        return -1;
    }
    printf(BT_SCO_FIFO_TAG, "readThreadLoop\n");
    readThreadLoop();

    printf(BT_SCO_FIFO_TAG, "readThreadLoop exit\n");
    uninit();
    return 0;

}

int FifoServerGo(void)
{
    printf(BT_SCO_FIFO_TAG, "FifoServerGo\n");
    pthread_create(&gMsgServer.selfThread, NULL, fifoThread , NULL);
    return 0;
}


int main(void)
{
    printf(BT_SCO_FIFO_TAG, "audioFifoThread starting\n");
    
    while(1) {
        printf(BT_SCO_FIFO_TAG, "Starting server instance...\n");

        if(FifoServerGo() == 0) {
            if (gMsgServer.selfThread != 0) {
                pthread_join(gMsgServer.selfThread, NULL);
            }
        }

        printf(BT_SCO_FIFO_TAG, "Server instance stopped, restarting in 3 seconds...\n");
        sleep(3);

        initGlobal();
    }
    
    return 0;
}

