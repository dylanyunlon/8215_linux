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
#ifndef _DRV_AUDIN_IF_H_
#define _DRV_AUDIN_IF_H_

#include "x_typedef.h"
#include "drv_ibc.h"
#include "drv_if_pbbuf.h"
#include "x_audin.h"

/// This interface represents IID_IPBBUF interface.
/// This interface contains CPSA functions.
typedef struct _IAUDINBUF
{
  s32 (*pi4GetReceiveBuffer)(
           void *pvTag,           ///< [IN] the object handle
           READ_BUFFER *prReadBuffer ///< [IN] 
           );
  s32 (*pi4BufferReceived)(
           void *pvTag,
           READ_BUFFER *prReadBuffer
           );
  s32 (*pi4CancelReadyToGetReadBuffer)(
           void *pvTag
           );
  s32 (*pi4SubscribeDrvCb)(
           void *pvTag,
           DRV_PBBUF_NFY_FCT_T *prDrvNfyFctInfo
           );
  s32 (*pi4UnsubscribeDrvCb)(
           void *pvTag
           );
  s32 (*pi4SentBufferInfo)(
           void *pvTag,
           SENT_BUFFER_INFO *prSentBufferInfo
           );
#if CONFIG_DRV_HDMI_RX
  void (*vAudInParsingInfo)(
  	AUDIN_PARSING_INFO_T * prAudinPsringInfo
  	);
#endif
  BOOL (*fgAudInIsRAW)(
           void        
           );
} IAUDINBUF;

/*
/// OLD direct interface
extern s32 i4AudinDrvGetReceiveBuffer(UINT16 u2CompId, READ_BUFFER *prReadBuffer);
extern s32 i4AudinDrvCancelReadyToGetReadBuffer(UINT16 u2CompId);
extern s32 i4AudinDrvBufferReceived(UINT16 u2CompId, READ_BUFFER *prReadBuffer);
extern s32 i4AudinDrvSubscribeDrvCb(UINT16 u2CompId, DRV_PBBUF_NFY_FCT_T *prDrvNfyFctInfo);
extern s32 i4AudinDrvUnsubscribeDrvCb(UINT16 u2CompId);
*/



#endif
