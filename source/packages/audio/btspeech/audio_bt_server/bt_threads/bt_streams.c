/*
* Copyright (c) 2016 AutoChips Inc.
*
*  This Source Code Form is subject to the terms of the Mozilla Public
*  License, v. 2.0. If a copy of the MPL was not distributed with this
*  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*
*/


#include <sys/types.h>
#include <sys/resource.h>
#include <unistd.h>
#include <syslog.h>
#include <stdio.h>
#include <unistd.h>
#include <alsa/asoundlib.h>
#include "alsa_ops.h"
//#include "bt_enhancement.h"
#include "src_api.h"
#include "audio_bt_com_priv.h"


#define BT_SCO_STREAMS "btScoStreams"

#define DEBUG_STREAMS_PATH "/tmp/bt_streams/"
#define DEBUG_STREAMS_ON_OFF "/tmp/bt_sco_dump"

#define EMPTY_DATA_CYCLE  5

static snd_pcm_t *gCaptureHandle = NULL;
static snd_output_t *gCaptureLog = NULL;

static snd_pcm_t *gCapBTHandle = NULL;
static snd_output_t *gCapBTLog = NULL;

static snd_pcm_t *gPlayBTHandle = NULL;
static snd_output_t *gPlayBTLog = NULL;

static snd_pcm_t *gPlaySpkHandle = NULL;
static snd_output_t *gPlaySpkLog = NULL;

static int gShouldStop = 0;
static int gStreamsStarted = 0;

//static signed short  gEPL[DEBUG_EPL_SIZE];

static int gDebugSeq = 0;

static int gIs8k = 0;

static pthread_mutex_t gStoppingMutex ;

typedef struct Debug_BT_SCO{
int fd_dl_pre ;
int fd_dl_post;
int fd_dl_8k;
int fd_ul_pre ;
int fd_ul_post;
int fd_ul_8k;
//int fd_EPL;

int dataCnt; //for how many samples
} Debug_BT_SCO;

Debug_BT_SCO gDbgBtSco;

void stopping_mutex_init() __attribute__((constructor));

void stopping_mutex_init()
{
    printf(BT_SCO_STREAMS, "stopping and starting _mutex_init\n");

    pthread_mutex_init(&gStoppingMutex, NULL);
    return;
}

static void debug_increase_cnt(int cnt)
{
    gDbgBtSco.dataCnt += cnt;
}

static void debug_clear()
{
    gDbgBtSco.fd_dl_pre = -1;
    gDbgBtSco.fd_dl_post = -1;
    gDbgBtSco.fd_ul_pre = -1;
    gDbgBtSco.fd_ul_post = -1;
    //gDbgBtSco.fd_EPL = -1;

    gDbgBtSco.fd_dl_8k = -1;
    gDbgBtSco.fd_ul_8k = -1;

    gDbgBtSco.dataCnt = 0;
}

static int debug_open(char* name)
{
    if(NULL == name){printf(BT_SCO_STREAMS, "debug_open name = null\n");return -1;}

    char fullname[100];
    char* path = DEBUG_STREAMS_PATH;

    sprintf(fullname, "%s%s_%d.wav", path, name, gDebugSeq);

    mkdir(path, 0777);


    int fd = open(fullname, O_RDWR | O_CREAT | O_TRUNC, 0777);
    if(fd < 0)
    {
        printf(BT_SCO_STREAMS, "open %s fail\n", fullname);
    }
    return fd;
}

static void debug_write(int fd, unsigned char * buffer, int count)
{
    if(fd > 0)
    {
        write(fd, buffer, count);
    }
}

static void debug_write_dl_pre(unsigned char * buffer, int count)
{
    return debug_write(gDbgBtSco.fd_dl_pre,  buffer, count);
}

typedef struct WavHeader {
	char rld[4];
	int rLen;
	char wld[4];
	char fld[4];
	int fLen;
	short wFormatTag;
	short wChannels;
	int nSamplesPersec;
	int nAvgBitsPerSample;
	short wBlockAlign;
	short wBitsPerSample;
	char dld[4];
	int wSampleLength;
} WavHeader;


static void debug_fill_len(int fd, int cnt)
{
    int len = 0;
    if(fd < 0){return;}

    len = cnt;

    int offset = (int)(&(((WavHeader*)0)->wSampleLength));
    printf(BT_SCO_STREAMS, "debug_close  offset:%d\n", offset);

    lseek(fd, offset, SEEK_SET);
    write(fd, &len, sizeof(int));


    offset = (int)(&(((WavHeader*)0)->rLen));
    printf(BT_SCO_STREAMS, "debug_close 2 offset:%d\n", offset);

    lseek(fd, offset, SEEK_SET);
    len += sizeof(WavHeader) - 8;
    write(fd, &len, sizeof(int));

}

static void debug_close(int fd)
{
    if(fd > 0)
    {
        close(fd);
    }
    return;
}

static void debug_fill_head(WavHeader* head, int rate)
{
    head->rld[0] = 'R';
    head->rld[1] = 'I';
    head->rld[2] = 'F';
    head->rld[3] = 'F';

    head->wld[0] = 'W';
    head->wld[1] = 'A';
    head->wld[2] = 'V';
    head->wld[3] = 'E';

    head->fld[0] = 'f';
    head->fld[1] = 'm';
    head->fld[2] = 't';
    head->fld[3] = ' ';

    head->dld[0] = 'd';
    head->dld[1] = 'a';
    head->dld[2] = 't';
    head->dld[3] = 'a';


    head->fLen = 16;
    head->wFormatTag = 1;
    head->wChannels = 1;
    head->nSamplesPersec = rate;
    head->nAvgBitsPerSample = rate * 2;
    head->wBlockAlign = 2;
    head->wBitsPerSample = 16;

    //head->wSampleLength = 16;
    //head->rLen = 16;

}

static void debug_init()
{
    debug_clear();

    if(0 != access(DEBUG_STREAMS_ON_OFF, F_OK)){return;}

    gDbgBtSco.fd_dl_pre =   debug_open("dl_pre");
    gDbgBtSco.fd_dl_post =  debug_open("dl_post");
    gDbgBtSco.fd_dl_8k =  debug_open("dl_8k");
    gDbgBtSco.fd_ul_pre =   debug_open("ul_pre");
    gDbgBtSco.fd_ul_post =  debug_open("ul_post");
    gDbgBtSco.fd_ul_8k =  debug_open("ul_8k");
    //gDbgBtSco.fd_EPL =      debug_open("epl_data");

    /*==============================================*/
    /*          write wav head for 16k pcm          */
    /*==============================================*/
    WavHeader wh;
    memset(&wh, 0, sizeof(WavHeader));
    debug_fill_head(&wh, 16000);
    debug_write(gDbgBtSco.fd_dl_pre, &wh, sizeof(WavHeader));
    debug_write(gDbgBtSco.fd_dl_post, &wh, sizeof(WavHeader));
    debug_write(gDbgBtSco.fd_ul_pre, &wh, sizeof(WavHeader));
    debug_write(gDbgBtSco.fd_ul_post, &wh, sizeof(WavHeader));

    /*==============================================*/
    /*          write wav head for 8k pcm           */
    /*==============================================*/
    memset(&wh, 0, sizeof(WavHeader));
    debug_fill_head(&wh, 8000);
    debug_write(gDbgBtSco.fd_dl_8k, &wh, sizeof(WavHeader));
    debug_write(gDbgBtSco.fd_ul_8k, &wh, sizeof(WavHeader));

    return;
}


static void debug_unInit()
{
    /*==============================================*/
    /*  fill length in wav head for 16k pcm         */
    /*==============================================*/
    debug_fill_len(gDbgBtSco.fd_dl_pre, gDbgBtSco.dataCnt);
    debug_fill_len(gDbgBtSco.fd_dl_post, gDbgBtSco.dataCnt);
    debug_fill_len(gDbgBtSco.fd_ul_pre, gDbgBtSco.dataCnt);
    debug_fill_len(gDbgBtSco.fd_ul_post, gDbgBtSco.dataCnt);

    /*==============================================*/
    /*  fill length in wav head for  8k pcm         */
    /*==============================================*/
    if(gIs8k)
    {
        debug_fill_len(gDbgBtSco.fd_dl_8k, gDbgBtSco.dataCnt / 2);
        debug_fill_len(gDbgBtSco.fd_ul_8k, gDbgBtSco.dataCnt / 2);
    }

    /*==============================================*/
    /*                      close fd                */
    /*==============================================*/
    debug_close(gDbgBtSco.fd_dl_pre);
    debug_close(gDbgBtSco.fd_dl_post);
    debug_close(gDbgBtSco.fd_dl_8k);
    debug_close(gDbgBtSco.fd_ul_pre);
    debug_close(gDbgBtSco.fd_ul_post);
    debug_close(gDbgBtSco.fd_ul_8k);
    //debug_close(gDbgBtSco.fd_EPL);

    debug_clear();
    return;
}

void bt_dl_ul_streams_stop(void)
{
    pthread_mutex_lock(&gStoppingMutex);
    gShouldStop = 1;
    return ;
}

int is_bt_dl_ul_streams_started(void)
{

    pthread_mutex_lock(&gStoppingMutex);
    pthread_mutex_unlock(&gStoppingMutex);

    return gStreamsStarted;
}

int putData2NextNode(unsigned char * btBuffer, unsigned char * micBuffer, int count)
{
    unsigned char ul8kBuf[160 * 2] ;
    int ret = 0;
    if(gIs8k)
    {
        samples16kToSamples8k(micBuffer, ul8kBuf);
        debug_write(gDbgBtSco.fd_ul_8k, ul8kBuf, count);

        if(gPlayBTHandle)ret = alsa_pcm_write(gPlayBTHandle, ul8kBuf, count / 2);

        if(ret != count / 2)
        {
            printf(BT_SCO_STREAMS, "alsa_pcm_write  gPlayBTHandle less:   ret :%d\n", ret );
        }
    }
    else
    {
        if(gPlayBTHandle) ret = alsa_pcm_write(gPlayBTHandle, micBuffer, count);

        if(ret != count)
        {
            printf(BT_SCO_STREAMS, "alsa_pcm_write  gPlayBTHandle less:   ret :%d\n", ret );
        }
    }

    if(gPlaySpkHandle)ret = alsa_pcm_write(gPlaySpkHandle, btBuffer, count);
    if(ret != count)
    {
        printf(BT_SCO_STREAMS, "alsa_pcm_write  gPlaySpkHandle less:   ret :%d\n", ret );
    }

    return 0;
}



int setupUpLinkAlsa(int rate)
{
    if (alsa_pcm_open(&gPlayBTHandle, "hw:0,1", SND_PCM_STREAM_PLAYBACK, &gPlayBTLog) < 0)
        return (-1);

    if (alsa_pcm_open(&gCaptureHandle, "hw:0,0", SND_PCM_STREAM_CAPTURE, &gCaptureLog) < 0)   
        return (-1);

    if (alsa_set_hw_params(gPlayBTHandle, 1, rate, SND_PCM_FORMAT_S16_LE) < 0)
        return (-1);

    if (alsa_set_hw_params(gCaptureHandle, 1, 16000, SND_PCM_FORMAT_S16_LE) < 0)
        return (-1);

    alsa_set_sw_params(gPlayBTHandle);

    snd_pcm_dump(gPlayBTHandle, gPlayBTLog);
    snd_pcm_dump(gCaptureHandle, gCaptureLog);

    snd_pcm_prepare(gPlayBTHandle);
    snd_pcm_prepare(gCaptureHandle);

    return 0;
}

int setupDownLinkAlsa(int rate)
{
    if (alsa_pcm_open(&gCapBTHandle, "hw:0,1", SND_PCM_STREAM_CAPTURE, &gCapBTLog) < 0)
        return (-1);

    if (alsa_pcm_open(&gPlaySpkHandle, "hw:0,0", SND_PCM_STREAM_PLAYBACK, &gPlaySpkLog) < 0) 
        return (-1);

    if (alsa_set_hw_params(gCapBTHandle, 1, rate, SND_PCM_FORMAT_S16_LE) < 0)
        return (-1);

    if (alsa_set_hw_params(gPlaySpkHandle, 1, 16000, SND_PCM_FORMAT_S16_LE) < 0)
        return (-1);

    alsa_set_sw_params(gPlaySpkHandle);

    snd_pcm_dump(gPlaySpkHandle, gPlaySpkLog);
    snd_pcm_dump(gCapBTHandle, gCapBTLog);

    snd_pcm_prepare(gCapBTHandle);
    snd_pcm_prepare(gPlaySpkHandle);

    return 0;
}

int closeAlsa()
{


    if(gCapBTHandle){alsa_pcm_close(gCapBTHandle, gCapBTLog);}
    if(gCaptureHandle){alsa_pcm_close(gCaptureHandle, gCaptureLog);}
    if(gPlayBTHandle){alsa_pcm_close(gPlayBTHandle, gPlayBTLog);}
    if(gPlaySpkHandle){alsa_pcm_close(gPlaySpkHandle, gPlaySpkLog);}

    gCaptureHandle = NULL;
    gCaptureLog = NULL;
    gCapBTHandle = NULL;
    gCapBTLog = NULL;
    gPlayBTHandle = NULL;
    gPlayBTLog = NULL;
    gPlaySpkHandle = NULL;
    gPlaySpkLog = NULL;

    return 0;
}

int bt_dl_ul_thread(const int rate)
{
    printf(BT_SCO_STREAMS, "bt_dl_ul_thread, rate:%d\n", rate);
    gIs8k = (8000 == rate);
    int loopCnt = 0;

    const int size = 160 * 2 ;

    unsigned char dl8kBuf[80 * 2 * 2];
    unsigned char btBuf[160 * 2 * 2];
    unsigned char micBuf[160 * 2 * 2];
    int count = 0;
    int restartFg = 0;
    int i = 0;

Restart:
    restartFg = 1;
    gStreamsStarted = 1;
    gDebugSeq++;
    loopCnt = 0;

    debug_clear();
    debug_init();

    if (setupDownLinkAlsa(rate) < 0)
        goto Alsaclose;

    if (setupUpLinkAlsa(rate) < 0)
        goto Alsaclose;

    memset(dl8kBuf, 0, size);
    memset(btBuf, 0, 2 * size);
    memset(micBuf, 0, 2 * size);

    if (alsa_pcm_start(gCaptureHandle) < 0)
        goto Alsaclose;

    for(i = 0;i < EMPTY_DATA_CYCLE;i ++)
    {
        alsa_pcm_write(gPlaySpkHandle, btBuf, size);
        if (gIs8k)
            alsa_pcm_write(gPlayBTHandle, btBuf, size / 2);
        else
            alsa_pcm_write(gPlayBTHandle, btBuf, size);
    }

    if (setpriority(PRIO_PROCESS, 0, BT_SERVER_THREAD_NICE_PRIORITY)) {
        printf(BT_SCO_STREAMS, "setpriority with nice(%d) fail!\n", BT_SERVER_THREAD_NICE_PRIORITY);
    }

    //BTEnhancementInit();
    while(1)
    {
        count = 0;
        if(gShouldStop)
        {
            printf(BT_SCO_STREAMS, "streams stop\n");
            restartFg = 0;
            break;
        }

        if(gIs8k)
        {
            count = alsa_pcm_read(gCapBTHandle, dl8kBuf, size / 2);
            if(count != (size / 2))
            {
                printf(BT_SCO_STREAMS, "gCapBTHandle count:%d, size:%d\n", count, (size/2));
                break;
            }
            debug_write(gDbgBtSco.fd_dl_8k, dl8kBuf, size);
            samples8kToSamples16k(dl8kBuf, btBuf);
        }
        else
        {
            count = alsa_pcm_read(gCapBTHandle, btBuf, size);
            if(count != size)
            {
                printf(BT_SCO_STREAMS, "gCapBTHandle count:%d, size:%d\n", count, size);
                break;
            }
        }
  
        count = alsa_pcm_read(gCaptureHandle, micBuf, size);

        if(count != size)
        {
            printf(BT_SCO_STREAMS, "gCaptureHandle count:%d, size:%d\n", count, size);  
            break;
        }

        debug_write(gDbgBtSco.fd_dl_pre, btBuf, size * 2);
        debug_write(gDbgBtSco.fd_ul_pre, micBuf, size * 2);

        /*======================================================*/
        /*bt sco's first frames samples have contaminated data  */
        /* the 9th sample is 28123 the 10th sample is -18723    */
        /*      so abandon first 40ms data to work around       */
        /*          for mic, there is 0.5s abnormal data,       */
        /*          but Enhancement algorithm will fix it       */
        /*======================================================*/
        if(loopCnt < 2)
        {
            memset(btBuf, 0, 2 * size);
            memset(micBuf, 0, 2 * size);
        }

        //putBTEnhancementData(btBuf, micBuf, count);

        debug_write(gDbgBtSco.fd_dl_post, btBuf, size * 2);
        debug_write(gDbgBtSco.fd_ul_post, micBuf, size * 2);

        //getdebugEPL(gEPL);
        //debug_write(gDbgBtSco.fd_EPL, gEPL, DEBUG_EPL_SIZE * 2);

        debug_increase_cnt(size * 2);

        putData2NextNode(btBuf, micBuf, count);
        loopCnt++;
	}
    //BTEnhancementUnInit();
Alsaclose:
    debug_unInit();
    closeAlsa();

    if (restartFg)
    {
        printf(BT_SCO_STREAMS, "btScoStreams Restart \n");
        goto Restart;
    }
    gShouldStop = 0;
    gStreamsStarted = 0;
    gIs8k = 0;
    pthread_mutex_unlock(&gStoppingMutex);
    return 0;
}

