#include <linux/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <unistd.h>
#include <locale.h>
#include <stdlib.h>
//#include <glib.h>
#include <stdio.h>
#include <atcsurface.h>
#include <linux/fb.h>
#include "async_queue.h"
#include "avcodec.h"
#include "load_data.h"
#include "display.h"

#undef  LOG_TAG
#define LOG_TAG "DTDEMO"

#define PRINT_BASE(lvl, format, arg...) do {   \
    struct timespec ts;                             \
    struct tm tm;                                   \
    clock_gettime(CLOCK_REALTIME, &ts);             \
    localtime_r(&ts.tv_sec, &tm);                   \
    fprintf(stderr, "%02d:%02d:%02d.%03u " lvl "[MM]" LOG_TAG "[%s:%d] " format, \
         tm.tm_hour, tm.tm_min, tm.tm_sec, (unsigned)(ts.tv_nsec / 1000000), \
         __FUNCTION__, __LINE__, ##arg); \
} while(0)

#define PRINT_ERROR(format, arg...)  PRINT_BASE("[E]", format, ##arg)
#define PRINT_INFO(format, arg...) PRINT_BASE("[I]", format, ##arg)
#define PRINT_WARNING(format, arg...) PRINT_BASE("[W]", format, ##arg)
#define PRINT_DEBUG(format, arg...) PRINT_BASE("[D]", format, ##arg)

static void onInputBufferAvailable(AVCodec *inst, AVCodecBuffer *codecBuffer);
static void onOutputBufferAvailable(AVCodec *inst, AVCodecBuffer *codecBuffer);

static bool  need_stop = false;
static uint32_t frame_count = 0;
static int64_t base_pts = 0;
static IAtcSurface *video_surface = NULL;
static void *loaddatainst = NULL;
AsyncQueue *inputbufferQ = NULL;

static BufferCallback callbacks = {onInputBufferAvailable, onOutputBufferAvailable, NULL};

static void onInputBufferAvailable(AVCodec *inst, AVCodecBuffer *codecBuffer)
{
    PRINT_INFO("enter, codecBuffer:%p, base:%p\n", codecBuffer, codecBuffer->base());
    if (inputbufferQ != NULL)
        async_queue_push(inputbufferQ, codecBuffer);
}
static void onOutputBufferAvailable(AVCodec *inst, AVCodecBuffer *codecBuffer)
{
    PRINT_INFO("enter, codecBuffer:%p, base:%p\n", codecBuffer, codecBuffer->base());
    if (inst)
        inst->releaseOutputBuffer(codecBuffer);
    if (codecBuffer->size() > 0)
        frame_count++;
    if (codecBuffer->flags() & BUFFER_FLAG_EOS)
        need_stop = true;
}

#define LOG_TO_STD
#ifdef LOG_TO_STD
__attribute__((constructor(200)))
static void dt_log_init(void)
{
  int fd;
  printf("dt_log_init enter\n");
  fd = open("/media/udisk2/log.txt", O_RDWR | O_CREAT | O_APPEND);
  if (fd >= 0) {
    dup2(fd, 2);
    close(fd);
  }
}
#endif

void DataReceived(__u8 *data, __u32 len, void *pvarg)
{
  AVCodec *decoder = (AVCodec *)pvarg;
  PRINT_INFO( "[DTDEMO] %s atc_vdec_decode(%p, %p, %d)\r\n",
    __FUNCTION__, pvarg, data, len);
  AVCodecBuffer *codecBuffer = (AVCodecBuffer *)async_queue_pop(inputbufferQ);
  memcpy(codecBuffer->base(), data, len);
  codecBuffer->setRange(0, len);
  if (len == 0) {
      codecBuffer->setBufferInfo(0, 0, 0, BUFFER_FLAG_EOS);
  } else {
      codecBuffer->setBufferInfo(0, 0, base_pts, 0);
      base_pts += 33333;
  }
  decoder->queueInputBuffer(codecBuffer);
}

bool get_resolution_and_bpp(int *width, int *height, int *bpp)
{
    int fd = -1;
    struct fb_var_screeninfo var_info;
    int logo_bpp = 0;
    if (width == NULL || height==NULL){
        PRINT_INFO( "Error: input para is NULL");
        return -1;
    }

    fd = open("/dev/fb0", O_RDWR);
    if (fd < 0){
        PRINT_INFO( "Error: open fbdev failed.");
        return -1;
    }

    if (ioctl(fd,FBIOGET_VSCREENINFO,&var_info)==-1){
        PRINT_INFO( "Error: get screen info failed.");
        close(fd);
        return -1;
    }

    if (ioctl(fd,DISPLAY_GET_LOGO_BPP,&logo_bpp) < 0){
        PRINT_INFO( "Error: get screen info failed.");
        close(fd);
        return -1;
    }

    close(fd);
    *width = var_info.xres;
    *height = var_info.yres;
    *bpp = logo_bpp;
    return 0;
}

int main(int argc, char *argv[])
{
    char *szfilename = "/media/test.h264";
    int ret = 0;
    AVCodec *decoder = NULL;

    if (argc > 1) {
        szfilename = argv[1];
    }

/*
    ret = setenv("DT_DEBUG_LOG", "4", 1);
    if (-1 == ret) {
        printf("[mediaplayer]set env fail for %s (%d)\r\n", strerror(errno), errno);
    }
*/
    PRINT_INFO("atc directrender test");
    video_surface = atc_createsurface(ATCSURF_TYPE_DEFAULT,
        1920, 1080,
        ATC_PIX_FMT_NV12M_PRIVATE1);
    if (NULL == video_surface) {
        PRINT_INFO( "CreateVideoSurface failed\r\n");
        return -1;
    }

    IAtcSurface_setLayerZOrder(video_surface, 3);
    int width = 0, height = 0;
    int bpp = 0;
    if (get_resolution_and_bpp(&width,&height,&bpp) == -1){
        PRINT_INFO( " fail in get resolution\r\n");
        return -1;
    };
    IAtcSurface_setWindow(video_surface, 0, 0, width, height);

    loaddatainst = OpenDataLoad(szfilename, false);
    if (NULL == loaddatainst) {
        PRINT_INFO( "[DTDEMO] fail in OpenDataLoad\r\n");
        return -1;
    }
    inputbufferQ = async_queue_new();

    decoder = AVCodec::CreateByType("video/avc");
    if (decoder == NULL)
        return -1;
    ret = decoder->configure(1920, 1080, video_surface);
    if (ret)
        return -1;
    decoder->setCallback(&callbacks);
    ret = decoder->start();
    if (ret)
        return -1;

    PRINT_INFO( "(loaddata: %p)\r\n", loaddatainst);
    if (NULL != loaddatainst) {
    PRINT_INFO( "DataLoadRegDataReceive\r\n");
      if (!DataLoadRegDataReceive(loaddatainst, DataReceived, decoder)){
        PRINT_INFO( "[DTDEMO] %s fail in DataLoadRegDataReceive(%p)\r\n",
          __FUNCTION__, loaddatainst);
        return -1;
      }
    }

  PRINT_INFO( "StartDataLoad\r\n");
  if (!StartDataLoad(loaddatainst)) {
    PRINT_INFO( "[DTDEMO] %s fail in StartDataLoad(%p)\r\n",
      __FUNCTION__, loaddatainst);
    return -1;
  }

  while (!need_stop) {
      usleep(100000);
  }

  PRINT_INFO( "[DTDEMO] %s exit, frame_count:%d\r\n", __FUNCTION__, frame_count);
  decoder->stop();
  decoder->release();
  delete decoder;
  if (NULL != video_surface) {
    IAtcSurface_release(video_surface);
  }

  return 0;
}

