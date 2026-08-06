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

#include <linux/mm.h>
#include <linux/gfp.h>
#include <asm/page.h>
#include <linux/semaphore.h>
#include <media/atc/memdbg_c.h>
#include <media/atc/mm_debug.h>
#include <media/atc/ose_mem.h>
#include "types.h"
#include "windows.h"
#include "mmisc_osemem.h"
#include "memmanager.h"
#include "mmisc.h"
#include "assert.h"
#include "x_os.h"

#define MAX_PATH 256

#define OSE_RES

#define MULTIMEDIA_BUF_VA_START (OSE_GetMMReservedMemStartAddr())
#define MULTIMEDIA_BUF_PA_START (OSE_VAToPA((void *)OSE_GetMMReservedMemStartAddr()))
#define MULTIMEDIA_BUF_SIZE     (OSE_GetMMReservedMemSize())

#define OSE_HOUSEKEEPING_BUF_SIZE      ((u32)(16*1024))
#define OSE_HOUSEKEEPING_BUF_VA_START  (MULTIMEDIA_BUF_VA_START + MULTIMEDIA_BUF_SIZE-OSE_HOUSEKEEPING_BUF_SIZE)
#define OSE_HOUSEKEEPING_BUF_PA_START  (MULTIMEDIA_BUF_PA_START + MULTIMEDIA_BUF_SIZE-OSE_HOUSEKEEPING_BUF_SIZE)


#ifndef BSP_NO_MEMMAN
/* get greatest common divisor */
static u32 x_GetGCD(u32 u4Num1, u32 u4Num2)
{
	u32 u4GCD = 0;

	u4GCD = (0 == u4Num2) ? (u4Num1) : x_GetGCD(u4Num2, (u4Num1 % u4Num2));
	return u4GCD;
}

/* get least common multiple */
static u32 x_GetLCM(u32 u4Num1, u32 u4Num2)
{
	u32 u4GCD = 0;
	u32 u4LCM = 0;

	u4GCD = x_GetGCD(u4Num1, u4Num2);
	u4LCM = (u4Num1 * u4Num2) / u4GCD;
	return u4LCM;
}

#define NEW_ALIGN_VALUE(u4Align) x_GetLCM(u4Align, 2*sizeof(g_guard))
#endif

typedef struct _OSE_MEM_NODE {
	struct _OSE_MEM_NODE *prior;
	struct _OSE_MEM_NODE *next;

	OSE_MEM_REGION eRegion;
	void *pvVirAddr;
	u32 u4Size;
#ifndef BSP_NO_MEMMAN
	void *pvActualVirAddr;
	u32 u4ActualSize;
#endif
	bool fgUsed;

} OSE_MEM_NODE;

typedef struct _OSE_MEM_LIST {
	OSE_MEM_NODE *head;
	OSE_MEM_NODE *tail;

	u32 u4TotalSizeUsed;
} OSE_MEM_LIST;

typedef struct _OSE_DYNAMIC_NODE {
	struct _OSE_DYNAMIC_NODE *next;

	void *pvVirAddr;
	uintptr_t ptrPhyAddr;
	u32 u4Order;
#ifndef BSP_NO_MEMMAN
	u32 u4ActualSize;
	void *pvActualVirAddr;
	uintptr_t ptrActualPhyAddr;
#endif
} OSE_DYNAMIC_NODE;

#define         OSE_SEMA_NAME  TEXT("OSE_SEMA")

static struct semaphore g_hOSESema;
static OSE_DYNAMIC_NODE *g_DynamicMemNodeList;

static char *g_arRegionText[] = {
	TEXT("NONE"),
	TEXT("DEMUXER"),
	TEXT("VDEC"),
	TEXT("ADEC"),
	TEXT("RLE"),
	TEXT("JDEC"),
	TEXT("PNG"),
	TEXT("GIF"),
	TEXT("WCH"),
	TEXT("REGION_MAX")
};

static OSE_MEM_NODE *OSE_GET_NODE(OSE_MEM_NODE *pNode, u32 u4MaxCnt)
{
	u32 i = 0;

	/* Get the node whose VirAddr field is not zero */
	for (i = 0; i < u4MaxCnt; ++i) {
		if (pNode[i].pvVirAddr == NULL)
			return &(pNode[i]);
	}
	return NULL;
}

static void OSE_RESET_NODE(OSE_MEM_NODE *pNode)
{
	mm_memset(pNode, 0, sizeof(OSE_MEM_NODE));
}

bool OSE_Init(void)
{
	OSE_MEM_LIST *pOSEMemList = NULL;
	OSE_MEM_NODE *pOSENodeList = NULL;
	u32 u4MaxNodeCnt = 0;

	pOSEMemList = (OSE_MEM_LIST *) (OSE_HOUSEKEEPING_BUF_VA_START);
	pOSENodeList = (OSE_MEM_NODE *) (OSE_HOUSEKEEPING_BUF_VA_START + sizeof(OSE_MEM_LIST));

	u4MaxNodeCnt = (OSE_HOUSEKEEPING_BUF_SIZE - sizeof(OSE_MEM_LIST)) / sizeof(OSE_MEM_NODE);

	init_MUTEX(&g_hOSESema);
	down(&g_hOSESema);

	mm_memset(pOSEMemList, 0, sizeof(OSE_MEM_LIST));
	mm_memset(pOSENodeList, 0, (OSE_HOUSEKEEPING_BUF_SIZE - sizeof(OSE_MEM_LIST)));

	pOSEMemList->head = pOSEMemList->tail = OSE_GET_NODE(pOSENodeList, u4MaxNodeCnt);
	if (NULL != pOSEMemList->head) {
		pOSEMemList->u4TotalSizeUsed = 0;
		pOSEMemList->head->fgUsed = FALSE;
		pOSEMemList->head->next = NULL;
		pOSEMemList->head->prior = NULL;
		pOSEMemList->head->pvVirAddr = MULTIMEDIA_BUF_VA_START;
		pOSEMemList->head->u4Size = (MULTIMEDIA_BUF_SIZE - OSE_HOUSEKEEPING_BUF_SIZE);
		/* pOSEMemList->head->u4Size = (MULTIMEDIA_BUF_SIZE); */
		pOSEMemList->head->eRegion = OSE_REGION_NONE;
#ifndef BSP_NO_MEMMAN
		pOSEMemList->head->pvActualVirAddr = pOSEMemList->head->pvVirAddr;
		pOSEMemList->head->u4ActualSize = pOSEMemList->head->u4Size;
#endif
	}

	g_DynamicMemNodeList = NULL;

	up(&g_hOSESema);

	return TRUE;
}

void OSE_Uninit(void)
{
}

bool OSE_MemInit(OSE_MEM_REGION eMemRegion)
{
#ifdef CHECK_OSE_MEMORY
	pr_debug("[OSE]OSE_MemInit Module: %s , g_hOSESema: 0x%p.\r\n",
		 g_arRegionText[eMemRegion], g_hOSESema);
	pr_debug
	    ("[OSE]OSE_HOUSEKEEPING_BUF_VA_START VA : 0x%08x, PA : 0x%08x, MM VA:0x%08x, MM PA:0x%08x, MM Size:0x%08x, MM_IO_VIRT_TOP:0x%08x, MM IO Size:0x%08x\r\n",
	     OSE_HOUSEKEEPING_BUF_VA_START, OSE_HOUSEKEEPING_BUF_PA_START, MULTIMEDIA_BUF_VA_START,
	     MULTIMEDIA_BUF_PA_START, MULTIMEDIA_BUF_SIZE, MULTIMEDIA_IO_VIRT_TOP,
	     MULTIMEDIA_IO_SIZE);
#endif
	return TRUE;
}
EXPORT_SYMBOL(OSE_MemInit);

void OSE_MemUninit(OSE_MEM_REGION eMemRegion)
{
#ifdef CHECK_OSE_MEMORY
	pr_debug("[OSE]OSE_MemUninit Module: %s .\r\n"), g_arRegionText[eMemRegion]));
#endif
}
EXPORT_SYMBOL(OSE_MemUninit);


static void PrintMemList(OSE_MEM_LIST *pMemList)
{
	OSE_MEM_NODE *pNode = pMemList->head;

	pr_err("[OSE]Total size : 0x%08x, used size : 0x%08x\r\n",
	       MULTIMEDIA_BUF_SIZE, pMemList->u4TotalSizeUsed);
	pr_err("[OSE]Memory List:\r\n");

	while (NULL != pNode) {
		pr_err("\tAddr: %p, Size: 0x%08x, Used: 0x%1x, Region: %s\r\n",
		  (void *)(pNode->pvVirAddr), pNode->u4Size, (u8) (pNode->fgUsed),
		       g_arRegionText[pNode->eRegion]);
		pNode = pNode->next;
	}
}

#ifndef BSP_NO_MEMMAN
bool OSE_CheckDynamicNode(void)
{
	OSE_DYNAMIC_NODE *pDynamicNode = g_DynamicMemNodeList;
	bool bResult = TRUE;

	while (NULL != pDynamicNode) {
		if ((mm_memcmp((u8 *) pDynamicNode->pvVirAddr, g_guard, sizeof(g_guard)) != 0)
		    ||
		    (mm_memcmp
		     ((u8 *) ((uintptr_t) (pDynamicNode->pvActualVirAddr) - sizeof(g_guard)),
		      g_guard, sizeof(g_guard)) != 0)
		    ||
		    (mm_memcmp
		     ((u8 *) ((uintptr_t) (pDynamicNode->pvActualVirAddr) +
			      pDynamicNode->u4ActualSize), g_guard, sizeof(g_guard)) != 0)) {
			pr_debug
			    ("[OSE][OSE_CheckDynamicNode]+++++: pvVirAddr: %p, pvActualVirAddr: %p, u4ActualSize: 0x%lx\r\n",
			     pDynamicNode->pvVirAddr, pDynamicNode->pvActualVirAddr,
			     pDynamicNode->u4ActualSize);
			MM_ASSERT(0);
			bResult = FALSE;
		}
		pDynamicNode = pDynamicNode->next;
	}

	return bResult;
}

bool OSE_CheckReservedNode(OSE_MEM_LIST *pOSEMemList)
{
	OSE_MEM_NODE *pNode = pOSEMemList->head;
	bool bResult = TRUE;

	while (NULL != pNode) {
		if (pNode->fgUsed) {
			if ((mm_memcmp((u8 *) pNode->pvVirAddr, g_guard, sizeof(g_guard)) != 0)
			    ||
			    (mm_memcmp
			     ((u8 *) (pNode->pvActualVirAddr - sizeof(g_guard)), g_guard,
			      sizeof(g_guard)) != 0)
			    ||
			    (mm_memcmp
			     ((u8 *) (pNode->pvActualVirAddr + pNode->u4ActualSize), g_guard,
			      sizeof(g_guard)) != 0)) {
				pr_debug
				    ("[OSE][OSE_CheckReservedNode]+++++: Module: %s, Used: 0x%1x, ptrVirAddr: %p, ptrActualVirAddr: %p, u4Size: 0x%lx, u4ActualSize: 0x%lx.\r\n",
				     g_arRegionText[pNode->eRegion], (u8) (pNode->fgUsed),
				     pNode->pvVirAddr, pNode->pvActualVirAddr,
				     pNode->u4Size, pNode->u4ActualSize);
				MM_ASSERT(0);
				PrintMemList(pOSEMemList);
				bResult = FALSE;
			}
		}
		pNode = pNode->next;
	}
	return bResult;
}

bool OSE_CheckMemoryLeak(OSE_MEM_REGION eMemRegion)
{
	bool bRet = TRUE;
	OSE_MEM_LIST *pOSEMemList = (OSE_MEM_LIST *) OSE_HOUSEKEEPING_BUF_VA_START;

	down(&g_hOSESema);

	if (!OSE_CheckDynamicNode()) {
		pr_err
		    ("[OSE][OSE_CheckMemoryLeak]ERROR: Check the system memory crc failure: Module %s\r\n",
		     g_arRegionText[eMemRegion]);
		MM_ASSERT(0);
		bRet = FALSE;
	}

	if (!OSE_CheckReservedNode(pOSEMemList)) {
		pr_err
		    ("[OSE][OSE_CheckMemoryLeak]ERROR: Check the reserved memory crc failure: Module %s\r\n",
		     g_arRegionText[eMemRegion]);
		MM_ASSERT(0);
		bRet = FALSE;
	}

	up(&g_hOSESema);

	return bRet;
}
#endif

static void DoMemRegionAlloc(OSE_MEM_LIST *pMemList, OSE_MEM_NODE *pNode,
			     uintptr_t ptrAlignedAddr, u32 u4Size, char *file, u32 u4Line) {
	ptrdiff_t prior_sz = ptrAlignedAddr - (uintptr_t)pNode->pvVirAddr;
	u32 u4NextSize = pNode->u4Size - prior_sz - u4Size;
	u32 u4MaxNodeCnt =
	    (OSE_HOUSEKEEPING_BUF_SIZE - sizeof(OSE_MEM_LIST)) / sizeof(OSE_MEM_NODE);
	OSE_MEM_NODE *pOSENodeList =
	    (OSE_MEM_NODE *) (OSE_HOUSEKEEPING_BUF_VA_START + sizeof(OSE_MEM_LIST));

	if ((ptrdiff_t) 0 != prior_sz) {
		OSE_MEM_NODE *pPriorNode = OSE_GET_NODE(pOSENodeList, u4MaxNodeCnt);

		if (NULL == pPriorNode)
			return;

		pPriorNode->prior = pNode->prior;
		pPriorNode->next = pNode;
		pPriorNode->eRegion = OSE_REGION_NONE;
		pPriorNode->pvVirAddr = pNode->pvVirAddr;
		pPriorNode->fgUsed = FALSE;
		pPriorNode->u4Size = (u32) prior_sz;
#ifndef BSP_NO_MEMMAN
		pPriorNode->pvActualVirAddr = pNode->pvVirAddr;
		pPriorNode->u4ActualSize = (u32) prior_sz;
#endif
		if (pNode->prior != NULL)
			pNode->prior->next = pPriorNode;
		else
			pMemList->head = pPriorNode;

		pNode->prior = pPriorNode;
	}

	if (0 != u4NextSize) {
		OSE_MEM_NODE *pNextNode = OSE_GET_NODE(pOSENodeList, u4MaxNodeCnt);

		if (NULL == pNextNode)
			return;

		pNextNode->prior = pNode;
		pNextNode->next = pNode->next;
		pNextNode->eRegion = OSE_REGION_NONE;
		pNextNode->pvVirAddr = (void *)(ptrAlignedAddr + u4Size);
		pNextNode->fgUsed = FALSE;
		pNextNode->u4Size = u4NextSize;
#ifndef BSP_NO_MEMMAN
		pNextNode->pvActualVirAddr = pNextNode->pvVirAddr;
		pNextNode->u4ActualSize = pNextNode->u4Size;
#endif
		if (pNode->next != NULL)
			pNode->next->prior = pNextNode;
		else
			pMemList->tail = pNextNode;

		pNode->next = pNextNode;
	}

	pNode->fgUsed = TRUE;
	pNode->pvVirAddr = (void *)ptrAlignedAddr;
	pNode->u4Size = u4Size;
}


/* from begin to end */
static OSE_MEM_NODE *u4GetMemRegionBack(OSE_MEM_LIST *pMemList,
					u32 u4Size, u32 u4Align, char *file, u32 u4Line) {
	OSE_MEM_NODE *pNode = pMemList->head;
	uintptr_t ptrAlignedAddr = (uintptr_t) u4Align;

	/* find the unused node */
	while (NULL != pNode) {
		if (!pNode->fgUsed) {
			ptrAlignedAddr = OSE_BUF_ALIGN_MASK((uintptr_t)pNode->pvVirAddr, u4Align);
			/* find the node which have enough memory size */
			if (((ptrAlignedAddr - (uintptr_t)pNode->pvVirAddr) + u4Size) <= (uintptr_t)pNode->u4Size)
				break;
		}
		pNode = pNode->next;
	}

	if (NULL != pNode) {
		ptrAlignedAddr = OSE_BUF_ALIGN_MASK((uintptr_t)pNode->pvVirAddr, u4Align);
		DoMemRegionAlloc(pMemList, pNode, ptrAlignedAddr, u4Size, file, u4Line);
		pMemList->u4TotalSizeUsed += u4Size;
		return pNode;
	} else {
		return NULL;
	}
}

/* from end to begin */
static OSE_MEM_NODE *u4GetMemRegionForward(OSE_MEM_LIST *pMemList,
					   u32 u4Size, u32 u4Align, char *file, u32 u4Line) {
	OSE_MEM_NODE *pNode = pMemList->tail;
	uintptr_t ptrAlignedAddr = u4Align;

	/* find the unused node */
	while (NULL != pNode) {
		if (!pNode->fgUsed) {
			ptrAlignedAddr =
			    ((uintptr_t)pNode->pvVirAddr + pNode->u4Size - u4Size) & (~((uintptr_t)u4Align - 1));
			if (ptrAlignedAddr >= (uintptr_t)pNode->pvVirAddr)
				break;
		}
		pNode = pNode->prior;
	}

	if (NULL != pNode) {
		ptrAlignedAddr = ((uintptr_t)pNode->pvVirAddr + pNode->u4Size - u4Size) & (~((uintptr_t)u4Align - 1));
		DoMemRegionAlloc(pMemList, pNode, ptrAlignedAddr, u4Size, file, u4Line);
		pMemList->u4TotalSizeUsed += u4Size;
		return pNode;
	}

	return NULL;
}


static void vDoMemRegionFree(OSE_MEM_LIST *pMemList,
  void *pvVirAddr, char *file, u32 u4Line)
{
	OSE_MEM_NODE *pNode = pMemList->head;

	for (; NULL != pNode; pNode = pNode->next) {
#ifndef BSP_NO_MEMMAN
		if (pNode->pvActualVirAddr == pvVirAddr) {
			MM_ASSERT(pNode->fgUsed);
			break;
		}
#else
		if (pNode->pvVirAddr == pvVirAddr) {
			MM_ASSERT(pNode->fgUsed);
			break;
		}
#endif
	}

	if (NULL != pNode) {
		OSE_MEM_NODE *pTmpNode = NULL;

		pMemList->u4TotalSizeUsed -= pNode->u4Size;

		/* merge to prior node */
		if ((NULL != pNode->prior) && (FALSE == pNode->prior->fgUsed)) {
			pNode->prior->u4Size += pNode->u4Size;
			pNode->prior->next = pNode->next;

			if (NULL != pNode->next)
				pNode->next->prior = pNode->prior;
			else
				pMemList->tail = pNode->prior;

			pTmpNode = pNode->prior;
			OSE_RESET_NODE(pNode);
			pNode = pTmpNode;
		}
		/* merge to next node */
		if ((NULL != pNode->next) && (FALSE == pNode->next->fgUsed)) {
			pNode->next->pvVirAddr = pNode->pvVirAddr;
			pNode->next->u4Size += pNode->u4Size;
			pNode->next->prior = pNode->prior;

			if (NULL != pNode->prior)
				pNode->prior->next = pNode->next;
			else
				pMemList->head = pNode->next;

			pTmpNode = pNode->next;
			OSE_RESET_NODE(pNode);
			pNode = pTmpNode;
		}

		pNode->fgUsed = FALSE;
		pNode->eRegion = OSE_REGION_NONE;
	} else {
		pr_debug("[OSE]RegionFree pMemList: %p, pvVirAddr: %p not found, %s, %ud\r\n",
			 pMemList, pvVirAddr, file, u4Line);
		PrintMemList(pMemList);
		MM_ASSERT(0);
	}
}

/* if failed to alloc memory from reserved memory pool, the try alloc memory from system memory pool */
void *OSE_MemAllocCustom_R(OSE_MEM_REGION eMemRegion, u32 u4Size, u32 u4Align,
			   uintptr_t *pptrPhyAddr, char *file, u32 u4Line) {
	void *pvVirAddr = NULL;
	OSE_MEM_LIST *pOSEMemList = (OSE_MEM_LIST *) OSE_HOUSEKEEPING_BUF_VA_START;
	OSE_MEM_NODE *pNode = NULL;
#ifndef BSP_NO_MEMMAN
	alloc_node *prNode = vmalloc(sizeof(alloc_node));

	bool bRet = FALSE;
#endif
	if (0 == u4Size) {
		MM_ASSERT(0);
		pr_debug("[OSE]Alloc Size = 0, Module: %s, file: %s[%ud].\r\n",
			 g_arRegionText[eMemRegion], file, u4Line);
	}

	down(&g_hOSESema);

#ifndef BSP_NO_MEMMAN
	u4Size += (sizeof(g_guard) + NEW_ALIGN_VALUE(u4Align));
#endif
#ifdef CHECK_OSE_MEMORY
	pr_debug
	    ("[OSE]Module: %s is allocating reserved memory, file: %s[%d], size: 0x%08x, u4Align: 0x%08x.\r\n",
	     g_arRegionText[eMemRegion], file, u4Line, u4Size, u4Align);
#endif
	/* alloc memory from reserved memory pool */
	if ((OSE_VDEC != eMemRegion) && (OSE_JDEC != eMemRegion) && (OSE_PNG != eMemRegion)
	    && (OSE_GIF != eMemRegion))
		pNode = u4GetMemRegionBack(pOSEMemList, u4Size, u4Align, file, u4Line);
	else
		pNode = u4GetMemRegionForward(pOSEMemList, u4Size, u4Align, file, u4Line);

	/* fail to alloc memory */
	if (NULL == pNode) {
		/* PrintMemList(pOSEMemList); */

		pvVirAddr = NULL;
		*pptrPhyAddr = (uintptr_t) NULL;
	} else {
		pNode->eRegion = eMemRegion;
#ifndef BSP_NO_MEMMAN
		pNode->pvActualVirAddr = pNode->pvVirAddr + NEW_ALIGN_VALUE(u4Align);
		pNode->u4ActualSize = u4Size - NEW_ALIGN_VALUE(u4Align) - sizeof(g_guard);
		pvVirAddr = pNode->pvActualVirAddr;

		mm_memcpy((void *)pNode->pvVirAddr, g_guard, sizeof(g_guard));
		mm_memcpy((void *)(pNode->pvActualVirAddr - sizeof(g_guard)), g_guard,
			  sizeof(g_guard));
		mm_memcpy((void *)(pNode->pvActualVirAddr + pNode->u4ActualSize), g_guard,
			  sizeof(g_guard));
		/* save the allocator info */

		prNode->lptr = prNode->rptr = NULL;
		prNode->len = pNode->u4ActualSize;
		prNode->line = u4Line;
		prNode->pvAddr = pNode->pvActualVirAddr;
		strcpy(prNode->file, file);

		BTreeInsertLib(prNode);
#else
		pvVirAddr = pNode->pvVirAddr;
#endif

		*pptrPhyAddr = OSE_VAToPA(pvVirAddr);
	}

	/* fail to alloc memory from reserved and system pool */
	if (NULL == pvVirAddr) {
		pr_err
		    ("[OSE]OSE_MemAllocCustom_R Alloc memory failed, u4Size=0x%8x, u4Align=0x%8x, memRegion=%s\r\n",
		     u4Size, u4Align, g_arRegionText[eMemRegion]);
		PrintMemList(pOSEMemList);
	}
#ifdef CHECK_OSE_MEMORY
	else {
		pr_info
		    ("[OSE]OSE_MemAllocCustom_R Alloc memory OK, ptrVirAddr:%p, *pptrPhyAddr:%p,  u4Size=0x%8x, u4Align=0x%8x, memRegion=%s\r\n",
		     pvVirAddr, (void *)(*pptrPhyAddr), u4Size, u4Align,
		     g_arRegionText[eMemRegion]);
	}
#endif
	up(&g_hOSESema);

#ifndef BSP_NO_MEMMAN
	bRet = OSE_CheckMemoryLeak(eMemRegion);
	if (!bRet) {
		pr_info
		    ("[OSE][OSE_MemAllocCustom_R]ERROR: Module:%s is allocating memory.ptrVirAddr:%p, *pptrPhyAddr:%p, u4Size:0x%08x, file:%s[%ud]\r\n",
		     g_arRegionText[eMemRegion], pvVirAddr, (void *)(*pptrPhyAddr), u4Size,
		     file, u4Line);
	}
#endif

	return pvVirAddr;
}
EXPORT_SYMBOL(OSE_MemAllocCustom_R);


/* alloc memory from system memory pool */
void *OSE_MemAllocCustom_S(OSE_MEM_REGION eMemRegion, u32 u4Size, u32 u4Align,
			   uintptr_t *pptrPhyAddr, char *file, u32 u4Line) {
	void *pvVirAddr = NULL;
	OSE_DYNAMIC_NODE *pDynamicNode = NULL;
#ifndef BSP_NO_MEMMAN
	alloc_node *prNode = vmalloc(sizeof(alloc_node));
	bool bRet = FALSE;
#endif

	if (0 == u4Size) {
		MM_ASSERT(0);
		pr_info("[OSE]Alloc Size = 0, Module: %s, file: %s.\r\n",
			g_arRegionText[eMemRegion], file);
	}

	down(&g_hOSESema);

#ifndef BSP_NO_MEMMAN
	u4Size += (sizeof(g_guard) + NEW_ALIGN_VALUE(u4Align));
#endif

	pDynamicNode = (OSE_DYNAMIC_NODE *) LocalAlloc(LPTR, sizeof(OSE_DYNAMIC_NODE));
	if (NULL == pDynamicNode) {
		*pptrPhyAddr = (uintptr_t) 0;
		pvVirAddr = NULL;
	} else {
		pDynamicNode->u4Order = get_order(u4Size);
		pDynamicNode->pvVirAddr =
		    (void *)__get_free_pages(GFP_KERNEL, pDynamicNode->u4Order);
		pDynamicNode->ptrPhyAddr = __pa(pDynamicNode->pvVirAddr);

		if (NULL != pDynamicNode->pvVirAddr) {
#ifndef BSP_NO_MEMMAN
			pDynamicNode->u4ActualSize =
			    u4Size - NEW_ALIGN_VALUE(u4Align) - sizeof(g_guard);
			pDynamicNode->pvActualVirAddr =
			    (u8 *) pDynamicNode->pvVirAddr + NEW_ALIGN_VALUE(u4Align);
			pDynamicNode->ptrActualPhyAddr =
			    pDynamicNode->ptrPhyAddr + NEW_ALIGN_VALUE(u4Align);

			*pptrPhyAddr = pDynamicNode->ptrActualPhyAddr;
			pvVirAddr = pDynamicNode->pvActualVirAddr;

			mm_memcpy(pDynamicNode->pvVirAddr, g_guard, sizeof(g_guard));
			mm_memcpy((void *)((uintptr_t) (pDynamicNode->pvActualVirAddr) -
					   sizeof(g_guard)), g_guard, sizeof(g_guard));
			mm_memcpy((void *)((uintptr_t) (pDynamicNode->pvActualVirAddr) +
					   pDynamicNode->u4ActualSize), g_guard, sizeof(g_guard));
			/* save the allocator info */
			prNode->lptr = prNode->rptr = NULL;
			prNode->len = pDynamicNode->u4ActualSize;
			prNode->line = u4Line;
			prNode->pvAddr = pDynamicNode->pvActualVirAddr;
			strcpy(prNode->file, file);

			BTreeInsertLib(prNode);

#else
			*pptrPhyAddr = pDynamicNode->ptrPhyAddr;
			pvVirAddr = pDynamicNode->pvVirAddr;
#endif
			/* add the node to the List */
			pDynamicNode->next = g_DynamicMemNodeList;
			g_DynamicMemNodeList = pDynamicNode;
		} else {
			*pptrPhyAddr = (uintptr_t) 0;
			pvVirAddr = NULL;

			LocalFree(pDynamicNode);
		}
	}

	/* fail to alloc memory from system pool */
	if (NULL == pvVirAddr) {
		pr_err
		    ("[OSE]OSE_MemAllocCustom_S Alloc memory failed, u4Size=0x%08x, u4Align=0x%08x, eMemRegion=%s\r\n",
		     u4Size, u4Align, g_arRegionText[eMemRegion]);
	}

	up(&g_hOSESema);

#ifndef BSP_NO_MEMMAN
	bRet = OSE_CheckMemoryLeak(eMemRegion);
	if (!bRet) {
		pr_err
		    ("[OSE][OSE_MemAllocCustom_S]ERROR: Module:%s is allocating memory. ptrVirAddr:%p, *pptrPhyAddr:%p, u4Size:0x%08x, file:%s[%d]\r\n",
		     g_arRegionText[eMemRegion], pvVirAddr, (void *)(*pptrPhyAddr), u4Size,
		     file, u4Line);
	}
#endif

	return pvVirAddr;
}
EXPORT_SYMBOL(OSE_MemAllocCustom_S);


void OSE_MemFreeCustom_R(OSE_MEM_REGION eMemRegion, void *pvVirAddr, char *file, u32 u4Line)
{
	OSE_MEM_LIST *pOSEMemList = (OSE_MEM_LIST *) OSE_HOUSEKEEPING_BUF_VA_START;
	OSE_DYNAMIC_NODE *pDynamicNode = g_DynamicMemNodeList;
	OSE_DYNAMIC_NODE *pPrevNode = NULL;
#ifndef BSP_NO_MEMMAN
	bool fgRet = FALSE;
#endif

	if (NULL == pvVirAddr)
		return;

	down(&g_hOSESema);

	if (((uintptr_t) pvVirAddr > ((uintptr_t) MULTIMEDIA_BUF_VA_START + MULTIMEDIA_BUF_SIZE))
	    || ((uintptr_t) pvVirAddr < (uintptr_t) MULTIMEDIA_BUF_VA_START)) {
		while (NULL != pDynamicNode) {
#ifndef BSP_NO_MEMMAN
			if (pDynamicNode->pvActualVirAddr == pvVirAddr) {
#else
			if (pDynamicNode->pvVirAddr == pvVirAddr) {
#endif
				/* the first node is matched */
				if (NULL == pPrevNode)
					g_DynamicMemNodeList = pDynamicNode->next;
				else
					pPrevNode->next = pDynamicNode->next;

#ifndef BSP_NO_MEMMAN
				fgRet = BTreeDelBufNotFree(pvVirAddr);
				if (!fgRet)
					pr_err
					    ("[OSE]Delete System BTree Node Failed, pvVirAddr:0x%p, file:%s[%d]. \r\n",
					     (void *)pvVirAddr, file, u4Line);
#endif

				/* free the memory and delete the node */
				free_pages((uintptr_t) pDynamicNode->pvVirAddr,
					   pDynamicNode->u4Order);
				LocalFree(pDynamicNode);
				break;
			}
			pPrevNode = pDynamicNode;
			pDynamicNode = pDynamicNode->next;
		}
	}
	/* the memory is allocated from reserved memory pool */
	else {
#ifndef BSP_NO_MEMMAN
		fgRet = BTreeDelBufNotFree(pvVirAddr);
		if (!fgRet) {
			pr_err
			    ("[OSE]Delete Reserved BTree Node Failed, pvVirAddr:0x%p, file:%s[%d]. \r\n",
			     (void *)pvVirAddr, file, u4Line);
		}
#endif
		vDoMemRegionFree(pOSEMemList, pvVirAddr, file, u4Line);
	}
	up(&g_hOSESema);

}
EXPORT_SYMBOL(OSE_MemFreeCustom_R);

uintptr_t OSE_VAToPA(void *pvVirAddr)
{
	OSE_DYNAMIC_NODE *pDynamicNode = g_DynamicMemNodeList;

	if (((uintptr_t) pvVirAddr > (uintptr_t) OSE_HOUSEKEEPING_BUF_VA_START)
	    || ((uintptr_t) pvVirAddr < (uintptr_t) MULTIMEDIA_BUF_VA_START)) {
		while (NULL != pDynamicNode) {
#ifndef BSP_NO_MEMMAN
			if (pDynamicNode->pvActualVirAddr == pvVirAddr) {
#else
			if (pDynamicNode->pvVirAddr == pvVirAddr) {
#endif
#ifndef BSP_NO_MEMMAN
				return pDynamicNode->ptrActualPhyAddr;
#else
				return pDynamicNode->ptrPhyAddr;
#endif
			} else {
				pDynamicNode = pDynamicNode->next;
			}
		}
	}

	if ((phys_addr_t)0 == g_rsvmem_info->phys_addr)
		return (uintptr_t)0;

	if (NULL == g_rsvmem_info->virt_addr)
		return (uintptr_t)0;

	return (uintptr_t)(g_rsvmem_info->phys_addr +
			((uintptr_t) pvVirAddr - (uintptr_t) (g_rsvmem_info->virt_addr)));
}
EXPORT_SYMBOL(OSE_VAToPA);

void *OSE_PAToVA(uintptr_t ptrPhy)
{
	OSE_DYNAMIC_NODE *pDynamicNode = g_DynamicMemNodeList;

	if ((ptrPhy > (uintptr_t) OSE_HOUSEKEEPING_BUF_PA_START)
	    || (ptrPhy < (uintptr_t) MULTIMEDIA_BUF_PA_START)) {
		while (NULL != pDynamicNode) {
#ifndef BSP_NO_MEMMAN
			if (pDynamicNode->ptrActualPhyAddr == ptrPhy) {
#else
			if (pDynamicNode->ptrPhyAddr == ptrPhy) {
#endif
#ifndef BSP_NO_MEMMAN
				return pDynamicNode->pvActualVirAddr;
#else
				return pDynamicNode->pvVirAddr;
#endif
			} else {
				pDynamicNode = pDynamicNode->next;
			}
		}
	}

	if (0 == g_rsvmem_info->phys_addr)
		return NULL;

	if (0 == g_rsvmem_info->virt_addr)
		return NULL;

	return (void *)(g_rsvmem_info->virt_addr + ((uintptr_t) ptrPhy - g_rsvmem_info->phys_addr));
}
EXPORT_SYMBOL(OSE_PAToVA);

/*
---------------------------------------------------------------------------
*  Multimedia Mem Cfg
---------------------------------------------------------------------------
*/
static u32 MM_GetMemCfg(void)
{
	u32 dwMemorySize = 256;

	return dwMemorySize;
}

static void *MM_GetReservedMemSAddr(void)
{
	if (NULL == g_rsvmem_info)
		return NULL;

	return (g_rsvmem_info->virt_addr);
}

/* Video Reserved memory size */
static u32 MM_GetReservedMemSize(void)
{
	if (NULL == g_rsvmem_info)
		return 0;

	return g_rsvmem_info->size;
}


/* Pbbuf Size For Interleave File */
static u32 MM_GetAVPbbufSize(void)
{
	if (128 == MM_GetMemCfg())	/* 128 version: 1M */
		return 0x40000;

	/* 256 version: 2M */
	return 0x200000;
}

/* Pbbuf Size For Badinterleave File */
static u32 MM_GetAPbbufSize(void)
{
	if (128 == MM_GetMemCfg())
		return 0x40000;	/* 256K */

	return 0x100000;	/* 1M */
}

/* Slot size, Default 256K */
static u32 MM_GetPbbufSlotSize(void)
{
	if (128 == MM_GetMemCfg())
		return 0x20000;	/* 128k */

	return 0x80000;		/* 512k */
}

/* SP pbbuf size */
static u32 MM_GetSPPbbufSize(void)
{
	if (128 == MM_GetMemCfg())	/* 128 version */
		return 0x10000;	/* 64k */

	/* 256 version */
	return 0x100000;	/* 1M */
}

/* sp pbbuf slot size */
static u32 MM_GetSPPbbufSlotSize(void)
{
	if (128 == MM_GetMemCfg())	/* 128 version */
		return 0x8000;	/* 32k */

	/* 256 version */
	return 0x10000;		/* 64k */
}

/* Video FIFO Size */
static u32 MM_GetVFIFOSize(void)
{
	return 0x200000;	/* 2M */
}

/* Video FIFO Size for High Bitrate */
static u32 MM_GetHighBitrateVFIFOSize(void)
{
	return 0x700000;	/* 7M */
}

/* SubPicture FIFO Size */
static u32 MM_GetSPFIFOSize(void)
{
	if (128 == MM_GetMemCfg())
		return 0x10000;	/* 64k */

	return 0x100000;	/* 1M */
}

static u32 MM_GetVDECBUFSize(void)
{
	/* return 0x1880000; // 12.5M + 12M */
	return 0xA00000;	/* 10M */
}

static u32 MM_GetAFIFOSize(void)
{
	return 0x104000;	/* 1040KB */
}

/*
---------------------------------------------------------------------------
*  Multimedia Mem Cfg
---------------------------------------------------------------------------
*/

const MM_CONFIG_T *MM_Config_GetInstance()
{
	static MM_CONFIG_T rMMConfig = {
	MM_GetMemCfg,
		    MM_GetReservedMemSAddr,
		    MM_GetReservedMemSize,
		    MM_GetAVPbbufSize,
		    MM_GetAPbbufSize,
		    MM_GetPbbufSlotSize,
		    MM_GetSPPbbufSize,
		    MM_GetSPPbbufSlotSize,
		    MM_GetVFIFOSize,
		    MM_GetHighBitrateVFIFOSize,
		    MM_GetSPFIFOSize, MM_GetVDECBUFSize, MM_GetAFIFOSize,};

	return &rMMConfig;
}
EXPORT_SYMBOL(MM_Config_GetInstance);

void OSE_PrintOSEMemoryCfg(void)
{
	OSE_MEM_LIST *pOSEMemList = (OSE_MEM_LIST *) OSE_HOUSEKEEPING_BUF_VA_START;

	pr_info("\t++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
	pr_info("\t+++ Chip Memory Size: 0x%08x.\r\n", OSE_GetChipMemSize());
	pr_info("\t+++ Video Start Address: 0x%p.\r\n", OSE_GetMMReservedMemStartAddr());
	pr_info("\t+++ Total Video Size: 0x%08x.\r\n", OSE_GetMMReservedMemSize());
	pr_info("\t++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
	pr_info("\t+++ Video PBbuf Size: 0x%08x.\r\n", OSE_GetPbbufInterleaveSize());
	pr_info("\t+++ Audio PBbuf Size: 0x%08x.\r\n", OSE_GetPbbufBadInterleaveSize());
	pr_info("\t+++ A/V PBbuf Slot Size: 0x%08x.\r\n", OSE_GetPbbufAVSlotSize());
	pr_info("\t+++ SubPic PBbuf Size: 0x%08x.\r\n", OSE_GetSPPbbufSize());
	pr_info("\t+++ SubPic PBbuf Slot Size: 0x%08x.\r\n", OSE_GetSPPbbufSlotSize());
	pr_info("\t+++ Video FIFO Size: 0x%08x.\r\n", OSE_GetVFIFOSize());
	pr_info("\t+++ SubPic FIFO Size: 0x%08x.\r\n", OSE_GetSPFIFOSize());
	pr_info("\t+++ Video Decoder Frame Buf Size: 0x%08x.\r\n", OSE_GetVdecBufSize());
	pr_info("\t++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
	PrintMemList(pOSEMemList);
	pr_info("\t++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
}
EXPORT_SYMBOL(OSE_PrintOSEMemoryCfg);
