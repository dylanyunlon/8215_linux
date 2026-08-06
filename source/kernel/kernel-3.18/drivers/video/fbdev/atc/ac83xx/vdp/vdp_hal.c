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

/*****************************************************************************
*  Video Plane: Interface
*****************************************************************************/

#ifndef _VDP_HAL_C_
#define _VDP_HAL_C_

/*#include "strict_warning.h"*/
#ifndef __ARM2__
#include <linux/module.h>
#include <media/atc/pmx_hal.h>
#include <media/atc/display_inc.h>
#include "windows.h"
#include "x_debug.h"
#else
#include "x_types.h"
#include "assert.h"
#include "pmx_hal.h"
#include "display_inc.h"
#endif
#include "vdp_hw.h"
#include "vdp_hal.h"
#include "log.h"
#include "pmx_hw.h"

#include "x_lint.h"
#include "x_hal_ic.h"

#include "drv_common.h"
#include "drv_config.h"
#include "chip_ver.h"

#include "x_bim.h"
#include "x_os.h"
#include "x_assert.h"

#ifndef __ARM2__
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <asm/io.h>

int g_vdp_dump = 0;
VDP_BUF_IN_INFO vdpInBuf;
VDP_BUF_IN_INFO vdpOutBuf =
{
	.flag = 0
};
#endif

/******************************************************************************
* Local variable
******************************************************************************/
#define DEINTERLACE_DEFAULT_START_ADDRESS   0x50
#define DEINTERLACE_DEFAULT_SIZE            40



__u32 deinterlace_default[] = { /* start from 0x70042150*/
	0x000007ff, 0x000007ff, 0x10000000, 0x10000000,
	0x00000000, 0x80018700, 0x00000909, 0x05110000,
	0x00001800, 0x00000000, 0x00610707, 0x00300000,
	0x000bdf80, 0x00072080, 0x480186a0, 0x1b011940,
	0x0000c350, 0x00012003, 0x3202a02a, 0x0600c002,
	0x6401c01c, 0x32051004, 0x01436004, 0x0a002604,
	0x0332323c, 0x00320814, 0x00320432, 0x00005537,
	0x00100000, 0x01200510, 0x00002000, 0x00647700,
	0x00001407, 0x00000000, 0x00000000, 0x00000000,
	0xA0000000, 0x00000000, 0x00000000, 0x00000000,
};

HAL_VDO_UNION_T _rVdp1SwReg;
HAL_VDO_UNION_T _rVdp2SwReg;
__u8 _rVdp1RegMode[VDP_HAL_VDO_REG_NUM];
__u8 _rVdp2RegMode[VDP_HAL_VDO_REG_NUM];

static VDP_REGION_T _rVdp1SrcRegion = { 0, 0, 2, 1 };
static VDP_REGION_T _rVdp2SrcRegion = { 0, 0, 2, 1 };
static VDP_REGION_T _rVdp1OutRegion = { 0, 0, 2, 1 };
static VDP_REGION_T _rVdp2OutRegion = { 0, 0, 2, 1 };

static bool _fgVdp1ResetInVSync = FALSE;
static bool _fgVdp2ResetInVSync = FALSE;




static VDP_ACT_START_POS _tVdp2ActPos[] = {
	{RES_480P,          0x06D, 0x02C, 0x02C},
	{RES_576P,          0x079, 0x032, 0x032},
};

void vVdpHalReset(__u8 ucVdpId)
{
	if (ucVdpId == VDP_1) {
		_fgVdp1ResetInVSync = TRUE;
	} else {
		_fgVdp2ResetInVSync = TRUE;
	}
}

void vVdpHalInit(__u8 ucVdpId, bool fgHwReset)
{
	volatile __u32 *prVdpDiRegs = NULL;
	volatile HAL_VDO_UNION_T *prVdpHwRegs = NULL;
	__u32 u4Idx;

	GET_VDP_HW_PTR(ucVdpId, prVdpHwRegs);

	if (fgHwReset) {
		prVdpDiRegs = (ucVdpId == VDP_1) ? (__u32 *)(vdof_reg + DEINTERLACE_DEFAULT_START_ADDRESS) :
			      (__u32 *)(vdor_reg + DEINTERLACE_DEFAULT_START_ADDRESS);

		for (u4Idx = 0; u4Idx < DEINTERLACE_DEFAULT_SIZE; u4Idx++) {
			*prVdpDiRegs++ = deinterlace_default[u4Idx];
		}

		prVdpHwRegs->rField.u4VDORST = 0xFF;
		prVdpHwRegs->rField.u4VDORST = 0;
	}

	if (ucVdpId == VDP_1) {
		for (u4Idx = 0; u4Idx < VDP_HAL_VDO_REG_NUM; u4Idx++) {
			_rVdp1SwReg.au4Reg[u4Idx] = prVdpHwRegs->au4Reg[u4Idx];
		}
	} else {
		for (u4Idx = 0; u4Idx < VDP_HAL_VDO_REG_NUM; u4Idx++) {
			_rVdp2SwReg.au4Reg[u4Idx] = prVdpHwRegs->au4Reg[u4Idx];
		}
	}
}
EXPORT_SYMBOL(vVdpHalInit);

void vVdpHalSetSrcSize(__u8 ucVdpId, __u32 Width, __u32 Height)
{
	HAL_VDO_UNION_T *prVdpSwReg;
	__u8           *prVdpRegMode;
	__u8           *prPmxDispRegMode;
	PMX_DISP_REG_UNION_PTR_T prPmxDispReg;

	GET_VDP_PTR(ucVdpId, prVdpSwReg, prVdpRegMode);
	GET_PMX_DISP_PTR(ucVdpId, prPmxDispReg, prPmxDispRegMode);

	prVdpSwReg->rField.u4HBLOCK = ((Width + 7) >> 3);
	prVdpSwReg->rField.u4PIC_HEIGHT = Height;

	if (Width > 720) {
		prVdpSwReg->rField.u4DW_NEED_HD = ((Width + 3) >> 2);
		prVdpSwReg->rField.fgHDEN = 1;
	} else {
		prVdpSwReg->rField.u4DW_NEED_SD = ((Width + 3) >> 2);
		prVdpSwReg->rField.fgHDEN = 0;
	}

	prPmxDispReg.pDispMain->rField.u4PXLLEN = Width;

#ifndef __ARM2__
	if (1 == g_vdp_dump) {//OutBuf:420
		vdpOutBuf.u4Width = Width;
		vdpOutBuf.u4Height = Height;
		vdpOutBuf.u4YSize = Height * Width;
		vdpOutBuf.u4CSize = vdpOutBuf.u4YSize / 2;
	}
#endif

	FB_PRINT(FB_LOG_LVL_REGRW, "VDP", "vVdpHalSetSrcSize: ucVdpId %d, 0x10 %x\r\n"
		, (int)ucVdpId, (unsigned int)prVdpSwReg->au4Reg[(0x10 / 4)]);

	prVdpRegMode[(0x10 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	prVdpRegMode[(0xE0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	prPmxDispRegMode[(0x9C / 4)] |= PMX_HAL_REG_MODE_WRITE;
}

#ifndef __ARM2__
void vdp_dump_template(const unsigned long virt_addr, const unsigned int size, const unsigned char vdpDumpFile[])
{
	struct file *fp = NULL;
	mm_segment_t old_fs = 0;
	unsigned int ret = 0;
	loff_t pos = 0;
	fp = filp_open(vdpDumpFile, O_CREAT | O_RDWR, 0777);
	if (IS_ERR(fp)) {
		ret = PTR_ERR(fp);
		FB_PRINT(FB_LOG_LVL_ERR, "VDP", "[vdp_dump_template] open file %s error return %d\n", vdpDumpFile, ret);
		return;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	pos = 0;
	ret = vfs_write(fp, (char __user *)virt_addr, size, &pos);
	FB_PRINT(FB_LOG_LVL_INFO, "VDP", "[vdp_dump_template] write file virt_addr[0x%lx],size[%d], pos[%d]\r\n"
		, (unsigned long)virt_addr, (int)size, (int)pos);

	filp_close(fp, NULL);
	set_fs(old_fs);
}

void vVdpDumpTest(const unsigned char path[], const unsigned int dumpNum, const unsigned int io)
{
	char *virt_addr = NULL;
	unsigned char vdpDumpFile[200] = "";
	__u32 y_addr = 0;
	__u32 y_size = 0;
	__u32 c_addr = 0;
	__u32 c_size = 0;
	__u32 buf_cnt = 0;
	__u32 addr_cnt = 0;

	FB_PRINT(FB_LOG_LVL_INFO, "VDP", "[vVdpDumpTest] InBuf:Width*Height(%d*%d) \r\n"
		, vdpInBuf.u4Width, vdpInBuf.u4Height);
	FB_PRINT(FB_LOG_LVL_INFO, "VDP", "[vVdpDumpTest] OutBuf:Width*Height(%d*%d) \r\n"
		, vdpOutBuf.u4Width, vdpOutBuf.u4Height);

	for (; buf_cnt < dumpNum; buf_cnt++)
	{
		if (io == VDP_DUMP_IN) //in buf dump
		{
			y_addr = vdpInBuf.u4YAddr[0];
			c_addr = vdpInBuf.u4CAddr[0];
			y_size = vdpInBuf.u4YSize;
			c_size = vdpInBuf.u4CSize;

			if (y_addr == NULL || c_addr == NULL || y_size <= 0 || c_size <= 0)
			{
				FB_PRINT(FB_LOG_LVL_ERR, "VDP", "[vVdpDumpTest]InBuf: addr is NULL,y_addr(0x%x),c_addr(0x%x),y_size(%d),c_size(%d)\r\n"
					, y_addr, c_addr, y_size, c_size);
				continue;
			}

			sprintf(vdpDumpFile, "%s/vdp_in-y%d.bin", path, buf_cnt);
			virt_addr = ioremap(y_addr, y_size);
			if (virt_addr == NULL)
			{
				FB_PRINT(FB_LOG_LVL_ERR, "VDP", "[vVdpDumpTest]InBuf: virt_addr(0x%x) error\r\n", virt_addr);
				continue;
			}
			vdp_dump_template(virt_addr,y_size,vdpDumpFile);

			sprintf(vdpDumpFile, "%s/vdp_in-c%d.bin", path, buf_cnt);
			virt_addr = ioremap(c_addr, c_size);
			if (virt_addr == NULL)
			{
				FB_PRINT(FB_LOG_LVL_ERR, "VDP", "[vVdpDumpTest]InBuf: virt_addr(0x%x) error\r\n", virt_addr);
				continue;
			}
			vdp_dump_template(virt_addr,c_size,vdpDumpFile);
		}
		else if (io == VDP_DUMP_OUT) //out buf dump
		{
			if (vdpOutBuf.flag == 0)
			{
				if (addr_cnt == VDP_BUF_NUM)
				{
					addr_cnt = 0;
				}
				y_addr = vdpOutBuf.u4YAddr[addr_cnt];
				c_addr = vdpOutBuf.u4CAddr[addr_cnt];
				addr_cnt++;
			}
			else if (vdpOutBuf.flag == 1)
			{
				y_addr = vdpOutBuf.u4YAddr[0];
				c_addr = vdpOutBuf.u4CAddr[0];
			}

			y_size = vdpOutBuf.u4YSize;
			c_size = vdpOutBuf.u4CSize;

			if (y_addr == NULL || c_addr == NULL || y_size <= 0 || c_size <= 0)
			{
				FB_PRINT(FB_LOG_LVL_ERR, "VDP", "[vVdpDumpTest]OutBuf: addr is NULL,y_addr(0x%x),c_addr(0x%x),y_size(%d),c_size(%d)\r\n"
					, y_addr, c_addr, y_size, c_size);
				continue;
			}

			sprintf(vdpDumpFile, "%s/vdp_out-y%d.bin", path, buf_cnt);
			virt_addr = ioremap(y_addr, y_size);
			if (virt_addr == NULL)
			{
				FB_PRINT(FB_LOG_LVL_ERR, "VDP", "[vVdpDumpTest]OutBuf: virt_addr(0x%x) error\r\n", virt_addr);
				continue;
			}
			vdp_dump_template(virt_addr,y_size,vdpDumpFile);

			sprintf(vdpDumpFile, "%s/vdp_out-c%d.bin", path, buf_cnt);
			virt_addr = ioremap(c_addr, c_size);
			if (virt_addr == NULL)
			{
				FB_PRINT(FB_LOG_LVL_ERR, "VDP", "[vVdpDumpTest]OutBuf:  virt_addr(0x%x) error\r\n", virt_addr);
				continue;
			}
			vdp_dump_template(virt_addr,c_size,vdpDumpFile);
		}
		FB_PRINT(FB_LOG_LVL_INFO, "VDP", "[vVdpDumpTest] Dump Success! File:%s for VDP\r\n", vdpDumpFile);
	}
}
#endif

void vVdpHalSetSrcRegion(__u8 ucVdpId, __u32 X, __u32 Y, __u32 Width, __u32 Height)
{
	if (ucVdpId == VDP_1) {
		_rVdp1SrcRegion.X = X;
		_rVdp1SrcRegion.Y = Y;
		_rVdp1SrcRegion.Width = Width;
		_rVdp1SrcRegion.Height = Height;
	}

	if (ucVdpId == VDP_2) {
		_rVdp2SrcRegion.X = X;
		_rVdp2SrcRegion.Y = Y;
		_rVdp2SrcRegion.Width = Width;
		_rVdp2SrcRegion.Height = Height;
	}
}

void vVdpHalSetOutRegion(__u8 ucVdpId, __u32 X, __u32 Y, __u32 Width, __u32 Height)
{
	if (ucVdpId == VDP_1) {
		_rVdp1OutRegion.X = X;
		_rVdp1OutRegion.Y = Y;
		_rVdp1OutRegion.Width = Width;
		_rVdp1OutRegion.Height = Height;
	}

	if (ucVdpId == VDP_2) {
		_rVdp2OutRegion.X = X;
		_rVdp2OutRegion.Y = Y;
		_rVdp2OutRegion.Width = Width;
		_rVdp2OutRegion.Height = Height;
	}
}

void vVdpHalSetMode(__u8 ucVdpId, __u32 u4Mode)
{
	HAL_VDO_UNION_T *prVdpSwReg;
	__u8           *prVdpRegMode;
	PMX_DISP_REG_UNION_PTR_T prPmxDispReg;
	__u8           *prPmxDispRegMode;
	VDP_REGION_T    *prSrcRegion;
	VDP_REGION_T    *prOutRegion;
	__u32           u4Cnt, u4PosX = 0, u4PosY = 0, u4PosEY = 0;
	__s32            i4FmtHendMargin, i4FmtVendMargin, i4CcirHShift, i4CcirVShift, i4CcirHendMargin, i4CcirVendMargin;
	__u32           u4VdpOutWidth, u4VdpOutHeight, u4HFactor;
	__s32            i4StartLine, i4TmpStartLine, i4VScale;

	GET_VDP_PTR_REGION(ucVdpId, prVdpSwReg, prVdpRegMode, prSrcRegion, prOutRegion);
	GET_PMX_DISP_PTR(ucVdpId, prPmxDispReg, prPmxDispRegMode);


	if (ucVdpId == VDP_1) {
		if(u4Mode > RES_576P)
		{
			u4PosX = g_rPanelSetting.rFmtStart.u4OffsetX;
			u4PosY = u4PosEY = g_rPanelSetting.rFmtStart.u4OffsetY;
		}
		else
		{
			u4PosX = g_rVdoWindowSetting.u4FmtSdHstart;
			u4PosY = u4PosEY = g_rVdoWindowSetting.u4FmtSdVstart;
		}
	} else {
		for (u4Cnt = 0; u4Cnt < sizeof(_tVdp2ActPos) / sizeof(_tVdp2ActPos[0]); u4Cnt++) {
			if (_tVdp2ActPos[u4Cnt].u4Mode == u4Mode) {
				u4PosX = _tVdp2ActPos[u4Cnt].u4PosX;
				u4PosY = _tVdp2ActPos[u4Cnt].u4PosY;
				u4PosEY = _tVdp2ActPos[u4Cnt].u4PosEY;
				break;
			}
		}
	}


	FB_PRINT(FB_LOG_LVL_INFO, "VDP", "[@vVdpHalSetMode] u4Mode,u4Cnt=%d,%d,_u4LCDWidth,_u4LCDHeight =%d,%d,\r\n",
		u4Mode,u4Cnt,_u4LCDWidth,_u4LCDHeight);
	FB_PRINT(FB_LOG_LVL_INFO, "VDP", "[@vVdpHalSetMode] u4PosX,u4PosY,u4PosEY=0x%x,0x%x,0x%x \r\n",u4PosX,u4PosY,u4PosEY);

	if((u4Mode == RES_480P) || (u4Mode == RES_480I)) {
		u4VdpOutWidth = 720;
		u4VdpOutHeight = 480;
#if MASTER_MODE_ENABLE

		if (ucVdpId == VDP_1) {
			prPmxDispReg.pDispMain->rField.fgADJ_T = 1;
			prPmxDispReg.pDispMain->rField.u4H_TOTAL = 0x35A;
			prPmxDispReg.pDispMain->rField.u4V_TOTAL = 0x20E;
			prPmxDispRegMode[(0xD4 / 4)] |= PMX_HAL_REG_MODE_WRITE;
		}

#endif
	}
	else if((u4Mode == RES_576P) || (u4Mode == RES_576I)) {
		u4VdpOutWidth = 720;
		u4VdpOutHeight = 576;
#if MASTER_MODE_ENABLE

		if (ucVdpId == VDP_1) {
			prPmxDispReg.pDispMain->rField.fgADJ_T = 1;
			prPmxDispReg.pDispMain->rField.u4H_TOTAL = 0x360;
			prPmxDispReg.pDispMain->rField.u4V_TOTAL = 0x272;
			prPmxDispRegMode[(0xD4 / 4)] |= PMX_HAL_REG_MODE_WRITE;
		}

#endif
	}
	else {
		u4VdpOutWidth = _u4LCDWidth;
		u4VdpOutHeight = _u4LCDHeight;
	}

	prVdpSwReg->rField.fgVDOEN = 1;
	prVdpSwReg->rField.u4P_SKIP = prSrcRegion->X & ~0x1;

	prVdpSwReg->rField.u4TOTAL_STAMBR = 0x1E;
	prVdpSwReg->rField.u4MAX_STAMBR = 0x1D;

	prVdpSwReg->rField.fgSP_LD = 1;

	if ((ucVdpId == VDP_1) && (prSrcRegion->Height == u4VdpOutHeight) && (prOutRegion->Height == _u4LCDHeight)) {
		prVdpSwReg->rField.u4VSCALE = 1 << VDO_VSCALE_SHIFT;
	} else {
		if (prPmxDispReg.pDispMain->rField.fgPRGS) {
			prVdpSwReg->rField.u4VSCALE = (prSrcRegion->Height << VDO_VSCALE_SHIFT) / prOutRegion->Height;
		} else {
			prVdpSwReg->rField.u4VSCALE =
				((prSrcRegion->Height << VDO_VSCALE_SHIFT) / prOutRegion->Height) << 1;
		}
	}

	FB_PRINT(FB_LOG_LVL_REGRW, "VDP", "vVdpHalSetMode: ucVdpId %d, u4VdpOutWidth %d, u4VdpOutHeight %d\r\n"
		, (int)ucVdpId, (int)u4VdpOutWidth, (int)u4VdpOutHeight);
	FB_PRINT(FB_LOG_LVL_REGRW, "VDP", "vVdpHalSetMode: ucVdpId %d, _u4LCDWidth %d, _u4LCDHeight %d\r\n"
		, (int)ucVdpId, (int)_u4LCDWidth, (int)_u4LCDHeight);
	FB_PRINT(FB_LOG_LVL_REGRW, "VDP", "vVdpHalSetMode: ucVdpId %d, prOutRegion->Width %d, prOutRegion->Height %d\r\n",
		(int)ucVdpId, (int)prOutRegion->Width, (int)prOutRegion->Height);
	FB_PRINT(FB_LOG_LVL_REGRW, "VDP", "vVdpHalSetMode: ucVdpId %d, prSrcRegion->Width %d, prSrcRegion->Height %d\r\n",
		(int)ucVdpId, (int)prSrcRegion->Width, (int)prSrcRegion->Height);

	if (prSrcRegion->Width > 720) {
		/*Set start line and start sub-line*/
		prVdpSwReg->au4Reg[0x20 / 4] = 0x00000000;
		prVdpSwReg->au4Reg[0x24 / 4] = 0x00000000;
		prVdpSwReg->au4Reg[0x28 / 4] = 0x00000000;
		prVdpSwReg->au4Reg[0x2C / 4] = 0x00000000;
		prVdpSwReg->au4Reg[0x50 / 4] = 0x00000000;
		prVdpSwReg->au4Reg[0x54 / 4] = 0x00000000;
	} else {
		if (prPmxDispReg.pDispMain->rField.fgPRGS) {
			/*Set start line and start sub-line*/
			i4StartLine = prSrcRegion->Y;
			prVdpSwReg->rField.u4YSLTT  = i4StartLine;
			prVdpSwReg->rField.u4YSSLTT = i4StartLine;
			i4TmpStartLine = i4StartLine - 1;
			prVdpSwReg->rField.u4YSLBT  = i4TmpStartLine > 0 ? i4TmpStartLine : 0x800 + i4TmpStartLine;
			prVdpSwReg->rField.u4YSSLBT = i4StartLine + 0x80;

			i4VScale = prVdpSwReg->rField.u4VSCALE;
			prVdpSwReg->rField.u4YSLTB  = i4VScale + i4StartLine;
			prVdpSwReg->rField.u4YSSLTB = i4StartLine;
			i4TmpStartLine = i4VScale + i4StartLine - 1;
			prVdpSwReg->rField.u4YSLBB  = i4TmpStartLine > 0 ? i4TmpStartLine : 0x800 + i4TmpStartLine;
			prVdpSwReg->rField.u4YSSLBB = i4StartLine + 0x80;

			i4TmpStartLine = (i4StartLine >> 1) - 2;
			prVdpSwReg->rField.u4CSLTT  = i4TmpStartLine > 0 ? i4TmpStartLine : 0x800 + i4TmpStartLine;
			prVdpSwReg->rField.u4CSSLTT = (i4StartLine >> 1) + 0xE0;
			i4TmpStartLine = (i4StartLine >> 1) - 1;
			prVdpSwReg->rField.u4CSLBT  = i4TmpStartLine > 0 ? i4TmpStartLine : 0x800 + i4TmpStartLine;
			prVdpSwReg->rField.u4CSSLBT = (i4StartLine >> 1) + 0x60;

			i4TmpStartLine = (i4VScale >> 1) + (i4StartLine >> 1) - 2;
			prVdpSwReg->rField.u4CSLTB  = i4TmpStartLine > 0 ? i4TmpStartLine : 0x800 - i4TmpStartLine;
			prVdpSwReg->rField.u4CSSLTB = (i4VScale % 2) + (i4StartLine >> 1) + 0xE0;
			i4TmpStartLine = (i4VScale >> 1) + (i4StartLine >> 1) - 1;
			prVdpSwReg->rField.u4CSLBB  = i4TmpStartLine > 0 ? i4TmpStartLine : 0x800 - i4TmpStartLine;
			prVdpSwReg->rField.u4CSSLBB = (i4VScale % 2) + (i4StartLine >> 1) + 0x60;
		} else {
			/*Set start line and start sub-line for interlace mode*/
			if (prSrcRegion->Y == 0) {
				prVdpSwReg->au4Reg[0x20 / 4] = 0x00010000;
				prVdpSwReg->au4Reg[0x24 / 4] = 0x07FF07FE;
			}

			prVdpSwReg->au4Reg[0x28 / 4] = 0x80800000;
			prVdpSwReg->au4Reg[0x2C / 4] = 0x2060a0e0;
			prVdpSwReg->au4Reg[0x50 / 4] = 0x000007FF;
			prVdpSwReg->au4Reg[0x54 / 4] = 0x000007FF;
		}
	}
	i4FmtHendMargin =  g_rVdoWindowSetting.rFmtWindowSetting.i4H0EndMargin;
	i4FmtVendMargin =  g_rVdoWindowSetting.rFmtWindowSetting.i4V0EndMargin;
	i4CcirHShift =     g_rVdoWindowSetting.rFmtWindowSetting.i4H1StartShift;
	i4CcirVShift =     g_rVdoWindowSetting.rFmtWindowSetting.i4V1StartShift;
	i4CcirHendMargin = g_rVdoWindowSetting.rFmtWindowSetting.i4H1EndMargin;
	i4CcirVendMargin = g_rVdoWindowSetting.rFmtWindowSetting.i4V1EndMargin;

	if ((ucVdpId == 1) && (prSrcRegion->Width == u4VdpOutWidth) && (prOutRegion->Width == _u4LCDWidth)) {
		prPmxDispReg.pDispMain->rField.u4HSFACTOR = 1 << FMT_HSCALE_SHIFT;
		prPmxDispReg.pDispMain->rField.u4HACTBGN = u4PosX + prOutRegion->X;
		prPmxDispReg.pDispMain->rField.u4HACTEND = u4PosX + prOutRegion->X + prSrcRegion->Width + i4FmtHendMargin;
		prPmxDispReg.pDispMain->rField.u4VOACTBGN = u4PosY + prOutRegion->Y;
		prPmxDispReg.pDispMain->rField.u4VOACTEND = u4PosY + prOutRegion->Y + prSrcRegion->Height + i4FmtVendMargin;
		prPmxDispReg.pDispMain->rField.u4VEACTBGN = u4PosEY + prOutRegion->Y;
		prPmxDispReg.pDispMain->rField.u4VEACTEND = u4PosEY + prOutRegion->Y + prSrcRegion->Height + i4FmtVendMargin;
	} else {
		u4HFactor = (prSrcRegion->Width << FMT_HSCALE_SHIFT) / prOutRegion->Width;

		if (u4HFactor > 0x200) {
			prPmxDispReg.pDispMain->rField.u4HSFACTOR = u4HFactor >> 1;
			prVdpSwReg->rField.fgXHALF = 1;
		} else {
			prPmxDispReg.pDispMain->rField.u4HSFACTOR = u4HFactor;
			prVdpSwReg->rField.fgXHALF = 0;
		}

		prPmxDispReg.pDispMain->rField.u4HACTBGN = u4PosX + prOutRegion->X;
		prPmxDispReg.pDispMain->rField.u4HACTEND = u4PosX + prOutRegion->X + prOutRegion->Width + i4FmtHendMargin;
		prPmxDispReg.pDispMain->rField.u4VOACTBGN = u4PosY + prOutRegion->Y;
		prPmxDispReg.pDispMain->rField.u4VOACTEND = u4PosY + prOutRegion->Y + prOutRegion->Height + i4FmtVendMargin;
		prPmxDispReg.pDispMain->rField.u4VEACTBGN = u4PosEY + prOutRegion->Y;
		prPmxDispReg.pDispMain->rField.u4VEACTEND = u4PosEY + prOutRegion->Y + prOutRegion->Height + i4FmtVendMargin;
	}

	prPmxDispReg.pDispMain->rField.fgHSON = 1;
	prPmxDispRegMode[(0xA0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	prPmxDispRegMode[(0xA4 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	prPmxDispRegMode[(0xA8 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	prPmxDispRegMode[(0xB0 / 4)] |= PMX_HAL_REG_MODE_WRITE;

	if (ucVdpId == VDP_1) {
		prPmxDispReg.pDispMain->rField.fgCCIR = 1;
		prPmxDispReg.pDispMain->rField.u4CCIR_HBGN =  u4PosX + i4CcirHShift;
		if (prSrcRegion->Width > 720) {
			prPmxDispReg.pDispMain->rField.u4CCIR_HEND =  u4PosX + i4CcirHShift + u4VdpOutWidth + i4CcirHendMargin;
		} else {
			prPmxDispReg.pDispMain->rField.u4CCIR_HEND =  u4PosX + i4CcirHShift + u4VdpOutWidth + i4CcirHendMargin - 1;
		}
		prPmxDispReg.pDispMain->rField.u4CCIR_VOBGN = u4PosY + i4CcirVShift;
		prPmxDispReg.pDispMain->rField.u4CCIR_VOEND = u4PosY + i4CcirVShift + u4VdpOutHeight + i4CcirVendMargin;
		prPmxDispReg.pDispMain->rField.u4CCIR_VEBGN = u4PosEY;
		prPmxDispReg.pDispMain->rField.u4CCIR_VEEND = u4PosEY + i4CcirVShift + u4VdpOutHeight + i4CcirVendMargin;
		prPmxDispRegMode[(0xD0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
		prPmxDispRegMode[(0xE0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
		prPmxDispRegMode[(0xF0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	}

	/* Add auto reset function; disable for read pull down motion and comb*/
	/*prVdpSwReg->rField.u4AUTO_RST_WIDTH = 0xFFFF;*/
	/*prVdpSwReg->rField.fgAUTO_RST_EN = 1;*/
	/*prVdpRegMode[(0xD4 / 4)] |= PMX_HAL_REG_MODE_WRITE;*/

	prVdpRegMode[(0x14 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	prVdpRegMode[(0x18 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	prVdpRegMode[(0x1C / 4)] |= PMX_HAL_REG_MODE_WRITE;
	prVdpRegMode[(0x20 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	prVdpRegMode[(0x24 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	prVdpRegMode[(0x28 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	prVdpRegMode[(0x2C / 4)] |= PMX_HAL_REG_MODE_WRITE;
	prVdpRegMode[(0x34 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	prVdpRegMode[(0x50 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	prVdpRegMode[(0x54 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	prVdpRegMode[(0x64 / 4)] |= PMX_HAL_REG_MODE_WRITE;

	prPmxDispRegMode[(0xA0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	prPmxDispRegMode[(0xA4 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	prPmxDispRegMode[(0xA8 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	prPmxDispRegMode[(0xB0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
}

void vVdpHalSetFifo(__u8 ucVdpId)
{
	HAL_VDO_UNION_T *prVdpSwReg;
	__u8           *prVdpRegMode;

	GET_VDP_PTR(ucVdpId, prVdpSwReg, prVdpRegMode);

	if (prVdpSwReg->rField.u4VSCALE > (VDO_VSCALE_STEP << 1)) {
		if (prVdpSwReg->rField.fgHDEN) {
			prVdpSwReg->au4Reg[0x34 / 4] = 0x3c483c3c;
		} else {
			prVdpSwReg->au4Reg[0x34 / 4] = 0x24482424;
		}
	} else {
		if (prVdpSwReg->rField.fgHDEN) {
			if (_u4LCDHeight == 800) {
				prVdpSwReg->au4Reg[0x34 / 4] = 0x28482828;
			} else if (_u4LCDWidth == 1920) {
				prVdpSwReg->au4Reg[0x34 / 4] = 0x3c483c3c;
			} else {
				prVdpSwReg->au4Reg[0x34 / 4] = 0x38483838;
			}
		} else {
			prVdpSwReg->au4Reg[0x34 / 4] = 0x14481414;
		}
	}

	prVdpRegMode[(0x34 / 4)] |= PMX_HAL_REG_MODE_WRITE;
}

void vVdpHalSetYuv422(__u8 ucVdpId, bool fgYUV422)
{
	HAL_VDO_UNION_T *prVdpSwReg;
	__u8 *prVdpRegMode;

	GET_VDP_PTR(ucVdpId, prVdpSwReg, prVdpRegMode);

	prVdpSwReg->rField.fgYUV422 = fgYUV422;
	prVdpRegMode[(0x30 / 4)] |= PMX_HAL_REG_MODE_WRITE;
}

void vVdpHalSetScanLine(__u8 ucVdpId, bool fgScLineEnable)
{
	HAL_VDO_UNION_T *prVdpSwReg;
	__u8 *prVdpRegMode;

	GET_VDP_PTR(ucVdpId, prVdpSwReg, prVdpRegMode);

	prVdpSwReg->rField.fgSC_LN = fgScLineEnable;
	prVdpRegMode[(0xE0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
}

void vVdpHalSetHFilterMode(__u8 ucVdpId, bool fgFIRMode)
{
	PMX_DISP_REG_UNION_PTR_T prPmxDispReg;
	__u8 *pucPmxDispRegMode;

	GET_PMX_DISP_PTR(ucVdpId, prPmxDispReg, pucPmxDispRegMode);

	prPmxDispReg.pDispMain->rField.fgHSLR = !fgFIRMode;

	pucPmxDispRegMode[(0xB0) / 4] |= PMX_HAL_REG_MODE_WRITE;
}

void vVdpHalSetYBufPtr(__u8 ucVdpId, __u32 u4AddrY, __u32 u4AddrC)
{
	if ((u4AddrY & 0x1FF) || (u4AddrC & 0x1FF)) {
		FB_PRINT(FB_LOG_LVL_ERR, "VDP", "vVdpHalSetVdoPtr: address not align, u4AddrY = 0x%x, u4AddrC = 0x%x\r\n",
			(unsigned int)u4AddrY, (unsigned int)u4AddrC);
		ASSERT(0);
	} else {
		HAL_VDO_UNION_T *prVdpSwReg;
		__u8 *prVdpRegMode;

		GET_VDP_PTR(ucVdpId, prVdpSwReg, prVdpRegMode);

		prVdpSwReg->rField.u4W_ADDR_Y = u4AddrY >> 7;
		prVdpSwReg->rField.u4X_ADDR_Y = u4AddrY >> 7;
		prVdpSwReg->rField.u4Y_ADDR_Y = u4AddrY >> 7;
		prVdpSwReg->rField.u4Z_ADDR_Y = u4AddrY >> 7;

		prVdpSwReg->rField.u4X_ADDR_C = u4AddrC >> 7;
		prVdpSwReg->rField.u4Y_ADDR_C = u4AddrC >> 7;
		prVdpSwReg->rField.u4Z_ADDR_C = u4AddrC >> 7;
		prVdpRegMode[(0x00 / 4)] |= PMX_HAL_REG_MODE_WRITE;
		prVdpRegMode[(0x04 / 4)] |= PMX_HAL_REG_MODE_WRITE;
		prVdpRegMode[(0x08 / 4)] |= PMX_HAL_REG_MODE_WRITE;
		prVdpRegMode[(0x0C / 4)] |= PMX_HAL_REG_MODE_WRITE;
		prVdpRegMode[(0x80 / 4)] |= PMX_HAL_REG_MODE_WRITE;
		prVdpRegMode[(0x84 / 4)] |= PMX_HAL_REG_MODE_WRITE;
		prVdpRegMode[(0xFC / 4)] |= PMX_HAL_REG_MODE_WRITE;
	}
}

void vVdpHalSetDeintWXYZ(__u8 ucVdpId, const __u32 *pu4AddrY, const __u32 *pu4AddrC)
{
	if ((pu4AddrY) && (pu4AddrC)) {
		HAL_VDO_UNION_T *prVdpSwReg;
		__u8 *prVdpRegMode;

		GET_VDP_PTR(ucVdpId, prVdpSwReg, prVdpRegMode);

		prVdpSwReg->rField.u4W_ADDR_Y = (__u32)(pu4AddrY[0] >> 7);
		prVdpSwReg->rField.u4X_ADDR_Y = (__u32)(pu4AddrY[1] >> 7);
		prVdpSwReg->rField.u4Y_ADDR_Y = (__u32)(pu4AddrY[2] >> 7);
		prVdpSwReg->rField.u4Z_ADDR_Y = (__u32)(pu4AddrY[3] >> 7);

		prVdpSwReg->rField.u4X_ADDR_C = (__u32)(pu4AddrC[1] >> 7);
		prVdpSwReg->rField.u4Y_ADDR_C = (__u32)(pu4AddrC[2] >> 7);
		prVdpSwReg->rField.u4Z_ADDR_C = (__u32)(pu4AddrC[3] >> 7);


		prVdpRegMode[(0x00 / 4)] |= PMX_HAL_REG_MODE_WRITE;
		prVdpRegMode[(0x04 / 4)] |= PMX_HAL_REG_MODE_WRITE;
		prVdpRegMode[(0x08 / 4)] |= PMX_HAL_REG_MODE_WRITE;
		prVdpRegMode[(0x0C / 4)] |= PMX_HAL_REG_MODE_WRITE;
		prVdpRegMode[(0x80 / 4)] |= PMX_HAL_REG_MODE_WRITE;
		prVdpRegMode[(0x84 / 4)] |= PMX_HAL_REG_MODE_WRITE;
		prVdpRegMode[(0xFC / 4)] |= PMX_HAL_REG_MODE_WRITE;
	} else {
		ASSERT(0);
	}
}

void vVdpHalSetFieldInfo(__u8 ucVdpId, bool fgTopFldFirst, bool fgFirstFld)
{
	HAL_VDO_UNION_T *prVdpSwReg;
	__u8 *prVdpRegMode;

	GET_VDP_PTR(ucVdpId, prVdpSwReg, prVdpRegMode);

	if (fgTopFldFirst) {
		prVdpSwReg->rField.fgPFLD = !fgFirstFld;      /*Field Setting 0 for top, 1 for bottom.*/
	} else {
		prVdpSwReg->rField.fgPFLD = fgFirstFld;
	}

	prVdpRegMode[(0x30 / 4)] |= PMX_HAL_REG_MODE_WRITE;
}

void vVdpHalGetFieldInfo(__u32 u4VdpId, __u32 *prFldInfo)
{
	volatile HAL_VDO_UNION_T *prVdpHwReg;

	GET_VDP_HW_PTR(u4VdpId, prVdpHwReg);

	*prFldInfo = prVdpHwReg->rField.fgPFLD;
}

void vVdpHalSetDeintMode(__u8 ucVdpId, __u8 ucDiMode)
{
	HAL_VDO_UNION_T *prVdpSwReg;
	__u8 *prVdpRegMode;

	GET_VDP_PTR(ucVdpId, prVdpSwReg, prVdpRegMode);

	prVdpSwReg->rField.u4TST = 0x10;
	prVdpRegMode[(0x1C / 4)] |= PMX_HAL_REG_MODE_WRITE;

	prVdpSwReg->rField.u4CTHRD = 0x5;
	prVdpRegMode[(0x38 / 4)] |= PMX_HAL_REG_MODE_WRITE;

	if (ucDiMode > VDP_DI_HD_MODE) {
		prVdpSwReg->rField.fgMA4F = 1;
		prVdpSwReg->rField.fgMDDI_CLK_ON = 1;

		prVdpSwReg->rField.fgY_I2P_NOFRM = 1;
		prVdpSwReg->rField.fgC_I2P_NOFRM = 1;

		prVdpSwReg->au4Reg[0xE0 / 4] |= 0xA0000000;

		prVdpSwReg->rField.fgY_FRM = 0;
		prVdpSwReg->rField.fgC_FRM = 0;
	} else {
		prVdpSwReg->rField.fgMA4F = 0;
		prVdpSwReg->rField.fgMDDI_CLK_ON = 0;

		prVdpSwReg->rField.fgY_I2P_NOFRM = 0;
		prVdpSwReg->rField.fgC_I2P_NOFRM = 0;

		prVdpSwReg->au4Reg[0xE0 / 4] &= 0x5FFFFFFF;

		if (ucDiMode == VDP_DI_FRAME_MODE) {
			prVdpSwReg->rField.fgY_FRM = 1;
			prVdpSwReg->rField.fgC_FRM = 1;
		} else {
			prVdpSwReg->rField.fgY_FRM = 0;
			prVdpSwReg->rField.fgC_FRM = 0;
		}
	}

	prVdpRegMode[(0x30 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	prVdpRegMode[(0x7C / 4)] |= PMX_HAL_REG_MODE_WRITE;
	prVdpRegMode[(0x88 / 4)] |= PMX_HAL_REG_MODE_WRITE;
	prVdpRegMode[(0xE0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
}

void vVdpHalSetPdDetect(__u8 ucVdpId, bool fgEnable)
{
	HAL_VDO_UNION_T  *prVdpSwReg;
	__u8 *prVdpRegMode;

	GET_VDP_PTR(ucVdpId, prVdpSwReg, prVdpRegMode);

	prVdpSwReg->rField.fgPD32 = fgEnable;
	prVdpRegMode[(0x1C / 4)] |= PMX_HAL_REG_MODE_WRITE;

	prVdpSwReg->rField.fgPD32NEW_EN = fgEnable;
	/*prVdpSwReg->rField.fgPD32NEW_CNT = fgEnable;*/
	prVdpSwReg->rField.fgVDO_PD_CTRL_MODE = fgEnable;
	prVdpRegMode[(0xC4 / 4)] |= PMX_HAL_REG_MODE_WRITE;
}

void vVdpHalSetMergeZ(__u32 u4VDP, bool fgMergeZ)
{
	HAL_VDO_UNION_T *prVdpSwReg;
	__u8 *prVdpRegMode;

	GET_VDP_PTR(u4VDP, prVdpSwReg, prVdpRegMode);

	if (fgMergeZ) {
		prVdpSwReg->rField.fgDISP3 = 1;
		prVdpSwReg->rField.fgTFLD = 1;
		prVdpSwReg->rField.fgSTILL  = 1;
	} else {
		prVdpSwReg->rField.fgDISP3 = 0;
		prVdpSwReg->rField.fgTFLD = 0;
		prVdpSwReg->rField.fgSTILL  = 1;
	}

	prVdpRegMode[(0x1C / 4)] |= PMX_HAL_REG_MODE_WRITE;
}


void vVdpHalDisMergeZ(__u32 u4VDP)
{
	HAL_VDO_UNION_T *prVdpSwReg;
	__u8 *prVdpRegMode;

	GET_VDP_PTR(u4VDP, prVdpSwReg, prVdpRegMode);

	prVdpSwReg->rField.fgSTILL = 0;
	prVdpSwReg->rField.fgDISP3 = 0;
	prVdpSwReg->rField.fgTFLD = 0;
	prVdpRegMode[(0x1C / 4)] |= PMX_HAL_REG_MODE_WRITE;
}

void vVdpHalResume(__u8 ucVdpId)
{
	__u32 u4RegIdx = 0;
	HAL_VDO_UNION_T *prVdpSwReg = NULL;
	HAL_VDO_UNION_T *prVdpHwReg = NULL;
	__u8 *prVdpRegMode = NULL;

	GET_VDP_HW_PTR(ucVdpId, prVdpHwReg);
	GET_VDP_PTR(ucVdpId, prVdpSwReg, prVdpRegMode);

	for (u4RegIdx = 0; u4RegIdx < VDP_HAL_VDO_REG_NUM; u4RegIdx++) {
		prVdpHwReg->au4Reg[u4RegIdx] = prVdpSwReg->au4Reg[u4RegIdx];
		prVdpRegMode[u4RegIdx] &= ~PMX_HAL_REG_MODE_WRITE;
	}

	prVdpHwReg->rField.u4VDORST = 0xFF;
	prVdpHwReg->rField.u4VDORST = 0x0;
}

void vVdpHalIsr(__u8 ucVdpId)
{
	HAL_VDO_UNION_T *prVdpSwReg;
	volatile HAL_VDO_UNION_T *prVdpHwReg;
	__u8 *pVdpRegMode;
	__u32 u4RegIdx;

	GET_VDP_PTR(ucVdpId, prVdpSwReg, pVdpRegMode);
	GET_VDP_HW_PTR(ucVdpId, prVdpHwReg);

	/* update plane mixer display register at vsync*/
	for (u4RegIdx = 0; u4RegIdx < VDP_HAL_VDO_REG_NUM; u4RegIdx++) {
		if (pVdpRegMode[u4RegIdx] & PMX_HAL_REG_MODE_WRITE) {
			prVdpHwReg->au4Reg[u4RegIdx] = prVdpSwReg->au4Reg[u4RegIdx];
			pVdpRegMode[u4RegIdx] &= ~PMX_HAL_REG_MODE_WRITE;
		}

		if (pVdpRegMode[u4RegIdx] & PMX_HAL_REG_MODE_READ) {
			prVdpSwReg->au4Reg[u4RegIdx] = prVdpHwReg->au4Reg[u4RegIdx];
		}
	}

	if (prVdpHwReg->rField.fgMA4F == 1) {
		/* chroma detect*/
		if ((prVdpHwReg->rField.u4XZ_MOTION_C < 0xFF) && (prVdpHwReg->rField.u4FLD_WY_MOTION < 0xFF)) {
			/* turn off chroma detection*/
			prVdpHwReg->rField.fgCRM_3FMD = 0;
			prVdpHwReg->rField.fgCRM_FDIFF = 0;
			prVdpHwReg->rField.fgCRM_EXP_OFF = 0;
		} else {
			prVdpHwReg->rField.fgCRM_3FMD = 1;
			prVdpHwReg->rField.fgCRM_FDIFF = 1;
			prVdpHwReg->rField.fgCRM_EXP_OFF = 1;
		}
	} else {
		prVdpHwReg->rField.fgCRM_3FMD = 0;
		prVdpHwReg->rField.fgCRM_FDIFF = 0;
		prVdpHwReg->rField.fgCRM_EXP_OFF = 0;
	}

	if (prVdpHwReg->rField.fgDISP3 == 1) { /* Workaround HW MergeZ issue*/
		prVdpHwReg->rField.u4X_ADDR_C = prVdpHwReg->rField.u4Z_ADDR_C;
	}

	if (_fgVdp1ResetInVSync && (ucVdpId == VDP_1)) {
		prVdpHwReg->rField.u4VDORST = 0xFF;
		prVdpHwReg->rField.u4VDORST = 0;

		_fgVdp1ResetInVSync = FALSE;
	}

	if (_fgVdp2ResetInVSync && (ucVdpId == VDP_2)) {
		prVdpHwReg->rField.u4VDORST = 0xFF;
		prVdpHwReg->rField.u4VDORST = 0;

		_fgVdp2ResetInVSync = FALSE;
	}
}

void vVdpHalGetMotionComb(__u32 u4VdpId, __u32 *prXYComb, __u32 *prWXComb, __u32 *prWYMotion, __u32 *prXZMotion)
{
	volatile HAL_VDO_UNION_T *prVdpHwReg;

	GET_VDP_HW_PTR(u4VdpId, prVdpHwReg);

	*prXYComb = prVdpHwReg->rField.u4YX_CMB_CNT;
	*prWXComb = prVdpHwReg->rField.u4FLD_WX_COMB;
	*prWYMotion = prVdpHwReg->rField.u4FLD_WY_MOTION;
	*prXZMotion = prVdpHwReg->rField.u4XZ_MOTION_C;
	/*FB_PRINT(FB_LOG_LVL_REGRW, "VDP", "vVdpHalGetMotionComb id %d, XY %x, WX %x, WY %x, XZ %x\r\n"
		, (int)u4VdpId, (unsigned int)prVdpHwReg->rField.u4YX_CMB_CNT
		, (unsigned int)prVdpHwReg->rField.u4FLD_WX_COMB
		, (unsigned int)prVdpHwReg->rField.u4FLD_WY_MOTION
		, (unsigned int)prVdpHwReg->rField.u4XZ_MOTION_C);*/
}
#endif



