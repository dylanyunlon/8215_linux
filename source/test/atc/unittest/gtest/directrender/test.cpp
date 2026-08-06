#include <linux/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
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
#include "test.h"

#undef  LOG_TAG
#define LOG_TAG "DTTEST"

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

#define FRAME_WIDTH 800
#define FRAME_HEIGHT 480
#define FRAME_SIZE ((FRAME_WIDTH * FRAME_HEIGHT * 3) / 2)  // YUV420 frame size

static void onInputBufferAvailable(AVCodec *inst, AVCodecBuffer *codecBuffer);
static void onOutputBufferAvailable(AVCodec *inst, AVCodecBuffer *codecBuffer);

static bool  need_stop = false;
static uint32_t frame_count = 0;
AsyncQueue *inputbufferQ = NULL;
static int  displayw = 0;
static int  displayh = 0;
static int frame_index = 0;
const char* golden_file;
int test_result = 0;
int vdec_fd = -1;

static BufferCallback callbacks = {onInputBufferAvailable, onOutputBufferAvailable, NULL};

int compare_frame(char* frame) {
    FILE* f = fopen(golden_file, "rb");
    if (!f) {
        perror("Failed to open golden file");
        return -1;
    }

    if(fseek(f, frame_index * FRAME_SIZE, SEEK_SET) != 0) {
        perror("Failed to fseek");
        fclose(f);
        return -1;
    }
    frame_index++;

    unsigned char* golden_frame = (unsigned char* )malloc(FRAME_SIZE);
    if(!golden_frame) {
        perror("Failed to allocate memory");
        fclose(f);
        return -1;
    }

    size_t read_bytes = fread(golden_frame, 1, FRAME_SIZE, f);
    fclose(f);
    printf("frame_index %d golden_frame 0x%x,0x%x,frame 0x%x.0x%x", frame_index,golden_frame[0],golden_frame[1],frame[0],frame[1]);
    if (read_bytes != FRAME_SIZE) {
        fprintf(stderr, "Golden file size mismatch. Expected %zu, got %zu\n", FRAME_SIZE, read_bytes);
        free(golden_frame);
        return -1;
    }
    int result = memcmp(golden_frame, frame, FRAME_SIZE);
    free(golden_frame);
    return result == 0 ? 0 :1;
}

int writedumpfile(char * filename, void * data, __u32 len)
{
    FILE *fp = NULL;
    size_t WriteBytes = 0;

    if (NULL == data)
    {
        return -1;
    }
    else
    {
        fp = fopen(filename , "a+");
        if (NULL == fp)
        {
            PRINT_ERROR("Open File Failed, %s.92\r\n", strerror(errno));
            return -1;
        }
        else
        {
            WriteBytes = fwrite(data, len, 1, fp);
            if(WriteBytes != 1)
            {
                PRINT_ERROR("Write File Failed, %s.\r\n", strerror(errno));
                fclose(fp);
                return -1;
            }
            else
            {
                fclose(fp);
                return 0;
            }
        }
    }
}

// DUMP NV12 YUV
void vVdecDumpYUV(char* filename, const char *src_y, const char *src_c)
{
    __u32 width = ((displayw + 15) >> 4) << 4;
    __u32 height = ((displayh + 31) >> 5) << 5;
    __u32 u4FileSize = width * height + width * height / 2;
    __u32 u4YLen = width * height;

    //const char *src_y = (const char *)prDisplay_Info->ptrVirYAddr;
    //const char *src_c = (const char *)prDisplay_Info->ptrVirCAddr;

    __u32 u4BlockSize, u4BlockPerLine, u4Idx, u4ByteSize;
    __u32 u4BlockNum, u4BlockStartX, u4BlockStartY, u4BlockCoordX, u4BlockCoordY;
    char value;
    char *dst_ptr = NULL;

    u4ByteSize = width * height;
    u4BlockSize = 16*32;
    u4BlockPerLine = (width / 16);

    dst_ptr = (char *)MM_ALLOC(sizeof(char) * u4FileSize);
    if(NULL == dst_ptr)
    {
        PRINT_ERROR("vVdecDumpYUV Alloc Mem Failed!\r\n");
        need_stop = true;
        test_result = 1;
        return;
    }

    mm_memset(dst_ptr, 0, sizeof(char) * u4FileSize);

    for (u4Idx = 0; u4Idx < u4ByteSize; u4Idx++)
    {
        u4BlockNum = u4Idx /u4BlockSize;
        u4BlockStartX = (u4BlockNum % u4BlockPerLine) * 16;
        u4BlockStartY = (u4BlockNum / u4BlockPerLine) * 32;
        u4BlockCoordX = (u4Idx % u4BlockSize) % 16;// horizontal offset for the block
        u4BlockCoordY = (u4Idx % u4BlockSize) / 16;// vertical offset for the block

        if (((u4BlockStartX + u4BlockCoordX) >= displayw) ||
            ((u4BlockStartY + u4BlockCoordY) >= displayh))
        {
            //alignment data
        }
        else
        {
            value = *(src_y + u4Idx);
            dst_ptr[(u4BlockStartX + u4BlockCoordX) + (u4BlockStartY + u4BlockCoordY) * width] = value;
        }

    }

    u4BlockSize = 16*(32 >> 1);
    u4BlockPerLine = (width / 16);

    for (u4Idx = 0; u4Idx < (u4ByteSize >> 1); u4Idx+=2)
    {
        u4BlockNum = u4Idx /u4BlockSize;
        u4BlockStartX = (u4BlockNum % u4BlockPerLine) * 16;
        u4BlockStartY = (u4BlockNum / u4BlockPerLine) * (32 >> 1);
        u4BlockCoordX = (u4Idx % u4BlockSize) % 16;
        u4BlockCoordY = (u4Idx % u4BlockSize) / 16;

        if (((u4BlockStartX + u4BlockCoordX) >= displayw) ||
            ((u4BlockStartY + u4BlockCoordY) >= (displayh >> 1)))
        {
            // align data
        }
        else
        {
            value = *(src_c + u4Idx);
            dst_ptr[(u4BlockStartX + u4BlockCoordX) + (u4BlockStartY + u4BlockCoordY) * width + u4YLen] = value;
            value = *(src_c + u4Idx + 1);
            dst_ptr[(u4BlockStartX + u4BlockCoordX + 1) + (u4BlockStartY + u4BlockCoordY) * width + u4YLen] = value;

        }
    }
    printf("u4FileSize:%d,FRAME_SIZE:%d\n", u4FileSize, FRAME_SIZE);

    if (compare_frame(dst_ptr) != 0) {
        printf("Mismatch detected!\n");
        need_stop = true;
        test_result = 1;
    } else {
        printf("Frame Match\n");
    }

    //if (0 < writedumpfile(filename, (void*)dst_ptr, u4FileSize))
    //{
        //PRINT_ERROR("write N21 to pc fail\n");
    //}

    MM_FREE(dst_ptr);
}

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
    if (codecBuffer->flags() & BUFFER_FLAG_EOS) {
        need_stop = true;
    }else {
        struct VOUT_PARAM *out_buffer = (struct VOUT_PARAM *)(codecBuffer->base());
        PRINT_INFO("enter, displayw:%d, displayh:%d\n", displayw, displayh);
        PRINT_INFO("enter, codecBuffer:%p, base:%p\n", codecBuffer, codecBuffer->base());
        PRINT_INFO("enter, y=0x%08x\n", out_buffer->y_phy_addr);

        __u32 u4FrameBufSize = (DEC_ALIGN_MASK(displayw, 16) * DEC_ALIGN_MASK(displayh, 32) * 3 ) >> 1; // YUV420
        void  *pvMemPa= (unsigned char *) mmap64(NULL, u4FrameBufSize,
                PROT_READ | PROT_WRITE, MAP_SHARED, vdec_fd, out_buffer->y_phy_addr);
        if (pvMemPa == (__u8*)MAP_FAILED)
        {
            PRINT_ERROR("[VDEC] mmap non-cache memory fail :%s. \r\n", strerror(-errno));
            need_stop = true;
            test_result = 1;
            pvMemPa = NULL;
            return;
        }
        static int count = 0;
        char filename[64];
        snprintf(filename, sizeof(filename), "/media/udisk2/dump%d.bin", count++);
        vVdecDumpYUV(filename, (const char *)pvMemPa, (const char *)pvMemPa + DEC_ALIGN_MASK(displayw, 16) * DEC_ALIGN_MASK(displayh, 32));
        PRINT_INFO("enter, pvMemPa:%p\n", pvMemPa);
    }
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
  PRINT_INFO( "[DTDEMO] %s atc_vdec_decode(%p, %p, %d)\r\n",
    __FUNCTION__, pvarg, data, len);
  AVCodec *codec = (AVCodec *)pvarg;
  AVCodecBuffer *codecBuffer = (AVCodecBuffer *)async_queue_pop(inputbufferQ);
  memcpy(codecBuffer->base(), data, len);
  codecBuffer->setRange(0, len);
  if (len == 0) {
      codecBuffer->setBufferInfo(0, 0, 0, BUFFER_FLAG_EOS);
  } else {
      codecBuffer->setBufferInfo(0, 0, 0, 0);
  }
  codec->queueInputBuffer(codecBuffer);
  PRINT_INFO("queueInputBuffer done\n");
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

    if (ioctl(fd,0x00020027,&logo_bpp) < 0){
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

int test( const char *filename, const char *goldenname)
{
    int ret = 0;
    AVCodec *codec = NULL;
    void *loaddatainst = NULL;
    IAtcSurface *video_surface = NULL;
    need_stop = false;
    frame_index = 0;
    test_result = 0;
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
    displayw = 800;//DEC_ALIGN_MASK(width, 16);
    displayh = 480;//DEC_ALIGN_MASK(height, 32);
    golden_file = goldenname;

    vdec_fd = open("/dev/vdec", O_RDWR);
    if (vdec_fd < 0)
    {
        PRINT_ERROR("[DTDEMO] cannot open /dev/vdec \r\n");
        return -1;
    }

    loaddatainst = OpenDataLoad(filename, false);
    if (NULL == loaddatainst) {
        PRINT_INFO( "[DTDEMO] fail in OpenDataLoad\r\n");
        return -1;
    }
    inputbufferQ = async_queue_new();

    codec = AVCodec::CreateByType("video/avc");
    if (codec == NULL)
        return -1;
    ret = codec->configure(800, 480, video_surface);
    if (ret)
        return -1;
    codec->setCallback(&callbacks);
    ret = codec->start();
    if (ret)
        return -1;

    PRINT_INFO( "(loaddata: %p)\r\n", loaddatainst);
  if (NULL != loaddatainst) {
    PRINT_INFO( "DataLoadRegDataReceive\r\n");
    if (!DataLoadRegDataReceive(loaddatainst, DataReceived, codec)){
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
  if (frame_count == 0) {
      test_result = 1;
  }

  PRINT_INFO( "[DTDEMO] %s exit, frame_count:%d\r\n", __FUNCTION__, frame_count);
  codec->stop();
  codec->release();
  delete codec;
  if (NULL != video_surface) {
    IAtcSurface_setLayerZOrder(video_surface, 0);
    IAtcSurface_release(video_surface);
    video_surface = NULL;
  }
  if (vdec_fd >= 0)
  {
    int ret = close(vdec_fd);

    if (ret < 0)
    {
      PRINT_ERROR("[DTDEMO] close video decoder driver fail, %s\r\n", strerror(errno));
    }
    vdec_fd = (int)(-1);
  }

  return test_result;
}
