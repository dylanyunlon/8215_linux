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
 * @file dmx_pvr_ddi.c
 *
 * @par Project
 *	  MT3360
 *
 * @par Description
 *
 *
 * @par Author_Name
 *	  Shuhui Zhang
 *
 */

/*-----------------------------------------------------------------------------*/
/* Include files*/
/*-----------------------------------------------------------------------------*/
#ifdef __linux__
#include <linux/mm.h>
#include <linux/errno.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <mach/base_regs.h>
#include <mach/ac83xx_basic.h>
#include <mach/ac83xx_gpio_pinmux.h>
#include <mach/irqs_vector.h>
#else
#include "irqs_vector.h"
#endif /* __linux__*/

#include "x_os.h"
#include "x_rtos.h"
#include "x_assert.h"
#include "x_hal_ic.h"
#include "x_ckgen_8317.h"
#include "x_ckgen.h"
#include "dmx_mem.h"
#include "dmx_pvr_if.h"
#include "dmx_pvr.h"
#include "dmx_pvr_ddi.h"
#include "dmx_psr_util.h"

#ifndef __linux__
#pragma warning(disable : 4127) /*disable warning C4127: conditional expression is constant*/
#endif

/*-----------------------------------------------------------------------------*/
/* Configurations*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Constant definitions*/
/*-----------------------------------------------------------------------------*/
#define	DDI_MAX_PACKET_SIZE			255

/*-----------------------------------------------------------------------------*/
/* Type definitions*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Macro definitions*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Static variables*/
/*-----------------------------------------------------------------------------*/
static PVR_DDI_STRUCT_T _rPvrDDI;

/*-----------------------------------------------------------------------------*/
/* Static functions*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* The caller of this function must ensure that prDDI is valid.*/
/* This function must ensures that*/
/* (1) _rDmxDDI.u4Threshold is a multiple of 16 bytes*/
/* (2) _rPvrDDI.ptrRp and _rPvrDDI.ptrWp are aligned to the proper boundaries.*/
/*-----------------------------------------------------------------------------*/
static bool _DDI_AllocBuf(const PVR_DDI_T *prDDI)
{
	uintptr_t ptrBufStart =  0, ptrBufEnd, ptrPhyBufStart, ptrPhyBufEnd;

	if (NULL == prDDI) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail for invalid args.\r\n"),
			DMX_FUNC_NAME);
		DMX_ASSERT(FALSE);
		return FALSE;
	}

	if ((0 != _rPvrDDI.ptrBufAddr) || (0 != _rPvrDDI.u4BufSize)) {
		PVR_LOG_ERR(TEXT("[PVR] %s line %d fail for DDI Buffer had been allocated.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return FALSE;
	}

	if (!_PVR_IsAligned(prDDI->u4BufSize, DDI_BUF_ALIGNMENT)) {
		PVR_LOG_ERR(TEXT("%s line %d fail for The requested DDI buffer size(0x%08x) is not aligned.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prDDI->u4BufSize);
		return FALSE;
	}

	if (!_PVR_IsAligned(prDDI->u4Threshold, DDI_BUF_ALIGNMENT)) {
		PVR_LOG_ERR(TEXT("%s line %d fail for The requested threshold size(0x%08x) is not aligned.\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prDDI->u4Threshold);
		return FALSE;
	}

	if (prDDI->fgAllocBuf) {
#ifdef __linux__
		DMX_NewHwAlignMemory(prDDI->u4BufSize, DDI_BUF_ALIGNMENT, ptrBufStart);
#else
		DMX_NewHwAlignMemory(prDDI->u4BufSize, DDI_BUF_ALIGNMENT, (void *)ptrBufStart);
#endif /* #ifdef __linux__*/
		if (ptrBufStart == 0) {
			PVR_LOG_ERR(TEXT("%s line %d fail for Memory allocation failed!\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			return FALSE;
		}
	} else {
		if (!_PVR_IsAligned(prDDI->ptrBufAddr, DDI_BUF_ALIGNMENT)) {
			PVR_LOG_ERR(TEXT("%s line %d fail for The DDI buffer address is not aligned.\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			return FALSE;
		}
		ptrBufStart = prDDI->ptrBufAddr;
	}

	ptrBufEnd	= ptrBufStart + prDDI->u4BufSize;
	ptrPhyBufStart	= DMX_PHYSICAL(ptrBufStart);
	ptrPhyBufEnd	= DMX_PHYSICAL(ptrBufEnd);

	_PVR_Lock();
	_rPvrDDI.fgAllocBuf	= prDDI->fgAllocBuf;	/* Buffer has been allocated*/
	_rPvrDDI.ptrBufAddr	= ptrBufStart;
	_rPvrDDI.u4BufSize	= prDDI->u4BufSize;
	_rPvrDDI.ptrRp		= ptrBufStart;
	_rPvrDDI.ptrWp		= ptrBufStart;
	_rPvrDDI.u4Threshold	= prDDI->u4Threshold;
	DDI_WRITE32(DDI_REG_DMA_BUF_START, ptrPhyBufStart);		/* Buffer start*/
	DDI_WRITE32(DDI_REG_DMA_BUF_END, ptrPhyBufEnd);			/* Buffer end*/
	DDI_WRITE32(DDI_REG_DMA_RP_INIT, ptrPhyBufStart);		/* Initial RP*/
	DDI_WRITE32(DDI_REG_DMA_WP, ptrPhyBufStart);			/* Write pointer*/
	DDI_WRITE32(DDI_REG_DMA_AP, ptrPhyBufStart);			/* Alert pointer*/
	/* The update of RP will be done by hardware when DMA is activiated.*/
	_PVR_Unlock();

	PVR_LOG_TRACE(TEXT("%s line %d -- DDI Src Buf's SA(0x%08x), EA(0x%08x), ")
		TEXT("InitRP(0x%08x), WP(0x%08x), RP(0x%08x)\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, ptrBufStart, ptrBufEnd, ptrBufStart, ptrBufStart, ptrBufStart);

	return TRUE;
}


/*-----------------------------------------------------------------------------*/
/* Free the resources managed by software.*/
static bool _DDI_FreeBuf(void)
{
	if (_rPvrDDI.fgAllocBuf)
		DMX_FreeHwMemory((void *)_rPvrDDI.ptrBufAddr);

	_PVR_Lock();
	_rPvrDDI.fgAllocBuf = FALSE;
	_rPvrDDI.ptrBufAddr = 0;
	_rPvrDDI.u4BufSize = 0;
	_rPvrDDI.ptrRp = 0;
	_rPvrDDI.ptrWp = 0;
	_PVR_Unlock();

	return TRUE;
}

/*-----------------------------------------------------------------------------*/
/** _DDI_Reset*/
/* Reset the hardware including the hardware buffer pointers.*/
/*-----------------------------------------------------------------------------*/
static void _DDI_Reset(bool fgResetHardware)
{
	u32 u4Reg;
	PVR_DDI_PORT_T ePort;

	FUNC_ENTRY;

	ePort = _PVR_DDI_GetPort();

	PVR_LOG_DBG(TEXT("[PVR] %s line %d -- Origin DDI output Port: %d!\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, ePort);

	smp_mb();

	_PVR_Lock();

	if (fgResetHardware) {
		/* Reset DDI*/
		/* 0x3000F: DDI(including Rate control circuit and DDI to demuxer*/
		/*interface) soft reset, turn on DDI DRAM clock.*/
		DDI_WRITE32(DDI_REG_GLOBAL_CTRL, 0x3000F);
		smp_mb();
		/* 0x20F: Release soft reset, turn on DDI DRAM clock.*/
		DDI_WRITE32(DDI_REG_GLOBAL_CTRL, 0x20F);
	}

	smp_mb();

	DDI_WRITE32(DDI_REG_DCR_INT_MASK, 0x7);		/* Disable all DMA interrupts*/
	smp_mb();
	DDI_WRITE32(DDI_REG_DCR_INT_CLR, 0x7);		/* Clear all DMA interrupts*/

	smp_mb();
	/* Select port 1(to DMX TS index 1 without passing through the framer)*/
	u4Reg = DDI_READ32(DDI_REG_DMX_RX_CTRL) & 0x10;
	u4Reg |= 0x4; /* set DDI to demux Output Delay Byte to 4*/
	/* 0xF set DDI to demux Output Delay Byte to 15*/
	smp_mb();
	DDI_WRITE32(DDI_REG_DMX_RX_CTRL, u4Reg);

	smp_mb();
	DDI_WRITE32(DDI_REG_DMA_CTRL, 4);		/* Stop the DMA operation */

	smp_mb();
	DDI_WRITE32(DDI_REG_DMA_BUF_START, 0);		/* Start pointer*/
	DDI_WRITE32(DDI_REG_DMA_BUF_END, 0);		/* End pointer*/
	DDI_WRITE32(DDI_REG_DMA_RP_INIT, 0);		/* Initial Read pointer*/
	DDI_WRITE32(DDI_REG_DMA_WP, 0);			/* Write pointer*/
	DDI_WRITE32(DDI_REG_DMA_AP, 0);			/* Alert pointer*/
	/* The update of RP will be done by hardware when DMA is activiated.*/

	smp_mb();
	/* Reset RP to 0.*/
	DDI_WRITE32(DDI_REG_DMA_CTRL, 5);		/* Start the DMA operation*/
	smp_mb();
	DDI_WRITE32(DDI_REG_DMA_CTRL, 4);		/* Stop the DMA operation*/

	smp_mb();
	/* Clear the interrupt generated when RP is reset to 0 above.*/
	DDI_WRITE32(DDI_REG_DCR_INT_CLR, 0x7);		/* Clear all DMA interrupts*/

	smp_mb();
	_PVR_Unlock();

	smp_mb();

	_PVR_DDI_SetPort(ePort);

	FUNC_EXIT;
}

/*-----------------------------------------------------------------------------*/
/* Inter-file functions*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/** _PVR_DDI_Init\*/
/* Disable empty/alert interrupts here, and then enable them when data is*/
/* written to the buffer.  The interrupts also need to be disabled when the*/
/* Empty or Alert interrupts are triggered.*/
/*-----------------------------------------------------------------------------*/
bool _PVR_DDI_Init(void)
{
	FUNC_ENTRY;

	_PVR_Lock();
	mm_memset(&_rPvrDDI, 0, sizeof(PVR_DDI_STRUCT_T));
	_rPvrDDI.eMode = PVR_DDI_MODE_STREAM;	/* default transfer mode*/
	_rPvrDDI.eState = PVR_DDI_STOP;		/* in case enum is changed carelessly*/
	_rPvrDDI.eDDIPortType = PVR_DDI_PORT_NOT_FRAMER;
	_rPvrDDI.u1PacketSize  = DDI_PACKET_SIZE;
	_rPvrDDI.u1SyncOffset  = 0;
	_rPvrDDI.fgDDIISRInited  = FALSE;
	_rPvrDDI.hDDINotifyEG  = NULL_HANDLE;
	_rPvrDDI.pfnDDINotify  = NULL;
	_rPvrDDI.fgAllocBuf = FALSE;

	_PVR_Unlock();

	smp_mb();

	_DDI_Reset(TRUE);

	smp_mb();

	if (!_PVR_DDI_SetPacketSize(DDI_PACKET_SIZE)) {
		PVR_LOG_ERR(TEXT("[PVR] %s line %d fail for Cannot set DDI packet size!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		FUNC_EXIT;
		return FALSE;
	}

	smp_mb();

	if (!_PVR_DDI_InitISR()) {
		PVR_LOG_ERR(TEXT("[PVR] %s line %d fail in _PVR_DDI_InitISR!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		FUNC_EXIT;
		return FALSE;
	}

	FUNC_EXIT;

	return TRUE;
}

/*-----------------------------------------------------------------------------*/
/** _PVR_DDI_DeInit*/
/*-----------------------------------------------------------------------------*/
bool _PVR_DDI_DeInit(void)
{
	if (PVR_DDI_STOP != _rPvrDDI.eState) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail, Please stop DDI before free its buffer\r\n"),
			DMX_FUNC_NAME);
		return FALSE;
	}

	_DDI_Reset(FALSE);	/* Only reset some HW registers (RIP, WP, AP, and etc).*/

	if (_rPvrDDI.hDDINotifyEG) {
		x_ev_group_set_event(_rPvrDDI.hDDINotifyEG, PVR_DDI_NOTIFY_EV_BUF_EXIT, X_EV_OP_OR);
		x_ev_group_delete(_rPvrDDI.hDDINotifyEG);
		_rPvrDDI.hDDINotifyEG = NULL_HANDLE;
	}

#if !DMX_SUPPORT_DEVICE_TREE
	_PVR_DDI_UninitISR();
#endif /* !DMX_SUPPORT_DEVICE_TREE */

	if (!_DDI_FreeBuf())
		return FALSE;	/* Current implementation will never fall into here.*/

	return TRUE;
}

/*-----------------------------------------------------------------------------*/
/** _PVR_DDI_Set*/
/* Todo: consider to let users turn on/off "PCR Rate Compensation/Control".*/
/*-----------------------------------------------------------------------------*/
bool _PVR_DDI_Set(u32 u4Flags, const PVR_DDI_T *prDDI)
{
	if (NULL == prDDI) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail for invalid args.\r\n"),
			DMX_FUNC_NAME);
		return FALSE;
	}

	if (u4Flags == PVR_DDI_FLAG_NONE) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail for invaid args(u4Flags: %d)\r\n"),
			DMX_FUNC_NAME, u4Flags);
		return FALSE;
	}

	_PVR_Lock();

	if (u4Flags & PVR_DDI_FLAG_MODE)
		_rPvrDDI.eMode = prDDI->eMode;

	if (u4Flags & PVR_DDI_FLAG_ALLOCBUF) {
		if (PVR_DDI_STOP != _rPvrDDI.eState) {
			_PVR_Unlock();
			PVR_LOG_ERR(TEXT("%s line %d fail, Please stop DDI before memory allocation.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			return FALSE;
		}

		_PVR_Unlock();
		if (!_DDI_AllocBuf(prDDI)) {
			PVR_LOG_ERR(TEXT("%s line %d fail in _DDI_AllocBuf\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			return FALSE;
		}
		_PVR_Lock();
	}

	if (u4Flags & PVR_DDI_FLAG_CALLBACK) {
		DMX_ASSERT(prDDI->pfnDDINotify != NULL);
		_rPvrDDI.pfnDDINotify = prDDI->pfnDDINotify;
	}

	if (u4Flags & PVR_DDI_FLAG_RATE) {
		/* Rate = (N / M) Mbits/sec*/
		_rPvrDDI.u4RateN = prDDI->u4RateN;
		_rPvrDDI.u4RateM = prDDI->u4RateM;
		DDI_WRITE32(DDI_REG_PERIOD_M, _rPvrDDI.u4RateM);
		DDI_WRITE32(DDI_REG_PERIOD_N, _rPvrDDI.u4RateN);
		DDI_WRITE32(DDI_REG_RATE_CMD, 0x3);	/* Update M, N; Clear period counter*/
	}

	if (u4Flags & PVR_DDI_FLAG_SET_BUF) {
		/*Max Xia Added, Reference to Func --_DDI_AllocBuf*/
		/* Buffer start*/
		DDI_WRITE32(DDI_REG_DMA_BUF_START, DMX_PHYSICAL(prDDI->ptrBufAddr));
		/* Buffer end*/
		DDI_WRITE32(DDI_REG_DMA_BUF_END, DMX_PHYSICAL(prDDI->ptrBufAddr) + prDDI->u4BufSize);
		/* Initial RP*/
		DDI_WRITE32(DDI_REG_DMA_RP_INIT, DMX_PHYSICAL(prDDI->ptrBufAddr));
		/* Write pointer*/
		DDI_WRITE32(DDI_REG_DMA_WP, DMX_PHYSICAL(prDDI->ptrBufAddr));
	}

	_PVR_Unlock();

	return TRUE;
}


/*-----------------------------------------------------------------------------*/
/** _PVR_DDI_Get*/
/*-----------------------------------------------------------------------------*/
bool _PVR_DDI_Get(u32 u4Flags, PVR_DDI_T *prDDI)
{
	if (NULL == prDDI) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail for invalid args.\r\n"),
			DMX_FUNC_NAME);
		return FALSE;
	}

	if (u4Flags == PVR_DDI_FLAG_NONE) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail for invaid args(u4Flags)\r\n"),
			DMX_FUNC_NAME, u4Flags);
		return FALSE;
	}

	if (u4Flags & PVR_DDI_FLAG_MODE)
		prDDI->eMode = _rPvrDDI.eMode;

	if (u4Flags & PVR_DDI_FLAG_ALLOCBUF) {
		prDDI->fgAllocBuf = _rPvrDDI.fgAllocBuf;
		prDDI->ptrBufAddr = _rPvrDDI.ptrBufAddr;
		prDDI->u4BufSize = _rPvrDDI.u4BufSize;
		prDDI->u4Threshold = _rPvrDDI.u4Threshold;
	}

	if (u4Flags & PVR_DDI_FLAG_CALLBACK)
		prDDI->pfnDDINotify = _rPvrDDI.pfnDDINotify;

	if (u4Flags & PVR_DDI_FLAG_RATE) {
		prDDI->u4RateN = _rPvrDDI.u4RateN;
		prDDI->u4RateM = _rPvrDDI.u4RateM;
	}

	if (u4Flags & PVR_DDI_FLAG_DATA_SIZE) {
		u32 ptrBufStart, u4BufEnd, u4BufSize, ptrRp, ptrWp;

		if (!_rPvrDDI.fgAllocBuf) {
			prDDI->u4DataSize = 0;
		} else {
			ptrBufStart = DMX_NONCACHE(DDI_READ32(DDI_REG_DMA_BUF_START));
			u4BufEnd   = DMX_NONCACHE(DDI_READ32(DDI_REG_DMA_BUF_END));
			u4BufSize = u4BufEnd - ptrBufStart;
			ptrWp = DMX_NONCACHE(DDI_READ32(DDI_REG_DMA_WP));
			DMX_ASSERT((ptrBufStart <= ptrWp) && (ptrWp < u4BufEnd));
			ptrRp = DMX_NONCACHE(DDI_READ32(DDI_REG_DMA_RP));
			if (ptrRp == 0) {
				prDDI->u4DataSize = ptrWp - ptrBufStart;
			} else {
				DMX_ASSERT((ptrBufStart <= ptrRp) && (ptrRp < u4BufEnd));
				prDDI->u4DataSize = (ptrWp >= ptrRp) ?
									(ptrWp - ptrRp) :
									((ptrWp + u4BufSize) - ptrRp);
			}
		}
	}

	return TRUE;
}


/*-----------------------------------------------------------------------------*/
/** _PVR_DDI_SetPort*/
/*-----------------------------------------------------------------------------*/
void _PVR_DDI_SetPort(PVR_DDI_PORT_T ePort)
{
	u32 u4Reg;
	u32 u4Ctrl;
	u8 u1TsIdx;

	PVR_LOG_DBG(TEXT("[DDI] %s(ePort: %d) -- enter\r\n"),
		DMX_FUNC_NAME, ePort);

	smp_mb();

	u4Reg = DDI_READ32(DDI_REG_DMX_RX_CTRL);

	if (ePort == PVR_DDI_PORT_FRAMER0) {
		/*Framer 0*/
		u1TsIdx = 0;
		u4Reg &= ~0x10;

		smp_mb();

		_PVR_SetFrontEndEx(0, PVR_FE_DDI);
	} else if (ePort == PVR_DDI_PORT_FRAMER1) {
		/*Framer 1*/
		u1TsIdx = 1;
		u4Reg &= ~0x10;

		smp_mb();
		_PVR_SetFrontEndEx(1, PVR_FE_DDI);
	} else if (ePort == PVR_DDI_PORT_NOT_FRAMER) {
		/* TSIdx3 Bypass*/
		u1TsIdx = 3;
		u4Reg |= 0x10;

		smp_mb();

		u4Ctrl = DMXCMD_READ32(PVR_REG_DBM_CONTROL);
		u4Ctrl |= (1<<29);
		smp_mb();
		DMXCMD_WRITE32(PVR_REG_DBM_CONTROL, u4Ctrl);
		smp_mb();
		DMXCMD_WRITE32(PVR_REG_DBM_BYPASS_PID_4, 0x20);		/* Steer to FTuP*/
	} else {
		/* kind of redundant: C type-checking mechanism shall warn us*/
		PVR_LOG_ERR(TEXT("[DDI] %s(ePort: %d) fail for invalid ePort\r\n"),
			DMX_FUNC_NAME, ePort);
		return;
	}

	_PVR_Lock();
	DDI_WRITE32(DDI_REG_DMX_RX_CTRL, u4Reg);
	_rPvrDDI.eDDIPortType = ePort;
	_PVR_Unlock();

	PVR_LOG_DBG(TEXT("[DDI] %s(ePort: %d) -- success\r\n"),
		DMX_FUNC_NAME, ePort);
}


/*-----------------------------------------------------------------------------*/
/** _PVR_DDI_SetPortEx*/
/*-----------------------------------------------------------------------------*/
void _PVR_DDI_SetPortEx(PVR_DDI_PORT_T ePort, u8 u1Pidx, bool fgForce,  bool fgTs)
{
	u32 u4Reg;
	u8 u1TsIdx;
	PVR_DDI_PORT_T ePrePort;

	if (!fgForce) {
		_PVR_Lock();
		ePrePort = _rPvrDDI.eDDIPortType;
		_PVR_Unlock();
		if (ePrePort == ePort) {
			PVR_LOG_DBG(TEXT("[DMX][EMU] %s(ePort: %d, fgForce: %s) -- exit 1\r\n"),
				DMX_FUNC_NAME, ePort, (fgForce ? "TRUE" : "FALSE"));
			return;
		}
	}

	u4Reg = DDI_READ32(DDI_REG_DMX_RX_CTRL);

	if (ePort == PVR_DDI_PORT_FRAMER0) {
		u1TsIdx = 0;
		u4Reg &= ~0x10;

		_PVR_SetFrontEndEx(0, PVR_FE_DDI);

		if (fgTs) {
			DMXCMD_WRITE32(PVR_REG_DBM_BYPASS_PID, 0);

			DMXCMD_WRITE32(PVR_REG_DBM_CONTROL, DMXCMD_READ32(PVR_REG_DBM_CONTROL) & ~(1<<26));

			_PVR_SetFramerMode(PVR_FMR0, PVR_FRAMER_PARALLEL, TRUE, TRUE);

			/*enable error handling*/
			_PVR_SetFramerPacketErrorHandling(0, TRUE, 0xF0C0);
		} else {
			u32 u4Ctrl;

			DMXCMD_WRITE32(PVR_REG_DBM_BYPASS_PID, u1Pidx << 16 | 0x20);

			DMXCMD_WRITE32(PVR_REG_DBM_CONTROL, DMXCMD_READ32(PVR_REG_DBM_CONTROL) | (1<<26));

			_PVR_SetFramerMode(PVR_FMR0, PVR_FRAMER_PARALLEL, TRUE, TRUE);

			/* Framer 0 Reset*/
			u4Ctrl = DMXCMD_READ32(PVR_REG_FRAMER_CONTROL) & (~0x2000);
			u4Ctrl |= 0x1000;
			DMXCMD_WRITE32(PVR_REG_FRAMER_CONTROL, u4Ctrl);
			DMX_THREAD_DELAY(1);
			u4Ctrl &= (~0x1000);
			DMXCMD_WRITE32(PVR_REG_FRAMER_CONTROL, u4Ctrl);

			/*disable error handling*/
			_PVR_SetFramerPacketErrorHandling(0, FALSE, 0);
		}
	} else if (ePort == PVR_DDI_PORT_FRAMER1) {
		u1TsIdx = 1;
		u4Reg &= ~0x10;

		_PVR_SetFrontEndEx(1, PVR_FE_DDI);

		if (fgTs) {
			DMXCMD_WRITE32(PVR_REG_DBM_BYPASS_PID_2, 0);

			DMXCMD_WRITE32(PVR_REG_DBM_CONTROL, DMXCMD_READ32(PVR_REG_DBM_CONTROL) & ~(1<<27));

			/* use internal sync*/
			_PVR_SetFramerMode(PVR_FMR1, PVR_FRAMER_PARALLEL, TRUE, TRUE);

			/*enable error handling*/
			_PVR_SetFramerPacketErrorHandling(1, TRUE, 0xF0C0);
		} else {
			u32 u4Ctrl;

			DMXCMD_WRITE32(PVR_REG_DBM_BYPASS_PID_2, u1Pidx << 16 | 0x20);

			DMXCMD_WRITE32(PVR_REG_DBM_CONTROL, DMXCMD_READ32(PVR_REG_DBM_CONTROL) | (1<<27));

			/* use external sync*/
			_PVR_SetFramerMode(PVR_FMR1, PVR_FRAMER_PARALLEL, TRUE, TRUE);

			/* Framer 1 Reset*/
			u4Ctrl = DMXCMD_READ32(PVR_REG_FRAMER_CONTROL) & (~0x2000);
			u4Ctrl |= 0x2000;
			DMXCMD_WRITE32(PVR_REG_FRAMER_CONTROL, u4Ctrl);
			DMX_THREAD_DELAY(1);
			u4Ctrl &= (~0x2000);
			DMXCMD_WRITE32(PVR_REG_FRAMER_CONTROL, u4Ctrl);

			/*disable error handling*/
			_PVR_SetFramerPacketErrorHandling(1, FALSE, 0);
		}
	} else if (ePort == PVR_DDI_PORT_NOT_FRAMER) {
		/*TSIdx3 Bypass*/
		u1TsIdx = 3;
		u4Reg |= 0x10;

		DMXCMD_WRITE32(PVR_REG_DBM_CONTROL, DMXCMD_READ32(PVR_REG_DBM_CONTROL) | (1<<29));
		DMXCMD_WRITE32(PVR_REG_DBM_BYPASS_PID_4, 0x20);		/* Steer to FTuP*/
	} else {
		/* kind of redundant: C type-checking mechanism shall warn us*/
		PVR_LOG_ERR(TEXT("[DDI] %s(ePort: %d, fgForce: %s) fail for invalid ePort\r\n"),
			DMX_FUNC_NAME, ePort, (fgForce ? "TRUE" : "FALSE"));
		return;
	}

	_PVR_Lock();
	DDI_WRITE32(DDI_REG_DMX_RX_CTRL, u4Reg);
	_rPvrDDI.eDDIPortType = ePort;
	_PVR_Unlock();
}


/*-----------------------------------------------------------------------------*/
/** _PVR_DDI_GetPort*/
/*-----------------------------------------------------------------------------*/
PVR_DDI_PORT_T _PVR_DDI_GetPort(void)
{
	PVR_DDI_PORT_T ePort;

	_PVR_Lock();
	ePort = _rPvrDDI.eDDIPortType;
	_PVR_Unlock();

	return ePort;
}

/*-----------------------------------------------------------------------------*/
/** _PVR_DDI_HWReset
*/
/*-----------------------------------------------------------------------------*/
void _PVR_DDI_HWReset(void)
{
	u32 u4Reg, u4Reg1;
	u32 u4RegM, u4RegN;

	u4Reg = DDI_READ32(DDI_REG_DMX_RX_CTRL);
	u4Reg1 = DDI_READ32(DDI_REG_PKT_QUADBYTE_LIMIT);

	u4RegM = DDI_READ32(DDI_REG_PERIOD_M);
	u4RegN = DDI_READ32(DDI_REG_PERIOD_N);

	/* DDI soft reset*/
	DDI_WRITE32(DDI_REG_GLOBAL_CTRL, 0);
	/* 0x208: Release soft reset, turn on DDI DRAM clock.*/
	DDI_WRITE32(DDI_REG_GLOBAL_CTRL, 0x208);

	DDI_WRITE32(DDI_REG_DMX_RX_CTRL, u4Reg);
	DDI_WRITE32(DDI_REG_PKT_QUADBYTE_LIMIT, u4Reg1);

	DDI_WRITE32(DDI_REG_PERIOD_M, u4RegM);
	DDI_WRITE32(DDI_REG_PERIOD_N, u4RegN);
	DDI_WRITE32(DDI_REG_RATE_CMD, 3);
}

/*-----------------------------------------------------------------------------*/
/** _PVR_DDI_SetPacketSize*/
/*-----------------------------------------------------------------------------*/
bool _PVR_DDI_SetPacketSize(u8 u1PacketSize)
{
	u8 u1SyncOffset;
	u32 u4Reg;

	FUNC_ENTRY;

	if (_rPvrDDI.eState != PVR_DDI_STOP) {
		PVR_LOG_ERR(TEXT("[DDI] %s line %d fail, Please stop DDI first\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return FALSE;
	}
	smp_mb();

	switch (u1PacketSize) {
	case 188:
		u1SyncOffset = 0;
		break;

	case 192:
		u1SyncOffset = 4;
		break;

	case 204:
		u1SyncOffset = 16;
		break;

	default:
		PVR_LOG_ERR(TEXT("[DDI] %s line %d fail for Packet size (%u) is not supported\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u1PacketSize);
		FUNC_EXIT;
		return FALSE;
	}

	smp_mb();

	u4Reg = DDI_READ32(DDI_REG_PKT_QUADBYTE_LIMIT);
	u4Reg = (u4Reg & 0xFFFFFF00) | (u1PacketSize >> 2);
	smp_mb();
	DDI_WRITE32(DDI_REG_PKT_QUADBYTE_LIMIT, u4Reg);

	FUNC_EXIT;

	/* Set framer prebuffer*/
	return TRUE;
}

/*-----------------------------------------------------------------------------*/
/** _PVR_DDI_GetPacketSize*/
/*-----------------------------------------------------------------------------*/
u8 _PVR_DDI_GetPacketSize(void)
{
	u8 u1PacketSize;

	_PVR_Lock();
	u1PacketSize = (u8)(DDI_READ32(DDI_REG_PKT_QUADBYTE_LIMIT) & 0xFFFFFF00);
	_PVR_Unlock();

	return u1PacketSize;
}


/*-----------------------------------------------------------------------------*/
/** _PVR_DDI_SetSyncOffset
*
* The function _PVR_DDI_SetPacketSize() calculate a default offset value for the Sync byte.
* Call this function to change the offset value if the default value does not meet user's requirement.
*/
/*-----------------------------------------------------------------------------*/
bool _PVR_DDI_SetSyncOffset(u8 u1Offset)
{
	u8 u1MinOffset = 0, u1MaxOffset;

	u1MaxOffset = _rPvrDDI.u1PacketSize - (u8)188;

	if ((u1MinOffset <= u1Offset) && (u1Offset <= u1MaxOffset)) {
		_rPvrDDI.u1SyncOffset = u1Offset;
		return TRUE;
	}

	PVR_LOG_ERR(TEXT("[DDI] %s line %d fail for Sync byte offset (%d) is not supported\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, u1Offset);

	return FALSE;
}


/*-----------------------------------------------------------------------------*/
/** _PVR_DDI_Free
*/
/*-----------------------------------------------------------------------------*/
bool _PVR_DDI_Free(void)
{
	if (_rPvrDDI.eState != PVR_DDI_STOP) {
		PVR_LOG_ERR(TEXT("[DDI] %s line %d fail, Please stop DDI before its buffer is freed\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return FALSE;
	}

	_DDI_Reset(FALSE);	/* Only reset some HW registers (RIP, WP, AP, and etc).*/

	if (!_DDI_FreeBuf()) {
		PVR_LOG_ERR(TEXT("[DDI] %s line %d fail in _DDI_FreeBuf\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return FALSE;	/* Current implementation will never fall into here.*/
	}

	return TRUE;
}

/* Make Source End Address > WPtr*/
void _PVR_DDI_FixDMAEndAddr(uintptr_t *pptrEA, uintptr_t ptrWPtr)
{
	if ((*pptrEA - ptrWPtr) > DDI_BUF_ALIGNMENT)
		return;

	*pptrEA += DDI_BUF_ALIGNMENT;

	if (ptrWPtr >= *pptrEA)
		_PVR_DDI_FixDMAEndAddr(pptrEA, ptrWPtr);
}

/*-----------------------------------------------------------------------------*/
/** _PVR_DDI_SingleMove
*	For the Single move mode.
*
*	The parameter u4BufEnd must be the address right next to the real end of  the buffer.
*	In other words, the data byte addressed by u4BufEnd does not belong to the buffer.
*	Both start/end addresses must be aligned to 16-byte boundary.
*	Both Read/Write pointers must be aligned to 4-byte boundary.
*/
/* The addresses passed into this function shall be DMX_NONCACHE addresses.*/
/*-----------------------------------------------------------------------------*/
bool _PVR_DDI_SingleMove(uintptr_t ptrBufferSa, uintptr_t ptrBufferEa,
						 uintptr_t ptrSrcAddr, u32 u4Size)
{
	u32 u4BufSize, u4SrcDataSz;
	uintptr_t ptrPhyBufRp, ptrPhyBufWp, ptrPhyBufSa;
	u8  u1SkipBytes;

	if (PVR_DDI_STOP != _rPvrDDI.eState) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail for DDI is still moving data...\r\n"),
			DMX_FUNC_NAME);
		return FALSE;
	}

	if ((PVR_DDI_MODE_SINGLE != _rPvrDDI.eMode) &&
		(PVR_DDI_MODE_NONBLOCKING != _rPvrDDI.eMode)) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail, Please switch DDI to SINGLE or ")
			TEXT("NONBLOCKING mode first.\r\n"),
			DMX_FUNC_NAME);
		return FALSE;
	}

	if ((ptrBufferSa < 1) || (ptrBufferEa < 1) || (ptrSrcAddr < 1) ||
		(ptrBufferEa - ptrBufferSa < u4Size)) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail for invalid args(ptrBufSa: 0x%x, ")
			TEXT("ptrBufEa: 0x%x, ptrSrcSz: 0x%x)\r\n"),
			DMX_FUNC_NAME, ptrBufferSa, ptrBufferEa, u4Size);
		DMX_ASSERT(FALSE);
		return FALSE;
	}

	/* Prepare Buffer StartAddr/EndAddr/ReadPtr for DDI Transfer*/

	/* Buffer Start Address*/
	ptrPhyBufSa = DMX_PHYSICAL(ptrBufferSa);
	DMX_ASSERT(ptrPhyBufSa > 0x0270);
	u1SkipBytes = 0;
	if (!_PVR_IsAligned(ptrPhyBufSa, DDI_BUF_ALIGNMENT))
		ptrPhyBufSa = _PVR_Align_Dec(ptrPhyBufSa, DDI_BUF_ALIGNMENT, &u1SkipBytes);

	DMX_ASSERT(_PVR_IsAligned(ptrPhyBufSa, DDI_BUF_ALIGNMENT));

	/* Source Data Size DDI_PACKET_SIZE aligned*/
	u4SrcDataSz = u4Size;
	u4SrcDataSz += (DDI_PACKET_SIZE - 1);
	u4SrcDataSz -= u4SrcDataSz % DDI_PACKET_SIZE;

	/* Buffer End Address*/
	if (ptrSrcAddr + u4SrcDataSz > ptrBufferEa)
		ptrBufferEa = ptrSrcAddr + u4SrcDataSz;

	DMX_ASSERT(ptrBufferEa > 0x0270);
	ptrBufferEa = DMX_PHYSICAL(ptrBufferEa);
	DMX_ASSERT(ptrBufferEa > 0x0270);
	if (!_PVR_IsAligned(ptrBufferEa, DDI_BUF_ALIGNMENT))
		ptrBufferEa = _PVR_Align(ptrBufferEa, DDI_BUF_ALIGNMENT);
	DMX_ASSERT(_PVR_IsAligned(ptrBufferEa, DDI_BUF_ALIGNMENT));

	DMX_ASSERT(ptrSrcAddr > 0x0270);
	DMX_ASSERT(ptrBufferEa > 0x0270);
	/* Buffer Read Address*/
	ptrPhyBufRp = DMX_PHYSICAL(ptrSrcAddr);

	if (!((ptrPhyBufRp >= ptrPhyBufSa) && (ptrPhyBufRp < ptrBufferEa))) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail for invalid args (PhyBufStart: 0x%x, ")
			TEXT("PhyBufEnd: 0x%x, PhyBufRp: 0x%x, Size: 0x%x\r\n"),
			DMX_FUNC_NAME, ptrPhyBufSa, ptrBufferEa, ptrPhyBufRp, u4Size);
		DMX_ASSERT(FALSE);
		return FALSE;
	}

	ptrPhyBufWp = ptrPhyBufRp + u4SrcDataSz;
	DMX_ASSERT(ptrPhyBufWp > 0x0270);

	/* Modify Buffer End Address, if the EA-WP <= DDI_BUF_ALIGNEMENT*/
	_PVR_DDI_FixDMAEndAddr(&ptrBufferEa, ptrPhyBufWp);

	u4BufSize = ptrBufferEa - ptrPhyBufSa;

	PVR_LOG_DBG(TEXT("[PVR] %s -- PhyBufStart: 0x%x, PhyBufEnd: 0x%x, PhyBufRp: 0x%x, Size: 0x%x\r\n"),
		DMX_FUNC_NAME, ptrPhyBufSa, ptrBufferEa, ptrPhyBufRp, u4Size);

	_PVR_DDI_SetDMAInt(FALSE, FALSE);	/* disable Emtpy Int; disable Alert Int */

	/* Stop the DMA operation to make sure last DMA stop*/
	DDI_WRITE32(DDI_REG_DMA_CTRL, 4);

	/* Flushing cache can aVOID the cache consistency problem at the cost of*/
	/* performance.  If a user needs to move his local data via DDI DMA, he*/
	/* needs to flush cache by himself.  This convention can aVOID unnecessary*/
	/* cache-flushing operation in the driver.*/

	_PVR_Lock();
	_rPvrDDI.fgAllocBuf  = FALSE;
	_rPvrDDI.u4Threshold = 0;
	_rPvrDDI.ptrBufAddr = ptrBufferSa;
	_rPvrDDI.u4BufSize = u4BufSize;
	_rPvrDDI.ptrRp = ptrSrcAddr;
	_rPvrDDI.ptrWp = _rPvrDDI.ptrRp + (ptrPhyBufWp - ptrPhyBufRp);
	_rPvrDDI.eState = PVR_DDI_PLAY;

	/* Set DDI ring buffer*/
	DMX_ASSERT(ptrPhyBufSa > 0x0270);
	DDI_WRITE32(DDI_REG_DMA_BUF_START, ptrPhyBufSa);		/* Start pointer*/
	DMX_ASSERT(ptrBufferEa > 0x0270);
	DDI_WRITE32(DDI_REG_DMA_BUF_END, ptrBufferEa);		/* End pointer*/
	DMX_ASSERT(ptrPhyBufRp > 0x0270);
	DDI_WRITE32(DDI_REG_DMA_RP_INIT, ptrPhyBufRp);		/* Initial RP*/
	DMX_ASSERT(ptrPhyBufWp > 0x0270);
	DDI_WRITE32(DDI_REG_DMA_WP, ptrPhyBufWp);			/* Write pointer*/

	_PVR_Unlock();

	/*_PVR_DDI_SetDMAInt(TRUE, FALSE);	 enable Emtpy Int; disable Alert Int*/

	DDI_WRITE32(DDI_REG_DMA_CTRL, 5);	/* Start the DMA operation*/

	return TRUE;
}
/*-----------------------------------------------------------------------------*/
/** _PVR_DDI_PowerDown
 *
 *	After the DDI is powered down, users need to call DDI_Init() to bring it
 *	back to life.
 */
/*-----------------------------------------------------------------------------*/
bool _PVR_DDI_PowerDown(void)
{
	u32 u4Reg;

	_PVR_Lock();

	u4Reg = DDI_READ32(DDI_REG_GLOBAL_CTRL);
	u4Reg &= ~(1 << 3); /* Turn Off DDI DRAM Clock*/
	DDI_WRITE32(DDI_REG_GLOBAL_CTRL, u4Reg);

	_PVR_Unlock();

	return TRUE;
}

void _PVR_DDI_SetDelayByte(u8 u1DelayByte)
{
	u32 u4Reg;

	DMX_ASSERT(u1DelayByte <= 0xF);

	/*set ddi delay byte*/
	u4Reg = DDI_READ32(DDI_REG_DMX_RX_CTRL);
	u4Reg &= ~(0xf);
	u4Reg |= u1DelayByte;
	DDI_WRITE32(DDI_REG_DMX_RX_CTRL, u4Reg);
}


void _PVR_DDI_EndSingleMove(void)
{
	PVR_INPUT_TYPE_T eInputType;

	eInputType = _PVR_GetInputType();
	if (PVR_IN_PLAYBACK_MM != eInputType) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail for Incorrect input type(%d)!\r\n"),
			DMX_FUNC_NAME, eInputType);
		return;
	}

	if (PVR_DDI_MODE_SINGLE != _rPvrDDI.eMode) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail, Please switch DDI to SINGLE mode first.\r\n"),
			DMX_FUNC_NAME);
		return;
	}

	DDI_WRITE32(DDI_REG_DMA_CTRL, 4);	/* Stop the DMA operation*/

	_PVR_Lock();
	_rPvrDDI.eState = PVR_DDI_STOP;		/* in case enum is changed carelessly*/
	_PVR_Unlock();

	/*_PVR_DDI_SetDMAInt(TRUE, FALSE);	 enable Emtpy Int; disable Alert Int*/

	/* Reset DDI to make ReadPtrInit is not 4 Bytes Align.*/
	_PVR_DDI_HWReset();

	/* Reset DBM for next Single Move*/
	if (!_PVR_ResetDbmSafely())
		PVR_LOG_ERR(TEXT("%s fail in _PVR_ResetDbmSafely\r\n"), DMX_FUNC_NAME);
	else
		PVR_LOG_DBG(TEXT("%s success in _PVR_ResetDbmSafely\r\n"), DMX_FUNC_NAME);
}

bool _PVR_DDI_SetISREventHandle(HANDLE_T hEvent)
{
	_rPvrDDI.hDDINotifyEG = hEvent;
	return TRUE;
}

void _PVR_DDI_IrqHandler(u16 u2Vector)
{
	u32 u4Reg, u4Status, u4Mask;

	UNUSED(u2Vector);

	u4Status = DDI_READ32(DDI_REG_DCR_INT_SET);
	u4Mask = DDI_READ32(DDI_REG_DCR_INT_MASK);

	u4Reg = u4Status & (~u4Mask);

#if 0
	/* Buffer is empty. (Empty s32)*/
	if (u4Reg & 0x2) {
		PVR_LOG_DBG(TEXT("[PVR] %s -- DDI Empty interrupt.\r\n"), DMX_FUNC_NAME);
		if (_rPvrDDI.hDDINotifyEG) {
			x_ev_group_set_event(_rPvrDDI.hDDINotifyEG,
				PVR_DDI_NOTIFY_EV_BUF_EMPTY, X_EV_OP_OR);
		}
	}
	/* Buffer has some more space. (Alert s32)*/
	if (u4Reg & 0x4)
		PVR_LOG_DBG(TEXT("[PVR] %s -- DDI Alert interrupt.\r\n"), DMX_FUNC_NAME);
#endif

	/* It is essential to clear the interrupt before notifying users.*/
	DDI_WRITE32(DDI_REG_DCR_INT_CLR, u4Status);		/* Clear the interrupt*/

}

/*-----------------------------------------------------------------------------*/
/** _PVR_DDI_InitISR
 *	Initialize interrupt handler
 *
 *	@retval void
 */
/*-----------------------------------------------------------------------------*/
bool _PVR_DDI_InitISR(void)
{
	x_os_isr_fct pfnOldIsr = NULL;

	/* Register ISR*/
	if (!_rPvrDDI.fgDDIISRInited) {
		PVR_LOG_TRACE(TEXT("[PVR] %s -- enable dmx ftup interrupt!\r\n"),
			      DMX_FUNC_NAME);
		UNUSED(pfnOldIsr);

		_rPvrDDI.fgDDIISRInited = TRUE;
	}

	PVR_LOG_DBG(TEXT("[PVR] %s success!\r\n"), DMX_FUNC_NAME);

	return TRUE;
}

bool _PVR_DDI_UninitISR(void)
{
	x_os_isr_fct pfnOldIsr = NULL;

	/* UnRegister ISR*/
	if (_rPvrDDI.fgDDIISRInited) {
		UNUSED(pfnOldIsr);

		_rPvrDDI.fgDDIISRInited = FALSE;
	}

	PVR_LOG_DBG(TEXT("[PVR] %s success!\r\n"), DMX_FUNC_NAME);

	return TRUE;
}

/*-----------------------------------------------------------------------------*/
/** _PVR_DDI_SetDMAInt
 *	@retval void
 */
/*-----------------------------------------------------------------------------*/
void _PVR_DDI_SetDMAInt(bool fgEmpty, bool fgAlert)
{
	u32 u4Reg;

	_PVR_Lock();
	/* DDI_WRITE32(DDI_REG_DCR_INT_CLR, 0x7);*/
	u4Reg = DDI_READ32(DDI_REG_DCR_INT_MASK);
	_PVR_Unlock();

	if (fgEmpty) {
		PVR_LOG_DBG(TEXT("[PVR] %s line %d Enable DDI Empty Interrupt!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		u4Reg &= ~0x2;
	} else {
		PVR_LOG_DBG(TEXT("[PVR] %s line %d Disable DDI Empty Interrupt!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		u4Reg |= 0x2;
	}

	if (fgAlert) {
		PVR_LOG_DBG(TEXT("[PVR] %s line %d Enable DDI Alert Interrupt!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		u4Reg &= ~0x4;
	} else {
		PVR_LOG_DBG(TEXT("[PVR] %s line %d Disable DDI Alert Interrupt!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		u4Reg |= 0x4;
	}

	_PVR_Lock();
	DDI_WRITE32(DDI_REG_DCR_INT_MASK, u4Reg);
	_PVR_Unlock();
}

bool _PVR_DDI_DumpInfo(void)
{
	uintptr_t ptrSa, ptrEa, ptrRp, ptrWp, ptrRpInit;

	DMXLOG_TRACE(TEXT("[PVR] DDI Before DMA -- SrcBufSa: 0x%x, Sz: 0x%x,")
		TEXT(" Rp: 0x%x, Wp: 0x%x\r\n"),
		DMX_PHYSICAL(_rPvrDDI.ptrBufAddr), _rPvrDDI.u4BufSize,
		DMX_PHYSICAL(_rPvrDDI.ptrRp), DMX_PHYSICAL(_rPvrDDI.ptrWp));
	ptrSa = DDI_READ32(DDI_REG_DMA_BUF_START);
	ptrEa = DDI_READ32(DDI_REG_DMA_BUF_END);
	ptrRpInit = DDI_READ32(DDI_REG_DMA_RP_INIT);
	ptrRp = DDI_READ32(DDI_REG_DMA_RP);
	ptrWp = DDI_READ32(DDI_REG_DMA_WP);
	DMXLOG_TRACE(TEXT("[PVR] DDI After DMA -- SrcBufSa: 0x%x, Ea: 0x%x,")
		TEXT(" RpInit: 0x%x, Rp: 0x%x, Wp: 0x%x\r\n"),
		ptrSa, ptrEa, ptrRpInit, ptrRp, ptrWp);

	return TRUE;
}

