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

#ifndef _MIC_DRV_H_
#define _MIC_DRV_H_

#include "drv_config.h"

// *********************************************************************
// Global constant
// *********************************************************************
#define MIC_DRV_OK	     (INT32)(0)
#define MIC_DRV_FAIL    (INT32)(-1)


//For Mic in buffer
#define MP_WORK_MEM_LENGTH    (0x100) //0x100 * 4 BYTE (DWORD ALIGNMENT)
//One Bank width : 64 samples * (2/3)DWORD = 48 DWORD = 192 (BYTE)

#define MP_BUF_DWORD_LEN         (0xF0)     //AEC need 320 byte, MIC capture  one bank date into MIC buffer  per time
                                         //960byte is Least Common Multiple, 960 / 4 = 240 <=> 0xF0

#define MP_BUF_BYTE_LEN       (960)


#define REG_MPBUF1_SADR        (0x288)
#define REG_MPBUF2_SADR        (0x289)
#define REG_MPBUF3_SADR        (0x28A)
#define REG_MPBUF_ADR          (0x28B)
#define REG_MPBUF_CFG          (0x28C)

typedef enum
{
   ADDR_MODE_DRAM_ALE = 0,
   ADDR_MODE_INPUT_SAMPLE_COUNT
} UPDATE_DRAM_ADDR_MODE_E;

// *********************************************************************
// Export Variable
// *********************************************************************

extern UINT32 _u4MPBuf[3];



// *********************************************************************
// Export API
// *********************************************************************
VOID MicInit();
VOID MicUnInit();
INT32 MicGetBuffer(VOID* pFrameBuffer, UINT32 u4GetMicBufWidth);

#endif
