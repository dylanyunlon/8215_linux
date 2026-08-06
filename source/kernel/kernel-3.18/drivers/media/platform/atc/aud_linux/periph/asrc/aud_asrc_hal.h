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



#ifndef __ASRC_HAL_3360_H_
#define __ASRC_HAL_3360_H_

#ifdef __linux__
#include "aud_drv.h"
#endif
#include "aud_asrc_hw.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#ifdef __linux__
#define AC83XX_PCM_ASRC_VA              AC83XX_PCM_VA
#define AC83XX_PCM_ASRC_PA              AC83XX_PCM_PA
#endif
//=================================================//

typedef struct
{
    u32 u4FS;
    u32 u4Ref;
    bool fgFixUse;
    
}ASRC_PALETTE_CFG_T, *PASRC_PALETTE_CFG_T;


typedef struct 
{
    u32 u4IPhyAddr;
    u32 u4IVirAddr;
    u32 u4ISz;                 // Total size of input buffer (in bytes)
    u32 u4IChSz;
    
    u32 u4OPhyAddr;
    u32 u4OVirAddr;
    u32 u4OSz;                 // Total size of output buffer (in bytes)
    u32 u4OChSz;
    
}ASRC_MEMORY_T, *PASRC_MEMORY_T;


typedef struct
{
    ASRC_MGR_CLS_PUB rPub;
    
    u32 u4State; 
    u32 u4StartNum;
    
    ASRC_CLS_TYPE eType;
    u32 u4Vect;  

    PASRC_HW_CLS_PUB prHw;
    bool afgAsrcFixUse[ASRC_CHSET_NUM];

    ASRC_MEMORY_T rMemory;    
    ASRC_PALETTE_CFG_T arPaletteCfg[ASRC_PALETTE_NUM];
    
#if (ASRC_AUTO_TRACING)
    u32 u4AutoTracingClk;
#endif
    
}ASRC_MGR_CLS, *PASRC_MGR_CLS;


//=======================================================//

typedef struct 
{ 
    u32 u4State;         // ASRC_BUF_ST_E
    
    u32 u4IChSize;
    u32 u4OChSize;
    
    u32 u4ISAdr;
    u32 u4OSAdr;

    u32 u4IVirSAdr[2];       // Virtual Start address of input buffer of each channel
    u32 u4OVirSAdr[2];       // Virtual start address of output buffer of each channel
    
}ASRC_CHS_BUF_T, *PASRC_CHS_BUF_T;


typedef struct 
{
    ASRC_CHS_CLS_PUB rPub;
    
    u32 u4State;

    PASRC_MGR_CLS prMgr;
    u32 u4Idx;

    ASRC_CHS_ISRCB_T rIsrCb;
    ASRC_CHS_FMT_T rFmt;
    ASRC_CHS_BUF_T rBuf;

}ASRC_CHS_CLS, *PASRC_CHS_CLS;


//=========================================================//

PASRC_CHS_CLS_PUB AsrcChs_New(ASRC_CLS_TYPE eType, u32 u4Idx);
PASRC_CHS_CLS_PUB AsrcChs_Get(ASRC_CLS_TYPE eType, u32 u4Idx);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif
