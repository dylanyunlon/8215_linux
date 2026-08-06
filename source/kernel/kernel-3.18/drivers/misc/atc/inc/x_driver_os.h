/*
* Copyright (c) 2016 AutoChips Inc.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

#ifndef _X_DRIVER_OS_H_
#define _X_DRIVER_OS_H_


/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/

#include "x_common.h"
#include "x_os.h"


/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/

/* OS driver return values */

#define OSR_DRV_THREAD_ACTIVE  ((INT32)   2)
#define OSR_DRV_WOULD_BLOCK    ((INT32)   1)
#define OSR_DRV_OK             ((INT32)   0)
#define OSR_DRV_EXIST          ((INT32)  -1)
#define OSR_DRV_INV_ARG        ((INT32)  -2)
#define OSR_DRV_TIMEOUT        ((INT32)  -3)
#define OSR_DRV_NO_RESOURCE    ((INT32)  -4)
#define OSR_DRV_NOT_EXIT       ((INT32)  -5)
#define OSR_DRV_NOT_FOUND      ((INT32)  -6)
#define OSR_DRV_INVALID        ((INT32)  -7)
#define OSR_DRV_NOT_INIT       ((INT32)  -8)
#define OSR_DRV_DELETED        ((INT32)  -9)
#define OSR_DRV_TOO_MANY       ((INT32) -10)
#define OSR_DRV_FAIL           ((INT32) -11)


typedef VOID (*x_os_drv_thread_main_fct) (const CHAR* ps_name);

typedef VOID (*x_os_drv_isr_fct) (UINT16 ui2_vector_id);

typedef VOID* (*x_os_drv_memcpy) (VOID*        pv_to,
                                  const VOID*  pv_from,
                                  SIZE_T       z_len);

typedef VOID* (*x_os_drv_memmove) (VOID*        pv_to,
                                   const VOID*  pv_from,
                                   SIZE_T       z_len);

typedef VOID* (*x_os_drv_memset) (VOID*   pv_mem,
                                  UINT8   ui1_c,
                                  SIZE_T  z_len);

typedef struct
{
    x_os_drv_memcpy   pf_drv_memcpy;
    x_os_drv_memmove  pf_drv_memmove;
    x_os_drv_memset   pf_drv_memset;
}   DRV_MEM_FUNCTIONS_T;


/*-----------------------------------------------------------------------------
                    functions declarations
 ----------------------------------------------------------------------------*/

/* Thread API's */
extern INT32 x_os_drv_thread_create (VOID**                    ppv_th_id,
                                     const CHAR*               ps_name,
                                     SIZE_T                    z_stacksize,
                                     UINT8                     ui1_pri,
                                     UINT16                    ui2_flags,
                                     VOID*                     pv_tag,
                                     x_os_drv_thread_main_fct  pf_main);

extern VOID x_os_drv_thread_exit (VOID);

extern VOID x_os_drv_thread_delay (UINT32  ui4_delay);

extern INT32 x_os_drv_thread_get_pri (VOID*   pv_th_id,
                                      UINT8*  pui1_pri);

extern INT32 x_os_drv_thread_set_pri (VOID*  pv_th_id,
                                      UINT8  ui1_pri);

extern VOID x_os_drv_thread_suspend (VOID);

extern INT32 x_os_drv_thread_resume (VOID*  pv_th_id);

extern INT32 x_os_drv_thread_self (VOID**  pv_th_id,
                                   VOID**  ppv_tag);

extern INT32 x_os_drv_thread_stack_stats (VOID*    pv_th_id,
                                          SIZE_T*  pz_alloc_stack,
                                          SIZE_T*  pz_max_used_stack);


/* Semaphore API's */
extern INT32 x_os_drv_sema_create (VOID**  ppv_sema_id,
                                   UINT32  ui4_init_value);

extern INT32 x_os_drv_sema_delete (VOID*  pv_sema_id);

extern INT32 x_os_drv_sema_lock (VOID*          pv_sema_id,
                                 SEMA_OPTION_T  e_option);

extern INT32 x_os_drv_sema_lock_timeout (VOID*   pv_sema_id,
                                         UINT32  ui4_time);

extern INT32 x_os_drv_sema_unlock (VOID*  pv_sema_id);


/* Critical section API's */
extern CRIT_STATE_T x_os_drv_crit_start (VOID);

extern VOID x_os_drv_crit_end (CRIT_STATE_T  t_old_level);


/* Timer / tick API's */
extern UINT32 x_os_drv_get_tick_period (VOID);

extern UINT32 x_os_drv_get_fine_tick (VOID);

extern VOID x_os_drv_tick (VOID);


/* ISR API's */
extern INT32 x_os_drv_reg_isr(UINT16             ui2_vector_id,
                              x_os_drv_isr_fct   pf_isr,
                              x_os_drv_isr_fct*  ppf_old_isr);

/* To regiser driver-specific memory functions */
extern INT32
x_os_drv_reg_mem_functions (const DRV_MEM_FUNCTIONS_T*  pt_functions);

/* Get the time (sec and micro-sec) since the system boot up. */
extern INT32 x_os_drv_get_sys_uptime (UINT32* pui4_sec,
                                       UINT32* pui4_micro_sec);

#endif /* _X_DRIVER_OS_H_ */

