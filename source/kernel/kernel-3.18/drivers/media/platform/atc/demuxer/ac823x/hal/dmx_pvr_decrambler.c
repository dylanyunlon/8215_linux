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
 * @file dmx_pvr_descrambler.c
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
/* #include <media/atc/mm_debug.h> */
#else
#include "mm_debug.h"
#endif /* __linux__*/

#include "dmx_def.h"
#include "dmx_pvr.h"

/*-----------------------------------------------------------------------------*/
/* Configurations*/
/*-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------*/
/* Constant definitions*/
/*-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------*/
/* Type definitions*/
/*-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------*/
/* Macro definitions*/
/*-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------*/
/* Static variables*/
/*-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------*/
/* Static functions*/
/*-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------*/
/* Inter-file functions*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/** _PVR_DMEM_CA_Init
 */
/*-----------------------------------------------------------------------------*/
void _PVR_DMEM_CA_Init(void)
{
	/*clear KEY and Ctrl*/
	memset_io((void *)PVR_DMEM_CA_KEY_BASE, 0, PVR_DMEM_CA_KEY_SIZE * PVR_DMEM_CA_KEY_NUM);
	mb();
	memset_io((void *)PVR_DMEM_CA_CTRL_BASE, 0, 4 * PVR_DMEM_CA_KEY_NUM);
	mb();

	/*clear PVR_DMEM_MM_KEY*/
	memset_io((void *)PVR_DMEM_MM_KEY_BASE, 0, PVR_DMEM_MM_KEY_FIELD_SIZE * 4);
	mb();
}

bool _PVR_SetDmemAesKey(const u16 u2KeyLen, const u8 au1Keys[PVR_DMEM_MM_KEY_LEN])
{
	s32  i4Idx = 0;
	u32 u4Word = 0;
	u32 u4MapIdx = 0;
	u32 u4DecreaseIdx = 0;

	switch (u2KeyLen) {
	case 128:
		u4MapIdx = 4;
		u4DecreaseIdx = 16;
		break;
	case 192:
		u4MapIdx = 2;
		u4DecreaseIdx = 8;
		break;
	case 256:
		u4MapIdx = 0;
		u4DecreaseIdx = 0;
		break;
	default:
		PVR_LOG_ERR(TEXT("%s line %d fail in invalid KeyLen(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u2KeyLen);
		return FALSE;
	}
	smp_mb();

	if (au1Keys != NULL) {
		for (i4Idx = PVR_DMEM_MM_KEY_LEN - u4DecreaseIdx - 4; i4Idx >= 0; i4Idx -= 4, u4MapIdx++) {
			u4Word = (au1Keys[i4Idx] << 24) | (au1Keys[i4Idx + 1] << 16) |
				(au1Keys[i4Idx + 2] << 8) | (au1Keys[i4Idx + 3]);
			smp_mb();
			PVR_DMEM_MM_KEY(u4MapIdx) = u4Word;
			mb();
		}
	}

	return TRUE;
}

bool _PVR_SetDmemAesIV(const u16 u2KeyLen, const u8 au1Ivs[PVR_DMEM_MM_IV_LEN])
{
	s32  i4Idx = 0;
	u32 u4Word = 0;
	u32 u4MapIdx = 0;

	if (au1Ivs != NULL) {
		for (u4MapIdx = 0, i4Idx = PVR_DMEM_MM_IV_LEN - 4; i4Idx >= 0; i4Idx -= 4, u4MapIdx++) {
			u4Word = (au1Ivs[i4Idx] << 24) | (au1Ivs[i4Idx + 1] << 16) |
				(au1Ivs[i4Idx + 2] << 8) | (au1Ivs[i4Idx + 3]);
			smp_mb();
			PVR_DMEM_MM_IV(u4MapIdx) = u4Word;
			mb();
		}
	}

	return TRUE;
}

