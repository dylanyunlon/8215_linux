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

#ifndef ATCOMX_UTILS_H
#define ATCOMX_UTILS_H

#include <stdbool.h>
#include <linux/types.h>
#include <string.h>

#include <async_queue.h>
#include "atcsema.h"
#include <OMX_Core.h>
#include <OMX_Component.h>

/* Enums. */

enum _AtcOmxPortType
{
  GOMX_PORT_INPUT,
  GOMX_PORT_OUTPUT,
  GOMX_PORT_NUM
};/* Enums. */

typedef struct _AtcOmxCore AtcOmxCore;
typedef struct _AtcOmxPort AtcOmxPort;
typedef struct _AtcOmxImp AtcOmxImp;
typedef struct _AtcOmxSymbolTable AtcOmxSymbolTable;
typedef enum _AtcOmxPortType AtcOmxPortType;

typedef void (*AtcOmxCb) (AtcOmxCore * core);
typedef void (*AtcOmxPortCb) (AtcOmxPort * port);


typedef OMX_ERRORTYPE (*initFnPtr) (void);
typedef OMX_ERRORTYPE (*deinitFnPtr) (void);
typedef OMX_ERRORTYPE (*gethandleFnPtr) (OMX_HANDLETYPE * handle,
      OMX_STRING name, OMX_PTR data, OMX_CALLBACKTYPE * callbacks);
typedef OMX_ERRORTYPE (*freehandleFnPtr) (OMX_HANDLETYPE handle);

/* Structures. */

struct _AtcOmxSymbolTable
{
  OMX_ERRORTYPE (*init) (void);
  OMX_ERRORTYPE (*deinit) (void);
  OMX_ERRORTYPE (*get_handle) (OMX_HANDLETYPE * handle,
      OMX_STRING name, OMX_PTR data, OMX_CALLBACKTYPE * callbacks);
  OMX_ERRORTYPE (*free_handle) (OMX_HANDLETYPE handle);
};

struct _AtcOmxImp
{
  __u32 client_count;
  void *dl_handle;
  AtcOmxSymbolTable sym_table;
  pthread_mutex_t mutex;
};

struct _AtcOmxCore
{
  void * object;

  OMX_HANDLETYPE omx_handle;
  OMX_ERRORTYPE omx_error;

  OMX_STATETYPE omx_state;
  pthread_cond_t omx_state_condition;
  pthread_mutex_t omx_state_mutex;

  void *ports[GOMX_PORT_NUM];

  ATCSEMA *done_sem;
  ATCSEMA *flush_sem;
  ATCSEMA *port_sem;

  AtcOmxCb settings_changed_cb;
  AtcOmxImp *imp;

  bool done;

  char *library_name;
  char *component_name;
  void *app_data;
  OMX_CALLBACKTYPE *callbacks;
};

struct _AtcOmxPort
{
  AtcOmxCore *core;
  AtcOmxPortType type;

  __u32 num_buffers;
  __u32 buffer_size;
  __u32 port_index;
  OMX_BUFFERHEADERTYPE **buffers;

  pthread_mutex_t mutex;
  bool enabled;
  bool omx_allocate;   /**< Setup with OMX_AllocateBuffer rather than OMX_UseBuffer */
  AsyncQueue *queue;
};

/* Functions. */

void atc_omx_init (void);
void atc_omx_deinit (void);

AtcOmxCore *atc_omx_core_new (void *object);
void atc_omx_core_free (AtcOmxCore * core);
bool atc_omx_core_init (AtcOmxCore * core);
void atc_omx_core_deinit(AtcOmxCore *core);
void atc_omx_core_prepare (AtcOmxCore * core);
void atc_omx_core_start (AtcOmxCore * core);
void atc_omx_core_pause (AtcOmxCore * core);
void atc_omx_core_stop (AtcOmxCore * core);
void atc_omx_core_unload (AtcOmxCore * core);
void atc_omx_core_populate(AtcOmxCore * core);
void atc_omx_core_set_done (AtcOmxCore * core);
void atc_omx_core_wait_for_done (AtcOmxCore * core);
void atc_omx_core_flush_start (AtcOmxCore * core);
void atc_omx_core_flush_stop (AtcOmxCore * core);
AtcOmxPort *atc_omx_core_new_port (AtcOmxCore * core, __u32 index);

AtcOmxPort *atc_omx_port_new (AtcOmxCore * core, __u32 index);
void atc_omx_port_free (AtcOmxPort * port);
void atc_omx_port_setup (AtcOmxPort * port);
void atc_omx_port_push_buffer (AtcOmxPort * port,
    OMX_BUFFERHEADERTYPE * omx_buffer);
OMX_BUFFERHEADERTYPE *atc_omx_port_request_buffer (AtcOmxPort * port);
OMX_BUFFERHEADERTYPE *atc_omx_port_request_buffer_nb(AtcOmxPort * port);
OMX_BUFFERHEADERTYPE *atc_omx_port_find_buffer(AtcOmxPort * port, __u8 *pbuf);
void atc_omx_port_release_buffer (AtcOmxPort * port,
    OMX_BUFFERHEADERTYPE * omx_buffer);
void atc_omx_port_resume (AtcOmxPort * port);
void atc_omx_port_pause (AtcOmxPort * port);
void atc_omx_port_flush (AtcOmxPort * port);
void atc_omx_port_enable (AtcOmxPort * port);
void atc_omx_port_disable (AtcOmxPort * port);
void atc_omx_port_finish (AtcOmxPort * port);
OMX_ERRORTYPE atc_omx_port_get_port_definition (AtcOmxPort * port,
    OMX_PARAM_PORTDEFINITIONTYPE * port_def);
OMX_ERRORTYPE atc_omx_port_update_port_definition (AtcOmxPort * port,
    OMX_PARAM_PORTDEFINITIONTYPE * port_def);
void *atcomx_core_new (void *object, char *library_name, char *component_name, OMX_CALLBACKTYPE *callbacks, void *app_data);
OMX_ERRORTYPE atc_omx_cmp_get_parameter (AtcOmxCore *core,
  OMX_INDEXTYPE index, void *param);
OMX_ERRORTYPE atc_omx_cmp_set_parameter (AtcOmxCore * core, OMX_INDEXTYPE index,
  void *param);
OMX_ERRORTYPE atc_omx_cmp_get_ext_idx(AtcOmxCore * core, OMX_STRING paramName,
  OMX_INDEXTYPE* index);
OMX_ERRORTYPE atc_omx_cmp_set_config (AtcOmxCore * core, OMX_INDEXTYPE index,
  void * config);

#define ATC_OMX_INIT_PARAM(st) do {  \
  memset ((st), 0, sizeof (*(st))); \
  (st)->nSize = sizeof (*(st)); \
  (st)->nVersion.s.nVersionMajor = OMX_VERSION_MAJOR; \
  (st)->nVersion.s.nVersionMinor = OMX_VERSION_MINOR; \
  (st)->nVersion.s.nRevision = OMX_VERSION_REVISION; \
  (st)->nVersion.s.nStep = OMX_VERSION_STEP; \
} while(0);


#endif /* ATCOMX_UTILS_H */
