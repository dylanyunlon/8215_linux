#include "cap_prvt.h"

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>


/*static u_int32_t _get_fourcc_format(u_int32_t format)
{
    u_int32_t  fmt_fourcc = 0;
    
    if (ATC_PIX_FMT_RGB565 == format) {
        fmt_fourcc = V4L2_PIX_FMT_RGB565; 
    } else if (ATC_PIX_FMT_RGB888 == format) {
        fmt_fourcc = V4L2_PIX_FMT_RGB24; 
    } else if (ATC_PIX_FMT_RGB8888 == format) {
        fmt_fourcc = V4L2_PIX_FMT_RGB32;
    } else if (ATC_PIX_FMT_NV12M_PRIVATE1 == format) {
        fmt_fourcc = v4l2_fourcc('A', 'V', '1', '2');
    }

    return (fmt_fourcc);
}
*/


static int32_t _requestBuffers(atc_v4l2_capture *capture)
{
    //struct v4l2_format  fmt;
    struct v4l2_requestbuffers  reqbufs;
    struct v4l2_buffer   buffer;
    int  i;

    if (!capture) {
        return (-1);
    }
    /*
    fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    fmt.fmt.pix = capture->pix_fmt;
    if (0 > ioctl(capture->dev_fd, VIDIOC_S_FMT, &fmt)) {
        return (-1);
    }
        */
    printf(" _requestBuffers enter\n");
    memset(&reqbufs, 0, sizeof(reqbufs));
    reqbufs.count = capture->buf_count;
    reqbufs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbufs.memory = V4L2_MEMORY_MMAP;
    if (0 > ioctl(capture->dev_fd, VIDIOC_REQBUFS, &reqbufs)) {
        return (-1);
    }
    printf(" _requestBuffers after \n");
    capture->buf_count = reqbufs.count;
    capture->buffers =(atc_internal_buffer *) malloc(capture->buf_count * sizeof(atc_internal_buffer));
    if (!capture->buffers) {
        return (-1);
    }
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer.memory = V4L2_MEMORY_MMAP;
    memset(capture->buffers, 0, sizeof(capture->buf_count * sizeof(atc_internal_buffer)));
    for (i = 0; i < capture->buf_count; i++) 
    {
        buffer.index = i;
        if (0 > ioctl(capture->dev_fd, VIDIOC_QUERYBUF, &buffer)) {
            goto failed;
        }
        capture->buffers[i].data = mmap(NULL, buffer.length, PROT_READ | PROT_WRITE,
                 MAP_SHARED, capture->dev_fd, buffer.m.offset);
        if (MAP_FAILED == capture->buffers[i].data) {
            capture->buffers[i].data = NULL;
            printf("call mmap failed, line = %d\n", __LINE__);
            goto failed;
        }
        yc_addr_t *addr = (yc_addr_t *)(capture->buffers[i].data);
        addr->y = 0x12345678;
        addr->c = 0x23456789;
        
        printf("_requestBuffers - data pointer is %p, offset is %d, len is %d\n", capture->buffers[i].data, buffer.m.offset, buffer.length);
        capture->buffers[i].len = buffer.length;
    }
    capture->buffer_requested = 1;
    
    return (0);

failed:
    return (-1);
}

void  avin_selectport(atc_v4l2_capture *capture)
{
}
int  avin_start(atc_v4l2_capture *capture)
{
    int  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    
    if (!capture) {
        printf("avin_start 1\n");
        return (-1);
    }
    if (0 > capture->dev_fd) {
        printf("avin_start 2\n");
        return (-1);
    }
    if (capture->cap.capabilities & V4L2_CAP_STREAMING) {
        printf("avin_start 3\n");
        if (0 > ioctl(capture->dev_fd, VIDIOC_STREAMON, &type)) {
            printf("avin_start 4\n");
            return (-1);
        }
        printf("avin_start 4\n");
    }

    return (0);
}
int  avin_dequeueBuffer(atc_v4l2_capture *capture, atc_buffer_t *buf)
{
    
    struct v4l2_buffer  buffer;
    
    if (!capture) {
        return (-1);
    }

    memset(&buffer, 0, sizeof(struct v4l2_buffer));
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; 
    buffer.memory = V4L2_MEMORY_MMAP;
    printf("_dequeueBuffer 1\n");
    if (0 > ioctl(capture->dev_fd, VIDIOC_DQBUF, &buffer)) {
        printf("call ioctl VIDIOC_DQBUF failed, line = %d\n", __LINE__);
        return (-1);
    }
    
    buf->width = 720;
    buf->height = 576;
    buf->format = 0;
    buf->stride = 720 * 2;
    buf->bits = capture->buffers[buffer.index].data;
    printf("_dequeueBuffer - w = %d, h = %d, format = %d, fourcc = %d\n", buf->width, buf->height, buf->format, buf->fmt_fourcc); 
    
    return (0);
}

int  avin_queueBuffer(atc_v4l2_capture *capture, atc_buffer_t *buf)
{
        struct  v4l2_buffer  buffer;
        //struct    v4l2_format  fmt;
        //int   need_s_fmt = 0;
        int     i, buf_idx = -1;
        if (!capture) {
            return (-1);
        }
        
        for (i = 0; i < capture->buf_count; i++) {
            if (capture->buffers[i].data == buf->bits) {
                buf_idx = i;
            }
        }
        if (buf_idx < 0) {
            return (-1);
        }
        printf("_queueBuffer - w = %d, h = %d, format = %d\n", buf->width, buf->height, buf->format);

        printf("_queueBuffer -> index = %d\n", buf_idx);
        buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer.memory = V4L2_MEMORY_MMAP;
        buffer.index = buf_idx;
        if (0 > ioctl(capture->dev_fd, VIDIOC_QBUF, &buffer)) {
            return (-1);
        }
        
    return (0);

}

int  avin_queueBuffer2(atc_v4l2_capture *capture, atc_buffer_t *buf)
{
    struct  v4l2_buffer  buffer;
    //struct    v4l2_format  fmt;
    //int   need_s_fmt = 0;
    int     i, buf_idx = 0;
    if (!capture) {
        return (-1);
    }
    
    for (i = 0; i < capture->buf_count; i++) {
        printf("avin_queueBuffer2 -> index = %d\n", i);
        buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer.memory = V4L2_MEMORY_MMAP;
        buffer.index = i;
        if (0 > ioctl(capture->dev_fd, VIDIOC_QBUF, &buffer)) {
            return (-1);
        }
        
    }
        
    return (0);

}

int  avin_stop(atc_v4l2_capture *capture)
{
    int  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    
    if (!capture) {
        return (-1);
    }
    if (0 > capture->dev_fd) {
        return (-1);
    }
    if (capture->cap.capabilities & V4L2_CAP_STREAMING) {
        if (0 > ioctl(capture->dev_fd, VIDIOC_STREAMOFF, &type)) {
            return (-1);
        }
    }

    return (0);
}

atc_v4l2_capture *atc_createv4l2capture(void)
{
    atc_v4l2_capture  *capture = NULL;
    struct v4l2_format fmt;

    capture = (atc_v4l2_capture *)malloc(sizeof(atc_v4l2_capture)); 
    if (!capture)
    {
        return (NULL);
    }
    capture->dev_fd = open(V4L2_AVIN_DEV_NAME, O_RDWR); 
    if (0 > capture->dev_fd)
    {
        goto failed;
    }
    if (0 > ioctl(capture->dev_fd, VIDIOC_QUERYCAP, &capture->cap))
    {
        goto failed;
    }
    if (!(capture->cap.capabilities & (V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING)))
    {
        printf(" atc_createv4l2capture 3  \n");
        goto failed;
    }
    printf(" atc_createv4l2capture 4 \n");
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (0 > ioctl(capture->dev_fd, VIDIOC_G_FMT, &fmt)) {
        capture->pix_fmt.width = 0;
        capture->pix_fmt.height = 0;
        capture->pix_fmt.pixelformat = v4l2_fourcc('A', 'V', '1', '2');
        capture->pix_fmt.field = V4L2_FIELD_NONE;
        capture->pix_fmt.bytesperline = 0;
        capture->pix_fmt.sizeimage = 0;
        capture->pix_fmt.colorspace = V4L2_COLORSPACE_JPEG; 
        capture->pix_fmt.priv = 0; 
    } else {
        capture->pix_fmt = fmt.fmt.pix;
    }
    
    pthread_mutex_init(&capture->lock, NULL);
    capture->buf_count = 5;
    capture->streaming = 0;
    capture->buffers = NULL;
    capture->buffer_requested = 0;

    capture->count = 1;

    if (!capture->buffer_requested) {
        printf(" before  _requestBuffers  \n");
        int ret = _requestBuffers(capture);
        if (0 > ret) {
            goto failed;
        }
    }
   
    return (capture);

failed:
    if (capture->dev_fd >= 0)
    {
        close(capture->dev_fd);
    }
    free(capture);

    return (NULL);
}
