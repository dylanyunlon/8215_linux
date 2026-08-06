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
 * @file dmx_def.h
 *
 * @par Project
 *
 * @par Description
 *    Demuxer main macros definitions
 *
 * @par Author_Name
 *    Shuhui Zhang
 *
 */

#ifndef DMX_INTERNAL_DEFINE_H
#define DMX_INTERNAL_DEFINE_H


#ifdef __cplusplus
extern "C" {
#endif

#include "x_typedef.h"
#include "chip_ver.h"
#include "drv_config.h"
#include "drv_common.h"
#ifdef __linux__
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include "types.h"
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/init.h>
#include "windows.h"
#include "winutil.h"
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_splitter.h>
#include <media/atc/drv_esm_if.h>
#include <media/atc/x_dmx.h>
#include <media/atc/memchk_cfg.h>
#include <media/atc/memdbg_c.h>
#include <media/atc/mm_errcode.h>
#include <media/atc/mm_common.h>
/* #include <media/atc/mm_debug.h> */
#else
#include "dmx_define.h"
#include "dmx_splitter.h"
#include "drv_esm_if.h"
#include "x_dmx.h"
#include "memchk_cfg.h"
#include "memdbg_c.h"
#include "mm_errcode.h"
#include "mm_common.h"
#include "mm_debug.h"
#endif				/* __linux__ */

#include "dmx_errcode.h"
#include "dmx_log.h"
#include <linux/types.h>

#define DMX_MOD_NAME        TEXT("DMX")	/* /> Moudle name */
/* 03: DMX Optimization */
#define DMX_VER_MAIN        00		/* /> main version */
#define DMX_VER_MINOR       00		/* /> minor version, Large feature tens add 1 and clear unit, */
					/* /> other feature and add new function need add 1. */
#define DMX_VER_REV         01		/* /> version number, you should add 1 when check in aud */

/* #ifdef LOG_LEVEL_DEFALUT_MASK */
/* #undef LOG_LEVEL_DEFALUT_MASK */
/* #define LOG_LEVEL_DEFALUT_MASK 0xF //only print Error andr fatal */
/* #endif */

#ifdef __linux__
/* #define DMX_SEMA_USING_SPINLOCK */
#endif

#define DMX_SUPPORT_DEVICE_TREE     1

/*support 32 bit user space, 64 bit kernel*/
//#define CONFIG_COMPAT


/* ////////////////////////////////////////////////////////////////////////////// */
/*                 Control Open/Close Sub Module                               // */
/* ////////////////////////////////////////////////////////////////////////////// */

#define DMX_CHECK_MEM_VALIBILITY    1
#define DMX_DISABLE_CFA             0
#define DMX_DISABLE_DMA_DATA        0
#define DMX_DISABLE_GET_REALAU      0
#define DMX_DISABLE_FILL_AUINFO     0

#define DMX_DISABLE_COMP_AU      	0

#if DMX_DISABLE_COMP_AU
#define DMX_DISABLE_COMP_VIDEOAU  0
#define DMX_DISABLE_COMP_AUDIOAU  1
#define DMX_DISABLE_COMP_SPAU			0
#define DMX_DISABLE_COMP_OTHAU  	0
#else
#define DMX_DISABLE_COMP_VIDEOAU  0
#define DMX_DISABLE_COMP_AUDIOAU  0
#define DMX_DISABLE_COMP_SPAU   	0
#define DMX_DISABLE_COMP_OTHAU  	0
#endif	/* DMX_DISABLE_COMP_AU */

#if DMX_DISABLE_COMP_VIDEOAU
#define DMX_DISABLE_COMP_MPEG2AU   1
#define DMX_DISABLE_COMP_VC1AU     1
#define DMX_DISABLE_COMP_AVCAU     1
#define DMX_DISABLE_COMP_MPEG4AU   1
#define DMX_DISABLE_COMP_NONHDRAU  1
#else
#define DMX_DISABLE_COMP_MPEG2AU   0
#define DMX_DISABLE_COMP_VC1AU     0
#define DMX_DISABLE_COMP_AVCAU     0
#define DMX_DISABLE_COMP_MPEG4AU   0
#define DMX_DISABLE_COMP_NONHDRAU  0
#endif	/* DMX_DISABLE_COMP_VIDEOAU */


#define DMX_DISABLE_VID_STM         0
#define DMX_DISABLE_VID_DMA         0
#define DMX_DISABLE_AUD_STM         0
#define DMX_DISABLE_AUD_DMA         0

#if DMX_DISABLE_DMA_DATA
#ifdef DMX_DISABLE_VID_DMA
#undef DMX_DISABLE_VID_DMA
#define DMX_DISABLE_VID_DMA  1
#endif
#ifdef DMX_DISABLE_AUD_DMA
#undef DMX_DISABLE_AUD_DMA
#define DMX_DISABLE_AUD_DMA  1
#endif
#ifdef DMX_DISABLE_COMP_AU
#undef DMX_DISABLE_COMP_AU
#define DMX_DISABLE_COMP_AU  1
#endif
#endif	/* DMX_DISABLE_DMA_DATA */

#if DMX_DISABLE_VID_STM
#ifdef DMX_DISABLE_VID_DMA
#undef DMX_DISABLE_VID_DMA
#define DMX_DISABLE_VID_DMA  1
#endif
#endif	/* DMX_DISABLE_VID_STM */

#if (DMX_DISABLE_VID_STM || DMX_DISABLE_VID_DMA)
#ifdef DMX_DISABLE_COMP_VIDEOAU
#undef DMX_DISABLE_COMP_VIDEOAU
#define DMX_DISABLE_COMP_VIDEOAU  1
#endif
#ifdef DMX_DISABLE_COMP_AU
#undef DMX_DISABLE_COMP_AU
#define DMX_DISABLE_COMP_AU  1
#endif
#endif	/* (DMX_DISABLE_VID_STM || DMX_DISABLE_VID_DMA) */

#if DMX_DISABLE_AUD_STM
#ifdef DMX_DISABLE_AUD_DMA
#undef DMX_DISABLE_AUD_DMA
#define DMX_DISABLE_AUD_DMA  1
#endif
#endif
#if (DMX_DISABLE_AUD_STM || DMX_DISABLE_AUD_DMA)
#ifdef DMX_DISABLE_COMP_AUDIOAU
#undef DMX_DISABLE_COMP_AUDIOAU
#define DMX_DISABLE_COMP_AUDIOAU  1
#endif
#ifdef DMX_DISABLE_COMP_AU
#undef DMX_DISABLE_COMP_AU
#define DMX_DISABLE_COMP_AU  1
#endif
#endif	/* (DMX_DISABLE_AUD_STM || DMX_DISABLE_AUD_DMA) */

#define ENABLE_DMX_ADVANCED_VER           1
#define DMX_MAX_INST_BYTES_CNT            7

#define DMX_IFRM_CNT_IN_CTRL_RW           2
#define MAX_VID_AU_CNT_IN_CTRL_RW         4

/* ////////////////////////////////////////////////////////////////////////////// */
/*        Splitter, Parser CC&Filter, PBBUF, GAU Instances Count Definition       */
/* ////////////////////////////////////////////////////////////////////////////// */

#define MAX_DMX_INSTANCE_CNT              2

#define DMX_DEV_CNT                       2


#define MAX_SPT_INST_CNT_PER_DMX    3
#define DMX_MAX_SPT_INST_CNT           (u32)(MAX_SPT_INST_CNT_PER_DMX * MAX_DMX_INSTANCE_CNT)

#define DMX_MAX_PBBUF_INST_CNT   (DMX_MAX_SPT_INST_CNT)
/* /< maximum control center count */
#define MAX_PSR_CC_CNT           (DMX_MAX_SPT_INST_CNT)

/* /< maximum video filter count per CC */
#define MAX_VF_IN_CC                        (1)
/* /< maximum audio filter count per CC */
#define MAX_AF_IN_CC                        (1)
/* /< maximum sub picture filter count per CC */
#define MAX_SPF_IN_CC                       (1)
/* /< maximum section filter count per CC */
#define MAX_SCTF_IN_CC                      (1)
/* /< maximum SPT_DATA_BUF filter count per CC */
#define MAX_DMAF_IN_CC                      (1)
/* /< maximum ground filter count per CC */
#define MAX_GF_IN_CC                        (1)


/* Max Parser Filter Count Per Parser CC */
#define MAX_PSR_FILTER_PER_CC			(MAX_VF_IN_CC + MAX_AF_IN_CC   + \
						MAX_SPF_IN_CC  + MAX_SCTF_IN_CC + \
						MAX_DMAF_IN_CC + MAX_GF_IN_CC)

#define MAX_SPT_STM_CONNECTED               (MAX_PSR_FILTER_PER_CC)

#define MAX_CACHE_PBBUF                     (2)	/* /< Maximum cached pbbuf count */

#define MAX_GAU_INSTANCE_CNT                (4 * MAX_DMX_INSTANCE_CNT)

#define DMX_GAU_GETAU_Q_ELEM_CNT        (5)

#define DMX_MAX_ORGSTM_CNT_PER_SPT      (SPT_DATA_SECTION)
#define DMX_MAX_DMASTM_CNT_PER_SPT      (1)
#define DMX_MAX_GRDSTM_CNT_PER_SPT      (1)

#define MAX_STM_INST_CNT_PER_DMX			   ((u32)((DMX_MAX_ORGSTM_CNT_PER_SPT + \
						DMX_MAX_DMASTM_CNT_PER_SPT + DMX_MAX_GRDSTM_CNT_PER_SPT) * \
						MAX_SPT_INST_CNT_PER_DMX))

#define DMX_MAX_ORG_STM_CNT     ((u32)(DMX_MAX_ORGSTM_CNT_PER_SPT * \
  MAX_SPT_INST_CNT_PER_DMX * MAX_DMX_INSTANCE_CNT))
#define DMX_MAX_DMA_STM_CNT     ((u32)(DMX_MAX_DMASTM_CNT_PER_SPT * \
    MAX_SPT_INST_CNT_PER_DMX * MAX_DMX_INSTANCE_CNT))
#define DMX_MAX_GRD_STM_CNT     ((u32)(DMX_MAX_GRDSTM_CNT_PER_SPT * \
      MAX_SPT_INST_CNT_PER_DMX * MAX_DMX_INSTANCE_CNT))
    
#define MAX_STREAM_INSTANCE_CNT	((u32)(DMX_MAX_ORG_STM_CNT + \
  DMX_MAX_DMA_STM_CNT + DMX_MAX_GRD_STM_CNT))

/* ////////////////////////////////////////////////////////////////////////////// */
/*                         Some Mechanism Open/Off Control                     // */
/* ////////////////////////////////////////////////////////////////////////////// */
/* Designate whether to support resplitter in fast forward / rewind, 1: Support, 0: Unsupport(default) */
#define DMX_SUPPORT_RSP_IN_FFRW             1

/* Whether support Pure Audio Resplitter, 1: Support, 0: Unsupport(default) */
#define DMX_RSP_SUPPORT_PURE_AUDIO          0

/* Designate whether to disable dma audio in fast forward / rwwind, 1: disable(default), 0: enable */
#define DMX_DISABLE_DMA_AUD_IN_FF           1

/* Designate whether to disable dma audio for the file whch has video streams in fast rwwind,*/
/*1: disable(default), 0: enable */
#define DMX_DISABLE_PUSE_AUD_IN_RW          0

/* 1: Designate whether to print decrypt flow log, 1: print, 0: unprint */
#define DMX_PRINT_DECRYPT_KEY_LOG           0

/* 1: Config Resplitter Memory for 128M, 0: Alloc Pbbuf using polling method */
#define DMX_CFG_RSPMEM_BY_MEMSIZE       		1

#if DMX_SUPPORT_DIVXDRM
/* 1: Designated whether to decryt DRM data ussing demuxer hw, 1: HW, 0: SW(Divx SDK) */
#define DMX_DRM_DECRYPT_USE_HW              1
/* 1: Designated whether Resplitter support DivxDRM Decrypt, 1: Support, 0: Unsupport */
#define DMX_SPT_RSP_USING_DIVXDRM           1
#else	/* DMX_SUPPORT_DIVXDRM */
/* 1: Designated whether to decryt DRM data ussing demuxer hw, 1: HW, 0: SW(Divx SDK) */
#define DMX_DRM_DECRYPT_USE_HW              0
/* 1: Designated whether Resplitter support DivxDRM Decrypt, 1: Support, 0: Unsupport */
#define DMX_SPT_RSP_USING_DIVXDRM           0
#endif	/* DMX_SUPPORT_DIVXDRM */

/* 1: Alloc Pbbuf using wait-infinite method, 0: Alloc Pbbuf using polling method */
#define DMX_NEW_PBBUF_MECHANISM             0

/* Performance Test */
#define DMX_PFM_TEST                        0

/* Minimum data size of AFIFO which will be regard as empty */
#define DMX_AFIFO_EMP_DATA_MINSZ        (10 * 1024)

#define DMX_DRV_AFIFO_MM_SIZE           (0x10400)

/* ////////////////////////////////////////////////////////////////////////////// */
/*      Some Limitation, such as reserved fifo space, alignment, and so on        */
/* ////////////////////////////////////////////////////////////////////////////// */
#define DMX_MAX_LOG_AUDIO_AU_CNT           (5)

#define DMX_STM_AU_CNT_THRESHOLD           (5)

#define PSR_RESERVE_FIFO_SPACE             (6144)	/* 6K */

#define PSR_FIFO_ALIGNMENT                 (256)

#define DMX_MAX_ERRCHUNK_CNT   		(10)

/* Demuxer PVR HW max search video startcodes count one time. */
#define DMX_MAX_VID_STARTCODE_CNT     (200)

/* 32K, Video PES Header Buffer Size (Demuxer HW using) */
#define DMX_PESHDR_WORKBUF_SIZE       (0x8000)


#define DMX_MAX_VIDEOAU_CNT           (500)
#define DMX_MAX_AUDIOAU_CNT           (5000)
#define DMX_MAX_SPAU_CNT              (700)
#define DMX_MAX_SECAU_CNT             (100)

#define DMX_MAX_VIDEOAU_CNT_128M      (500)
#define DMX_MAX_AUDIOAU_CNT_128M      (1000)
#define DMX_MAX_SPAU_CNT_128M         (500)
#define DMX_MAX_SECAU_CNT_128M    		(100)

#define DMX_VID_TX_MAX_SIZE    (200ULL * 1024ULL)	/* 200k */
#define DMX_CMDQ_TX_MAX_SIZE   (500ULL * 1024ULL)	/* 500k */

#define DMX_PTS_1S                         (90000)
#define DMX_VSYNC_PTS_DELTA   (3 * DMX_PTS_1S)
#define DMX_ASYNC_PTS_DELTA   (3 * DMX_PTS_1S)
#define DMX_SPSYNC_PTS_DELTA  (3 * DMX_PTS_1S)


/* ////////////////////////////////////////////////////////////////////////////// */
/*                     Fast Forward / Rewind related macro                     // */
/* ////////////////////////////////////////////////////////////////////////////// */
#define DMX_FFRW_FIFO_SKIPAU_SZ(fifosz)    (60 * (fifosz) / 100)


/* ////////////////////////////////////////////////////////////////////////////// */
/*                                COMMON MACRO                                 // */
/* ////////////////////////////////////////////////////////////////////////////// */

#ifndef smp_mb
#define smp_mb()
#endif

#ifndef mb
#define mb()
#endif

#define DMX_THREAD_DELAY(msec)             mdelay(msec)

#define UNUSE_PARAMETER(param)             ((param) = (param))
#define DMX_MAX_PATH_LEN                   (200)
#define DMX_FILE_NAME                      (TEXT(__FILE__))
#define DMX_FUNC_NAME                      (TEXT(__func__))
#define DMX_LINE_NO                        (__LINE__)
#define DMX_UINT64_16U_LOGSTR              TEXT("0x%08x%08x")
#define DMX_UINT64_16U_LOG_H(val)          (u32)((val) >> 32)
#define DMX_UINT64_16U_LOG_L(val)          (u32)(val)
#define DMX_UINT64_10U_LOG(val)            (val)
#define DMX_PTS_LOG_MS(pts)                PTS_TO_MS(pts)
#define DMX_PTS_LOG_PTS(pts)               (pts)

#define DMX_UINT64_10U_LOGSTR              "%lld"
#define DMX_PTS_LOGSTR                     "%lld ms(%lld)"
#define DMX_GET_LASTERR                    (RET_DMX_UNEXPECT)

#define DMX_INVALID_UINT8                  ((u8)(-1))
#define DMX_INVALID_UINT16                 ((u16)(-1))
#define DMX_INVALID_UINT32                 ((u32)(-1))
#define DMX_INVALID_UINT64                 ((u64)(-1))
#define DMX_INVALID_UINTPTR_T              ((uintptr_t)(-1))

#define DMX_WAIT_INFINITE                  DMX_INVALID_UINT32

#define DMX_MAX(a, b)       (((a) > (b))?(a):(b))
#define DMX_MIN(a, b)       (((a) < (b))?(a):(b))
#define DMX_ARRAY_SIZE(a)   (sizeof(a)/sizeof((a)[0]))
/* / Data size calculation */
#define DMX_DATASIZE(rp, wp, size)      \
	(((rp) <= (wp)) ? ((wp) - (rp)) : (((wp) + (size)) - (rp)))


#define DMX_EMPTYSIZE(rp, wp, size)     \
	(((wp) < (rp)) ? ((rp) - (wp)) : (((rp) + (size)) - (wp)))

#define DMX_ABS_VAL(x)                  (((x) > 0) ? (x) : (-1 * (x)))

#define DMX_FBYTE(arg, idx)             (*((u8 *)&(arg) + (idx)))
#define DMX_PBYTE(arg, idx)             (*((u8 *)(arg) + (idx)))
#define DMX_FWORD(arg, idx)             (*((u16 *)&(arg) + (idx)))
#define DMX_PWORD(arg, idx)             (*((u16 *)(arg) + (idx)))

#define FBYTE7(arg)                     DMX_FBYTE(arg, 7)
#define FBYTE6(arg)                     DMX_FBYTE(arg, 6)
#define FBYTE5(arg)                     DMX_FBYTE(arg, 5)
#define FBYTE4(arg)                     DMX_FBYTE(arg, 4)
#define FBYTE3(arg)                     DMX_FBYTE(arg, 3)
#define FBYTE2(arg)                     DMX_FBYTE(arg, 2)
#define FBYTE1(arg)                     DMX_FBYTE(arg, 1)
#define FBYTE0(arg)                     DMX_FBYTE(arg, 0)

/* ********************************************************************* */
/*           Macro for Bit, Byte, u32 manipulation                      */
/* ********************************************************************* */
#ifdef __BIG_ENDIAN

/* from decmacro.h */
#define bLoByte(arg)                    DMX_FBYTE(arg, 1)
#define bHiByte(arg)                    DMX_FBYTE(arg, 0)

#define wLoWord(arg)                    DMX_FWORD(arg, 1)
#define wHiWord(arg)                    DMX_FWORD(arg, 0)

#define DMX_LOBYTE(arg)                 DMX_FBYTE(arg, 1)
#define DMX_HIBYTE(arg)                 DMX_FBYTE(arg, 0)

#define DMX_LOWORD(arg)                 DMX_FWORD(arg, 1)
#define DMX_HIWORD(arg)                 DMX_FWORD(arg, 0)

#define BYTE0(arg)                      DMX_FBYTE(arg, 3)
#define BYTE1(arg)                      DMX_FBYTE(arg, 2)
#define BYTE2(arg)                      DMX_FBYTE(arg, 1)
#define BYTE3(arg)                      DMX_FBYTE(arg, 0)

#define LOAD_BYTE(arg1, arg2)           do {\
	arg2 = (u8)DMX_PBYTE(arg1, 0); \
	arg2 = (arg2) & (u8)0xff; \
} while (0)

#define LOADB_WORD(arg1, arg2)          do {\
	DMX_HIBYTE(arg2) = DMX_PBYTE(arg1, 0);  \
	DMX_LOBYTE(arg2) = DMX_PBYTE(arg1, 1);  \
	arg2 = (arg2) & 0xffff; \
} while (0)

#define LOADB_WORD2BYTE(wordarg, pu1Buf) do {\
	wordarg = (wordarg) & 0xffff;            \
	DMX_PBYTE(pu1Buf, 0) = DMX_FBYTE(wordarg, 0); \
	DMX_PBYTE(pu1Buf, 1) = DMX_FBYTE(wordarg, 1); \
} while (0)

#define LOADB_3BYTES2DWORD(arg1, arg2)  (arg2 = (u32)DMX_PBYTE(arg1, 2)        +    \
						(u32)(DMX_PBYTE(arg1, 1) << 8)  +    \
						(u32)(DMX_PBYTE(arg1, 0) << 16))

#define LOADB_DWRD(arg1, arg2)          (arg2 = (u32)DMX_PBYTE(arg1, 3)        +    \
						(u32)(DMX_PBYTE(arg1, 2) << 8)  +    \
						(u32)(DMX_PBYTE(arg1, 1) << 16) +    \
						(u32)(DMX_PBYTE(arg1, 0) << 24))

#define LOADB_DWRD2BYTE(dwordarg, pu1Buf) do {\
	DMX_PBYTE(pu1Buf, 3) = DMX_FBYTE((u8*)dwordarg, 3);   \
	DMX_PBYTE(pu1Buf, 2) = DMX_FBYTE((u8*)dwordarg, 2); \
	DMX_PBYTE(pu1Buf, 1) = DMX_FBYTE((u8*)dwordarg, 1); \
	DMX_PBYTE(pu1Buf, 0) = DMX_FBYTE((u8*)dwordarg, 0); \
} while (0)

#define LOADB_DOBL(arg1, arg2)          do {\
	DMX_FBYTE((u8*)arg2, 7) = DMX_PBYTE(arg1, 7);    \
	DMX_FBYTE((u8*)arg2, 6) = DMX_PBYTE(arg1, 6);    \
	DMX_FBYTE((u8*)arg2, 5) = DMX_PBYTE(arg1, 5);    \
	DMX_FBYTE((u8*)arg2, 4) = DMX_PBYTE(arg1, 4);    \
	DMX_FBYTE((u8*)arg2, 3) = DMX_PBYTE(arg1, 3);    \
	DMX_FBYTE((u8*)arg2, 2) = DMX_PBYTE(arg1, 2);    \
	DMX_FBYTE((u8*)arg2, 1) = DMX_PBYTE(arg1, 1);    \
	DMX_FBYTE((u8*)arg2, 0) = DMX_PBYTE(arg1, 0);	\
} while (0)

#define LOADL_WORD(arg1, arg2)          do {\
	DMX_LOBYTE(arg2) = (u32)DMX_PBYTE(arg1, 0);  \
	DMX_HIBYTE(arg2) = (u32)DMX_PBYTE(arg1, 1);  \
	arg2 = (arg2) & (u16)0xffff; \
} while (0)

#define LOADL_WORD2BYTE(wordarg, pu1Buf) do {\
	wordarg = (wordarg) & 0xffff;             \
	DMX_PBYTE(pu1Buf, 1) = DMX_FBYTE(wordarg, 0);   \
	DMX_PBYTE(pu1Buf, 0) = DMX_FBYTE(wordarg, 1); \
} while (0)

#define LOADL_3BYTES2DWORD(arg1, arg2)  (arg2 = (u32)DMX_PBYTE(arg1, 0)        +    \
						((u32)DMX_PBYTE(arg1, 1) << 8)  +    \
						((u32)DMX_PBYTE(arg1, 2) << 16))

#define LOADL_DWRD(arg1, arg2)          ((u32)(arg2 = (u32)DMX_PBYTE(arg1, 0)        +    \
						((u32)DMX_PBYTE(arg1, 1) << 8)  +    \
						((u32)DMX_PBYTE(arg1, 2) << 16) +    \
						((u32)DMX_PBYTE(arg1, 3) << 24)))

#define LOADL_DWRD2BYTE(dwordarg, pu1Buf) do {\
	DMX_PBYTE(pu1Buf, 3) = DMX_FBYTE(dwordarg, 0);   \
	DMX_PBYTE(pu1Buf, 2) = DMX_FBYTE(dwordarg, 1);   \
	DMX_PBYTE(pu1Buf, 1) = DMX_FBYTE(dwordarg, 2);   \
	DMX_PBYTE(pu1Buf, 0) = DMX_FBYTE(dwordarg, 3); \
} while (0)

#define LOADL_DOBL(arg1, arg2)          do {\
	DMX_FBYTE(arg2, 0) = DMX_PBYTE(arg1, 7);    \
	DMX_FBYTE(arg2, 1) = DMX_PBYTE(arg1, 6);    \
	DMX_FBYTE(arg2, 2) = DMX_PBYTE(arg1, 5);    \
	DMX_FBYTE(arg2, 3) = DMX_PBYTE(arg1, 4);    \
	DMX_FBYTE(arg2, 4) = DMX_PBYTE(arg1, 3);    \
	DMX_FBYTE(arg2, 5) = DMX_PBYTE(arg1, 2);    \
	DMX_FBYTE(arg2, 6) = DMX_PBYTE(arg1, 1);    \
	DMX_FBYTE(arg2, 7) = DMX_PBYTE(arg1, 0); \
} while (0)

#else				/* __BIG_ENDIAN */

#define bHiByte(arg)                    DMX_FBYTE(arg, 1)
#define bLoByte(arg)                    DMX_FBYTE(arg, 0)

#define wHiWord(arg)                    DMX_FWORD(arg, 1)
#define wLoWord(arg)                    DMX_FWORD(arg, 0)

#define DMX_HIBYTE(arg)                 DMX_FBYTE(arg, 1)
#define DMX_LOBYTE(arg)                 DMX_FBYTE(arg, 0)

#define DMX_HIWORD(arg)                 DMX_FWORD(arg, 1)
#define DMX_LOWORD(arg)                 DMX_FWORD(arg, 0)

#define BYTE3(arg)                      DMX_FBYTE(arg, 3)
#define BYTE2(arg)                      DMX_FBYTE(arg, 2)
#define BYTE1(arg)                      DMX_FBYTE(arg, 1)
#define BYTE0(arg)                      DMX_FBYTE(arg, 0)

#define LOAD_BYTE(arg1, arg2)           do {\
	arg2 = DMX_PBYTE(arg1, 0);            \
	arg2 = (arg2) & (u8)0xff; \
} while (0)

#define LOADL_WORD(arg1, arg2)          do {\
	DMX_LOBYTE(arg2) = DMX_PBYTE(arg1, 0);  \
	DMX_HIBYTE(arg2) = DMX_PBYTE(arg1, 1);  \
	arg2 = (arg2) & (u16)0xffff; \
} while (0)

#define LOADL_WORD2BYTE(wordarg, pu1Buf) do {\
	wordarg = wordarg & 0xffff;            \
	DMX_PBYTE(pu1Buf, 1) = DMX_FBYTE(wordarg, 1); \
	DMX_PBYTE(pu1Buf, 0) = DMX_FBYTE(wordarg, 0); \
} while (0)

#define LOADL_3BYTES2DWORD(arg1, arg2)   (arg2 = DMX_PBYTE(arg1, 0)        +    \
						(DMX_PBYTE(arg1, 1) << 8)  +    \
						(DMX_PBYTE(arg1, 2) << 16))

#define LOADL_DWRD(arg1, arg2)           ((u32)(arg2 = (u32)DMX_PBYTE(arg1, 0)        +    \
						((u32)DMX_PBYTE(arg1, 1) << 8)  +    \
						((u32)DMX_PBYTE(arg1, 2) << 16) +    \
						((u32)DMX_PBYTE(arg1, 3) << 24)))

#define LOADL_DWRD2BYTE(dwordarg, pu1Buf) do {\
	DMX_PBYTE(pu1Buf, 3) = DMX_FBYTE(dwordarg, 3);   \
	DMX_PBYTE(pu1Buf, 2) = DMX_FBYTE(dwordarg, 2);   \
	DMX_PBYTE(pu1Buf, 1) = DMX_FBYTE(dwordarg, 1);   \
	DMX_PBYTE(pu1Buf, 0) = DMX_FBYTE(dwordarg, 0); \
} while (0)

#define LOADL_DOBL(arg1, arg2)          do {\
	DMX_FBYTE(arg2, 7) = DMX_PBYTE(arg1, 7);    \
	DMX_FBYTE(arg2, 6) = DMX_PBYTE(arg1, 6);    \
	DMX_FBYTE(arg2, 5) = DMX_PBYTE(arg1, 5);    \
	DMX_FBYTE(arg2, 4) = DMX_PBYTE(arg1, 4);    \
	DMX_FBYTE(arg2, 3) = DMX_PBYTE(arg1, 3);    \
	DMX_FBYTE(arg2, 2) = DMX_PBYTE(arg1, 2);    \
	DMX_FBYTE(arg2, 1) = DMX_PBYTE(arg1, 1);    \
	DMX_FBYTE(arg2, 0) = DMX_PBYTE(arg1, 0); \
} while (0)

#define LOADB_WORD(arg1, arg2)          do {\
	DMX_HIBYTE(arg2) = DMX_PBYTE(arg1, 0);  \
	DMX_LOBYTE(arg2) = DMX_PBYTE(arg1, 1);  \
	arg2 = (arg2) & (u32)0xffff; \
} while (0)

#define LOADB_WORD2BYTE(wordarg, pu1Buf) do {\
	wordarg = (wordarg) & 0xffff;           \
	DMX_PBYTE(pu1Buf, 0) = DMX_FBYTE(wordarg, 1); \
	DMX_PBYTE(pu1Buf, 1) = DMX_FBYTE(wordarg, 0); \
} while (0)

#define LOADB_3BYTES2DWORD(arg1, arg2)  (arg2 = (u32)DMX_PBYTE((u8*)arg1, 2)        +    \
						(u32)(DMX_PBYTE((u8*)arg1, 1) << 8)  +    \
						(u32)(DMX_PBYTE((u8*)arg1, 0) << 16))

#define LOADB_DWRD(arg1, arg2)          (arg2 = (u32)DMX_PBYTE(arg1, 3)        +    \
						((u32)DMX_PBYTE(arg1, 2) << 8)  +    \
						((u32)DMX_PBYTE(arg1, 1) << 16) +    \
						((u32)DMX_PBYTE(arg1, 0) << 24))

#define LOADB_DWRD2BYTE(dwordarg, pu1Buf) do {\
	DMX_PBYTE(pu1Buf, 3) = DMX_FBYTE((u8*)dwordarg, 0);   \
	DMX_PBYTE(pu1Buf, 2) = DMX_FBYTE((u8*)dwordarg, 1);   \
	DMX_PBYTE(pu1Buf, 1) = DMX_FBYTE((u8*)dwordarg, 2);   \
	DMX_PBYTE(pu1Buf, 0) = DMX_FBYTE((u8*)dwordarg, 3); \
} while (0)

#define LOADB_DOBL(arg1, arg2)	do {\
	DMX_FBYTE(arg2, 0) = DMX_PBYTE(arg1, 7);    \
	DMX_FBYTE(arg2, 1) = DMX_PBYTE(arg1, 6);    \
	DMX_FBYTE(arg2, 2) = DMX_PBYTE(arg1, 5);    \
	DMX_FBYTE(arg2, 3) = DMX_PBYTE(arg1, 4);    \
	DMX_FBYTE(arg2, 4) = DMX_PBYTE(arg1, 3);    \
	DMX_FBYTE(arg2, 5) = DMX_PBYTE(arg1, 2);    \
	DMX_FBYTE(arg2, 6) = DMX_PBYTE(arg1, 1);    \
	DMX_FBYTE(arg2, 7) = DMX_PBYTE(arg1, 0); \
} while (0)

#endif				/* __BIG_ENDIAN */

#define LOADB_3BYTES(arg1, arg2)   do {\
	*((u8 *)(arg2) + 2) = *((u8 *)(arg1) + 2); \
	*((u8 *)(arg2) + 1) = *((u8 *)(arg1) + 1) ;\
	*((u8 *)(arg2)) = *((u8 *)(arg1)); \
} while (0)

/* Get the value of the arg's bit n */
#define GET_BIT(arg, n)                 (((arg) >> (n)) & 1)
#define GET_BIT0(arg)                   GET_BIT(arg, 0)
#define GET_BIT1(arg)                   GET_BIT(arg, 1)
#define GET_BIT2(arg)                   GET_BIT(arg, 2)
#define GET_BIT3(arg)                   GET_BIT(arg, 3)
#define GET_BIT4(arg)                   GET_BIT(arg, 4)
#define GET_BIT5(arg)                   GET_BIT(arg, 5)
#define GET_BIT6(arg)                   GET_BIT(arg, 6)
#define GET_BIT7(arg)                   GET_BIT(arg, 7)

/* Get the value of the arg's startbit-endbit */
#define GET_BITS(arg, StartBit, EndBit)   \
	(((arg) >> (StartBit)) & (0xFFFFFFFF >> (32 - (EndBit - StartBit + 1))))
#define GET_BITS01(arg)                 GET_BITS(arg, 0, 1)
#define GET_BITS02(arg)                 GET_BITS(arg, 0, 2)
#define GET_BITS03(arg)                 GET_BITS(arg, 0, 3)
#define GET_BITS04(arg)                 GET_BITS(arg, 0, 4)
#define GET_BITS05(arg)                 GET_BITS(arg, 0, 5)
#define GET_BITS06(arg)                 GET_BITS(arg, 0, 6)
#define GET_BITS12(arg)                 GET_BITS(arg, 1, 2)
#define GET_BITS13(arg)                 GET_BITS(arg, 1, 3)
#define GET_BITS14(arg)                 GET_BITS(arg, 1, 4)
#define GET_BITS15(arg)                 GET_BITS(arg, 1, 5)
#define GET_BITS16(arg)                 GET_BITS(arg, 1, 6)
#define GET_BITS17(arg)                 GET_BITS(arg, 1, 7)
#define GET_BITS23(arg)                 GET_BITS(arg, 2, 3)
#define GET_BITS24(arg)                 GET_BITS(arg, 2, 4)
#define GET_BITS25(arg)                 GET_BITS(arg, 2, 5)
#define GET_BITS26(arg)                 GET_BITS(arg, 2, 6)
#define GET_BITS27(arg)                 GET_BITS(arg, 2, 7)
#define GET_BITS34(arg)                 GET_BITS(arg, 3, 4)
#define GET_BITS35(arg)                 GET_BITS(arg, 3, 5)
#define GET_BITS36(arg)                 GET_BITS(arg, 3, 6)
#define GET_BITS37(arg)                 GET_BITS(arg, 3, 7)
#define GET_BITS45(arg)                 GET_BITS(arg, 4, 5)
#define GET_BITS46(arg)                 GET_BITS(arg, 4, 6)
#define GET_BITS47(arg)                 GET_BITS(arg, 4, 7)
#define GET_BITS56(arg)                 GET_BITS(arg, 5, 6)
#define GET_BITS57(arg)                 GET_BITS(arg, 5, 7)
#define GET_BITS67(arg)                 GET_BITS(arg, 6, 7)

#ifndef GET_ABS_VAL
#define GET_ABS_VAL(val)                DMX_ABS_VAL(val)
#endif

#define PTS_TO_MS(val)                  (((val) == INVALID_TIMESTAMP) ?  \
						((u64)(-1)) : ((u64)((val) / 90)))
#define MS_TO_PTS(val)                  ((u64)((val) * 90))

/* ////////////////////////////////////////////////////////////////////////////// */
/*                          AU composition Related                             // */
/* ////////////////////////////////////////////////////////////////////////////// */
#define DMX_GET_PICTYPE(u4PicType)         ((u32)((u4PicType) & 0xFFFF00FF))

/* ////////////////////////////////////////////////////////////////////////////// */
/*                          DMX ASSERT DEFINITION                              // */
/* ////////////////////////////////////////////////////////////////////////////// */

#define DMX_ASSERT(argu)	\
	{	if (!(argu))		\
		{                    \
			MM_RETAILMSG(1, (TEXT("DBGCHK Failed:%s at line %d in %s\r\n"), \
			TEXT(__func__), __LINE__, TEXT(__FILE__))); \
		}                    \
	}


#define CFA_ASSERT(argu)   DMX_ASSERT(argu)

#ifdef __linux__
typedef enum _DMX_PM_STATE {
	D0,
	D1,
	D2,
	D3,
	D4,
	INVALIED_PM_STATE,
} DMX_PM_STATE;
#define VALID_DX(state) (((state) == D0) || ((state) == D1) || ((state) == D2)\
			|| ((state) == D3) || ((state) == D4))
#endif				/* __linux__ */

/* ////////////////////////////////////////////////////////////////////////////// */
/*                     DMX Performance Statistic Related                       // */
/* ////////////////////////////////////////////////////////////////////////////// */

#if DMX_PFM_TEST

typedef unsigned long long LARGE_INTEGER;

typedef struct _DMX_CFA_PFM {
	LARGE_INTEGER u4CfaStartTick;
	LARGE_INTEGER u4CfaEndTick;
	LARGE_INTEGER u4CfaRunTime;
	u64 u8CfaRunCnt;
} CFA_PFM;

typedef struct _PSR_STM_PFM {
	LARGE_INTEGER u4StartPb2FifoTick;
	LARGE_INTEGER u4EndPb2FifoTick;
	LARGE_INTEGER u8Pb2FifoTime;
	u64 u8Pb2FifoCnt;

	LARGE_INTEGER u4HWStartTxTick;
	LARGE_INTEGER u4HWEndTxTick;
	LARGE_INTEGER u8HWTxTime;
	u64 u8HWTxCompleteCnt;
	u64 u8HWStartTxCnt;

	LARGE_INTEGER u4StartPsrEsmTick;
	LARGE_INTEGER u4EndPsrEsmTick;
	LARGE_INTEGER u8PsrEsmTime;
	u64 u8PsrEsmCnt;
	u64 u8CreateAUCnt;

	LARGE_INTEGER u4GetAUStartTick;
	LARGE_INTEGER u4GetAUEndTick;
	LARGE_INTEGER u8GetAUTime;
	u32 u4GetAUCnt;
	u32 u4GetAURealCnt;
	u32 u4FifoFullCnt;
	u32 u4ESTableFullCnt;
} PSR_STM_PFM;

typedef struct _PSR_DECRYPT_STM_PFM {
	LARGE_INTEGER u4DecryptStTick;
	LARGE_INTEGER u4DecryptEndTick;
	LARGE_INTEGER u8DecryptTime;
	u64 u8DecryptCnt;
	u64 u8DecryptPb2FifoCnt;
	u64 u8DecryptInSyncCnt;
} PSR_DECRYPT_STM_PFM;

typedef struct _PSR_SYNCPB_PFM {
	LARGE_INTEGER u4StartSyncPbTick;
	LARGE_INTEGER u4EndSyncPbTick;
	LARGE_INTEGER u8SyncPbbufTime;
	u64 u8SyncPbbufCnt;
} PSR_SYNCPB_PFM;

typedef enum {
	PSR_PFM_OPER_NONE,
	PSR_PFM_OPER_SYNCPB,
	PSR_PFM_OPER_DMA_V,
	PSR_PFM_OPER_DMA_A,
	PSR_PFM_OPER_DMA_SP
} E_PSR_PFM_OPER_TYPE_T;

typedef struct _DMX_SPTINST_PFM {
	LARGE_INTEGER u4StartTick;
	E_PSR_PFM_OPER_TYPE_T eCurOper;
	PSR_SYNCPB_PFM rSyncPb;
	CFA_PFM rCfa;
	LARGE_INTEGER u4EndTick;
	LARGE_INTEGER u4TotalTime;
} SPTINST_PFM;

typedef struct _PSR_PFM {
	PSR_STM_PFM rVideo;
	PSR_STM_PFM rAudio;
	PSR_STM_PFM rSP;

	PSR_DECRYPT_STM_PFM rDecryptV;
	PSR_DECRYPT_STM_PFM rDecryptA;

	SPTINST_PFM rInst[DMX_MAX_SPT_INST_CNT];

	LARGE_INTEGER u8Frequency;
} PSR_PFM;

EXTERN PSR_PFM g_rPsrPfm;
#endif	/* DMX_PFM_TEST */

EXTERN const char *g_aszSptDataTypeName[MAX_SPT_DATA_TYPE_CNT];
#define DMX_SPTDATATYPE_STR(eDataType) \
	(((eDataType) < MAX_SPT_DATA_TYPE_CNT) ? \
	g_aszSptDataTypeName[eDataType] : TEXT("UNKNOWN"))

EXTERN const char *g_aszStrmTypeName[MAX_ES_TYPE_CNT];
#define ESM_TYPESTR(eType) \
	(((eType) < MAX_ES_TYPE_CNT) ? g_aszStrmTypeName[(eType)] : TEXT("UNKNOWN"))

	#ifdef __cplusplus
}
#endif
#endif	/* #ifndef DMX_INTERNAL_DEFINE_H */
