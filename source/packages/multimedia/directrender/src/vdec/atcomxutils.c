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
#include <sys/time.h>
#include <dlfcn.h>
#include "atcomxutils.h"
#include "atcdtlog.h"

static inline void change_state (AtcOmxCore * core, OMX_STATETYPE state);

static inline void wait_for_state (AtcOmxCore * core, OMX_STATETYPE state);

static void
core_deinit (AtcOmxCore * core);

static inline void
got_buffer (AtcOmxCore * core,
    AtcOmxPort * port, OMX_BUFFERHEADERTYPE * omx_buffer);

static OMX_ERRORTYPE
EventHandler (OMX_HANDLETYPE omx_handle,
    OMX_PTR app_data,
    OMX_EVENTTYPE event, OMX_U32 data_1, OMX_U32 data_2, OMX_PTR event_data);

static OMX_ERRORTYPE
EmptyBufferDone (OMX_HANDLETYPE omx_handle,
    OMX_PTR app_data, OMX_BUFFERHEADERTYPE * omx_buffer);

static OMX_ERRORTYPE
FillBufferDone (OMX_HANDLETYPE omx_handle,
    OMX_PTR app_data, OMX_BUFFERHEADERTYPE * omx_buffer);

static inline const char *omx_state_to_str (OMX_STATETYPE omx_state);

static inline const char *omx_error_to_str (OMX_ERRORTYPE omx_error);

static inline AtcOmxPort *get_port (AtcOmxCore * core, __u32 index);

static inline void port_free_buffers (AtcOmxPort * port);

static inline void port_allocate_buffers (AtcOmxPort * port);

static inline void port_start_buffers (AtcOmxPort * port);

static OMX_CALLBACKTYPE atcomxcallbacks =
    { EventHandler, EmptyBufferDone, FillBufferDone };

/*
 * Util
 */

typedef void (*AtcOmxPortFunc) (AtcOmxPort * port);

static inline void
core_for_each_port (AtcOmxCore * core, AtcOmxPortFunc func)
{
  __u32 index;

  for (index = 0; index < GOMX_PORT_NUM; index++) {
    AtcOmxPort *port;

    port = get_port (core, index);

    if (port)
      func (port);
  }
}

/*
 * Main
 */

static AtcOmxImp *imp_new (const char * name);
static void imp_free (AtcOmxImp * imp);

static AtcOmxImp *
imp_new (const char * name)
{
  AtcOmxImp *imp;

  imp = (AtcOmxImp *)malloc(sizeof(AtcOmxImp));
  if (imp == NULL)
    return NULL;
  memset(imp, 0, sizeof(AtcOmxImp));

  /* Load the OpenMAX IL symbols */
  {
    void *handle;

    PRINT_TRACE ("enter, loading: %s\r\n", (char *)name);

    imp->dl_handle = handle = dlopen (name, RTLD_LAZY);

    PRINT_INFO("dlopen(%s) -> %p\r\n", name, handle);

    if (!handle) {
      PRINT_ERROR("fail for %s\r\n", dlerror ());
      free (imp);
      return NULL;
    }

    pthread_mutex_init(&imp->mutex, NULL);
    imp->sym_table.init = (initFnPtr)dlsym (handle, "OMX_Init");
    if (imp->sym_table.init == NULL) {
      PRINT_ERROR("fail in dlsym (handle, OMX_Init) %s\r\n", dlerror ());
    }
    imp->sym_table.deinit = (deinitFnPtr)dlsym (handle, "OMX_Deinit");
    if (imp->sym_table.deinit == NULL) {
      PRINT_ERROR("fail in dlsym (handle, OMX_Deinit); %s\r\n", dlerror ());
    }
    imp->sym_table.get_handle = (gethandleFnPtr)dlsym (handle, "OMX_GetHandle");
    if (imp->sym_table.get_handle == NULL) {
      PRINT_ERROR("fail in dlsym (handle, OMX_GetHandle); %s\r\n", dlerror ());
    }
    imp->sym_table.free_handle = (freehandleFnPtr)dlsym (handle, "OMX_FreeHandle");
    if (imp->sym_table.free_handle == NULL) {
      PRINT_ERROR("fail in dlsym (handle, OMX_FreeHandle); %s\r\n", dlerror ());
    }
  }

  return imp;
}

static void
imp_free (AtcOmxImp * imp)
{
  if (NULL != imp->dl_handle) {
    dlclose (imp->dl_handle);
  }
  pthread_mutex_destroy(&imp->mutex);
  free (imp);
}

static inline AtcOmxImp *
request_imp (const char * name)
{
  AtcOmxImp *imp = NULL;

  imp = imp_new (name);
  if (!imp)
    return NULL;
  pthread_mutex_lock(&imp->mutex);

  {
    OMX_ERRORTYPE omx_error;
    omx_error = imp->sym_table.init ();
    if (omx_error) {
      pthread_mutex_unlock(&imp->mutex);
      return NULL;
    }
  }
  pthread_mutex_unlock(&imp->mutex);

  return imp;
}

static inline void
release_imp (AtcOmxImp * imp)
{
  imp->sym_table.deinit();
  imp_free(imp);
}

/*
 * Core
 */

AtcOmxCore *
atc_omx_core_new (void *object)
{
  AtcOmxCore *core;

  core = (AtcOmxCore *)malloc(sizeof(AtcOmxCore));
  if (core == NULL)
    return NULL;
  memset(core, 0, sizeof(AtcOmxCore));

  core->object = object;
  //core->ports = NULL;

  pthread_cond_init(&core->omx_state_condition, NULL);
  pthread_mutex_init(&core->omx_state_mutex, NULL);

  core->done_sem = atc_sem_new ();
  core->flush_sem = atc_sem_new ();
  core->port_sem = atc_sem_new ();

  core->omx_state = OMX_StateInvalid;

  return core;
}

void
atc_omx_core_free (AtcOmxCore * core)
{
  core_deinit (core);

  atc_sem_free (core->port_sem);
  atc_sem_free (core->flush_sem);
  atc_sem_free (core->done_sem);

  pthread_mutex_destroy(&core->omx_state_mutex);
  pthread_cond_destroy(&core->omx_state_condition);

  free (core);
}

void
atc_omx_core_deinit(AtcOmxCore *core)
{
  if ((!core) || (!core->imp))
    return;

  if ((core->omx_state == OMX_StateLoaded) ||
    (core->omx_state == OMX_StateInvalid)) {
    if (core->omx_handle) {
      core->omx_error = core->imp->sym_table.free_handle (core->omx_handle);
      PRINT_DEBUG("OMX_FreeHandle(%p) -> %d\r\n",
          core->omx_handle, core->omx_error);
    }
  } else {
    PRINT_ERROR("Incorrect omx state: %d\r\n",
        core->omx_state);
  }

  release_imp (core->imp);
  core->imp = NULL;
}

bool
atc_omx_core_init (AtcOmxCore * core)
{
  PRINT_TRACE("enter, loading: %s (%s)\r\n",
      core->component_name, core->library_name);

  core->imp = request_imp (core->library_name);

  if (!core->imp) {
    PRINT_ERROR("fail in request_imp, loading: %s (%s)\r\n",
        core->component_name, core->library_name);
    return false;
  }

  core->omx_error = core->imp->sym_table.get_handle (&core->omx_handle,
      (char *) core->component_name, core, &atcomxcallbacks);

  PRINT_INFO("Get handle end\r\n");

  if (OMX_ErrorNone != core->omx_error) {
    PRINT_ERROR("fail for omx_error != NULL, loading: %s (%s)\r\n",
        core->component_name, core->library_name);

    return false;
  }

  core->omx_state = OMX_StateLoaded;
  PRINT_TRACE("exit, success\r\n");

  return true;
}

static void
core_deinit (AtcOmxCore * core)
{
  if (NULL == core->imp)
    return;

  if ((core->omx_state == OMX_StateLoaded) ||
      (core->omx_state == OMX_StateInvalid)) {
    if (NULL != core->omx_handle) {
      core->omx_error = core->imp->sym_table.free_handle (core->omx_handle);
      PRINT_TRACE("OMX_FreeHandle(%p) -> %d\r\n",
                   core->omx_handle, core->omx_error);
    }
  } else {
    PRINT_ERROR("core_deinit error, Incorrect state: %s, component_name: %s\r\n",
      omx_state_to_str (core->omx_state), core->component_name);
  }

  free (core->library_name);
  free (core->component_name);
  PRINT_TRACE("release_imp(%p)\r\n", core->imp);
  release_imp (core->imp);
  core->imp = NULL;
}

void
atc_omx_core_prepare (AtcOmxCore * core)
{
  PRINT_TRACE("enter, set state to be OMX_StateIdle");
  change_state (core, OMX_StateIdle);

  /* Allocate buffers. */
  core_for_each_port (core, port_allocate_buffers);

  wait_for_state (core, OMX_StateIdle);
  PRINT_TRACE("exit");
}

void
atc_omx_core_start (AtcOmxCore * core)
{
  PRINT_TRACE("enter, omx_state = %d\r\n", core->omx_state);
  switch (core->omx_state)  {
  case OMX_StateIdle:
    change_state (core, OMX_StateExecuting);
    wait_for_state (core, OMX_StateExecuting);
    PRINT_INFO ("--> core->object(%p) (omx_state = %d)\r\n",
      core->object, core->omx_state);
    if (core->omx_state == OMX_StateExecuting) {
      PRINT_INFO ("--> core->object(%p) (omx_state = %d), port_start_buffers\r\n",
        core->object, core->omx_state);
      core_for_each_port (core, port_start_buffers);
    }
    break;
  case OMX_StatePause:
    change_state (core, OMX_StateExecuting);
    wait_for_state (core, OMX_StateExecuting);
    break;
  case OMX_StateExecuting:
    break;
  default:
    PRINT_TRACE("fail for invalid omx_state(%d), can't change to OMX_StateExecuting\r\n",
      core->omx_state);
    break;
  }
  PRINT_TRACE("exit, omx_state = %d\r\n", core->omx_state);
}

void
atc_omx_core_stop (AtcOmxCore * core)
{
  if (core->omx_state == OMX_StateExecuting ||
      core->omx_state == OMX_StatePause) {
    PRINT_INFO("--> set state to be OMX_StateIdle");
    change_state (core, OMX_StateIdle);
    wait_for_state (core, OMX_StateIdle);
  }
}

void
atc_omx_core_pause (AtcOmxCore * core)
{
  PRINT_TRACE("enter, set state to be OMX_StatePause");
  if (core->omx_state == OMX_StatePause) {
    return;
  }

  change_state (core, OMX_StatePause);
  wait_for_state (core, OMX_StatePause);
  PRINT_TRACE("exit");
}

void
atc_omx_core_unload (AtcOmxCore * core)
{
  if (core->omx_state == OMX_StateIdle ||
      core->omx_state == OMX_StateWaitForResources ||
      core->omx_state == OMX_StateInvalid) {
    if (core->omx_state != OMX_StateInvalid) {
      PRINT_INFO("--> (omx_state=%d) set state to be OMX_StateLoaded\r\n",
        core->omx_state);
      change_state (core, OMX_StateLoaded);
    }

    core_for_each_port (core, port_free_buffers);

    if (core->omx_state != OMX_StateInvalid) {
      PRINT_INFO("--> (omx_state=%d) set state to be OMX_StateLoaded\r\n",
        core->omx_state);
      wait_for_state (core, OMX_StateLoaded);
    }
  }

  core_for_each_port (core, atc_omx_port_free);
  core->ports[GOMX_PORT_INPUT] = NULL;
  core->ports[GOMX_PORT_OUTPUT] = NULL;
}

void atc_omx_core_populate(AtcOmxCore * core)
{
  if (core->omx_state == OMX_StateIdle ||
      core->omx_state == OMX_StateWaitForResources ||
      core->omx_state == OMX_StateInvalid) {
    if (core->omx_state != OMX_StateInvalid) {
      PRINT_INFO("--> (omx_state=%d) set state to be OMX_StateLoaded\r\n",
        core->omx_state);
      change_state (core, OMX_StateLoaded);
    }

    core_for_each_port (core, port_free_buffers);

    if (core->omx_state != OMX_StateInvalid) {
      PRINT_INFO("--> (omx_state=%d) set state to be OMX_StateLoaded\r\n",
        core->omx_state);
      wait_for_state (core, OMX_StateLoaded);
    }
  }
}

static inline AtcOmxPort *
get_port (AtcOmxCore * core, __u32 index)
{
  if (index < GOMX_PORT_NUM) {
    return (AtcOmxPort *)core->ports[index];
  }

  return NULL;
}

AtcOmxPort *
atc_omx_core_new_port (AtcOmxCore * core, __u32 index)
{
  AtcOmxPort *port = get_port (core, index);

  if (port) {
    PRINT_ERROR("port %d already exists\r\n", index);
    return port;
  }

  port = atc_omx_port_new (core, index);
  core->ports[index] = port;

  return port;
}

void
atc_omx_core_set_done (AtcOmxCore * core)
{
  atc_sem_up (core->done_sem);
}

void
atc_omx_core_wait_for_done (AtcOmxCore * core)
{
  atc_sem_down (core->done_sem);
}

void
atc_omx_core_flush_start (AtcOmxCore * core)
{
  core_for_each_port (core, atc_omx_port_pause);
}

void
atc_omx_core_flush_stop (AtcOmxCore * core)
{
  core_for_each_port (core, atc_omx_port_flush);
  core_for_each_port (core, atc_omx_port_resume);
}

/*
 * Port
 */

/**
 * note: this is not intended to be called directly by elements (which should
 * instead use atc_omx_core_new_port())
 */
AtcOmxPort *
atc_omx_port_new (AtcOmxCore * core, __u32 index)
{
  AtcOmxPort *port;
  port = (AtcOmxPort *)malloc(sizeof(AtcOmxPort));
  if (port == NULL)
    return NULL;

  port->core = core;
  port->port_index = index;
  port->num_buffers = 0;
  port->buffer_size = 0;
  port->buffers = NULL;

  port->enabled = true;
  port->queue = async_queue_new();
  pthread_mutex_init(&port->mutex, NULL);

  port->omx_allocate = true;

  return port;
}

void
atc_omx_port_free (AtcOmxPort * port)
{
  pthread_mutex_destroy(&port->mutex);
  async_queue_free (port->queue);

  free (port->buffers);
  free (port);
}

void
atc_omx_port_setup (AtcOmxPort * port)
{
  AtcOmxPortType type = GOMX_PORT_NUM;
  OMX_PARAM_PORTDEFINITIONTYPE param;

  ATC_OMX_INIT_PARAM (&param);

  param.nPortIndex = port->port_index;
  OMX_GetParameter (port->core->omx_handle, OMX_IndexParamPortDefinition,
      &param);

  switch (param.eDir) {
    case OMX_DirInput:
      type = GOMX_PORT_INPUT;
      break;
    case OMX_DirOutput:
      type = GOMX_PORT_OUTPUT;
      break;
    default:
      break;
  }

  port->type = type;
    /** @todo should it be nBufferCountMin? */
  port->num_buffers = param.nBufferCountActual;
  port->buffer_size = param.nBufferSize;

  PRINT_INFO("type=%d, num_buffers=%d, buffer_size=%ld, buffers= %p, port_index=%d\r\n",
      port->type, port->num_buffers, port->buffer_size,
      port->buffers, port->port_index);

  if (NULL != port->buffers) {
    free (port->buffers);
  }
  port->buffers = (OMX_BUFFERHEADERTYPE **)malloc(sizeof(OMX_BUFFERHEADERTYPE *) * port->num_buffers);
}

static void
port_allocate_buffers (AtcOmxPort * port)
{
  __u32 i;
  __u32 size;

  size = port->buffer_size;

  for (i = 0; i < port->num_buffers; i++) {
    if (port->omx_allocate) {
      OMX_AllocateBuffer (port->core->omx_handle, &port->buffers[i],
          port->port_index, NULL, size);
    } else {
      __u8 *buffer_data;
      buffer_data = (__u8 *)malloc (size);
      if (buffer_data) {
        memset(buffer_data, 0, size);
      }
      OMX_UseBuffer (port->core->omx_handle, &port->buffers[i],
          port->port_index, NULL, size, buffer_data);
    }
    PRINT_INFO("[%d] port:%d omx_buffer:%p, pbuffer:%p\r\n",
        i, port->type, port->buffers[i], port->buffers[i]->pBuffer);
  }
}

static void
port_free_buffers (AtcOmxPort * port)
{
  __u32 i;

  for (i = 0; i < port->num_buffers; i++) {
    OMX_BUFFERHEADERTYPE *omx_buffer;
    omx_buffer = port->buffers[i];
    if (omx_buffer != NULL) {
      if (port->type == GOMX_PORT_OUTPUT) {
        if ((!port->omx_allocate) && (omx_buffer->pBuffer != NULL)) {
          PRINT_INFO("-- free(%p), omx_buffer: %p\r\n",
            omx_buffer->pBuffer, omx_buffer);
          free (omx_buffer->pBuffer);
          omx_buffer->pBuffer = NULL;
        }
      }
      OMX_FreeBuffer (port->core->omx_handle, port->port_index, omx_buffer);
      port->buffers[i] = NULL;
    }
  }
}

static void
port_start_buffers (AtcOmxPort * port)
{
  __u32 i;

  AtcOmxCore *core = port->core;

  for (i = 0; i < port->num_buffers; i++) {
    OMX_BUFFERHEADERTYPE *omx_buffer;

    omx_buffer = port->buffers[i];

    /* If it's an input port we will need to fill the buffer, so put it in
     * the queue, otherwise send to omx for processing (fill it up). */
    if (port->type == GOMX_PORT_INPUT) {
      PRINT_INFO("core->object(%p) -- got_buffer\r\n", core->object);
      got_buffer (port->core, port, omx_buffer);
    }
    else {
      PRINT_DEBUG("--> atc_omx_port_release_buffer");
      atc_omx_port_release_buffer (port, omx_buffer);
    }
  }
}

void
atc_omx_port_push_buffer (AtcOmxPort * port,
  OMX_BUFFERHEADERTYPE * omx_buffer)
{
  async_queue_push (port->queue, omx_buffer);
}

OMX_BUFFERHEADERTYPE *
atc_omx_port_request_buffer (AtcOmxPort * port)
{
  return (OMX_BUFFERHEADERTYPE *)async_queue_pop (port->queue);
}

OMX_BUFFERHEADERTYPE *
atc_omx_port_request_buffer_nb(AtcOmxPort * port)
{
  if (async_queue_length(port->queue) > 0)
    return (OMX_BUFFERHEADERTYPE *)async_queue_pop (port->queue);
  else
    return NULL;
}

OMX_BUFFERHEADERTYPE *
atc_omx_port_find_buffer(AtcOmxPort * port, __u8 *pbuf)
{
  __u32 i;
  OMX_BUFFERHEADERTYPE *omx_buffer = NULL;

  for (i = 0; i < port->num_buffers; i++) {
    omx_buffer = port->buffers[i];
    if ((omx_buffer != NULL) && (omx_buffer->pBuffer == pbuf)) {
      PRINT_DEBUG("find omx buffer(%p), omx_buffer: %p\r\n",
                  omx_buffer->pBuffer, omx_buffer);
      return omx_buffer;
    }
  }

  return NULL;
}

void
atc_omx_port_release_buffer (AtcOmxPort * port,
  OMX_BUFFERHEADERTYPE * omx_buffer)
{
  switch (port->type) {
    case GOMX_PORT_INPUT:
      OMX_EmptyThisBuffer (port->core->omx_handle, omx_buffer);
      break;
    case GOMX_PORT_OUTPUT:
      OMX_FillThisBuffer (port->core->omx_handle, omx_buffer);
      break;
    default:
      break;
  }
}

void
atc_omx_port_resume (AtcOmxPort * port)
{
  async_queue_enable (port->queue);
}

void
atc_omx_port_pause (AtcOmxPort * port)
{
  async_queue_disable (port->queue);
}

void
atc_omx_port_flush (AtcOmxPort * port)
{
  {
    OMX_SendCommand (port->core->omx_handle, OMX_CommandFlush, port->port_index,
        NULL);
    atc_sem_down (port->core->flush_sem);
  }
}

void
atc_omx_port_enable (AtcOmxPort * port)
{
  AtcOmxCore *core;

  core = port->core;

  OMX_SendCommand (core->omx_handle, OMX_CommandPortEnable, port->port_index,
      NULL);
  port_allocate_buffers (port);
  if (core->omx_state != OMX_StateLoaded) {
    PRINT_INFO("--> core->object(%p) (omx_state = %d)--> port_start_buffers\r\n",
      core->object, core->omx_state);
    port_start_buffers (port);
  }
  atc_omx_port_resume (port);

  atc_sem_down (core->port_sem);
}

void
atc_omx_port_disable (AtcOmxPort * port)
{
  AtcOmxCore *core;

  core = port->core;

  OMX_SendCommand (core->omx_handle, OMX_CommandPortDisable, port->port_index,
      NULL);
  atc_omx_port_pause (port);
  atc_omx_port_flush (port);
  port_free_buffers (port);

  atc_sem_down (core->port_sem);
}

void
atc_omx_port_finish (AtcOmxPort * port)
{
  port->enabled = false;
  async_queue_disable (port->queue);
}

OMX_ERRORTYPE
atc_omx_port_get_port_definition (AtcOmxPort * port,
    OMX_PARAM_PORTDEFINITIONTYPE * port_def)
{
  AtcOmxCore *core = NULL;
  OMX_ERRORTYPE err;

  if (port == NULL)
    return OMX_ErrorBadParameter;

  core = port->core;

  ATC_OMX_INIT_PARAM (port_def);

  port_def->nPortIndex = port->port_index;

  err = atc_omx_cmp_get_parameter (core,
    OMX_IndexParamPortDefinition,
      port_def);

  return err;
}

OMX_ERRORTYPE
atc_omx_port_update_port_definition (AtcOmxPort * port,
    OMX_PARAM_PORTDEFINITIONTYPE * port_def)
{
  OMX_ERRORTYPE err = OMX_ErrorNone;
  AtcOmxCore *core = NULL;
  OMX_PARAM_PORTDEFINITIONTYPE cur_port_def;

  if (port == NULL)
    return OMX_ErrorBadParameter;

  core = port->core;

  if (port_def) {
    port_def->nPortIndex = port->port_index;
    err = atc_omx_cmp_set_parameter (core, OMX_IndexParamPortDefinition,
        port_def);
    if (err != OMX_ErrorNone) {
      PRINT_DEBUG("set omx component port %u definition fail, err: %s (0x%08x)\r\n",
        port->port_index, omx_error_to_str (core->omx_error), err);
      return err;
    }
  }
  ATC_OMX_INIT_PARAM (&cur_port_def);
  cur_port_def.nPortIndex = port->port_index;
  err = atc_omx_cmp_get_parameter (core, OMX_IndexParamPortDefinition,
      &cur_port_def);
  if (err != OMX_ErrorNone) {
    PRINT_DEBUG("get omx component port %u definition fail, err: %s (0x%08x)\r\n",
      port->port_index, omx_error_to_str (core->omx_error), err);
  }

  return err;
}

OMX_ERRORTYPE
atc_omx_cmp_get_parameter (AtcOmxCore *core,
   OMX_INDEXTYPE index, void *param)
{
  OMX_ERRORTYPE err;

  if ((core == NULL) || (param == NULL) || (core->omx_handle == NULL))
    return OMX_ErrorUndefined;

  PRINT_DEBUG("Getting omx component parameter at index 0x%08x\r\n", index);
  err = OMX_GetParameter (core->omx_handle, index, param);
  PRINT_DEBUG("Got omx component parameter at index 0x%08x: %s (0x%08x)\r\n",
    core->omx_error, omx_error_to_str (core->omx_error), err);

  return err;
}

OMX_ERRORTYPE
atc_omx_cmp_set_parameter (AtcOmxCore * core, OMX_INDEXTYPE index,
    void *param)
{
  OMX_ERRORTYPE err;

  if ((core == NULL) || (param == NULL) || (core->omx_handle == NULL))
    return OMX_ErrorUndefined;

  PRINT_DEBUG("Setting omx component parameter at index 0x%08x\r\n", index);
  err = OMX_SetParameter (core->omx_handle, index, param);
  PRINT_DEBUG("Set omx component parameter at index 0x%08x: %s  (0x%08x)\r\n",
    index, omx_error_to_str (core->omx_error), err);

  return err;
}

/* comp->lock must be unlocked while calling this */
OMX_ERRORTYPE
atc_omx_cmp_set_config (AtcOmxCore * core, OMX_INDEXTYPE index,
    void * config)
{
  OMX_ERRORTYPE err;

  if ((core == NULL) || (config == NULL))
    return OMX_ErrorBadParameter;

  PRINT_DEBUG("Setting %s configuration at index 0x%08x\r\n",
    core->component_name, index);
  err = OMX_SetConfig (core->omx_handle, index, config);
  if (err != OMX_ErrorNone) {
    PRINT_ERROR("Set %s configuration at index 0x%08x, err:%s\r\n",
      core->component_name,
      index, omx_error_to_str(err));
  }

  return err;
}

OMX_ERRORTYPE
atc_omx_cmp_get_ext_idx(AtcOmxCore * core, OMX_STRING paramName,
    OMX_INDEXTYPE* index)
{
  OMX_ERRORTYPE err = OMX_ErrorNone;

  if ((core == NULL) || (index == NULL))
    return OMX_ErrorBadParameter;

  err = OMX_GetExtensionIndex(core->omx_handle, paramName, index);

  return err;
}

/*
 * Helper functions.
 */

static inline void
change_state (AtcOmxCore * core, OMX_STATETYPE state)
{
  PRINT_DEBUG("state=%d\r\n", state);
  OMX_SendCommand (core->omx_handle, OMX_CommandStateSet, state, NULL);
}

static inline void
complete_change_state (AtcOmxCore * core, OMX_STATETYPE state)
{
  pthread_mutex_lock(&core->omx_state_mutex);

  core->omx_state = state;
  pthread_cond_signal(&core->omx_state_condition);
  PRINT_DEBUG("state=%d\r\n", state);

  pthread_mutex_unlock(&core->omx_state_mutex);
}

static inline void
wait_for_state (AtcOmxCore * core, OMX_STATETYPE state)
{
  struct timeval now_timer;
  struct timespec wait_timer;
  bool signaled;

  pthread_mutex_lock(&core->omx_state_mutex);

  if (core->omx_error != OMX_ErrorNone) {
    PRINT_ERROR("fail for (state=%d, expected=%d) omx error(%d), name: %s\r\n",
      core->omx_state, state, core->omx_error, core->component_name);
    goto leave;
  }
  gettimeofday(&now_timer, NULL);
  wait_timer.tv_sec = now_timer.tv_sec + 3;
  wait_timer.tv_nsec = now_timer.tv_usec * 1000;

  /* try once */
  if (core->omx_state != state) {
    signaled =
        pthread_cond_timedwait(&core->omx_state_condition, &core->omx_state_mutex,
        &wait_timer);

    if (signaled) {
      PRINT_ERROR("fail for timed out switching from '%s' to '%s'\r\n",
          omx_state_to_str (core->omx_state), omx_state_to_str (state));
    }
  }

  if (core->omx_error != OMX_ErrorNone) {
    PRINT_ERROR("fail for (state=%d, expected=%d) omx error(%d), name: %s\r\n",
      core->omx_state, state, core->omx_error, core->component_name);
    goto leave;
  }

  if (core->omx_state != state) {
    PRINT_ERROR("fail for wrong state received: state=%d, expected=%d, component_name: %s\r\n",
      core->omx_state, state, core->component_name);
  }

leave:
  pthread_mutex_unlock(&core->omx_state_mutex);
}

/*
 * Callbacks
 */

static inline void
got_buffer (AtcOmxCore * core, AtcOmxPort * port, OMX_BUFFERHEADERTYPE * omx_buffer)
{
  if (!omx_buffer) {
    return;
  }

  if (port) {
    PRINT_DEBUG(" push_buffer port[%d]", port->type);
    if (omx_buffer->nFlags & OMX_BUFFERFLAG_EOS) {
      PRINT_INFO("--> reach eos, omx_buffer: %p\r\n", omx_buffer);
    }
    atc_omx_port_push_buffer (port, omx_buffer);
  }
}

/*
 * OpenMAX IL atcomxcallbacks.
 */

static OMX_ERRORTYPE
EventHandler (OMX_HANDLETYPE omx_handle,
    OMX_PTR app_data,
    OMX_EVENTTYPE event, OMX_U32 data_1, OMX_U32 data_2, OMX_PTR event_data)
{
  AtcOmxCore *core;

  core = (AtcOmxCore *) app_data;

  switch (event) {
    case OMX_EventCmdComplete:
    {
      OMX_COMMANDTYPE cmd;

      cmd = (OMX_COMMANDTYPE) data_1;

      PRINT_DEBUG("OMX_EventCmdComplete: %d\r\n", cmd);

      switch (cmd) {
        case OMX_CommandStateSet:
          complete_change_state (core, (OMX_STATETYPE)data_2);
          break;
        case OMX_CommandFlush:
          atc_sem_up (core->flush_sem);
          break;
        case OMX_CommandPortDisable:
        case OMX_CommandPortEnable:
          atc_sem_up (core->port_sem);
        default:
          break;
      }
      break;
    }
    case OMX_EventBufferFlag:
    {
      PRINT_DEBUG("OMX_EventBufferFlag");
      if (data_2 & OMX_BUFFERFLAG_EOS) {
        atc_omx_core_set_done (core);
      }
      break;
    }
    case OMX_EventPortSettingsChanged:
    {
      PRINT_DEBUG("OMX_EventPortSettingsChanged");
                /** @todo only on the relevant port. */
      if (core->settings_changed_cb) {
        core->settings_changed_cb (core);
      }
      break;
    }
    case OMX_EventError:
    {
      core->omx_error = (OMX_ERRORTYPE)data_1;
      PRINT_ERROR("unrecoverable error: %s (0x%lx)\r\n",
          omx_error_to_str ((OMX_ERRORTYPE)data_1), data_1);
      /* component might leave us waiting for buffers, unblock */
      atc_omx_core_flush_start (core);
      /* unlock wait_for_state */
      pthread_mutex_lock(&core->omx_state_mutex);
      pthread_cond_signal(&core->omx_state_condition);
      pthread_mutex_unlock(&core->omx_state_mutex);
      break;
    }
    default:
      break;
  }

  return OMX_ErrorNone;
}

static OMX_ERRORTYPE
EmptyBufferDone (OMX_HANDLETYPE omx_handle,
    OMX_PTR app_data, OMX_BUFFERHEADERTYPE * omx_buffer)
{
  AtcOmxCore *core;
  AtcOmxPort *port;

  core = (AtcOmxCore *) app_data;
  port = get_port (core, omx_buffer->nInputPortIndex);

  //PRINT_DEBUG("omx_buffer is 0x%x, got_bufferr\n", (unsigned)omx_buffer);
  if (core->callbacks != NULL)
    core->callbacks->EmptyBufferDone(NULL, core->app_data, omx_buffer);
  else
    got_buffer (core, port, omx_buffer);

  return OMX_ErrorNone;
}

static OMX_ERRORTYPE
FillBufferDone (OMX_HANDLETYPE omx_handle,
    OMX_PTR app_data, OMX_BUFFERHEADERTYPE * omx_buffer)
{
  AtcOmxCore *core;
  AtcOmxPort *port;

  core = (AtcOmxCore *) app_data;
  port = get_port (core, omx_buffer->nOutputPortIndex);

  //PRINT_DEBUG("omx_buffer=%p\r\n", omx_buffer);
  if (core->callbacks != NULL)
    core->callbacks->FillBufferDone(NULL, core->app_data, omx_buffer);
  else
    got_buffer (core, port, omx_buffer);

  return OMX_ErrorNone;
}

void *
atcomx_core_new (void *object, char *library_name, char *component_name, OMX_CALLBACKTYPE *callbacks, void *app_data)
{
  AtcOmxCore *core = NULL;

  if ((NULL == object) ||
    (library_name == object) ||
    (component_name == object)) {
    PRINT_ERROR("fail for invalid args\r\n");
    return NULL;
  }

  core = atc_omx_core_new (object);
  core->library_name = strdup (library_name);
  core->component_name = strdup (component_name);
  if (!atc_omx_core_init (core)) {
    PRINT_ERROR("fail in atc_omx_core_init\r\n");
    atc_omx_core_free(core);
    return NULL;
  }
  core->callbacks = callbacks;
  core->app_data = app_data;

  return core;
}

static inline const char *
omx_state_to_str (OMX_STATETYPE omx_state)
{
  switch (omx_state) {
    case OMX_StateInvalid:
      return "invalid";
    case OMX_StateLoaded:
      return "loaded";
    case OMX_StateIdle:
      return "idle";
    case OMX_StateExecuting:
      return "executing";
    case OMX_StatePause:
      return "pause";
    case OMX_StateWaitForResources:
      return "wait for resources";
    default:
      return "unknown";
  }
}

static inline const char *
omx_error_to_str (OMX_ERRORTYPE omx_error)
{
  switch (omx_error) {
    case OMX_ErrorNone:
      return "None";

    case OMX_ErrorInsufficientResources:
      return
          "There were insufficient resources to perform the requested operation";

    case OMX_ErrorUndefined:
      return "The cause of the error could not be determined";

    case OMX_ErrorInvalidComponentName:
      return "The component name string was not valid";

    case OMX_ErrorComponentNotFound:
      return "No component with the specified name string was found";

    case OMX_ErrorInvalidComponent:
      return "The component specified did not have an entry point";

    case OMX_ErrorBadParameter:
      return "One or more parameters were not valid";

    case OMX_ErrorNotImplemented:
      return "The requested function is not implemented";

    case OMX_ErrorUnderflow:
      return "The buffer was emptied before the next buffer was ready";

    case OMX_ErrorOverflow:
      return "The buffer was not available when it was needed";

    case OMX_ErrorHardware:
      return "The hardware failed to respond as expected";

    case OMX_ErrorInvalidState:
      return "The component is in invalid state";

    case OMX_ErrorStreamCorrupt:
      return "Stream is found to be corrupt";

    case OMX_ErrorPortsNotCompatible:
      return "Ports being connected are not compatible";

    case OMX_ErrorResourcesLost:
      return "Resources allocated to an idle component have been lost";

    case OMX_ErrorNoMore:
      return "No more indices can be enumerated";

    case OMX_ErrorVersionMismatch:
      return "The component detected a version mismatch";

    case OMX_ErrorNotReady:
      return "The component is not ready to return data at this time";

    case OMX_ErrorTimeout:
      return "There was a timeout that occurred";

    case OMX_ErrorSameState:
      return
          "This error occurs when trying to transition into the state you are already in";

    case OMX_ErrorResourcesPreempted:
      return
          "Resources allocated to an executing or paused component have been preempted";

    case OMX_ErrorPortUnresponsiveDuringAllocation:
      return
          "Waited an unusually long time for the supplier to allocate buffers";

    case OMX_ErrorPortUnresponsiveDuringDeallocation:
      return
          "Waited an unusually long time for the supplier to de-allocate buffers";

    case OMX_ErrorPortUnresponsiveDuringStop:
      return
          "Waited an unusually long time for the non-supplier to return a buffer during stop";

    case OMX_ErrorIncorrectStateTransition:
      return "Attempting a state transition that is not allowed";

    case OMX_ErrorIncorrectStateOperation:
      return
          "Attempting a command that is not allowed during the present state";

    case OMX_ErrorUnsupportedSetting:
      return
          "The values encapsulated in the parameter or config structure are not supported";

    case OMX_ErrorUnsupportedIndex:
      return
          "The parameter or config indicated by the given index is not supported";

    case OMX_ErrorBadPortIndex:
      return "The port index supplied is incorrect";

    case OMX_ErrorPortUnpopulated:
      return
          "The port has lost one or more of its buffers and it thus unpopulated";

    case OMX_ErrorComponentSuspended:
      return "Component suspended due to temporary loss of resources";

    case OMX_ErrorDynamicResourcesUnavailable:
      return
          "Component suspended due to an inability to acquire dynamic resources";

    case OMX_ErrorMbErrorsInFrame:
      return "Frame generated macroblock error";

    case OMX_ErrorFormatNotDetected:
      return "Cannot parse or determine the format of an input stream";

    case OMX_ErrorContentPipeOpenFailed:
      return "The content open operation failed";

    case OMX_ErrorContentPipeCreationFailed:
      return "The content creation operation failed";

    case OMX_ErrorSeperateTablesUsed:
      return "Separate table information is being used";

    case OMX_ErrorTunnelingUnsupported:
      return "Tunneling is unsupported by the component";

    default:
      return "Unknown error";
  }
}
