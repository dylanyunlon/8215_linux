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

/******************************************************************************
*[File]                     aud_asrc_hw.h
*[Version]                  v1.0
*[Revision Date]            2014-03-10
*[Author]                   tongfa.luo@autochips.com 
*[Description]
*       asrc hw register control 
*
******************************************************************************/
#ifndef _AUD_ASRC_HW_H_
#define _AUD_ASRC_HW_H_

#include "aud_if_hw_asrc.h"
#include "aud_asrc_reg.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


/********************************************************************
*
* Asrc register struct
*
********************************************************************/

typedef struct
{
    void (*Enable)(void * pThis, bool fgAsrcEn);
    bool (*ChSetEnable)(void * pThis, u32 u4Idx, bool fgChSetEn);
    bool (*ChSetClear)(void * pThis, u32 u4Idx);

    void (*ClearIFR)(void * pThis, bool fgClear);

    u32 (*GetIBufEmptyFlag)(void * pThis, u32 u4ChIdx);
    u32 (*GetIBufAmountFlag)(void * pThis, u32 u4Idx);
    u32 (*GetOBufOvFlag)(void * pThis, u32 u4Idx);
    u32 (*GetOBufAmountFlag)(void * pThis, u32 u4Idx);

    void (*INTEnable)(void * pThis, u32 u4Idx, u32 u4IntType);
    void (*INTDisable)(void * pThis, u32 u4Idx, u32 u4IntType);

    bool  (*SetChCalcAmount)(void * pThis, u32 u4Idx, u32 u4CalcNum);
    bool (*SetChFs)(void * pThis, u32 u4Idx, u32 u4IPalette, u32 u4OPalette);
    void (*SetChMono)(void * pThis, u32 u4Idx, bool fgMono);
    u32 (*GetChMono)(void * pThis, u32 u4Idx);
    bool (*SetChBitWidth)(void * pThis, u32 u4Idx, u32 u4IBW, u32 u4OBW);

    bool (*SetPaletteFS)(void * pThis, u32 u4PaletteIdx, u32 u4FS);
    u32 (*GetPaletteFS)(void * pThis, u32 u4PaletteIdx);

    bool (*SetIBufAddr)(void * pThis, u32 u4StartAddr, u32 u4ChBufSize);
    bool (*SetOBufAddr)(void * pThis, u32 u4StartAddr, u32 u4ChBufSize);

    bool (*SetIRP)(void * pThis, u32 u4Idx, u32 u4IBufRp);
    bool (*SetIWP)(void * pThis, u32 u4Idx, u32 u4IBufWp);
    bool (*SetOWP)(void * pThis, u32 u4Idx, u32 u4OBufWp);
    bool (*SetORP)(void * pThis, u32 u4Idx, u32 u4OBufRp);

    u32 (*GetOWP)(void * pThis, u32 u4Idx);
    u32 (*GetORP)(void * pThis, u32 u4Idx);
    u32 (*GetIRP)(void * pThis, u32 u4Idx);
    u32 (*GetIWP)(void * pThis, u32 u4Idx);

    bool (*SetIBufIntrCnt)(void * pThis, u32 u4Idx, u32 u4IntrCnt);
    bool (*SetOBufIntrCnt)(void * pThis, u32 u4Idx, u32 u4IntrCnt);

    bool (*SetMaxOutPerIn)(void * pThis, u32 u4Idx, u32 u4MaxOutPerIn);

    void (*AutoTraceEnable)(void * pThis, bool fgEnable, u32 u4TracingClk);

    void (*Reset)(void * pThis);

    void (*LogAllRegs)(void * pThis);
    
    u32 (*Delete)(void * pThis);
    
}ASRC_HW_CLS_PUB, *PASRC_HW_CLS_PUB;


PASRC_HW_CLS_PUB AsrcHw_New(ASRC_CLS_TYPE eType);


#ifdef __cplusplus
}
#endif // __cplusplus


#endif  //_AUD_ASRC_HW_H_

