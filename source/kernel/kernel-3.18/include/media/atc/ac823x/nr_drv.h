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
/*
 *Department: ATC-SD-SD2
 *
 *Aurthor: yezhiqiang(atc0064)
 *
 *Date: 2017-03-10
 */

#ifndef _NR_DRV_H_
#define _NR_DRV_H_

#ifndef __ARM2__
#include <linux/spinlock.h>
#include <linux/spinlock_types.h>
#endif

#if 0
typedef enum {
	SRC_TYPE_UNKOWN,
	SRC_TYPE_CVBS,
	SRC_TYPE_YPBPR,
	SRC_TYPE_VGA,
	SRC_TYPE_MAX
} NR_SRC_TYPE_E;
#endif

typedef void (*get_nr_buffer_index)(unsigned int *bufindex);
typedef struct _NR_PRM_T_
{
	unsigned char u1WchId;
	unsigned char fgBypassEn;  
	unsigned char fgBurstRdEn;
	unsigned char u1AddrSwapMode;
	unsigned char u1FrameMode;
	unsigned char u1DemoMode;

	unsigned int u4PicWidth;
	unsigned int u4PicHeight;
	unsigned int u4FrameWidth;
	unsigned int u4FrameHeight;

	unsigned int u4Strength;
	unsigned int u4MNRStrength;  
	unsigned int u4FNRStrength;
	unsigned int u4BNRStrength;

	unsigned char fgRangeRemapYEn;
	unsigned char fgRangeRemapUVEn;
	unsigned int u4RangeMapY;
	unsigned int u4RangeMapUV;
	unsigned char fgNoiseMeterEn;
	unsigned char fgUseBlockMeter;

	get_nr_buffer_index GetNrBufIndx;
}NR_PRM_T;

typedef struct _NR_BUFFER_T_{
	unsigned long  u4CurrRdYAddr;   //Read Y Start
	unsigned long  u4CurrRdCAddr;	 //Read C Start
	unsigned long  u4LastRdYAddr;
	unsigned long  u4LastRdCAddr;
	unsigned long  u4CurrWrYAddr; //Write Y Start
	unsigned long  u4CurrWrCAddr; //Write C Start
} NR_BUFFER_T;

#define NR_BUF_MAX_CNT		5
typedef struct {
	unsigned int u4BufIdx;
	unsigned long u4YBuf[NR_BUF_MAX_CNT];
	unsigned long u4CBuf[NR_BUF_MAX_CNT];
} NR_ALLC_BUF_T, *PNR_ALLC_BUF_T;

typedef struct _nrLock {
#ifndef __ARM2__
	spinlock_t lock;
#else
	unsigned int lock;
#endif
	unsigned long flags;
	unsigned int u4LockCnt;
} nrLock;


#define VIRT_TO_BUS(vaddr)	(((unsigned long)__pa(vaddr)) - 0x100000000L)
#define BUS_TO_VIRT(baddr)	__va((unsigned long)(baddr) + 0x100000000L)

#ifdef __ARM2__

typedef int irqreturn_t;
#define IRQ_HANDLED 0x1;

typedef int (*irq_handler_t)(int u4Vector, void * dev_id);
#endif

#endif
