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

#include <stdbool.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <linux/types.h>
#include "atcsurface.h"
#include "atcvbufferpool.h"
#include "atcvsinkbuffer.h"
#include "atcdtlog.h"
#include "display.h"
#include "x_ver.h"

#define ATC_DTVIDBUF_INST_MOD "[atcdtvideobuf]"
#define ATC_DTVIDBUF_INST_MAJ  1
#define ATC_DTVIDBUF_INST_MIN  1
#define ATC_DTVIDBUF_INST_REV  1

#define  DEBUG_ATCSURFACE_RETCODE 0

enum  {
  atcbuf_idle,
  atcbuf_indec,
  atcbuf_inme
};

void atc_vsink_buffer_free (void *buf)
{
  ATCVSINKBUFFER *atcvsinkbuf = (ATCVSINKBUFFER *)buf;
  __s32 index = 0;

  if (atcvsinkbuf == NULL)
    return;

  if (atcvsinkbuf->bufctrl != NULL) {
    free(atcvsinkbuf->bufctrl);
  }

  index = atcvsinkbuf->index;

  atcvsinkbuf->bufctrl = NULL;
  atcvsinkbuf->data = NULL;
  atcvsinkbuf->size = 0;
  pthread_mutex_destroy(&atcvsinkbuf->lock);

  free(atcvsinkbuf);
  PRINT_DEBUG("(idx: %d) exit\r\n", index);
}

ATCVSINKBUFFER *
atc_vsink_buffer_new (void * owner,
  void *atcsurface, __s32 index,
  ATC_VSINK_FMT_INFO_T *format_info)
{
  ATCVSINKBUFFER *atcsinkbuf = NULL;
  atc_buffer_t *atcbuf = NULL;
  struct VOUT_PARAM *vid_out_data = NULL;
  __s32 ret = 0;

  MOD_VERSION_INFO(ATC_DTVIDBUF_INST_MOD,
    ATC_DTVIDBUF_INST_MAJ,
    ATC_DTVIDBUF_INST_MIN,
    ATC_DTVIDBUF_INST_REV);

  PRINT_DEBUG("new(idx: %d), pool(%p) enter\r\n",
    index, owner);

  if (NULL == owner) {
    PRINT_ERROR("fail for invalid args\r\n");
    return NULL;
  }

  atcsinkbuf = (ATCVSINKBUFFER *)malloc(sizeof(ATCVSINKBUFFER));
  if (NULL == atcsinkbuf) {
    PRINT_ERROR("fail for no memory\r\n");
    return NULL;
  }
  memset(atcsinkbuf, 0, sizeof(ATCVSINKBUFFER));

  pthread_mutex_init(&atcsinkbuf->lock, NULL);
  atcsinkbuf->index = -1;
  atcsinkbuf->format = 0;
  atcsinkbuf->fourcc = 0;
  atcsinkbuf->width  = 0;
  atcsinkbuf->height = 0;
  atcsinkbuf->stride = 0;
  atcsinkbuf->state = atcbuf_idle;
  atcsinkbuf->need_destroy = FALSE;
  atcsinkbuf->bufctrl = NULL;
  atcsinkbuf->data = NULL;
  atcsinkbuf->size = 0;
  atcsinkbuf->pool = NULL;

  ATC_SINK_BUFFER_LOCK(atcsinkbuf);

  atcsinkbuf->pool = owner;

  atcsinkbuf->index = index;
  atcsinkbuf->format = format_info->format;
  atcsinkbuf->fourcc = format_info->fourcc;
  atcsinkbuf->width  = format_info->width;
  atcsinkbuf->height = format_info->height;
  atcsinkbuf->stride = format_info->stride;

  atcsinkbuf->state = atcbuf_idle;

  PRINT_DEBUG("new --> format: 0x%08x\r\n",
    format_info->format);
  if (ATC_PIX_FMT_NV12M_PRIVATE1 == format_info->format) {
    PRINT_DEBUG("new --> g_malloc atc_buffer_t\r\n");
    if (atcsinkbuf->bufctrl != NULL) {
      free(atcsinkbuf->bufctrl);
      atcsinkbuf->bufctrl = NULL;
    }
    atcsinkbuf->bufctrl = malloc(sizeof(atc_buffer_t));
    if (NULL == atcsinkbuf->bufctrl) {
      PRINT_ERROR("new fail in g_malloc atc_buffer_t\r\n");
      ATC_SINK_BUFFER_UNLOCK(atcsinkbuf);
      atc_vsink_buffer_free(atcsinkbuf);
      return NULL;
    }
    memset(atcsinkbuf->bufctrl, 0, sizeof(atc_buffer_t));
  }

  PRINT_DEBUG("new --> IAtcSurface_dequeueBuffer\r\n");
  ret = IAtcSurface_dequeueBuffer((IAtcSurface *)atcsurface, (atc_buffer_t *)atcsinkbuf->bufctrl);
  if (ret != 0) {
    PRINT_TRACE("new (idx: %d) fail in IAtcSurface_dequeueBuffer\r\n",
      index);
    goto dequeuebuffail;
  }

  atcbuf = (atc_buffer_t *)(atcsinkbuf->bufctrl);

  if (NULL == atcbuf->bits) {
    PRINT_TRACE("new (idx: %d) fail for atcbuf->bits == NULL\r\n",
      index);
    goto dequeuebuffail;
  }

  atcsinkbuf->data = atcbuf->bits;

  atcsinkbuf->state = atcbuf_inme;

  if (ATC_PIX_FMT_NV12M_PRIVATE1 == format_info->format) {
    atcbuf->format = ATC_PIX_FMT_NV12M_PRIVATE1;
    vid_out_data = (struct VOUT_PARAM *)(atcbuf->bits);
    if (vid_out_data != NULL) {
      PRINT_TRACE("new(%p) --> dequeue pool buffer (index=%d), bufctrl=%p, bits=%p, y=0x%x, c=0x%x",
        atcsinkbuf, index, atcsinkbuf->bufctrl, vid_out_data,
        (unsigned int)(vid_out_data->y_phy_addr),
        (unsigned int)(vid_out_data->c_phy_addr));
      atcsinkbuf->size = sizeof(struct VOUT_PARAM);
    } else {
      PRINT_TRACE("new(%p) FATAL ERROR --> dequeue pool buffer (index=%d), bufctrl=%p, bits=(nil)",
        atcsinkbuf, index, atcsinkbuf->bufctrl);
      atcsinkbuf->size = 0;
			IAtcSurface_queueBuffer((IAtcSurface *)atcsurface, (atc_buffer_t *)atcsinkbuf->bufctrl);
			goto dequeuebuffail;
    }
  }

  ATC_SINK_BUFFER_UNLOCK(atcsinkbuf);
  PRINT_DEBUG("new(idx: %d) success, pool(%p), return %p\r\n",
    index, owner, atcsinkbuf);

  return atcsinkbuf;

  /* ERRORS */
dequeuebuffail:
  {
    ATC_SINK_BUFFER_UNLOCK(atcsinkbuf);
    atc_vsink_buffer_free(atcsinkbuf);
    return NULL;
  }
}


