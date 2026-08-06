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

#ifdef __linux__
#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/mm.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/semaphore.h>
#include <linux/compat.h>
#include <asm/uaccess.h>
#include "x_os.h"
#include "x_ver.h"
#include "x_assert.h"
#include "winutil.h"
#include <media/atc/memchk_cfg.h>
#include <media/atc/mm_debug.h>
#else
#include "x_typedef.h"
#include "x_assert.h"
#include "winutil.h"
#include "memchk_cfg.h"
#include "mm_debug.h"
#endif				/* __linux__ */

#include "memmanager.h"

#define  MEMMANAGER_SEMA_NAME  "MEMDMANGER_SEMA"

static alloc_node *g_pMemDbgHeap;
static struct semaphore m_csMemDbg;
static bool g_fgMemDbgInited = FALSE;
static void MemDBGDump(void);
static void destory_tree(alloc_node *pcur);
static void walk_alloc_tree(alloc_node *pcur, size_t *pttl);

bool MemDBGInit(void)
{
	if (g_fgMemDbgInited)
		return TRUE;

	/* RETAILMSG(1, (TEXT("[MEMDBG] Init\r\n"))); */
	pr_info("[MM_MEMDBG] MemDBGInit\r\n");

	/* InitializeCriticalSection((LPCRITICAL_SECTION)&m_csMemDbg); */
	/* RETAILMSG(1, (TEXT("[MEMDBG] CS CREATE SUCCESS\r\n"))); */
	init_MUTEX(&m_csMemDbg);

	g_fgMemDbgInited = TRUE;
	return TRUE;
}
EXPORT_SYMBOL(MemDBGInit);

void MemDBGUninit(void)
{
	if (!g_fgMemDbgInited)
		return;

	pr_info("[MM_MEMDBG] MemDBGUninit\r\n");
	MemDBGDump();

	/* EnterCriticalSection(&m_csMemDbg); */
	down(&m_csMemDbg);
	destory_tree(g_pMemDbgHeap);
	g_pMemDbgHeap = NULL;
	up(&m_csMemDbg);

	g_fgMemDbgInited = FALSE;
}
EXPORT_SYMBOL(MemDBGUninit);

static void BTreeInsert(alloc_node **pTree, alloc_node *pnode)
{
	alloc_node **ppuplink = pTree;
	alloc_node *pcur = NULL;

	if (NULL == pnode) {
		pr_info("[Memmanager][BTreeInsert] pnode= NULL.\r\n");
		return;
	}

	if (NULL == pTree) {
		pr_info("[Memmanager][BTreeInsert] pTree= NULL.\r\n");
		return;
	}

	if (NULL == *pTree) {
		*pTree = pnode;
		pr_info("[Memmanager]BTreeInsert firstnode %s, line %ud, len:%d, pvAddr:%p.\r\n",
			pnode->file, pnode->line, pnode->len, pnode->pvAddr);
		return;
	}

	pcur = *pTree;		/* pointer to the root */
	/* walk_alloc_tree(pcur, &ttl); */
	while (NULL != pcur) {
		if (pnode->pvAddr == pcur->pvAddr) {
			pr_info("[MM_MEMDBG][BTreeInsert]: duplicate memory allocated !current file: %s[%d] pnode(%p), pvAddr(%p) **\r\n",
			     pnode->file, pnode->line, pnode, pnode->pvAddr);

			MM_ASSERT(0);
			return;
		}
		if (pnode->pvAddr < pcur->pvAddr) {
			ppuplink = &(pcur->lptr);
			pcur = pcur->lptr;
		} else {
			ppuplink = &(pcur->rptr);
			pcur = pcur->rptr;
		}
	}
	*ppuplink = pnode;
	pr_debug("[Memmanager]BTreeInsert node len:%d, pvAddr:%p, line %ud, file: %s.\r\n",
		pnode->len, pnode->pvAddr, pnode->line, pnode->file);
}

void BTreeInsertLib(alloc_node *pnode)
{
	if (!g_fgMemDbgInited)
		MemDBGInit();

	down(&m_csMemDbg);
	BTreeInsert(&g_pMemDbgHeap, pnode);
	up(&m_csMemDbg);
}
EXPORT_SYMBOL(BTreeInsertLib);


static void *BTreeDel(alloc_node **pTree, void *p)
{
	uintptr_t ptrRetAddr = 0;
	alloc_node *pcur = NULL;
	alloc_node **ppuplink = pTree;

	if (NULL == p)
		return NULL;

	if (NULL == *pTree) {
		pr_err("[MM_MEMDBG][BTreeDel]: *** FATAL: delete with empty heap ! ***\r\n");
		return NULL;
	}

	pcur = *pTree;

	while (pcur) {
		void *pcurblk = (void *)(pcur->pvAddr);

		if (p == pcurblk) {
			u8 *pmem = (u8 *) p;

			ptrRetAddr = ((uintptr_t) pcur->pvAddr - sizeof(g_guard));
			if ((memcmp((u8 *) (pmem - sizeof(g_guard)), g_guard, sizeof(g_guard)) != 0)
					|| (memcmp((u8 *) (pmem + pcur->len), g_guard, sizeof(g_guard)) != 0)) {
				pr_err("[MM_MEMDBG][BTreeDel]*** FATAL: corrupted memory at %s[%d] %p\r\n",
						 pcur->file, pcur->line, p);
			}
			
			if ((NULL != pcur->lptr) && (NULL != pcur->rptr)) {
				alloc_node *pend = pcur->lptr;

				while (NULL != pend->rptr)
					pend = pend->rptr;
				*ppuplink = pcur->lptr;
				pend->rptr = pcur->rptr;
			} else {
				*ppuplink = (pcur->lptr) ? pcur->lptr : pcur->rptr;
			}
			vfree(pcur);	/* delete the node */
			return (void *)ptrRetAddr;
		} else if (p < pcurblk) {
			ppuplink = &pcur->lptr;
			pcur = pcur->lptr;
		} else {
			ppuplink = &pcur->rptr;
			pcur = pcur->rptr;
		}
	}

	pr_err("[MM_MEMDBG]*** FATAL: delete on unalloced memory  0x%p\r\n***", p);

	MM_ASSERT(0);
	return NULL;
}

#if CONFIG_COMPAT
static void *BTreeDel32(alloc_node **pTree, void *p)
{
	uintptr_t ptrRetAddr = 0;
	alloc_node *pcur = NULL;
	alloc_node **ppuplink = pTree;

	if (NULL == p)
		return NULL;

	if (NULL == *pTree) {
		pr_err("[MM_MEMDBG][BTreeDel]: *** FATAL: delete with empty heap ! ***\r\n");
		return NULL;
	}

	pcur = *pTree;

	while (pcur) {
		void *pcurblk = (void *)(pcur->pvAddr);

		if (p == pcurblk) {
			u8 *pmem = (u8 *) p;
			char szGuard[32];
			char *pszGuard64 = NULL;

			memset(szGuard, 0, sizeof(szGuard));
			ptrRetAddr = ((uintptr_t) pcur->pvAddr - sizeof(g_guard));

			pszGuard64 = (char __user *)compat_alloc_user_space(32);
			if (NULL == pszGuard64) {
				pr_err("[MM_MEMDBG][BTreeDel]: *** %s line %d fail in compat_alloc_user_space! ***\r\n",
					__func__, __LINE__);
			} else {
				memset(pszGuard64, 0, 32);
				if (copy_in_user(pszGuard64, (pmem - sizeof(g_guard)), sizeof(g_guard))) {
					pr_err("[MM_MEMDBG][BTreeDel]: *** %s line %d fail in copy_in_user! ***\r\n",
						__func__, __LINE__);
				} else {
					if (copy_from_user(szGuard, pszGuard64, sizeof(g_guard))) {
						pr_err("[MM_MEMDBG][BTreeDel]: *** %s line %d fail in copy_from_user! ***\r\n",
							__func__, __LINE__);
					} else {
						if (memcmp((u8 *)szGuard, g_guard, sizeof(g_guard)) != 0) {
							pr_err("[MM_MEMDBG][BTreeDel]*** 1 FATAL: corrupted memory at %p line[%d] %s \r\n",
									 p,  pcur->line, pcur->file);
						}
				  }
				}
				memset(pszGuard64, 0, 32);
				if (copy_in_user(pszGuard64, (pmem + pcur->len), sizeof(g_guard))) {
					pr_err("[MM_MEMDBG][BTreeDel]: *** %s line %d fail in copy_in_user! ***\r\n",
						__func__, __LINE__);
				} else {
					if (copy_from_user(szGuard, pszGuard64, sizeof(g_guard))) {
						pr_err("[MM_MEMDBG][BTreeDel]: *** %s line %d fail in copy_from_user! ***\r\n",
							__func__, __LINE__);
					} else {
						if (memcmp((u8 *)szGuard, g_guard, sizeof(g_guard)) != 0) {
							pr_err("[MM_MEMDBG][BTreeDel]*** 2 FATAL: corrupted memory at %p line[%d] %s \r\n",
									 p,  pcur->line, pcur->file);
						}
				  }
				}			
		  }
			
			if ((NULL != pcur->lptr) && (NULL != pcur->rptr)) {
				alloc_node *pend = pcur->lptr;

				while (NULL != pend->rptr)
					pend = pend->rptr;
				*ppuplink = pcur->lptr;
				pend->rptr = pcur->rptr;
			} else {
				*ppuplink = (pcur->lptr) ? pcur->lptr : pcur->rptr;
			}
			vfree(pcur);	/* delete the node */
			return (void *)ptrRetAddr;
		} else if (p < pcurblk) {
			ppuplink = &pcur->lptr;
			pcur = pcur->lptr;
		} else {
			ppuplink = &pcur->rptr;
			pcur = pcur->rptr;
		}
	}

	pr_err("[MM_MEMDBG]*** FATAL: delete on unalloced memory  0x%p\r\n***", p);

	MM_ASSERT(0);
	return NULL;
}
#endif

bool BTreeDelBufNotFree(void *p)
{
	void * pvRetAddr = NULL;

	if (!g_fgMemDbgInited)
		MemDBGInit();


	down(&m_csMemDbg);
	/* pr_err("[MM_MEMDBG][BTreeDelBufNotFree]free memory 0x%p \r\n", p); */

	pvRetAddr = BTreeDel(&g_pMemDbgHeap, p);
	if (NULL == pvRetAddr) {
		up(&m_csMemDbg);
		return FALSE;
	}
	up(&m_csMemDbg);
	return TRUE;
}
EXPORT_SYMBOL(BTreeDelBufNotFree);


static u32 BTreeGetMemSize(alloc_node **pTree, void *p)
{
	alloc_node *pcur = NULL;
	alloc_node **ppuplink = pTree;

	if (NULL == p)
		return 0;
	if (NULL == *pTree) {
		pr_err("[MM_MEMDBG][BTreeGetMemSize]: *** FATAL: delete with empty heap ! ***\r\n");
		MM_ASSERT(0);
		return 0;
	}
	pcur = *pTree;
	while (pcur) {
		void *pcurblk = (void *)(pcur->pvAddr);

		if (p == pcurblk) {
			u8 *pmem = (u8 *) p;

			if ((memcmp(pmem - sizeof(g_guard), g_guard, sizeof(g_guard)) != 0)
			    || (memcmp(pmem + pcur->len, g_guard, sizeof(g_guard)) != 0)) {
				pr_err
				    ("[MM_MEMDBG][BTreeGetMemSize]*** FATAL: corrupted memory at %s[%ud] 0x%p\r\n",
				     pcur->file, pcur->line, p);
				MM_ASSERT(0);
			}

			return pcur->len;
		} else if (p < pcurblk) {
			ppuplink = &pcur->lptr;
			pcur = pcur->lptr;
		} else {
			ppuplink = &pcur->rptr;
			pcur = pcur->rptr;
		}
	}

	pr_err("[MM_MEMDBG]*** FATAL: delete on unalloced memory  0x%p\r\n***", p);
	MM_ASSERT(0);
	return 0;
}

static void dumplog(alloc_node *pcur)
{
	if (NULL == pcur)
		return;

	/* write to log file */
	pr_info("[MM_MEMDBG]%s[%d] alloc %ud bytes at %p\r\n",
		pcur->file, pcur->line, pcur->len, pcur->pvAddr);
}

static void walk_alloc_tree(alloc_node *pcur, size_t *pttl)
{
	if (pcur) {
		walk_alloc_tree(pcur->lptr, pttl);
		dumplog(pcur);
		*pttl += pcur->len;
		walk_alloc_tree(pcur->rptr, pttl);
	}
}

static void destory_tree(alloc_node *pcur)
{
	if (pcur) {
		destory_tree(pcur->lptr);
		destory_tree(pcur->rptr);
		vfree(pcur);
	}
}

void MemDBGDump(void)
{
	down(&m_csMemDbg);
	if (g_pMemDbgHeap) {
		size_t ttl = 0;

		pr_info("[MemManager]Memory leaks detected\r\n");
		pr_info("=====================\r\n");
		walk_alloc_tree(g_pMemDbgHeap, &ttl);
		pr_info("\r\n");
		pr_info("=====================\r\n");
		pr_info("Total bytes: %d\r\n", ttl);
		pr_info("=====================\r\n");
	} else {
		pr_info("[MM_MEMDBG] No Memory leaks detected!\r\n");
	}

	/* LeaveCriticalSection(&m_csMemDbg); */
	up(&m_csMemDbg);
}

void *MemDbgAlloc(size_t nSize, char *lpszFileName, int nLine)
{
	alloc_node *prNode = NULL;
	u8 *pmem = NULL;
	void *p = NULL;

	/* if mem manager drvier is closed, then open it. */
	if (!g_fgMemDbgInited)
		MemDBGInit();

	if (0 == nSize) {
		MM_ASSERT(0);
		pr_err("[MM_MEMDBG] malloc memory size is invalid(0): %s, line %d\r\n",
		       lpszFileName, nLine);
		nSize = 1;
	}
	/* pr_err("[MM_MEMDBG][MemDbgFree]free memory size:0x%x, %s, line %d\r\n", nSize, lpszFileName,nLine); */
	prNode = vmalloc(sizeof(alloc_node));
	if (NULL == prNode)
		return NULL;

	p = vmalloc(nSize + 2 * sizeof(g_guard));
	if (NULL != p) {
		/* Add CRC in the header and tailer of allocoated memory */
		memcpy(p, g_guard, sizeof(g_guard));
		pmem = (u8 *) p + sizeof(g_guard);
		/* mm_memset( pmem, 0, nSize); */
		memcpy(pmem + nSize, g_guard, sizeof(g_guard));

		memset(prNode, 0, sizeof(alloc_node));
		/* save the allocator info */
		prNode->lptr = prNode->rptr = NULL;
		prNode->len = nSize;
		prNode->line = nLine;
		prNode->pvAddr = (void *)pmem;

		strcpy(prNode->file, lpszFileName);

		/* EnterCriticalSection(&m_csMemDbg); */
		down(&m_csMemDbg);
		BTreeInsert(&g_pMemDbgHeap, prNode);
		up(&m_csMemDbg);
	} else {
		vfree(prNode);
	}
	return pmem;
}
EXPORT_SYMBOL(MemDbgAlloc);
void MemDbgFree(void *p, char *lpszFileName, int nLine)
{
	void *pvOutAddr = NULL;

	if (!g_fgMemDbgInited)
		MemDBGInit();
	down(&m_csMemDbg);
	/* pr_err("[MM_MEMDBG][MemDbgFree]free memory 0x%p: %s, line %d\r\n", p, lpszFileName,nLine); */
	pvOutAddr = BTreeDel(&g_pMemDbgHeap, p);
	if (pvOutAddr)
		vfree(pvOutAddr);
	up(&m_csMemDbg);
}
EXPORT_SYMBOL(MemDbgFree);

bool MM_MemDbg_InsertNode(void *pvInsertNode)
{
	alloc_node *prNode = NULL;

	if (NULL == pvInsertNode) {
		pr_err("[Memmanager][IOCTL_InsertNode]input parameter wrong!\r\n");
		return FALSE;
	}

	prNode = vmalloc(sizeof(alloc_node));
	if (NULL == prNode)
		return FALSE;

	memcpy(prNode, pvInsertNode, sizeof(alloc_node));

	down(&m_csMemDbg);

	BTreeInsert(&g_pMemDbgHeap, prNode);
	up(&m_csMemDbg);

	return TRUE;
}


bool MM_MemDbg_RemoveNode(void *pvNode, void **pvAddr)
{
	if ((NULL == pvNode) || (NULL == pvAddr))
		return FALSE;

	down(&m_csMemDbg);
	*pvAddr = BTreeDel(&g_pMemDbgHeap, pvNode);
	up(&m_csMemDbg);

	return TRUE;
}

bool MM_MemDbg_RemoveNode32(void *pvNode, void **pvAddr)
{
	if ((NULL == pvNode) || (NULL == pvAddr))
		return FALSE;

	down(&m_csMemDbg);
#if CONFIG_COMPAT
	*pvAddr = BTreeDel32(&g_pMemDbgHeap, pvNode);
#else
	*pvAddr = BTreeDel(&g_pMemDbgHeap, pvNode);
#endif
	up(&m_csMemDbg);

	return TRUE;
}

bool MM_MemDbg_Dump(void)
{
	MemDBGDump();
	return TRUE;
}

bool MM_MemDbg_Flush(void)
{
	down(&m_csMemDbg);

	destory_tree(g_pMemDbgHeap);
	g_pMemDbgHeap = NULL;
	up(&m_csMemDbg);

	return TRUE;
}

bool MM_MemDbg_GetNodeSize(void *pvNode, u32 *pu4Sz)
{
	if ((NULL == pvNode) || (NULL == pu4Sz)) {
		pr_err("[Memmanager] %s fail for invalid args!\r\n", __func__);
		return FALSE;
	}

	down(&m_csMemDbg);

	*pu4Sz = BTreeGetMemSize(&g_pMemDbgHeap, pvNode);
	up(&m_csMemDbg);

	return TRUE;
}

void MM_MemDbg_Init(void)
{
	MemDBGInit();
	pr_debug("[MM_MEMDBG] - %s\r\n", __func__);
}


void MM_MemDbg_Deinit(void)
{
	MemDBGUninit();
	pr_debug("[MM_MEMDBG] - %s\r\n", __func__);
}


bool MM_MemDbg_Open(void)
{
	pr_info("[MM_MEMDBG] - %s\r\n", __func__);
	return TRUE;
}


bool MM_MemDbg_Close(void)
{
	pr_info("[MM_MEMDBG] - %s, g_fgMemDbgInited: %d\r\n",
		__func__, (g_fgMemDbgInited ? 1 : 0));
	/* MemDBGUninit(); */
	if (!g_fgMemDbgInited)
		return FALSE;

	pr_info("[MM_MEMDBG] - %s --> MemDBGDump\r\n", __func__);
	MemDBGDump();

	return TRUE;
}
