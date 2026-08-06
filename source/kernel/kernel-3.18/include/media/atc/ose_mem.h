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
#ifndef OSE_H
#define OSE_H

#include "x_os.h"
#include "memdbg_c.h"

#ifdef __cplusplus
extern "C" {
#endif

#if MEMCHK_DEBUG
#undef BSP_NO_MEMMAN
#else
#define BSP_NO_MEMMAN
#endif

/* #define CHECK_OSE_MEMORY */

#define AFIFO_ALIGN				4096	/* Demux Use */
#define VFIFO_ALIGN				4096	/* Demux Use */
#define PBBUF_ALIGN				4096	/* Demux Use */
#define JDEC_ALIGN				4096	/* Image Decoder Use */

#define VFIFO_SIZE					OSE_BUF_ALIGN_MASK((4 * 1024 * 1024),  VFIFO_ALIGN)

/* SubTittle DMX Used */
#define FIFO_SP_THRESHOLD					(7 / 10)

#define OSE_GetPbbufInterleaveSize()		\
	MM_Config_GetInstance()->pfGetAVPbbufSize()
#define OSE_GetPbbufBadInterleaveSize()	\
	MM_Config_GetInstance()->pfGetAPbbufSize()
#define OSE_GetPbbufAVSlotSize()				\
	MM_Config_GetInstance()->pfGetPbbufSlotSize()
#define OSE_GetSPPbbufSize()						\
	MM_Config_GetInstance()->pfGetSPPbbufSize()
#define OSE_GetSPPbbufSlotSize()				\
	MM_Config_GetInstance()->pfGetSPPbbufSlotSize()
#define OSE_GetVFIFOSize()							\
	MM_Config_GetInstance()->pfGetVFIFOSize()
#define OSE_GetHighBitrateVFIFOSize()		\
	MM_Config_GetInstance()->pfGetHighBitrateVFIFOSize()
#define OSE_GetSPFIFOSize()							\
	MM_Config_GetInstance()->pfGetSPFIFOSize()
#define OSE_GetVdecBufSize()						\
	MM_Config_GetInstance()->pfGetVDECBufSize()
#define OSE_GetAFIFOSize()							\
		MM_Config_GetInstance()->pfGetAFIFOSize()


#define OSE_GetMMReservedMemSize()			\
	MM_Config_GetInstance()->pfGetMMReservedMemSize()
/* Audio DSP Start Address in Reserved Memory */

#define OSE_GetMMReservedMemStartAddr()			\
	MM_Config_GetInstance()->pfGetMMReservedMemAddr()

#define OSE_GetChipMemSize()				\
	MM_Config_GetInstance()->pfGetMemCfg()

/* ADD BY MTK40495  END */

	typedef enum {
		OSE_REGION_NONE = 0,
		OSE_DEMUXER,
		OSE_VDEC,
		OSE_ADEC,
		OSE_RLE,
		OSE_JDEC,
		OSE_PNG,
		OSE_GIF,
		OSE_WCH,
		OSE_REGION_MAX
	} OSE_MEM_REGION;

#define OSE_BUF_ALIGN_MASK(value, mask) \
	((((value) + ((mask) - 1)) / (mask)) * (mask))
#define OSE_MemAllocCustom(a, b, c, d)	 \
	OSE_MemAllocCustom_R(a, b, c, d, __FILE__, __LINE__)
#define OSE_MemAllocCustom_s(a, b, c, d)  \
	OSE_MemAllocCustom_S(a, b, c, d, __FILE__, __LINE__)
#define OSE_MemFreeCustom(a, b)				 \
	OSE_MemFreeCustom_R(a, b, __FILE__, __LINE__)

bool OSE_MemInit(OSE_MEM_REGION eMemRegion);
void OSE_MemUninit(OSE_MEM_REGION eMemRegion);

void *OSE_MemAllocCustom_R(OSE_MEM_REGION eMemRegion,
	__u32 u4Size, __u32 u4Align,
	uintptr_t *pptrPhy, char *file, __u32 u4Line);
void *OSE_MemAllocCustom_S(OSE_MEM_REGION eMemRegion,
	__u32 u4Size, __u32 u4Align,
	uintptr_t *pptrPhy, char *file, __u32 u4Line);
void OSE_MemFreeCustom_R(OSE_MEM_REGION eMemRegion,
	void *pvVirAddr, char *file, __u32 u4Line);

uintptr_t OSE_VAToPA(void *pvVirAddr);
void *OSE_PAToVA(uintptr_t ptrPhy);

void OSE_PrintOSEMemoryCfg(void);


typedef __u32(*MM_GET_MEMCFG) (void);
typedef void *(*MM_GET_RESERVEDMEM_SA) (void);
typedef __u32(*MM_GET_RESERVEDMEM_SIZE) (void);
typedef __u32(*MM_GET_AVPBBUF_SIZE) (void);
typedef __u32(*MM_GET_APBBUF_SIZE) (void);
typedef __u32(*MM_GET_PBBUF_SLOT_SIZE) (void);
typedef __u32(*MM_GET_VFIFO_SIZE) (void);
typedef __u32(*MM_GET_HIGHBITRATEVFIFOSIZE) (void);
typedef __u32(*MM_GET_SPFIFO_SIZE) (void);
typedef __u32(*MM_GET_VDECBUF_SIZE) (void);
typedef __u32(*MM_GET_SPPBBUF_SIZE) (void);
typedef __u32(*MM_GET_SPPBBUF_SLOT_SIZE) (void);
typedef __u32(*MM_GET_AFIFO_SIZE) (void);

typedef struct mmcfg {
	MM_GET_MEMCFG pfGetMemCfg;
	MM_GET_RESERVEDMEM_SA  pfGetMMReservedMemAddr;
	MM_GET_RESERVEDMEM_SIZE pfGetMMReservedMemSize;
	MM_GET_AVPBBUF_SIZE pfGetAVPbbufSize;
	MM_GET_APBBUF_SIZE pfGetAPbbufSize;
	MM_GET_PBBUF_SLOT_SIZE pfGetPbbufSlotSize;
	MM_GET_SPPBBUF_SIZE pfGetSPPbbufSize;
	MM_GET_SPPBBUF_SLOT_SIZE pfGetSPPbbufSlotSize;
	MM_GET_VFIFO_SIZE pfGetVFIFOSize;
	MM_GET_HIGHBITRATEVFIFOSIZE pfGetHighBitrateVFIFOSize;
	MM_GET_SPFIFO_SIZE pfGetSPFIFOSize;
	MM_GET_VDECBUF_SIZE pfGetVDECBufSize;
	MM_GET_AFIFO_SIZE pfGetAFIFOSize;
} MM_CONFIG_T;

/*
---------------------------------------------------------------------------
*  Multimedia Mem Cfg
---------------------------------------------------------------------------
*/
const MM_CONFIG_T *MM_Config_GetInstance(void);

#ifdef __cplusplus
}
#endif
#endif
