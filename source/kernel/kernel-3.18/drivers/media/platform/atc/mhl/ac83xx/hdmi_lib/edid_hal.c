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

/* #include "typedef.h" */
#include "x_ckgen.h"
#include "edid_data.h"
#include "hdmi_hw_reg.h"

#include "edid_data.h"
#include "edid_hal.h"
#include "hdmi_debug.h"
#include "hdmi_hw_reg.h"

/* #include "metazoneex.h" */


#define EDID_NUM_MAX   (256)


/* load Edid from Metazone to pOutBuf */
VOID MZ_LoadEdidData(UINT8 *pOutBuf, UINT32 u4Size)
{
	MZ_LoadBinaryData(0, pOutBuf, u4Size);

}

/* load Hdcp from Metazone to pOutBuf */
VOID MZ_LoadHdcpData(UINT8 *pOutBuf, UINT32 u4Size)
{
	MZ_LoadBinaryData(0, pOutBuf, u4Size);

}

/* load binary data from metazone to pOutBuf */
VOID MZ_LoadBinaryData(UINT32 u4BinaryStart, UINT8 *pOutBuf, UINT32 u4Size)
{
	/* UINT32 u4BinaryIndex = 0; */
	/* UINT32 u4Remain = 0; */
	/* UINT32 u4Copysize = 0; */
	/* UINT8 *pBuf = 0; */

#if 0
	if (MZ_SUCCESS != MetaZone_Init()) {
		HDMI_LOG(HDMI_LOG_ERROR, "MetaZone Init fail \r\n");
	} else {
		pBuf = pOutBuf;
		u4BinaryIndex = 0;
		u4Remain = u4Size;

		for (u4Remain = u4Size; u4Remain > 0; u4Remain -= u4Copysize) {
			if (u4Remain >= MZ_BINARY_MAX_SIZE)
				u4Copysize = MZ_BINARY_MAX_SIZE;
			else
				u4Copysize = u4Remain;

			MetaZone_ReadBinary(u4BinaryStart + u4BinaryIndex, pBuf, u4Copysize);
			HDMI_LOG(HDMI_LOG_INFO, "Metazone binary read, index=0x%x, size=%d \r\n",
			u4BinaryStart + u4BinaryIndex, u4Copysize);

			u4BinaryIndex++;
			pBuf += u4Copysize;
		}
	}

#endif
}


