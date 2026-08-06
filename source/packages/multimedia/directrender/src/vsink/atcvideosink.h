/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * AutoChips Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */

#ifndef _ATC_3RDPARTY_VSINK_H_
#define _ATC_3RDPARTY_VSINK_H_
#include <stdbool.h>

#include "atcsurface.h"

typedef struct {
  __s32 use_count; /* buffer count used, min_count <= use_count <= max_count */
  __s32 max_count; /* max buffer count can be used */
  __s32 min_count; /* min buffer count can be used */
  __s32 width;        /* video width */
  __s32 height;       /* video height */
  __u32 stride;       /* video stride */
  __u64 support_fmts; /* video sink support formats, it must >= ((__u64)1 << format) */
  __u32 format;       /* video sink current format, value refers to ATC_PIX_FMT_.. defined in atcsurface.h */
  __u32 fourcc;       /* video sink current format fourcc */
  bool  interlaced;   /* designate whether video data is interaced  */
} ATC_VSINK_CFG_T;

typedef struct {
  __s32 width;        /* video width */
  __s32 height;       /* video height */
  __u32 stride;       /* video stride */
  __u32 format;       /* video sink current format, value refers to ATC_PIX_FMT_.. defined in atcsurface.h */
  __u32 fourcc;       /* video sink current format fourcc */
  bool  interlaced;   /* designate whether video data is interaced  */
} ATC_VSINK_FMT_INFO_T;

#ifdef __cplusplus
extern "C" {
#endif

void *atc_video_sink_open(void);
bool atc_video_sink_set_surface(void *inst, void *surface);
bool atc_video_sink_get_config(void *inst, ATC_VSINK_CFG_T *prCfg);
bool atc_video_sink_set_format (void *inst, ATC_VSINK_FMT_INFO_T *prFormat);
bool atc_video_sink_set_buffer_count(void *inst, __s32 count);
bool atc_video_sink_start(void *inst);
bool atc_video_sink_dequeue_buffer(void *inst,
  void **ppBuffer, __u32 *pBufSz);
bool atc_video_sink_queue_buffer(void *inst, void *pbuffer, __u32 bufsz,
  __u32 datasz);
bool atc_video_sink_cancel_buffer(void *inst, void *pbuffer, __u32 bufsz);
bool atc_video_sink_stop(void *inst);
bool atc_video_sink_close(void *inst);

#ifdef __cplusplus
}
#endif

#endif /* _ATC_3RDPARTY_VSINK_H_ */


