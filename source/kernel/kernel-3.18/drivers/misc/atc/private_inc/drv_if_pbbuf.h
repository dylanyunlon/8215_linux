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
#ifndef _DRV_PBBUF_IF_H_
#define _DRV_PBBUF_IF_H_

#include "x_typedef.h"
#include "drv_ibc.h"


//Data Type
#define PB_BUF_DATA     1
#define PB_BUF_INBAND   2

// Notification for other driver.
typedef enum
{
  DRV_PBBUF_COND_READY_TO_GET_READ_BUFFER = 0,
  DRV_PBBUF_COND_RELEASE_ALL_SLOTS
} DRV_PBBUF_NOTIFY_COND_T;

typedef VOID (*DRV_PBBUF_NFY_FCT)
(
  VOID *pvTag,
  DRV_PBBUF_NOTIFY_COND_T eReadyCond,
  INT32 i4Data1,
  UINT32 u4Data2
);

typedef struct _DRV_PBBUF_NFY_FCT_T
{
  VOID *pvTag;
  DRV_PBBUF_NFY_FCT pfNfyFct;
} DRV_PBBUF_NFY_FCT_T;

typedef struct _READ_BUFFER_TAG
{
  UINT32 u4DataType;
  BYTE *pcPlayBuffer;
  UINT32 u4DataSize;
  UINT32 u4PlayOffset;
  UINT32 u4PlaySize;
  UINT32 u4BufferHandle;
  UINT64 u8SrcOffset;
  BOOL   fgFollowedByIbc;
  DRV_InbandCmd *pIbc;
  UINT32 u4BufferSize;
} READ_BUFFER;


typedef struct _SENT_BUFFER_INFO_TAG
{
  UINT32 u4SentDataSize;
  UINT32 u4PbbufSize;
} SENT_BUFFER_INFO;

/// This interface represents IID_IPBBUF interface.
/// This interface contains CPSA functions.
typedef struct _IPBBUF
{
  INT32 (*pi4GetReceiveBuffer)(
           void *pvTag,           ///< [IN] the object handle
           READ_BUFFER *prReadBuffer ///< [IN] 
           );
  INT32 (*pi4BufferReceived)(
           void *pvTag,
           READ_BUFFER *prReadBuffer
           );
  INT32 (*pi4CancelReadyToGetReadBuffer)(
           void *pvTag
           );
  INT32 (*pi4SubscribeDrvCb)(
           void *pvTag,
           DRV_PBBUF_NFY_FCT_T *prDrvNfyFctInfo
           );
  INT32 (*pi4UnsubscribeDrvCb)(
           void *pvTag
           );
  INT32 (*pi4SentBufferInfo)(
           void *pvTag,
           SENT_BUFFER_INFO *prSentBufferInfo
           );
} IPBBUF;


/// OLD direct interface
extern INT32 i4PBDrvGetReceiveBuffer(UINT16 u2CompId, READ_BUFFER *prReadBuffer);
///extern INT32 i4PBDrvCancelReadyToGetReadBuffer(UINT16 u2CompId);
extern INT32 i4PBDrvBufferReceived(UINT16 u2CompId, READ_BUFFER *prReadBuffer);
////extern INT32 i4PBDrvSubscribeDrvCb(UINT16 u2CompId, DRV_PBBUF_NFY_FCT_T *prDrvNfyFctInfo);
///extern INT32 i4PBDrvUnsubscribeDrvCb(UINT16 u2CompId);


#endif
