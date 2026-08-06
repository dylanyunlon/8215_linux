/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#include "vdec_hw_common.h"
#include "x_typedef.h"
#include "x_hal_ic.h"


BOOL _fgVdecSimDumpOpen = FALSE; ///zhi0221,open vdec slim dump by cli cmd

void vVDecWriteVLD(UINT32 u4VDecID, UINT32 u4Addr, UINT32 u4Val)
{
   if (u4VDecID == 0)
    {
        vWriteReg(VLD_REG_OFFSET0 + u4Addr, u4Val);
    }
    else
    {
        vWriteReg(VLD_REG_OFFSET1 + u4Addr, u4Val);
    }
}

UINT32 u4VDecReadVLD(UINT32 u4VDecID, UINT32 u4Addr)
{
    UINT32 u4Val = 0;
    if (u4VDecID == 0)
    {
        u4Val = u4ReadReg(VLD_REG_OFFSET0 + u4Addr);
        return u4Val;
    }
    else
    {
        return (u4ReadReg(VLD_REG_OFFSET1 + u4Addr));
    }
}


void vVDecWriteAVCMV(UINT32 u4VDecID, UINT32 u4Addr, UINT32 u4Val)
{
     u4VDecID = 0;

    if (u4VDecID == 0)
    {
        vWriteReg(AVC_MV_REG_OFFSET0 + u4Addr, u4Val);
    }
    else
    {
        vWriteReg(AVC_MV_REG_OFFSET1 + u4Addr, u4Val);
    }
}

UINT32 u4VDecReadAVCMV(UINT32 u4VDecID, UINT32 u4Addr)
{
     u4VDecID = 0;

    if (u4VDecID == 0)
    {
        UINT32 u4Val = u4ReadReg(AVC_MV_REG_OFFSET0 + u4Addr); 
        return u4Val;
    }
    else
    {
        return (u4ReadReg(AVC_MV_REG_OFFSET1 + u4Addr));
    }
}


// **************************************************************************************
// Function : void vVDecWriteVLDTOP(UINT32 u4VDecID, UINT32 u4Addr, UINT32 u4Val)
// Description : Write value to VLD TOP register
// Parameter : u4VDecID : VLD ID
//                   u4Addr: the register adress, base is VLD_TOP_REG_OFFSET0/VLD_TOP_REG_OFFSET1
//                   u4Val: the value to set register
// Return    : None
// **************************************************************************************
void vVDecWriteVLDTOP(UINT32 u4VDecID, UINT32 u4Addr, UINT32 u4Val)
{
    u4VDecID = 0;

    if (u4VDecID == 0)
    {
        vWriteReg(VLD_TOP_REG_OFFSET0 + u4Addr, u4Val);
    }
    else
    {
        vWriteReg(VLD_TOP_REG_OFFSET1 + u4Addr, u4Val);
    }
}



// **************************************************************************************
// Function : UINT32 u4VDecReadVLDTOP(UINT32 u4VDecID, UINT32 u4Addr)
// Description : Write value to VLD TOP register
// Parameter : u4VDecID : VLD ID
//                   u4Addr: the register adress, base is VLD_TOP_REG_OFFSET0/VLD_TOP_REG_OFFSET1
// Return    : the value from register
// **************************************************************************************
UINT32 u4VDecReadVLDTOP(UINT32 u4VDecID, UINT32 u4Addr)
{
    UINT32 u4Val = 0;
    u4VDecID = 0;

    if (u4VDecID == 0)
    {
        u4Val = u4ReadReg(VLD_TOP_REG_OFFSET0 + u4Addr);
        return u4Val;
    }
    else
    {
        return (u4ReadReg(VLD_TOP_REG_OFFSET1 + u4Addr));
    }
}

// **************************************************************************************
// Function : void vVDecWriteMC(UINT32 u4VDecID, UINT32 u4Addr, UINT32 u4Val)
// Description : Write value to MC register
// Parameter : u4VDecID : VLD ID
//                   u4Addr: the register adress, base is MC_REG_OFFSET0/MC_REG_OFFSET1
//                   u4Val: the value to set register
// Return    : None
// **************************************************************************************
void vVDecWriteMC(UINT32 u4VDecID, UINT32 u4Addr, UINT32 u4Val)
{
    u4VDecID = 0;

    if (u4VDecID == 0)
    {
        vWriteReg(MC_REG_OFFSET0 + u4Addr, u4Val);
    }
    else
    {
        vWriteReg(MC_REG_OFFSET1 + u4Addr, u4Val);
    }
}

// **************************************************************************************
// Function : UINT32 u4VDecReadMC(UINT32 u4VDecID, UINT32 u4Addr)
// Description : Write value to MC register
// Parameter : u4VDecID : VLD ID
//                   u4Addr: the register adress, base is MC_REG_OFFSET0/MC_REG_OFFSET1
// Return    : the value from register
// **************************************************************************************
UINT32 u4VDecReadMC(UINT32 u4VDecID, UINT32 u4Addr)
{
    UINT32 u4Val = 0;
    u4VDecID = 0;

    if (u4VDecID == 0)
    {
        u4Val = (u4ReadReg(MC_REG_OFFSET0 + u4Addr));
        return u4Val;
    }
    else
    {
        u4Val = (u4ReadReg(MC_REG_OFFSET1 + u4Addr));
        return u4Val;
    }
}

// **************************************************************************************
// Function : void vVDecWriteCRC(UINT32 u4VDecID, UINT32 u4Addr, UINT32 u4Val)
// Description : Write value to CRC register
// Parameter : u4VDecID : VLD ID
//                   u4Addr: the register adress, base is VDEC_CRC_REG_OFFSET0/VDEC_CRC_REG_OFFSET1
//                   u4Val: the value to set register
// Return    : None
// **************************************************************************************
void vVDecWriteCRC(UINT32 u4VDecID, UINT32 u4Addr, UINT32 u4Val)
{
    u4VDecID = 0;

    if (u4VDecID == 0)
    {
        vWriteReg(VDEC_CRC_REG_OFFSET0 + u4Addr, u4Val);
    }
    else
    {
		vWriteReg(VDEC_CRC_REG_OFFSET0 + u4Addr, u4Val);
    }
}


// **************************************************************************************
// Function : UINT32 u4VDecReadCRC(UINT32 u4VDecID, UINT32 u4Addr)
// Description : Write value to CRC register
// Parameter : u4VDecID : VLD ID
//                   u4Addr: the register adress, base is VDEC_CRC_REG_OFFSET0
// Return    : the value from register
// **************************************************************************************
UINT32 u4VDecReadCRC(UINT32 u4VDecID, UINT32 u4Addr)
{
    UINT32 u4Val;
    u4VDecID = 0;

    if (u4VDecID == 0)
    {
        u4Val = (u4ReadReg(VDEC_CRC_REG_OFFSET0 + u4Addr));
        return u4Val;
    }
    else
    {
        u4Val = (u4ReadReg(VDEC_CRC_REG_OFFSET0 + u4Addr));
        return u4Val;
    }
}



// **************************************************************************************
// Function : void vVDecWriteDV(UINT32 u4VDecID, UINT32 u4Addr, UINT32 u4Val)
// Description : Write value to DV register
// Parameter : u4VDecID : VLD ID
//                   u4Addr: the register adress, base is DV_REG_OFFSET0/DV_REG_OFFSET1
//                   u4Val: the value to set register
// Return    : None
// **************************************************************************************
void vVDecWriteDV(UINT32 u4VDecID, UINT32 u4Addr, UINT32 u4Val)
{
    u4VDecID = 0;

    if (u4VDecID == 0)
    {
        vWriteReg(DV_REG_OFFSET0 + u4Addr, u4Val);
    }
    else
    {
        vWriteReg(DV_REG_OFFSET1 + u4Addr, u4Val);
    }
}



