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


/*!
 * @file dmx_dump.h
 *
 * @par Project
 *
 *
 * @par Description
 *    Demuxer error codes definitions
 *
 * @par Author_Name
 *    Shuhui Zhang
 *
 */

#ifndef DMX_INTERNAL_ERRCODE_H
#define DMX_INTERNAL_ERRCODE_H

#ifdef __cplusplus
extern "C" {

#endif

#include "x_os.h"
#ifdef __linux__
#include <linux/mm.h>
#include <linux/errno.h>
#include <media/atc/dmx_define.h>
#include <media/atc/mm_errcode.h>
/* #include <media/atc/mm_debug.h> */
#else
#include "dmx_define.h"
#include "mm_errcode.h"
#include "mm_debug.h"
#endif	/* __linux__ */

/* ////////////////////////////////////////////////////////////////////////////// */
/* /             Error Code for demuxer module and its submodule.             /// */
/* ////////////////////////////////////////////////////////////////////////////// */

/* Success */
#define RET_DMX_OK                           MAKE_DMX_INT_STATE_ERRCODE(0x00000000)

/******************************************************************************/
/* State Error Code                                                           */
/******************************************************************************/

/* Fifo Full */
#define RET_DMX_FIFO_FULL                    MAKE_DMX_INT_STATE_ERRCODE(0x00000001)
/* Pbbuf Slot Busy, i.e. No slots for required */
#define RET_DMX_PBBUF_BUSY                   MAKE_DMX_INT_STATE_ERRCODE(0x00000002)
/* Unsupport to do the task, maybe that, unsupport codec or other reasons */
#define RET_DMX_UNSUPPORT                    MAKE_DMX_INT_STATE_ERRCODE(0x00000003)
/* Hasn't read the threshold, this is only used for polling get au */
#define RET_DMX_NO_REACH_THRESHOLD           MAKE_DMX_INT_STATE_ERRCODE(0x00000004)
/* Has reason the threshold, this is only used for polling get au */
#define RET_DMX_REACH_EOS                    MAKE_DMX_INT_STATE_ERRCODE(0x00000005)
/* Get not get the au for some reason */
#define RET_DMX_NO_AU                        MAKE_DMX_INT_STATE_ERRCODE(0x00000006)
/* Fifo full but no corresponding au, i.e. the data in the fifo is */
#define RET_DMX_ERR_DATA                     MAKE_DMX_INT_STATE_ERRCODE(0x00000007)
/* Can't find the rsp log au which satisfys the requirement */
#define RET_DMX_NO_RSP_LOGAU                 MAKE_DMX_INT_STATE_ERRCODE(0x00000008)
/* Can't find the rsp log au which satisfys the requirement */
#define RET_DMX_NEED_JUMP                    MAKE_DMX_INT_STATE_ERRCODE(0x00000008)
/* Need to wait hw release when to do suspend */
#define RET_DMX_SUSPEND_NEED_WAIT            MAKE_DMX_INT_STATE_ERRCODE(0x00000009)
/* Need to inform suspend OK */
#define RET_DMX_SUSPEND_OK                   MAKE_DMX_INT_STATE_ERRCODE(0x0000000A)
/* wait timeout */
#define RET_DMX_TIMEOUT		                 MAKE_DMX_INT_STATE_ERRCODE(0x0000000B)

/******************************************************************************/
/* Real Error Code                                                            */
/******************************************************************************/

/* Param Wrong */
#define RET_DMX_PARAM_WRONG                  MAKE_DMX_INT_REAL_ERRCODE(0x00000001)
/* No Memory */
#define RET_DMX_NO_MEM                       MAKE_DMX_INT_REAL_ERRCODE(0x00000002)
/* Overflow, or no free element */
#define RET_DMX_OVER_LIMIT                   MAKE_DMX_INT_REAL_ERRCODE(0x00000003)
/* Operation Forbidden */
#define RET_DMX_OPERATE_FORBID               MAKE_DMX_INT_REAL_ERRCODE(0x00000004)
/* SW start another DMA while HW is in DMA */
#define RET_DMX_PTX_BUSY                     MAKE_DMX_INT_REAL_ERRCODE(0x00000005)
/* Encounter the exception which we think the code can't run the process */
#define RET_DMX_UNEXPECT                     MAKE_DMX_INT_REAL_ERRCODE(0x00000006)
/* Os operations fail, such as create event, semaphore, file or other system resource */
#define RET_DMX_OS_OPERA_FAIL                MAKE_DMX_INT_REAL_ERRCODE(0x00000007)
/* Element, or instance han't been initailized */
#define RET_DMX_NO_INIT                      MAKE_DMX_INT_REAL_ERRCODE(0x00000008)
/* No Psr CC */
#define RET_DMX_NO_CC                        MAKE_DMX_INT_REAL_ERRCODE(0x00000009)
/* No AU Table(For ESM) */
#define RET_DMX_NO_AUTABLE                   MAKE_DMX_INT_REAL_ERRCODE(0x0000000A)
/* Stream type, or es type hasn't been set */
#define RET_DMX_NO_SET_TYPE                  MAKE_DMX_INT_REAL_ERRCODE(0x0000000B)
/* Error Status */
#define RET_DMX_ERR_STATE                    MAKE_DMX_INT_REAL_ERRCODE(0x0000000C)
/* Element can't be found */
#define RET_DMX_NOT_FOUND                    MAKE_DMX_INT_REAL_ERRCODE(0x0000000E)
/* Element Already Exists */
#define RET_DMX_ALREADY_EXIST                MAKE_DMX_INT_REAL_ERRCODE(0x0000000F)
/* Demuxer HW encounter error */
#define RET_DMX_HW_ERROR                     MAKE_DMX_INT_REAL_ERRCODE(0x00000010)
/* No designated Cfa interface */
#define RET_DMX_NO_CFA_INTERFACE             MAKE_DMX_INT_REAL_ERRCODE(0x00000011)
/* No designated Cfa Private data */
#define RET_DMX_NO_CFA_PRIV_DATA             MAKE_DMX_INT_REAL_ERRCODE(0x00000012)
/* Function hasn't been implemented */
#define RET_DMX_NO_IMPLEMENT                 MAKE_DMX_INT_REAL_ERRCODE(0x00000013)
/* HW give error Pic Type */
#define RET_DMX_ERR_PIC_TYPE                 MAKE_DMX_INT_REAL_ERRCODE(0x00000014)

/* /> DECRYPT Related errorcode */

/******************************************************************************/
/* Real Error Code                                                            */
/******************************************************************************/
#define RET_DMX_CPSA_GEN_ERR        MAKE_DMX_INT_REAL_ERRCODE(0x00000032)

/******************************************************************************/
/* State Error Code                                                           */
/******************************************************************************/
#define RET_DMX_CPSA_NEVER_REGED     MAKE_DMX_INT_STATE_ERRCODE(0x00000032)

#define RET_DMX_CPSA_NEVER_AUTHED    MAKE_DMX_INT_STATE_ERRCODE(0x00000033)

#define RET_DMX_CPSA_NOT_REGED       MAKE_DMX_INT_STATE_ERRCODE(0x00000034)

#define RET_DMX_CPSA_RENTAL_EXPIRED  MAKE_DMX_INT_STATE_ERRCODE(0x00000035)

#define DMX_FAILED(Status)           (RET_DMX_OK != (Status))
#define DMX_SUCCEED(Status)          (RET_DMX_OK == (Status))

#define DMX_ERR_IS_NO_RES(err)		((RET_DMX_NO_MEM == (err)) ||        \
					(RET_DMX_OS_OPERA_FAIL == (err)))


#define E_DMX_OK                    RET_DMX_OK
#define E_DMX_ERROR                 RET_DMX_UNEXPECT
#define E_DMX_ABORT_RANGE           RET_DMX_OVER_LIMIT
#define E_DMX_NO_MEM                RET_DMX_NO_MEM
#define E_DMX_NO_MEMORY             RET_DMX_NO_MEM
#define E_DMX_UNSUPPORT             RET_DMX_UNSUPPORT

#define DMX_RETURN_LOG_OPEN          1

#if DMX_RETURN_LOG_OPEN
#define MM_RETURN(err)      MM_RETURN_I(err, __FILE__, __LINE__)
#define MM_RETURN_I(err, file, line)                                                            \
    do                                                                                          \
    {                                                                                           \
        MRESULT u4Err = (MRESULT)err;     \
        if (u4Err != (MRESULT)RET_DMX_OK) \
        {                                                                                       \
            if (MM_IS_STATE_ERROR(u4Err))                                                       \
            {                                                                                   \
                pr_debug("[STATE_ERROR][0x%x] is returned at line %d in file %s\r\n", (unsigned int)u4Err, line, file);   \
            }                                                                                   \
            else \
            {                                                                                   \
                pr_err("[REAL_ERROR][0x%x] is returned at line %d in file %s\r\n",    \
                                (unsigned int)u4Err, line, file);                            \
            }                                                                                   \
        }                                                                                       \
        return (MRESULT)u4Err;                                                                         \
    } while(0)
#else
#define MM_RETURN(err)    return err
#endif


#ifdef __cplusplus
}


#endif

#endif	/* #ifndef DMX_INTERNAL_ERRCODE_H */
