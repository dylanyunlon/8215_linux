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

#include <linux/types.h>
#include <stdbool.h>
#include <string.h>             /* for memcpy */
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <linux/types.h>
#include <sys/time.h>
#include <errno.h>
#include "atcomxutils.h"
#include "atcomxvdecinst.h"
#include <async_queue.h>
#include "atcdtlog.h"
#include "atcsurface.h"
#include "display.h"
#include "x_ver.h"

#define ATC_DTOMXVDEC_INST_MOD "[atcdtomxvdec]"
#define ATC_DTOMXVDEC_INST_MAJ  1
#define ATC_DTOMXVDEC_INST_MIN  1
#define ATC_DTOMXVDEC_INST_REV  4

#define ATC_DIRECTRENDER_CONFIG_FILE "/data/dt.conf"
#define ATC_DT_DUMPPATH_PREFX "dump_path="
#define ATC_DT_VDEC_DUMP_FILENAME "dtvdecinput.raw"

#define ATC_CLOCK_TIME_NONE   ((__s64)(-1))

typedef struct _AtcVidDecInst ATCVIDDECINST;

typedef struct _AtcVidDecInputFormatInfo ATCVIDDECINPUTFMTINFO;

struct _AtcVidDecInputFormatInfo
{
  char  *role;
  OMX_VIDEO_CODINGTYPE eFormat;
  __s32 width;
  __s32 height;
  __s32 fps_n;
  __s32 fps_d;
  bool  interlaced;
};

typedef  struct _AtcVidDecOutputCfgInfo
{
  __u32 max_buf_cnt;
  __u32 min_buf_cnt;
  __u32 cur_buf_cnt;
  bool  had_set_bufcnt;
} ATCVIDDECOUTPUTCFGINFO;

typedef bool (*ATCVIDDECINSTCB) (ATCVIDDECINST * decoder);

#define ATC_DT_OMXVDEC_MAX_PATH 256
static OMX_ERRORTYPE
EmptyBufferDone (OMX_HANDLETYPE omx_handle,
    OMX_PTR app_data, OMX_BUFFERHEADERTYPE * omx_buffer);
static OMX_ERRORTYPE
FillBufferDone (OMX_HANDLETYPE omx_handle,
    OMX_PTR app_data, OMX_BUFFERHEADERTYPE * omx_buffer);

static OMX_CALLBACKTYPE atcomxcallbacks =
    { NULL, EmptyBufferDone, FillBufferDone };


struct _AtcVidDecInst
{
  AtcOmxCore *gomx;
  AtcOmxPort *in_port;
  AtcOmxPort *out_port;

  bool ready;
  pthread_mutex_t   ready_lock;

  ATC_VDEC_ERRCODE last_error_ret;

  ATCVIDDECINPUTFMTINFO input_format;
  ATCVIDDECOUTPUTCFGINFO output_cfg;

  bool had_set_infmt;
  bool set_omxbuffer;

  void *  output_tmp_buf;
  __s32    output_tmp_buf_sz;

    /** @todo these are hacks, OpenMAX IL spec should be revised. */
  bool share_input_buffer;
  bool share_output_buffer;

  FILE *dump_fp;
  char dump_filename[ATC_DT_OMXVDEC_MAX_PATH];
  BUFFER_CALLBACKTYPE *callbacks;
  void *app_data;
};

static void omx_atomic_set(__u32 *const atom, __u32 value)
{
    volatile __u32 *aval;
    bool written;

    do
    {
        aval = atom;
        written = __sync_bool_compare_and_swap(aval, *aval, value);
    }
    while (!written);
}

static __s64 GetCurrentTimeUs(void)
{
    struct timeval tv;
    memset(&tv, 0, sizeof(tv));

    gettimeofday(&tv, NULL);

    return (__s64)tv.tv_usec + (__s64)tv.tv_sec * 1000000;
}

static OMX_ERRORTYPE
EmptyBufferDone (OMX_HANDLETYPE omx_handle,
    OMX_PTR app_data, OMX_BUFFERHEADERTYPE * omx_buffer)
{
  ATCVIDDECINST *decoder = (ATCVIDDECINST *)app_data;
  PRINT_DEBUG("omx_buffer: "
      "size=%lu, "
      "len=%lu, "
      "flags=%lu, "
      "offset=%lu, "
      "timestamp=%lld , pBuffer:%p\n",
      omx_buffer->nAllocLen, omx_buffer->nFilledLen, omx_buffer->nFlags,
      omx_buffer->nOffset, omx_buffer->nTimeStamp, omx_buffer->pBuffer);

  if (decoder->callbacks) {
    VDEC_BUFFER_INFO_T buf_info;
    buf_info.buffer = omx_buffer->pBuffer;
    buf_info.bufSz = omx_buffer->nAllocLen;
    buf_info.datasz = omx_buffer->nFilledLen;
    buf_info.timestampus = omx_buffer->nTimeStamp;
    buf_info.flags = omx_buffer->nFlags;
    decoder->callbacks->onEmptyBufferDone(decoder->app_data, &buf_info);
  }
  return OMX_ErrorNone;
}

static OMX_ERRORTYPE
FillBufferDone (OMX_HANDLETYPE omx_handle,
    OMX_PTR app_data, OMX_BUFFERHEADERTYPE * omx_buffer)
{
  ATCVIDDECINST *decoder = (ATCVIDDECINST *)app_data;
  PRINT_DEBUG("omx_buffer: "
      "size=%lu, "
      "len=%lu, "
      "flags=%lu, "
      "offset=%lu, "
      "timestamp=%lld , pBuffer:%p\n",
      omx_buffer->nAllocLen, omx_buffer->nFilledLen, omx_buffer->nFlags,
      omx_buffer->nOffset, omx_buffer->nTimeStamp, omx_buffer->pBuffer);

  if (decoder->callbacks) {
    VDEC_BUFFER_INFO_T buf_info;
    buf_info.buffer = omx_buffer->pBuffer;
    buf_info.bufSz = omx_buffer->nAllocLen;
    buf_info.datasz = omx_buffer->nFilledLen;
    buf_info.timestampus = omx_buffer->nTimeStamp;
    buf_info.flags = omx_buffer->nFlags;

    if (NULL != omx_buffer->pOutputPortPrivate) {
        OMX_OUTPUTPORT_PRIVATE * priv =
              (OMX_OUTPUTPORT_PRIVATE *)(omx_buffer->pOutputPortPrivate);
        buf_info.width = priv->rPicSize.u4Width;
        buf_info.height = priv->rPicSize.u4Height;
        buf_info.flags &= ~(OMX_BUFFERFLAG_SIZECHANGE);
        if ((buf_info.width != decoder->input_format.width) ||
              (buf_info.height != decoder->input_format.height)) {
            PRINT_TRACE("port config chage: %d x %d to %d x %d\r\n",
                decoder->input_format.width, decoder->input_format.height,
                buf_info.width, buf_info.height);
            buf_info.flags |= OMX_BUFFERFLAG_SIZECHANGE;
            decoder->input_format.width = buf_info.width;
            decoder->input_format.height = buf_info.height;
        }
    }
    decoder->callbacks->onFillBufferDone(decoder->app_data, &buf_info);
  }

  return OMX_ErrorNone;
}

static void
setup_ports (ATCVIDDECINST * self)
{
  PRINT_TRACE("setup_ports enter\n");
  /* Input port configuration. */
  atc_omx_port_setup (self->in_port);

  /* Output port configuration. */
  atc_omx_port_setup (self->out_port);

  self->in_port->omx_allocate = TRUE;
  self->out_port->omx_allocate = TRUE;

  PRINT_TRACE("setup_ports: in: %s, out: %s\n",
      self->in_port->omx_allocate ? "TRUE" : "false",
      self->out_port->omx_allocate ? "TRUE" : "false");
  PRINT_TRACE("setup_ports end\n");
}

static bool
omx_setup (ATCVIDDECINST * self)
{
  OMX_ERRORTYPE omx_err   = OMX_ErrorNone;
  AtcOmxCore *gomx = NULL;
  OMX_PARAM_PORTDEFINITIONTYPE port_def;

  gomx = (AtcOmxCore *) self->gomx;

  omx_err = atc_omx_port_get_port_definition(self->in_port, &port_def);
  if (omx_err != OMX_ErrorNone) {
    PRINT_ERROR("fail in get omx component input port definition, err: 0x%08x\r\n",
      omx_err);
    return false;
  }

  PRINT_TRACE("-- eCompressionFormat = %d\r\n", self->input_format.eFormat);
  port_def.format.video.eCompressionFormat = self->input_format.eFormat;

  omx_err = atc_omx_port_update_port_definition(self->in_port, &port_def);
  if (omx_err != OMX_ErrorNone) {
    PRINT_ERROR("fail in update omx component input port definition, err: 0x%08x\r\n",
      omx_err);
    return false;
  }

  omx_err = atc_omx_port_get_port_definition(self->out_port, &port_def);
  if (omx_err != OMX_ErrorNone) {
    PRINT_ERROR("fail in get omx component input port definition, err: 0x%08x\r\n",
      omx_err);
    return false;
  }

  port_def.nBufferSize = sizeof(struct VOUT_PARAM);

  omx_err = atc_omx_port_update_port_definition(self->out_port, &port_def);
  if (omx_err != OMX_ErrorNone) {
    PRINT_ERROR("fail in update omx component input port definition, err: 0x%08x\r\n",
      omx_err);
    return false;
  }

  if (NULL != self->output_tmp_buf) {
    if (port_def.nBufferSize > self->output_tmp_buf_sz) {
      free(self->output_tmp_buf);
      self->output_tmp_buf = (void *)malloc(port_def.nBufferSize);
      if (NULL == self->output_tmp_buf) {
        PRINT_TRACE("fail for no memory\r\n");
        return false;
      }
      self->output_tmp_buf_sz = port_def.nBufferSize;
    }
  } else {
    self->output_tmp_buf = (void *)malloc(port_def.nBufferSize);
    if (NULL == self->output_tmp_buf) {
      PRINT_TRACE("fail for no memory\r\n");
      return false;
    }
    self->output_tmp_buf_sz = port_def.nBufferSize;
  }

  PRINT_TRACE("succes\r\n");

  return TRUE;
}

static bool
set_component_role(
  ATCVIDDECINST *atcomxvdec, AtcOmxCore *gomx, OMX_VIDEO_CODINGTYPE eVType)
{
  OMX_ERRORTYPE err = OMX_ErrorNone;
  const char *role = NULL;
  __u32 role_len = 0;
  struct MimeToRole {
      const char *Role;
      OMX_VIDEO_CODINGTYPE compression_format;
  };
  const struct MimeToRole kMimeToRole[] = {
      { "video_decoder.vp8",   OMX_VIDEO_CodingVP8},
      { "video_decoder.avc" ,   OMX_VIDEO_CodingAVC},
      { "video_decoder.mpeg4",   OMX_VIDEO_CodingMPEG4},
      { "video_decoder.h263",   OMX_VIDEO_CodingH263},
      { "video_decoder.mpeg2" ,   OMX_VIDEO_CodingMPEG2},
      { "video_decoder.jpeg" ,   OMX_VIDEO_CodingMJPEG},
      { "video_decoder.rv"   ,   OMX_VIDEO_CodingRV},
      { "video_decoder.mpeg1" ,   OMX_VIDEO_CodingMPEG1},
      { "video_decoder.vc1",   OMX_VIDEO_CodingWMV},
      { "video_decoder.wmv1",   OMX_VIDEO_CodingWMV},
      { "video_decoder.wmv2",   OMX_VIDEO_CodingWMV},
      { "video_decoder.mjpeg",   OMX_VIDEO_CodingMJPEG},
      { "video_decoder.vp6",   OMX_VIDEO_CodingVP6   },
      { "video_decoder.vp8",   OMX_VIDEO_CodingVP8  },
      { "video_decoder.hevc",   OMX_VIDEO_CodingHEVC},
  };

  static const size_t kNumMimeToRole =
      sizeof(kMimeToRole) / sizeof(kMimeToRole[0]);

  size_t i;
  for (i = 0; i < kNumMimeToRole; ++i) {
      if (eVType == kMimeToRole[i].compression_format) {
          break;
      }
  }

  if (i == kNumMimeToRole) {
      return false;
  }

  role = kMimeToRole[i].Role;

  PRINT_DEBUG ("--> core->omx_state: %d\r\n",
    atcomxvdec->gomx->omx_state);

  atcomxvdec->input_format.eFormat = eVType;
  if (NULL != atcomxvdec->input_format.role) {
    free(atcomxvdec->input_format.role);
  }


  role_len = strlen(role) + 1;
  if (role_len >= OMX_MAX_STRINGNAME_SIZE) {
    PRINT_TRACE ("fail for invalid role name: %s\r\n",
      role);
    return false;
  }

  atcomxvdec->input_format.role = (char *)malloc(sizeof(char) * role_len);
  if (NULL == atcomxvdec->input_format.role) {
    PRINT_TRACE ("fail for no memory\r\n");
    return false;
  }

  memset(atcomxvdec->input_format.role, 0, role_len);
  strcpy((char *)atcomxvdec->input_format.role, role);

  if (role != NULL) {
    OMX_PARAM_COMPONENTROLETYPE roleParams;

    ATC_OMX_INIT_PARAM (&roleParams);
    memset(roleParams.cRole, 0, sizeof(roleParams.cRole));
    strcpy((char *)roleParams.cRole, role);

    err = OMX_SetParameter (gomx->omx_handle, OMX_IndexParamATCSetFormat,
      &roleParams);
    if (err != OMX_ErrorNone) {
      PRINT_ERROR("Error setting ATCVid parameters:(0x%08x)",err);
      return false;
    }
  }

  PRINT_DEBUG ("--> core->omx_state: %d\r\n",
    atcomxvdec->gomx->omx_state);

  return TRUE;
}

bool atc_vdec_comp_flags(char* s1, char* s2, int len)
{
  int i = 0;

  for (i = 0; i< len; i++,s1++,s2++){
    if (*s1 == '\0'){
      return TRUE;
    }
    if (*s1 != *s2){
      return false;
    }
  }
  return TRUE;
}

void *atc_vdec_open(void)
{
  ATCVIDDECINST *decoder = NULL;
  char szBuf[ATC_DT_OMXVDEC_MAX_PATH];
  FILE *fp = NULL;

  decoder = (ATCVIDDECINST *)malloc(sizeof(ATCVIDDECINST));

  if (NULL == decoder) {
    PRINT_TRACE("fail for no memory\r\n");
    goto OPEN_FAILED;
  }

  memset(decoder, 0, sizeof(ATCVIDDECINST));

  MOD_VERSION_INFO(ATC_DTOMXVDEC_INST_MOD,
    ATC_DTOMXVDEC_INST_MAJ,
    ATC_DTOMXVDEC_INST_MIN,
    ATC_DTOMXVDEC_INST_REV);

  decoder->gomx = (AtcOmxCore *)atcomx_core_new (decoder,
    "libatcomxcore.so",
    "OMX.video_decoder.atc", &atcomxcallbacks, decoder);
  if (NULL == decoder->gomx) {
    PRINT_TRACE("fail in open ATC OMX VDEC component\r\n");
    goto OPEN_FAILED;
  }
  decoder->in_port = atc_omx_core_new_port (decoder->gomx, 0);
  if (NULL == decoder->gomx) {
    PRINT_TRACE("fail in create input port for ATC OMX VDEC component\r\n");
    goto OPEN_FAILED;
  }
  decoder->out_port = atc_omx_core_new_port (decoder->gomx, 1);
  if (NULL == decoder->gomx) {
    PRINT_TRACE("fail in create output port for ATC OMX VDEC component\r\n");
    goto OPEN_FAILED;
  }

  pthread_mutex_init(&decoder->ready_lock, NULL);
  decoder->ready = false;
  decoder->gomx->settings_changed_cb = NULL;

  decoder->set_omxbuffer = false;
  decoder->had_set_infmt = false;

  decoder->last_error_ret = RET_ATCVDECINST_OK;

  decoder->input_format.role = NULL;
  decoder->input_format.eFormat = OMX_VIDEO_CodingUnused;
  decoder->input_format.fps_n = 0;
  decoder->input_format.fps_d = 1;
  decoder->input_format.interlaced = false;
  decoder->input_format.width = 0;
  decoder->input_format.height = 0;

  decoder->output_tmp_buf_sz = 0;
  decoder->output_tmp_buf = NULL;

  PRINT_TRACE("success --> core->omx_state: %d\r\n",
    decoder->gomx->omx_state);

  decoder->dump_fp = NULL;

#if 0
    decoder->dump_fp = fopen(decoder->dump_filename , "a+");
    if (NULL == decoder->dump_fp) {
      PRINT_ERROR(" fail in open dump file(%s), err: %s.\r\n",
        decoder->dump_filename, strerror(errno));
    }
#endif

  return decoder;

OPEN_FAILED:

  if (NULL != decoder->in_port) {
    PRINT_TRACE ("--> atc_omx_port_finish(in_port)\r\n");
    atc_omx_port_finish (decoder->in_port);
  }

  if (NULL != decoder->out_port) {
    PRINT_TRACE ("--> atc_omx_port_finish(out_port)\r\n");
    atc_omx_port_finish (decoder->out_port);
  }

  if (NULL != decoder->gomx) {
    PRINT_ERROR("fail in open ATC OMX VDEC component\r\n");
    PRINT_TRACE ("--> atc_omx_core_stop\r\n");
    atc_omx_core_stop (decoder->gomx);
    PRINT_TRACE ("--> atc_omx_core_unload\r\n");
    atc_omx_core_unload (decoder->gomx);
  }

  if (NULL != decoder) {
    free(decoder);
  }

  return NULL;
}

bool atc_vdec_start(void *inst)
{
  ATCVIDDECINST *self = (ATCVIDDECINST *)inst;
  AtcOmxCore *gomx = NULL;

  PRINT_DEBUG ("entry\r\n");

  if (NULL == self) {
    PRINT_ERROR("fail for invalid args\r\n");
    return false;
  }

  gomx = (AtcOmxCore *) self->gomx;
  if (NULL == gomx) {
    PRINT_ERROR("fail for OpenMax ATC VDEC Component\r\n");
    return false;
  }

  pthread_mutex_lock(&self->ready_lock);

  if (!self->had_set_infmt) {
    PRINT_ERROR("fail for input format hasn't been set\r\n");
    pthread_mutex_unlock(&self->ready_lock);
    return false;
  }

  self->last_error_ret = RET_ATCVDECINST_OK;

  pthread_mutex_unlock(&self->ready_lock);

  if (gomx->omx_state == OMX_StateLoaded) {
    pthread_mutex_lock(&self->ready_lock);

    PRINT_TRACE("omx: prepare\n");

    omx_setup (self);

    PRINT_TRACE ("setup_ports\r\n");
    setup_ports (self);

    PRINT_TRACE ("g_omx_core_prepare\r\n");
    atc_omx_core_prepare (self->gomx);

    PRINT_TRACE ("setup_buflist, had_set_infmt=%d\r\n",
      self->had_set_infmt);
    if (!self->had_set_infmt) {
      pthread_mutex_unlock(&self->ready_lock);
      return false;
    }

    self->ready = TRUE;

    pthread_mutex_unlock(&self->ready_lock);

    if (gomx->omx_state != OMX_StateIdle) {
      PRINT_ERROR("fail in omx core prepare\r\n");
      return false;
    }

    PRINT_DEBUG (" --> g_omx_core_start\r\n");
    atc_omx_core_start (gomx);

    PRINT_DEBUG ("--> gomx->omx_state = %d\r\n", gomx->omx_state);
    if (gomx->omx_state != OMX_StateExecuting) {
      PRINT_ERROR("fail in atc_omx_core_start\r\n");
      return false;
    }
  }

  /* we do not start the task yet if the pad is not connected */
  if (self->ready) {
    /** @todo link callback function also needed */
    atc_omx_port_resume (self->in_port);
    atc_omx_port_resume (self->out_port);
  }

  if (self->callbacks != NULL) {
    OMX_BUFFERHEADERTYPE *omx_buffer = NULL;
    do {
      omx_buffer = atc_omx_port_request_buffer_nb(self->in_port);
      PRINT_TRACE("request_buffer %p\r\n", omx_buffer);
      if (omx_buffer == NULL)
        break;
      VDEC_BUFFER_INFO_T buf_info;
      buf_info.buffer = omx_buffer->pBuffer;
      buf_info.bufSz = omx_buffer->nAllocLen;
      self->callbacks->onEmptyBufferDone(self->app_data, &buf_info);
    } while(omx_buffer != NULL);
  }

  PRINT_TRACE("success\r\n");

  return TRUE;
}

bool  atc_vdec_get_output_formats(void *inst,
  __u64 *psupport_fmts, __u32 *pformat)
{
  if ((NULL == inst) || (NULL == psupport_fmts) || (NULL == pformat)) {
    PRINT_ERROR("fail for invalid args\r\n");
    return false;
  }

  *psupport_fmts = (1 << ATC_PIX_FMT_NV12M_PRIVATE1);
  *pformat = ATC_PIX_FMT_NV12M_PRIVATE1;

  return TRUE;
}

bool  atc_vdec_set_input_format(void *inst,
  ATC_VDEC_INPUT_FMT_INFO_T *format)
{
  ATCVIDDECINST *self = (ATCVIDDECINST *)inst;
  AtcOmxCore *gomx;
  OMX_PARAM_PORTDEFINITIONTYPE port_def;
  OMX_PARAM_COMPONENTROLETYPE roleParams;
  OMX_INDEXTYPE index = OMX_IndexComponentStartUnused;
  OMX_ERRORTYPE err = OMX_ErrorNone;
  bool fgRet = false;
  bool is_format_change = false;
  bool needs_disable = false;
  OMX_ERRORTYPE omx_err = OMX_ErrorNone;
  OMX_BOOL is_our_extractor = OMX_TRUE;
  OMX_VIDEO_PROGRAM_TYPE omx_video_program_type = OMX_VIDEO_PROGRAM_UNKNOWN;

  if ((NULL == self) || (NULL == format)) {
    PRINT_ERROR("fail for invalid args\r\n");
    return false;
  }
  PRINT_TRACE (" receive eVProgramType: 0x%x\r\n", format->eVProgramType);

  memset(&roleParams, 0, sizeof(OMX_PARAM_COMPONENTROLETYPE));

  gomx = (AtcOmxCore *) self->gomx;
  if (NULL == gomx) {
    PRINT_ERROR("fail for OpenMax ATC VDEC Component\r\n");
    return false;
  }

  PRINT_DEBUG ("--> core->omx_state: %d\r\n",
    gomx->omx_state);

  PRINT_DEBUG ("--> width:%d, height:%d\r\n",
  	format->width, format->height);

  atc_omx_port_get_port_definition (self->in_port, &port_def);

  /* Check if the caps change is a real format change or if only irrelevant
   * parts of the caps have changed or nothing at all.
   */
  is_format_change |= port_def.format.video.nFrameWidth != format->width;
  is_format_change |= port_def.format.video.nFrameHeight != format->height;
  //Check whether Framerate is changed. Need to onsider whether fps_d is 0.
  is_format_change |= ((port_def.format.video.xFramerate == 0) &&
  (format->fps_n != 0) && (format->fps_d != 0));
  if (format->fps_d != 0)
    is_format_change |=
    (port_def.format.video.xFramerate != ((format->fps_n ) / (format->fps_d)));
  else{
    is_format_change |= (port_def.format.video.xFramerate != 0);
    PRINT_TRACE("--> fps_d of framerate is 0\r\n");
  }

  needs_disable = (gomx->omx_state != OMX_StateLoaded);

  PRINT_TRACE("--> needs_disable: %d, is_format_change: %d\r\n",
    needs_disable, is_format_change);

  /* If the component is not in Loaded state and a real format change happens
   * we have to disable the port and re-allocate all buffers. If no real
   * format change happened we can just exit here.
   */
  if (needs_disable && !is_format_change) {
    PRINT_TRACE("--> Already running and same input format\r\n");
    return TRUE;
  }

  if (needs_disable && is_format_change) {
    if (!atc_vdec_stop(self)) {
      PRINT_TRACE("fail in atc_vdec_reset\r\n");
      return false;
    }
    atc_omx_core_populate(gomx);
  }

  self->input_format.fps_d = format->fps_d;
  self->input_format.fps_n = format->fps_n;
  self->input_format.width = format->width;
  self->input_format.height = format->height;
  self->input_format.interlaced = format->interlaced;

  PRINT_TRACE("--> framerate_num: %d, framerate_denom: %d\r\n",
    self->input_format.fps_n,
    self->input_format.fps_d);

  PRINT_DEBUG ("--> VideoCodingType = 0x%08x\r\n",
    format->eVType);

  if (!set_component_role(self, gomx, format->eVType)) {
    PRINT_ERROR("fail in gst_omx_atcwfdviddec_set_component_role");
    return false;
  }

  err = atc_omx_cmp_get_ext_idx(gomx,
    (OMX_STRING)"OMX.atc.index.extractorType", &index);
  if (err != OMX_ErrorNone) {
    PRINT_ERROR("fail in get index.extractorType index, error is 0x%x\r\n", err);
    return false;
  }

  is_our_extractor = OMX_FALSE;

  PRINT_TRACE("set atc extractor type\r\n");
  err = atc_omx_cmp_set_config(gomx, index, &is_our_extractor);
  if (err != OMX_ErrorNone) {
    PRINT_ERROR ("fail in set index.extractorType config, error is 0x%x\r\n", err);
    return false;
  }

  PRINT_DEBUG ("--> core->omx_state: %d\r\n",
    gomx->omx_state);

  omx_err = atc_omx_port_get_port_definition(self->in_port, &port_def);
  if (NULL == gomx) {
    PRINT_ERROR("fail in atc_omx_port_get_port_definition, err: 0x%08x\r\n",
      omx_err);
    return false;
  }
  port_def.format.video.nFrameWidth = self->input_format.width;
  port_def.format.video.nFrameHeight = self->input_format.height;
  omx_err = atc_omx_port_update_port_definition(self->in_port, &port_def);
  if (NULL == gomx) {
    PRINT_ERROR("fail in atc_omx_port_update_port_definition, err: 0x%08x\r\n", omx_err);
    return false;
  }
  if ( format->eVProgramType == VIDEO_PROGRAM_WELINK)
  {
      err = atc_omx_cmp_get_ext_idx(gomx,
        (OMX_STRING)"OMX.atc.index.VideoProgramType", &index);
      if (err != OMX_ErrorNone) {
        PRINT_ERROR("fail in get OMX.atc.index.VideoProgramType index, error is 0x%x\r\n", err);
        return false;
      }
      omx_video_program_type = OMX_VIDEO_PROGRAM_WELINK;
      PRINT_TRACE("set video program type Welink, omx_video_program_type: 0x%x\r\n", omx_video_program_type);
      err = atc_omx_cmp_set_config(gomx, index, &omx_video_program_type);
      if (err != OMX_ErrorNone) {
        PRINT_ERROR ("fail in set OMX.atc.index.isWelink config, error is 0x%x\r\n", err);
        return false;
      }
  }

  self->had_set_infmt = TRUE;

  PRINT_DEBUG ("exit, return %s \r\n",
    (fgRet ? "TRUE" : "false"));

  return TRUE;
}

void atc_vdec_dump_input_data(ATCVIDDECINST *inst, __u8 *pdata, __u32 datasz)
{
  size_t WriteBytes = 0;

  if ((NULL == inst) || (NULL == inst->dump_fp)) {
    return;
  }

  WriteBytes = fwrite(pdata, datasz, 1, inst->dump_fp);
  if (WriteBytes != 1) {
    PRINT_ERROR("fail in write data to dump file, err: %s.\r\n", strerror(errno));
  }
}

void atc_vdec_set_callbacks(void *inst, BUFFER_CALLBACKTYPE *callbacks, void *app_data)
{
    ATCVIDDECINST *self = (ATCVIDDECINST *)inst;
    PRINT_TRACE("callbacks:%p, app_data:%p\r\n", callbacks, app_data);
    self->callbacks = callbacks;
    self->app_data = app_data;
}

bool  atc_vdec_decode(void *inst,
  __u8 *pdata, __u32 datasz, __s64 timestampus, __u32 flags)
{
  AtcOmxCore *gomx = NULL;
  AtcOmxPort *in_port = NULL;
  ATCVIDDECINST *self = (ATCVIDDECINST *)inst;
  bool ret = false;

  if ((NULL == self) || (NULL == self->gomx)) {
    PRINT_ERROR ("fail for invalid args\r\n");
    return false;
  }

  atc_vdec_dump_input_data((ATCVIDDECINST *)inst, pdata, datasz);

  gomx = self->gomx;

  PRINT_DEBUG ("omx_state is %d, datasz is %d\r\n", gomx->omx_state, datasz);

  in_port = self->in_port;
  if (in_port->enabled) {
    __u32 data_offset = 0;

  if (gomx->omx_state != OMX_StateExecuting) {
    PRINT_ERROR (" --> Whoa! very wrong omx state: %d\r\n", gomx->omx_state);
    goto out_flushing;
  }

    {
      OMX_BUFFERHEADERTYPE *omx_buffer;
      omx_buffer = atc_omx_port_find_buffer(in_port, pdata);

      if (NULL == omx_buffer) {
        PRINT_TRACE("omx_buffer is NULL pdata:%p\r\n", pdata);
        goto out_flushing;
      }
      //omx_buffer->nFlags &= ~(OMX_BUFFERFLAG_EOS | OMX_BUFFERFLAG_REPEAT | OMX_BUFFERFLAG_SIZECHANGE);
      //omx_buffer->nFlags &= ~(OMX_BUFFERFLAG_DISCONTINUITY | OMX_BUFFERFLAG_TIMEOUTAU);
      omx_buffer->nFlags = flags;
      if (timestampus != ATC_CLOCK_TIME_NONE) {
        omx_buffer->nTimeStamp = timestampus;
      } else {
        omx_buffer->nTimeStamp = 0;
      }
      omx_buffer->nOffset = 0;
      omx_buffer->nFilledLen = datasz;
      PRINT_DEBUG("atc_omx_port_release_buffer, datasz: %d, flags:0x%x, pts:%lld\r\n",
          datasz, omx_buffer->nFlags, omx_buffer->nTimeStamp);

      atc_omx_port_release_buffer (in_port, omx_buffer);
    }
    ret = TRUE;
  } else {
    PRINT_TRACE("done");
  }
  PRINT_DEBUG("exit, ret: %s\r\n",
    (ret ? "TRUE" : "false"));

  return ret;

  /* special conditions */
out_flushing:
  {
    if (gomx->omx_error) {
      PRINT_TRACE("fail in ATC OMX VDEC Component, omx_err: 0x%08x\r\n",
        gomx->omx_error);
    } else if ((gomx->omx_state != OMX_StateExecuting) &&
        (gomx->omx_state != OMX_StatePause)) {
      PRINT_TRACE("fail for ATC OMX VDEC Component is in wrong state(%d)\r\n",
        gomx->omx_state);
    }

    return false;
  }
}

ATC_VDEC_ERRCODE atc_vdec_release_output_buffer(void * inst,
  VDEC_BUFFER_INFO_T *prOutInfo)
{
  ATCVIDDECINST *self = (ATCVIDDECINST *)inst;
  AtcOmxCore *gomx = NULL;
  AtcOmxPort *out_port = NULL;
  ATC_VDEC_ERRCODE ret = RET_ATCVDECINST_OK;

  if (NULL == self) {
    PRINT_ERROR ("fail for invalid args\r\n");
    return RET_ATCVDECINST_FAIL;
  }

  gomx = self->gomx;

  PRINT_DEBUG ("enter\r\n");

  /* do not bother if we have been setup to bail out */
  if ((ret = self->last_error_ret) != RET_ATCVDECINST_OK) {
    PRINT_ERROR(" ret is %d\r\n", ret);
    goto LEAVEGETOUTPUT;
  }

  {
    if (!self->ready) {
      PRINT_ERROR("exit for not ready\r\n");
      ret = RET_ATCVDECINST_WRONG_STATE;
      return RET_ATCVDECINST_FAIL;
    }

    if ((ret = self->last_error_ret) != RET_ATCVDECINST_OK) {
      PRINT_ERROR(" ret is %d\r\n", ret);
      ret = RET_ATCVDECINST_WRONG_STATE;
      goto LEAVEGETOUTPUT;
    }

    if (gomx->omx_state != OMX_StateExecuting) {
      PRINT_ERROR ("fail for omx_state(%d) is not in OMX_StateExecuting\r\n", gomx->omx_state);
      ret = RET_ATCVDECINST_WRONG_STATE;
      goto LEAVEGETOUTPUT;
    }

    out_port = self->out_port;

    if (out_port->enabled) {
      OMX_BUFFERHEADERTYPE *omx_buffer = NULL;
      omx_buffer = atc_omx_port_find_buffer(out_port, (__u8 *)prOutInfo->buffer);
      if (omx_buffer == NULL) {
        PRINT_TRACE("can not find buffer:%p\r\n", prOutInfo->buffer);
        goto LEAVEGETOUTPUT;
      }
      omx_buffer->nFilledLen = 0;
      omx_buffer->nFlags = 0;
      atc_omx_port_release_buffer (out_port, omx_buffer);
    } else {
      ret = RET_ATCVDECINST_WRONG_STATE;
    }
  }

LEAVEGETOUTPUT:

  if (gomx->omx_error != OMX_ErrorNone) {
    PRINT_TRACE ("gomx->omx_error = %d\r\n",
      gomx->omx_error);
    ret = RET_ATCVDECINST_COMPONENT_ERR;
  }

  return ret;
}

bool atc_vdec_flush(void *inst)
{
  ATCVIDDECINST *self = (ATCVIDDECINST *)inst;
  AtcOmxCore *gomx = NULL;

  PRINT_INFO(" enter\r\n");

  if (NULL == self) {
    PRINT_ERROR ("fail for invalid args\r\n");
    return false;
  }
  gomx = (AtcOmxCore *) self->gomx;
  if (NULL == gomx) {
    PRINT_ERROR("fail for OpenMax ATC VDEC Component\r\n");
    return false;
  }
  pthread_mutex_lock(&self->ready_lock);

  if (self->ready) {
    /** @todo disable this until we properly reinitialize the buffers. */
    /* unlock loops */
    atc_omx_core_flush_start (gomx);
    atc_omx_core_flush_stop (gomx);
  }

  pthread_mutex_unlock(&self->ready_lock);
  PRINT_INFO(" exit\r\n");
  return true;
}

bool atc_vdec_stop(void *inst)
{
  ATCVIDDECINST *self = (ATCVIDDECINST *)inst;
  AtcOmxCore *gomx = NULL;

  PRINT_DEBUG ("enter\r\n");

  if (NULL == self) {
    PRINT_ERROR ("fail for invalid args\r\n");
    return false;
  }

  gomx = (AtcOmxCore *) self->gomx;
  if (NULL == gomx) {
    PRINT_ERROR("fail for OpenMax ATC VDEC Component\r\n");
    return false;
  }

  /* persuade task to bail out */
  omx_atomic_set((__u32 *)&self->last_error_ret, (__u32)RET_ATCVDECINST_FLUSHING);

  pthread_mutex_lock(&self->ready_lock);

  if (self->ready) {
    /** @todo disable this until we properly reinitialize the buffers. */
    /* unlock loops */
    atc_omx_core_flush_start (gomx);
    atc_omx_core_flush_stop (gomx);

    /* unlock */
    PRINT_TRACE ("--> atc_omx_port_pause(in_port)\r\n");
    atc_omx_port_pause (self->in_port);
    PRINT_TRACE ("--> atc_omx_port_pause(out_port)\r\n");
    atc_omx_port_pause (self->out_port);

    PRINT_TRACE ("--> g_omx_core_stop\r\n");
    atc_omx_core_stop (self->gomx);
  }

  pthread_mutex_unlock(&self->ready_lock);

  return TRUE;
}

bool  atc_vdec_close(void *inst)
{
  ATCVIDDECINST *self = (ATCVIDDECINST *)inst;
  AtcOmxCore *gomx = NULL;

  if (NULL == self) {
    PRINT_ERROR ("fail for invalid args\r\n");
    return false;
  }

  gomx = self->gomx;

  if (NULL == gomx) {
    PRINT_TRACE ("fail for no omx component\r\n");
    return false;
  }

  pthread_mutex_lock(&self->ready_lock);

  if (self->ready) {
    /* unlock */
    PRINT_TRACE ("--> g_omx_port_finish(in_port)\r\n");
    atc_omx_port_finish (self->in_port);

    PRINT_TRACE ("--> g_omx_port_finish(out_port)\r\n");
    atc_omx_port_finish (self->out_port);

    PRINT_TRACE ("--> g_omx_core_stop\r\n");
    atc_omx_core_stop (gomx);

    PRINT_TRACE ("--> g_omx_core_unload\r\n");
    atc_omx_core_unload (gomx);

    PRINT_TRACE ("--> g_omx_core_unload success\r\n");
    self->ready = false;
    self->had_set_infmt = false;
    self->set_omxbuffer = false;
  }

  pthread_mutex_unlock(&self->ready_lock);
  PRINT_TRACE ("--> core->omx_state: %d\r\n",
    gomx->omx_state);
  if (gomx->omx_state != OMX_StateLoaded &&
      gomx->omx_state != OMX_StateInvalid) {
    PRINT_TRACE ("fail for invalid omx_state: %d\r\n",
      gomx->omx_state);
    return false;
  }

  PRINT_TRACE ("--> atc_omx_core_free\r\n");
  atc_omx_core_free (gomx);
  PRINT_TRACE ("--> atc_omx_core_free done\r\n");

  pthread_mutex_destroy(&self->ready_lock);

  if (NULL != self->output_tmp_buf) {
    free(self->output_tmp_buf);
    self->output_tmp_buf = NULL;
    self->output_tmp_buf_sz = 0;
  }

  if (NULL != self->dump_fp) {
    fclose(self->dump_fp);
    self->dump_fp = NULL;
  }

  if (NULL != self->input_format.role) {
    free(self->input_format.role);
    self->input_format.role = NULL;
  }

  free(self);

  return TRUE;
}

