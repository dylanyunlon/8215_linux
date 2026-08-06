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


#ifndef __USB_AUD_H__
#define __USB_AUD_H__

#include "drv_aud.h"
#include "drv_if_pbbuf.h"


/*******************************************************************************
**  Function     : CreateUsbAudInstance
**  descriptions : Invoked by ipod client driver when ipod client driver recieved IPOD
**                 connection notify
**  parameters   : pAudDevice           [IN]Audio Device structure
**  return       : pUsbAud              the instance of USB AUDIO DRIVER
*******************************************************************************/
u32 CreateUsbAudInstance(void *pAudDevice);


/*******************************************************************************
**  Function     : DestroyUsbAudInstance
**  descriptions : Invoked by ipod client driver when ipod client driver recieved IPOD
**                 disconnection notify
**  parameters   : pUsbAudInst           [IN] USB Audio Driver structure
**  return       : None
*******************************************************************************/
void DestroyUsbAudInstance(u32 h_obj);


/*******************************************************************************
**  Function     : SwitchUsbAud
**  descriptions : Invoked by Audio In driver when switch input to USB Audio Driver
**  parameters   : h_obj     [IN] USB Audio Handle
**  return       : TRUE            Switch Successful
**                 FALSE           Switch Failed
*******************************************************************************/
void SwitchUsbAud(u32 h_obj);


/*******************************************************************************
**  Function     : fgSetUsbAudOnOff
**  descriptions : Invoked by Audio In driver when switch input to USB Audio Driver
**  parameters   : fgOn            [IN]Start/stop USB Audio Driver
**  return       : TRUE            Successful
                   FALSE           Failed
*******************************************************************************/
bool fgSetUsbAudOnOff(bool fgOn);


/*******************************************************************************
**  Function     : u1UsbAudGetChannelNum
**  descriptions : Invoked by Audio In driver to Get audio channel number
**  parameters   : None
**  return       : UINT8    The number of channel
*******************************************************************************/
u8 u1UsbAudGetChannelNum(void);


/*******************************************************************************
**  Function     : u1UsbAudGetCodec
**  descriptions : Invoked by Audio In driver to Get audio codec
**  parameters   : None
**  return       : AUD_DRV_FMT_T    The codec of current audio stream
*******************************************************************************/
u8 u1UsbAudGetCodec(void);


/*******************************************************************************
**  Function     : u1UsbAudGetSampleRate
**  descriptions : Invoked by Audio In driver to Get Sample Rate
**  parameters   : None
**  return       : 
*******************************************************************************/
u8 u1UsbAudGetSampleRate(void);


//Interface for Splitter
// extern PBBuf
/*******************************************************************************
**  Function     : i4UsbAudIFGetReceiveBuffer
**  descriptions : Provide a read buffer for demux/parser's request, PBBUF removes
**                 the slot from the sent linked list
**  parameters   : pvTag                [IN]
**                 prReadBuffer         [OUT] the READ_BUFFER pointer
**  return       : S_PBBUF_OK      A SENT slot is available for reading request
**                 E_PBBUF_BUSY    No SENT slot can be available
*******************************************************************************/
extern s32 i4UsbAudIFGetReceiveBuffer(void *pvTag, READ_BUFFER *prReadBuffer);


/*******************************************************************************
**  Function     : i4UsbAudIFBufferReceived
**  descriptions : Finish reading the read buffer, PBBUF  Update Read Pointer
**  parameters   : pvTag                [IN]
**                 prReadBuffer         [OUT] the READ_BUFFER pointer
**  return       : S_PBBUF_OK      A SENT slot is available for reading request
**                 E_PBBUF_BUSY    No SENT slot can be available
*******************************************************************************/
s32 i4UsbAudIFBufferReceived(void *pvTag, READ_BUFFER *prReadBuffer);


/*******************************************************************************
**  Function     : i4UsbAudIFCancelReadyToGetReadBuffer
**  descriptions : 
**  parameters   : pvTag                [IN]
**  return       : 
*******************************************************************************/
s32 i4UsbAudIFCancelReadyToGetReadBuffer(void *pvTag);


/*******************************************************************************
**  Function     : i4UsbAudIFSubscribeDrvCb
**  descriptions : Parser register callback funtion to multiple line in module
**  parameters   : pvTag                [IN]
**                 prReadBuffer         [OUT] the READ_BUFFER pointer
**  return       : S_PBBUF_OK      Always return ok
*******************************************************************************/
s32 i4UsbAudIFSubscribeDrvCb(void *pvTag, DRV_PBBUF_NFY_FCT_T *prDrvNfyFctInfo);


/*******************************************************************************
**  Function     : i4UsbAudIFUnsubscribeDrvCb
**  descriptions : 
**  parameters   : pvTag                [IN]
**  return       : S_PBBUF_OK      Always return ok
*******************************************************************************/
s32 i4UsbAudIFUnsubscribeDrvCb(void *pvTag);


/*******************************************************************************
**  Function     : i4UsbAudSentBufferInfo
**  descriptions : 
**  parameters   : pvTag                [IN]
**                 prSentBufferInfo     []
**  return       : 
*******************************************************************************/
s32 i4UsbAudSentBufferInfo(void *pvTag, SENT_BUFFER_INFO *prSentBufferInfo);


/*******************************************************************************
**  Function     : fgUsbAudIsRAW
**  descriptions : CFA check Data Type is RAW or PCM
**  parameters   : pvTag                [IN]
**                 prSentBufferInfo     []
**  return       : 
*******************************************************************************/
bool fgUsbAudIsRAW(void);


/*******************************************************************************
**  Function     : vUsbAudReadyToGetReadBuffer
**  descriptions : If AudIn slot buffer is ready , callback this function to parser
**  parameters   : None
**  return       : None
*******************************************************************************/
void vUsbAudReadyToGetReadBuffer(void);


/*******************************************************************************
**  Function     : fgUsbAudSetSampleRate
**  descriptions : Set Sample Rate
**  parameters   : pUsbAudInfo         [IN] USB Audio Driver Information
**                 u4SampleRate        [IN] Sample Rate
**  return       : TRUE                Successful
**                 FASLE               Failed
*******************************************************************************/
bool fgUsbAudSetSampleRate(u32 h_obj, UINT32 u4SampleRate);


/*******************************************************************************
**  Function     : fgUsbAudSetStreamInteface (No Use)
**  descriptions : Set Stream Interface(When Configuration, has been set)
**  parameters   : pAudDev             [IN] Audio device Information
**  return       : TRUE                Successful
**                 FASLE               Failed
*******************************************************************************/
bool fgUsbAudSetStreamInteface(u32 h_obj);


/*******************************************************************************
**  Function     : fgUsbAudStartIso (For Test)
**  descriptions : Start ISO Transfer
**  parameters   : None
**  return       : None
*******************************************************************************/
bool fgUsbAudStartIso(void);


/*******************************************************************************
**  Function     : fgUsbAudStopIso (For Test)
**  descriptions : Stop ISO Transfer
**  parameters   : None
**  return       : None
*******************************************************************************/
bool fgUsbAudStopIso(void);


/*******************************************************************************
**  Function     : vUsbAudNfyLock (For Test)
**  descriptions : Notify Lock to LPE
**  parameters   : None
**  return       : None
*******************************************************************************/
void vUsbAudNfyLock(void);


/*******************************************************************************
**  Function     : vSetSendDataLen (For Test)
**  descriptions : Set data len Parse Get from buffer
**  parameters   : u4Len             [IN] length of data
**  return       : None
*******************************************************************************/
void vSetSendDataLen(u32 u4Len);


#endif ///End of __USB_AUD_H__
