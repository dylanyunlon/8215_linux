/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * AutoChips Inc. (C) 2016. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */

#ifndef _ARM2_SHARE_MEMORY_
#define _ARM2_SHARE_MEMORY_

#include "x_typedef.h"

// ShareMemory Messages definition
#define SMEM_MSG_START					0x0001	// ShareMemory Start	
#define SMEM_MSG_END						0x0002	// ShareMemory End
#define SMEM_MSG_RESET					0x0003	// ShareMemory Reset
#define SMEM_MSG_SET_INFO				0x0004	// Set ShareMemory SA and Size
#define SMEM_MSG_SEND_DATA			0x0005	// Sending data to ARM1
#define SMEM_MSG_UPDATE_PTR			0x0006	// Update WPtr and RPtr to ARM1
#define SMEM_MSG_SEND_END				0x0007	// ARM1 Use this to notify ARM2, the transfer data has been wrote to file. 
#define SMEM_MSG_ERROR					0x0008  // Notify ARM1 that ARM2 occur error

// Share Memory Error Code
#define SMEM_ERR_NOT_INIT								0x00
#define SMEM_ERR_TRANSFER_ERR						0x01
#define SMEM_ERR_SIZE_ERR								0x02
#define SMEM_ERR_MP3_DATA_NOT_HANDLED		0x03

// ShareMemory Temp Messages definition
#define SMEM_MSG_RECV_MP3_DATA						0x0101	// ARM1 send mp3 data to ARM2
#define SMEM_MSG_MP3_DATA_HANDLED					0x0102	// MP3 Data has handled


// ShareMemory States definition
#define SMEM_STA_NOT_SET							0x00
#define SMEM_STA_START								0x01
#define SMEM_STA_IDEL									0x02
#define SMEM_STA_SEND_DATA						0x03
#define SMEM_STA_RECV_DATA						0x04
#define SMEM_STA_NORMAL								0x05
#define SMEM_STA_END									0x10

#define SMEM_MP3STA_IDEL							0x11
#define SMEM_MP3STA_RECV_MP3_DATA			0x12
#define SMEM_MP3STA_HANDLED_MP3_DATA	0x13



typedef struct
{
  UINT32 u4StartAddress;
  UINT32 u4EndAddress;
  UINT32 u4Size;
  UINT32 u4WPtr;
  UINT32 u4RPtr;
}SHARE_MEM_T, *PSHARE_MEM_T;

typedef struct
{
  UINT32 u4TransferSA_PA;
  UINT32 u4TransferSize;
  BOOL bLastTransfer;
}SHARE_MEM_TRANSFER_T, *PSHARE_MEM_TRANSFER_T;

#define SMEM_MSG_SET_INFO_SA_SIZE		0x00
#define SMEM_MSG_SET_INFO_WP_RP			0x01

#endif //_ARM2_SHARE_MEMORY_