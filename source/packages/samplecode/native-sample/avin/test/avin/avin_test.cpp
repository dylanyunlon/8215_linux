#include <stdio.h>

#include <fcntl.h>
#include <unistd.h>

#include <linux/videodev2.h>
#include <avin.h>
#include <atcsurface.h>
#include <sys/ioctl.h>
#include <string.h>
#include "cap_prvt.h"


typedef struct _RECT {
    int32_t  left;
    int32_t  top;
    int32_t  right;
    int32_t  bottom;
} RECT;

typedef struct OVERLAY_PARAM
{
    u_int32_t y_phy_addr;
    u_int32_t c_phy_addr;
    u_int32_t device_name;
    //De-interlace interface
    u_int32_t duration;
    u_int32_t di_flags;
} OVERLAY_PARAM;



typedef struct _OVERLAY_PARAM2
{
    u_int32_t     index;
    u_int32_t     w;
    u_int32_t     h;
    RECT          src_rt;
    RECT          dst_rt;
    u_int32_t     y_phys_addr;
    u_int32_t     c_phys_addr;
    u_int32_t     status;
    u_int32_t     flags;
    u_int32_t     device_name;
    //De-interlace interface
    u_int32_t     duration;
    int32_t       fgProgSrc;
    int32_t   fgTopFiledFirst;
    int32_t   fgRepeatFirstField;// first field 1, and second field 0
    int32_t   fgProgSeq;         // Progressive sequece
    int32_t   fgPullDownFlagValid;
} OVERLAY_PARAM2;



int main(int argc, char **argv)
{
    int fd;
    atc_v4l2_capture   *capture = NULL;
    IAtcSurface *overlay;
    //struct v4l2_capability  cap;
    
    fd = open("/dev/ttyMT0", O_RDWR);
    dup2(fd, 0);
    dup2(fd, 1);
    dup2(fd, 2);
    close(fd);

    fprintf(stderr, "Oh, My God 1111111111111111111111111111111111\n");
    printf("avin_test - AAAAAAAAAAAAAAAAAAAAAAAAAAAaaaaaaaaaa\n");
    /*
    fd = open(V4L2_AVIN_DEV_NAME, O_RDWR); 
    if (0 > fd)
    {
        printf("avin_test - 1\r\n");
        return (-1);
    }
    if (0 > ioctl(fd, VIDIOC_QUERYCAP, &cap))
    {
        printf("avin_test - 2\r\n");
        return (-1);
    }
    if (!(cap.capabilities & (V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_AUDIO | 
          V4L2_CAP_STREAMING)))
    {
        return (-1);
    }
    */
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    // ***********************for audio test**************************
    /*
    struct v4l2_audio  audio;

    memset(&audio, 0, sizeof(audio));
    audio.index = 0;
    if (0 > ioctl(fd, VIDIOC_S_AUDIO, &audio)) {
        printf("avin_test - 3\r\n");
        return (-1);
    }
    
   
    if (0 > ioctl(fd, VIDIOC_STREAMON, &type)) {
        printf("avin_test - 4\r\n");
        return (-1);
    }
    */
    // ***********************for audio test end**************************
    printf("call CreateVideoSurface - VIDSURF_TYPE_DEFAULT, 720x480, PIX_FMT_NV12M_PRIVATE1\n");
    overlay = atc_createsurface(ATCSURF_TYPE_DEFAULT, 720, 480, ATC_PIX_FMT_NV12M_PRIVATE1);
    if (!overlay) {
        printf("main(): atc_createsurface failed \n");
    } else {
        IAtcSurface_setBufferCount(overlay, 4); 
        IAtcSurface_setLayerZOrder(overlay, 3);
    }
    
    printf("main(): call atc_createv4l2capture \n");
    capture = atc_createv4l2capture();
    
    if (!capture) {
        printf("main(): atc_createv4l2capture failed \n");
    } else {
        atc_buffer_t  buf;
        int i;
        printf("main(): atc_createv4l2capture  Success\n");
        avin_queueBuffer2(capture, &buf);
        
        if (avin_start(capture) < 0){
            printf("main(): avin_start  fail\n");
            return -1;
        }
        printf("main(): avin_start  Success\n");
        
        while (true) {
            if (0 > avin_dequeueBuffer(capture, &buf)) {
                printf("main(): avin_dequeueBuffer failed\n");
            } else {
                yc_addr_t *addr = (yc_addr_t *)buf.bits;  
                printf("main(): avin_dequeueBuffer success vb addr-> y= %d, c = %d\n yeah~ yeah~~~~~~ ", addr->y, addr->c);
                if (0 > avin_queueBuffer(capture, &buf)) {
                    printf("main(): avin_queueBuffer failed \n");
                } else {
                    printf(" avin_queueBuffer success f\n");
                }              

     //**************************************************************
     
                if (0 > IAtcSurface_dequeueBuffer(overlay, &buf)) {
                   printf("main(): IAtcSurface_dequeueBuffer failed T_T T_T T_T T_T T_T\n");
                } else {
                   OVERLAY_PARAM  *param = (OVERLAY_PARAM *)buf.bits;  
                   printf("video buffer info -> w = %d, h = %d, format = %d, data = %p\n", buf.width, buf.height, buf.format, buf.bits);
                   param->y_phy_addr= addr->y;
                   param->c_phy_addr= addr->c;
                   
                   buf.width = 720;
                   buf.height = 480;
                   if (0 > IAtcSurface_queueBuffer(overlay, &buf)) {
                       printf("main(): IVideoSurface_queueBuffer failed T_T T_T T_T T_TT_T\n");
                   } else {
                       printf("main(): IVideoSurface_queueBuffer success ^_^ ^_^ ^_^ ^_^ ^_^ ^_^\n");
                   }                 
                }
       //**************************************************************  
            }
        }
    }
    
    sleep(10);

    if (0 > ioctl(fd, VIDIOC_STREAMOFF, &type)) {
    return (-1);
    }
    close(fd);

    return (0);
}
