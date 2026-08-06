#ifndef _DVP_HAL_H_
#define _DVP_HAL_H_

#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
//#include <tvd_drv_if.h>
#include <media/atc/wch_if.h>
#include "videodev2.h"

#define AUD_DEV_NAME    "/dev/adec"

#define DIR_FRONT  0x1
#define DIR_REAR   0x2

#define DVP_CAPTURE_MAX_DEVICES  1
#define DVP_VIDEO_INDEX     0

#define WCH_WIDTH  720
#define WCH_HEIGHT 480

int dvp_open_audio(bool IsFront);
int dvp_close_audio(bool IsFront);
int dvp_open_video(WCH_BUFF_INFO_T *bufferAddr, u32 length);
int dvp_close_video(void);
int dvp_config_video(void);

int registerfasync(struct fasync_struct *async_quene);

static void wch_buffer_dvpdone(u32 *bufindex);

extern bool ADE_IOControl(u32 context, u32 code, u8 *pInBuffer, u32 inSize,
                  u8 *pOutBuffer, u32 outSize, u32 *pOutSize);

extern bool ADE_Close(u32 dwContext);
extern u32 ADE_Open(u32 dwContext, u32 dwAccessMode, u32 dwShareMode);


int dvp_informAudioSampleRate(u8 *rate);
int dvp_getAudioSpectrum(void *spectrum, u32 length);
int dvp_getWcDVpIndex(void *index, u32 length);


#endif

