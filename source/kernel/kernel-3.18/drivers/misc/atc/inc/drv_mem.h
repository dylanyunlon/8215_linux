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
#include "x_typedef.h"

#define AUD_VERIFY_ONLY_VERSION   0         // for audio IC verify used. default is 0

#if (!AUD_VERIFY_ONLY_VERSION)
#define AUD_FIFO_STATIC_ALLOC           0         // don't support audio alloc static memory
#else
#define AUD_FIFO_STATIC_ALLOC           1         // support audio alloc static memory
#endif

// 20080606, roman change AFIFO size as spec.
// 1313563+ 7456= 1321019 (bytes)
// Align to 256 bytes alignment + system reserved
// (1314048 + 7680) + 1024 = 1322752 (bytes)
// 20080922, roman change AFIFO size for Dolby TrueHD
// 1322752 + 0x19000
//#define DSP_RESERVED_FIFO_SIZE          (1425152)
// 20090603, YCHung,for 2ch SPDIFin MUST BE times of 2*3 bytes

#ifdef CONFIG_DRAM256_MODEL
#undef CONFIG_DRAM256_MODEL
#define CONFIG_DRAM256_MODEL            1
#endif

#if CONFIG_DRAM256_MODEL
#define DSP_RESERVED_FIFO_SIZE                  0x116000//(1130464 + 7456 + 1024)//0x119000 // 0x116000//
#else
#define DSP_RESERVED_FIFO_SIZE                  0x300000//(1425408)
#endif

#if(CONFIG_DRV_AUDIO_IN)
  #ifdef  CONFIG_DRV_HDMI_RX
#define DSP_RESERVED_AUDIN_FIFO_SIZE            (0x210000) // Allocate 2M  for Multiple Line In buffer //
  #else
#define DSP_RESERVED_AUDIN_FIFO_SIZE            (0x203A00) // Allocate 2M  for Multiple Line In buffer
  #endif
#endif

#if CONFIG_AUD_BTN_MEM_PHASE3
#define AUD_EFFSND_STREAMING_SUPPORT 1
#else
#define AUD_EFFSND_STREAMING_SUPPORT 0
#endif

#if AUD_EFFSND_STREAMING_SUPPORT
#define DSP_RESERVED_EFFSND_STRM_BUF_SLICE_SIZE     (128*1024)
#define DSP_RESERVED_EFFSND_STRM_BUF_SIZE           (8*DSP_RESERVED_EFFSND_STRM_BUF_SLICE_SIZE)
#define DSP_RESERVED_EFFSND_STRM_BUF_ALIGNMENT      0x100
#endif
#define DSP_RESERVED_EFFSND_FIFO_SIZE               (0x900000)
#define DSP_RESERVED_EFFSND_FIFO_ALIGNMENT          0x100

// memory layout change for effect sound buffer overlap in channel 1
// AFIFO will be placed at bottom of channel 1
#define DSP_RESERVED_AFIFO_ALIGNMENT                (256)

void drv_mem_set_pip(BOOL fgPIP);
void* x_alloc_aligned_pbbuf_mem(UINT32 u4Size, UINT32 u4Align);
void x_free_aligned_pbbuf_mem(void *pUser);

void* x_alloc_aligned_vfifo_mem(UINT32 u4Size, UINT32 u4Align);
void x_free_aligned_vfifo_mem(void *pUser);
void* x_get_aligned_vfifo_mem(UINT32 u4Size, UINT32 u4Align);

void* x_alloc_afifo_mem(UINT32 u4Size);
void x_free_afifo_mem(void* p);
void* x_alloc_aligned_afifo_mem(UINT32 u4Size, UINT32 u4Align);
void x_free_aligned_afifo_mem(void *pUser);
#endif // __DRV_MEM_H

