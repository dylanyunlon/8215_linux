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

#ifndef _DRV_FS_CMD_H_
#define _DRV_FS_CMD_H_

#define DRV_FS_RDONLY 0
#define DRV_FS_W_C 1
#define DRV_FS_RW_C 2

#define MAX_FILENAME_SZ 50
#define MAX_READ_BYTES  32*1024

#define DRV_FSR_SUCCESS 0
#define DRV_FSR_FAIL    -1
#define DRV_FSR_NULL_POINT            -2
#define DRV_FSR_BUF_ADDR_ALIGN_ERR    -3
#define DRV_FSR_ONLY_SUPPORT_RDONLY   -4
#define DRV_FSR_PARAMETER_ERR         -5


extern INT32 DrvFSMount(UINT32 dwDriveNo, UINT32 *pu4DrvFSTag);

extern INT32 DrvFSUnMount(void);

extern INT32 DrvFSUSBMount(UINT32 dwDriveNo, UINT32 *pu4DrvFSTag);

extern INT32 DrvFSUSBUnMount(void);


extern INT32 DrvFSOpenFile(char* pcDirFileName, UINT32 dwFlags, INT32* piFd);

extern INT32 DrvFSGetFileSize(INT32 iFd, UINT32 *pu4FileSize);
extern INT32 DrvFSSeekFile(INT32 iFd, INT64 iOffset, INT32 iOrigin);
extern INT32 DrvFSReadFile(INT32 iFd, void* pbBuf, UINT32 u4Count);
extern INT32 DrvFSWriteFile(INT32 iFd, const void *pbBuf, DWORD dwSize);
extern INT32 DrvFSCloseFile(INT32 iFd);


#endif /* #ifndef _DRV_FS_CMD_H_ */

