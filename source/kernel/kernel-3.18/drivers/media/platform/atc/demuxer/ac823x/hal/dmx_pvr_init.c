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
 * @file dmx_pvr_init.c
 *
 * @par Project
 *	  MT3360
 *
 * @par Description
 *	  Demuxer pvr initialization interfaces definitions
 *
 * @par Author_Name
 *	  Shuhui Zhang
 *
 */

/*-----------------------------------------------------------------------------*/
/* Include files*/
/*-----------------------------------------------------------------------------*/

#include <linux/mm.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

#include "x_os.h"
#include "x_bim.h"
#include "x_assert.h"
#include "x_hal_ic.h"
#include "x_util.h"
#include "x_typedef.h"
#include "x_hal_ic.h"
#include "drv_config.h"
#include "chip_ver.h"
#include "x_ckgen.h"

#include "x_ckgen_8317.h"
#include <media/atc/dmx_define.h>
#include "mach/irqs.h"

#include "dmx_def.h"
#include "dmx_mem.h"
#include "dmx_pvr.h"
#include "dmx_pvr_imem.h"
#include "dmx_pvr_ddi.h"
#include "dmx_pvr_mpp.h"

#ifndef __linux__
#if DMX_SUPPORT_DEVICE_TREE
#include <linux/clk.h>
#include <linux/clk-private.h>
#include <linux/pinctrl/consumer.h>
#endif				/* DMX_SUPPORT_DEVICE_TREE */
#pragma warning(disable : 4127)	/*disable warning C4127: conditional expression is constant */
#endif

#define PVR_DBG_LOCAL_ARBITER               0

/*-----------------------------------------------------------------------------*/
/* Configurations*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Constant definitions*/
/*-----------------------------------------------------------------------------*/

#define NUM_UP_TEST_RESULT_WORDS			16
#define PVR_MAX_DBM_WAIT_COUNT				10
#define PVR_MAX_FTT_WAIT_COUNT				5000

#define AC8317_TMPORY_DISABLE_GPIO			1

/*-----------------------------------------------------------------------------*/
/* Type definitions*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Macro definitions*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Static variables*/
/*-----------------------------------------------------------------------------*/

#ifdef __linux__

/*static CRIT_STATE_T			_rPvrLock;*/
static unsigned long _u4PvrLockFlags;
spinlock_t _rPvrLock;
/*/ Is interrupt locking? For making sure Lock()/Unlock() are not nested*/
static bool _fgPvrLocking = FALSE;
static DMX_PM_STATE _DmxHwCurrentDx = D4;
#else				/* __linux__ */
static CEDEVICE_POWER_STATE _DmxHwCurrentDx = D4;
static CRITICAL_SECTION _rDmxHwLock;
#endif				/* __linux__ */

typedef struct {
	PVR_INPUT_TYPE_T ePvrInputType;
	u32 u4uCodeVersion;
	bool fgPvrMicroProcStopped;
	bool fgPvrInited;
	bool fgPvrISRInited;
	PVR_PID_STRUCT_T *prPvrPidStructs;
	DMX_PIC_INFO_T *prPicInfos;
	PVR_FTUP_INT_STATUS_INFO_T rFtupStatus;
	DMX_HAL_FUNC_INFO_T rHalFuncInfo;
} PVR_HW_GLOBAL_INFO_T;

PVR_VIDEO_TYPE_T g_eDmxHwVideoType[DMX_DEV_CNT] = {PVR_VIDEO_UNKNOWN, PVR_VIDEO_UNKNOWN};

/*-----------------------------------------------------------------------------*/
/* Static functions*/
/*-----------------------------------------------------------------------------*/


static PVR_HW_GLOBAL_INFO_T _rDmxPvrInfo = { 0 };

/*-----------------------------------------------------------------------------*/
/** _PVR_GetPidStruct
 *	Get the PID structure of a given PID index
 *
 *	@param	u4PidIndex		PID index
 *
 *	@retval The pointer of the PID structure
 */
/*-----------------------------------------------------------------------------*/
PVR_PID_STRUCT_T *_PVR_GetPidStruct(u32 u4PidIndex)
{
	if (u4PidIndex >= PVR_NUM_PID_INDEX) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail for invalid Pid Idx: %d\r\n"),
			    DMX_FUNC_NAME, u4PidIndex);
		DMX_ASSERT(FALSE);
		return NULL;
	}

	return &(_rDmxPvrInfo.prPvrPidStructs[u4PidIndex]);
}

/*-----------------------------------------------------------------------------*/
/** _PVR_GetPicturesInfo
 *	Get Video Pictures info array
 *
 *	@param	void
 *
 *	@retval The pointer of the Video Pictures info array
 */
/*-----------------------------------------------------------------------------*/
DMX_PIC_INFO_T *_PVR_GetPicturesInfo(void)
{
	return _rDmxPvrInfo.prPicInfos;
}

DMX_HAL_FUNC_INFO_T *_PVR_GetGlobalCbInfo(void)
{
	return &(_rDmxPvrInfo.rHalFuncInfo);
}

PVR_FTUP_INT_STATUS_INFO_T *_PVR_GetFtupIntStatus(void)
{
	return &(_rDmxPvrInfo.rFtupStatus);
}

/*-----------------------------------------------------------------------------*/
/** _PVR_Lock
 *	Enter demux critical section
 *
 *	@retval void
 */
/*-----------------------------------------------------------------------------*/
void _PVR_Lock(void)
{
	/*_rPvrLock = x_crit_start();*/
	spin_lock_irqsave(&_rPvrLock, _u4PvrLockFlags);
	_fgPvrLocking = TRUE;
	UNUSED(_fgPvrLocking);	/* Make Lint happy */
}


/*-----------------------------------------------------------------------------*/
/** _PVR_Unlock
 *	Leave demux critical section
 *
 *	@retval void
 */
/*-----------------------------------------------------------------------------*/
void _PVR_Unlock(void)
{
	_fgPvrLocking = FALSE;

	/*x_crit_end(_rPvrLock); */
	spin_unlock_irqrestore(&_rPvrLock, _u4PvrLockFlags);
}

/*-----------------------------------------------------------------------------*/
/** _DdmxIsFTIRunning
 *	Query if FTI is running or not
 *
 *	@retval TRUE			Yes
 *	@retval FALSE			No
 */
/*-----------------------------------------------------------------------------*/
static bool _DmxIsFTIRunning(void)
{
	u32 u4State;
	/* FTI Global Control Register bit[29:28]: FTI Global Status, 00: RUN, 01: HALT, 1x: Reserved. */
	u4State = (DMXCMD_READ32(PVR_REG_CONTROL) >> 28) & 0x3;
	PVR_LOG_DBG(TEXT("uP state %d (FTI Global Status, 00: RUN, 01: HALT, 1x: Reserved)\r\n"),
		    u4State);

	return (0 == u4State);
}

VOID _PVR_DisableDramLocalArbiter(void)
{
    UINT32 u4DramReg;

    _PVR_Lock();

    #if PVR_DBG_LOCAL_ARBITER
    PVR_LOG_INFO(TEXT("[PVR] %s line %d -- disable dram local arbiter 0 and 1's dmx sub agent\r\n"),
            DMX_FUNC_NAME, DMX_LINE_NO);
    #endif // PVR_DBG_LOCAL_ARBITER
    u4DramReg = SYSDRAM_READ32(0x4); //0x52004
    u4DramReg &= ~(1 << 25);
    SYSDRAM_WRITE32(0x4, u4DramReg);
    u4DramReg = SYSDRAM_READ32(0x104);//0x52104
    u4DramReg &= ~(1 << 31);
    SYSDRAM_WRITE32(0x104, u4DramReg);

    _PVR_Unlock();
}

void _PVR_EnableDramLocalArbiter(void)
{
    u32 u4DramReg = 0;
    
    #if PVR_DBG_LOCAL_ARBITER
    u32 u4DramReg1, u4DramReg2;
    PVR_LOG_INFO(TEXT("[PVR] %s 0xF0012800=0x%08x, 0xF0012474=0x%08x, 0xF0012430=0x%08x!\r\n"),
        DMX_FUNC_NAME,
        DMXCMD_READ32(PVR_REG_REG_FILE_ADDR_REG),
        DMXCMD_READ32(PVR_DMX_STATUS),
        DMXCMD_READ32(PVR_REG_CMD_BUS_STATUS));

    PVR_LOG_INFO(TEXT("[PVR] %s line %d -- enable dram local arbiter 0 and 1's dmx sub agent\r\n"),
            DMX_FUNC_NAME, DMX_LINE_NO);
    #endif // PVR_DBG_LOCAL_ARBITER
    
    _PVR_Lock();
    u4DramReg = SYSDRAM_READ32(0x4);
    u4DramReg |= (1 << 25);
    SYSDRAM_WRITE32(0x4, u4DramReg);
    u4DramReg = SYSDRAM_READ32(0x104);
    u4DramReg |= (1 << 31);
    SYSDRAM_WRITE32(0x104, u4DramReg);
    _PVR_Unlock();

    #if PVR_DBG_LOCAL_ARBITER
    u4DramReg1 = SYSDRAM_READ32(0x4);
    u4DramReg2 = SYSDRAM_READ32(0x104);
    
    u4DramReg = SYSCMD_READ32(0x6130);
    u4DramReg &= ~(1 << 0);
    SYSCMD_WRITE32(0x6130, u4DramReg);
    u4DramReg |= (1 << 0);
    SYSCMD_WRITE32(0x6130, u4DramReg);
    
    PVR_LOG_INFO(TEXT("[PVR] (0xF0052004=0x%08x, 0xF0052104=0x%08x) after enable local arbiter!\r\n"),
        u4DramReg1, u4DramReg2);
    #endif // PVR_DBG_LOCAL_ARBITER
}

void _PVR_DumpDramKeyRegs(bool fgFirstClear)
{
    u32 u4DramReg = 0;
    u32 u46134Reg, u46154Reg, u46158Reg, u4615cReg;
    u32 u452004Reg, u452104Reg;
    u32 u452018Reg1, u452018Reg2;
    u32 u45201cReg1, u45201cReg2;
    
    _PVR_Lock();

    if (fgFirstClear)
    {
        u4DramReg = SYSCMD_READ32(0x6130);
        u4DramReg &= ~(1 << 0);
				SYSCMD_WRITE32(0x6130, u4DramReg);
        u4DramReg |= (1 << 0);
				SYSCMD_WRITE32(0x6130, u4DramReg);
    }

    u4DramReg = SYSCMD_READ32(0x7008);
    PVR_LOG_INFO(TEXT("[PVR] %s line %d -- 0x7008=0x%08x\r\n"),
            DMX_FUNC_NAME, DMX_LINE_NO, u4DramReg);
    
    u46134Reg = SYSCMD_READ32(0x6134);
    u46154Reg = SYSCMD_READ32(0x6154);
    u46158Reg = SYSCMD_READ32(0x6158);
    u4615cReg = SYSCMD_READ32(0x615c);

    PVR_LOG_INFO(TEXT("[PVR] %s -- (1) 0x6134=0x%08x, 0x6154=0x%08x\r\n"),
            DMX_FUNC_NAME, u46134Reg, u46154Reg);

    PVR_LOG_INFO(TEXT("[PVR] %s -- (1) 0x6158=0x%08x, 0x615c=0x%08x\r\n"),
            DMX_FUNC_NAME, u46158Reg, u4615cReg);

    u4DramReg = SYSCMD_READ32(0x6130);
    u4DramReg &= ~(1 << 0);
		SYSCMD_WRITE32(0x6130, u4DramReg);
    u4DramReg |= (1 << 0);
		SYSCMD_WRITE32(0x6130, u4DramReg);
    
    u46134Reg = SYSCMD_READ32(0x6134);
    u46154Reg = SYSCMD_READ32(0x6154);
    u46158Reg = SYSCMD_READ32(0x6158);
    u4615cReg = SYSCMD_READ32(0x615c);
    
    PVR_LOG_INFO(TEXT("[PVR] %s -- (2) 0x6134=0x%08x, 0x6154=0x%08x\r\n"),
            DMX_FUNC_NAME, u46134Reg, u46154Reg);

    PVR_LOG_INFO(TEXT("[PVR] %s -- (2) 0x6158=0x%08x, 0x615c=0x%08x\r\n"),
            DMX_FUNC_NAME, u46158Reg, u4615cReg);
    
    u452004Reg = SYSDRAM_READ32(0x4); // 0x52004
    u452004Reg &= ~(0x1F << 16);
    u452004Reg |= (0x8 << 16);
		SYSDRAM_WRITE32(0x4, u452004Reg);
    
    u452018Reg1 = SYSDRAM_READ32(0x18);
    u452018Reg2 = SYSDRAM_READ32(0x18);
    PVR_LOG_INFO(TEXT("[PVR] %s -- (set8) 0x52018: 0x%08x, 0x%08x\r\n"),
            DMX_FUNC_NAME, u452018Reg1, u452018Reg2);
    
    u452004Reg = SYSCMD_READ32(0x4);
    u452004Reg &= ~(0x1F << 16);
    u452004Reg |= (0x9 << 16);
		SYSDRAM_WRITE32(0x4, u452004Reg);
    
    u452018Reg1 = SYSDRAM_READ32(0x18);
    u452018Reg2 = SYSDRAM_READ32(0x18);
    PVR_LOG_INFO(TEXT("[PVR] %s -- (set9) 0x52018: 0x%08x, 0x%08x\r\n"),
          DMX_FUNC_NAME, u452018Reg1, u452018Reg2);

    u452104Reg = SYSDRAM_READ32(0x104);
    u452104Reg &= ~(0x1F << 16);
    u452104Reg |= (0x8 << 16);
		SYSDRAM_WRITE32(0x104, u452104Reg);

    u452018Reg1 = SYSDRAM_READ32(0x118);
    u452018Reg2 = SYSDRAM_READ32(0x118);
    PVR_LOG_INFO(TEXT("[PVR] %s -- (set8) 0x52118: 0x%08x, 0x%08x\r\n"),
            DMX_FUNC_NAME, u452018Reg1, u452018Reg2);

    u452104Reg = SYSDRAM_READ32(0x104);
    u452104Reg &= ~(0x1F << 16);
    u452104Reg |= (0x9 << 16);
		SYSDRAM_WRITE32(0x104, u452104Reg);

    u452018Reg1 = SYSDRAM_READ32(0x118);
    u452018Reg2 = SYSDRAM_READ32(0x118);
    PVR_LOG_INFO(TEXT("[PVR] %s -- (set9) 0x52118: 0x%08x, 0x%08x\r\n"),
        DMX_FUNC_NAME, u452018Reg1, u452018Reg2);

    u45201cReg1 = SYSDRAM_READ32(0x1c);
    u45201cReg2 = SYSDRAM_READ32(0x1c);
    
    PVR_LOG_INFO(TEXT("[PVR] %s -- 0x5201c: 0x%08x, 0x%08x\r\n"),
            DMX_FUNC_NAME, u45201cReg1, u45201cReg2);
    
    u45201cReg1 = SYSDRAM_READ32(0x11c);
    u45201cReg2 = SYSDRAM_READ32(0x11c);
    
    PVR_LOG_INFO(TEXT("[PVR] %s -- 0x5211c: 0x%08x, 0x%08x\r\n"),
        DMX_FUNC_NAME, u45201cReg1, u45201cReg2);
    
    u4DramReg = SYSCMD_READ32(0x6130);
    u4DramReg &= ~(1 << 0);
		SYSCMD_WRITE32(0x6130, u4DramReg);
    u4DramReg |= (1 << 0);
		SYSCMD_WRITE32(0x6130, u4DramReg);

  _PVR_Unlock();
}

bool _PVR_IsDramLocalArbiterDisable(void)
{
    u32 u4DramReg1 = 0, u4DramReg2 = 0;

    _PVR_Lock();
    u4DramReg1 = SYSDRAM_READ32(0x4);
    u4DramReg2 = SYSDRAM_READ32(0x104);
    if ((0 == (u4DramReg1 & (1 << 25))) ||
        (0 == (u4DramReg2 & (1 << 31))))
    {
        _PVR_Unlock();
        #if PVR_DBG_LOCAL_ARBITER
        PVR_LOG_INFO(TEXT("[PVR] (0xF0052004=0x%08x, 0xF0052104=0x%08x) local arbiter is disable !\r\n"),
            u4DramReg1, u4DramReg2);
        #endif // PVR_DBG_LOCAL_ARBITER
        return TRUE;
    }

    _PVR_Unlock();
    #if PVR_DBG_LOCAL_ARBITER
    PVR_LOG_INFO(TEXT("[PVR] (0xF0052004=0x%08x, 0xF0052104=0x%08x) local arbiter is enable!\r\n"),
        u4DramReg1, u4DramReg2);
    #endif // PVR_DBG_LOCAL_ARBITER

    return FALSE;
}

void _PVR_DumpDramLocalArbiter(u32 u4CurStatus)
{
    u32 u4DramReg1 = 0, u4DramReg2 = 0;

    _PVR_Lock();
    u4DramReg1 = SYSDRAM_READ32(0x4);
    u4DramReg2 = SYSDRAM_READ32(0x104);
    _PVR_Unlock();

    switch (u4CurStatus) {
    case 1:
        PVR_LOG_ERR(TEXT("[PVR] (0xF0052004=0x%08x, 0xF0052104=0x%08x)before set dmx reset!\r\n"),
            u4DramReg1, u4DramReg2);
        break;
    case 2:
        PVR_LOG_ERR(TEXT("[PVR] (0xF0052004=0x%08x, 0xF0052104=0x%08x)after set dmx reset!\r\n"),
            u4DramReg1, u4DramReg2);
        break;
    case 3:
        PVR_LOG_ERR(TEXT("[PVR] (0xF0052004=0x%08x, 0xF0052104=0x%08x)after set dmx clock and clear dmx reset!\r\n"),
            u4DramReg1, u4DramReg2);
        break;
    case 4:
        PVR_LOG_ERR(TEXT("[PVR] (0xF0052004=0x%08x, 0xF0052104=0x%08x)after dmx dma hung!\r\n"),
            u4DramReg1, u4DramReg2);
    break;
  case 5:
        PVR_LOG_ERR(TEXT("[PVR] (0xF0052004=0x%08x, 0xF0052104=0x%08x)after close dmx clock and set dmx reset!\r\n"),
            u4DramReg1, u4DramReg2);
        break;
    case 6:
        PVR_LOG_ERR(TEXT("[PVR] (0xF0052004=0x%08x, 0xF0052104=0x%08x)after disable localarbiter!\r\n"),
            u4DramReg1, u4DramReg2);
        break;
    default:
        PVR_LOG_ERR(TEXT("[PVR] 0xF0052004=0x%08x, 0xF0052104=0x%08x!\r\n"),
             u4DramReg1, u4DramReg2);
    break;      
    }
}


#if DMX_SUPPORT_DEVICE_TREE
bool _PVR_SelTSClk(u8 u1TsIdx, u32 u4TsSel)
{
	int ret = -1;

	if (u4TsSel > 2)
		u4TsSel = 0;

	if (u1TsIdx == 0) {
		switch (u4TsSel) {
		case 1:
			g_dmxdevinfo->dmx_ts0_in_clk_parent = clk_get(NULL, "ext_ts0_clk");
			if (IS_ERR(g_dmxdevinfo->dmx_ts0_in_clk_parent)) {
				PVR_LOG_ERR(TEXT("%s fail in get ext_ts0_clk\r\n"), DMX_FUNC_NAME);
				return FALSE;
			}
			ret = clk_set_parent(g_dmxdevinfo->dmx_ts0_in_clk,
					     g_dmxdevinfo->dmx_ts0_in_clk_parent);
			if (0 != ret) {
				PVR_LOG_ERR(TEXT
					    ("%s fail in set dmx_ts0_in_clk's parent to be ext_ts0_clk\r\n"),
					    DMX_FUNC_NAME);
				return FALSE;
			}
			g_dmxdevinfo->dmx_ts0_in_clk_parent =
			    clk_get_parent(g_dmxdevinfo->dmx_ts0_in_clk);
			break;
		case 2:
			g_dmxdevinfo->dmx_ts0_in_clk_parent = clk_get(NULL, "demux_ck");
			if (IS_ERR(g_dmxdevinfo->dmx_ts0_in_clk_parent)) {
				PVR_LOG_ERR(TEXT("%s fail in get demux_ck for ts0\r\n"),
					    DMX_FUNC_NAME);
				return FALSE;
			}
			ret = clk_set_parent(g_dmxdevinfo->dmx_ts0_in_clk,
					     g_dmxdevinfo->dmx_ts0_in_clk_parent);
			if (0 != ret) {
				PVR_LOG_ERR(TEXT
					    ("%s fail in set dmx_ts0_in_clk's parent to be demux_ck\r\n"),
					    DMX_FUNC_NAME);
				return FALSE;
			}
			g_dmxdevinfo->dmx_ts0_in_clk_parent =
			    clk_get_parent(g_dmxdevinfo->dmx_ts0_in_clk);
			break;
		case 0:
		default:
			g_dmxdevinfo->dmx_ts0_in_clk_parent = clk_get(NULL, "clk_null");
			if (IS_ERR(g_dmxdevinfo->dmx_ts0_in_clk_parent))
				return FALSE;
			ret = clk_set_parent(g_dmxdevinfo->dmx_ts0_in_clk,
					     g_dmxdevinfo->dmx_ts0_in_clk_parent);
			if (0 != ret) {
				PVR_LOG_ERR(TEXT
					    ("%s fail in set dmx_ts0_in_clk's parent to be clk_null\r\n"),
					    DMX_FUNC_NAME);
				return FALSE;
			}
			g_dmxdevinfo->dmx_ts0_in_clk_parent =
			    clk_get_parent(g_dmxdevinfo->dmx_ts0_in_clk);
			break;
		}
		g_dmxdevinfo->ts0_in_sel = u4TsSel;
	} else if (u1TsIdx == 1) {
		switch (u4TsSel) {
		case 1:
			g_dmxdevinfo->dmx_ts1_in_clk_parent = clk_get(NULL, "ext_ts1_clk");
			if (IS_ERR(g_dmxdevinfo->dmx_ts1_in_clk_parent)) {
				PVR_LOG_ERR(TEXT("%s fail in get ext_ts1_clk\r\n"), DMX_FUNC_NAME);
				return FALSE;
			}
			ret = clk_set_parent(g_dmxdevinfo->dmx_ts1_in_clk,
					     g_dmxdevinfo->dmx_ts1_in_clk_parent);
			if (0 != ret) {
				PVR_LOG_ERR(TEXT
					    ("%s fail in set dmx_ts1_in_clk's parent to be ext_ts1_clk\r\n"),
					    DMX_FUNC_NAME);
				return FALSE;
			}
			g_dmxdevinfo->dmx_ts1_in_clk_parent =
			    clk_get_parent(g_dmxdevinfo->dmx_ts1_in_clk);
			break;
		case 2:
			g_dmxdevinfo->dmx_ts1_in_clk_parent = clk_get(NULL, "demux_ck");
			if (IS_ERR(g_dmxdevinfo->dmx_ts1_in_clk_parent)) {
				PVR_LOG_ERR(TEXT("%s fail in get demux_ck for ts1\r\n"),
					    DMX_FUNC_NAME);
				return FALSE;
			}
			ret = clk_set_parent(g_dmxdevinfo->dmx_ts1_in_clk,
					     g_dmxdevinfo->dmx_ts1_in_clk_parent);
			if (0 != ret) {
				PVR_LOG_ERR(TEXT
					    ("%s fail in set dmx_ts1_in_clk's parent to be demux_ck\r\n"),
					    DMX_FUNC_NAME);
				return FALSE;
			}
			g_dmxdevinfo->dmx_ts1_in_clk_parent =
			    clk_get_parent(g_dmxdevinfo->dmx_ts1_in_clk);
			break;
		case 0:
		default:
			g_dmxdevinfo->dmx_ts1_in_clk_parent = clk_get(NULL, "clk_null");
			if (IS_ERR(g_dmxdevinfo->dmx_ts1_in_clk_parent))
				return FALSE;
			ret = clk_set_parent(g_dmxdevinfo->dmx_ts1_in_clk,
					     g_dmxdevinfo->dmx_ts1_in_clk_parent);
			if (0 != ret) {
				PVR_LOG_ERR(TEXT
					    ("%s fail in set dmx_ts1_in_clk's parent to be clk_null\r\n"),
					    DMX_FUNC_NAME);
				return FALSE;
			}
			g_dmxdevinfo->dmx_ts1_in_clk_parent =
			    clk_get_parent(g_dmxdevinfo->dmx_ts1_in_clk);
			break;
		}
		g_dmxdevinfo->ts1_in_sel = u4TsSel;
	}

	return TRUE;
}
#endif				/* DMX_SUPPORT_DEVICE_TREE */

/*-----------------------------------------------------------------------------*/
/** _PVR_CkgenInit
 *	Select clock and pin mux for FTI
 *
 *	@retval void
 */
/*-----------------------------------------------------------------------------*/
void _PVR_CkgenInit(bool fgOn, PVR_INPUT_TYPE_T eInputType)
{
	u32 u4Ctrl = 0;
	int ret = -1;

	if (fgOn) {
#if DMX_SUPPORT_DEVICE_TREE
		PVR_LOG_INFO(TEXT("======== dmx: got clk, name [%s]\n"),
			     ((NULL !=
			       g_dmxdevinfo->dmx_top_clk) ? g_dmxdevinfo->
			      dmx_top_clk->name : "NULL"));

		PVR_LOG_DBG(TEXT("%s(TRUE), eInputType: %d!\r\n"), DMX_FUNC_NAME, eInputType);

		g_dmxdevinfo->dmx_top_clk_parent = clk_get(NULL, "syspll_d3");
		ret = clk_set_parent(g_dmxdevinfo->dmx_top_clk, g_dmxdevinfo->dmx_top_clk_parent);
		g_dmxdevinfo->dmx_top_clk_parent = clk_get_parent(g_dmxdevinfo->dmx_top_clk);

		switch (eInputType) {
		case PVR_IN_PLAYBACK_MM:
			PVR_LOG_DBG(TEXT("%s(TRUE) -- PVR_IN_PLAYBACK_MM!\r\n"), DMX_FUNC_NAME);
			clk_prepare_enable(g_dmxdevinfo->dmx_gateclk);
			clk_prepare_enable(g_dmxdevinfo->dmx_ts0_gateclk);
			clk_prepare_enable(g_dmxdevinfo->dmx_ts1_gateclk);
			clk_prepare_enable(g_dmxdevinfo->dmx_27m_gateclk);
			break;
		case PVR_IN_PLAYBACK_TS:
			PVR_LOG_DBG(TEXT("%s(TRUE) -- PVR_IN_PLAYBACK_TS!\r\n"), DMX_FUNC_NAME);
			clk_prepare_enable(g_dmxdevinfo->dmx_gateclk);
			clk_prepare_enable(g_dmxdevinfo->dmx_27m_gateclk);
			clk_disable_unprepare(g_dmxdevinfo->dmx_ts0_gateclk);
			clk_disable_unprepare(g_dmxdevinfo->dmx_ts1_gateclk);
			break;
		case PVR_IN_BROADCAST_TS:
			PVR_LOG_DBG(TEXT("%s(TRUE) -- PVR_IN_BROADCAST_TS!\r\n"), DMX_FUNC_NAME);
			clk_prepare_enable(g_dmxdevinfo->dmx_gateclk);
			clk_prepare_enable(g_dmxdevinfo->dmx_ts0_gateclk);
			clk_disable_unprepare(g_dmxdevinfo->dmx_ts0_gateclk);
			clk_prepare_enable(g_dmxdevinfo->dmx_27m_gateclk);
			break;
		default:
			clk_prepare_enable(g_dmxdevinfo->dmx_gateclk);
			clk_prepare_enable(g_dmxdevinfo->dmx_ts0_gateclk);
			clk_disable_unprepare(g_dmxdevinfo->dmx_ts0_gateclk);
			clk_prepare_enable(g_dmxdevinfo->dmx_27m_gateclk);
			break;
		}

		smp_mb();

		_PVR_SelTSClk(0, PVR_TS_SEL_NOTHING);
		_PVR_SelTSClk(1, PVR_TS_SEL_NOTHING);

#else				/* DMX_SUPPORT_DEVICE_TREE */

		PVR_LOG_DBG(TEXT("%s(TRUE), eInputType: %d!\r\n"), DMX_FUNC_NAME, eInputType);
		if (!CKGEN_AgtSelClk(e_CLK_SEL_DEMUX, CLK_REG0_DEMUX_SEL_SYSPLL_D3)) {
			PVR_LOG_DBG(TEXT("%s(TRUE) fail in CKGEN_AgtSelClk(e_CLK_SEL_DEMUX,")
				     TEXT(" CLK_REG0_DEMUX_SEL_SYSPLL_D3)\r\n"), DMX_FUNC_NAME);
		}

		switch (eInputType) {
		case PVR_IN_PLAYBACK_MM:
			PVR_LOG_DBG(TEXT("%s(TRUE) -- PVR_IN_PLAYBACK_MM!\r\n"), DMX_FUNC_NAME);
			CKGEN_AgtOnClk(e_CLK_DEMUX);
			CKGEN_AgtOnClk(e_CLK_DEMUX_TS0);
			CKGEN_AgtOnClk(e_CLK_DEMUX_TS1);
			CKGEN_AgtOnClk(e_CLK_DEMUX_27M);
			/*CKGEN_AgtOnClk(e_CLK_DEMUX); */
			/*Turn on tst_en */
			/*CKGEN_WRITE32(REG_RW_SYNC_RESET_CFG1, */
			/*(CKGEN_READ32(REG_RW_SYNC_RESET_CFG1) | (1<<7))); */
			break;
		case PVR_IN_PLAYBACK_TS:
			PVR_LOG_DBG(TEXT("%s(TRUE) -- PVR_IN_PLAYBACK_TS!\r\n"), DMX_FUNC_NAME);
			CKGEN_AgtOnClk(e_CLK_DEMUX);
			CKGEN_AgtOnClk(e_CLK_DEMUX_TS0);
			CKGEN_AgtOffClk(e_CLK_DEMUX_TS1);
			CKGEN_AgtOffClk(e_CLK_DEMUX_27M);
			/*Turn on tst_en */
			/*CKGEN_WRITE32(REG_RW_SYNC_RESET_CFG1, */
			/* (CKGEN_READ32(REG_RW_SYNC_RESET_CFG1) | (1<<7))); */
			break;
		case PVR_IN_BROADCAST_TS:
			PVR_LOG_DBG(TEXT("%s(TRUE) -- PVR_IN_BROADCAST_TS!\r\n"), DMX_FUNC_NAME);
			CKGEN_AgtOnClk(e_CLK_DEMUX);
			CKGEN_AgtOnClk(e_CLK_DEMUX_TS0);
			CKGEN_AgtOffClk(e_CLK_DEMUX_TS1);
			CKGEN_AgtOnClk(e_CLK_DEMUX_27M);
			/*Turn on tst_en */
			/*CKGEN_WRITE32(REG_RW_SYNC_RESET_CFG1, */
			/*(CKGEN_READ32(REG_RW_SYNC_RESET_CFG1) | (1<<7))); */
			break;
		default:
			CKGEN_AgtOffClk(e_CLK_DEMUX);
			CKGEN_AgtOffClk(e_CLK_DEMUX_TS0);
			CKGEN_AgtOffClk(e_CLK_DEMUX_TS1);
			CKGEN_AgtOffClk(e_CLK_DEMUX_27M);
			/*Turn on tst_en */
			/*CKGEN_WRITE32(REG_RW_SYNC_RESET_CFG1, */
			/*(CKGEN_READ32(REG_RW_SYNC_RESET_CFG1) | (1<<7))); */
			break;
		}

		smp_mb();
		/* Config framer0 to ts none(no input) */
		CKGEN_AgtSelClk(e_CLK_SEL_TS0, CLK_REG9_TS0_SEL_NOTHING);

		/* Config framer1 to ts none(no input) */
		CKGEN_AgtSelClk(e_CLK_SEL_TS1, CLK_REG9_TS1_SEL_NOTHING);
#endif				/* DMX_SUPPORT_DEVICE_TREE */

		smp_mb();
		/* Power on to dram clock */
		/* FTI Global Control Register Bit[0]: FTI dmx_ck power off, 0-PowerOn, 1-PowerOff */
		u4Ctrl = DMXCMD_READ32(PVR_REG_CONTROL);
		u4Ctrl &= ~0x1;
		DMXCMD_WRITE32(PVR_REG_CONTROL, u4Ctrl);

		smp_mb();
		/* Set DMX SRAM used by DMX (FTUP Control Register's Bit[26]: */
		/*FTUP DMEM/IMEM for NAND boot/ABIST use) */
		u4Ctrl = DMXCMD_READ32(PVR_REG_FTUP_CONTROL) & 0xFBFFFFFF;
		DMXCMD_WRITE32(PVR_REG_FTUP_CONTROL, u4Ctrl);

		PVR_LOG_INFO(TEXT("Reg[0x54]: 0x%08x, Reg[0xA0]: 0x%08x, Reg[0x0C]:")
			TEXT(" 0x%08x, Reg[0x30]:0x%08x, Reg[0xBC]: 0x%08x!\r\n"),
			SYSCMD_READ32(0x54),
			SYSCMD_READ32(0xA0),
			SYSCMD_READ32(0x0C),
			SYSCMD_READ32(0x30),
			SYSCMD_READ32(0xBC));

		smp_mb();
		_PVR_Lock();
		_DmxHwCurrentDx = D0;
		_PVR_Unlock();
	} else {
		u32 u4Counter = 0;

		PVR_LOG_DBG(TEXT("%s(FALSE)!\r\n"), DMX_FUNC_NAME);

		while (u4Counter < 1000) {
			u4Ctrl = DMXCMD_READ32(PVR_REG_REG_FILE_ADDR_REG);
			if (0 != (u4Ctrl & (1 << 30)))
				break;
			u4Counter++;
		}
		if (u4Counter >= 1000) {
			PVR_LOG_ERR(TEXT("%s(FALSE), not get ftup wdle!\r\n"), DMX_FUNC_NAME);
		}

		PVR_LOG_DBG(TEXT("%s(FALSE) --> reset dmx up!\r\n"), DMX_FUNC_NAME);
		// Reset demux (uP), so uP can run into the entry pointer of uCode

		/* Reset demux (uP), so uP can run into the entry pointer of uCode */
		VERIFY(_PVR_Reset());

		#if PVR_DBG_LOCAL_ARBITER
		_PVR_DumpDramKeyRegs(FALSE);

		_PVR_DumpDramLocalArbiter(5);

		_PVR_DisableDramLocalArbiter();

		smp_mb();

		_PVR_DumpDramLocalArbiter(6);

		DMX_THREAD_DELAY(200);

		#else  // PVR_DBG_LOCAL_ARBITER

		_PVR_DisableDramLocalArbiter();

		#endif // PVR_DBG_LOCAL_ARBITER

		/* Turn off DMX clock(disable all demuxer clocks, include: */
#if DMX_SUPPORT_DEVICE_TREE
		clk_disable(g_dmxdevinfo->dmx_27m_gateclk);
		clk_disable(g_dmxdevinfo->dmx_ts0_gateclk);
		clk_disable(g_dmxdevinfo->dmx_ts1_gateclk);
		clk_disable_unprepare(g_dmxdevinfo->dmx_gateclk);
#else				/* DMX_SUPPORT_DEVICE_TREE */
		CKGEN_AgtOffClk_NoReset(e_CLK_DEMUX_27M);
		CKGEN_AgtOffClk_NoReset(e_CLK_DEMUX_TS0);
		CKGEN_AgtOffClk_NoReset(e_CLK_DEMUX_TS1);
		CKGEN_AgtOffClk(e_CLK_DEMUX);
#endif				/* DMX_SUPPORT_DEVICE_TREE */

		smp_mb();

		/* Power Off to dram clock */
		/* FTI Global Control Register Bit[0]: FTI dmx_ck power off, 0-PowerOn, 1-PowerOff */
		u4Ctrl = DMXCMD_READ32(PVR_REG_CONTROL);
		u4Ctrl |= 0x1;
		DMXCMD_WRITE32(PVR_REG_CONTROL, u4Ctrl);

		smp_mb();
		_PVR_Lock();
		_rDmxPvrInfo.fgPvrMicroProcStopped = TRUE;
		_DmxHwCurrentDx = D4;
		_PVR_Unlock();
	}
}

#if !AC8317_TMPORY_DISABLE_GPIO
static void _DmxSetTsInPinuxSel(void)
{
	int ret = 0;

#if DMX_SUPPORT_DEVICE_TREE
	g_dmxdevinfo->pins_tsclk_sel_set = pinctrl_lookup_state(
		g_dmxdevinfo->pinctrl_demuxer, "ts_clkin_sel");
	if (IS_ERR(g_dmxdevinfo->pins_tsclk_sel_set)) {
		PVR_LOG_ERR(TEXT("fail in get ts_clkin_sel pinctrl\r\n"));
		return;
	}
	ret = pinctrl_select_state(g_dmxdevinfo->pins_tsclk_sel_set);
	if (ret) {
		PVR_LOG_ERR(TEXT("fail in get pinctrl select state for ts_clkin_sel pin\r\n"));
		return;
	}

	g_dmxdevinfo->pins_tsden_sel_set = pinctrl_lookup_state(
		g_dmxdevinfo->pinctrl_demuxer, "ts_den_sel");
	if (IS_ERR(g_dmxdevinfo->pins_tsden_sel_set)) {
		PVR_LOG_ERR(TEXT("fail in get ts_den_sel pinctrl\r\n"));
		return;
	}
	ret = pinctrl_select_state(g_dmxdevinfo->pins_tsden_sel_set);
	if (ret) {
		PVR_LOG_ERR(TEXT("fail in get pinctrl select state for ts_den_sel pin\r\n"));
		return;
	}

	g_dmxdevinfo->pins_tssync_sel_set = pinctrl_lookup_state(
		g_dmxdevinfo->pinctrl_demuxer, "ts_sync_sel");
	if (IS_ERR(g_dmxdevinfo->pins_tssync_sel_set)) {
		PVR_LOG_ERR(TEXT("fail in get ts_sync_sel pinctrl\r\n"));
		return;
	}
	ret = pinctrl_select_state(g_dmxdevinfo->pins_tssync_sel_set);
	if (ret) {
		PVR_LOG_ERR(TEXT("fail in get pinctrl select state for ts_sync_sel pin\r\n"));
		return;
	}

	g_dmxdevinfo->pins_tsd0_sel_set = pinctrl_lookup_state(
		g_dmxdevinfo->pinctrl_demuxer, "ts_d0_sel");
	if (IS_ERR(g_dmxdevinfo->pins_tsd0_sel_set)) {
		PVR_LOG_ERR(TEXT("fail in get ts_d0_sel pinctrl\r\n"));
		return;
	}
	ret = pinctrl_select_state(g_dmxdevinfo->pins_tsd0_sel_set);
	if (ret) {
		PVR_LOG_ERR(TEXT("fail in get pinctrl select state for ts_d0_sel pin\r\n"));
		return;
	}
	g_dmxdevinfo->pins_tsd1_sel_set = pinctrl_lookup_state(
		g_dmxdevinfo->pinctrl_demuxer, "ts_d1_sel");
	if (IS_ERR(g_dmxdevinfo->pins_tsd1_sel_set)) {
		PVR_LOG_ERR(TEXT("fail in get ts_d1_sel pinctrl\r\n"));
		return;
	}
	ret = pinctrl_select_state(g_dmxdevinfo->pins_tsd1_sel_set);
	if (ret) {
		PVR_LOG_ERR(TEXT("fail in get pinctrl select state for ts_d1_sel pin\r\n"));
		return;
	}

	g_dmxdevinfo->pins_tsd2_sel_set = pinctrl_lookup_state(
		g_dmxdevinfo->pinctrl_demuxer, "ts_d2_sel");
	if (IS_ERR(g_dmxdevinfo->pins_tsd2_sel_set)) {
		PVR_LOG_ERR(TEXT("fail in get ts_d2_sel pinctrl\r\n"));
		return;
	}
	ret = pinctrl_select_state(g_dmxdevinfo->pins_tsd2_sel_set);
	if (ret) {
		PVR_LOG_ERR(TEXT("fail in get pinctrl select state for ts_d2_sel pin\r\n"));
		return;
	}
	g_dmxdevinfo->pins_tsd3_sel_set = pinctrl_lookup_state(
		g_dmxdevinfo->pinctrl_demuxer, "ts_d3_sel");
	if (IS_ERR(g_dmxdevinfo->pins_tsd3_sel_set)) {
		PVR_LOG_ERR(TEXT("fail in get ts_d3_sel pinctrl\r\n"));
		return;
	}
	ret = pinctrl_select_state(g_dmxdevinfo->pins_tsd3_sel_set);
	if (ret) {
		PVR_LOG_ERR(TEXT("fail in get pinctrl select state for ts_d3_sel pin\r\n"));
		return;
	}

	g_dmxdevinfo->pins_tsd4_sel_set = pinctrl_lookup_state(
		g_dmxdevinfo->pinctrl_demuxer, "ts_d4_sel");
	if (IS_ERR(g_dmxdevinfo->pins_tsd4_sel_set)) {
		PVR_LOG_ERR(TEXT("fail in get ts_d4_sel pinctrl\r\n"));
		return;
	}
	ret = pinctrl_select_state(g_dmxdevinfo->pins_tsd4_sel_set);
	if (ret) {
		PVR_LOG_ERR(TEXT("fail in get pinctrl select state for ts_d4_sel pin\r\n"));
		return;
	}
	g_dmxdevinfo->pins_tsd5_sel_set = pinctrl_lookup_state(
		g_dmxdevinfo->pinctrl_demuxer, "ts_d5_sel");
	if (IS_ERR(g_dmxdevinfo->pins_tsd5_sel_set)) {
		PVR_LOG_ERR(TEXT("fail in get ts_d5_sel pinctrl\r\n"));
		return;
	}
	ret = pinctrl_select_state(g_dmxdevinfo->pins_tsd5_sel_set);
	if (ret) {
		PVR_LOG_ERR(TEXT("fail in get pinctrl select state for ts_d5_sel pin\r\n"));
		return;
	}

	g_dmxdevinfo->pins_tsd6_sel_set = pinctrl_lookup_state(
		g_dmxdevinfo->pinctrl_demuxer, "ts_d6_sel");
	if (IS_ERR(g_dmxdevinfo->pins_tsd6_sel_set)) {
		PVR_LOG_ERR(TEXT("fail in get ts_d6_sel pinctrl\r\n"));
		return;
	}
	ret = pinctrl_select_state(g_dmxdevinfo->pins_tsd6_sel_set);
	if (ret) {
		PVR_LOG_ERR(TEXT("fail in get pinctrl select state for ts_d6_sel pin\r\n"));
		return;
	}
	g_dmxdevinfo->pins_tsd7_sel_set = pinctrl_lookup_state(
		g_dmxdevinfo->pinctrl_demuxer, "ts_d7_sel");
	if (IS_ERR(g_dmxdevinfo->pins_tsd7_sel_set)) {
		PVR_LOG_ERR(TEXT("fail in get ts_d7_sel pinctrl\r\n"));
		return;
	}
	ret = pinctrl_select_state(g_dmxdevinfo->pins_tsd7_sel_set);
	if (ret) {
		PVR_LOG_ERR(TEXT("fail in get pinctrl select state for ts_d7_sel pin\r\n"));
		return;
	}
#endif
}
#endif

/*-----------------------------------------------------------------------------*/
/** _DmxSetExtPinmux
 */
/*-----------------------------------------------------------------------------*/
static void _DmxSetExtPinmux(u8 u1TsIdx, PVR_PINMUX_SEL_T ePin)
{
	switch (ePin) {
	case PVR_PINMUX_EXT_S:
#if !AC8317_TMPORY_DISABLE_GPIO
		/*set ts_sel0~ts_sel3 all reg to be 1, to make ts_clkin, ts_den, ts_sync, ts_d0~d7 all open */
		_DmxSetTsInPinuxSel();

#endif

		break;

	case PVR_PINMUX_EXT_P:
#if !AC8317_TMPORY_DISABLE_GPIO
		/*set ts_sel0~ts_sel3 all reg to be 1, to make ts_clkin, ts_den, ts_sync, ts_d0~d7 all open */
		_DmxSetTsInPinuxSel();

#endif

		break;
	default:
		PVR_LOG_ERR(TEXT("[DMX][HW] %s line %d -- Wrong Pinmux table please check!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		return;
	}
}

#if DMX_SUPPORT_DEVICE_TREE
/*-----------------------------------------------------------------------------*/
/** _DmxSetTSInputCkgen
 */
/*-----------------------------------------------------------------------------*/
static void _DmxSetTSInputCkgen(u8 u1Idx, PVR_FRONTEND_T eFrontEnd)
{
	PVR_FRONTEND_T ePreType;
	u32 u4TS1Val = 0, u4TS0Val = 0;

	PVR_LOG_TRACE(TEXT("[DMX][HW] %s (u1Idx: %d, eFrontEnd: %d)\r\r\n"),
		      DMX_FUNC_NAME, u1Idx, eFrontEnd);

	if (u1Idx >= PVR_FRAMER_COUNT) {
		PVR_LOG_ERR(TEXT("[DMX][HW] %s line %d fail for invalid u1Idx(%u)!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, u1Idx);
		return;
	}

	smp_mb();
	/*--------------------------*/
	/* Set Framer Ckgen */
	/*--------------------------*/
	ePreType = PVR_FE_NO_TSVALID;
	smp_mb();

	u4TS0Val = g_dmxdevinfo->ts0_in_sel;
	u4TS1Val = g_dmxdevinfo->ts1_in_sel;
	smp_mb();

	switch (eFrontEnd) {
	case PVR_FE_DDI:
		if (0 == u1Idx)
			u4TS0Val = PVR_TS_SEL_DEMUX;
		else if (1 == u1Idx)
			u4TS1Val = PVR_TS_SEL_DEMUX;

		break;
	case PVR_FE_EXT_S:
		if (0 == u1Idx)
			u4TS0Val = PVR_TS_SEL_EXT_TS;
		else if (1 == u1Idx)
			u4TS1Val = PVR_TS_SEL_EXT_TS;
		break;
	case PVR_FE_EXT_P:
		u4TS0Val = PVR_TS_SEL_EXT_TS;
		u4TS1Val = PVR_TS_SEL_NOTHING;
		break;
	default:
		break;
	}

	smp_mb();
	if (_PVR_IsFramerEnabled(u1Idx)) {
		/*To avoid affecting framer logic, we disable framer before changing clock source. */
		_PVR_SetFramerEnabled(u1Idx, FALSE);
		smp_mb();

		if (0 == u1Idx) {
			_PVR_SelTSClk(u1Idx, u4TS0Val);
			if (PVR_FE_EXT_P == eFrontEnd)
				_PVR_SelTSClk(1, u4TS1Val);
		} else if (1 == u1Idx) {
			_PVR_SelTSClk(1, u4TS1Val);
		}

		smp_mb();
		_PVR_SetFramerEnabled(u1Idx, TRUE);
	} else {
		if (0 == u1Idx) {
			_PVR_SelTSClk(0, u4TS0Val);
			if (PVR_FE_EXT_P == eFrontEnd)
				_PVR_SelTSClk(1, u4TS1Val);
		} else if (1 == u1Idx) {
			_PVR_SelTSClk(1, u4TS1Val);
		}
	}
	smp_mb();
}
#else				/* DMX_SUPPORT_DEVICE_TREE */
static void _DmxSetTSInputCkgen(u8 u1Idx, PVR_FRONTEND_T eFrontEnd)
{
	PVR_FRONTEND_T ePreType;
	u32 u4TS1Val = 0, u4TS0Val = 0;

	PVR_LOG_TRACE(TEXT("[DMX][HW] %s (u1Idx: %d, eFrontEnd: %d)\r\r\n"),
		      DMX_FUNC_NAME, u1Idx, eFrontEnd);

	if (u1Idx >= PVR_FRAMER_COUNT) {
		PVR_LOG_ERR(TEXT("[DMX][HW] %s line %d fail for invalid u1Idx(%u)!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, u1Idx);
		return;
	}

	smp_mb();
	/* -------------------------- */
	/* Set Framer Ckgen */
	/* -------------------------- */
	ePreType = PVR_FE_NO_TSVALID;
	smp_mb();

	u4TS0Val = CKGEN_AgtGetClk(e_CLK_SEL_TS0);
	u4TS1Val = CKGEN_AgtGetClk(e_CLK_SEL_TS1);
	smp_mb();

	switch (eFrontEnd) {
	case PVR_FE_DDI:
		if (0 == u1Idx)
			u4TS0Val = CLK_REG9_TS0_SEL_DEMUX;
		else if (1 == u1Idx)
			u4TS1Val = CLK_REG9_TS1_SEL_DEMUX;
		break;
	case PVR_FE_EXT_S:
		if (0 == u1Idx)
			u4TS0Val = CLK_REG9_TS0_SEL_EXT_TS0;
		else if (1 == u1Idx)
			u4TS1Val = CLK_REG9_TS1_SEL_EXT_TS1;
		break;
	case PVR_FE_EXT_P:
		u4TS0Val = CLK_REG9_TS0_SEL_EXT_TS0;
		u4TS1Val = CLK_REG9_TS1_SEL_NOTHING;
		break;
	default:
		break;
	}

	smp_mb();
	if (_PVR_IsFramerEnabled(u1Idx)) {
		/* To avoid affecting framer logic, we disable framer before changing clock source. */
		_PVR_SetFramerEnabled(u1Idx, FALSE);
		smp_mb();

		if (0 == u1Idx) {
			CKGEN_AgtSelClk(e_CLK_SEL_TS0, u4TS0Val);
			if (PVR_FE_EXT_P == eFrontEnd)
				CKGEN_AgtSelClk(e_CLK_SEL_TS1, u4TS1Val);
		} else if (1 == u1Idx) {
			CKGEN_AgtSelClk(e_CLK_SEL_TS1, u4TS1Val);
		}

		smp_mb();
		_PVR_SetFramerEnabled(u1Idx, TRUE);
	} else {
		if (0 == u1Idx) {
			CKGEN_AgtSelClk(e_CLK_SEL_TS0, u4TS0Val);
			smp_mb();
			if (PVR_FE_EXT_P == eFrontEnd)
				CKGEN_AgtSelClk(e_CLK_SEL_TS1, u4TS1Val);
		} else if (1 == u1Idx) {
			CKGEN_AgtSelClk(e_CLK_SEL_TS1, u4TS1Val);
		}
	}
	smp_mb();
}
#endif				/* DMX_SUPPORT_DEVICE_TREE */

/*-----------------------------------------------------------------------------*/
/** _DmxSetAsyncFifoReset,
 * Framer u1Idx Async fifo reset.
 */
/*-----------------------------------------------------------------------------*/
static void _DmxSetAsyncFifoReset(u8 u1Idx, bool fgReset)
{
	u32 u4Ctrl;
	u8 au1Toggle[PVR_FRAMER_COUNT] = { 4, 5 };

	PVR_LOG_TRACE(TEXT("[DMX][HW] %s(u1Idx: %d, fgReset: %s)\r\n"),
		      DMX_FUNC_NAME, u1Idx, (fgReset ? "TRUE" : "FALSE"));

	if (u1Idx >= PVR_FRAMER_COUNT)
		DMX_ASSERT(FALSE);

	if (u1Idx == PVR_FRAMER_TOTAL_INDEX)
		return;

	smp_mb();
	u4Ctrl = DMXCMD_READ32(PVR_REG_DBM_BUF_CTRL) & ~(0x1 << au1Toggle[u1Idx]);

	smp_mb();
	if (fgReset)
		u4Ctrl = DMXCMD_READ32(PVR_REG_DBM_BUF_CTRL) | (0x1 << au1Toggle[u1Idx]);

	smp_mb();
	DMXCMD_WRITE32(PVR_REG_DBM_BUF_CTRL, u4Ctrl);
}


/*-----------------------------------------------------------------------------*/
/** _PVR_SetFrontEndEx
 */
/*-----------------------------------------------------------------------------*/
bool _PVR_SetFrontEndEx(u8 u1Idx, PVR_FRONTEND_T eFrontEnd)
{
	PVR_LOG_DBG(TEXT("[DMX][HW] %s(u1Idx: %d, eFrontEnd: %d) enter\r\n"),
		    DMX_FUNC_NAME, u1Idx, eFrontEnd);

	if (u1Idx >= PVR_FRAMER_COUNT) {
		PVR_LOG_ERR(TEXT
			    ("[DMX][HW] %s fail for Tsindex %u has no framer or wrong tsindex!\r\n"),
			    DMX_FUNC_NAME, u1Idx);
		return FALSE;
	}

	if (eFrontEnd == PVR_FE_NO_TSVALID) {
		/* this is for sepecial two tuner config, there is no TS_VAILD input */
		/* set it to 2 bit mode with serial input for ts index 0 and 1 */
		if (u1Idx == 0) {
			/* Set Framer0 input to be 2-bit asynchornous TS format */
			DMXCMD_WRITE32(PVR_REG_CONFIG2,
				       (DMXCMD_READ32(PVR_REG_CONFIG2) & (0xFCFFFFFF)) |
				       0x02000000);
			return TRUE;
		} else if (u1Idx == 1) {
			/* Set Framer1 input to be 2-bit asynchornous TS format */
			DMXCMD_WRITE32(PVR_REG_CONFIG2,
				       (DMXCMD_READ32(PVR_REG_CONFIG2) & (0xF3FFFFFF)) |
				       0x08000000);
			return TRUE;
		}
		PVR_LOG_ERR(TEXT("[DMX][HW] %s line %d PVR_FE_NO_TSVALID is only used")
			     TEXT(" for framer 0 or framer 1, buf Current FramerIdx = %u!\r\n"),
			    DMX_FUNC_NAME, DMX_FUNC_NAME, u1Idx);
		return FALSE;
	}

	smp_mb();
	/*--------------------*/
	/* Set async fifo reset */
	/*--------------------*/
	if (eFrontEnd > PVR_FE_DDI) {	/* External */
		_DmxSetAsyncFifoReset(u1Idx, FALSE);
	} else {
		_DmxSetAsyncFifoReset(u1Idx, TRUE);
	}

	smp_mb();
	if (eFrontEnd == PVR_FE_DDI) {
		_PVR_SetFramerMode(u1Idx, PVR_FRAMER_PARALLEL, TRUE, TRUE);
	} else {
		if ((eFrontEnd & 0x10) != 0) {	/* External Clock input */
			PVR_PINMUX_SEL_T ePinmux;

			/* Set pinmux */
			/*--------------------*/
			ePinmux = (PVR_PINMUX_SEL_T) eFrontEnd;
			smp_mb();
			if (PVR_FE_EXT_2BIT == eFrontEnd)
				ePinmux = PVR_PINMUX_EXT_S;

			smp_mb();
			_DmxSetExtPinmux(u1Idx, ePinmux);
		}

		smp_mb();
		switch ((u32) eFrontEnd & 0xF) {
		case 0:
			_PVR_SetFramerMode(u1Idx, PVR_FRAMER_SERIAL, TRUE, TRUE);
			break;
		case 1:
			_PVR_SetFramerMode(u1Idx, PVR_FRAMER_PARALLEL, TRUE, TRUE);
			break;
		case 2:
			_PVR_SetFramerMode(u1Idx, PVR_FRAMER_TWOBIT, TRUE, TRUE);
		default:
			break;
		}
	}

	smp_mb();
	/*--------------------*/
	/* Set framer input */
	/*--------------------*/
	_DmxSetTSInputCkgen(u1Idx, eFrontEnd);

	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- exit, success\r\n"), DMX_FUNC_NAME);

	return TRUE;
}

/*-----------------------------------------------------------------------------*/
/** _PVR_BypassErrorHandlingTable_Enable
	TRUE: bypass error handling table
	FALSE: DO framr error handling table
 */
/*-----------------------------------------------------------------------------*/
void _PVR_BypassErrorHandlingTable_Enable(u8 u1FramerIdx, bool fgEnable)
{
	u32 u4Reg;

	DMX_ASSERT(u1FramerIdx < PVR_FRAMER_COUNT);

	if (u1FramerIdx == 0) {
		if (fgEnable) {
			u4Reg = DMXCMD_READ32(PVR_REG_FRAMER_CTRL3);
			u4Reg |= (0x1 << 4);
			smp_mb();
			DMXCMD_WRITE32(PVR_REG_FRAMER_CTRL3, u4Reg);
		} else {
			u4Reg = DMXCMD_READ32(PVR_REG_FRAMER_CTRL3);
			u4Reg &= ~(0x1 << 4);
			smp_mb();
			DMXCMD_WRITE32(PVR_REG_FRAMER_CTRL3, u4Reg);
		}
	} else if (u1FramerIdx == 1) {
		if (fgEnable) {
			u4Reg = DMXCMD_READ32(PVR_REG_FRAMER_CTRL3);
			u4Reg |= (0x1 << 5);
			smp_mb();
			DMXCMD_WRITE32(PVR_REG_FRAMER_CTRL3, u4Reg);
		} else {
			u4Reg = DMXCMD_READ32(PVR_REG_FRAMER_CTRL3);
			u4Reg &= ~(0x1 << 5);
			smp_mb();
			DMXCMD_WRITE32(PVR_REG_FRAMER_CTRL3, u4Reg);
		}

	} else {
		PVR_LOG_ERR(TEXT("%s line %d -- Input the wrong Framer number(%u)!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, u1FramerIdx);
	}
}

bool _PVR_IsFramerEnabled(u8 u1Framer)
{
	bool fgEnable = FALSE;

	switch (u1Framer) {
	case 0:
		fgEnable = (0 != (DMXCMD_READ32(PVR_REG_FRAMER_CONTROL) & (0x01)));
		break;
	case 1:
		fgEnable = (0 != (DMXCMD_READ32(PVR_REG_FRAMER_CONTROL) & (0x10)));
		break;
	default:
		break;
	}
	smp_mb();
	return fgEnable;
}

/*-----------------------------------------------------------------------------*/
/** _PVR_SetFramer
 * Set Framer 0 or 1 's Params, such as Input mode, Sync Mode, Sample clock edge
 */
/*-----------------------------------------------------------------------------*/
void _PVR_SetFramer(u8 u1FramerIdx, bool fgEnable,
		    bool fgParallel, bool fgExtSync, bool fgPosEdge)
{
	u32 u4Reg, u4Val = 0x0;

	if (u1FramerIdx >= PVR_FRAMER_COUNT) {
		PVR_LOG_ERR(TEXT("[DMX][HW] %s line %d fail for invalid u1FramerIdx(%u)!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, u1FramerIdx);
		return;
	}

	smp_mb();
	/*disable framer first, before set framer again. */
	u4Reg = DMXCMD_READ32(PVR_REG_FRAMER_CONTROL);
	smp_mb();
	u4Reg &= ~(0x1 << (4 * u1FramerIdx));
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_FRAMER_CONTROL, u4Reg);

	/*set value */
	u4Val |= (fgEnable ? 0x1 : 0x0);	/*framer enable */
	u4Val |= ((fgParallel ? 0x1 : 0x0) << 1);	/*parallel or serial */
	u4Val |= ((fgExtSync ? 0x1 : 0x0) << 2);	/*external or internal */
	u4Val |= ((fgPosEdge ? 0x1 : 0x0) << 3);	/*external or internal */

	smp_mb();
	/*set framer */
	u4Reg = DMXCMD_READ32(PVR_REG_FRAMER_CONTROL);
	smp_mb();
	u4Reg &= ~(0xF << (4 * u1FramerIdx));
	smp_mb();
	u4Reg |= (u4Val << (4 * u1FramerIdx));
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_FRAMER_CONTROL, u4Reg);
}

void _PVR_EnableFramer(u8 u1FramerIdx, bool fgEnable)
{
	u32 u4Reg;

	if (u1FramerIdx >= PVR_FRAMER_COUNT)
		return;

	smp_mb();
	if (fgEnable) {
		/*set framer */
		u4Reg = DMXCMD_READ32(PVR_REG_FRAMER_CONTROL);
		smp_mb();
		u4Reg |= (0x1 << (4 * u1FramerIdx));
		smp_mb();
		DMXCMD_WRITE32(PVR_REG_FRAMER_CONTROL, u4Reg);
		PVR_LOG_TRACE(TEXT
			      ("[DMX][HW] %s(%u, TRUE) line %d -- Framer Control Register Value = 0x%08x\r\r\n"),
			      DMX_FUNC_NAME, u1FramerIdx, DMX_LINE_NO,
			      DMXCMD_READ32(PVR_REG_FRAMER_CONTROL));
	} else {
		/*disable framer first, before set framer again. */
		u4Reg = DMXCMD_READ32(PVR_REG_FRAMER_CONTROL);
		smp_mb();
		u4Reg &= ~(0x1 << (4 * u1FramerIdx));
		smp_mb();
		DMXCMD_WRITE32(PVR_REG_FRAMER_CONTROL, u4Reg);
		PVR_LOG_TRACE(TEXT
			      ("[DMX][HW] %s(%u, FALSE) line %d -- Framer Control Register Value = 0x%08x\r\r\n"),
			      DMX_FUNC_NAME, u1FramerIdx, DMX_LINE_NO,
			      DMXCMD_READ32(PVR_REG_FRAMER_CONTROL));
	}
}

/*-----------------------------------------------------------------------------*/
/** _PVR_Framer_130byteEnable

 */
/*-----------------------------------------------------------------------------*/
bool _PVR_Framer_130byteEnable(u8 u1Framer, bool fgDemod130byteTs,
			       bool fgInputEnable, u8 u1InputPktSize,
			       bool fgOutputEnable, u8 u1OutputPktSize)
{
	u32 u4Reg;

	DMX_ASSERT(u1Framer < PVR_FRAMER_COUNT);

	if (u1Framer == 0) {
		/*set framer 0 input packet size */
		u4Reg = DMXCMD_READ32(PVR_REG_FRAMER_PREBYTE_CTRL);
		u4Reg &= ~(0xFF);
		u4Reg |= u1InputPktSize;
		smp_mb();
		DMXCMD_WRITE32(PVR_REG_FRAMER_PREBYTE_CTRL, u4Reg);

		if (fgInputEnable) {
			/*set framer0 pre-byte scheme enable */
			u4Reg = DMXCMD_READ32(PVR_REG_FRAMER_PREBYTE_CTRL);
			u4Reg |= (0x1 << 8);
			smp_mb();
			DMXCMD_WRITE32(PVR_REG_FRAMER_PREBYTE_CTRL, u4Reg);
		} else {
			/*set framer0 pre-byte scheme disable */
			u4Reg = DMXCMD_READ32(PVR_REG_FRAMER_PREBYTE_CTRL);
			u4Reg &= ~(0x1 << 8);
			smp_mb();
			DMXCMD_WRITE32(PVR_REG_FRAMER_PREBYTE_CTRL, u4Reg);
		}

		smp_mb();
		/*set framer 0 output packet size */
		u4Reg = DMXCMD_READ32(PVR_REG_CONFIG4);
		u4Reg &= ~(0xFF);
		u4Reg |= (u1OutputPktSize);
		DMXCMD_WRITE32(PVR_REG_CONFIG4, u4Reg);

		smp_mb();
		/*set output enable and dbm auto switch */
		if (fgOutputEnable) {
			u4Reg = DMXCMD_READ32(PVR_REG_CONFIG4);
			u4Reg |= (0x1 << 24);	/*Framer 0 output pkg size use PVR_REG_CONFIG4's bit[7:0] */
			u4Reg |= (0x1 << 27);	/*dbm output pkg size use auto switch pkg size */
			smp_mb();
			DMXCMD_WRITE32(PVR_REG_CONFIG4, u4Reg);
		} else {
			u4Reg = DMXCMD_READ32(PVR_REG_CONFIG4);
			u4Reg &= ~(0x1 << 24);	/*Framer 0 output pkg size use FTI Conguration Register 2 defined */
			u4Reg &= ~(0x1 << 27);	/*dbm output pkg size use FTI Conguration Register 2 defined */
			smp_mb();
			DMXCMD_WRITE32(PVR_REG_CONFIG4, u4Reg);
		}

		smp_mb();
		/*Bit[28]: External Tuner and internal Demod input to Framer0 is */
		/*130Byte TS. 0: Normal TS Packet, 1: 130Byte TS. */
		if (fgDemod130byteTs) {
			u4Reg = DMXCMD_READ32(PVR_REG_CONFIG4);
			u4Reg |= (0x1 << 28);
			smp_mb();
			DMXCMD_WRITE32(PVR_REG_CONFIG4, u4Reg);
		} else {
			u4Reg = DMXCMD_READ32(PVR_REG_CONFIG4);
			u4Reg &= ~(0x1 << 28);
			smp_mb();
			DMXCMD_WRITE32(PVR_REG_CONFIG4, u4Reg);
		}

	} else if (u1Framer == 1) {
		/*set framer 0 input packet size */
		u4Reg = DMXCMD_READ32(PVR_REG_FRAMER_PREBYTE_CTRL);
		u4Reg &= ~(0xFF << 16);
		u4Reg |= (u1InputPktSize << 16);
		smp_mb();
		DMXCMD_WRITE32(PVR_REG_FRAMER_PREBYTE_CTRL, u4Reg);

		smp_mb();
		/*set input enable */
		if (fgInputEnable) {
			/*set framer1 pre-byte scheme enable */
			u4Reg = DMXCMD_READ32(PVR_REG_FRAMER_PREBYTE_CTRL);
			u4Reg |= (0x1 << 24);
			smp_mb();
			DMXCMD_WRITE32(PVR_REG_FRAMER_PREBYTE_CTRL, u4Reg);
		} else {
			/*set framer1 pre-byte scheme disable */
			u4Reg = DMXCMD_READ32(PVR_REG_FRAMER_PREBYTE_CTRL);
			u4Reg &= ~(0x1 << 24);
			smp_mb();
			DMXCMD_WRITE32(PVR_REG_FRAMER_PREBYTE_CTRL, u4Reg);
		}

		smp_mb();
		/*set framer 1 output packet size */
		u4Reg = DMXCMD_READ32(PVR_REG_CONFIG4);
		smp_mb();
		u4Reg &= ~(0xFF << 8);
		smp_mb();
		u4Reg |= (u1OutputPktSize << 8);
		smp_mb();
		DMXCMD_WRITE32(PVR_REG_CONFIG4, u4Reg);

		/*set output enable and dbm auto switch */
		smp_mb();
		if (fgOutputEnable) {
			u4Reg = DMXCMD_READ32(PVR_REG_CONFIG4);
			smp_mb();
			u4Reg |= (0x1 << 25);	/*Framer 1 output pkg size use PVR_REG_CONFIG4's bit[7:0] */
			u4Reg |= (0x1 << 27);	/*dbm output pkg size use auto switch pkg size */
			smp_mb();
			DMXCMD_WRITE32(PVR_REG_CONFIG4, u4Reg);
		} else {
			u4Reg = DMXCMD_READ32(PVR_REG_CONFIG4);
			smp_mb();
			u4Reg &= ~(0x1 << 25);	/*Framer 1 output pkg size use FTI Conguration Register 2 defined */
			u4Reg &= ~(0x1 << 27);	/*dbm output pkg size use auto switch pkg size */
			smp_mb();
			DMXCMD_WRITE32(PVR_REG_CONFIG4, u4Reg);
		}

		smp_mb();
		/*Bit[29]: External Tuner and internal Demod input to Framer1 is */
		/*130Byte TS. 0: Normal TS Packet, 1: 130Byte TS. */
		if (fgDemod130byteTs) {
			u4Reg = DMXCMD_READ32(PVR_REG_CONFIG4);
			u4Reg |= (0x1 << 29);
			smp_mb();
			DMXCMD_WRITE32(PVR_REG_CONFIG4, u4Reg);
		} else {
			u4Reg = DMXCMD_READ32(PVR_REG_CONFIG4);
			u4Reg &= ~(0x1 << 29);
			smp_mb();
			DMXCMD_WRITE32(PVR_REG_CONFIG4, u4Reg);
		}

	} else {
		PVR_LOG_ERR(TEXT("%s line %d -- Input the wrong Framer number(%u)!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, u1Framer);
		return FALSE;
	}

	smp_mb();

	return TRUE;

}

/*-----------------------------------------------------------------------------*/
/** _PVR_SetFramerMode
 */
/*-----------------------------------------------------------------------------*/
void _PVR_SetFramerMode(u8 u1Framer, PVR_FRAMER_MODE_T eMode, bool fgExtSync, bool fgPosEdge)
{
	u32 u4Ctrl, u4RegOffset;
	u8 u1FramerControl;
	u32 u4Ctrl1;

	PVR_LOG_DBG(TEXT
		    ("[DMX][HW] %s(u1Framer: %u, eMode: %d, fgExtSync: %s, fgPosEdge: %s)\r\r\n"),
		    DMX_FUNC_NAME, u1Framer, eMode, (fgExtSync ? "TRUE" : "FALSE"),
		    (fgPosEdge ? "TRUE" : "FALSE"));

	if (u1Framer >= PVR_FRAMER_COUNT) {
		PVR_LOG_ERR(TEXT("[DMX][HW] %s line %d fail for invalid u1Framer(%u).\r\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, u1Framer);
		return;
	}

	smp_mb();
	if (eMode == PVR_FRAMER_TWOBIT) {
		u4Ctrl1 = DMXCMD_READ32(PVR_REG_CONFIG1);

		if (u1Framer == 0) {
			/*bit25: framer0 input is 2-bit asynchonous TS format(4pin 2bit, from internal PVR or demod) */
			/*bit24: framer0 input is 2-bit synchonous TS format(4pin 2bit, from external IC) */
			u4Ctrl = DMXCMD_READ32(PVR_REG_CONFIG2);
			u4Ctrl &= ~(3 << 24);	/*clear framer 0 input 2-bit asynchonous or synchonous TS format */
			u4Ctrl |= (2 << 24);	/*Set framer 0 input 2-bit asynchonous TS format */
			smp_mb();
			DMXCMD_WRITE32(PVR_REG_CONFIG2, u4Ctrl);

			smp_mb();
			/* framer 0 TS input from external TS is 2-bit mode, 0-disable, 1-enable */
			u4Ctrl1 |= (1 << 26);
			smp_mb();
			PVR_LOG_DBG(TEXT("[DMX][HW] %s line %d Set Framer 0 2-bit mode!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);
		} else if (u1Framer == 1) {
			/*bit27: framer1 input is 2-bit asynchonous TS format(4pin 2bit, from internal PVR or demod) */
			/*bit26: framer1 input is 2-bit synchonous TS format(4pin 2bit, from external IC) */
			u4Ctrl = DMXCMD_READ32(PVR_REG_CONFIG2);
			u4Ctrl &= ~(3 << 26);	/*clear framer 1 input 2-bit asynchonous or synchonous TS format */
			u4Ctrl |= (2 << 26);	/*Set framer 1 input 2-bit asynchonous TS format */
			smp_mb();
			DMXCMD_WRITE32(PVR_REG_CONFIG2, u4Ctrl);

			smp_mb();
			/* framer 1 TS input from external TS is 2-bit mode, 0-disable, 1-enable */
			u4Ctrl1 |= (1 << 27);
			smp_mb();
			PVR_LOG_DBG(TEXT("[DMX][HW] %s line %d Set Framer 1 2-bit mode!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);
		} else {
			PVR_LOG_DBG(TEXT("[DMX][HW] %s line %d Framer index error!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);
		}

		smp_mb();
		DMXCMD_WRITE32(PVR_REG_CONFIG1, u4Ctrl1);
	} else {
		u4Ctrl1 = DMXCMD_READ32(PVR_REG_CONFIG1);

		smp_mb();
		if (u1Framer == 0) {
			u4Ctrl = DMXCMD_READ32(PVR_REG_CONFIG2);
			/*clear framer 0 input 2-bit asynchonous or synchonous TS format */
			u4Ctrl &= ~(3 << 24);
			smp_mb();
			DMXCMD_WRITE32(PVR_REG_CONFIG2, u4Ctrl);
			smp_mb();
			/* framer 0 TS input from external TS is 2-bit mode, 0-disable, 1-enable */
			u4Ctrl1 &= ~(1 << 26);
			smp_mb();
			PVR_LOG_DBG(TEXT
				    ("[DMX][HW] %s line %d Disable Framer 0 2-bit mode, FTI Cfg2 Reg: 0x%x!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, DMXCMD_READ32(PVR_REG_CONFIG2));
		} else if (u1Framer == 1) {
			u4Ctrl = DMXCMD_READ32(PVR_REG_CONFIG2);
			/*clear framer 1 input 2-bit asynchonous or synchonous TS format */
			u4Ctrl &= ~(3 << 26);
			smp_mb();
			DMXCMD_WRITE32(PVR_REG_CONFIG2, u4Ctrl);
			smp_mb();
			/* framer 1 TS input from external TS is 2-bit mode, 0-disable, 1-enable */
			u4Ctrl1 &= ~(1 << 27);
			smp_mb();
			PVR_LOG_DBG(TEXT
				    ("[DMX][HW] %s line %d Disable Framer 1 2-bit mode, FTI Cfg2 Reg: 0x%x!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, DMXCMD_READ32(PVR_REG_CONFIG2));
		} else {
			PVR_LOG_DBG(TEXT("[DMX][HW] %s line %d Framer index error!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);
		}

		smp_mb();
		DMXCMD_WRITE32(PVR_REG_CONFIG1, u4Ctrl1);
	}

	smp_mb();
	u1FramerControl = 0x0;	/* disable */
	smp_mb();

	if ((eMode == PVR_FRAMER_PARALLEL) || (eMode == PVR_FRAMER_TWOBIT))
		u1FramerControl |= 0x2;	/* Parallel input */

	smp_mb();
	if (fgExtSync)
		u1FramerControl |= 0x4;	/* External Sync */


	if (fgPosEdge)
		u1FramerControl |= 0x8;	/* Sample on Positive Clock Edge */

	smp_mb();
	u4Ctrl = DMXCMD_READ32(PVR_REG_FRAMER_CONTROL);
	u4RegOffset = (u32) u1Framer;
	smp_mb();
	if (((u4Ctrl >> (u4RegOffset * 4)) & 0xF) != u1FramerControl) {
		/*disable framer before set framer again */
		u4Ctrl = (u4Ctrl & ~((0xf) << (4 * u4RegOffset)));
		smp_mb();
		DMXCMD_WRITE32(PVR_REG_FRAMER_CONTROL, u4Ctrl);
		smp_mb();
		/*set framer again, Becarefull */
		u4Ctrl =
		    (u4Ctrl & ~((0xf) << (4 * u4RegOffset))) | ((u1FramerControl) <<
								(4 * u4RegOffset));
		smp_mb();
		DMXCMD_WRITE32(PVR_REG_FRAMER_CONTROL, u4Ctrl);

		smp_mb();
		_PVR_ResetFramer(u1Framer);
	}
}


/*-----------------------------------------------------------------------------*/
/** _DmxLoadIMem
 *	@retval TRUE			Succeed
 *	@retval FALSE			Fail
 */
/*-----------------------------------------------------------------------------*/
static bool _DmxLoadIMem(bool fgForceWrite, const u32 *pu4IData, u32 u4Len)
{
	u32 u4Cmd, u4Data, i;
	u16 u2CodeVersion;
	u32 u4Reg = 0;
	u8 u1Major, u1Minor;

	FUNC_ENTRY;

	if (NULL == pu4IData) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail for invalid args -- pu4IDate == NULL!\r\n"),
			    DMX_FUNC_NAME);
		FUNC_EXIT;
		return FALSE;
	}

	/* Check uP status */
	if (_DmxIsFTIRunning()) {
		PVR_LOG_DBG(TEXT("[DMX][HW] %s line %d -- FTI is running\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		if (!fgForceWrite) {
			PVR_LOG_ERR(TEXT
				    ("[DMX][HW] %s line %d --FTI uP is running, can't load to I-mem!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);
			FUNC_EXIT;
			return FALSE;
		}
		PVR_LOG_DBG(TEXT
			    ("[DMX][HW] %s line %d --FTI uP is running, now will halt it\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		if (!_PVR_EnableFTI(FALSE)) {
			PVR_LOG_ERR(TEXT
				    ("[PVR] %s line %d fail in _PVR_EnableFTI(FALSE)!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);
			return FALSE;
		}

		while (_DmxIsFTIRunning()) {
			PVR_LOG_DBG(TEXT("[DMX][HW] %s line %d --wait uP stop...\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);
		}
	}

	/* Bit25: FTUP instruction mode, 0-32bit instruction mode, 1-25bit instruction mode */
	u4Reg = DMXCMD_READ32(PVR_REG_FTUP_CONTROL);
	u4Reg &= ~(1 << 25);
	DMXCMD_WRITE32(PVR_REG_FTUP_CONTROL, u4Reg);

	PVR_LOG_DBG(TEXT("[DMX][HW] %s line %d -- Load IMem\r\n"), DMX_FUNC_NAME, DMX_LINE_NO);

	/* Load to I-mem */
	for (i = 0; i < u4Len; i++) {
		_PVR_Lock();

		/* Issue a write command */
		DMXCMD_WRITE32(PVR_REG_MEM_DATA, pu4IData[i]);
		/* instruction memory, write, no interrupt, write address */
		u4Cmd = (u32) ((1 << 0) | (2 << 8) | (i << 16));
		DMXCMD_WRITE32(PVR_REG_MEM_CMD, u4Cmd);

		/* Wait for ready */
		while (1) {
			u4Cmd = DMXCMD_READ32(PVR_REG_MEM_CMD);
			if (((u4Cmd >> 8) & 0xf) == 0)
				break;
		}

		_PVR_Unlock();
	}

	PVR_LOG_DBG(TEXT
		    ("[DMX][HW] %s line %d -- Load IMem End, Start to verify whether load OK\r\n"),
		    DMX_FUNC_NAME, DMX_LINE_NO);

	/* Verify */
	for (i = 0; i < u4Len; i++) {
		_PVR_Lock();

		/* Issue a read command */
		/* instruction memory, read, no interrupt, write address */
		u4Cmd = (u32) ((1 << 0) | (1 << 8) | (i << 16));
		DMXCMD_WRITE32(PVR_REG_MEM_CMD, u4Cmd);

		/* Wait for ready */
		while (1) {
			u4Cmd = DMXCMD_READ32(PVR_REG_MEM_CMD);
			if (((u4Cmd >> 8) & 0xf) == 0)
				break;
		}
		/* Verify */
		u4Data = DMXCMD_READ32(PVR_REG_MEM_DATA);

		_PVR_Unlock();

		if (u4Data != pu4IData[i]) {
			PVR_LOG_TRACE(TEXT("[DMX][HW] %s line %d -- Load IMem error, ")
				       TEXT("at word %u, write 0x%08x, read 0x%08x\r\n"),
				      DMX_FUNC_NAME, DMX_LINE_NO, i, pu4IData[i], u4Data);
			return FALSE;
		}
	}

	PVR_LOG_DBG(TEXT("[DMX][HW] %s line %d -- Verify End\r\n"), DMX_FUNC_NAME, DMX_LINE_NO);

	_PVR_Lock();
	_rDmxPvrInfo.u4uCodeVersion = pu4IData[0];
	_PVR_Unlock();

	u2CodeVersion = (u16) (pu4IData[0] & 0xffff);
	u1Major = (u8) ((u2CodeVersion >> 8) & 0xf);
	u1Minor = (u8) (u2CodeVersion & 0xff);
	PVR_LOG_TRACE(TEXT("[DMX][HW] Demux uCode version: %u.%u\r\n"), u1Major, u1Minor);

	/* Reset demux (uP), so uP can run into the entry pointer of uCode */
	VERIFY(_PVR_Reset());

	/* Suppress warnings in lint and release build */
	UNUSED(u1Major);
	UNUSED(u1Minor);

	PVR_LOG_DBG(TEXT("[DMX][HW] %s line %d -- exit, success\r\n"), DMX_FUNC_NAME, DMX_LINE_NO);

	return TRUE;
}


/*-----------------------------------------------------------------------------*/
/** _PVR_LoadIMem
 *	@retval TRUE			Succeed
 *	@retval FALSE			Fail
 */
/*-----------------------------------------------------------------------------*/
bool _PVR_LoadIMem(bool fgForceWrite, const u32 *pu4IData, u32 u4Len)
{
	u8 i = 0;
	bool fgRet;

	PVR_LOG_DBG(TEXT("[DMX][HW] %s line %d -- enter\r\n"), DMX_FUNC_NAME, DMX_LINE_NO);

	for (i = 0; i < 10; i++) {
		PVR_LOG_DBG(TEXT("[DMX][HW] %s line %d -- _DmxLoadIMem, i=%d\r\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, i);
		fgRet = _DmxLoadIMem(fgForceWrite, pu4IData, u4Len);
		if (fgRet) {
			PVR_LOG_DBG(TEXT("[DMX][HW] %s line %d -- exit, success, i=%d\r\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, i);
			return TRUE;
		}
	}

	PVR_LOG_ERR(TEXT("[DMX][HW] %s line %d -- fail in _DmxLoadIMem 10 times\r\n"),
		    DMX_FUNC_NAME, DMX_LINE_NO);

	PVR_LOG_ERR(TEXT("[DMX][HW] %s line %d -- exit, fail!!!\r\n"), DMX_FUNC_NAME, DMX_LINE_NO);

	return FALSE;
}

/*-----------------------------------------------------------------------------*/
/** _PVR_LoaduCode
 *	Load default uCode
*/
/*-----------------------------------------------------------------------------*/
bool _PVR_LoaduCode(void)
{
	if (!_PVR_LoadIMem(TRUE, FTI_IMEM, FTI_IMEM_LEN))
		return FALSE;

	smp_mb();
	return TRUE;
}

/*-----------------------------------------------------------------------------*/
/** _DmxInitFVR
 */
/*-----------------------------------------------------------------------------*/
static void _DmxInitFVR(void)
{
	u32 i, u4Ctrl;

	/*-----------------------------------------------------*/
	/* Reset DMX Record hardware */
	/*-----------------------------------------------------*/

	/* Reset the Record Global Region */
	memset_io((void *)&(FVR_GBL_ARY_W(0, 0)), 0, FVR_GBL_SIZE * 4);
	memset_io((void *)&(FVR_GBL_ARY_W(1, 0)), 0, FVR_GBL_SIZE * 4);

	/*reset DMX record hardware common region 124byte */
	memset_io((void *)FVR_GBL_COMMON_REGION, 0, FVR_GBL_COMMON_SIZE * 4);

	/* Reset the Record PID data structure region */
	for (i = 0; i < FVR_NUM_PID_INDEX; i++) {
		FVR_PID_INDEX_TABLE(i) = 0;
		memset_io((void *)&(FVR_PER_PID_S(i)), 0, FVR_PID_SIZE * 4);
	}

	/* set the Record PID data structure start offset */
	u4Ctrl = DMXCMD_READ32(PVR_REG_PID_STRUCT_OFFSET) & 0xFFFF;
	u4Ctrl |= (FVR_PER_PID_OFFSET) << 16;
	DMXCMD_WRITE32(PVR_REG_PID_STRUCT_OFFSET, u4Ctrl);

	u4Ctrl = DMXCMD_READ32(PVR_REG_CONFIG1) & 0xFD00FEFF;
	u4Ctrl |= (1 << 25);	/* Disable record path */
	u4Ctrl |= ((FVR_PID_SIZE * 4) << 16);	/* Set record pid data structure size */
	/* Direct map (Record PID data structure index is direct map to PID index) */
	u4Ctrl |= (1 << 8);
	DMXCMD_WRITE32(PVR_REG_CONFIG1, u4Ctrl);

	/* Disable insert timestamp */
	/* Bit29: Record Packets need to be inserted time stamp, 0: Disable insert timestamp, 1: Enable */
	u4Ctrl = DMXCMD_READ32(PVR_REG_CONFIG2) & 0xDFFFFFFF;
	DMXCMD_WRITE32(PVR_REG_CONFIG2, u4Ctrl);
}


/*_PVR_SetDbmChannel4: enable channel4*/
bool _PVR_SetDbmChannel4(bool fg_playback_enable, bool fg_record_enable)
{
	u32 u4Ctrl;

	/* Bit[6] RC_TS_IDX2_EN: Record PID filter check the TS index bit[2] value enable, 0-disable, 1-enable */
	/* Bit[7] PB_TS_IDX2_EN: Playback PID filter check the TS index bit[2] value enable, 0-disable, 1-enable */
	/* When Bit[6] or Bit[7] is set to 1, the DBM will extend to 5 channel, else DBM working in 4 Channel. */
	u4Ctrl = DMXCMD_READ32(PVR_REG_DBM_MULTI_STREAM_MODE) & ~(0xC0);
	smp_mb();
	if (fg_playback_enable)
		u4Ctrl = u4Ctrl | (1 << 7);

	if (fg_record_enable)
		u4Ctrl = u4Ctrl | (1 << 6);

	smp_mb();

	DMXCMD_WRITE32(PVR_REG_DBM_MULTI_STREAM_MODE, u4Ctrl);

	return TRUE;

}

/*get the dbmchannel5 status: true:enable , false:disable*/
bool _PVR_GetDbmChannel4(bool *fg_playback_enable, bool *fg_record_enable)
{
	if ((fg_playback_enable == NULL) || (fg_record_enable == NULL)) {
		PVR_LOG_ERR(TEXT("[DMX][HW] %s line %d fail for invalid args.\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		return FALSE;
	}

	smp_mb();
	(*fg_playback_enable) = (DMXCMD_READ32(PVR_REG_DBM_MULTI_STREAM_MODE) >> 7) & 0x1;
	(*fg_record_enable) = (DMXCMD_READ32(PVR_REG_DBM_MULTI_STREAM_MODE) >> 6) & 0x1;

	smp_mb();
	if ((*fg_playback_enable) || (*fg_record_enable))
		return TRUE;

	return FALSE;
}

void _PVR_Set_FTuPDMAThreshold(u32 u4DMAThreshold)
{
	u32 u4Reg;

	u4Reg = DMXCMD_READ32(PVR_REG_FTUP_CONTROL);
	u4Reg &= 0xFF00FFFF;
	u4Reg |= u4DMAThreshold << 16;
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_FTUP_CONTROL, u4Reg);
}

/*-----------------------------------------------------------------------------*/
/** _DmxInitHWPath
 */
/*-----------------------------------------------------------------------------*/
static bool _DmxInitHWPath(void)
{
	u32 i = 0;
	u32 u4Reg = 0;

	/*/> Initialize Framer1 and Framer2 input source */

	for (i = 0; i < PVR_FRAMER_TOTAL_INDEX; i++) {
		if (!_PVR_SetFrontEndEx(i, PVR_FE_NO_TSVALID))
			return FALSE;
	}

	/*/> Initialize MiniPVR Path, disable its output */

	/* Reset MiniPVR */
	DMXCMD_WRITE32(PVR_REG_PVR_CONTROL, 0x100);
	DMX_THREAD_DELAY(1);
	DMXCMD_WRITE32(PVR_REG_PVR_CONTROL, 0x0);

	/* Clear MiniPVR control */
	_PVR_Lock();
	DMXCMD_WRITE32(PVR_REG_PVR_CONTROL, 0x0);
	_PVR_Unlock();

	/* DBM TsIndex2 's input source to be PB */
	_PVR_SetDbm_InputSource(2, PVR_DBM_INPUT_PB);

	/* Set MiniPVR Framer's input packet size to be 188 */
	u4Reg = DMXCMD_READ32(PVR_REG_FRAMER_CTRL2) & ~0xFF000000;
	u4Reg |= ((u32) (188 & 0xFF) << 24);
	DMXCMD_WRITE32(PVR_REG_FRAMER_CTRL2, u4Reg);

	u4Reg = DMXCMD_READ32(PVR_REG_FRAMER_CTRL1);
	u4Reg |= 0x1 << 2;	/* enable pre-byte scheme */
	DMXCMD_WRITE32(PVR_REG_FRAMER_CTRL1, u4Reg);

	/* diable MiniPVR Framer */
	u4Reg = DMXCMD_READ32(PVR_REG_FRAMER_CTRL1);
	if ((u4Reg & 0x01) != 0) {
		u4Reg &= ~0x01;
		DMXCMD_WRITE32(PVR_REG_FRAMER_CTRL1, u4Reg);	/* diable framer */
		/* wait for framer state to idle */
		for (i = 0; i < 100; i++) {
			u4Reg = DMXCMD_READ32(PVR_REG_FRAMER_CTRL1);
			if (((u4Reg >> 16) & 0xFF) == 0x01) {
				/* in idle state */
				break;
			}
			DMX_THREAD_DELAY(10);
		}

		if (i >= 100) {
			PVR_LOG_ERR(TEXT("[PVR] %s line %d fail in disable MiniPVR Framer\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);
			return FALSE;
		}
	}

	/* Set Playback Steering Framer Reset */
	u4Reg = DMXCMD_READ32(PVR_REG_FRAMER_CTRL1) & (~0x10);
	u4Reg |= 0x1 << 4;
	DMXCMD_WRITE32(PVR_REG_FRAMER_CTRL1, u4Reg);
	DMX_THREAD_DELAY(1);
	u4Reg &= ~(0x1 << 4);
	DMXCMD_WRITE32(PVR_REG_FRAMER_CTRL1, u4Reg);

	/* Disable PES/DBM path */
	u4Reg = DMXCMD_READ32(PVR_REG_PES_DBM_STEER_CTRL) & 0xFFFCFFFF;
	u4Reg &= ~(1 << 17);
	DMXCMD_WRITE32(PVR_REG_PES_DBM_STEER_CTRL, u4Reg);

	return TRUE;
}

static bool _DmxInitMultimedia(void)
{
	u32 u4Ctrl;
	u32 i = 0;
	PVR_DDI_T rDDI;
	DMX_PVRPLAY_STRUCT_T rPlay;

	FUNC_ENTRY;

	_PVR_Lock();

	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- Clear PVR_DMEM_BASE, %p, sz: %d\r\n"),
		DMX_FUNC_NAME, PVR_DMEM_BASE, PVR_DMEM_MAX_LEN);

	memset_io((void *)(PVR_DMEM_BASE), 0, PVR_DMEM_MAX_LEN);

	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- Clear PLAYBACK_GBL_BASE, %p, sz: %d\r\n"),
		DMX_FUNC_NAME, PLAYBACK_GBL_BASE, PLAYBACK_GBL_SIZE * 4);
	/*---------------------------------------------------------*/
	/* Clear Play back global region */
	/*---------------------------------------------------------*/
	memset_io((void *)PLAYBACK_GBL_BASE, 0, PLAYBACK_GBL_SIZE * 4);

	mb();

	smp_mb();

	/*---------------------------------------------------------*/
	/* Clear DMX SRAM and struct */
	/*---------------------------------------------------------*/

	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- Clear HW Playback PID index table and Playback PID Data Structure\r\n"),
	    DMX_FUNC_NAME);
	/* Clear HW Playback PID index table and Playback PID Data Structure */
	for (i = 0; i < PVR_NUM_PID_INDEX; i++) {
		PID_INDEX_TABLE(i) = 0;
		PVR_LOG_DBG(TEXT("[DMX][HW] %s -- Clear &(PID_S(i)): %p, sz: %d\r\n"),
			DMX_FUNC_NAME, (void *)&(PID_S(i)), PVR_DMEM_ENTRY_LEN * 4);
		memset_io((void *)&(PID_S(i)), 0, PVR_DMEM_ENTRY_LEN * 4);
		/* Debug, reset continuity counter to 0xff */
		PID_S_W(i, 3) = 0xffff0000;
	}
	smp_mb();

	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- Clear HW Section filters\r\n"), DMX_FUNC_NAME);
	/* Clear Section filter */
	/* HW Generic Section Filter has 80 entries */
	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- Clear PVR_SECTION_CONTROL: %p\r\n"), DMX_FUNC_NAME, PVR_SECTION_CONTROL);
	memset_io((void *)PVR_SECTION_CONTROL, 0, 4 * PVR_NUM_FILTER_INDEX);
	mb();
	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- Clear PVR_SECTION_POSNEG: %p\r\n"), DMX_FUNC_NAME, PVR_SECTION_POSNEG);
	memset_io((void *)PVR_SECTION_POSNEG, 0, 8 * PVR_NUM_FILTER_INDEX);
	mb();
	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- Clear PVR_SECTION_PATTERN: %p\r\n"), DMX_FUNC_NAME, PVR_SECTION_PATTERN);
	memset_io((void *)PVR_SECTION_PATTERN, 0, 8 * PVR_NUM_FILTER_INDEX);
	mb();
	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- Clear PVR_SECTION_MASK: %p\r\n"), DMX_FUNC_NAME, PVR_SECTION_MASK);
	memset_io((void *)PVR_SECTION_MASK, 0, 8 * PVR_NUM_FILTER_INDEX);
	mb();

	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- Clear One-Byte Section filters: %p, size: %d\r\n"),
		DMX_FUNC_NAME, (void *)PVR_ONEBYTE_FILTER_BASE, 2 * PVR_NUM_ONEBYTE_FILTER_INDEX);
	/* One-Byte Section Filter Patten-Mask Region */
	memset_io((void *)PVR_ONEBYTE_FILTER_BASE, 0, 2 * PVR_NUM_ONEBYTE_FILTER_INDEX);
	smp_mb();

	/* CT setting init */
	VERIFY(_PVR_CTInit());
	smp_mb();

	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- Set Playback PID structure Start Offset\r\n"),
		    DMX_FUNC_NAME);
	/* Set Playback PID structure Start Offset */
	u4Ctrl = DMXCMD_READ32(PVR_REG_PID_STRUCT_OFFSET) & 0xFFFF0000;
	DMXCMD_WRITE32(PVR_REG_PID_STRUCT_OFFSET, u4Ctrl);

	_PVR_Unlock();
	smp_mb();

	/* Initialize interrupt handler */
	if (!_PVR_InitISR()) {
		PVR_LOG_ERR(TEXT("[PVR] %s line %d fail in _PVR_InitISR()!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		FUNC_EXIT;
		return FALSE;
	}

	_PVR_Lock();

	smp_mb();

	/*-----------------------------------------------------*/
	/* Frmaer Error Table */
	/*-----------------------------------------------------*/
	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- Set Framer 0 & 1 Error Handling Table\r\n"),
		    DMX_FUNC_NAME);
	/* enable error handling */
	_PVR_SetFramerPacketErrorHandling(0, TRUE, 0xF0C0);
	_PVR_SetFramerPacketErrorHandling(1, TRUE, 0xF0C0);
	smp_mb();

	/*-----------------------------------------------------*/
	/* Record default setting */
	/*-----------------------------------------------------*/
	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- Disable Record Path\r\n"), DMX_FUNC_NAME);
	/* Disable record */
	u4Ctrl = DMXCMD_READ32(PVR_REG_CONFIG1) | (1 << 25);
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_CONFIG1, u4Ctrl);

	smp_mb();

	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- Set packet length to 188 bytes\r\n"), DMX_FUNC_NAME);
	/* Set packet length to 188 bytes */
	u4Ctrl = DMXCMD_READ32(PVR_REG_CONFIG2);
	u4Ctrl = (u4Ctrl & 0xFFFFFF00) | 0x08;	/* maximum burst size */
	u4Ctrl = (u4Ctrl & 0xFFFF00FF) | (188 << 8);	/* packet len = 188 bytes */
	u4Ctrl = (u4Ctrl & 0xFF00FFFF) | (0x47 << 16);	/* TS packet sync byte */
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_CONFIG2, u4Ctrl);

	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- Set DBM to normal mode and enable DBM\r\n"),
		    DMX_FUNC_NAME);

	smp_mb();

	/* Diable All Framer */
	for (i = 0; i < PVR_FRAMER_COUNT; i++)
		_PVR_SetFramerMode(i, PVR_FRAMER_PARALLEL, TRUE, TRUE);

	smp_mb();

	/* Set DBM to normal mode and enable DBM */
	u4Ctrl = DMXCMD_READ32(PVR_REG_DBM_CONTROL);
	u4Ctrl = (u4Ctrl & 0xcfffffff) | (1 << 30);
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_DBM_CONTROL, u4Ctrl);

	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- _PVR_SetDbmChannel4, disable DBM channel 5\r\n"),
		    DMX_FUNC_NAME);
	/*Disable dbm channel5 default */
	_PVR_SetDbmChannel4(FALSE, FALSE);

	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- Set Playback PID structure size\r\n"), DMX_FUNC_NAME);

	u4Ctrl = DMXCMD_READ32(PVR_REG_CONFIG1);
	u4Ctrl &= 0xffffEf00;	/* Select Group 1 DRAM agent */
	u4Ctrl |= (PVR_DMEM_ENTRY_LEN * 4);	/* Set Playback PID structure sizea */
	u4Ctrl |= (1 << 9);	/* DMA delay ack, debug */
	u4Ctrl |= (1 << 10);	/* Enable multi-PID channel */
	/* Direct map (Record PID data structure index is direct map to PID index) */
	u4Ctrl |= (1 << 8);
	u4Ctrl |= (1 << 12);	/* Select Group 2 DRAM agent(agent_1) */
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_CONFIG1, u4Ctrl);

	_DmxInitFVR();

	_PVR_Unlock();

	smp_mb();

	_PVR_SetBypassMode(DMX_DDI_MM_MOVE_TSIDX, DDI_PACKET_SIZE, TRUE, FALSE);

#if 0
	_PVR_Lock();
	u4Ctrl = DMXCMD_READ32(PVR_REG_DBM_CONTROL);
	/* Set DBM output spacing */
	u4Ctrl = (u4Ctrl & 0xff00ffff) | (8 << 16);
	/* Set DBM max playback TS packets */
	u4Ctrl = (u4Ctrl & 0xffff00ff) | (1 << 8);
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_DBM_CONTROL, u4Ctrl);
	_PVR_Unlock();
#endif

	smp_mb();

	_PVR_SetScrambleSchemeEx(PVR_SCRAMBLE_BOTH_TSFLAG_PESSTART, 0);

	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- _PVR_VCodeInit\r\n"), DMX_FUNC_NAME);
	if (!_PVR_VCodeInit()) {
		PVR_LOG_ERR(TEXT("[DMX][HW] %s -- fail in _PVR_VCodeInit\r\n"), DMX_FUNC_NAME);
		FUNC_EXIT;
		return FALSE;
	}
	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- _PVR_VCodeInit pass\r\n"), DMX_FUNC_NAME);

	smp_mb();

	_PVR_DMEM_CA_Init();

	smp_mb();

	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- _DmxInitHWPath\r\n"), DMX_FUNC_NAME);
	if (!_DmxInitHWPath()) {
		PVR_LOG_ERR(TEXT("[DMX][HW] %s -- fail in _DmxInitHWPath\r\n"), DMX_FUNC_NAME);
		FUNC_EXIT;
		return FALSE;
	}

	_PVR_SetInputMode(PVR_PULL);
	/*_PVR_SetInputMode(PVR_HALF_PUSH);*/

	smp_mb();

	/* Set Interrupt mask for enable all interrupts. */
	DMXCMD_WRITE32(PVR_REG_INT_MASK, 0xFFFFFFFF);

	smp_mb();

	_PVR_Set_DefaultPIDDataStruct();

	smp_mb();

	/* Enable FTI */
	if (!_PVR_EnableFTI(TRUE)) {
		PVR_LOG_ERR(TEXT("[PVR] %s line %d fail in _PVR_EnableFTI(FALSE)!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		FUNC_EXIT;
		return FALSE;
	}

	smp_mb();

	if (!_PVR_DDI_Init()) {
		PVR_LOG_ERR(TEXT("[DMX][HW] %s -- fail in _PVR_DDI_Init\r\n"), DMX_FUNC_NAME);
		FUNC_EXIT;
		return FALSE;
	}

	smp_mb();

	PVR_LOG_DBG(TEXT("[PVR] %s -- _PVR_DDI_SetPort(PVR_DDI_PORT_NOT_FRAMER)\r\n"),
		    DMX_FUNC_NAME);

	_PVR_DDI_SetPort(PVR_DDI_PORT_NOT_FRAMER);

	smp_mb();

	rDDI.eMode = PVR_DDI_MODE_SINGLE;
	rDDI.u4RateN = 0;	/* (N/8M) MB/sec, 0/0: full speed */
	rDDI.u4RateM = 0;

	smp_mb();

	PVR_LOG_DBG(TEXT("[PVR] %s -- _PVR_DDI_Set(PVR_DDI_FLAG_MODE | PVR_DDI_FLAG_RATE)\r\n"),
		    DMX_FUNC_NAME);
	if (!_PVR_DDI_Set((u32) (PVR_DDI_FLAG_MODE | PVR_DDI_FLAG_RATE), &rDDI)) {
		PVR_LOG_ERR(TEXT("[PVR] %s failed in _PVR_DDI_Set(PVR_DDI_MOD_SIGNAL)\r\n"),
			    DMX_FUNC_NAME);
		FUNC_EXIT;
		return FALSE;
	}

	PVR_LOG_DBG(TEXT("[PVR] %s -- _PVR_DDI_SetDMAInt(FALSE, FALSE)\r\n"), DMX_FUNC_NAME);
	smp_mb();

	_PVR_DDI_SetDMAInt(FALSE, FALSE);	/* disable Emtpy Int; disable Alert Int */

	smp_mb();

	if (!_PVR_MPP_Init())
	{
		PVR_LOG_ERR(TEXT("[DMX][HW] %s -- fail in _PVR_MPP_Init\r\n"),
            		DMX_FUNC_NAME);
    	FUNC_EXIT;
    	return FALSE;
	}

	smp_mb();

	// Play init
	rPlay.fgAllocBuf = FALSE;
	rPlay.eMode = PVR_MPP_MODE_SINGLE;
	rPlay.fgContainTimeStamp = FALSE;
	rPlay.fgIgnoreTimeStamp = TRUE;
	rPlay.u2TimeStampFreqDiv = DMX_PVRPLAY_TIMESTAMP_DIV_BASE;

	if (!_PVR_MPP_Set((UINT32)(PVRPLAY_FLAGS_MODE | PVRPLAY_FLAGS_TIMESTAMP), &rPlay))
    	{
        	PVR_LOG_ERR(TEXT("[PVR] %s failed in _PVR_MPP_Set(PVR_DDI_MOD_SIGNAL)\r\n"),
            		DMX_FUNC_NAME);
        	FUNC_EXIT;
        	return FALSE;
    	}

	smp_mb();

	PVR_LOG_DBG(TEXT("[PVR] %s -- _PVR_DDI_SetPort(PVR_DDI_PORT_NOT_FRAMER)\r\n"),
		DMX_FUNC_NAME);

	_PVR_MPP_SetPort(DMX_PVRPLAY_PORT_DBM);

	smp_mb();

	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- set ftup dma threshold to be 0\r\n"),
            DMX_FUNC_NAME);

	_PVR_Set_FTuPDMAThreshold(0);

	FUNC_EXIT;

	return TRUE;
}

static bool _DmxDeInitMultimedia(void)
{
	u32 u4Ctrl;
	u32 i = 0;

	FUNC_ENTRY;

	smp_mb();
	for (i = 0;i < DMX_DEV_CNT; i++){
		if (!_PVR_SetVideoType(i, PVR_VIDEO_UNKNOWN)){
			PVR_LOG_ERR(TEXT("[DMX][HW] %s -- fail in _PVR_SetVideoType(%d)\r\n"),
				DMX_FUNC_NAME, i);
			FUNC_EXIT;
			return FALSE;
		}
	}

	smp_mb();

	if (!_PVR_DDI_DeInit()) {
		PVR_LOG_ERR(TEXT("[DMX][HW] %s -- fail in _PVR_DDI_Init\r\n"), DMX_FUNC_NAME);
		FUNC_EXIT;
		return FALSE;
	}
	smp_mb();

	if (!_PVR_MPP_DeInit())
	{
		PVR_LOG_ERR(TEXT("[DMX][HW] %s -- fail in _PVR_MPP_DeInit\r\n"),
            		DMX_FUNC_NAME);
        	FUNC_EXIT;
        	return FALSE;
	}
	smp_mb();

	if (!_PVR_EnableFTI(FALSE)) {	/* If the hw hasn't been initialized, this function will hung */
		PVR_LOG_ERR(TEXT("[PVR] %s line %d fail in _PVR_EnableFTI(FALSE)!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
	}

	/* Initialize interrupt handler */
	if (!_PVR_DeInitISR()) {
		PVR_LOG_ERR(TEXT("[PVR] %s line %d fail in _PVR_DeInitISR()!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		FUNC_EXIT;
		return FALSE;
	}

	_PVR_Lock();

	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- Clear Play back global region\r\n"), DMX_FUNC_NAME);

	memset_io((void *)(PVR_DMEM_BASE), 0, PVR_DMEM_MAX_LEN);

	/*---------------------------------------------------------*/
	/* Clear Play back global region */
	/*---------------------------------------------------------*/
	memset_io((void *)PLAYBACK_GBL_BASE, 0, PLAYBACK_GBL_SIZE * 4);

	mb();

	smp_mb();

	/*---------------------------------------------------------*/
	/* Clear DMX SRAM and struct */
	/*---------------------------------------------------------*/

	PVR_LOG_DBG(TEXT
		    ("[DMX][HW] %s -- Clear HW Playback PID index table and Playback PID Data Structure\r\n"),
		    DMX_FUNC_NAME);
	/* Clear HW Playback PID index table and Playback PID Data Structure */
	for (i = 0; i < PVR_NUM_PID_INDEX; i++) {
		PID_INDEX_TABLE(i) = 0;
		memset_io((void *)&(PID_S(i)), 0, PVR_DMEM_ENTRY_LEN * 4);
		/* Debug, reset continuity counter to 0xff */
		PID_S_W(i, 3) = 0xffff0000;
	}
	smp_mb();

	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- Reset SW Playback PID data structures\r\n"),
		    DMX_FUNC_NAME);
	/* Set SW Playback PID data structures */
	for (i = 0; i < PVR_NUM_PID_INDEX; i++) {
		PVR_PID_STRUCT_T *prPidStruct;

		prPidStruct = _PVR_GetPidStruct(i);
		mm_memset((void *)prPidStruct, 0, sizeof(PVR_PID_STRUCT_T));
	}
	smp_mb();

	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- Clear HW Section filters\r\n"), DMX_FUNC_NAME);
	/* Clear Section filter */
	/* HW Generic Section Filter has 80 entries */
	memset_io((void *)PVR_SECTION_CONTROL, 0, 4 * PVR_NUM_FILTER_INDEX);
	mb();
	memset_io((void *)PVR_SECTION_POSNEG, 0, 8 * PVR_NUM_FILTER_INDEX);
	mb();
	memset_io((void *)PVR_SECTION_PATTERN, 0, 8 * PVR_NUM_FILTER_INDEX);
	mb();
	memset_io((void *)PVR_SECTION_MASK, 0, 8 * PVR_NUM_FILTER_INDEX);
	mb();

	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- Clear One-Byte Section filters\r\n"), DMX_FUNC_NAME);
	/* One-Byte Section Filter Patten-Mask Region */
	memset_io((void *)PVR_ONEBYTE_FILTER_BASE, 0, 2 * PVR_NUM_ONEBYTE_FILTER_INDEX);
	smp_mb();

	/* CT setting init */
	VERIFY(_PVR_CTInit());
	smp_mb();

	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- Set Playback PID structure Start Offset\r\n"),
		    DMX_FUNC_NAME);
	/* Set Playback PID structure Start Offset */
	u4Ctrl = DMXCMD_READ32(PVR_REG_PID_STRUCT_OFFSET) & 0xFFFF0000;
	DMXCMD_WRITE32(PVR_REG_PID_STRUCT_OFFSET, u4Ctrl);

	_PVR_Unlock();

	_PVR_Lock();

	smp_mb();

	/*-----------------------------------------------------*/
	/*Frmaer Error Table */
	/*-----------------------------------------------------*/
	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- Set Framer 0 & 1 Error Handling Table\r\n"),
		    DMX_FUNC_NAME);
	/*enable error handling */
	_PVR_SetFramerPacketErrorHandling(0, TRUE, 0xF0C0);
	_PVR_SetFramerPacketErrorHandling(1, TRUE, 0xF0C0);
	smp_mb();

	/*-----------------------------------------------------*/
	/* Record default setting */
	/*-----------------------------------------------------*/
	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- Disable Record Path\r\n"), DMX_FUNC_NAME);
	/* Disable record */
	u4Ctrl = DMXCMD_READ32(PVR_REG_CONFIG1) | (1 << 25);
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_CONFIG1, u4Ctrl);

	smp_mb();

	/* Set packet length to 188 bytes */
	u4Ctrl = DMXCMD_READ32(PVR_REG_CONFIG2);
	u4Ctrl = (u4Ctrl & 0xFFFFFF00) | 0x08;	/* maximum burst size */
	u4Ctrl = (u4Ctrl & 0xFFFF00FF) | (188 << 8);	/* packet len = 188 bytes */
	u4Ctrl = (u4Ctrl & 0xFF00FFFF) | (0x47 << 16);	/* TS packet sync byte */
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_CONFIG2, u4Ctrl);

	smp_mb();

	/* Diable All Framer */
	for (i = 0; i < PVR_FRAMER_COUNT; i++) {
		//_PVR_EnableFramer(i, FALSE);
		//_PVR_ResetFramer(i);
		_PVR_SetFramerMode(i, PVR_FRAMER_PARALLEL, TRUE, TRUE);
	}

	smp_mb();

	_PVR_ResetDbmSafely();

	u4Ctrl = DMXCMD_READ32(PVR_REG_CONFIG1);
	u4Ctrl &= 0xffffEf00;	/* Select Group 1 DRAM agent */
	u4Ctrl |= (PVR_DMEM_ENTRY_LEN * 4);	/* Set Playback PID structure sizea */
	u4Ctrl |= (1 << 9);	/* DMA delay ack, debug */
	u4Ctrl |= (1 << 10);	/* Enable multi-PID channel */
	/* Direct map (Record PID data structure index is direct map to PID index) */
	u4Ctrl |= (1 << 8);
	u4Ctrl |= (1 << 12);	/* Select Group 2 DRAM agent(agent_1) */
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_CONFIG1, u4Ctrl);

	_PVR_Unlock();

	smp_mb();

	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- set ts3 to be DDI, and bypass mode\r\n"),
      		DMX_FUNC_NAME);
	_PVR_SetBypassMode(DMX_DDI_MM_MOVE_TSIDX, DDI_PACKET_SIZE, TRUE, FALSE);

	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- set ts2 to be MiniPVR, and bypass mode\r\n"),
      		DMX_FUNC_NAME);
    	_PVR_SetBypassMode(DMX_FVR_MM_MOVE_TSIDX, DDI_PACKET_SIZE, TRUE, FALSE);

#if 0
	_PVR_Lock();
	u4Ctrl = DMXCMD_READ32(PVR_REG_DBM_CONTROL);
	/* Set DBM output spacing */
	u4Ctrl = (u4Ctrl & 0xff00ffff) | (8 << 16);
	/* Set DBM max playback TS packets */
	u4Ctrl = (u4Ctrl & 0xffff00ff) | (1 << 8);
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_DBM_CONTROL, u4Ctrl);
	_PVR_Unlock();
#endif

	smp_mb();

	_PVR_SetScrambleSchemeEx(PVR_SCRAMBLE_BOTH_TSFLAG_PESSTART, 0);

	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- _PVR_VCodeInit\r\n"), DMX_FUNC_NAME);
	if (!_PVR_VCodeInit()) {
		PVR_LOG_ERR(TEXT("[DMX][HW] %s -- fail in _PVR_VCodeInit\r\n"), DMX_FUNC_NAME);
		FUNC_EXIT;
		return FALSE;
	}
	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- _PVR_VCodeInit pass\r\n"), DMX_FUNC_NAME);

	smp_mb();

	_PVR_DMEM_CA_Init();

	smp_mb();

	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- _DmxInitHWPath\r\n"), DMX_FUNC_NAME);
	if (!_DmxInitHWPath()) {
		PVR_LOG_ERR(TEXT("[DMX][HW] %s -- fail in _DmxInitHWPath\r\n"), DMX_FUNC_NAME);
		FUNC_EXIT;
		return FALSE;
	}

	_PVR_SetInputMode(PVR_PULL);
	/*_PVR_SetInputMode(PVR_HALF_PUSH);*/

	smp_mb();

	/* Clear all interrupts */
	for (i = 0; i < PVR_INT_QUEUE_DEPTH; i++) {
		DMXCMD_WRITE32(PVR_REG_DBM_ERROR_STATUS_REG, 1);
		DMXCMD_WRITE32(PVR_REG_DBM_NONERR_STATUS_REG, 1);
		DMXCMD_WRITE32(PVR_REG_DESCRAMBLER_ERROR_STATUS_REG, 1);
		DMXCMD_WRITE32(PVR_REG_DESCRAMBLER_NONERR_STATUS_REG, 1);
		DMXCMD_WRITE32(PVR_REG_STEER_ERROR_STATUS_REG, 1);
		DMXCMD_WRITE32(PVR_REG_STEER_NONERR_STATUS_REG, 1);
		DMXCMD_WRITE32(PVR_REG_FTUP_ERROR_STATUS_REG, 1);
		DMXCMD_WRITE32(PVR_REG_FTUP_NONERR_STATUS_REG1, 1);
		DMXCMD_WRITE32(PVR_REG_PCR_ERROR_STATUS_REG, 1);
		DMXCMD_WRITE32(PVR_REG_PCR_NONERR_STATUS_REG1, 1);
	}

	/* Set Interrupt mask for disable all interrupts. */
	DMXCMD_WRITE32(PVR_REG_INT_MASK, 0x00000000);

	smp_mb();

	FUNC_EXIT;

	return TRUE;
}


/*-----------------------------------------------------------------------------*/
/* Inter-file functions*/
/*-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------*/
/** _PVR_ActivateDbmReset
 *
 *	Disable DBM and pull reset to high
 *
 *	Note:
 *	  This API is used to resolve a hardware bug.
 *	  If framer3 is disabled and enabled frequently, the sync fifo of framer3
 *	  could be overflow (DBM error 32).
 *	  The workaround is we have to soft reset framer3 after disabling it.
 *	  However, the soft reset (0x17204[5]) has a problem on Cobra and Vipher.
 *	  The alternative workaround is we reset DBM before disabling framer3.
 *	  And, we need to keep reset signal until framer3 is disabled completely.
 */
/*-----------------------------------------------------------------------------*/
bool _PVR_ActivateDbmReset(void)
{
	bool fgRet = TRUE;
	u32 u4Reg, u4Counter;

	/* Disable DBM */
	_PVR_Lock();
	u4Reg = DMXCMD_READ32(PVR_REG_DBM_CONTROL);
	u4Reg &= ~(1 << 30);	/* Clear "Enable" bit */
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_DBM_CONTROL, u4Reg);
	_PVR_Unlock();
	smp_mb();

	/* Polling the value of 0x17410 */
	/*              if bit[16..12] != 0x10 */
	/*                      then reset DBM */
	u4Counter = 0;
	while (u4Counter < PVR_MAX_DBM_WAIT_COUNT) {
		u4Reg = DMXCMD_READ32(PVR_REG_DBM_STATUS_REG_3);
		if (((u4Reg >> 12) & 0x1F) == 0x10) {
			DMX_THREAD_DELAY(1);
			u4Counter++;
		} else {
			break;
		}
	}
	smp_mb();

	if (u4Counter == PVR_MAX_DBM_WAIT_COUNT) {
		PVR_LOG_ERR(TEXT
			    ("%s line %d failed to reset DBM safely (still outputing TS packets)!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		fgRet = FALSE;
	}
	smp_mb();

	/* Reset DBM */
	_PVR_Lock();
	u4Reg = DMXCMD_READ32(PVR_REG_DBM_CONTROL);
	u4Reg |= 0x80000000;	/* Set "Reset" bit */
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_DBM_CONTROL, u4Reg);
	_PVR_Unlock();

	return fgRet;
}

/*-----------------------------------------------------------------------------*/
/** _PVR_ReleaseDbmReset
 *
 *	Pull reset to low
 *
 *	Note:
 *	  This API is used to resolve a hardware bug.
 *	  If framer3 is disabled and enabled frequently, the sync fifo of framer3
 *	  could be overflow (DBM error 32).
 *	  The workaround is we have to soft reset framer3 after disabling it.
 *	  However, the soft reset (0x17204[5]) has a problem on Cobra and Vipher.
 *	  The alternative workaround is we reset DBM before disabling framer3.
 *	  And, we need to keep reset signal until framer3 is disabled completely.
 */
/*-----------------------------------------------------------------------------*/
bool _PVR_ReleaseDbmReset(void)
{
	u32 u4Reg;

	_PVR_Lock();
	u4Reg = DMXCMD_READ32(PVR_REG_DBM_CONTROL);
	u4Reg &= 0x7FFFFFFF;	/* Clear "Reset" bit */
	u4Reg |= (1 << 30);	/* Set "Enable" bit */
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_DBM_CONTROL, u4Reg);
	smp_mb();
	_PVR_SetDbmChannel4(FALSE, FALSE);	/* disable dbm channel4 default. */
	_PVR_Unlock();

	return TRUE;
}

/*-----------------------------------------------------------------------------*/
/** _PVR_ResetDbmSafely
 *
 *	Reset DBM safely (at packet boundary) in three steps.
 */
/*-----------------------------------------------------------------------------*/
bool _PVR_ResetDbmSafely(void)
{
	bool fgRet = TRUE;
	u32 u4Reg, u4Counter;

	/* 1/3. Disable DBM */
	u4Reg = DMXCMD_READ32(PVR_REG_DBM_CONTROL);
	u4Reg &= ~(1 << 30);	/* Clear "Enable" bit */
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_DBM_CONTROL, u4Reg);

	smp_mb();
	/* 2/3. Polling the value of 0x20017410 */
	/*              if bit[16..12] != 0x10 */
	/*                      then reset DBM */
	u4Counter = 0;
	while (u4Counter < PVR_MAX_DBM_WAIT_COUNT) {
		u4Reg = DMXCMD_READ32(PVR_REG_DBM_STATUS_REG_3);
		if (((u4Reg >> 12) & 0x1F) == 0x10) {
			DMX_THREAD_DELAY(1);
			u4Counter++;
		} else {
			break;
		}
	}

	smp_mb();
	/* 3/3. Reset DBM which also resets Framer and PID index table. Enable DBM. */
	if (u4Counter == PVR_MAX_DBM_WAIT_COUNT) {
		PVR_LOG_ERR(TEXT
			    ("%s line %d failed to reset DBM safely (still outputing TS packets)!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		fgRet = FALSE;
	}

	u4Reg = DMXCMD_READ32(PVR_REG_DBM_CONTROL);
	u4Reg |= 0x80000000;	/* Set "Reset" bit */
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_DBM_CONTROL, u4Reg);
	smp_mb();
	u4Reg &= 0x7FFFFFFF;	/* Clear "Reset" bit */
	u4Reg |= (1 << 30);	/* Set "Enable" bit */
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_DBM_CONTROL, u4Reg);
	smp_mb();
	_PVR_SetDbmChannel4(FALSE, FALSE);	/* disable dbm channel4 default. */
	return fgRet;
}


/*-----------------------------------------------------------------------------*/
/** _PVR_ClearInterruptQueue
 */
/*-----------------------------------------------------------------------------*/
void _PVR_ClearInterruptQueue(void)
{
	u32 i;

	_PVR_Lock();

	DMXCMD_WRITE32(PVR_REG_DBM_ERROR_STATUS_REG, 1);
	DMXCMD_WRITE32(PVR_REG_DBM_NONERR_STATUS_REG, 1);
	DMXCMD_WRITE32(PVR_REG_DESCRAMBLER_ERROR_STATUS_REG, 1);
	DMXCMD_WRITE32(PVR_REG_DESCRAMBLER_NONERR_STATUS_REG, 1);
	DMXCMD_WRITE32(PVR_REG_STEER_ERROR_STATUS_REG, 1);
	DMXCMD_WRITE32(PVR_REG_STEER_NONERR_STATUS_REG, 1);
	DMXCMD_WRITE32(PVR_REG_FTUP_ERROR_STATUS_REG, 1);
	DMXCMD_WRITE32(PVR_REG_PCR_ERROR_STATUS_REG, 1);
	DMXCMD_WRITE32(PVR_REG_PCR_NONERR_STATUS_REG1, 1);

	smp_mb();
	/* Clear all interrupts */
	for (i = 0; i < PVR_INT_QUEUE_DEPTH; i++)
		DMXCMD_WRITE32(PVR_REG_FTUP_NONERR_STATUS_REG1, 1);

	_PVR_Unlock();
}

VOID _PVR_EnableDmxInterrupt(VOID)
{
    DMXCMD_WRITE32(PVR_REG_INT_MASK, 0xFFFFFFFF);
}

VOID _PVR_DisableDmxInterrupt(VOID)
{
    DMXCMD_WRITE32(PVR_REG_INT_MASK, 0x00000000);
}


/*-----------------------------------------------------------------------------*/
/** _PVR_ResetFTuP
 *
 *	This function resets the uP, and sets its "boot address" to 0.
 */
/*-----------------------------------------------------------------------------*/
void _PVR_ResetFTuP(void)
{
	/* Reset uP and its booting address. */
	u32 u4Reg, u4WaitCnt;

	_PVR_Lock();
	_rDmxPvrInfo.fgPvrMicroProcStopped = TRUE;
	_PVR_Unlock();
	smp_mb();
	u4Reg = DMXCMD_READ32(PVR_REG_FTUP_CONTROL);
	u4Reg |= 0x81000000;	/* Set FTuP halt Request(Bit31), and Disable FTUP DMEM/IMEM for Nand ABIST use */
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_FTUP_CONTROL, u4Reg);
	smp_mb();
	
	u4WaitCnt = 0;
	while (u4WaitCnt < PVR_MAX_FTT_WAIT_COUNT) {
		if ((DMXCMD_READ32(PVR_REG_FTUP_CONTROL) & (1 << 30)) != 0) {
			break;
		}
		u4WaitCnt++;
		DMX_THREAD_DELAY(1);
	}
	smp_mb();

	u4Reg = DMXCMD_READ32(PVR_REG_FTUP_CONTROL);
	u4Reg &= 0xFeFF0000;	/* Enable "FTuP Stalling"; Clear "Boot address" */
	u4Reg |= 0x1;		/* Change "Boot address" to 1. */
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_FTUP_CONTROL, u4Reg);	/* Write it twice! */
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_FTUP_CONTROL, u4Reg);	/* Write it twice! */

	smp_mb();
	_PVR_ClearInterruptQueue();

	smp_mb();
	/* Enable uP */
	u4Reg = DMXCMD_READ32(PVR_REG_FTUP_CONTROL);
	u4Reg &= 0x7FFF0000;	/* Change "Boot address" to 0, and Clear FTuP halt Request(Bit31) */
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_FTUP_CONTROL, u4Reg);

	smp_mb();
	_PVR_Lock();
	_rDmxPvrInfo.fgPvrMicroProcStopped = FALSE;
	_PVR_Unlock();
}


/*-----------------------------------------------------------------------------*/
/** _PVR_EnableFTI
 *	Enable or disable FTI
 *
 *	@param	fgEnable		TRUE: enable, FALSE: disable
 *
 *	@retval void
 */
/*-----------------------------------------------------------------------------*/
bool _PVR_EnableFTI(bool fgEnable)
{
	u32 u4Ctrl, i;
	u32 u4WaitCnt = 0;

	FUNC_ENTRY;

	u4Ctrl = DMXCMD_READ32(PVR_REG_CONTROL);
	if (fgEnable) {
		PVR_LOG_DBG(TEXT("[DMX][HW] %s (TRUE)\r\n"), DMX_FUNC_NAME);
		/* Enable FTI */
		u4Ctrl &= 0x3fffffff;	/* Set FTI Global Control to be "Run", i.e. Bit[31:30] = 00 */
		smp_mb();
		DMXCMD_WRITE32(PVR_REG_CONTROL, u4Ctrl);

		smp_mb();

		/* Enable uP */
		u4Ctrl = DMXCMD_READ32(PVR_REG_FTUP_CONTROL);
		u4Ctrl &= 0x7fffffff;
		smp_mb();
		DMXCMD_WRITE32(PVR_REG_FTUP_CONTROL, u4Ctrl);

		smp_mb();

		_PVR_Lock();
		_rDmxPvrInfo.fgPvrMicroProcStopped = FALSE;
		_PVR_Unlock();
	} else {
		PVR_LOG_DBG(TEXT("[DMX][HW] %s (FALSE)\r\n"), DMX_FUNC_NAME);
		/* Halt uP */
		/*u4Ctrl = DMXCMD_READ32(PVR_REG_FTUP_CONTROL); */
		/*u4Ctrl |= 0x80000000; */
		/*DMXCMD_WRITE32(PVR_REG_FTUP_CONTROL, u4Ctrl); */
		/* MT5391, turn on "bit[24]: disble uP stall" */
		u4Ctrl = DMXCMD_READ32(PVR_REG_FTUP_CONTROL);
		u4Ctrl |= 0x81000000;	/* MT5391 */
		smp_mb();
		DMXCMD_WRITE32(PVR_REG_FTUP_CONTROL, u4Ctrl);
		smp_mb();

		u4WaitCnt = 0;
		while (u4WaitCnt < PVR_MAX_FTT_WAIT_COUNT) {
			if ((DMXCMD_READ32(PVR_REG_FTUP_CONTROL) & (1 << 30)) != 0)
				break;

			u4WaitCnt++;
			DMX_THREAD_DELAY(1);
		}
		smp_mb();

		if (u4WaitCnt >= PVR_MAX_FTT_WAIT_COUNT) {
			PVR_LOG_ERR(TEXT("[PVR] %s Failed to Stop Ftup Safely!\r\n"),
				    DMX_FUNC_NAME);
			return FALSE;
		}
		smp_mb();

		u4Ctrl = DMXCMD_READ32(PVR_REG_FTUP_CONTROL);
		u4Ctrl &= 0xfeffffff;	/* Enable "FTuP Stalling" */
		DMXCMD_WRITE32(PVR_REG_FTUP_CONTROL, u4Ctrl);

		smp_mb();

		/* Clear all interrupts */
		for (i = 0; i < PVR_INT_QUEUE_DEPTH; i++) {
			DMXCMD_WRITE32(PVR_REG_DBM_ERROR_STATUS_REG, 1);
			DMXCMD_WRITE32(PVR_REG_DBM_NONERR_STATUS_REG, 1);
			DMXCMD_WRITE32(PVR_REG_DESCRAMBLER_ERROR_STATUS_REG, 1);
			DMXCMD_WRITE32(PVR_REG_DESCRAMBLER_NONERR_STATUS_REG, 1);
			DMXCMD_WRITE32(PVR_REG_STEER_ERROR_STATUS_REG, 1);
			DMXCMD_WRITE32(PVR_REG_STEER_NONERR_STATUS_REG, 1);
			DMXCMD_WRITE32(PVR_REG_FTUP_ERROR_STATUS_REG, 1);
			DMXCMD_WRITE32(PVR_REG_FTUP_NONERR_STATUS_REG1, 1);
			DMXCMD_WRITE32(PVR_REG_PCR_ERROR_STATUS_REG, 1);
			DMXCMD_WRITE32(PVR_REG_PCR_NONERR_STATUS_REG1, 1);
		}
		smp_mb();

		/* Halt FTI */
		_PVR_Lock();
		_rDmxPvrInfo.fgPvrMicroProcStopped = TRUE;
		_PVR_Unlock();

		smp_mb();
		/* Set FTI Global Control to be "Halt", i.e. Bit[31:30] = 01 */
		u4Ctrl = ((u4Ctrl & 0x3fffffff) | 0x40000000);
		smp_mb();
		DMXCMD_WRITE32(PVR_REG_CONTROL, u4Ctrl);

		smp_mb();
		u4WaitCnt = 0;
		while (u4WaitCnt < PVR_MAX_FTT_WAIT_COUNT) {
			/* Check if FTI is halted */
			if (0x1 != ((DMXCMD_READ32(PVR_REG_CONTROL) >> 28) & 0x3)) {
				DMX_THREAD_DELAY(1);
				u4WaitCnt++;
			} else {
				break;
			}
		}
	}

	FUNC_EXIT;

	return TRUE;
}


/*-----------------------------------------------------------------------------*/
/** _PVR_Reset
 *	Reset FTI
 *
 *	@retval TRUE			Succeed
 *	@retval FALSE			Fail
 */
/*-----------------------------------------------------------------------------*/
bool _PVR_Reset(void)
{
	u32 i;
	u32 u4WaitCnt = 0;

	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- Enter\r\n"), DMX_FUNC_NAME);

	_PVR_Lock();

	_DmxIsFTIRunning();

	smp_mb();

	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- Clear all interrupts \r\n"), DMX_FUNC_NAME);

	/* Clear all interrupts */
	for (i = 0; i < PVR_INT_QUEUE_DEPTH; i++) {
		DMXCMD_WRITE32(PVR_REG_DBM_ERROR_STATUS_REG, 1);
		DMXCMD_WRITE32(PVR_REG_DBM_NONERR_STATUS_REG, 1);
		DMXCMD_WRITE32(PVR_REG_DESCRAMBLER_ERROR_STATUS_REG, 1);
		DMXCMD_WRITE32(PVR_REG_DESCRAMBLER_NONERR_STATUS_REG, 1);
		DMXCMD_WRITE32(PVR_REG_STEER_ERROR_STATUS_REG, 1);
		DMXCMD_WRITE32(PVR_REG_STEER_NONERR_STATUS_REG, 1);
		DMXCMD_WRITE32(PVR_REG_FTUP_ERROR_STATUS_REG, 1);
		DMXCMD_WRITE32(PVR_REG_FTUP_NONERR_STATUS_REG1, 1);
		DMXCMD_WRITE32(PVR_REG_PCR_ERROR_STATUS_REG, 1);
		DMXCMD_WRITE32(PVR_REG_PCR_NONERR_STATUS_REG1, 1);
	}

	smp_mb();

	_DmxIsFTIRunning();

	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- Reset FTI \r\n"), DMX_FUNC_NAME);

	smp_mb();

	_rDmxPvrInfo.fgPvrMicroProcStopped = TRUE;

	smp_mb();

	/* Set FTI Global Control to be "Reset", i.e. Bit[31:30] = 10, Bit[27:26] = 11 */
	DMXCMD_WRITE32(PVR_REG_CONTROL, 0x8f000000);

	smp_mb();

	_DmxIsFTIRunning();

	_PVR_Unlock();

	smp_mb();

	/* Wait for ready */
	u4WaitCnt = 0;
	while (u4WaitCnt < PVR_MAX_FTT_WAIT_COUNT) {
		/* Check if FTI is halted */
		if (0x1 != ((DMXCMD_READ32(PVR_REG_CONTROL) >> 28) & 0x3)) {
			DMX_THREAD_DELAY(1);
			u4WaitCnt++;
		} else {
			break;
		}
	}
	smp_mb();

	if (u4WaitCnt >= PVR_MAX_FTT_WAIT_COUNT) {
		PVR_LOG_ERR(TEXT("[PVR] %s line %d Failed to Stop FTI Safely!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		return FALSE;
	}

	smp_mb();

	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- FTI is halted\r\n"), DMX_FUNC_NAME);

	smp_mb();

	_PVR_SetDbm_InputSource(DMX_DDI_MM_MOVE_TSIDX, PVR_DBM_INPUT_DDI);

	smp_mb();

	PVR_LOG_TRACE(TEXT("[DMX][HW] %s success\r\n"), DMX_FUNC_NAME);

	return TRUE;
}

/*-----------------------------------------------------------------------------*/
/** _PVR_ResetFramer
 */
/*-----------------------------------------------------------------------------*/
void _PVR_ResetFramer(u8 u1Framer)
{
	u32 u4Ctrl;
	u8 i;
	bool fgEnable = FALSE;

	PVR_LOG_DBG(TEXT("[DMX][HW] %s line %d enter (%d)\r\n"), DMX_FUNC_NAME, DMX_LINE_NO,
		    u1Framer);

	if (u1Framer == PVR_FRAMER_TOTAL_INDEX) {
		PVR_LOG_ERR(TEXT("[DMX][HW] _PVR_ResetFramer exit for (%d) > 2\r\n"), u1Framer);
		return;
	}

	/* Reset framer - bit12 for framer0 reset, bi13 for framer 1 reset */
	if (u1Framer == 0) {
		fgEnable = (0 != (DMXCMD_READ32(PVR_REG_FRAMER_CONTROL) & (0x01)));

		smp_mb();

		/* disable framer 0 */
		PVR_LOG_DBG(TEXT("[DMX][HW] _PVR_ResetFramer -- disable framer %d\r\n"), u1Framer);
		u4Ctrl = DMXCMD_READ32(PVR_REG_FRAMER_CONTROL) & (~0x1);
		DMXCMD_WRITE32(PVR_REG_FRAMER_CONTROL, u4Ctrl);

		PVR_LOG_DBG(TEXT
			    ("[DMX][HW] _PVR_ResetFramer -- wait for framer %d state to idle\r\n"),
			    u1Framer);
		smp_mb();

		PVR_LOG_DBG(TEXT
			    ("[DMX][HW] _PVR_ResetFramer -- Reg[PVR_REG_FRAMER0_FSM]: 0x%x\r\n"),
			    u1Framer, DMXCMD_READ32(PVR_REG_FRAMER0_FSM));

		/* wait for framer 0 state to idle */
		for (i = 0; i < 100; i++) {
			u4Ctrl = DMXCMD_READ32(PVR_REG_FRAMER0_FSM);
			if ((u4Ctrl & 0xFFFFF) == 0x10101) {
				/* in idle state */
				break;
			}
			DMX_THREAD_DELAY(10);
		}
		if (i >= 100) {
			PVR_LOG_ERR(TEXT
				    ("[PVR] %s line %d fail in wait for framer 0 state to idle\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);
			return;
		}
		smp_mb();

		PVR_LOG_DBG(TEXT
			     ("[DMX][HW] _PVR_ResetFramer -- whether Check TS%d's current status")
			     TEXT(" is currently not receiving any data from framer\r\n"),
			    u1Framer);
		PVR_LOG_DBG(TEXT("[DMX][HW] _PVR_ResetFramer -- Reg[PVR_REG_FRAMER0_FSM]: 0x%x, ")
			     TEXT("Reg[PVR_REG_DBM_STATUS_REG_3]: 0x%x\r\n"),
			    u1Framer, DMXCMD_READ32(PVR_REG_FRAMER0_FSM),
			    DMXCMD_READ32(PVR_REG_DBM_STATUS_REG_3));
		smp_mb();

		for (i = 0; i < 100; i++) {
			u4Ctrl = DMXCMD_READ32(PVR_REG_DBM_STATUS_REG_3);
			/* whether Check TS0's current status is currently not receiving any data from framer */
			if ((u4Ctrl & 0x3) == 0x0) {
				/* in idle state */
				break;
			}
			DMX_THREAD_DELAY(10);
		}
		smp_mb();
		if (i >= 100) {
			PVR_LOG_ERR(TEXT("[PVR] %s line %d fail in wait for TS0's current status ")
				     TEXT("to be not receiving any data from framer\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);
			return;
		}
		smp_mb();

		PVR_LOG_DBG(TEXT("[DMX][HW] _PVR_ResetFramer -- Set Framer %d reset signal\r\n"),
			    u1Framer);

		u4Ctrl = DMXCMD_READ32(PVR_REG_FRAMER_CONTROL) & (~0x1000);	/* clear Framer 0 reset signal */
		u4Ctrl |= 0x1000;	/* Set Framer 0 reset signal */
		DMXCMD_WRITE32(PVR_REG_FRAMER_CONTROL, u4Ctrl);

		smp_mb();
		PVR_LOG_DBG(TEXT("[DMX][HW] _PVR_ResetFramer -- Clear Framer %d reset signal\r\n"),
			    u1Framer);
		DMX_THREAD_DELAY(1);
		u4Ctrl &= (~0x1000);	/* clear Framer 0 reset signal */
		DMXCMD_WRITE32(PVR_REG_FRAMER_CONTROL, u4Ctrl);
		smp_mb();

		if (fgEnable) {
			PVR_LOG_DBG(TEXT("[DMX][HW] _PVR_ResetFramer -- enable Framer %d\r\n"),
				    u1Framer);
			/* enable framer 0 */
			u4Ctrl = DMXCMD_READ32(PVR_REG_FRAMER_CONTROL) | (0x1);
			DMXCMD_WRITE32(PVR_REG_FRAMER_CONTROL, u4Ctrl);
		}
	} else if (u1Framer == 1) {
		fgEnable = (0 != (DMXCMD_READ32(PVR_REG_FRAMER_CONTROL) & (0x10)));
		smp_mb();

		/* disable framer */
		PVR_LOG_DBG(TEXT("[DMX][HW] _PVR_ResetFramer -- disable framer %d\r\n"), u1Framer);
		u4Ctrl = DMXCMD_READ32(PVR_REG_FRAMER_CONTROL) & (~0x10);
		smp_mb();
		DMXCMD_WRITE32(PVR_REG_FRAMER_CONTROL, u4Ctrl);

		smp_mb();

		/* wait for framer state to idle */
		PVR_LOG_DBG(TEXT
			    ("[DMX][HW] _PVR_ResetFramer -- wait for framer %d state to idle\r\n"),
			    u1Framer);
		PVR_LOG_DBG(TEXT
			    ("[DMX][HW] _PVR_ResetFramer -- Reg[PVR_REG_FRAMER1_FSM]: 0x%x\r\n"),
			    u1Framer, DMXCMD_READ32(PVR_REG_FRAMER1_FSM));

		smp_mb();

		for (i = 0; i < 100; i++) {
			u4Ctrl = DMXCMD_READ32(PVR_REG_FRAMER1_FSM);
			if ((u4Ctrl & 0xFFFFF) == 0x10101) {
				/* in idle state */
				break;
			}
			DMX_THREAD_DELAY(10);
		}
		smp_mb();
		if (i >= 100) {
			PVR_LOG_ERR(TEXT
				    ("[PVR] %s line %d fail in wait for framer 1 state to idle\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);
			return;
		}
		smp_mb();

		PVR_LOG_DBG(TEXT
			     ("[DMX][HW] _PVR_ResetFramer -- whether Check TS%d's current status")
			     TEXT(" is currently not receiving any data from framer\r\n"),
			    u1Framer);
		PVR_LOG_DBG(TEXT("[DMX][HW] _PVR_ResetFramer -- Reg[PVR_REG_FRAMER1_FSM]: 0x%x,")
			     TEXT(" Reg[PVR_REG_DBM_STATUS_REG_3]: 0x%x\r\n"),
			    u1Framer, DMXCMD_READ32(PVR_REG_FRAMER1_FSM),
			    DMXCMD_READ32(PVR_REG_DBM_STATUS_REG_3));
		smp_mb();

		for (i = 0; i < 100; i++) {
			u4Ctrl = DMXCMD_READ32(PVR_REG_DBM_STATUS_REG_3);
			if ((u4Ctrl & 0xC) == 0x0) {
				/* in idle state */
				break;
			}
			DMX_THREAD_DELAY(10);
		}
		smp_mb();
		if (i >= 100) {
			PVR_LOG_ERR(TEXT("[PVR] %s line %d fail in wait for TS1's current status")
				     TEXT(" to be not receiving any data from framer\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);
			return;
		}
		smp_mb();

		PVR_LOG_DBG(TEXT("[DMX][HW] _PVR_ResetFramer -- Set Framer %d reset signal\r\n"),
			    u1Framer);

		u4Ctrl = DMXCMD_READ32(PVR_REG_FRAMER_CONTROL) & (~0x2000);
		u4Ctrl |= 0x2000;
		DMXCMD_WRITE32(PVR_REG_FRAMER_CONTROL, u4Ctrl);
		smp_mb();
		DMX_THREAD_DELAY(1);
		PVR_LOG_DBG(TEXT("[DMX][HW] _PVR_ResetFramer -- Clear Framer %d reset signal\r\n"),
			    u1Framer);
		u4Ctrl &= (~0x2000);
		smp_mb();
		DMXCMD_WRITE32(PVR_REG_FRAMER_CONTROL, u4Ctrl);
		smp_mb();

		if (fgEnable) {
			PVR_LOG_DBG(TEXT("[DMX][HW] _PVR_ResetFramer -- enable Framer %d\r\n"),
				    u1Framer);
			/* enable framer */
			u4Ctrl = DMXCMD_READ32(PVR_REG_FRAMER_CONTROL) | (0x10);
			DMXCMD_WRITE32(PVR_REG_FRAMER_CONTROL, u4Ctrl);
		}
	} else {
		DMX_ASSERT(FALSE);
	}

	PVR_LOG_DBG(TEXT("[DMX][HW] Reset framer (%d)\r\n"), u1Framer);
}

/*-----------------------------------------------------------------------------*/
/** _PVR_SetFramerEnabled
 */
/*-----------------------------------------------------------------------------*/
void _PVR_SetFramerEnabled(u8 u1Framer, bool fgEnable)
{
	u32 u4Ctrl;
	u8 i;

	switch (u1Framer) {
	case 0:
		if (fgEnable) {
			u4Ctrl = DMXCMD_READ32(PVR_REG_FRAMER_CONTROL) | (0x1);
			smp_mb();
			DMXCMD_WRITE32(PVR_REG_FRAMER_CONTROL, u4Ctrl);
		} else {
			u4Ctrl = DMXCMD_READ32(PVR_REG_FRAMER_CONTROL) & (~0x1);
			smp_mb();
			DMXCMD_WRITE32(PVR_REG_FRAMER_CONTROL, u4Ctrl);

			smp_mb();
			/* wait for framer state to idle */
			for (i = 0; i < 100; i++) {
				u4Ctrl = DMXCMD_READ32(PVR_REG_FRAMER0_FSM);
				if ((u4Ctrl & 0xFFFFF) == 0x10101) {
					/* in idle state */
					break;
				}
				DMX_THREAD_DELAY(10);
			}
			smp_mb();
			if (i >= 100) {
				PVR_LOG_ERR(TEXT
					    ("[PVR] %s line %d fail in wait for framer 0 state to idle\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO);
				return;
			}

			smp_mb();
			for (i = 0; i < 100; i++) {
				u4Ctrl = DMXCMD_READ32(PVR_REG_DBM_STATUS_REG_3);
				if ((u4Ctrl & 0x3) == 0x0) {
					/* in idle state */
					break;
				}
				DMX_THREAD_DELAY(10);
			}
			smp_mb();
			if (i >= 100) {
				PVR_LOG_ERR(TEXT
					     ("[PVR] %s line %d fail in wait for TS0's current status")
					     TEXT(" to be not receiving any data from framer\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO);
				return;
			}
		}
		break;
	case 1:
		if (fgEnable) {
			u4Ctrl = DMXCMD_READ32(PVR_REG_FRAMER_CONTROL) | (0x10);
			smp_mb();
			DMXCMD_WRITE32(PVR_REG_FRAMER_CONTROL, u4Ctrl);
		} else {
			u4Ctrl = DMXCMD_READ32(PVR_REG_FRAMER_CONTROL) & (~0x10);
			smp_mb();
			DMXCMD_WRITE32(PVR_REG_FRAMER_CONTROL, u4Ctrl);

			smp_mb();
			/* wait for framer state to idle */
			for (i = 0; i < 100; i++) {
				u4Ctrl = DMXCMD_READ32(PVR_REG_FRAMER1_FSM);
				if ((u4Ctrl & 0xFFFFF) == 0x10101) {
					/* in idle state */
					break;
				}
				DMX_THREAD_DELAY(10);
			}
			smp_mb();
			if (i >= 100) {
				PVR_LOG_ERR(TEXT
					    ("[PVR] %s line %d fail in wait for framer 1 state to idle\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO);
				return;
			}

			smp_mb();
			for (i = 0; i < 100; i++) {
				u4Ctrl = DMXCMD_READ32(PVR_REG_DBM_STATUS_REG_3);
				if ((u4Ctrl & 0xC) == 0x0) {
					/* in idle state */
					break;
				}
				DMX_THREAD_DELAY(10);
			}
			smp_mb();
			if (i >= 100) {
				PVR_LOG_ERR(TEXT
					     ("[PVR] %s line %d fail in wait for TS1's current status")
					     TEXT(" to be not receiving any data from framer\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO);
				return;
			}
		}
		break;
	default:
		PVR_LOG_ERR(TEXT("%s line %d fail for unsupported framer %u!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, u1Framer);
		break;
	}
}

/*-----------------------------------------------------------------------------*/
/** _PVR_GetFramerIndex
 *	Get DTV framer
 */
/*-----------------------------------------------------------------------------*/
u8 _PVR_GetFramerIndex(void)
{
	/* Fixme, 0 use as default first */
	return 0;
}


/*-----------------------------------------------------------------------------*/
/** _PVR_Start
 *	Start demux
 *
 *	@retval TRUE			Succeed
 *	@retval FALSE			Fail
 */
/*-----------------------------------------------------------------------------*/
bool _PVR_Start(void)
{
	return TRUE;
}


/*-----------------------------------------------------------------------------*/
/** _PVR_Stop
 *	Stop demux
 *
 *	@retval TRUE			Succeed
 *	@retval FALSE			Fail
 */
/*-----------------------------------------------------------------------------*/
bool _PVR_Stop(void)
{
	u32 u4Ctrl, i;

	_PVR_Lock();

	/* Disable framers */
	u4Ctrl = DMXCMD_READ32(PVR_REG_FRAMER_CONTROL);
	u4Ctrl &= ~((1 << 0) | (1 << 4));	/* disable framer 0 and 1 */
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_FRAMER_CONTROL, u4Ctrl);

	smp_mb();

	/* Clear all interrupts */
	for (i = 0; i < PVR_INT_QUEUE_DEPTH; i++) {
		DMXCMD_WRITE32(PVR_REG_DBM_ERROR_STATUS_REG, 1);
		DMXCMD_WRITE32(PVR_REG_DBM_NONERR_STATUS_REG, 1);
		DMXCMD_WRITE32(PVR_REG_DESCRAMBLER_ERROR_STATUS_REG, 1);
		DMXCMD_WRITE32(PVR_REG_DESCRAMBLER_NONERR_STATUS_REG, 1);
		DMXCMD_WRITE32(PVR_REG_STEER_ERROR_STATUS_REG, 1);
		DMXCMD_WRITE32(PVR_REG_STEER_NONERR_STATUS_REG, 1);
		DMXCMD_WRITE32(PVR_REG_FTUP_ERROR_STATUS_REG, 1);
		DMXCMD_WRITE32(PVR_REG_FTUP_NONERR_STATUS_REG1, 1);
		DMXCMD_WRITE32(PVR_REG_PCR_ERROR_STATUS_REG, 1);
		DMXCMD_WRITE32(PVR_REG_PCR_NONERR_STATUS_REG1, 1);
	}

	_PVR_Unlock();

	return TRUE;
}

/*-----------------------------------------------------------------------------*/
/** _PVR_InitISR
 *	Initialize interrupt handler
 *
 *	@retval void
 */
/*-----------------------------------------------------------------------------*/
bool _PVR_InitISR(void)
{
	x_os_isr_fct pfnOldIsr = NULL;

	/* Register ISR */
#if DMX_SUPPORT_DEVICE_TREE
	if (!_rDmxPvrInfo.fgPvrISRInited) {
		PVR_LOG_TRACE(TEXT("[PVR] %s -- enable dmx ftup interrupt!\r\n"),
			      DMX_FUNC_NAME);
	  /*enable_irq(g_dmxdevinfo->ftup_irq); */
		UNUSED(pfnOldIsr);
		smp_mb();
		_rDmxPvrInfo.fgPvrISRInited = TRUE;
	}
#endif				/* DMX_SUPPORT_DEVICE_TREE */
	/* Enable all demux interrupts */
	smp_mb();

	DMXCMD_WRITE32(PVR_REG_INT_MASK, 0xFFFFFFFF);

	PVR_LOG_DBG(TEXT("[PVR] %s enable all interrupt -- PVR_REG_INT_MASK: 0x%x!\r\n"),
		    DMX_FUNC_NAME, 0xFFFFFFFF);

	PVR_LOG_DBG(TEXT("[PVR] %s -- Enable Demuxer HW Interrupt.\r\n"), DMX_FUNC_NAME);

	return TRUE;
}

bool _PVR_DeInitISR(void)
{
	x_os_isr_fct pfnOldIsr = NULL;

	/* Unregister ISR */
	if (_rDmxPvrInfo.fgPvrISRInited) {
#if DMX_SUPPORT_DEVICE_TREE
		PVR_LOG_TRACE(TEXT("[PVR] %s -- disable dmx ftup interrupt!\r\n"),
			      DMX_FUNC_NAME);
		/* disable_irq(g_dmxdevinfo->ftup_irq); */
#endif				/* DMX_SUPPORT_DEVICE_TREE */

		UNUSED(pfnOldIsr);

		smp_mb();
		_rDmxPvrInfo.fgPvrISRInited = FALSE;
	}

	PVR_LOG_INFO(TEXT("[PVR] %s -- Disable Demuxer HW Interrupt.\r\n"), DMX_FUNC_NAME);

	return TRUE;
}

/*-----------------------------------------------------------------------------*/
/** _PVR_InitAllocMem
 *	Allocate memory for global structures, _arPidStruct, _prPvrFilterStruct
 *
 *	@retval void
 */
/*-----------------------------------------------------------------------------*/
MRESULT _PVR_InitAllocMem(void)
{
	MRESULT mrRet = RET_DMX_UNEXPECT;

	FUNC_ENTRY;

	if (NULL == _rDmxPvrInfo.prPvrPidStructs)
		DMX_NewMemory((sizeof(PVR_PID_STRUCT_T) * PVR_NUM_PID_INDEX),
			      _rDmxPvrInfo.prPvrPidStructs);

	smp_mb();
	if (NULL == _rDmxPvrInfo.prPvrPidStructs) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail in alloc PVR Pid Data Structures (No Mem)\r\n"),
			    DMX_FUNC_NAME);
		mrRet = RET_DMX_NO_MEM;
		goto errallocexit;
	}

	smp_mb();

	mm_memset((void *) (_rDmxPvrInfo.prPvrPidStructs), 0,
		   sizeof(PVR_PID_STRUCT_T) * PVR_NUM_PID_INDEX);

	if (NULL == _rDmxPvrInfo.prPicInfos)
		DMX_NewMemory((sizeof(DMX_PIC_INFO_T) * DMX_MAX_VID_STARTCODE_CNT),
			      _rDmxPvrInfo.prPicInfos);

	smp_mb();
	if (NULL == _rDmxPvrInfo.prPicInfos) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail in alloc PVR Picture Infos array (No Mem)\r\n"),
			    DMX_FUNC_NAME);
		mrRet = RET_DMX_NO_MEM;
		goto errallocexit;
	}
	smp_mb();

	mm_memset((void *) (_rDmxPvrInfo.prPicInfos), 0,
		   sizeof(DMX_PIC_INFO_T) * DMX_MAX_VID_STARTCODE_CNT);

	_PVR_Lock();
	g_eDmxHwVideoType[DDI_PVR_DMA_PATH_ID] = PVR_VIDEO_UNKNOWN;
	g_eDmxHwVideoType[MINI_PVR_DMA_PATH_ID] = PVR_VIDEO_UNKNOWN;
	_PVR_Unlock();

	FUNC_EXIT;

	MM_RETURN(RET_DMX_OK);

errallocexit:
	if (NULL != _rDmxPvrInfo.prPvrPidStructs) {
		DMX_FreeMemory(_rDmxPvrInfo.prPvrPidStructs);
		_rDmxPvrInfo.prPvrPidStructs = NULL;
	}
	if (NULL != _rDmxPvrInfo.prPicInfos) {
		DMX_FreeMemory(_rDmxPvrInfo.prPicInfos);
		_rDmxPvrInfo.prPicInfos = NULL;
	}
#ifdef ENABLE_SECTION_FILTER
	if (NULL != _rDmxPvrInfo.prFilterStruct) {
		DMX_FreeMemory(_rDmxPvrInfo.prFilterStruct);
		_rDmxPvrInfo.prFilterStruct = NULL;
	}
#endif

	MM_RETURN(mrRet);
}

void _PVR_UninitAllocMem(void)
{
	void *pvFree = NULL;

	_PVR_Lock();
	if (NULL != _rDmxPvrInfo.prPvrPidStructs) {
		pvFree = _rDmxPvrInfo.prPvrPidStructs;
		_rDmxPvrInfo.prPvrPidStructs = NULL;
		_PVR_Unlock();
		DMX_FreeMemory(pvFree);
		_PVR_Lock();
	}

	if (NULL != _rDmxPvrInfo.prPicInfos) {
		pvFree = _rDmxPvrInfo.prPicInfos;
		_rDmxPvrInfo.prPicInfos = NULL;
		_PVR_Unlock();
		DMX_FreeMemory(pvFree);
		_PVR_Lock();
	}
#ifdef ENABLE_SECTION_FILTER
	if (NULL != _rDmxPvrInfo.prFilterStruct) {
		pvFree = _rDmxPvrInfo.prFilterStruct;
		_rDmxPvrInfo.prFilterStruct = NULL;
		_PVR_Unlock();
		DMX_FreeMemory(pvFree);
		_PVR_Lock();
	}
#endif

	_PVR_Unlock();
}

MRESULT _PVR_SetInputType(PVR_INPUT_TYPE_T eInputType)
{
	FUNC_ENTRY;

	PVR_LOG_DBG(TEXT("%s line %d  -- eInputType: %d, OldInputType: %d\r\n"),
		    DMX_FUNC_NAME, DMX_LINE_NO, eInputType, _rDmxPvrInfo.ePvrInputType);

	_PVR_Lock();

	if (_rDmxPvrInfo.ePvrInputType == eInputType) {
		_PVR_Unlock();
		MM_RETURN(RET_DMX_OK);
	}

	_PVR_Unlock();

	switch (eInputType) {
	case PVR_IN_PLAYBACK_MM:
		if (!_DmxInitMultimedia()) {
			PVR_LOG_ERR(TEXT
				    ("%s line %d fail in _DmxInitMultimedia(),	eInputType: %d\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, eInputType);
			FUNC_EXIT;
			return FALSE;
		}
		break;
	case PVR_IN_NONE:
		if (PVR_IN_PLAYBACK_MM == _rDmxPvrInfo.ePvrInputType) {
			if (!_DmxDeInitMultimedia()) {
				PVR_LOG_ERR(TEXT
					    ("%s line %d fail in _DmxDeInitMultimedia(),  eInputType: %d\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, eInputType);
				FUNC_EXIT;
				return FALSE;
			}
		}
		break;
	default:
		PVR_LOG_ERR(TEXT("%s line %d fail for unsupport InputType: %d\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, eInputType);
		FUNC_EXIT;
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	smp_mb();

	_PVR_Lock();
	_rDmxPvrInfo.ePvrInputType = eInputType;
	_PVR_Unlock();

	FUNC_EXIT;

	MM_RETURN(RET_DMX_OK);
}

/*-----------------------------------------------------------------------------*/
/** _PVR_GetInputType
 */
/*-----------------------------------------------------------------------------*/
PVR_INPUT_TYPE_T _PVR_GetInputType(void)
{
	PVR_INPUT_TYPE_T eInputType;

	_PVR_Lock();
	eInputType = _rDmxPvrInfo.ePvrInputType;
	_PVR_Unlock();

	return eInputType;
}

/*-----------------------------------------------------------------------------*/
/** _PVR_EnablePower
 */
/*-----------------------------------------------------------------------------*/
void _PVR_EnablePower(bool fgEnable, PVR_INPUT_TYPE_T eInputType)
{
	_PVR_CkgenInit(fgEnable, eInputType);
}

/*-----------------------------------------------------------------------------*/
/** _PVR_Init
 *	Initialize demux - can run multiple times
 *
 *	@retval TRUE			Succeed
 *	@retval FALSE			Fail
 */
/*-----------------------------------------------------------------------------*/
MRESULT _PVR_Init(PVR_INPUT_TYPE_T eInputType)
{
	MRESULT mrRet = RET_DMX_OK;
	BOOL fgNeedPowerOn = FALSE;
	u32 i = 0;

	FUNC_ENTRY;

	mm_memset(&_rDmxPvrInfo, 0, sizeof(PVR_HW_GLOBAL_INFO_T));

	smp_mb();

#ifdef __linux__
	spin_lock_init(&_rPvrLock);
#endif				/* __linux__ */

	/* Allocate memory for global structures */
	mrRet = _PVR_InitAllocMem();
	if (DMX_FAILED(mrRet)) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail in _PVR_InitAllocMem, mrRet: 0x%x\r\n"),
			    DMX_FUNC_NAME, mrRet);
		FUNC_EXIT;
		MM_RETURN(mrRet);
	}
	smp_mb();

	PVR_LOG_DBG(TEXT("[PVR] %s line %d -- _PVR_EnablePower(TRUE, eInputType(0x%x))\r\n"),
		    DMX_FUNC_NAME, eInputType);
	smp_mb();
	
	_PVR_Lock();
    if (_DmxHwCurrentDx == D4) {
        PVR_LOG_DBG(TEXT("[PVR] %s line %d -- _DmxHwCurrentDx: D4\r\n"),
            DMX_FUNC_NAME, DMX_LINE_NO);
        fgNeedPowerOn = TRUE;
    }
    _PVR_Unlock();
	
    if (_PVR_IsDramLocalArbiterDisable()) {
        PVR_LOG_DBG(TEXT("[PVR] %s line %d -- local arbiter is disable\r\n"),
            DMX_FUNC_NAME, DMX_LINE_NO);
        fgNeedPowerOn = TRUE;
    }

    if (fgNeedPowerOn) {
        // Select clock and pin mux
        PVR_LOG_DBG(TEXT("[PVR] %s line %d -- do POWER_ON flow\r\n"),
          DMX_FUNC_NAME, DMX_LINE_NO);
        _PVR_EnablePower(TRUE, eInputType);

        smp_mb();

        // Load default uCode (for normal transport stream)
        PVR_LOG_DBG(TEXT("[PVR] %s line %d -- download microcode\r\n"),
          DMX_FUNC_NAME, DMX_LINE_NO);
        if (!_PVR_LoaduCode()) {
            PVR_LOG_ERR(TEXT("[PVR] %s fail in _PVR_LoaduCode(), eInputType: %d\r\n"),
                DMX_FUNC_NAME, eInputType);
            _PVR_EnablePower(FALSE, eInputType);
            _PVR_UninitAllocMem();
            FUNC_EXIT;
            MM_RETURN(RET_DMX_HW_ERROR);
        }
    } 
    else
    {
        PVR_LOG_DBG(TEXT("[PVR] %s line %d -- already POWER_ON, don't need to repoweron\r\n"),
                DMX_FUNC_NAME, DMX_LINE_NO);
    }

    smp_mb();

	PVR_LOG_DBG(TEXT("[DMX][HW] %s -- Reset SW Playback PID data structures\r\n"),
            DMX_FUNC_NAME);
    // Set SW Playback PID data structures
    for (i = 0; i < PVR_NUM_PID_INDEX; i++) {
        PVR_PID_STRUCT_T* prPidStruct;
        prPidStruct = _PVR_GetPidStruct(i);
        mm_memset((void*)prPidStruct, 0, sizeof(PVR_PID_STRUCT_T));
    }
    smp_mb();
	
	PVR_LOG_DBG(TEXT("[PVR] %s line %d -- set dmx's DMEM\r\n"),
        DMX_FUNC_NAME, DMX_LINE_NO);

	mrRet = _PVR_SetInputType(eInputType);
	if (DMX_FAILED(mrRet)) {
		PVR_LOG_ERR(TEXT
			    ("[PVR] %s fail in _PVR_SetInputType(eInputType: %d), mrRet: 0x%x\r\n"),
			    DMX_FUNC_NAME, eInputType, mrRet);
		_PVR_EnablePower(FALSE, eInputType);
		_PVR_UninitAllocMem();
		FUNC_EXIT;
		MM_RETURN(mrRet);
	}

	smp_mb();
	
	if (fgNeedPowerOn) {
        _PVR_EnableDramLocalArbiter();
    }

	_PVR_Lock();
	_rDmxPvrInfo.fgPvrInited = TRUE;
	_PVR_Unlock();

	FUNC_EXIT;

	MM_RETURN(RET_DMX_OK);
}

void _PVR_Uninit(void)
{
	u32 i = 0;
	
	if (_rDmxPvrInfo.fgPvrInited) {
		smp_mb();
		if (!_PVR_Stop()) {
			PVR_LOG_ERR(TEXT("[PVR] %s line %d fail in _PVR_Stop!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);
		}
		if (!_PVR_SetInputType(PVR_IN_NONE)) {
			PVR_LOG_ERR(TEXT
				    ("[PVR] %s line %d fail in _PVR_SetInputType(PVR_IN_NONE)!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);
		}
	} else {		/* in case that this function is called while pvr hw hasn't been initialized */

		smp_mb();
		if (!_PVR_Stop()) {
			PVR_LOG_ERR(TEXT("[PVR] %s line %d fail in _PVR_Stop!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);
		}
		smp_mb();

		_PVR_SetVideoType(DDI_PVR_DMA_PATH_ID, PVR_VIDEO_UNKNOWN);
		_PVR_SetVideoType(MINI_PVR_DMA_PATH_ID, PVR_VIDEO_UNKNOWN);
		
		smp_mb();

		_PVR_DDI_DeInit();

		smp_mb();
		if (!_PVR_DeInitISR()) {
			PVR_LOG_ERR(TEXT("[PVR] %s line %d fail in _PVR_UninitISR!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);
		}
	}

	smp_mb();
	_PVR_EnablePower(FALSE, _rDmxPvrInfo.ePvrInputType);

	smp_mb();
	_PVR_Lock();
	_rDmxPvrInfo.fgPvrMicroProcStopped = TRUE;
	_rDmxPvrInfo.ePvrInputType = PVR_IN_NONE;
	g_eDmxHwVideoType[DDI_PVR_DMA_PATH_ID] = PVR_VIDEO_UNKNOWN;
	g_eDmxHwVideoType[MINI_PVR_DMA_PATH_ID] = PVR_VIDEO_UNKNOWN;
	
#ifndef __linux__
	_DmxHwCurrentDx = D4;
#endif				/* __linux__ */
	_PVR_Unlock();

	// Set SW Playback PID data structures
	for (i = 0; i < PVR_NUM_PID_INDEX; i++)
	{
		PVR_PID_STRUCT_T* prPidStruct;
		prPidStruct = _PVR_GetPidStruct(i);
		mm_memset((void*)prPidStruct, 0, sizeof(PVR_PID_STRUCT_T));
	}
	smp_mb();


	_PVR_UninitAllocMem();

	_rDmxPvrInfo.fgPvrInited = FALSE;
}

/*-----------------------------------------------------------------------------*/
/** _PVR_SetInputMode
 */
/*-----------------------------------------------------------------------------*/
void _PVR_SetInputMode(PVR_INPUT_MODE_T rMode)
{
	u32 u4Reg;

	_PVR_Lock();
	u4Reg = DMXCMD_READ32(PVR_REG_CONFIG2);
	_PVR_Unlock();

	smp_mb();

	u4Reg &= 0x3fffffff;

	switch (rMode) {
	case PVR_PUSH:
		break;

	case PVR_HALF_PUSH:
		u4Reg |= (1U << 30);
		break;

	case PVR_PULL:
		u4Reg |= (3U << 30);
		break;

	default:
		DMX_ASSERT(FALSE);
		break;
	}

	smp_mb();

	_PVR_Lock();
	DMXCMD_WRITE32(PVR_REG_CONFIG2, u4Reg);

	/* Enable "DBM input full" signal to TS input device */
	smp_mb();

	u4Reg = DMXCMD_READ32(PVR_REG_DBM_BUF_CTRL) & (~0x3);
	u4Reg |= 0x1;
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_DBM_BUF_CTRL, u4Reg);

	_PVR_Unlock();
}


/*-----------------------------------------------------------------------------*/
/** _PVR_SetIgnorePESLen
 */
/*-----------------------------------------------------------------------------*/
void _PVR_SetIgnorePESLen(bool fgEnable)
{
	u32 u4Ctrl;

	_PVR_Lock();
	u4Ctrl = (CT_SETTING) & (~0xFF00);
	smp_mb();
	if (fgEnable)
		CT_SETTING = u4Ctrl | 0x100;

	mb();
	_PVR_Unlock();
}

/*-----------------------------------------------------------------------------*/
/** _PVR_SetScrambleScheme
 *	fgOrg - false means use PES header to decide scramble state
 *			true means use scramble flag to decide scramble state
 */
/*-----------------------------------------------------------------------------*/
void _PVR_SetScrambleScheme(bool fgOrg)
{
	u32 u4Ctrl;

	_PVR_Lock();
	u4Ctrl = (CT_SETTING) & (~0xFF0000);
	mb();
	if (fgOrg)
		u4Ctrl |= 0x10000;

	smp_mb();
	CT_SETTING = u4Ctrl;
	mb();
	_PVR_Unlock();
}


/*-----------------------------------------------------------------------------*/
/** _PVR_GetScrambleScheme
 *	return TRUE: new, FALSE: org
 */
/*-----------------------------------------------------------------------------*/
bool _PVR_GetScrambleScheme(void)
{
	u32 u4Ctrl;

	u4Ctrl = (CT_SETTING) & 0x10000;
	smp_mb();
	if (u4Ctrl != 0) {	/* original */
		return FALSE;
	}
	smp_mb();

	return TRUE;		/* new */
}


/*-----------------------------------------------------------------------------*/
/** _PVR_SetScrambleSchemeEx
 * u1Flag: bit0=1, both pes=0, flag=0 as scramble
 *		   bit1=1, pes=0, flag=1 as scramble
 *		   bit2=1, pes=1, flag=0 as scramble
 *		   bit3=1, both pes=1, flag=1 as scramble
 */
/*-----------------------------------------------------------------------------*/
void _PVR_SetScrambleSchemeEx(PVR_SCRAMBLE_TYPE_T eType, u8 u1Flag)
{
	u32 u4Ctrl;

	_PVR_Lock();
	u4Ctrl = (CT_SETTING) & (~0xFFFF0000);
	if (eType == PVR_SCRAMBLE_TSFLAG_ONLY) {
		u4Ctrl |= 0x10000;
	} else if (eType == PVR_SCRAMBLE_PESSTART_ONLY) {
		u4Ctrl |= 0x0;
	} else if (eType == PVR_SCRAMBLE_BOTH_TSFLAG_PESSTART) {
		u4Ctrl |= 0x20000;
		u4Ctrl |= ((u32) (u1Flag & 0xF) << 24);
	}

	mb();

	smp_mb();

	CT_SETTING = u4Ctrl;

	mb();

	_PVR_Unlock();
}


/*-----------------------------------------------------------------------------*/
/** _PVR_GetScrambleSchemeEx
 */
/*-----------------------------------------------------------------------------*/
bool _PVR_GetScrambleSchemeEx(PVR_SCRAMBLE_TYPE_T *peType, u8 *pu1Flag)
{
	u32 u4Ctrl;

	if (peType == NULL)
		return FALSE;

	_PVR_Lock();
	u4Ctrl = CT_SETTING;
	_PVR_Unlock();

	smp_mb();

	switch (((u4Ctrl & 0xFF0000) >> 16)) {
	case 0:
	default:
		*peType = PVR_SCRAMBLE_PESSTART_ONLY;
		break;
	case 1:
		*peType = PVR_SCRAMBLE_TSFLAG_ONLY;
		break;
	case 2:
		*peType = PVR_SCRAMBLE_BOTH_TSFLAG_PESSTART;
		if (pu1Flag == NULL)
			return FALSE;

		*pu1Flag = (u8) ((u4Ctrl >> 24) & 0xF);
		break;
	}

	return TRUE;
}


/*-----------------------------------------------------------------------------*/
/** _PVR_Version
 *	Show DMX version information
 */
/*-----------------------------------------------------------------------------*/
void _PVR_GetVersion(void)
{
	u16 u2CodeVersion;
	u8 u1Major, u1Minor;

	_PVR_Lock();
	u2CodeVersion = (u16) (_rDmxPvrInfo.u4uCodeVersion & 0xffff);
	_PVR_Unlock();

	smp_mb();
	u1Major = (u8) ((u2CodeVersion >> 8) & 0xf);
	u1Minor = (u8) (u2CodeVersion & 0xff);
	PVR_LOG_TRACE(TEXT("[PVR] PVR uCode Version: %u.%u \r\n"), u1Major, u1Minor);
}


/*-----------------------------------------------------------------------------*/
/** _PVR_IsMicroProcessorStopped
 *	Return TRUE if the micro-processor is stopped according to the global
 *	variable _rDmxPvrInfo.fgPvrMicroProcStopped.  Otherwise, return FALSE.
*/
/*-----------------------------------------------------------------------------*/
bool _PVR_IsMicroProcessorStopped(void)
{
	bool fgPvrMicroProcStopped;

	_PVR_Lock();
	fgPvrMicroProcStopped = _rDmxPvrInfo.fgPvrMicroProcStopped;
	_PVR_Unlock();

	smp_mb();
	return fgPvrMicroProcStopped;
}


/*-----------------------------------------------------------------------------*/
/** _PVR_SetDbm_InputSource
 */
/*-----------------------------------------------------------------------------*/
void _PVR_SetDbm_InputSource(u8 u1TsIdx, PVR_DBM_INPUT_SOURCE_T eSource)
{
	u32 u4Reg;

	FUNC_ENTRY;

	if ((u1TsIdx != 2) && (u1TsIdx != 3)) {
		PVR_LOG_ERR(TEXT("[DMX][HW] %s line %d Tsindex Error, ")
			     TEXT("_PVR_SetDbm_InputSource only support framer 2 and 3.\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		FUNC_EXIT;
		return;
	}

	smp_mb();

	_PVR_Lock();

	PVR_LOG_DBG(TEXT("[DMX][HW] %s line %d -- TsIdx: %u, eSource: %d\r\n"),
		    DMX_FUNC_NAME, DMX_LINE_NO, u1TsIdx, eSource);
	if (u1TsIdx == 2) {
		u4Reg = DMXCMD_READ32(PVR_REG_FRAMER_UNLOCK_CONTROL);
		u4Reg &= ~(0x3 << 24);
		u4Reg &= ~(0x3 << 26);

		switch (eSource) {
		case PVR_DBM_INPUT_DDI:
			u4Reg |= (0x2 << 24);	/* DDI to dbm ts2 index */
			u4Reg |= (0x2 << 26);	/* PB to dbm ts3 index */
			break;

		case PVR_DBM_INPUT_PB:
			u4Reg |= (0x0 << 24);	/* PB to dbm ts2 index */
			u4Reg |= (0x0 << 26);	/*  DDI to dbm ts3 index */
			break;

		default:
			break;
		}
		smp_mb();
		DMXCMD_WRITE32(PVR_REG_FRAMER_UNLOCK_CONTROL, u4Reg);
	} else if (u1TsIdx == 3) {
		u4Reg = DMXCMD_READ32(PVR_REG_FRAMER_UNLOCK_CONTROL);
		u4Reg &= ~(0x3 << 24);
		u4Reg &= ~(0x3 << 26);

		switch (eSource) {
		case PVR_DBM_INPUT_DDI:
			u4Reg |= (0x0 << 26);	/* DDI to dbm ts3 index */
			u4Reg |= (0x0 << 24);	/* PB to dbm ts2 index */
			break;

		case PVR_DBM_INPUT_PB:
			u4Reg |= (0x2 << 26);	/* PB to dbm ts3 index */
			u4Reg |= (0x2 << 24);	/* DDI to dbm ts2 index */
			break;

		default:
			break;
		}
		smp_mb();
		DMXCMD_WRITE32(PVR_REG_FRAMER_UNLOCK_CONTROL, u4Reg);
	} else {
		DMX_ASSERT(FALSE);
	}

	_PVR_Unlock();

	FUNC_EXIT;
}

/*-----------------------------------------------------------------------------*/
/** _PVR_SetFramerPktSize
 */
/*-----------------------------------------------------------------------------*/
bool _PVR_SetFramerPktSize(u8 u1TsIndex, u16 u2PktSize)
{
	u32 u4Reg;

	if (u1TsIndex >= PVR_FRAMER_COUNT)
		return FALSE;

	_PVR_Lock();

	smp_mb();

	if (u1TsIndex == 0) {
		u4Reg = DMXCMD_READ32(PVR_REG_FRAMER_PREBYTE_CTRL) & 0xFFFF0000;
		u4Reg |= 0x0100;
		u4Reg |= (u32) (u2PktSize & 0xFF);
		smp_mb();
		DMXCMD_WRITE32(PVR_REG_FRAMER_PREBYTE_CTRL, u4Reg);
	} else if (u1TsIndex == 1) {
		u4Reg = DMXCMD_READ32(PVR_REG_FRAMER_PREBYTE_CTRL) & 0xFFFF;
		u4Reg |= 0x01000000;
		u4Reg |= ((u32) (u2PktSize & 0xFF) << 16);
		smp_mb();
		DMXCMD_WRITE32(PVR_REG_FRAMER_PREBYTE_CTRL, u4Reg);
	} else {
		DMX_ASSERT(FALSE);
	}
	smp_mb();

	_PVR_ResetFramer(u1TsIndex);

	_PVR_Unlock();

	return TRUE;
}

MRESULT _PVR_PowerDown(void)
{
	PVR_LOG_TRACE(TEXT("[PVR] %s -- POWER_DOWN\r\n"), DMX_FUNC_NAME);
	smp_mb();

	if (_rDmxPvrInfo.fgPvrInited) {
		if (!_PVR_Stop()) {
			PVR_LOG_ERR(TEXT("[PVR] %s line %d fail in _PVR_Stop!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);
			MM_RETURN(RET_DMX_HW_ERROR);
		}
		smp_mb();
		if (!_PVR_EnableFTI(FALSE)) {	/* If the hw hasn't been initialized, this function will hung */
			PVR_LOG_ERR(TEXT("[PVR] %s line %d fail in _PVR_EnableFTI(FALSE)!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);
		}
	} else {		/* in case that this function is called while pvr hw hasn't been initialized */

		if (!_PVR_Stop()) {
			PVR_LOG_ERR(TEXT("[PVR] %s line %d fail in _PVR_Stop!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);
			MM_RETURN(RET_DMX_HW_ERROR);
		}
	}
	smp_mb();

	_PVR_EnablePower(FALSE, _rDmxPvrInfo.ePvrInputType);

	smp_mb();

	_PVR_Lock();
	_rDmxPvrInfo.fgPvrMicroProcStopped = TRUE;
	_rDmxPvrInfo.ePvrInputType = PVR_IN_NONE;
	g_eDmxHwVideoType[DDI_PVR_DMA_PATH_ID] = PVR_VIDEO_UNKNOWN;
	g_eDmxHwVideoType[MINI_PVR_DMA_PATH_ID] = PVR_VIDEO_UNKNOWN;
	_PVR_Unlock();

	PVR_LOG_TRACE(TEXT("[PVR] %s success, exit\r\n"), DMX_FUNC_NAME);

	MM_RETURN(RET_DMX_OK);
}

MRESULT _PVR_PowerOn(PVR_INPUT_TYPE_T eInputType)
{
	MRESULT mrRet = RET_DMX_OK;

	FUNC_ENTRY;

	smp_mb();
	/* Select clock and pin mux */
	_PVR_EnablePower(TRUE, eInputType);

	smp_mb();
	/* Load default uCode (for normal transport stream) */
	if (!_PVR_LoaduCode()) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail in _PVR_LoaduCode\r\n"), DMX_FUNC_NAME);
		return FALSE;
	}

	smp_mb();
	mrRet = _PVR_SetInputType(eInputType);
	if (DMX_FAILED(mrRet)) {
		PVR_LOG_ERR(TEXT
			    ("[PVR] %s fail in _PVR_SetInputType(eInputType: %d), mrRet: 0x%x\r\n"),
			    DMX_FUNC_NAME, eInputType, mrRet);
		_PVR_EnablePower(FALSE, eInputType);
		MM_RETURN(mrRet);
	}

	smp_mb();
	_PVR_Lock();
	g_eDmxHwVideoType[DDI_PVR_DMA_PATH_ID] = PVR_VIDEO_UNKNOWN;
	g_eDmxHwVideoType[MINI_PVR_DMA_PATH_ID] = PVR_VIDEO_UNKNOWN;
	_PVR_Unlock();

	_PVR_EnableDramLocalArbiter();

	FUNC_EXIT;

	MM_RETURN(mrRet);
}


bool _PVR_GetPowerStateString(DMX_PM_STATE eState, char *szPowerStr, u32 u4Len)
{
	if (u4Len < 3)
		return FALSE;

	mm_memset(szPowerStr, 0, u4Len);

	switch (eState) {
	case D0:
		strcpy(szPowerStr, "D0");
		break;
	case D1:
		strcpy(szPowerStr, "D1");
		break;
	case D2:
		strcpy(szPowerStr, "D2");
		break;
	case D3:
		strcpy(szPowerStr, "D3");
		break;
	case D4:
		strcpy(szPowerStr, "D4");
		break;
	default:
		strcpy(szPowerStr, "DN");
		break;
	}

	return TRUE;
}


bool _PVR_SetPowerState(DMX_PM_STATE PowerState, PVR_INPUT_TYPE_T eInputType)
{
	char szCurPowerState[3] = { 0 };
	char szNewPowerState[3] = { 0 };
	MRESULT mrRet = RET_DMX_OK;

	if (_PVR_GetPowerStateString(_DmxHwCurrentDx, szCurPowerState, 3)) {
		PVR_LOG_TRACE(TEXT("[PVR] %s -- Current Power State: %S\r\n"),
			      DMX_FUNC_NAME, szCurPowerState);
	}

	smp_mb();
	if (_PVR_GetPowerStateString(PowerState, szNewPowerState, 3)) {
		PVR_LOG_TRACE(TEXT("[PVR] %s -- To Set Power State: %S\r\n"),
			      DMX_FUNC_NAME, szNewPowerState);
	}

	smp_mb();
	if (_DmxHwCurrentDx == PowerState)
		return TRUE;

	smp_mb();
	if (VALID_DX(PowerState)) {
		switch (PowerState) {
		case D0:	/* Power Up */
		case D1:
		case D2:
			/* Resume (if D4 --> D0) */
			if ((D3 == _DmxHwCurrentDx) || (D4 == _DmxHwCurrentDx)) {
				smp_mb();
				mrRet = _PVR_PowerOn(eInputType);
				if (DMX_FAILED(mrRet)) {
					PVR_LOG_ERR(TEXT
						    ("[PVR] %s line %d failed in _PVR_PowerOn, mrRet: 0x%x\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
					MM_RETURN(mrRet);
				}
			}
			break;

		case D3:
		case D4:	/* Power Down */
			/* STOP mode (if D0 --> D4) */
			if ((D0 == _DmxHwCurrentDx) || (D1 == _DmxHwCurrentDx)
			    || (D2 == _DmxHwCurrentDx))
				_PVR_PowerDown();
			break;
		default:
			break;
		}
		smp_mb();

		/* Save new state */
		_DmxHwCurrentDx = PowerState;
	} else {
		return FALSE;
	}

	return TRUE;
}


DMX_PM_STATE _PVR_GetPowerState(void)
{
	char szPowerState[3] = { 0 };

	_PVR_GetPowerStateString(_DmxHwCurrentDx, szPowerState, 3);
	PVR_LOG_TRACE(TEXT("[PVR] %s -- Current Power State: %s\r\n"), DMX_FUNC_NAME, szPowerState);

	return _DmxHwCurrentDx;
}
