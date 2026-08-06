/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of AutoChips Inc. (C) 2008 AutoChips Inc.
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
*  RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. AUTOCHIPS SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE AUTOCHIPS SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

/*******************************************************************************
*
* Filename:
* ---------
* file dvp_protocol.h
*
* Project:
* --------
*   CNB
*
* Description:
* ------------
*
*
* Author:
* -------
*
*
*------------------------------------------------------------------------------
*
*******************************************************************************/

#ifndef _DVP_PROTOCOL_H_
#define _DVP_PROTOCOL_H_

#include <linux/types.h>
#include "types.h"

#define MAX_DVP_SEND_DATA                      (300) /*FOR path*/
#define MAX_DVP2APACKET_PARSER_DATA            (3000)


/*AP protocol : uInstance + uCmd + uParam1 + uParam2 +
                uParam3 + uParam4 */
/*AP uart driver format : 0xFE + AP protocol +
    checksum(all data XOR except itself, 7-BYTE) */
struct AP2DVPPACKET_T {
    u8  uInstance;
    u8  uCmd;
    u8  uParam1;
    u8  uParam2;
    u8  uParam3;
    u8  uParam4;
};

struct AP2DVPCMD_T {
    u8  uCmd;
    u8  uParam1;
    u8  uParam2;
    u8  uParam3;
    u8  uParam4;
};


/*DVP protocol : uInstance + uLenH + uLenL +
    uCmd + uParam1 + uParam2 + auData[] */
/*DVP uart driver format : 0xFF + DVP protocol +
    checksum(all data XOR except itself, follow by uLenH/uLenL) */
struct DVP2APPACKET_T {
    u8  uInstance;
    u8  uLenH; /*length of auData[]*/
    u8  uLenL;
    u8  uCmd;
    u8  auData[MAX_DVP_SEND_DATA];
};

struct DVP2APPACKETPARSER_T {
    u8 uInstance;
    u8 uLenH; /*length of auData[]*/
    u8 uLenL;
    u8 uCmd;
    u8 auData[MAX_DVP2APACKET_PARSER_DATA];
};

bool InputProtocolData(u8 *puSrc, u16 u2Len);
bool ParseNextPacket(struct DVP2APPACKET_T *prPacket);
u32 GetProtocolDataLength(void);

#endif

