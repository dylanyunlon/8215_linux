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




#ifndef _AUD_COMM_DATATYPE_H_
#define _AUD_COMM_DATATYPE_H_

#include "aud_comm_macros.h"

//============================================================//

enum AUD_RET_TYPE_E
{
    AUD_RET_OK  = 0,
    AUD_RET_FAIL,
    AUD_RET_PARAMS_ERR,
    AUD_RET_INVALID_STATE,
};


enum AUD_STATE_E
{
    AUD_STATE_UNINIT = 0,
    AUD_STATE_INITED,
    AUD_STATE_STOPPED,
    AUD_STATE_STARTED    
};


//==============================================================//

typedef struct
{
    u32 u4PhySddr;       // physical start address
    u32 u4VirSAdr;       // Virtual start address of buffer 
    u32 u4ChBufSz;       // Channel Buffer size (in byte)    
    u32 u4DataOff;       // Data start offset (in byte)
    u32 u4DataSize;      // Data size of every channels (in byte)
    u32 u4Chn;           // channel number
    u32 u4BW;            // bit width
}AUD_DATA_BUF_T, *PAUD_DATA_BUF_T;


typedef struct
{
    u32 u4DstSAdr;
    u32 u4DstChBufSz;
    u32 u4DstChn;
    u32 u4DstBW;            

    u32 u4SrcSAdr;
    u32 u4SrcChBufSz;
    u32 u4SrcChn;
    u32 u4SrcBW;            

    u32 u4CopySamples;
    
}AUD_COPY_DATA_T, *PAUD_COPY_DATA_T;


typedef enum
{
    BUF_INIT = 0,
    BUF_EMPTY,
    BUF_NOT_EMPTY,
    BUF_FULL,
    BUF_NOT_FULL,
}AUD_BUF_STATE_E;


typedef struct
{
    u32 u4PhyAddr;
    u32 u4VirAddr;
    u32 u4ChSz;
    u32 u4Chn;      

    AUD_BUF_STATE_E eState;
    u32 u4Point;
    
}AUD_BUF_INFO_T, *PAUD_BUF_INFO_T;


typedef struct
{
    u32 u4PhyAddr;
    u32 u4VirAddr;
    u32 u4Size;
    
}AUD_MEMORY_INFO_T, *PAUD_MEMORY_INFO_T;


//==========================================================================//
#if 0

#ifdef AUD_DATA_TYPE_MACROS

typedef HANDLE                      HANDLE;
typedef bool                        bool;
typedef void                        VOID;
typedef unsigned s8               u8;

typedef signed s8                 s8;
typedef signed short                s16;
typedef signed long                 s32;
typedef signed s32                  INTx;
typedef unsigned s8               u8;
typedef unsigned short              u16;
typedef unsigned long               u32;

typedef void *                      void *; 
typedef volatile unsigned s8 *    P_U8;
typedef volatile signed s8 *      P_S8;
typedef volatile unsigned short *   P_U16;
typedef volatile signed short *     P_S16;
typedef volatile unsigned s32 *     P_U32;
typedef volatile signed s32 *       P_S32;
typedef unsigned long long *        P_U64;
typedef signed long long *          P_S64;


#endif //AUD_DATA_TYPE_MACROS
#endif


//==========================================================================//

#endif  //_AUD_COMM_DATATYPE_H_
