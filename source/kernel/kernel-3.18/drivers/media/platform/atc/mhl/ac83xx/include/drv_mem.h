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

#ifndef __DRV_MEM_H
#define __DRV_MEM_H

#include "drv_def.h"
#include "chip_ver.h"
#include "dram_model.h"
#include "drv_config.h"

#include "sys_config.h"

#define AUD_VERIFY_ONLY_VERSION   0         /* for audio IC verify used. default is 0 */
#define AUD_VERIFY_LINE_IN_ONLY_VERSION   0 /* for audio IC verify used. default is 0 */

#if (!AUD_VERIFY_ONLY_VERSION)
#define AUD_FIFO_STATIC_ALLOC     0         /* don't support audio alloc static memory */
#else
#define AUD_FIFO_STATIC_ALLOC     1         /* support audio alloc static memory */
#endif

/* 20080606, roman change AFIFO size as spec.
1313563+ 7456= 1321019 (bytes)
Align to 256 bytes alignment + system reserved
(1314048 + 7680) + 1024 = 1322752 (bytes)
20080922, roman change AFIFO size for Dolby TrueHD
1322752 + 0x19000
#define DSP_RESERVED_FIFO_SIZE          (1425152)
20090603, YCHung,for 2ch SPDIFin MUST BE times of 2*3 bytes */

#if CONFIG_DRAM256_MODEL
#define DSP_RESERVED_FIFO_SIZE          (1130464 + 7456 + 1024)
#else
#define DSP_RESERVED_FIFO_SIZE          (1425408)
#endif


/* (aud_wrkbuf_def) -- Water 2011-01-17 for Office-S, app can't get audio driver code */
#if ((CONFIG_DRV_3D_256_SUPPORT) && (CONFIG_DRV_3D_MVC_SUPPORT))
#define DSP_WORK_BUF_SIZE           \
	(3752 * 1024) /*  (3912 * 1024)   //3.9MB  // change to 3752 for Morning shrink Mic page */
#elif (CONFIG_DRV_2D_128MB_SD_SUPPORT)                 /* Add by mtk40292 for 2d 128M project; */
#define DSP_WORK_BUF_SIZE           \
	(3752 * 1024) /*  (3912 * 1024)   //3.9MB  // change to 3752 for Morning shrink Mic page */
#elif ((!CONFIG_DRAM256_MODEL) || (CONFIG_AUD_WORKBUF_384M_EN))

#if (CONFIG_CHIP_VER_CURR <= CONFIG_CHIP_VER_MT8550)
#define DSP_WORK_BUF_SIZE                   (17*512*1024)
#else
#define DSP_WORK_BUF_SIZE           \
	((17*512*1024) + 0x4000)        /*  Add 0x4000 reserved for MT8555 Page 8~11 */
#endif

#else

#if (CONFIG_CHIP_VER_CURR <= CONFIG_CHIP_VER_MT8550)
#define DSP_WORK_BUF_SIZE                   (11*512*1024)     /*  5.5 MBBytes */
#else
#define DSP_WORK_BUF_SIZE                   \
	((11*512*1024) + 0x4000)     /*  5.5 MBBytes (0x4000 Reserved for MT8555 Page8~11) */
#endif

#endif

/*  (aud_wrkbuf_def) -- Water 2011-01-17 for Office-S, app can't get audio driver code */
#if (CONFIG_CHIP_VER_CURR <= CONFIG_CHIP_VER_MT8530)
#define DSP_BUF_BOUNDARY                    (16*1024*1024)    /* 8530 , 16MB align */
#else
#define DSP_BUF_BOUNDARY                    (1*1024*1024)     /* 8550 & 8555 1MB align */
#endif

#if CONFIG_AUD_BTN_MEM_PHASE3
#define AUD_EFFSND_STREAMING_SUPPORT 1
#else
#define AUD_EFFSND_STREAMING_SUPPORT 0
#endif

#if (CONFIG_DRV_AUDIO_IN)
#if AUD_EFFSND_STREAMING_SUPPORT
#define DSP_RESERVED_AUDIN_FIFO_SIZE   (0x3F000)  /* Allocate 256K for Multiple Line In buffer, 4608 alighment */
#else
#ifdef CONFIG_DRV_HDMI_RX
#define DSP_RESERVED_AUDIN_FIFO_SIZE            (0x210000) /* Allocate 2M  for Multiple Line In buffer */
#else
#define DSP_RESERVED_AUDIN_FIFO_SIZE            (0x203A00) /* Allocate 2M  for Multiple Line In buffer */
#endif
#endif
/*#define DSP_RESERVED_AUDIN_FIFO_SIZE            (0x205B00)*/ /* Allocate 2M  for Multiple Line In buffer*/
#endif

#if AUD_EFFSND_STREAMING_SUPPORT
#define DSP_RESERVED_EFFSND_STRM_BUF_NUM            8
#define DSP_RESERVED_EFFSND_STRM_BUF_SLICE_SIZE       (32*1024)
#if CONFIG_DRV_INDEPEND_AUDIN_FIFO
#define DSP_RESERVED_EFFSND_STRM_BUF_ALLOC_SIZE    \
	(2*DSP_RESERVED_EFFSND_STRM_BUF_NUM*DSP_RESERVED_EFFSND_STRM_BUF_SLICE_SIZE)
#else
#define DSP_RESERVED_EFFSND_STRM_BUF_ALLOC_SIZE    \
	(DSP_RESERVED_EFFSND_STRM_BUF_NUM*DSP_RESERVED_EFFSND_STRM_BUF_SLICE_SIZE)
#endif
#define DSP_RESERVED_EFFSND_STRM_BUF_SIZE          \
	(DSP_RESERVED_EFFSND_STRM_BUF_NUM*DSP_RESERVED_EFFSND_STRM_BUF_SLICE_SIZE)
#define DSP_RESERVED_EFFSND_STRM_BUF_ALIGNMENT      0x100
#endif
#define DSP_RESERVED_EFFSND_FIFO_SIZE          (0x900000)
#define DSP_RESERVED_EFFSND_FIFO_ALIGNMENT      0x100

#if 1

/* memory layout change for effect sound buffer overlap in channel 1 */
/* AFIFO will be placed at bottom of channel 1 */
#define DSP_RESERVED_AFIFO_ALIGNMENT             (256)

#else

/* Xiangyang 090909 , for 8550 & 8555 1MB align */
#if (CONFIG_CHIP_VER_CURR <= CONFIG_CHIP_VER_MT8530)
#define DSP_RESERVED_AFIFO_ALIGNMENT             (16*1024*1024)
#else
#define DSP_RESERVED_AFIFO_ALIGNMENT             (1*1024*1024)
#endif

#endif /*CONFIG_DRAM256_MODEL */
void drv_mem_set_pip(BOOL fgPIP);

#if !CONFIG_SYS_MEM_PHASE3
void *x_alloc_aligned_pbbuf_mem(UINT32 u4Size, UINT32 u4Align);
void x_free_aligned_pbbuf_mem(void *pUser);
int x_kmem_sync_table(void *ptr, size_t size);
#endif
int x_kmem_sync_table_Usr2Kern(void *ptr, size_t size);

void *x_alloc_aligned_vfifo_mem(UINT32 u4Size, UINT32 u4Align);
void x_free_aligned_vfifo_mem(void *pUser);
void *x_get_aligned_vfifo_mem(UINT32 u4Size, UINT32 u4Align);

void *x_alloc_afifo_mem(UINT32 u4Size);
void x_free_afifo_mem(void *p);
void *x_alloc_aligned_afifo_mem(UINT32 u4Size, UINT32 u4Align);
void x_free_aligned_afifo_mem(void *pUser);
#endif /* __DRV_MEM_H */

