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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/mman.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/types.h>
#include "atcsurface.h"
#include "atcvsinkbuffer.h"
#include "atcvbufferpool.h"
#include "atcdtlog.h"
#include "display.h"
#include "x_ver.h"
#include "async_queue.h"

#define MAKE_FOURCC(a, b, c, d) ((__u32)((a)|(b)<<8|(c)<<16|(d)<<24))

#define ATC_DT_VBUFPOOL_NAME     "atcvbufferpool"
/* /> main version */
#define ATC_DT_VBUFPOOL_MAJ       1
/* /> minor version, Large feature tens add 1 and clear unit, */
/* /> other feature and add new function need add 1. */
#define ATC_DT_VBUFPOOL_MIN       1
/* /> version number, you should add 1 when check in aud */
#define ATC_DT_VBUFPOOL_REV       6

#define ATCVBUFPOOL_DBG_QDEQBUF 0
#if ATCVBUFPOOL_DBG_QDEQBUF
#define PRINT_QDQBUF    PRINT_INFO
#else
#define PRINT_QDQBUF    PRINT_DEBUG
#endif


enum  {
  atcbuf_idle,
  atcbuf_indec,
  atcbuf_inme,
};

enum
{
  state_none,
  State_Created,
  State_Initing,
  State_Inited,
  State_Running,
  State_Recycling
};
typedef struct _AtcVSinkBufferPool ATCVSINKBUFFERPOOL;

struct _AtcVSinkBufferPool
{
  void *parent;      /* the src/sink that owns us.. maybe we should be owned by v4l2object? */

  bool running;          /* with lock */
  __s32 num_live_buffers;     /* number of buffers not with driver */
  AsyncQueue* avail_buffers;/* atcpool of available buffers, not with the driver and which aren't held outside the bufferpool */
  __u32 buffer_count;
  IAtcSurface * atcsurface;
  void **buffers;
  ATCSEMA *avail_buffers_sem;
  bool waiting_buffer;

  ATC_VSINK_FMT_INFO_T format_info;

  bool flushing;
  __s32     state;

  pthread_mutex_t lock;

  bool in_get_buf;
} ;

#define ATC_VBUFPOOL_LOCK(atcpool)  pthread_mutex_lock(&(atcpool->lock))
#define ATC_VBUFPOOL_UNLOCK(atcpool)   pthread_mutex_unlock(&(atcpool->lock))

bool atc_vsink_buffer_pool_free_buffers (ATCVSINKBUFFERPOOL * atcpool)
{
  ATCVSINKBUFFER *atcsinkbuf = NULL;
  __s32 destroy_buf_cnt = 0;
  __s32 n;

  if (atcpool == NULL) {
    return TRUE;
  }

  ATC_VBUFPOOL_LOCK(atcpool);

  switch (atcpool->state) {
  case State_Running:
  case State_Recycling:
    ATC_VBUFPOOL_UNLOCK(atcpool);
    return FALSE;
  default:
    break;
  }

  if (NULL == atcpool->buffers) {
    if (NULL == atcpool->avail_buffers) {
      ATC_VBUFPOOL_UNLOCK(atcpool);
      return TRUE;
    }
    PRINT_INFO("(dcnt: %d, bufcnt: %d, queuelen:%d) --> need destroy atcpool\r\n",
      destroy_buf_cnt, atcpool->buffer_count,
      async_queue_length(atcpool->avail_buffers));
    if (async_queue_length(atcpool->avail_buffers) > 0) {
      void * obj = NULL;
      obj = async_queue_pop (atcpool->avail_buffers);
      while (obj != NULL) {
        obj = async_queue_pop (atcpool->avail_buffers);
      }
    }
    PRINT_INFO("atcpool->buffers == NULL\r\n");
    ATC_VBUFPOOL_UNLOCK(atcpool);
    return TRUE;
  }

  /* after this point, no more buffers will be queued or dequeued; no buffer
   * from atcpool->buffers that is NULL will be set to a buffer, and no buffer that
   * is not NULL will be pushed out. */

  /* miniobjects have no dispose, so they can't break ref-cycles, as buffers ref
   * the atcpool, we need to unref the buffer to properly finalize te atcpool */
  for (n = 0; n < atcpool->buffer_count; n++) {
    if (atcpool->buffers[n] == NULL) {
      PRINT_INFO("buffer %d is NULL\r\n", n);
      destroy_buf_cnt++;
      continue;
    }

    ATC_SINK_BUFFER_LOCK(atcpool->buffers[n]);
    atcsinkbuf = (ATCVSINKBUFFER *)atcpool->buffers[n];
    if ((atcsinkbuf->need_destroy) &&
      (atcsinkbuf->state != atcbuf_indec)) {
      destroy_buf_cnt++;
    }
    ATC_SINK_BUFFER_UNLOCK(atcpool->buffers[n]);
  }

  if ((destroy_buf_cnt > 0) &&
    (destroy_buf_cnt >= atcpool->buffer_count)) {
    PRINT_INFO("(dcnt: %d, bufcnt: %d, queuelen:%d) --> need destroy atcpool\r\n",
      destroy_buf_cnt, atcpool->buffer_count,
      async_queue_length(atcpool->avail_buffers));
    if (async_queue_length(atcpool->avail_buffers) > 0) {
      void * obj = NULL;
      obj = async_queue_pop_nb (atcpool->avail_buffers);
      while (obj != NULL) {
        obj = async_queue_pop_nb (atcpool->avail_buffers);
      }
    }

    for (n = 0; n < atcpool->buffer_count; n++) {
      if (NULL == atcpool->buffers) {
        PRINT_INFO("--> atcpool->buffers == NULL\r\n");
        continue;
      }
      if (atcpool->buffers[n] == NULL) {
        PRINT_INFO("--> buffer %d is NULL\r\n", n);
        continue;
      }
      ATC_VBUFPOOL_UNLOCK(atcpool);
      atc_vsink_buffer_free(atcpool->buffers[n]);
      ATC_VBUFPOOL_LOCK(atcpool);
    }
    PRINT_INFO("success\r\n");
    ATC_VBUFPOOL_UNLOCK(atcpool);

    return TRUE;
  }

  PRINT_ERROR("fail for some buffers in using, (%d/%d)\r\n",
    destroy_buf_cnt, atcpool->buffer_count);
  ATC_VBUFPOOL_UNLOCK(atcpool);

  return FALSE;
}

bool atc_vsink_buffer_pool_free (void *pool)
{
  ATCVSINKBUFFERPOOL *atcpool = (ATCVSINKBUFFERPOOL *)pool;

  if (NULL == atcpool) {
    return TRUE;
  }

  PRINT_TRACE("enter\r\n");
  if (!atc_vsink_buffer_pool_free_buffers(atcpool)) {
    PRINT_ERROR("fail in free_buffers\r\n");
    return FALSE;
  }

  if (atcpool->avail_buffers) {
    async_queue_free (atcpool->avail_buffers);
  }

  atcpool->avail_buffers = NULL;

  if (atcpool->atcsurface) {
    IAtcSurface_resetBuffers(atcpool->atcsurface);
    PRINT_INFO("--> IAtcSurface_release\r\n");
    IAtcSurface_release(atcpool->atcsurface);
    atcpool->atcsurface = NULL;
  }

  if (atcpool->buffers) {
    free (atcpool->buffers);
    atcpool->buffers = NULL;
  }

  if (atcpool->avail_buffers_sem) {
    atc_sem_free(atcpool->avail_buffers_sem);
    atcpool->avail_buffers_sem = NULL;
  }

  pthread_mutex_destroy(&atcpool->lock);

  free(atcpool);
  PRINT_TRACE("success\r\n");

  return TRUE;
}

/**
 * atc_vsink_buffer_pool_new:
 * @sinkelem:  the sink element that owns this atcpool
 * @num_buffers:  the requested number of buffers in the atcpool
 * @caps:  the caps to set on the buffer
 *
 * a buffer with no remaining references is  returned to the atcpool of available buffers
 *  (which can be accessed via atc_vsink_buffer_pool_get().
 *
 * Construct a new buffer atcpool.
 *
 * Returns: the new atcpool, use atc_vsink_buffer_pool_destroy() to free resources
 */

void *atc_vsink_buffer_pool_new (void *parent)
{
  ATCVSINKBUFFERPOOL *atcpool = NULL;

  MOD_VERSION_INFO(ATC_DT_VBUFPOOL_NAME, ATC_DT_VBUFPOOL_MAJ, ATC_DT_VBUFPOOL_MIN, ATC_DT_VBUFPOOL_REV);

  PRINT_TRACE("enter\r\n");
  if (NULL == parent) {
    PRINT_ERROR("fail for invalid args\r\n");
    return NULL;
  }

  atcpool = (ATCVSINKBUFFERPOOL *)malloc(sizeof(ATCVSINKBUFFERPOOL));
  if (NULL == atcpool) {
    PRINT_ERROR("fail for no memory\r\n");
    return NULL;
  }
  memset(atcpool, 0, sizeof(ATCVSINKBUFFERPOOL));

  atcpool->running = FALSE;
  atcpool->num_live_buffers = 0;
  atcpool->atcsurface = NULL;
  atcpool->flushing = FALSE;
  atcpool->buffers = NULL;
  atcpool->avail_buffers_sem = atc_sem_new();
  pthread_mutex_init(&atcpool->lock, NULL);
  atcpool->waiting_buffer = FALSE;
  atcpool->in_get_buf = FALSE;
  atcpool->state = State_Created;
  atcpool->parent = parent;

  PRINT_TRACE("success, return %p\r\n",
    atcpool);
  return (void *)atcpool;
}

bool
atc_vsink_buffer_pool_init_buffers (void *pool,
   __s32 num_buffers, ATC_VSINK_FMT_INFO_T *prFormat)
{
  ATCVSINKBUFFERPOOL *atcpool = (ATCVSINKBUFFERPOOL *)pool;
  __s32 n;
  __s32 ret = 0;

  PRINT_TRACE("enter\r\n");

  if ((NULL == atcpool) || (0 == num_buffers) || (NULL == prFormat)) {
    PRINT_ERROR("fail for invalid args\r\n");
    return FALSE;
  }

  if (prFormat->format != ATC_PIX_FMT_NV12M_PRIVATE1) {
    PRINT_ERROR("fail for invalid format: %d\r\n",
      prFormat->format);
    return FALSE;
  }

  ATC_VBUFPOOL_LOCK(atcpool);
  atcpool->format_info.format = prFormat->format;
  atcpool->format_info.width  = prFormat->width;
  atcpool->format_info.height = prFormat->height;
  atcpool->format_info.stride = prFormat->stride;
  atcpool->format_info.fourcc = prFormat->fourcc;

  atcpool->buffer_count = num_buffers;

  atcpool->buffers = (void **)malloc(sizeof(ATCVSINKBUFFER *) * num_buffers);
  if (NULL == atcpool->buffers) {
    PRINT_ERROR("fail for no memory\r\n");
    ATC_VBUFPOOL_UNLOCK(atcpool);
    return FALSE;
  }

  memset(atcpool->buffers, 0, sizeof(ATCVSINKBUFFER *) * num_buffers);

  PRINT_INFO("--> width: %d, height: %d\r\n",
    prFormat->width, prFormat->height);

  if (NULL == atcpool->atcsurface) {
    PRINT_ERROR("fail for no surface has been set\r\n");
    goto FAILNEW;
  }

  if (State_Initing != atcpool->state) {
    PRINT_ERROR("fail for atcpool state(%d) isn't in initing\r\n",
      atcpool->state);
    goto FAILNEW;
  }

  IAtcSurface_resetBuffers(atcpool->atcsurface);

  if (ATC_PIX_FMT_NV12M_PRIVATE1 == atcpool->format_info.format) {
    ret = IAtcSurface_setBuffersFormat(atcpool->atcsurface, ATC_PIX_FMT_NV12M_PRIVATE1, prFormat->fourcc);
  } else {
    ret = IAtcSurface_setBuffersFormat(atcpool->atcsurface, ATC_PIX_FMT_FOURCC, prFormat->fourcc);
  }

  if (ret != 0) {
    PRINT_ERROR("fail in IAtcSurface_setBuffersFormat\r\n");
    ATC_VBUFPOOL_UNLOCK(atcpool);
    return FALSE;
  }

  ret = IAtcSurface_setBuffersSize (atcpool->atcsurface,
    prFormat->width, prFormat->height);
  if (ret != 0) {
    PRINT_ERROR("fail in IAtcSurface_setBuffersSize\r\n");
    ATC_VBUFPOOL_UNLOCK(atcpool);
    return FALSE;
  }

  ret = IAtcSurface_setBufferCount(atcpool->atcsurface, num_buffers);
  if (ret != 0) {
    PRINT_ERROR("fail in IAtcSurface_setBufferCount\r\n");
    ATC_VBUFPOOL_UNLOCK(atcpool);
    return FALSE;
  }

  atcpool->avail_buffers = async_queue_new();

  /* now, map the buffers: */
  PRINT_DEBUG("--> buffer count = %d\r\n", num_buffers);

  for (n = 0; n < num_buffers; n++) {
    PRINT_INFO("-- atc_vsink_buffer_new(n: %d)\r\n",
      n);
    atcpool->buffers[n] = atc_vsink_buffer_new (atcpool,
      atcpool->atcsurface, n, prFormat);
    if (NULL == atcpool->buffers[n]) {
      PRINT_ERROR("fail in atc_vsink_buffer_new, idx: %d\r\n",
        n);
      ATC_VBUFPOOL_UNLOCK(atcpool);
      return FALSE;
    }
    atcpool->num_live_buffers++;
    async_queue_push (atcpool->avail_buffers, atcpool->buffers[n]);
  }

  atcpool->state = State_Inited;

  ATC_VBUFPOOL_UNLOCK(atcpool);
  PRINT_TRACE("success\r\n");

  return TRUE;

FAILNEW:
  {
    if (atcpool->buffers) {
      free(atcpool->buffers);
      atcpool->buffers = NULL;
    }
    ATC_VBUFPOOL_UNLOCK(atcpool);
    PRINT_ERROR("fail\r\n");
    return FALSE;
  }
}

bool
atc_vsink_buffer_pool_reset(void *pool,
    ATC_VSINK_FMT_INFO_T *prFormat)
{
  ATCVSINKBUFFERPOOL *atcpool = (ATCVSINKBUFFERPOOL *)pool;
  __s32 ret = 0;

  if ((NULL == atcpool) || (NULL == prFormat)) {
    PRINT_ERROR("fail for invalid args\r\n");
    return FALSE;
  }

  PRINT_DEBUG("init_buffers enter\r\n");

  ATC_VBUFPOOL_LOCK(atcpool);
  if (atcpool->format_info.format != prFormat->format) {
    PRINT_ERROR("reset failed for format changed\r\n");
    ATC_VBUFPOOL_UNLOCK(atcpool);
    return FALSE;
  }

  ret = IAtcSurface_setBuffersSize (atcpool->atcsurface,
    prFormat->width, prFormat->height);
  if (ret != 0) {
    PRINT_ERROR("reset fail in IAtcSurface_setBuffersSize\r\n");
    ATC_VBUFPOOL_UNLOCK(atcpool);
    return FALSE;
  }
  atcpool->format_info.width = prFormat->width;
  atcpool->format_info.height = prFormat->height;

  ATC_VBUFPOOL_UNLOCK(atcpool);
  return TRUE;
}

/**
 * atc_vsink_buffer_pool_get:
 * @atcpool:   the "this" object
 * @blocking:  should this call suspend until there is a buffer available
 *    in the buffer atcpool?
 *
 * Get an available buffer in the atcpool
 */
bool
atc_vsink_buffer_pool_get (void * pool,
  bool blocking, void **ppBuffer, __u32 *pBuflen)
{
  ATCVSINKBUFFERPOOL *atcpool = (ATCVSINKBUFFERPOOL *)pool;
  ATCVSINKBUFFER *atcsinkbuf = NULL;
  void * obj = NULL;

  if ((NULL == atcpool) || (NULL == ppBuffer) || (NULL == pBuflen)) {
    PRINT_ERROR("%s fail for invalid args\r\n");
    return FALSE;
  }

  PRINT_DEBUG("get --> enter\r\n");
  if (blocking) {
    ATC_VBUFPOOL_LOCK(atcpool);
    atcpool->in_get_buf = TRUE;
    while (atcpool->state != State_Recycling) {
      obj = async_queue_pop (atcpool->avail_buffers);
      if (obj == NULL) {
        if ((atcpool->state == State_Running) ||
            (atcpool->state == State_Inited)) {
          if (atcpool->flushing) {
            PRINT_INFO("--> atcpool is in flushing state\r\n");
            break;
          }
          atcpool->waiting_buffer = TRUE;
          ATC_VBUFPOOL_UNLOCK(atcpool);
          atc_sem_down(atcpool->avail_buffers_sem);
          ATC_VBUFPOOL_LOCK(atcpool);
          atcpool->waiting_buffer = FALSE;
          if (atcpool->flushing) {
            PRINT_INFO("--> atcpool is in flushing state\r\n");
            break;
          } else {
            continue;
          }
        }
      }
      atcpool->waiting_buffer = FALSE;
      break;
    }
    ATC_VBUFPOOL_UNLOCK(atcpool);
  } else {
    obj = async_queue_pop (atcpool->avail_buffers);
  }

  ATC_VBUFPOOL_LOCK (atcpool);

  if (atcpool->state == State_Recycling) {
    atcpool->in_get_buf = FALSE;
    PRINT_ERROR("get --> atcpool is in destroying state, cannot get avail buffer\r\n");
    ATC_VBUFPOOL_UNLOCK(atcpool);
    return FALSE;
  }

  if (NULL != obj) {
    atcsinkbuf = (ATCVSINKBUFFER *)(obj);
    ATC_SINK_BUFFER_LOCK(atcsinkbuf);
    atcsinkbuf->state = atcbuf_indec;
    *ppBuffer = atcsinkbuf->data;
    *pBuflen = atcsinkbuf->size;
    ATC_SINK_BUFFER_UNLOCK(atcsinkbuf);
  } else {
    atcpool->in_get_buf = FALSE;
    PRINT_ERROR("get --> can not get avail buffer\r\n");
    ATC_VBUFPOOL_UNLOCK(atcpool);
    return FALSE;
  }

  atcpool->running = TRUE;
  atcpool->state = State_Running;

  atcpool->in_get_buf = FALSE;

  ATC_VBUFPOOL_UNLOCK (atcpool);

  PRINT_DEBUG("get --> exit, bits: %p, atcsinkbuf: %p\r\n",
    *ppBuffer, atcsinkbuf);

  return TRUE;
}

/**
 * atc_vsink_buffer_pool_qbuf:
 * @atcpool: the atcpool
 * @buf: the buffer to queue
 *
 * Queue a buffer to the driver
 *
 * Returns: %TRUE for success
 */
bool
atc_vsink_buffer_pool_qbuf (void * pool, void *pBuffer, __u32 buflen)
{
  ATCVSINKBUFFERPOOL *atcpool = (ATCVSINKBUFFERPOOL *)pool;
  ATCVSINKBUFFER *atcsinkbuf = NULL;
  ATCVSINKBUFFER *tmp = NULL;
  atc_buffer_t *atcbuf = NULL;
  struct VOUT_PARAM *vid_out_data = NULL;
  int32_t ret = 0;
  __u32 index = 0;

  PRINT_DEBUG("enter\r\n");

  if ((NULL == atcpool) || (NULL == pBuffer) || (0 == buflen)) {
    PRINT_ERROR("fail for invalid args");
    return FALSE;
  }

  ATC_VBUFPOOL_LOCK (atcpool);

  atcsinkbuf = NULL;

  for (index = 0; index < atcpool->buffer_count; index++) {
    if (atcpool->buffers[index] == NULL) {
      continue;
    }
    tmp = (ATCVSINKBUFFER *)(atcpool->buffers[index]);
    if ((tmp != NULL) &&
      (tmp->data == pBuffer)) {
      atcsinkbuf = tmp;
      break;
    }
  }

  if (atcsinkbuf == NULL) {
    PRINT_ERROR("fail for no corresponding atcsinkbuffer, data: %p\r\n",
      pBuffer);
    ATC_VBUFPOOL_UNLOCK(atcpool);
    return FALSE;
  }

  ATC_SINK_BUFFER_LOCK(atcsinkbuf);

  atcbuf = (atc_buffer_t *)(atcsinkbuf->bufctrl);

  atcbuf->width = atcpool->format_info.width;
  atcbuf->height = atcpool->format_info.height;
  atcbuf->stride = atcpool->format_info.stride;
  if (ATC_PIX_FMT_NV12M_PRIVATE1 == atcsinkbuf->format) {
    atcbuf->format = ATC_PIX_FMT_NV12M_PRIVATE1;
    vid_out_data = (struct VOUT_PARAM *)(atcbuf->bits);
    if (vid_out_data != NULL) {
      vid_out_data->device_name = USB;
      PRINT_QDQBUF("(index=%d), bufctrl=%p, bits=%p, (y=0x%08x c=0x%08x)\r\n",
        atcsinkbuf->index, atcsinkbuf->bufctrl,
        atcbuf->bits, vid_out_data->y_phy_addr,
        vid_out_data->c_phy_addr);
    }
  } else {
    atcbuf->format = ATC_PIX_FMT_FOURCC;
  }
  atcbuf->fmt_fourcc = atcsinkbuf->fourcc;

  ret = IAtcSurface_queueBuffer(atcpool->atcsurface, (atc_buffer_t*)atcsinkbuf->bufctrl);
  if (ret != 0) {
    PRINT_ERROR("fail in IAtcSurface_queueBuffer, ret: %d, (state=%d, index=%d), bufctrl=%p, bits=%p\r\n",
      ret, atcsinkbuf->state, atcsinkbuf->index, atcsinkbuf->bufctrl, atcbuf->bits);
    ATC_SINK_BUFFER_UNLOCK(atcsinkbuf);
    ATC_VBUFPOOL_UNLOCK (atcpool);
    return FALSE;
  }

  atcsinkbuf->state = atcbuf_idle;

  atcpool->num_live_buffers--;

  ATC_SINK_BUFFER_UNLOCK(atcsinkbuf);

  ATC_VBUFPOOL_UNLOCK (atcpool);

  PRINT_DEBUG("num_live_buffers--: %d\r\n",
      atcpool->num_live_buffers);

  PRINT_DEBUG("qbuf success\r\n");

  return TRUE;
}


bool atc_vsink_buffer_pool_dqbuf (void *pool)
{
  ATCVSINKBUFFERPOOL *atcpool = (ATCVSINKBUFFERPOOL *)pool;
  ATCVSINKBUFFER *atcsinkbuf = NULL;
  int32_t ret = 0;
  atc_buffer_t atcbuf;
  __s32 index = 0;

  if ((NULL == atcpool) ||
    (NULL == atcpool->parent)) {
    PRINT_ERROR("fail for invalid args\r\n");
    return FALSE;
  }

  if (atcpool->state == State_Recycling) {
    PRINT_INFO("atcpool is in destroying state, so we do nothing\r\n");
    return FALSE;
  }

  memset(&atcbuf, 0, sizeof(atcbuf));

  PRINT_DEBUG("IAtcSurface_dequeueBuffer\r\n");

  ret = IAtcSurface_dequeueBuffer(atcpool->atcsurface, &atcbuf);
  if (ret != 0) {
    PRINT_ERROR("IAtcSurface_dequeueBuffer fail\r\n");
    return FALSE;
  }

  ATC_VBUFPOOL_LOCK (atcpool);
  for (index = 0; index < atcpool->buffer_count; index++) {
    atcsinkbuf = (ATCVSINKBUFFER *)atcpool->buffers[index];
    if ((atcsinkbuf != NULL) && (atcsinkbuf->data == atcbuf.bits)) {
      break;
    }
  }

  if (index < atcpool->buffer_count) {
    atc_buffer_t *prAtcbuf = NULL;

    prAtcbuf = (atc_buffer_t *)(atcsinkbuf->bufctrl);
    atcsinkbuf->state = atcbuf_inme;

    if (ATC_PIX_FMT_NV12M_PRIVATE1 == atcpool->format_info.format) {
      struct VOUT_PARAM *vid_out_data = (struct VOUT_PARAM *)(atcsinkbuf->data);
      if ((prAtcbuf != NULL) && (vid_out_data != NULL)){
        PRINT_QDQBUF("--> (index=%d), bufctrl=%p, bits=%p, (y=0x%08x c=0x%08x)\r\n",
          atcsinkbuf->index, atcsinkbuf->bufctrl,
          prAtcbuf->bits, vid_out_data->y_phy_addr,
          vid_out_data->c_phy_addr);
      } else {
        PRINT_DEBUG("--> dequeue atcpool buffer (index=%d), bufctrl=%p\r\n",
          index, atcsinkbuf->bufctrl);
      }
    }

    PRINT_DEBUG("(idx: %d) --> g_async_queue_push\r\n",
      index);

    async_queue_push (atcpool->avail_buffers, atcpool->buffers[index]);

    if (atcpool->waiting_buffer) {
      atc_sem_up(atcpool->avail_buffers_sem);
    }

    atcpool->num_live_buffers++;
  } else {
    PRINT_ERROR("fail for no corresponding atcsinkbuffer in buffers list, atcbuf's bits: %p\r\n",
      atcbuf.bits);
    ret = IAtcSurface_queueBuffer(atcpool->atcsurface, &atcbuf);
    if (ret != 0) {
      PRINT_ERROR("fail in IAtcSurface_queueBuffer(surface: %p)\r\n",
        atcpool->atcsurface);
    }
    ATC_VBUFPOOL_UNLOCK (atcpool);
    return FALSE;
  }

  ATC_VBUFPOOL_UNLOCK (atcpool);
  PRINT_DEBUG("exit\r\n");

  return TRUE;
}

bool
atc_vsink_buffer_pool_unshow (void * pool, void *pBuffer, __u32 buflen)
{
  ATCVSINKBUFFERPOOL *atcpool = (ATCVSINKBUFFERPOOL *)pool;
  ATCVSINKBUFFER *atcsinkbuf = NULL;
  ATCVSINKBUFFER *tmp = NULL;
  atc_buffer_t *atcbuf = NULL;
  __u32 index = 0;

  PRINT_DEBUG("enter\r\n");

  if ((NULL == pBuffer) || (NULL == atcpool)) {
    PRINT_ERROR("fail for invalid args");
    return FALSE;
  }

  ATC_VBUFPOOL_LOCK (atcpool);

  atcsinkbuf = NULL;

  for (index = 0; index < atcpool->buffer_count; index++) {
    if (atcpool->buffers[index] == NULL) {
      continue;
    }
    tmp = (ATCVSINKBUFFER *)(atcpool->buffers[index]);
    if ((tmp != NULL) &&
      (tmp->data == pBuffer)) {
      atcsinkbuf = tmp;
      break;
    }
  }

  if (atcsinkbuf == NULL) {
    PRINT_ERROR("fail for no corresponding atcsinkbuffer, data: %p\r\n",
      pBuffer);
    ATC_VBUFPOOL_UNLOCK(atcpool);
    return FALSE;
  }

  ATC_SINK_BUFFER_LOCK(atcsinkbuf);

  atcbuf = (atc_buffer_t *)(atcsinkbuf->bufctrl);

  PRINT_DEBUG("atcpool buffer (index=%d), bufctrl=%p, bits=%p, w:%d, h:%d, s: %d\r\n",
    atcsinkbuf->index, atcsinkbuf->bufctrl, atcbuf->bits,
    atcpool->format_info.width,
    atcpool->format_info.height,
    atcpool->format_info.stride);

  atcbuf->width = atcpool->format_info.width;
  atcbuf->height = atcpool->format_info.height;
  atcbuf->stride = atcpool->format_info.stride;

  if (ATC_PIX_FMT_NV12M_PRIVATE1 == atcpool->format_info.format) {
    struct VOUT_PARAM *vid_out_data = (struct VOUT_PARAM *)(atcsinkbuf->data);
    atc_buffer_t *prAtcbuf = (atc_buffer_t *)(atcsinkbuf->bufctrl);
    if ((prAtcbuf != NULL) && (vid_out_data != NULL)){
      PRINT_QDQBUF("atcpool buffer (index=%d), bufctrl=%p, y=0x%x, c=0x%x\r\n",
        atcsinkbuf->index,
        atcsinkbuf->bufctrl,
        (unsigned int)(vid_out_data->y_phy_addr),
        (unsigned int)(vid_out_data->c_phy_addr));
    } else {
      PRINT_QDQBUF("atcpool buffer (index=%d), bufctrl=%p\r\n",
        atcsinkbuf->index, atcsinkbuf->bufctrl);
    }
  }

  atcbuf->fmt_fourcc = atcsinkbuf->fourcc;
  atcsinkbuf->state = atcbuf_inme;

  async_queue_push (atcpool->avail_buffers, atcpool->buffers[index]);

  if (atcpool->waiting_buffer) {
    atc_sem_up(atcpool->avail_buffers_sem);
  }

  PRINT_DEBUG("success, num_live_buffers: %d, index=%d, state=%d\r\n",
    atcpool->num_live_buffers, index, atcsinkbuf->state);

  ATC_SINK_BUFFER_UNLOCK(atcsinkbuf);

  ATC_VBUFPOOL_UNLOCK (atcpool);

  atc_vsink_buffer_pool_recycle_buffers(atcpool);

  return TRUE;
}

/**
 * atc_vsink_buffer_pool_available_buffers:
 * @atcpool: the atcpool
 *
 * Check the number of buffers available to the driver, ie. buffers that
 * have been QBUF'd but not yet DQBUF'd.
 *
 * Returns: the number of buffers available.
 */
__s32
atc_vsink_buffer_pool_available_buffers (void *pool)
{
  ATCVSINKBUFFERPOOL *atcpool = (ATCVSINKBUFFERPOOL *)pool;
  if (NULL == atcpool) {
    PRINT_ERROR("fail for invalid args\r\n");
    return 0;
  }

  /*PRINT_TRACE("available_buffers = %d\r\n",*/
  /*  atcpool->buffer_count - atcpool->num_live_buffers);*/
  return atcpool->buffer_count - atcpool->num_live_buffers;
}

void atc_vsink_buffer_pool_set_surface (void *pool, void *surface)
{
  ATCVSINKBUFFERPOOL *atcpool = (ATCVSINKBUFFERPOOL *)pool;

  PRINT_TRACE("enter, surface: %p\r\n", surface);

  if (NULL == atcpool) {
    PRINT_ERROR("fail for atcpool is NULL\r\n");
    return;
  }

  ATC_VBUFPOOL_LOCK (atcpool);
  if (atcpool->state == State_Created) {
    if (atcpool->atcsurface) {
      PRINT_INFO("-- IAtcSurface_release\r\n");
      IAtcSurface_release(atcpool->atcsurface);
      atcpool->atcsurface = NULL;
    }
    atcpool->atcsurface = (IAtcSurface *)surface;
    if (atcpool->atcsurface != NULL) {
      PRINT_INFO("-- IAtcSurface_addRef\r\n");
      IAtcSurface_addRef(atcpool->atcsurface);
    }
    PRINT_INFO("--> exit, surface: %p\r\n",
      surface);
    atcpool->state = State_Initing;
  }
  ATC_VBUFPOOL_UNLOCK (atcpool);
  PRINT_TRACE("exit, surface: %p\r\n",
    surface);
}

void *atc_vsink_buffer_pool_get_surface (void * pool)
{
  ATCVSINKBUFFERPOOL *atcpool = (ATCVSINKBUFFERPOOL *)pool;
  void *surface = NULL;

  PRINT_TRACE("enter\r\n");

  if (NULL == atcpool) {
    PRINT_ERROR("set_surface_handle fail for atcpool is NULL\r\n");
    return NULL;
  }

  ATC_VBUFPOOL_LOCK (atcpool);
  switch (atcpool->state) {
  case State_Created:
  case State_Initing:
  case State_Inited:
  case State_Running:
    surface = ((void *)(atcpool->atcsurface));
    break;
  default:
    surface = NULL;
    break;
  }
  ATC_VBUFPOOL_UNLOCK (atcpool);

  PRINT_TRACE("exit, surface: %p\r\n", surface);
  return surface;
}

/**
 * atc_vsink_buffer_pool_destroy:
 * @atcpool: the atcpool
 *
 * Free all resources in the atcpool and the atcpool itself.
 */
bool
atc_vsink_buffer_pool_recycle_buffers (void * pool)
{
  ATCVSINKBUFFERPOOL *atcpool = (ATCVSINKBUFFERPOOL *)pool;
  ATCVSINKBUFFER *atcsinkbuf = NULL;
  __s32 indec_bufcnt = 0;
  __s32 n;

  PRINT_TRACE("--> enter\r\n");

  if (NULL == atcpool) {
    PRINT_ERROR("fail for atcpool is NULL\r\n");
    return TRUE;
  }

  ATC_VBUFPOOL_LOCK (atcpool);
  atcpool->running = FALSE;
  atcpool->state = State_Recycling;
  if (atcpool->waiting_buffer) {
    atc_sem_up(atcpool->avail_buffers_sem);
  }

  /* after this point, no more buffers will be queued or dequeued; no buffer
   * from atcpool->buffers that is NULL will be set to a buffer, and no buffer that
   * is not NULL will be pushed out. */

  /* miniobjects have no dispose, so they can't break ref-cycles, as buffers ref
   * the atcpool, we need to unref the buffer to properly finalize te atcpool */
  PRINT_INFO("--> buffer cnt: %d, waiting_buffer = %s\r\n",
    atcpool->buffer_count,
    (atcpool->waiting_buffer ? "TRUE" : "FALSE"));
  for (n = 0; n < atcpool->buffer_count; n++) {
    if (NULL == atcpool->buffers) {
      PRINT_INFO("--> atcpool->buffers == NULL\r\n");
      continue;
    }
    if (atcpool->buffers[n] == NULL) {
      PRINT_INFO("--> buffer %d is NULL\r\n", n);
      continue;
    }

    PRINT_DEBUG(" --> atcpool->buffers[%d]: %p)\r\n",
      n, atcpool->buffers[n]);

    ATC_SINK_BUFFER_LOCK(atcpool->buffers[n]);
    atcsinkbuf = (ATCVSINKBUFFER *)atcpool->buffers[n];
    PRINT_INFO("%d unref buffer(%p, state: %d), atcpool(%p)\r\n",
      n, atcsinkbuf, atcsinkbuf->state, atcpool);
    atcsinkbuf->need_destroy = TRUE;
    if (atcsinkbuf->state == atcbuf_indec) {
      indec_bufcnt++;
    }
    ATC_SINK_BUFFER_UNLOCK(atcpool->buffers[n]);
  }

  PRINT_INFO("atcpool(%p)'s waiting_buffer = %s\r\n",
    atcpool, (atcpool->waiting_buffer ? "TRUE" : "FALSE"));

  if (atcpool->waiting_buffer) {
    atc_sem_up(atcpool->avail_buffers_sem);
  }

  if (0 == indec_bufcnt) {
    atcpool->state = State_Inited;
    ATC_VBUFPOOL_UNLOCK (atcpool);
    PRINT_TRACE("exit, success, all buffer recycled\r\n");
    return TRUE;
  } else {
    ATC_VBUFPOOL_UNLOCK (atcpool);
  }

  PRINT_TRACE("exit, return FALSE\r\n");

  return FALSE;
}

