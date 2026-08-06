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

#ifndef _AUD_COMM_MISC_H_
#define _AUD_COMM_MISC_H_

#include "aud_comm_os.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


typedef void (*PFN_COPY_DATA)(PAUD_COPY_DATA_T prCopy, u32 u4Param);


//===========================================================//

void AudMisc_BufferData_Read(u32 u4SAdr, u32 u4Size);
void AudMisc_BufferData_Draw(u32 u4SAdr, u32 u4Size, u32 u4BW, u32 u4CutBit, u32 u4CutSpace);

//===========================================================//

void AudMisc_CopyData_File2Buf(PAUD_COPY_DATA_T prCopy, u32 u4Param);
void AudMisc_CopyData_Buf2File(PAUD_COPY_DATA_T prCopy, u32 u4Param);
void AudMisc_CopyData(PAUD_COPY_DATA_T prCopy, u32 u4Param);
void AudMisc_CopyData_Mgr(PAUD_DATA_BUF_T prDst, PAUD_DATA_BUF_T prSrc, PFN_COPY_DATA pfnCopy, u32 u4Param);
u32 AudMisc_FifoFreeSize_Get(u32 u4Wp, u32 u4Rp, u32 BufLen);
u32 AudMisc_FifoDataSize_Get(u32 u4Wp, u32 u4Rp, u32 BufLen);




//===========================================================//


#ifdef __cplusplus
}
#endif // __cplusplus

#endif  //_AUD_COMM_MISC_H_
