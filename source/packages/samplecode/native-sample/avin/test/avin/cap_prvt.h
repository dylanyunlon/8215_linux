#ifndef _CAP_PRVT_H_
#define _CAP_PRVT_H_

#include <atcsurface.h>
#include <linux/videodev2.h>
#include <pthread.h>

#define V4L2_AVIN_DEV_NAME  "/dev/video10"

typedef struct _atc_internal_buffer {
    void        *data;
    int          len;
} atc_internal_buffer;

typedef struct _atc_v4l2_capture {
    pthread_mutex_t lock;
    int count;     
    int dev_fd;
    struct v4l2_capability  cap;
    struct v4l2_pix_format  pix_fmt;
    int buf_count;
    atc_internal_buffer *buffers;
    int  buffer_requested;
    int  streaming;
} atc_v4l2_capture;

typedef struct yc_addr_t {
	__u32 y;
	__u32 c;
}yc_addr_t;

extern "C" atc_v4l2_capture *atc_createv4l2capture(void);

void              avin_selectport(atc_v4l2_capture *capture);
int              avin_start(atc_v4l2_capture *capture);
int              avin_dequeueBuffer(atc_v4l2_capture *capture, atc_buffer_t *buf);
int              avin_queueBuffer(atc_v4l2_capture *capture, atc_buffer_t *buf);
int              avin_queueBuffer2(atc_v4l2_capture *capture, atc_buffer_t *buf);
int              avin_stop(atc_v4l2_capture *capture);


#endif //_VIDSURF_PRVT_H_
