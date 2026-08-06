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
*[File]                 aud_if_hw_asrc.h
*[Version]              v1.0
*[Revision Date]        2014-03-10
*[Author]               tongfa.luo@autochips.com 
*[Description]
*      aud asrc hw driver intreface 
*
******************************************************************************/

#ifndef _AUD_IF_HW_ASRC_H_
#define _AUD_IF_HW_ASRC_H_

#ifdef __linux__
#include <mach/83xx_irqs_vector.h>
#endif
#include "aud_if_comm.h"
#include "aud_if_hw.h"


/**********************************************************************************
*
*  macros 
*
**********************************************************************************/

#define ASRC_IC_VFY                     (0)
#define ASRC_AUTO_TRACING               (0)

#define ASRC_BUFFER_RESERVE_SIZE        (48U) 
#define ASRC_OUTPUT_SAFE_READ_SIZE      (96U)

#define REG_GPS_ASRC_BASE               (0xA8600)
#define REG_AP_ASRC_BASE                (0xA8800)

#define GET_ASRC_VECTOR(eAsrcType)      ((eAsrcType == GPS_ASRC) ? VECTOR_ASRC_GPS : VECTOR_ASRC_AP)
#define GET_ASRC_REGBASE(eAsrcType)     ((eAsrcType == GPS_ASRC) ? REG_GPS_ASRC_BASE : REG_AP_ASRC_BASE)
#define GET_ASRC_TYPE(u4Vect)           ((u4Vect == VECTOR_ASRC_GPS) ? GPS_ASRC : AP_ASRC)

#define ASRC_CHSET_NUM                  (6U)
#define ASRC_PALETTE_NUM                (8U)


/**********************************************************************************
*
*   For Debug Log
*
**********************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

extern u32 _u4AsrcLog;
extern u32 _u4AsrcLogCnt[3];

#define ASRCLOG_ERR(exp, ...)                AUDLOG(_u4AsrcLog & ALOG_ERR,  T("[AUD][ASRC]"), exp, ##__VA_ARGS__)
#define ASRCLOG_WARN(exp, ...)               AUDLOG(_u4AsrcLog & ALOG_WARN, T("[AUD][ASRC]"), exp, ##__VA_ARGS__)
#define ASRCLOG_INFO(exp, ...)               AUDLOG(_u4AsrcLog & ALOG_INFO, T("[AUD][ASRC]"), exp, ##__VA_ARGS__)
#define ASRCLOG_CLI(exp, ...)                AUDLOG(_u4AsrcLog & ALOG_CLI,  T("[AUD][ASRC][CLI]"), exp, ##__VA_ARGS__)

#define ASRCLOG_DBG(exp, ...)                AUDLOG(_u4AsrcLog & ALOG_DBG,  T("[AUD][ASRC]"), exp, ##__VA_ARGS__)

#define ASRCLOG_TEST(exp, ...)               AUDLOG(_u4AsrcLog & ALOG_BIT8,  T("[AUD][ASRC][TEST]"), exp, ##__VA_ARGS__)

#define ASRCLOG_ERR_DBG(err, exp, ...)    \
    if (err){ \
        ASRCLOG_ERR(exp, ##__VA_ARGS__); \
    } else { \
        ASRCLOG_DBG(exp, ##__VA_ARGS__); \
    }


#if 0
#define ASRCLOG_DATA(exp)      \
    AUDLOG(_u4AsrcLog & ALOG_DBG, (T("[ASRC_DATA]")), exp) 
#else
#define ASRCLOG_DATA(exp)
#endif


/**********************************************************************************
*
*   Data type
*
**********************************************************************************/


typedef s32 (*PFN_ISR_CB)(u32 u4Param, u32 u4IntType);


typedef enum 
{
    GPS_ASRC = 0,
    AP_ASRC,
    ASRC_TYPE_NUM,
}ASRC_CLS_TYPE;


enum ASRC_BUF_INT_E
{
    ASRC_IBUF_EMPTY_INT =   (0x01 << 0),
    ASRC_IBUF_AMOUNT_INT =  (0x01 << 1),
    ASRC_OBUF_OV_INT =      (0x01 << 2),
    ASRC_OBUF_AMOUNT_INT =  (0x01 << 3),
    ASRC_ALL_INT =          (0x0F)
};


enum ASRC_BUF_ST_E
{
    ASRC_BUF_ST_IBUF_EMPTY =    (1 << 0),
    ASRC_BUF_ST_OBUF_FULL =     (1 << 1),
};


typedef struct
{
   PFN_ISR_CB pfnCb;
   u32 u4Param;
   u32 u4IntType;
   
}ASRC_CHS_ISRCB_T, *PASRC_CHS_ISRCB_T;


typedef struct 
{ 
  u32 u4Chn;                     // 1: Mono; 2: Stereo
  u32 u4IFS;                     // Input frequency
  u32 u4OFS;                     // Output frequency
  u32 u4IBW;                     // Input Bitwidth (16 or 24)
  u32 u4OBW;                     // Output Bitwidth (16 or 24)
  
  u32 u4IPalette;                // Palette index of input frequency
  u32 u4OPalette;                // Palette index of output frequency
  
}ASRC_CHS_FMT_T, *PASRC_CHS_FMT_T;  // Setting for a channel set


typedef struct
{
    PASRC_CHS_FMT_T prFmt;
    PASRC_CHS_ISRCB_T prIsrCb;
    
}ASRC_CHS_CFG_T, *PASRC_CHS_CFG_T;


//===================================================================================//

typedef struct
{
    AUD_HW_IO_IF_T rHwIf;

    u32 (*GetBufState)(void * pThis);

    u32 (*GetIdx)(void * pThis);

    void (*LogAttribute)(void * pThis);

    u32 (*Delete)(void * pThis);
    
}ASRC_CHS_CLS_PUB, *PASRC_CHS_CLS_PUB;

typedef struct
{     
    u32 (*AllocPalette)(void * pThis, u32 u4FS);   
    u32 (*FreePalette)(void * pThis, u32 u4Idx, u32 u4OIdx);   
    u32 (*SetFixPalette)(void * pThis, u32 u4FS);
    u32 (*ModifyFixPalette)(void * pThis, u32 u4FS);
    u32 (*ClrFixPalette)(void * pThis, u32 u4Idx);
      
    bool (*SetFixAsrc)(void * pThis);  
    bool (*ClrFixAsrc)(void * pThis);

    PASRC_CHS_CLS_PUB (*AllocAsrc)(void * pThis, bool fgUseFixAsrc); 

    void (*HibernationCtrl)(void * pThis, bool fgWakeUp);
    u32 (*Start)(void * pThis);    
    u32 (*Stop)(void * pThis);
        
#if (ASRC_AUTO_TRACING)
    void   (*SetAutoTracing)(void * pThis, u32 u4TracingClk);
    u32 (*GetAutoTracing)(void * pThis);   
#endif  

    void (*LogAttribute)(void * pThis);
    void (*LogChsAttribute)(void * pThis, u32 u4Idx);
    void (*LogAllRegs)(void * pThis);

    u32 (*Delete)(void * pThis);

}ASRC_MGR_CLS_PUB, *PASRC_MGR_CLS_PUB;


/**********************************************************************************
*
*   Functions For ASRC MANAGE
*
**********************************************************************************/

extern PASRC_MGR_CLS_PUB AsrcMgr_New(ASRC_CLS_TYPE eType, u32 u4IChSz, u32 u4OChSz);
extern PASRC_MGR_CLS_PUB AsrcMgr_Get(ASRC_CLS_TYPE eType);

extern void AsrcTest_Cmd(u32 u4Type, u32 u4Param1, u32 u4Param2);

#ifdef __cplusplus
}
#endif // __cplusplus


#endif  //_AUD_IF_HW_ASRC_H_
