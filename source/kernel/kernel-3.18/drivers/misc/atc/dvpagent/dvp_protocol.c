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
* file protocol.c
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
#include "dvp_cmd.h"
#include "dvp_protocol.h"
#include "chip_ver.h"
#include "dvpcomhw.h"

u8 _bAPRXUartData[sizeof(struct DVP2APPACKETPARSER_T)];
static u16 g_u2DataLen_bak;
static u32 g_u4DataLen;

bool InputProtocolData_bak(u8 *puSrc, u16 u2Len)
{
    u16 u2Temp = 0;

    memset(_bAPRXUartData, 0, sizeof(_bAPRXUartData));
    for (u2Temp = 0; u2Temp < u2Len; u2Temp++)
        _bAPRXUartData[u2Temp] = puSrc[u2Temp];
    g_u2DataLen_bak += u2Len;

    return TRUE;
}

bool InputProtocolData(u8 *puSrc, u16 u2Len)
{
    u16 u2Temp = 0;
    if (!puSrc)
        return FALSE;
    if (puSrc[0] != 0xFF || puSrc[1] != 0xFE) {
        pr_debug("[dvp][drv] InputProtocolData Sync header error\r\n");
        return FALSE;
    }
    u2Len -= 2;
    puSrc += 2;

    memset(_bAPRXUartData, 0, sizeof(_bAPRXUartData));
    for (u2Temp = 0; u2Temp < u2Len; u2Temp++)
        _bAPRXUartData[u2Temp] = puSrc[u2Temp];
    g_u4DataLen = u2Len;

    return TRUE;
}

bool ParseNextPacket_bak(struct DVP2APPACKET_T *prPacket)
{
    u8 u1Ofst = 0;
    u16 u2Temp = 0;
    u16 u2Len = 0;

    if (g_u2DataLen_bak < 3) {
        pr_debug("[dvp][drv] g_u2DataLen_bak < 3\r\n");
        g_u2DataLen_bak = 0;
        return FALSE;
    }
    memset(prPacket, 0, sizeof(struct DVP2APPACKET_T));
    prPacket->uInstance = _bAPRXUartData[u1Ofst++];
    prPacket->uLenH = _bAPRXUartData[u1Ofst++];
    prPacket->uLenL = _bAPRXUartData[u1Ofst++];
    prPacket->uCmd = _bAPRXUartData[u1Ofst++];

    u2Len = (prPacket->uLenH<<8) | prPacket->uLenL;
    if (u2Len) {
        for (u2Temp = 0; u2Temp < u2Len; u2Temp++)
            prPacket->auData[u2Temp] = _bAPRXUartData[u1Ofst++];
    }
    if (g_u2DataLen_bak < u1Ofst)
        g_u2DataLen_bak = 0;
    else
        g_u2DataLen_bak -= u1Ofst;

    return TRUE;
}

bool  ParseNextPacket(struct DVP2APPACKET_T *prPacket)
{
    u32 u4Ofst = 0;
    u32 u4Temp = 0;
    u32 u4Len = 0;

    if (g_u4DataLen < 3) {
        g_u4DataLen = 0;
        return FALSE;
    }
    memset(prPacket, 0, sizeof(struct DVP2APPACKET_T));

    prPacket->uInstance = _bAPRXUartData[u4Ofst++];
    prPacket->uLenH = _bAPRXUartData[u4Ofst++];
    prPacket->uLenL = _bAPRXUartData[u4Ofst++];
    prPacket->uCmd = _bAPRXUartData[u4Ofst++];
    pr_debug("[dvp][drv] ParseNextPacket uInst: %02X, uLenH: %02X, uLenL: %02X, uCmd: %02X\r\n",
        prPacket->uInstance, prPacket->uLenH, prPacket->uLenL,
        prPacket->uCmd);

    u4Len = (prPacket->uLenH<<8) | prPacket->uLenL;
    DVP_ASSERT(u4Len <= sizeof(_bAPRXUartData));
    if (u4Len > MAX_DVP_SEND_DATA)
        return FALSE;
    if (u4Len) {
        for (u4Temp = 0; u4Temp < u4Len; u4Temp++)
            prPacket->auData[u4Temp] = _bAPRXUartData[u4Ofst++];
    }
    g_u4DataLen = 0;

    return TRUE;
}

u32 GetProtocolDataLength_bak(void)
{
    return g_u2DataLen_bak;
}

u32 GetProtocolDataLength(void)
{
    return g_u4DataLen;
}


