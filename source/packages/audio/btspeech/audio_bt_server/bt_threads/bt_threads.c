
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <sys/time.h>
#include "streams_api.h"
#include <syslog.h>

#define BT_SCO_THREAD "btScoThread"

static pthread_t gBtStreamThread;


static void* btStreamThread(void* arg)
{
    pthread_detach(pthread_self());
    printf(BT_SCO_THREAD, "btStreamThread\n");
    bt_dl_ul_thread((int)arg);

    return 0;
}

#define LAST_STAUTS_START (0)
#define LAST_STAUTS_STOP (1)
static int gLastStatus = LAST_STAUTS_STOP;
static int btStreamGo(int rate)
{
    printf(BT_SCO_THREAD, "btStreamGo\n");
    pthread_create(&gBtStreamThread, NULL, btStreamThread, (void*)rate);
    return 0;
}


int stop_audio_bt(void)
{
    if(LAST_STAUTS_STOP == gLastStatus) return;
    gLastStatus = LAST_STAUTS_STOP;

    printf(BT_SCO_THREAD, "stop_audio_bt\n");
    bt_dl_ul_streams_stop();
    return 0;
}


int start_audio_bt(int rate)
{
    if(LAST_STAUTS_START == gLastStatus) return;
    gLastStatus = LAST_STAUTS_START;

    if(is_bt_dl_ul_streams_started())
    {
        printf(BT_SCO_THREAD, "bt_dl_ul_streams already started, return\n");
            return -1;
        //}

    }
    printf(BT_SCO_THREAD, "start_audio_bt\n");
    btStreamGo(rate);

    return 0;
}
//#endif


