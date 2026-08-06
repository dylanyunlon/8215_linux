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
 * @file dmx_spt_mem.c
 *
 *
 * @par Project
 *
 * @par Description
 *
 *
 * @par Author_Name
 *	  Shuhui Zhang
 *
 */
#ifdef __linux__
#include <linux/mm.h>
#include <linux/errno.h>
#include "windows.h"
#include <media/atc/dmx_define.h>
#include <media/atc/ioctl_dmx.h>
/* #include <media/atc/mm_debug.h> */
#else
#include "dmx_define.h"
#include "ioctl_dmx.h"
#include "mm_debug.h"
#endif /* __linux__ */

#include "dmx_def.h"
#include "dmx_mem.h"
#include "dmx_cli.h"
#include "dmx_pvr_ddi.h"

#ifndef __linux__
#pragma warning(disable : 4127) /* disable warning C4127: conditional expression is constant */
#endif

/*******************************************************************************
********************Memory Mapping Management***********************************
*******************************************************************************/
#define MAX_OF_DMX_FUNC_NAME_LEN 50
typedef struct _MEMORY_MAP {
	E_SPT_MEM_TYPE_T	eType;
	u32				u4Size;

#if DMX_MEM_GUARD_PATTEN_CHECK
	u32				u4Align;
	uintptr_t		ptrGuardPrefixAddr;
	uintptr_t		ptrGuardSuffixAddr;
#endif /* DMX_MEM_GUARD_PATTEN_CHECK */

	void		 *pvVirtualMem;
	void		 *pvPhysicalMem;

#if DMX_MEM_PRINT_LINE_FUNCTION
	char		 szFunc[MAX_OF_DMX_FUNC_NAME_LEN];
	s32		 i4Line;
#endif

	struct _MEMORY_MAP  *prNext;
} MEMORY_MAP_T;

static MEMORY_MAP_T *_prMemMap;

#if DMX_MEM_GUARD_PATTEN_CHECK
static MEMORY_MAP_T *_prVMemMap;

static u8 _au1guard_prefix[DMX_MEM_GUARD_PATTEN_LEN] = {
	0xBE, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xED,
};

static u8 _au1guard_suffix[DMX_MEM_GUARD_PATTEN_LEN] = {
	0xBE, 0xDA, 0xDA, 0xDA, 0xDA, 0xDA, 0xDA, 0xED
};
#endif /* DMX_MEM_GUARD_PATTEN_CHECK */

static spinlock_t _DmxMemLock;

uintptr_t DMX_MEM_Align(uintptr_t ptrAddr, u32 u4Alignment)
{
	u32 u4Unaligned;

	if (u4Alignment <= 1)
		return ptrAddr;

	u4Unaligned = ptrAddr % u4Alignment;
	if (u4Unaligned > 0)
		ptrAddr += u4Alignment - u4Unaligned;

	return ptrAddr;
}

bool DMX_CheckPesHdrMemGuard(uintptr_t ptrPhyVideoHdrSa)
{
#if DMX_MEM_GUARD_PATTEN_CHECK
	MEMORY_MAP_T *prMemRing = NULL;
	unsigned long flags = 0;

	spin_lock_irqsave(&_DmxMemLock, flags);

	prMemRing = _prMemMap;

	while (NULL != prMemRing) {
		if ((uintptr_t)(prMemRing->pvPhysicalMem) == ptrPhyVideoHdrSa)
			break;

		prMemRing = prMemRing->prNext;
	}

	if ((NULL == prMemRing) ||
		(prMemRing->pvVirtualMem == NULL)) {
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
			TEXT("[MEM] Video Header Memory SA(0x%x) error\r\n"),
			ptrPhyVideoHdrSa);
		spin_unlock_irqrestore(&_DmxMemLock, flags);
		OSE_PrintOSEMemoryCfg();
		return FALSE;
	}

	if (prMemRing->eType == SPT_MEM_TYPE_OSEMALLOC) {
		DmxLogD(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
			(TEXT("[MEM] Check Memory, GuardPrefixAddr(0x%08x), FreeAddr(0x%08x),")
			TEXT(" GuardSuffixAddr(0x%08x), size(0x%08x), Func(%s), line(%d)\r\n")),
			prMemRing->ptrGuardPrefixAddr, prMemRing->pvVirtualMem,
			prMemRing->ptrGuardSuffixAddr,
			prMemRing->u4Size, prMemRing->szFunc, prMemRing->i4Line);
		if (prMemRing->ptrGuardPrefixAddr + prMemRing->u4Align !=
			(uintptr_t)(prMemRing->pvVirtualMem)) {
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MEM] Memory ERROR, GuardPrefixAddr(0x%08x), FreeAddr(0x%08x),")
				TEXT(" GuardSuffixAddr(0x%08x), size(0x%08x), Func(%s), line(%d)\r\n"),
				prMemRing->ptrGuardPrefixAddr, prMemRing->pvVirtualMem,
				prMemRing->ptrGuardSuffixAddr,
				prMemRing->u4Size, prMemRing->szFunc, prMemRing->i4Line);
			goto failexit;
		}

		if (prMemRing->ptrGuardPrefixAddr + prMemRing->u4Align +
			DMX_MEM_Align(prMemRing->u4Size,
			prMemRing->u4Align) != prMemRing->ptrGuardSuffixAddr) {
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MEM] Memory ERROR, GuardPrefixAddr(0x%08x), FreeAddr(0x%08x),")
				TEXT(" GuardSuffixAddr(0x%08x), size(0x%08x), Func(%s), line(%d)\r\n"),
				prMemRing->ptrGuardPrefixAddr, prMemRing->pvVirtualMem,
				prMemRing->ptrGuardSuffixAddr,
				prMemRing->u4Size, prMemRing->szFunc, prMemRing->i4Line);
			goto failexit;
		}

		if (memcmp((void *)((uintptr_t)((uintptr_t)prMemRing->pvVirtualMem -
			DMX_MEM_GUARD_PATTEN_LEN * sizeof(u8))),
			_au1guard_prefix, DMX_MEM_GUARD_PATTEN_LEN * sizeof(u8))) {
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MEM] Memory Prefix Guard ERROR, GuardPrefixAddr(0x%08x), FreeAddr(0x%08x),")
				TEXT(" GuardSuffixAddr(0x%08x), size(0x%08x), Func(%s), line(%d)\r\n"),
				prMemRing->ptrGuardPrefixAddr, prMemRing->pvVirtualMem,
				prMemRing->ptrGuardSuffixAddr,
				prMemRing->u4Size, prMemRing->szFunc, prMemRing->i4Line);
			goto failexit;
		}

		if (memcmp((void *)(prMemRing->ptrGuardSuffixAddr), _au1guard_suffix,
				DMX_MEM_GUARD_PATTEN_LEN * sizeof(u8))) {
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MEM] Memory Suffix Guard ERROR, GuardPrefixAddr(0x%08x), FreeAddr(0x%08x),")
				TEXT(" GuardSuffixAddr(0x%08x), size(0x%08x), Func(%s), line(%d)\r\n"),
				prMemRing->ptrGuardPrefixAddr, prMemRing->pvVirtualMem,
				prMemRing->ptrGuardSuffixAddr,
				prMemRing->u4Size, prMemRing->szFunc, prMemRing->i4Line);
			goto failexit;
		}
	} else if (prMemRing->eType == SPT_MEM_TYPE_OSEPHY) {
		DmxLogD(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
			TEXT("[MEM] Check Reserved Memory, GuardPrefixAddr(0x%08x), FreeAddr(0x%08x),")
			TEXT(" GuardSuffixAddr(0x%08x), size(0x%08x), Func(%s), line(%d)\r\n"),
			prMemRing->ptrGuardPrefixAddr, prMemRing->pvVirtualMem,
			prMemRing->ptrGuardSuffixAddr,
			prMemRing->u4Size, prMemRing->szFunc, prMemRing->i4Line);
		if (prMemRing->ptrGuardPrefixAddr + prMemRing->u4Align !=
			(uintptr_t)(prMemRing->pvVirtualMem)) {
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MEM] Reserved Memory ERROR, GuardPrefixAddr(0x%08x), FreeAddr(0x%08x),")
				TEXT(" GuardSuffixAddr(0x%08x), size(0x%08x), Func(%s), line(%d)\r\n"),
				prMemRing->ptrGuardPrefixAddr, prMemRing->pvVirtualMem,
				prMemRing->ptrGuardSuffixAddr,
				prMemRing->u4Size, prMemRing->szFunc, prMemRing->i4Line);
			goto failexit;
		}

		if (prMemRing->ptrGuardPrefixAddr + prMemRing->u4Align + DMX_MEM_Align(prMemRing->u4Size,
				prMemRing->u4Align) != prMemRing->ptrGuardSuffixAddr) {
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MEM] Reserved Memory ERROR, GuardPrefixAddr(0x%08x), ")
				TEXT("FreeAddr(0x%08x), GuardSuffixAddr(0x%08x), size(0x%08x),")
				TEXT(" Func(%s), line(%d)\r\n"),
				prMemRing->ptrGuardPrefixAddr, prMemRing->pvVirtualMem,
				prMemRing->ptrGuardSuffixAddr,
				prMemRing->u4Size, prMemRing->szFunc, prMemRing->i4Line);
			goto failexit;
		}

		if (memcmp((void *)((uintptr_t)((uintptr_t)prMemRing->pvVirtualMem - DMX_MEM_GUARD_PATTEN_LEN *
				sizeof(u8))), _au1guard_prefix, DMX_MEM_GUARD_PATTEN_LEN * sizeof(u8))) {
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MEM] Memory Prefix Guard ERROR, GuardPrefixAddr(0x%08x),")
				TEXT(" FreeAddr(0x%08x), GuardSuffixAddr(0x%08x), size(0x%08x),")
				TEXT(" Func(%s), line(%d)\r\n"),
				prMemRing->ptrGuardPrefixAddr, prMemRing->pvVirtualMem,
				prMemRing->ptrGuardSuffixAddr,
				prMemRing->u4Size, prMemRing->szFunc, prMemRing->i4Line);
			goto failexit;
		}

		if (memcmp((void *)(prMemRing->ptrGuardSuffixAddr), _au1guard_suffix,
				DMX_MEM_GUARD_PATTEN_LEN * sizeof(u8))) {
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MEM] Reserved Memory Suffix Guard ERROR, ")
				TEXT("GuardPrefixAddr(0x%08x), FreeAddr(0x%08x), ")
				TEXT("GuardSuffixAddr(0x%08x), size(0x%08x), ")
				TEXT("Func(%s), line(%d)\r\n"),
				prMemRing->ptrGuardPrefixAddr, prMemRing->pvVirtualMem,
				prMemRing->ptrGuardSuffixAddr,
				prMemRing->u4Size, prMemRing->szFunc, prMemRing->i4Line);
			goto failexit;
		}
	} else {
	}

	spin_unlock_irqrestore(&_DmxMemLock, flags);

	return TRUE;

failexit:

	spin_unlock_irqrestore(&_DmxMemLock, flags);

	OSE_PrintOSEMemoryCfg();

	return FALSE;

#endif /* DMX_MEM_GUARD_PATTEN_CHECK */

	return TRUE;
}

void DMX_CheckMemGuard(MEMORY_MAP_T *prMemRing)
{
#if DMX_MEM_GUARD_PATTEN_CHECK
	if ((NULL == prMemRing) ||
		(prMemRing->pvVirtualMem == NULL))
		return;

	if (prMemRing->eType == SPT_MEM_TYPE_OSEMALLOC) {
		DmxLogD(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
			TEXT("[MEM] Check Memory, GuardPrefixAddr(0x%08x), FreeAddr(0x%08x),")
			TEXT(" GuardSuffixAddr(0x%08x), size(0x%08x), Func(%s), line(%d)\r\n"),
			prMemRing->ptrGuardPrefixAddr, prMemRing->pvVirtualMem,
			prMemRing->ptrGuardSuffixAddr,
			prMemRing->u4Size, prMemRing->szFunc, prMemRing->i4Line);
		if (prMemRing->ptrGuardPrefixAddr + prMemRing->u4Align !=
			(uintptr_t)(prMemRing->pvVirtualMem)) {
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MEM] Memory ERROR, GuardPrefixAddr(0x%08x), FreeAddr(0x%08x),")
				TEXT(" GuardSuffixAddr(0x%08x), size(0x%08x), Func(%s), line(%d)\r\n"),
				prMemRing->ptrGuardPrefixAddr, prMemRing->pvVirtualMem,
				prMemRing->ptrGuardSuffixAddr,
				prMemRing->u4Size, prMemRing->szFunc, prMemRing->i4Line);
		}

		if (prMemRing->ptrGuardPrefixAddr + prMemRing->u4Align +
			DMX_MEM_Align(prMemRing->u4Size,
				prMemRing->u4Align) != prMemRing->ptrGuardSuffixAddr) {
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MEM] Memory ERROR, GuardPrefixAddr(0x%08x), FreeAddr(0x%08x),")
				TEXT(" GuardSuffixAddr(0x%08x), size(0x%08x), Func(%s), line(%d)\r\n"),
				prMemRing->ptrGuardPrefixAddr, prMemRing->pvVirtualMem,
				prMemRing->ptrGuardSuffixAddr,
				prMemRing->u4Size, prMemRing->szFunc, prMemRing->i4Line);
		}

		if (memcmp((void *)((uintptr_t)((uintptr_t)prMemRing->pvVirtualMem -
			DMX_MEM_GUARD_PATTEN_LEN * sizeof(u8))),
			_au1guard_prefix, DMX_MEM_GUARD_PATTEN_LEN * sizeof(u8))) {
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MEM] Memory Prefix Guard ERROR, GuardPrefixAddr(0x%08x), FreeAddr(0x%08x),")
				TEXT(" GuardSuffixAddr(0x%08x), size(0x%08x), Func(%s), line(%d)\r\n"),
				prMemRing->ptrGuardPrefixAddr, prMemRing->pvVirtualMem,
				prMemRing->ptrGuardSuffixAddr,
				prMemRing->u4Size, prMemRing->szFunc, prMemRing->i4Line);
		}

		if (memcmp((void *)(prMemRing->ptrGuardSuffixAddr), _au1guard_suffix,
				DMX_MEM_GUARD_PATTEN_LEN * sizeof(u8))) {
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MEM] Memory Suffix Guard ERROR, GuardPrefixAddr(0x%08x), FreeAddr(0x%08x),")
				TEXT(" GuardSuffixAddr(0x%08x), size(0x%08x), Func(%s), line(%d)\r\n"),
				prMemRing->ptrGuardPrefixAddr, prMemRing->pvVirtualMem,
				prMemRing->ptrGuardSuffixAddr,
				prMemRing->u4Size, prMemRing->szFunc, prMemRing->i4Line);
		}
	} else if (prMemRing->eType == SPT_MEM_TYPE_OSEPHY) {
		DmxLogD(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
			TEXT("[MEM] Check Reserved Memory, GuardPrefixAddr(0x%08x), FreeAddr(0x%08x),")
			TEXT(" GuardSuffixAddr(0x%08x), size(0x%08x), Func(%s), line(%d)\r\n"),
			prMemRing->ptrGuardPrefixAddr, prMemRing->pvVirtualMem,
			prMemRing->ptrGuardSuffixAddr,
			prMemRing->u4Size, prMemRing->szFunc, prMemRing->i4Line);
		if (prMemRing->ptrGuardPrefixAddr + prMemRing->u4Align !=
			(u32)(prMemRing->pvVirtualMem)) {
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MEM] Reserved Memory ERROR, GuardPrefixAddr(0x%08x), FreeAddr(0x%08x),")
				TEXT(" GuardSuffixAddr(0x%08x), size(0x%08x), Func(%s), line(%d)\r\n"),
				prMemRing->ptrGuardPrefixAddr, prMemRing->pvVirtualMem,
				prMemRing->ptrGuardSuffixAddr,
				prMemRing->u4Size, prMemRing->szFunc, prMemRing->i4Line);
		}

		if (prMemRing->ptrGuardPrefixAddr + prMemRing->u4Align +
			DMX_MEM_Align(prMemRing->u4Size,
				prMemRing->u4Align) != prMemRing->ptrGuardSuffixAddr) {
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MEM] Reserved Memory ERROR, GuardPrefixAddr(0x%08x), FreeAddr(0x%08x),")
				TEXT(" GuardSuffixAddr(0x%08x), size(0x%08x), Func(%s), line(%d)\r\n"),
				prMemRing->ptrGuardPrefixAddr, prMemRing->pvVirtualMem, prMemRing->ptrGuardSuffixAddr,
				prMemRing->u4Size, prMemRing->szFunc, prMemRing->i4Line);
		}

		if (memcmp((void *)((uintptr_t)((uintptr_t)prMemRing->pvVirtualMem -
			DMX_MEM_GUARD_PATTEN_LEN * sizeof(u8))),
			_au1guard_prefix, DMX_MEM_GUARD_PATTEN_LEN * sizeof(u8))) {
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MEM] Memory Prefix Guard ERROR, GuardPrefixAddr(0x%08x), FreeAddr(0x%08x),")
				TEXT(" GuardSuffixAddr(0x%08x), size(0x%08x), Func(%s), line(%d)\r\n"),
				prMemRing->ptrGuardPrefixAddr, prMemRing->pvVirtualMem,
				prMemRing->ptrGuardSuffixAddr,
				prMemRing->u4Size, prMemRing->szFunc, prMemRing->i4Line);
		}

		if (memcmp((void *)(prMemRing->ptrGuardSuffixAddr), _au1guard_suffix,
				DMX_MEM_GUARD_PATTEN_LEN * sizeof(u8))) {
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MEM] Reserved Memory Suffix Guard ERROR, GuardPrefixAddr(0x%08x),")
				TEXT(" FreeAddr(0x%08x), GuardSuffixAddr(0x%08x), size(0x%08x), Func(%s),")
				TEXT(" line(%d)\r\n"),
				prMemRing->ptrGuardPrefixAddr, prMemRing->pvVirtualMem,
				prMemRing->ptrGuardSuffixAddr,
				prMemRing->u4Size, prMemRing->szFunc, prMemRing->i4Line);
		}
	} else {
	}
#endif /* DMX_MEM_GUARD_PATTEN_CHECK */

}

MRESULT dmx_Mem_Init(void)
{
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT, TEXT("[MEM] %s line %d enter\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);

	/* Nothing todo. */
	_prMemMap = NULL;

#if DMX_MEM_GUARD_PATTEN_CHECK
	_prVMemMap = NULL;
#endif /* DMX_MEM_GUARD_PATTEN_CHECK */

	spin_lock_init(&_DmxMemLock);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
			TEXT("[MEM] %s line %d fail in DMX_MEM_CREATE_LOCK\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(mrRet);
	}

	DmxLogT(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT, TEXT("[MEM] %s line %d exit, success\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);

	MM_RETURN(mrRet);
}

void dmx_Mem_Uninit(void)
{
	MEMORY_MAP_T *prMM = NULL, *prNextMM = NULL;

	DmxLogT(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT, TEXT("[MEM] %s line %d enter\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);

	prMM = _prMemMap;

	while (NULL != prMM) {
		prNextMM = prMM->prNext;
#if DMX_MEM_PRINT_LINE_FUNCTION
		switch (prMM->eType) {
		case SPT_MEM_TYPE_OSEMALLOC:
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MemLeak] Unfree Memory, Phy:0x%x, Vir:0x%x, u4Size:%d,")
				TEXT(" eType: %d, szFunc: %s, i4Line: %d\r\n"),
				prMM->pvPhysicalMem, prMM->pvVirtualMem, prMM->u4Size,
				prMM->eType, prMM->szFunc, prMM->i4Line);
			#if DMX_VMALLOC_FROM_RESEVED_MEM
			DMX_FreeHwMemory(prMM->pvVirtualMem);
			#else
			DMX_FreeMemory(prMM->pvVirtualMem);
			#endif
			break;
		case SPT_MEM_TYPE_OSEPHY:
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MemLeak] Unfree Memory, Phy:0x%x, Vir:0x%x, u4Size:%d,")
				TEXT(" eType: %d, szFunc: %s, i4Line: %d\r\n"),
				prMM->pvPhysicalMem, prMM->pvVirtualMem,
				prMM->u4Size, prMM->eType, prMM->szFunc, prMM->i4Line);
			DMX_FreeHwMemory(prMM->pvVirtualMem);
			break;
		case SPT_MEM_TYPE_EXT:
			/* do nothing */
			break;
		default:
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MemLeak] Unfree Memory, Phy:0x%x, Vir:0x%x, u4Size:%d,")
				TEXT(" eType: %d, szFunc: %s, i4Line: %d\r\n"),
				prMM->pvPhysicalMem, prMM->pvVirtualMem,
				prMM->u4Size, prMM->eType, prMM->szFunc, prMM->i4Line);
			break;
		}

#else /* DMX_MEM_PRINT_LINE_FUNCTION */
		switch (prMM->eType) {
		case SPT_MEM_TYPE_OSEMALLOC:
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MemLeak] Unfree Memory, Phy:0x%x, Vir:0x%x, ")
				TEXT("u4Size:%d, eType: %d\r\n"),
				prMM->pvPhysicalMem, prMM->pvVirtualMem,
				prMM->u4Size, prMM->eType);
			#if DMX_VMALLOC_FROM_RESEVED_MEM
			DMX_FreeHwMemory(prMM->pvVirtualMem);
			#else
			DMX_FreeMemory(prMM->pvVirtualMem);
			#endif
			break;
		case SPT_MEM_TYPE_OSEPHY:
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MemLeak] Unfree Memory, Phy:0x%x, Vir:0x%x, ")
				TEXT("u4Size:%d, eType: %d\r\n"),
				prMM->pvPhysicalMem, prMM->pvVirtualMem,
				prMM->u4Size, prMM->eType);
			DMX_FreeHwMemory(prMM->pvVirtualMem);
			break;
		case SPT_MEM_TYPE_EXT:
			/* do nothing */
			break;
		default:
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MemLeak] Unfree Memory, Phy:0x%x, Vir:0x%x, ")
				TEXT("u4Size:%d, eType: %d\r\n"),
				prMM->pvPhysicalMem, prMM->pvVirtualMem,
				prMM->u4Size, prMM->eType);
			break;
		}

#endif /* DMX_MEM_PRINT_LINE_FUNCTION */

		MM_FREE(prMM);
		prMM = prNextMM;
	}

	_prMemMap = NULL;

#if DMX_MEM_GUARD_PATTEN_CHECK
	prMM = _prVMemMap;
	while (NULL != prMM) {
		prNextMM = prMM->prNext;

		switch (prMM->eType) {
		case SPT_MEM_TYPE_OSEMALLOC:
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MemLeak] Unfree Memory, Phy:0x%x, Vir:0x%x, u4Size:%d,")
				TEXT(" eType: %d, szFunc: %s, i4Line: %d\r\n"),
				prMM->pvPhysicalMem, prMM->pvVirtualMem,
				prMM->u4Size, prMM->eType,
				prMM->szFunc, prMM->i4Line);
			MM_FREE(prMM->pvVirtualMem);
			break;
		default:
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MemLeak] Unfree Memory, Phy:0x%x, Vir:0x%x, u4Size:%d,")
				TEXT(" eType: %d, szFunc: %s, i4Line: %d\r\n"),
				prMM->pvPhysicalMem, prMM->pvVirtualMem,
				prMM->u4Size, prMM->eType,
				prMM->szFunc, prMM->i4Line);
			break;
		}
		MM_FREE(prMM);
		prMM = prNextMM;
	}
	_prVMemMap = NULL;
#endif /* DMX_MEM_GUARD_PATTEN_CHECK */

	DmxLogT(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
		TEXT("[MEM] %s line %d exit, success\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);

}

void DMX_Dump_Mem(void)
{
#if DMX_MEM_PRINT_LINE_FUNCTION
	MEMORY_MAP_T *prMM = NULL, *prNextMM = NULL;
	unsigned long flags = 0;

	spin_lock_irqsave(&_DmxMemLock, flags);

	prMM = _prMemMap;

	while (NULL != prMM) {
		prNextMM = prMM->prNext;
		switch (prMM->eType) {
		case SPT_MEM_TYPE_OSEMALLOC:
			DmxLogT(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[DMX] MEM -- Phy:0x%x, Vir:0x%x, u4Size:%d,")
				TEXT(" eType: %d, szFunc: %s, i4Line: %d\r\n"),
				prMM->pvPhysicalMem, prMM->pvVirtualMem,
				prMM->u4Size, prMM->eType,
				prMM->szFunc, prMM->i4Line);
			DMX_CheckMemGuard(prMM);
			break;
		case SPT_MEM_TYPE_OSEPHY:
			DmxLogT(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[DMX] MEM -- Phy:0x%x, Vir:0x%x, u4Size:%d,")
				TEXT(" eType: %d, szFunc: %s, i4Line: %d\r\n"),
				prMM->pvPhysicalMem, prMM->pvVirtualMem,
				prMM->u4Size, prMM->eType,
			   prMM->szFunc, prMM->i4Line);
			DMX_CheckMemGuard(prMM);
			break;
		case SPT_MEM_TYPE_EXT:
			/* do nothing */
			DmxLogT(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[DMX] MEM -- Audio Memory, Phy:0x%x, Vir:0x%x, u4Size:%d,")
				TEXT(" eType: %d, szFunc: %s, i4Line: %d\r\n"),
				prMM->pvPhysicalMem, prMM->pvVirtualMem,
				prMM->u4Size, prMM->eType,
			   prMM->szFunc, prMM->i4Line);
			DMX_CheckMemGuard(prMM);
			break;
		default:
			DmxLogT(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[DMX] MEM -- Phy:0x%x, Vir:0x%x, u4Size:%d,")
				TEXT(" eType: %d, szFunc: %s, i4Line: %d\r\n"),
				prMM->pvPhysicalMem, prMM->pvVirtualMem,
				prMM->u4Size, prMM->eType,
				prMM->szFunc, prMM->i4Line);
			DMX_CheckMemGuard(prMM);
			break;
		}

		prMM = prNextMM;
	}

#if DMX_MEM_GUARD_PATTEN_CHECK
	prMM = _prVMemMap;

	while (NULL != prMM) {
		prNextMM = prMM->prNext;
		switch (prMM->eType) {
		case SPT_MEM_TYPE_OSEMALLOC:
			DmxLogT(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[DMX] MEM -- Phy:0x%x, Vir:0x%x, u4Size:%d,")
				TEXT(" eType: %d, szFunc: %s, i4Line: %d\r\n"),
				prMM->pvPhysicalMem, prMM->pvVirtualMem,
				prMM->u4Size, prMM->eType,
				prMM->szFunc, prMM->i4Line);
			DMX_CheckMemGuard(prMM);
			break;
		default:
			DmxLogT(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[DMX] MEM -- Phy:0x%x, Vir:0x%x, u4Size:%d,")
				TEXT(" eType: %d, szFunc: %s, i4Line: %d\r\n"),
				prMM->pvPhysicalMem, prMM->pvVirtualMem,
				prMM->u4Size, prMM->eType,
				prMM->szFunc, prMM->i4Line);
			DMX_CheckMemGuard(prMM);
			break;
		}

		prMM = prNextMM;
	}
#endif /* DMX_MEM_GUARD_PATTEN_CHECK */

	spin_unlock_irqrestore(&_DmxMemLock, flags);

#endif /* DMX_MEM_PRINT_LINE_FUNCTION */
}

#if DMX_MEM_PRINT_LINE_FUNCTION
MRESULT Add_MemMap(void *pvVMem, void *pvPMem,
	E_SPT_MEM_TYPE_T eType, u32 u4Size, u32 u4Align,
	const char *szFunc, s32 i4Line)
#else
MRESULT Add_MemMap(void *pvVMem, void *pvPMem,
	E_SPT_MEM_TYPE_T eType, u32 u4Size)
#endif
{
	MEMORY_MAP_T *prMM = NULL, *prRing = NULL;
#if DMX_MEM_PRINT_LINE_FUNCTION
	u32 u4FuncNameSz = 0;
#endif /* DMX_MEM_PRINT_LINE_FUNCTION */
	unsigned long flags = 0;

	prMM = (MEMORY_MAP_T *)MM_ALLOC(sizeof(MEMORY_MAP_T));

	if (NULL == prMM) {
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
			TEXT("[MEM] %s fail in dmx_mem_alloc alloc map node\r\n"),
			DMX_FUNC_NAME);

		DMX_ASSERT(FALSE);

		MM_RETURN(RET_DMX_NO_MEM);
	}

	spin_lock_irqsave(&_DmxMemLock, flags);

	prMM->eType  = eType;
	prMM->u4Size = u4Size;

	prMM->pvVirtualMem	= pvVMem;
	prMM->pvPhysicalMem = pvPMem;

	switch (eType) {
	case SPT_MEM_TYPE_EXT: {
#if DMX_MEM_PRINT_LINE_FUNCTION
#if DMX_MEM_GUARD_PATTEN_CHECK
			prMM->u4Align = u4Align;
			prMM->ptrGuardPrefixAddr = 0;
			prMM->ptrGuardSuffixAddr = 0;
#endif /* DMX_MEM_GUARD_PATTEN_CHECK */
#endif /* DMX_MEM_PRINT_LINE_FUNCTION */
	   }
		break;
	case SPT_MEM_TYPE_OSEMALLOC:
	case SPT_MEM_TYPE_OSEPHY: {
#if DMX_MEM_PRINT_LINE_FUNCTION
#if DMX_MEM_GUARD_PATTEN_CHECK
			DMX_ASSERT(u4Align >= DMX_MEM_GUARD_PATTEN_LEN * sizeof(u8));
			prMM->u4Align = u4Align;
			prMM->pvVirtualMem	= (void *)(((uintptr_t)pvVMem) + u4Align);
			prMM->pvPhysicalMem = (void *)(((uintptr_t)pvPMem) + u4Align);

			prMM->ptrGuardPrefixAddr = (uintptr_t)pvVMem;
			prMM->ptrGuardSuffixAddr =
				(uintptr_t)pvVMem + u4Align + DMX_MEM_Align(u4Size, u4Align);
			memcpy((void *)(prMM->ptrGuardPrefixAddr), _au1guard_prefix,
				DMX_MEM_GUARD_PATTEN_LEN * sizeof(u8));
			memcpy((void *)((uintptr_t)((uintptr_t)(prMM->pvVirtualMem) -
				DMX_MEM_GUARD_PATTEN_LEN * sizeof(u8))),
				_au1guard_prefix, DMX_MEM_GUARD_PATTEN_LEN * sizeof(u8));
			memcpy((void *)(prMM->ptrGuardSuffixAddr), _au1guard_suffix,
				DMX_MEM_GUARD_PATTEN_LEN * sizeof(u8));
#endif /* DMX_MEM_GUARD_PATTEN_CHECK */
#endif /* DMX_MEM_PRINT_LINE_FUNCTION */
		}
		break;
	default:
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
			TEXT("[MEM] %s line %d fail in invalid eType(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eType);

		MM_FREE(prMM);

		spin_unlock_irqrestore(&_DmxMemLock, flags);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prMM->prNext = NULL;

#if DMX_MEM_PRINT_LINE_FUNCTION
	mm_memset(prMM->szFunc, 0, sizeof(char) * MAX_OF_DMX_FUNC_NAME_LEN);

	u4FuncNameSz = DMX_MIN(strlen(szFunc), MAX_OF_DMX_FUNC_NAME_LEN - 1);
	strncpy(prMM->szFunc, szFunc, u4FuncNameSz);

	prMM->i4Line = i4Line;

	DmxLogT(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
		TEXT("[MEM] %s line %d -- alloc Mem, (VMem: 0x%08x, Size: 0x%08x),")
		TEXT(" Func(%s), line(%d)\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO,
		(u32)(prMM->pvVirtualMem), prMM->u4Size,
		prMM->szFunc, prMM->i4Line);
#endif /* DMX_MEM_PRINT_LINE_FUNCTION */

	if (NULL == _prMemMap) {
		_prMemMap = prMM;

		spin_unlock_irqrestore(&_DmxMemLock, flags);
		MM_RETURN(RET_DMX_OK);
	}

	prRing = _prMemMap;

	while (NULL != prRing->prNext)
		prRing = prRing->prNext;

	prRing->prNext = prMM;

	spin_unlock_irqrestore(&_DmxMemLock, flags);

	MM_RETURN(RET_DMX_OK);
}

void Del_MemMap(void *pvVMem)
{
	MEMORY_MAP_T *prPrev = NULL, *prRing = NULL;
#if DMX_MEM_GUARD_PATTEN_CHECK
	void *pvGuardPrefixAddr = NULL;
#endif /* DMX_MEM_GUARD_PATTEN_CHECK */
	unsigned long flags = 0;

	spin_lock_irqsave(&_DmxMemLock, flags);

	prRing = _prMemMap;

	if (NULL == prRing) {
		DmxLogT(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
			TEXT("[MEM] %s line %d -- no MemMap\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		spin_unlock_irqrestore(&_DmxMemLock, flags);
		return;
	}

	while ((NULL != prRing) &&
		(prRing->pvVirtualMem != pvVMem)) {
		prPrev = prRing;
		prRing = prRing->prNext;
	}

	/* Delete the first Notes. */
	if (NULL == prPrev)
		_prMemMap = _prMemMap->prNext;
	else if (NULL != prRing)
		prPrev->prNext = prRing->prNext;

	if (NULL != prRing) {
		prRing->prNext = NULL;

#if DMX_MEM_PRINT_LINE_FUNCTION
		DmxLogD(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
			TEXT("[MEM] %s line %d -- free Mem, (VMem: 0x%08x, Size: 0x%08x),")
			TEXT(" Func(%s), line(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			(uintptr_t)(prRing->pvVirtualMem), prRing->u4Size,
			prRing->szFunc, prRing->i4Line);
#endif

#if DMX_MEM_GUARD_PATTEN_CHECK
		pvGuardPrefixAddr = (void *)(prRing->ptrGuardPrefixAddr);
#endif

		DMX_CheckMemGuard(prRing);

		MM_FREE(prRing);
	}

	spin_unlock_irqrestore(&_DmxMemLock, flags);

#if DMX_MEM_GUARD_PATTEN_CHECK
	if (pvGuardPrefixAddr != NULL)
		OSE_MemFreeCustom(OSE_DEMUXER, pvGuardPrefixAddr);
#endif

	return;

}


static uintptr_t GetVMemFromPMem(uintptr_t ptrPMem)
{
	MEMORY_MAP_T *prRing = NULL;

	prRing = _prMemMap;

	while (NULL != prRing) {
		if ((uintptr_t)(prRing->pvPhysicalMem) == ptrPMem)
			return (uintptr_t)(prRing->pvVirtualMem);
		prRing = prRing->prNext;
	}

	return 0;
}


static uintptr_t GetPMemFromVMem(uintptr_t ptrVMem)
{
	MEMORY_MAP_T *prRing = NULL;

	prRing = _prMemMap;

	while (NULL != prRing) {
		if ((uintptr_t)(prRing->pvVirtualMem) == ptrVMem)
			return (uintptr_t)(prRing->pvPhysicalMem);

		prRing = prRing->prNext;
	}

	return 0;
}

#if DMX_MEM_PRINT_LINE_FUNCTION

uintptr_t DMX_PHYSICALEX(uintptr_t ptrVMem, const char *szFunc, s32 i4Line)
{
	MEMORY_MAP_T *prRing;
	MEMORY_MAP_T *pFindRing;
	u32 u4MinOfst;
	void *pvPMem = NULL;
	unsigned long flags = 0;

	spin_lock_irqsave(&_DmxMemLock, flags);

	pvPMem = (void *)GetPMemFromVMem(ptrVMem);

	if (NULL != pvPMem) {
		spin_unlock_irqrestore(&_DmxMemLock, flags);
		return (u32)(pvPMem);
	}

	prRing = _prMemMap;
	pFindRing = NULL;
	u4MinOfst = DMX_INVALID_UINT32;
	while (NULL != prRing) {
		if ((uintptr_t)(prRing->pvVirtualMem) < ptrVMem) {
			if (ptrVMem - (uintptr_t)(prRing->pvVirtualMem) < u4MinOfst) {
				u4MinOfst = ptrVMem - (uintptr_t)(prRing->pvVirtualMem);
				pFindRing = prRing;
			}
		}
		prRing = prRing->prNext;
	}

	if (NULL != pFindRing) {
		u4MinOfst = (uintptr_t)(pFindRing->pvPhysicalMem) + u4MinOfst;

		spin_unlock_irqrestore(&_DmxMemLock, flags);
		return u4MinOfst;
	}

	DMX_ASSERT(FALSE);

	spin_unlock_irqrestore(&_DmxMemLock, flags);
	return 0;
}


uintptr_t DMX_NONCACHEEX(uintptr_t ptrPMem, const char *szFunc, s32 i4Line)
{
	MEMORY_MAP_T *prRing;
	MEMORY_MAP_T *pFindRing;
	u32		  u4MinOfst;
	void		 *pvVMem = NULL;
	unsigned long flags = 0;

	spin_lock_irqsave(&_DmxMemLock, flags);

	pvVMem = (void *)GetVMemFromPMem(ptrPMem);

	if (NULL != pvVMem) {
		spin_unlock_irqrestore(&_DmxMemLock, flags);
		return (uintptr_t)(pvVMem);
	}

	prRing = _prMemMap;
	pFindRing = NULL;
	u4MinOfst = DMX_INVALID_UINT32;
	while (NULL != prRing) {
		if ((uintptr_t)(prRing->pvPhysicalMem) < ptrPMem) {
			if (ptrPMem - (uintptr_t)(prRing->pvPhysicalMem) < u4MinOfst) {
				u4MinOfst = ptrPMem - (uintptr_t)(prRing->pvPhysicalMem);
				pFindRing = prRing;
			}
		}
		prRing = prRing->prNext;
	}

	if (NULL != pFindRing) {
		u4MinOfst = (uintptr_t)(pFindRing->pvVirtualMem) + u4MinOfst;

		spin_unlock_irqrestore(&_DmxMemLock, flags);
		return u4MinOfst;
	}

	DMX_ASSERT(FALSE);
	spin_unlock_irqrestore(&_DmxMemLock, flags);

	return 0;
}
#else

u32 DMX_PHYSICAL(uintptr_t ptrVMem)
{
	MEMORY_MAP_T *prRing;
	MEMORY_MAP_T *pFindRing;
	u32 u4MinOfst;
	void *pvPMem = NULL;
	unsigned long flags = 0;

	spin_lock_irqsave(&_DmxMemLock, flags);

	pvPMem = (void *)GetPMemFromVMem(ptrVMem);

	if (NULL != pvPMem) {
		spin_unlock_irqrestore(&_DmxMemLock, flags);
		return (u32)(pvPMem);
	}

	prRing = _prMemMap;
	pFindRing = NULL;
	u4MinOfst = DMX_INVALID_UINT32;
	while (NULL != prRing) {
		if ((uintptr_t)(prRing->pvVirtualMem) < ptrVMem) {
			if (ptrVMem - (uintptr_t)(prRing->pvVirtualMem) < u4MinOfst) {
				u4MinOfst = ptrVMem - (uintptr_t)(prRing->pvVirtualMem);
				pFindRing = prRing;
			}
		}
		prRing = prRing->prNext;
	}

	if (NULL != pFindRing) {
		u4MinOfst = (u32)(pFindRing->pvPhysicalMem) + u4MinOfst;
		spin_unlock_irqrestore(&_DmxMemLock, flags);
		return u4MinOfst;
	}

	DMX_ASSERT(FALSE);

	spin_unlock_irqrestore(&_DmxMemLock, flags);
	return 0;
}


uintptr_t DMX_NONCACHE(uintptr_t ptrPMem)
{
	MEMORY_MAP_T *prRing;
	MEMORY_MAP_T *pFindRing;
	u32		  u4MinOfst;
	void		 *pvVMem = NULL;
	unsigned long flags = 0;

	spin_lock_irqsave(&_DmxMemLock, flags);

	pvVMem = (void *)GetVMemFromPMem(ptrPMem);

	if (NULL != pvVMem) {
		spin_unlock_irqrestore(&_DmxMemLock, flags);
		return (uintptr_t)(pvVMem);
	}

	prRing = _prMemMap;
	pFindRing = NULL;
	u4MinOfst = DMX_INVALID_UINT32;
	while (NULL != prRing) {
		if ((uintptr_t)(prRing->pvPhysicalMem) < ptrPMem) {
			if (ptrPMem - (uintptr_t)(prRing->pvPhysicalMem) < u4MinOfst) {
				u4MinOfst = ptrPMem - (uintptr_t)(prRing->pvPhysicalMem);
				pFindRing = prRing;
			}
		}
		prRing = prRing->prNext;
	}

	if (NULL != pFindRing) {
		u4MinOfst = (uintptr_t)(pFindRing->pvVirtualMem) + u4MinOfst;
		spin_unlock_irqrestore(&_DmxMemLock, flags);
		return u4MinOfst;
	}

	DMX_ASSERT(FALSE);

	spin_unlock_irqrestore(&_DmxMemLock, flags);

	return 0;
}

#endif /* DMX_MEM_PRINT_LINE_FUNCTION */


#if DMX_MEM_PRINT_LINE_FUNCTION

bool dmx_memcheck_inlist(void *pvDst, u32 u4Size,
	const char *szFunc, s32 i4Line)
{
	MEMORY_MAP_T *prRing = NULL, *pFindVRing = NULL, *pFindRevRing = NULL;
	u32 u4VMinOfst = 0, u4RevMinOfst = 0;

	if (NULL == pvDst) {
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
			TEXT("[MEM] %s line %d fail for pvDst is NULL, Caller(%s line %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, szFunc, i4Line);
		return FALSE;
	}

	if (NULL == szFunc) {
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
			TEXT("[MEM] %s line %d fail for szFunc is NULL\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return FALSE;
	}

	if (0 == u4Size)
		return FALSE;

	pFindVRing = _prVMemMap;

	while (NULL != pFindVRing) {
		if (pFindVRing->pvVirtualMem == pvDst)
			break;
		pFindVRing = pFindVRing->prNext;
	}

	if (pFindVRing != NULL) {
		if (pFindVRing->u4Size < u4Size) {
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MEM] %s line %d fail for pvDst's u4Size(%d)")
				TEXT(" exceed the VMem(0x%x)'s Size(%d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Size,
				pFindVRing, pFindVRing->u4Size);
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MEM] %s line %d fail -- Caller(%s line %d),")
				TEXT(" VMem(0x%x)'Allocator(%s line %d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, szFunc, i4Line,
				pFindVRing, pFindVRing->szFunc, pFindVRing->i4Line);
			return FALSE;
		}
		return TRUE;
	}

	prRing = _prVMemMap;
	u4VMinOfst = DMX_INVALID_UINT32;
	while (NULL != prRing) {
		if ((uintptr_t)(prRing->pvVirtualMem) < (uintptr_t)pvDst) {
			if ((uintptr_t)pvDst - (uintptr_t)(prRing->pvVirtualMem) < u4VMinOfst) {
				u4VMinOfst = (uintptr_t)pvDst - (uintptr_t)(prRing->pvVirtualMem);
				pFindVRing = prRing;
			}
		}
		prRing = prRing->prNext;
	}

	pFindRevRing = _prMemMap;

	while (NULL != pFindRevRing) {
		if (pFindRevRing->pvVirtualMem == pvDst)
			break;
		pFindRevRing = pFindRevRing->prNext;
	}

	if (pFindRevRing != NULL) {
		if (pFindRevRing->u4Size < u4Size) {
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MEM] %s line %d fail for pvDst's u4Size(%d)")
				TEXT(" exceed the ReserveVMem(0x%x)'s Size(%d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Size,
				pFindRevRing, pFindRevRing->u4Size);
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MEM] %s line %d fail -- Caller(%s line %d),")
				TEXT(" ReserveVMem(0x%x)'Allocator(%s line %d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, szFunc, i4Line,
				pFindRevRing, pFindRevRing->szFunc, pFindRevRing->i4Line);
			DMX_Dump_Mem();
			return FALSE;
		}
		return TRUE;
	}

	prRing = _prMemMap;
	u4RevMinOfst = DMX_INVALID_UINT32;
	while (NULL != prRing) {
		if ((uintptr_t)(prRing->pvVirtualMem) < (uintptr_t)pvDst) {
			if ((uintptr_t)pvDst - (uintptr_t)(prRing->pvVirtualMem) < u4RevMinOfst) {
				u4RevMinOfst = (uintptr_t)pvDst - (uintptr_t)(prRing->pvVirtualMem);
				pFindRevRing = prRing;
			}
		}
		prRing = prRing->prNext;
	}

	if ((u4RevMinOfst != DMX_INVALID_UINT32) &&
		(u4VMinOfst != DMX_INVALID_UINT32)) {
		if (u4VMinOfst < u4RevMinOfst) {
			if (NULL == pFindVRing) {
				DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
					TEXT("[MEM] %s line %d fail -- Caller(%s line %d)")
					TEXT(" isn't in VMem list and ReservedMem List\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, szFunc, i4Line);
				DMX_Dump_Mem();
				return FALSE;
			}
			if ((uintptr_t)(pFindVRing->pvVirtualMem) +
				pFindVRing->u4Size < (uintptr_t)pvDst + u4Size) {
				DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
					TEXT("[MEM] %s line %d fail for pvDst(0x%lx)'s u4Size(%d)")
					TEXT(" exceed the VMem(0x%x)'s Size(%d)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, (u32)pvDst, u4Size,
					pFindVRing, pFindVRing->u4Size);
				DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
					TEXT("[MEM] %s line %d fail -- Caller(%s line %d),")
					TEXT(" VMem(0x%x)'Allocator(%s line %d)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, szFunc, i4Line,
					pFindVRing, pFindVRing->szFunc, pFindVRing->i4Line);
				DMX_Dump_Mem();
				return FALSE;
			}
			return TRUE;
		} else if (u4RevMinOfst < u4VMinOfst) {
			if (NULL == pFindRevRing) {
				DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
					TEXT("[MEM] %s line %d fail -- Caller(%s line %d)")
					TEXT(" isn't in VMem list and ReservedMem List\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, szFunc, i4Line);
				DMX_Dump_Mem();
				return FALSE;
			}
			if ((uintptr_t)(pFindRevRing->pvVirtualMem) +
				pFindRevRing->u4Size < (uintptr_t)pvDst + u4Size) {
				DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
					TEXT("[MEM] %s line %d fail for pvDst(0x%lx)'s u4Size(%d)")
					TEXT(" exceed the ReserveVMem(0x%x)'s Size(%d)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, (u32)pvDst, u4Size,
					pFindRevRing->pvVirtualMem, pFindRevRing->u4Size);
				DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
					TEXT("[MEM] %s line %d fail -- Caller(%s line %d),")
					TEXT(" ReserveVMem(0x%x)'Allocator(%s line %d)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, szFunc, i4Line,
					pFindRevRing->pvVirtualMem, pFindRevRing->szFunc, pFindRevRing->i4Line);
				DMX_Dump_Mem();
				return FALSE;
			}
			return TRUE;
		} else if ((pFindRevRing != NULL) &&
			((pFindVRing != NULL))) {
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MEM] %s line %d fail for pvDst(0x%lx) in ")
				TEXT("ReserveVMem(0x%lx)[0x%lx, 0x%lx),")
				TEXT(" also in VMem(0x%lx)[0x%lx, 0x%lx)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, (uintptr_t)pvDst,
				(uintptr_t)pFindRevRing, (uintptr_t)(pFindRevRing->pvVirtualMem),
				(uintptr_t)(pFindRevRing->pvVirtualMem) + pFindRevRing->u4Size,
				(uintptr_t)pFindVRing, (uintptr_t)(pFindVRing->pvVirtualMem),
				(uintptr_t)(pFindVRing->pvVirtualMem) + pFindVRing->u4Size);
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MEM] %s line %d fail -- Caller(%s line %d),")
				TEXT(" ReserveVMem(0x%x)'Allocator(%s line %d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, szFunc, i4Line,
				pFindRevRing, pFindRevRing->szFunc, pFindRevRing->i4Line);
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MEM] %s line %d fail -- Caller(%s line %d),")
				TEXT(" VMem(0x%x)'Allocator(%s line %d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, szFunc, i4Line,
				pFindVRing, pFindVRing->szFunc, pFindVRing->i4Line);
			DMX_Dump_Mem();
			return FALSE;
		}
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
			TEXT("[MEM] %s line %d fail for pvDst(0x%lx)")
			TEXT(" isn't in VMem List and ReservedMemory List\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, (uintptr_t)pvDst);
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
			TEXT("[MEM] %s line %d fail -- Caller(%s line %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, szFunc, i4Line);
		return FALSE;
	} else if (u4RevMinOfst != DMX_INVALID_UINT32) {
		if (NULL == pFindRevRing) {
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MEM] %s line %d fail -- Caller(%s line %d)")
				TEXT(" isn't in VMem list and ReservedMem List\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, szFunc, i4Line);
			DMX_Dump_Mem();
			return FALSE;
		}

		if ((uintptr_t)(pFindRevRing->pvVirtualMem) +
			pFindRevRing->u4Size < (uintptr_t)pvDst + u4Size) {
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MEM] %s line %d fail for pvDst(0x%lx)'s u4Size(%d)")
				TEXT(" exceed the ReserveVMem(0x%x)'s Size(%d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, (uintptr_t)pvDst, u4Size,
				pFindRevRing, pFindRevRing->u4Size);
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MEM] %s line %d fail -- Caller(%s line %d),")
				TEXT(" ReserveVMem(0x%x)'Allocator(%s line %d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, szFunc, i4Line,
				pFindRevRing, pFindRevRing->szFunc, pFindRevRing->i4Line);
			DMX_Dump_Mem();
			return FALSE;
		}
	} else if (u4VMinOfst != DMX_INVALID_UINT32) {
		if (NULL == pFindVRing) {
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MEM] %s line %d fail -- Caller(%s line %d)")
				TEXT(" isn't in VMem list and ReservedMem List\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, szFunc, i4Line);
			DMX_Dump_Mem();
			return FALSE;
		}

		if ((uintptr_t)(pFindVRing->pvVirtualMem) +
			pFindVRing->u4Size < (uintptr_t)pvDst + u4Size) {
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MEM] %s line %d fail for pvDst(0x%lx)'s")
				TEXT(" u4Size(%d) exceed the VMem(0x%x)'s Size(%d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, (uintptr_t)pvDst, u4Size,
				pFindVRing, pFindVRing->u4Size);
			DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
				TEXT("[MEM] %s line %d fail -- Caller(%s line %d),")
				TEXT(" VMem(0x%x)'Allocator(%s line %d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, szFunc, i4Line,
				pFindVRing, pFindVRing->szFunc, pFindVRing->i4Line);
			DMX_Dump_Mem();
			return FALSE;
		}
	} else {
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
			TEXT("[MEM] %s line %d fail for pvDst(0x%lx) isn't in VMem List")
			TEXT(" and ReservedMemory List\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, (uintptr_t)pvDst);
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
			TEXT("[MEM] %s line %d fail -- Caller(%s line %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, szFunc, i4Line);
		DMX_Dump_Mem();
		return FALSE;
	}

	return TRUE;
}

void dmx_memsetex(void *pvDst, u8 u1Value, u32 u4Size,
	const char *szFunc, s32 i4Line)
{
	unsigned long flags = 0;
	u8 *p1Dst = (u8 *)pvDst;
	u32 i = 0;

	spin_lock_irqsave(&_DmxMemLock, flags);

	if (!dmx_memcheck_inlist(pvDst, u4Size, szFunc, i4Line)) {
		spin_unlock_irqrestore(&_DmxMemLock, flags);
		return;
	}

	if (u4Size > 0)
		mm_memset(pvDst, u1Value, u4Size);

	if (u1Value != 0)
		for (i = 0; i < u4Size; i++)
			p1Dst[i] = u1Value;

	spin_unlock_irqrestore(&_DmxMemLock, flags);
}

void dmx_memcpyex(void *pvDst, void *pvSrc, u32 u4Size,
	const char *szFunc, s32 i4Line)
{
	unsigned long flags = 0;

	spin_lock_irqsave(&_DmxMemLock, flags);

	if (!dmx_memcheck_inlist(pvDst, u4Size, szFunc, i4Line)) {
		spin_unlock_irqrestore(&_DmxMemLock, flags);
		return;
	}

	if (u4Size > 0)
		mm_memcpy(pvDst, pvSrc, u4Size);

	spin_unlock_irqrestore(&_DmxMemLock, flags);
}

#if DMX_MEM_GUARD_PATTEN_CHECK

MRESULT Add_VMemMap(void *pvVMem, void *pvPMem,
	E_SPT_MEM_TYPE_T eType, u32 u4Size, u32 u4Align,
	const char *szFunc, s32 i4Line)
{
	MEMORY_MAP_T *prMM = NULL, *prRing = NULL;
	u32 u4FuncNameSz = 0;
	unsigned long flags = 0;

	/* here we should use malloc, not dmx_mem_alloc
	   because dmx_mem_alloc will call this function, this cause dead-circularly-call process */
	prMM = (MEMORY_MAP_T *)MM_ALLOC(sizeof(MEMORY_MAP_T));

	if (NULL == prMM) {
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
			TEXT("[MEM] %s fail in dmx_mem_alloc alloc map node\r\n"),
			DMX_FUNC_NAME);

		DMX_ASSERT(FALSE);

		MM_RETURN(RET_DMX_NO_MEM);
	}

	spin_lock_irqsave(&_DmxMemLock, flags);
	prMM->eType  = eType;
	prMM->u4Size = u4Size;
	prMM->pvVirtualMem	= (void *)(((uintptr_t)pvVMem) + u4Align);
	prMM->pvPhysicalMem = NULL;
	prMM->prNext = NULL;

#if DMX_MEM_PRINT_LINE_FUNCTION
#if DMX_MEM_GUARD_PATTEN_CHECK
	prMM->u4Align = u4Align;
	prMM->ptrGuardPrefixAddr = (uintptr_t)pvVMem;
	prMM->ptrGuardSuffixAddr =
		(uintptr_t)pvVMem + u4Align + DMX_MEM_Align(u4Size, u4Align);
	DMX_ASSERT(prMM->u4Align >= DMX_MEM_GUARD_PATTEN_LEN * sizeof(u8));
	memcpy((void *)(prMM->ptrGuardPrefixAddr), _au1guard_prefix,
		DMX_MEM_GUARD_PATTEN_LEN * sizeof(u8));
	memcpy((void *)((uintptr_t)((uintptr_t)(prMM->pvVirtualMem) -
		DMX_MEM_GUARD_PATTEN_LEN * sizeof(u8))),
		_au1guard_prefix, DMX_MEM_GUARD_PATTEN_LEN * sizeof(u8));
	memcpy((void *)(prMM->ptrGuardSuffixAddr), _au1guard_suffix,
		DMX_MEM_GUARD_PATTEN_LEN * sizeof(u8));
#endif /* DMX_MEM_GUARD_PATTEN_CHECK */
#endif /* DMX_MEM_PRINT_LINE_FUNCTION */

	mm_memset(prMM->szFunc, 0, sizeof(char) * MAX_OF_DMX_FUNC_NAME_LEN);
	u4FuncNameSz = DMX_MIN(strlen(szFunc), MAX_OF_DMX_FUNC_NAME_LEN - 1);
	strncpy(prMM->szFunc, szFunc, u4FuncNameSz);
	prMM->i4Line = i4Line;

	DmxLogD(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
		TEXT("[MEM] %s line %d -- alloc Mem, (VMem: 0x%08x, Size: 0x%08x),")
		TEXT(" Func(%s), line(%d)\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, (uintptr_t)(prMM->pvVirtualMem),
		prMM->u4Size, prMM->szFunc, prMM->i4Line);

	if (NULL == _prVMemMap) {
		_prVMemMap = prMM;

		spin_unlock_irqrestore(&_DmxMemLock, flags);
		MM_RETURN(RET_DMX_OK);
	}

	prRing = _prVMemMap;

	while (NULL != prRing->prNext)
		prRing = prRing->prNext;

	prRing->prNext = prMM;

	spin_unlock_irqrestore(&_DmxMemLock, flags);

	MM_RETURN(RET_DMX_OK);
}

void Del_VMemMap(void *pvVMem)
{
	MEMORY_MAP_T *prPrev = NULL, *prRing = NULL;
	unsigned long flags = 0;
#if DMX_MEM_GUARD_PATTEN_CHECK
	void *pvGuardPrefixAddr = NULL;
#endif /* DMX_MEM_GUARD_PATTEN_CHECK */

	spin_lock_irqsave(&_DmxMemLock, flags);
	prRing = _prVMemMap;

	if (NULL == prRing) {
		DmxLogT(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
			TEXT("[MEM] %s line %d -- no VMemMap\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		spin_unlock_irqrestore(&_DmxMemLock, flags);
		return;
	}

	while ((NULL != prRing) &&
		(prRing->pvVirtualMem != pvVMem)) {
		prPrev = prRing;
		prRing = prRing->prNext;
	}

	/* Delete the first Notes. */
	if (NULL == prPrev)
		_prVMemMap = _prVMemMap->prNext;
	else if (NULL != prRing)
		prPrev->prNext = prRing->prNext;

	if (prRing != NULL) {
		prRing->prNext = NULL;

#if DMX_MEM_GUARD_PATTEN_CHECK
		pvGuardPrefixAddr = (void *)(prRing->ptrGuardPrefixAddr);
#endif

		DmxLogD(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
			TEXT("[MEM] %s line %d -- free Mem, (VMem: 0x%08x, Size: 0x%08x),")
			TEXT(" Func(%s), line(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			(uintptr_t)(prRing->pvVirtualMem), prRing->u4Size,
			prRing->szFunc, prRing->i4Line);
		DMX_CheckMemGuard(prRing);

		MM_FREE(prRing);
	}

	spin_unlock_irqrestore(&_DmxMemLock, flags);

#if DMX_MEM_GUARD_PATTEN_CHECK
	if (pvGuardPrefixAddr != NULL)
		MM_FREE(pvGuardPrefixAddr);
#endif
	return;

}
#endif /* DMX_MEM_GUARD_PATTEN_CHECK */

#endif /* #if DMX_MEM_PRINT_LINE_FUNCTION */

bool Dmx_RingMemCpy(u8 *pu1DstBufSA, u8 *pu1DstBufEA,
	u8 *pu1DstAddr, u8 *pu1SrcAddr, u32 u4Size)
{
	u32 u4EndLen = 0;

	if ((pu1DstAddr > pu1DstBufEA) ||
		(pu1DstAddr < pu1DstBufSA)) {
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
			TEXT("[MEM] %s line %d fail for invalid DstAddr(0x%x),")
			TEXT(" (pu1DstBufSA: 0x%08x, pu1DstBufEA: 0x%08x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, (uintptr_t)pu1DstAddr,
			(uintptr_t)pu1DstBufSA, (uintptr_t)pu1DstBufEA);
		return FALSE;
	}
	if (u4Size > ((uintptr_t)pu1DstBufEA - (uintptr_t)pu1DstBufSA)) {
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,
			TEXT("[MEM] %s line %d fail for invalid u4Size(0x%x), ")
			TEXT("it > DstBufSz, (pu1DstBufSA: 0x%08x, ")
			TEXT("pu1DstBufEA: 0x%08x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Size,
			(uintptr_t)pu1DstBufSA, (uintptr_t)pu1DstBufEA);
		return FALSE;
	}

	u4EndLen = (uintptr_t)(pu1DstBufEA) - (uintptr_t)(pu1DstAddr);
	if (u4EndLen < u4Size) {
		dmx_memcpy(pu1DstAddr, pu1SrcAddr, u4EndLen);
		dmx_memcpy(pu1DstBufSA, pu1SrcAddr + u4EndLen, (u4Size - u4EndLen));
	} else
		dmx_memcpy(pu1DstAddr, pu1SrcAddr, u4Size);

	return TRUE;
}


