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
#include <linux/types.h>
#include <string.h>
#include "atcsurface.h"
#include "display.h"
#include "atcvsinkbuffer.h"
#include "atcvbufferpool.h"
#include "atcvideosink.h"
#include "atcdtlog.h"
#include <stdio.h>
#include "x_ver.h"

#define ATC_DTVSINK_INST_MOD "[atcdtvsink]"
#define ATC_DTVSINK_INST_MAJ  1
#define ATC_DTVSINK_INST_MIN  1
#define ATC_DTVSINK_INST_REV  1

#define ATC_VSINK_MAX_BUF_CNT     8
#define ATC_VSINK_MIN_BUF_CNT     3
#define ATC_VSINK_DEFAULT_BUF_CNT 3

#define ATC_VSINK_MIN_LATENCY             2
#define ATC_VSINK_MIN_LATENCY_INTERLACED  4

typedef struct _AtcVideoSinkInst ATCVSINKINST;

typedef enum _AtcVideoSinkState{
  ATC_VSINK_ST_IDLE,
  ATC_VSINK_ST_INITED,
  ATC_VSINK_ST_STMON,
  ATC_VSINK_ST_STMOFFING,
  ATC_VSINK_ST_STMOFF,
  ATC_VSINK_ST_CLOSING,
} ATCVSINKINSTSTATE;

struct _AtcVideoSinkInst {
  bool need_setsurface;
  ATC_VSINK_CFG_T cfg_info;
  __s32 min_queued_bufs;
  void *atcsurface;
  void *pool;
  ATCVSINKINSTSTATE state;
  pthread_mutex_t lock;
};

#define ATC_VSINKINST_LOCK(vsink)  pthread_mutex_lock(&(vsink->lock))
#define ATC_VSINKINST_UNLOCK(vsink)   pthread_mutex_unlock(&(vsink->lock))

bool atc_video_sink_set_surface(void *inst, void *surface)
{
  ATCVSINKINST *atcvideosink = (ATCVSINKINST *)(inst);

  PRINT_TRACE("enter, inst: %p, surface: %p\r\n",
    inst, surface);
  if ((atcvideosink == NULL) || (surface == NULL)) {
    PRINT_ERROR("fail for invalid args\r\n");
    return FALSE;
  }

  ATC_VSINKINST_LOCK(atcvideosink);

  switch (atcvideosink->state) {
  case ATC_VSINK_ST_IDLE:
  case ATC_VSINK_ST_INITED:
    if (NULL != atcvideosink->pool) {
      PRINT_INFO("atc_vsink_buffer_pool_set_surface\r\n", inst, surface);
      atc_vsink_buffer_pool_set_surface(atcvideosink->pool, surface);
    }
    PRINT_INFO(" -- IAtcSurface_addRef\r\n");
    IAtcSurface_addRef((IAtcSurface *)surface);
    atcvideosink->atcsurface = surface;
    PRINT_TRACE("success, inst: %p, surface: %p, %p\r\n", inst, surface,
      atcvideosink->atcsurface);
    ATC_VSINKINST_UNLOCK(atcvideosink);
    return TRUE;
  default:
    break;
  }

  if (atcvideosink->atcsurface == surface) {
    ATC_VSINKINST_UNLOCK(atcvideosink);
    PRINT_TRACE("success, inst: %p, surface: %p\r\n",
      inst, surface);
    return TRUE;
  }

  PRINT_ERROR("fail for can't set surface while state is %d\r\n",
    atcvideosink->state);

  ATC_VSINKINST_UNLOCK(atcvideosink);

  return FALSE;
}

bool atc_video_sink_set_buffer_count(void *inst, __s32 count)
{
  ATCVSINKINST *atcvideosink = (ATCVSINKINST *)(inst);

  PRINT_TRACE("enter, count: %d\r\n", count);

  if ((NULL == atcvideosink) ||
    (0 == count)) {
    PRINT_ERROR("fail for invalid args\r\n");
    return FALSE;
  }

  if ((count < atcvideosink->cfg_info.min_count) ||
    (count > atcvideosink->cfg_info.max_count)) {
    PRINT_ERROR("fail for invalid count(%d), (max: %d, min: %d)\r\n",
      count,
      atcvideosink->cfg_info.max_count,
      atcvideosink->cfg_info.min_count);
    return FALSE;
  }

  ATC_VSINKINST_LOCK(atcvideosink);

  switch (atcvideosink->state) {
  case ATC_VSINK_ST_IDLE:
  case ATC_VSINK_ST_INITED:
    atcvideosink->cfg_info.use_count = count;
    ATC_VSINKINST_UNLOCK(atcvideosink);
    PRINT_TRACE("success, count: %d\r\n", count);
    return TRUE;
  default:
    break;
  }

  PRINT_ERROR("fail for can't set buffer count while state is %d\r\n",
    atcvideosink->state);

  ATC_VSINKINST_UNLOCK(atcvideosink);

  return FALSE;
}

bool atc_video_sink_dequeue_buffer(void *inst,
  void **ppBuffer, __u32 *pBufSz)
{
  ATCVSINKINST *atcvideosink = (ATCVSINKINST *)(inst);

  PRINT_DEBUG("enter\r\n");

  if ((NULL == atcvideosink) ||
    (NULL == ppBuffer) ||
    (NULL == pBufSz)) {
    PRINT_ERROR("fail for invalid args\r\n");
    return FALSE;
  }

  *ppBuffer = NULL;
  *pBufSz = 0;

  /* initialize the buffer pool if not initialized yet (first buffer): */
  ATC_VSINKINST_LOCK(atcvideosink);

  switch (atcvideosink->state) {
  case ATC_VSINK_ST_INITED:
  case ATC_VSINK_ST_STMOFF:
  case ATC_VSINK_ST_STMON:
    break;
  case ATC_VSINK_ST_IDLE:
  case ATC_VSINK_ST_STMOFFING:
  case ATC_VSINK_ST_CLOSING:
  default:
    PRINT_ERROR("fail for state(%d) error, can't dequeue buffer\r\n",
      atcvideosink->state);
    ATC_VSINKINST_UNLOCK(atcvideosink);
    return FALSE;
  }

  if (NULL == atcvideosink->pool) {
    ATC_VSINK_FMT_INFO_T rFormat;

    if (NULL == atcvideosink->atcsurface) {
      PRINT_ERROR("fail for no surface has been set\r\n");
      ATC_VSINKINST_UNLOCK(atcvideosink);
      return FALSE;
    }

    atcvideosink->pool = atc_vsink_buffer_pool_new (atcvideosink);
    if (NULL == atcvideosink->pool) {
      PRINT_ERROR("fail in atc_sink_buffer_pool_new\r\n");
      ATC_VSINKINST_UNLOCK(atcvideosink);
      return FALSE;
    }

    memset(&rFormat, 0, sizeof(rFormat));
    rFormat.format = atcvideosink->cfg_info.format;
    rFormat.height = atcvideosink->cfg_info.height;
    rFormat.width  = atcvideosink->cfg_info.width;
    rFormat.stride = atcvideosink->cfg_info.stride;
    rFormat.fourcc = atcvideosink->cfg_info.fourcc;

    atc_vsink_buffer_pool_set_surface(atcvideosink->pool, atcvideosink->atcsurface);

    PRINT_DEBUG("--> atc_sink_buffer_pool_init_buffers\r\n");
    if (!atc_vsink_buffer_pool_init_buffers (atcvideosink->pool,
        atcvideosink->cfg_info.use_count, &rFormat)) {
      PRINT_ERROR("fail in atc_vsink_buffer_pool_init_buffers\r\n");
      ATC_VSINKINST_UNLOCK(atcvideosink);
      return FALSE;
    }
  }

  atcvideosink->state = ATC_VSINK_ST_STMON;

  ATC_VSINKINST_UNLOCK(atcvideosink);

  if (!atc_vsink_buffer_pool_get (atcvideosink->pool, TRUE, ppBuffer, pBufSz)) {
    PRINT_ERROR("fail in atc_vsink_buffer_pool_get\r\n");
    return FALSE;
  }

  PRINT_DEBUG("success, buffer: %p\r\n", *ppBuffer);

  return TRUE;
}

bool atc_video_sink_queue_buffer(void *inst, void *pbuffer,
  __u32 bufsz, __u32 datasz)
{
  ATCVSINKINST *atcvideosink = (ATCVSINKINST *)(inst);
  bool is_need_streamoff = FALSE;

  if ((NULL == atcvideosink) ||
    (NULL == pbuffer) ||
    (0 == bufsz) ||
    (0 == datasz)) {
    PRINT_ERROR("fail for invalid args\r\n");
    return FALSE;
  }

  ATC_VSINKINST_LOCK(atcvideosink);
  switch (atcvideosink->state) {
  case ATC_VSINK_ST_INITED:
  case ATC_VSINK_ST_STMON:
    break;
  case ATC_VSINK_ST_STMOFFING:
  case ATC_VSINK_ST_CLOSING:
    is_need_streamoff = TRUE;
    break;
  case ATC_VSINK_ST_IDLE:
  case ATC_VSINK_ST_STMOFF:
  default:
    PRINT_ERROR("fail for state(%d) error, can't queue buffer\r\n",
      atcvideosink->state);
    ATC_VSINKINST_UNLOCK(atcvideosink);
    return FALSE;
  }
  ATC_VSINKINST_UNLOCK(atcvideosink);

  PRINT_DEBUG("--> atc_sink_buffer_pool_qbuf\r\n");
  if (!atc_vsink_buffer_pool_qbuf (atcvideosink->pool, pbuffer, datasz)) {
    PRINT_ERROR("fail in atc_sink_buffer_pool_qbuf\r\n");
    return FALSE;
  }

  if (is_need_streamoff) {
    if (!atc_vsink_buffer_pool_recycle_buffers(atcvideosink->pool)) {
      PRINT_TRACE("STREAMOFFING, some buffers hasn't been recycled\r\n");
      return TRUE;
    }

    ATC_VSINKINST_LOCK(atcvideosink);
    if (ATC_VSINK_ST_CLOSING == atcvideosink->state) {
      ATC_VSINKINST_UNLOCK(atcvideosink);
      PRINT_TRACE("CLOSING, call atc_video_sink_close\r\n");
      atc_video_sink_close(atcvideosink);
      return TRUE;
    }
    atcvideosink->state = ATC_VSINK_ST_STMOFF;
    PRINT_TRACE("exit, STMOFF\r\n");
    ATC_VSINKINST_UNLOCK(atcvideosink);
    return TRUE;
  }

  PRINT_DEBUG("show_frame -->avail_buffer_count = %d\r\n",
    atc_vsink_buffer_pool_available_buffers(atcvideosink->pool));
  if (atc_vsink_buffer_pool_available_buffers(atcvideosink->pool) >
    atcvideosink->min_queued_bufs) {
    PRINT_DEBUG("--> atc_sink_buffer_pool_dqbuf\r\n");
    atc_vsink_buffer_pool_dqbuf (atcvideosink->pool);
  }

  return TRUE;
}

bool atc_video_sink_cancel_buffer(void *inst, void *pbuffer, __u32 bufsz)
{
  ATCVSINKINST *atcvideosink = (ATCVSINKINST *)(inst);
  bool is_need_streamoff = FALSE;

  if ((NULL == atcvideosink) || (NULL == pbuffer) || (0 == bufsz)) {
    PRINT_ERROR("fail for invalid args\r\n");
    return FALSE;
  }

  ATC_VSINKINST_LOCK(atcvideosink);
  switch (atcvideosink->state) {
  case ATC_VSINK_ST_INITED:
  case ATC_VSINK_ST_STMON:
    break;
  case ATC_VSINK_ST_STMOFFING:
  case ATC_VSINK_ST_CLOSING:
    is_need_streamoff = TRUE;
    break;
  case ATC_VSINK_ST_IDLE:
  case ATC_VSINK_ST_STMOFF:
  default:
    PRINT_ERROR("fail for state(%d) error, can't queue buffer\r\n",
      atcvideosink->state);
    ATC_VSINKINST_UNLOCK(atcvideosink);
    return FALSE;
  }
  ATC_VSINKINST_UNLOCK(atcvideosink);

  if (!atc_vsink_buffer_pool_unshow (atcvideosink->pool, pbuffer, bufsz)) {
    PRINT_ERROR("fail in atc_sink_buffer_pool_unshow\r\n");
    return FALSE;
  }

  if (is_need_streamoff) {
    if (!atc_vsink_buffer_pool_recycle_buffers(atcvideosink->pool)) {
      PRINT_TRACE("STREAMOFFING, some buffers hasn't been recycled\r\n");
      return TRUE;
    }

    ATC_VSINKINST_LOCK(atcvideosink);
    if (ATC_VSINK_ST_CLOSING == atcvideosink->state) {
      ATC_VSINKINST_UNLOCK(atcvideosink);
      PRINT_TRACE("CLOSING, call atc_video_sink_close\r\n");
      atc_video_sink_close(atcvideosink);
      return TRUE;
    }
    atcvideosink->state = ATC_VSINK_ST_STMOFF;
    PRINT_TRACE("exit, STMOFF\r\n");
    ATC_VSINKINST_UNLOCK(atcvideosink);
  }

  return TRUE;
}

void *atc_video_sink_open(void)
{
  ATCVSINKINST *atcvideosink = NULL;

  MOD_VERSION_INFO(ATC_DTVSINK_INST_MOD,
    ATC_DTVSINK_INST_MAJ,
    ATC_DTVSINK_INST_MIN,
    ATC_DTVSINK_INST_REV);

  PRINT_TRACE("enter\r\n");
  atcvideosink = (ATCVSINKINST *)malloc(sizeof(ATCVSINKINST));
  if (NULL == atcvideosink) {
    PRINT_ERROR("fail for no memory\r\n");
    return NULL;
  }

  memset(atcvideosink, 0, sizeof(ATCVSINKINST));

  atcvideosink->cfg_info.max_count = ATC_VSINK_MAX_BUF_CNT;
  atcvideosink->cfg_info.min_count = ATC_VSINK_MIN_BUF_CNT;
  atcvideosink->cfg_info.use_count = ATC_VSINK_DEFAULT_BUF_CNT;
  atcvideosink->min_queued_bufs = ATC_VSINK_MIN_LATENCY;
  atcvideosink->cfg_info.width = 0;
  atcvideosink->cfg_info.height = 0;
  atcvideosink->cfg_info.stride = 0;
  atcvideosink->cfg_info.support_fmts = (1 << ATC_PIX_FMT_NV12M_PRIVATE1);
  atcvideosink->cfg_info.format = ATC_PIX_FMT_NV12M_PRIVATE1;
  atcvideosink->cfg_info.interlaced = FALSE;
  atcvideosink->atcsurface = NULL;
  atcvideosink->pool = NULL;

  atcvideosink->state = ATC_VSINK_ST_IDLE;

  pthread_mutex_init(&(atcvideosink->lock), NULL);
  PRINT_TRACE("exit, success, return %p, state: %d\r\n",
    atcvideosink, atcvideosink->state);

  return atcvideosink;
}

bool atc_video_sink_start(void *inst)
{
  ATCVSINKINST *atcvideosink = (ATCVSINKINST *)(inst);

  if (NULL == atcvideosink) {
    PRINT_ERROR("fail for invalid args\r\n");
    return FALSE;
  }

  ATC_VSINKINST_LOCK(atcvideosink);
  PRINT_TRACE("enter, state: %d\r\n", atcvideosink->state);

  switch (atcvideosink->state) {
  case ATC_VSINK_ST_INITED:
  case ATC_VSINK_ST_STMOFF:
    break;
  case ATC_VSINK_ST_IDLE:
  case ATC_VSINK_ST_STMOFFING:
  case ATC_VSINK_ST_STMON:
  case ATC_VSINK_ST_CLOSING:
  default:
    PRINT_ERROR("fail for state is STMOFFING, can't reStart\r\n");
    ATC_VSINKINST_UNLOCK(atcvideosink);
    return FALSE;
  }

  if (NULL == atcvideosink->pool) {
    ATC_VSINK_FMT_INFO_T rFormat;

    if (NULL == atcvideosink->atcsurface) {
      PRINT_ERROR("fail for no surface has been set\r\n");
      ATC_VSINKINST_UNLOCK(atcvideosink);
      return FALSE;
    }

    atcvideosink->pool = atc_vsink_buffer_pool_new (atcvideosink);
    if (NULL == atcvideosink->pool) {
      PRINT_ERROR("--> fail in atc_sink_buffer_pool_new\r\n");
      ATC_VSINKINST_UNLOCK(atcvideosink);
      return FALSE;
    }

    memset(&rFormat, 0, sizeof(rFormat));
    rFormat.format = atcvideosink->cfg_info.format;
    rFormat.height = atcvideosink->cfg_info.height;
    rFormat.width = atcvideosink->cfg_info.width;
    rFormat.stride = atcvideosink->cfg_info.stride;
    rFormat.fourcc = atcvideosink->cfg_info.fourcc;

    PRINT_INFO("--> pool_set_surface(%p, %p)\r\n",
			atcvideosink->pool, atcvideosink->atcsurface);
    atc_vsink_buffer_pool_set_surface(atcvideosink->pool,
      atcvideosink->atcsurface);

    PRINT_DEBUG("--> atc_sink_buffer_pool_init_buffers\r\n");
    if (!atc_vsink_buffer_pool_init_buffers (atcvideosink->pool,
        atcvideosink->cfg_info.use_count, &rFormat)) {
      PRINT_ERROR("--> fail in atc_vsink_buffer_pool_init_buffers\r\n");
      ATC_VSINKINST_UNLOCK(atcvideosink);
      return FALSE;
    }
  }

  PRINT_TRACE("exit, success, state: %d\r\n", atcvideosink->state);
  ATC_VSINKINST_UNLOCK(atcvideosink);

  return TRUE;
}

bool atc_video_sink_stop(void *inst)
{
  ATCVSINKINST *atcvideosink = (ATCVSINKINST *)(inst);

  if (NULL == atcvideosink) {
    PRINT_ERROR("fail for invalid args\r\n");
    return FALSE;
  }

  ATC_VSINKINST_LOCK(atcvideosink);
  PRINT_TRACE("enter, state: %d\r\n",
    atcvideosink->state);
  switch (atcvideosink->state) {
  case ATC_VSINK_ST_STMON:
    atcvideosink->state = ATC_VSINK_ST_STMOFFING;
    break;
  case ATC_VSINK_ST_CLOSING:
    PRINT_ERROR("exit for in closing now\r\n");
    ATC_VSINKINST_UNLOCK(atcvideosink);
    return TRUE;
  case ATC_VSINK_ST_IDLE:
  case ATC_VSINK_ST_INITED:
  case ATC_VSINK_ST_STMOFF:
  case ATC_VSINK_ST_STMOFFING:
    PRINT_ERROR("exit, state is %d\r\n",
      atcvideosink->state);
    ATC_VSINKINST_UNLOCK(atcvideosink);
    return TRUE;
  default:
    PRINT_ERROR("fail for invalid state: %d, can't stop\r\n",
      atcvideosink->state);
    ATC_VSINKINST_UNLOCK(atcvideosink);
    return FALSE;
  }

  if (NULL == atcvideosink->pool) {
    atcvideosink->state = ATC_VSINK_ST_STMOFF;
    PRINT_TRACE("exit, STMOFF, no pool\r\n");
    ATC_VSINKINST_UNLOCK(atcvideosink);
    return TRUE;
  }

  ATC_VSINKINST_UNLOCK(atcvideosink);

  if (!atc_vsink_buffer_pool_recycle_buffers(atcvideosink->pool)) {
    PRINT_TRACE("STREAMOFFING, some buffers hasn't been recycled\r\n");
    return TRUE;
  }

  ATC_VSINKINST_LOCK(atcvideosink);
  atcvideosink->state = ATC_VSINK_ST_STMOFF;
  PRINT_TRACE("exit, STMOFF\r\n");
  ATC_VSINKINST_UNLOCK(atcvideosink);

  return TRUE;
}

bool atc_video_sink_close(void *inst)
{
  ATCVSINKINST *atcvideosink = (ATCVSINKINST *)inst;

  if (atcvideosink == NULL) {
    PRINT_ERROR("fail for invalid args\r\n");
    return FALSE;
  }

  ATC_VSINKINST_LOCK(atcvideosink);
  PRINT_TRACE("enter, state: %d\r\n",
    atcvideosink->state);
  switch (atcvideosink->state) {
  case ATC_VSINK_ST_IDLE:
  case ATC_VSINK_ST_INITED:
    if (NULL != atcvideosink->pool) {
      if (!atc_vsink_buffer_pool_recycle_buffers(atcvideosink->pool)) {
        PRINT_ERROR("fail, state: %d, some buffers hasn't been recycled\r\n",
          atcvideosink->state);
        ATC_VSINKINST_UNLOCK(atcvideosink);
        return FALSE;
      }
      atc_vsink_buffer_pool_free (atcvideosink->pool);
      atcvideosink->pool = NULL;
    }
    if (atcvideosink->atcsurface) {
      PRINT_INFO("-- IAtcSurface_release\r\n");
      IAtcSurface_release((IAtcSurface *)(atcvideosink->atcsurface));
      atcvideosink->atcsurface = NULL;
    }
    atcvideosink->state = ATC_VSINK_ST_CLOSING;
    ATC_VSINKINST_UNLOCK(atcvideosink);
    pthread_mutex_destroy(&(atcvideosink->lock));
    free(atcvideosink);
    return TRUE;
  default:
    break;
  }

  atcvideosink->state = ATC_VSINK_ST_CLOSING;

  ATC_VSINKINST_UNLOCK(atcvideosink);

  if (NULL != atcvideosink->pool) {
    if (!atc_vsink_buffer_pool_recycle_buffers(atcvideosink->pool)) {
      PRINT_TRACE("STREAMOFFING, some buffers hasn't been recycled\r\n");
      return FALSE;
    }
    atc_vsink_buffer_pool_free (atcvideosink->pool);
    atcvideosink->pool = NULL;
  }

  ATC_VSINKINST_LOCK(atcvideosink);
  if (atcvideosink->atcsurface) {
    PRINT_INFO("-- IAtcSurface_release\r\n");
    IAtcSurface_release((IAtcSurface *)(atcvideosink->atcsurface));
    atcvideosink->atcsurface = NULL;
  }
  PRINT_TRACE("exit, success\r\n",
    atcvideosink->state);
  ATC_VSINKINST_UNLOCK(atcvideosink);
  pthread_mutex_destroy(&(atcvideosink->lock));
  free(atcvideosink);

  return TRUE;
}

bool atc_video_sink_get_config(void *inst, ATC_VSINK_CFG_T *prCfg)
{
  ATCVSINKINST *atcvideosink = (ATCVSINKINST *)(inst);

  if ((NULL == atcvideosink) || (NULL == prCfg)) {
    PRINT_ERROR("fail for invalid args\r\n");
    return FALSE;
  }
  PRINT_TRACE("enter\r\n");
  prCfg->use_count = atcvideosink->cfg_info.use_count;
  prCfg->max_count = atcvideosink->cfg_info.max_count;
  prCfg->min_count = atcvideosink->cfg_info.min_count;
  prCfg->width = atcvideosink->cfg_info.width;
  prCfg->height = atcvideosink->cfg_info.height;
  prCfg->support_fmts = atcvideosink->cfg_info.support_fmts;
  prCfg->format = atcvideosink->cfg_info.format;
  PRINT_TRACE("exit\r\n");

  return TRUE;
}

bool atc_video_sink_set_format (void *inst, ATC_VSINK_FMT_INFO_T *prFormat)
{
  ATCVSINKINST *atcvideosink = (ATCVSINKINST *)(inst);

  if ((NULL == atcvideosink) || (NULL == prFormat)) {
    PRINT_ERROR("fail for invalid args\r\n");
    return FALSE;
  }

  ATC_VSINKINST_LOCK(atcvideosink);
  PRINT_TRACE("enter, state: %d\r\n", atcvideosink->state);

  switch (atcvideosink->state) {
  case ATC_VSINK_ST_IDLE:
    break;
  case ATC_VSINK_ST_INITED:
  case ATC_VSINK_ST_STMON:
  case ATC_VSINK_ST_STMOFF:
    if ((0 != atcvideosink->cfg_info.format) &&
      (atcvideosink->cfg_info.format != prFormat->format)) {
      PRINT_ERROR("fail for format already has been set\r\n");
      ATC_VSINKINST_UNLOCK(atcvideosink);
      return FALSE;
    }
    break;
  case ATC_VSINK_ST_STMOFFING:
  case ATC_VSINK_ST_CLOSING:
  default:
    PRINT_ERROR("fail for format already has been set\r\n");
    ATC_VSINKINST_UNLOCK(atcvideosink);
    return FALSE;
  }

  if (NULL != atcvideosink->pool) {
    PRINT_TRACE("--> atc_vsink_buffer_pool_reset(pool: %p)\r\n",
      atcvideosink->pool);
    if (!atc_vsink_buffer_pool_reset(atcvideosink->pool, prFormat)) {
      PRINT_ERROR("fail in atc_vsink_buffer_pool_reset(pool: %p)\r\n",
        atcvideosink->pool);
      ATC_VSINKINST_UNLOCK(atcvideosink);
      return FALSE;
    }
  }

  atcvideosink->cfg_info.format = prFormat->format;
  atcvideosink->cfg_info.width  = prFormat->width;
  atcvideosink->cfg_info.height = prFormat->height;
  atcvideosink->cfg_info.stride = prFormat->stride;
  atcvideosink->cfg_info.fourcc = prFormat->fourcc;
  atcvideosink->cfg_info.interlaced = prFormat->interlaced;
  if (!atcvideosink->cfg_info.interlaced) {
    atcvideosink->min_queued_bufs = ATC_VSINK_MIN_LATENCY;
  } else {
    atcvideosink->min_queued_bufs = ATC_VSINK_MIN_LATENCY_INTERLACED;
  }

  if (ATC_VSINK_ST_IDLE == atcvideosink->state) {
    atcvideosink->state = ATC_VSINK_ST_INITED;
  }

  PRINT_TRACE("exit, width: %d, height: %d, format: %d\r\n",
    atcvideosink->cfg_info.width,
    atcvideosink->cfg_info.height,
    prFormat->format);

  ATC_VSINKINST_UNLOCK(atcvideosink);

  return TRUE;
}


