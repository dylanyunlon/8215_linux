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
#include "x_types.h"
#include "x_typedef.h"
#include "vdec_hal_if_common.h"
#include "vdec_hw_common.h"
#include "drv_config.h"
#include <base_regs.h>
#include "x_ckgen.h"
//#include "mm_debug.h"

extern BOOL _fgVdecSimDumpOpen; ///zhi0221,open vdec slim dump by cli cmd
static VDEC_HAL_COMMON_DEC_PRM_T _arHalDecParam;

#if 1
BOOL VDEC_CKGEN_AgtOnClk(e_CLK_T eAgt)
{
    UINT32 u4Tmp, u4Reset;
	unsigned long flags;
    
	//spin_lock_irqsave(&ac83xx_ckgen_lock, flags);
    if (eAgt < e_CLK_GFX) {    // CONFIG 0:
        u4Tmp = CKGEN_READ32(REG_RW_CLKGATE_CFG0);
        u4Reset = CKGEN_READ32(REG_RW_SYNC_RESET_CFG0);
        switch (eAgt) {
            case e_CLK_VDEC_FULL:
                u4Tmp = u4Tmp | (CLK_PDN_VDEC_FULL_MASK);
                u4Reset = u4Reset | (CLK_RESET_VDEC_FULL_MASK);
                break;
            default:
                return FALSE;
        }
        CKGEN_WRITE32(REG_RW_CLKGATE_CFG0, u4Tmp);
		CKGEN_WRITE32(REG_RW_SYNC_RESET_CFG0, u4Reset);
    }

    return TRUE;
}


BOOL VDEC_CKGEN_AgtSelClk(e_CLK_SEL_T eAgt, UINT32 u4Sel)
{
    UINT32 u4Tmp;
	unsigned long flags;
    
	//spin_lock_irqsave(&ac83xx_ckgen_lock,flags);
    if (eAgt < e_CLK_SEL_AUD) {    // CONFIG 1:
        u4Tmp = CKGEN_READ32(REG_RW_AP_REG1);
        switch (eAgt) {
            case e_CLK_SEL_USB_27M:
                u4Sel = (u4Sel << CLK_REG1_USB_27M_CLK_SEL_OFFSET) & CLK_REG1_USB_27M_CLK_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG1_USB_27M_CLK_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_OSD:
                u4Sel = (u4Sel << CLK_REG1_OSD_SEL_OFFSET) & CLK_REG1_OSD_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG1_OSD_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
	        case e_CLK_SEL_DRAM:
                u4Sel = (u4Sel << CLK_REG1_DRAM_SEL_OFFSET) & CLK_REG1_DRAM_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG1_DRAM_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
			case e_CLK_SEL_AXIM:
                u4Sel = (u4Sel << CLK_REG1_CLK_AXIM_SEL_OFFSET) & CLK_REG1_CLK_AXIM_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG1_CLK_AXIM_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_SPM:
                u4Sel = (u4Sel << CLK_REG1_SPM_SEL_OFFSET) & CLK_REG1_SPM_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG1_SPM_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_VDEC_SYS:
                u4Sel = (u4Sel << CLK_REG1_VDEC_SYS_SEL_OFFSET) & CLK_REG1_VDEC_SYS_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG1_VDEC_SYS_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_JPEG:
                u4Sel = (u4Sel << CLK_REG1_JPEG_SEL_OFFSET) & CLK_REG1_JPEG_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG1_JPEG_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_RSZ:
                u4Sel = (u4Sel << CLK_REG1_RSZ_SEL_OFFSET) & CLK_REG1_RSZ_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG1_RSZ_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_FLASH:
                u4Sel = (u4Sel << CLK_REG1_FLASH_SEL_OFFSET) & CLK_REG1_FLASH_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG1_FLASH_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_BCLK:
                u4Sel = (u4Sel << CLK_REG1_BCLK_SEL_OFFSET) & CLK_REG1_BCLK_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG1_BCLK_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            default:
                return FALSE;
        }
        CKGEN_WRITE32(REG_RW_AP_REG1, u4Tmp);
    }

    return TRUE;
}

BOOL VDEC_CKGEN_AgtOffClk(e_CLK_T eAgt)
{
    UINT32 u4Tmp, u4Reset;
	unsigned long flags;
    
	//spin_lock_irqsave(&ac83xx_ckgen_lock, flags);
    if (eAgt < e_CLK_GFX) {    // CONFIG 0:
        u4Tmp = CKGEN_READ32(REG_RW_CLKGATE_CFG0);
        u4Reset = CKGEN_READ32(REG_RW_SYNC_RESET_CFG0);
        switch (eAgt) {
            case e_CLK_VDEC_FULL:
                u4Tmp = u4Tmp & (~CLK_PDN_VDEC_FULL_MASK);
                u4Reset = u4Reset & (~CLK_RESET_VDEC_FULL_MASK);
                break;
            default:
                return FALSE;
        }
        CKGEN_WRITE32(REG_RW_SYNC_RESET_CFG0, u4Reset);
        CKGEN_WRITE32(REG_RW_CLKGATE_CFG0, u4Tmp);
    }

    return TRUE;
}

#endif



// **************************************************************************
// Function : INT32 i4VDEC_HAL_Common_Init(UINT32 u4ChipID);
// Description : Turns on video decoder HAL
// Parameter : u4ChipID
// Return      : >0: init OK.
//                  <0: init failed
// **************************************************************************
INT32 i4VDEC_HAL_Common_Init(UINT32 u4ChipID)
{
    VDEC_CKGEN_AgtOnClk(e_CLK_VDEC_FULL);
    VDEC_CKGEN_AgtSelClk(e_CLK_SEL_VDEC_SYS, CLK_REG1_VDEC_SYS_SEL_CLK_APLL1); // 240MHZ


    return HAL_HANDLE_OK;
}


// **************************************************************************
// Function : INT32 i4VDEC_HAL_Common_Uninit(void);
// Description : Turns off video decoder HAL
// Parameter : void
// Return      : >0: uninit OK.
//                  <0: uninit failed
// **************************************************************************
INT32 i4VDEC_HAL_Common_Uninit(void)
{
    VDEC_CKGEN_AgtOffClk(e_CLK_VDEC_FULL);
   // spm_powerdown_vdec();
    return HAL_HANDLE_OK;
}


// **************************************************************************************
// Function : void vVDec_HAL_COMMON_SetVLDFIFO(UINT32 u4BSID, UINT32 u4VDecID, UINT32 u4VFifoSa, UINT32 u4VFifoEa)
// Description : Set VFIFO start address and end address
// Parameter : u4VDecID : VLD ID
//             u4VFifoSa: VFIFO start adress, it should physical adress
//             u4VFifoEa: VFIFO end adress, it should physical adress
// Return    : None
// **************************************************************************************
void vVDec_HAL_COMMON_SetVLDFIFO(UINT32 u4BSID, UINT32 u4VDecID, UINT32 u4VFifoSa, UINT32 u4VFifoEa)
{
    vVDecWriteVLD(u4VDecID, RW_VLD_VSTART + (u4BSID << 10), u4VFifoSa >> 6);
    vVDecWriteVLD(u4VDecID, RW_VLD_VEND + (u4BSID << 10), u4VFifoEa >> 6);
}


// **************************************************************************************
// Function : void vVDec_HAL_COMMON_ResetHW(UINT32 u4VDecID)
// Description : Reset Video decode HW
// Parameter : u4VDecID : VLD ID
//                   u4VDecType: VDec type
// Return    : None
// **************************************************************************************
void vVDec_HAL_COMMON_ResetHW(UINT32 u4VDecID, UINT32 u4VDecType)
{
    UINT32 u4Cnt = 0;
    UINT32 u4VDecPDNCtrlSpec;

    UINT32 u4VDecSysClk;
    UINT32 u4VDecPDNCtrlModule1 = 0;
    UINT32 u4VDecPDNCtrlModule2 = 0;

    // HW issue, wait for read pointer stable
    u4Cnt = 50000;
    if (u4VDecReadVLD(u4VDecID, RO_VLD_SRAMCTRL) & PROCESS_FLAG)
    {
        while((!(u4VDecReadVLD(u4VDecID, RO_VLD_SRAMCTRL)&AA_FIT_TARGET_SCLK)) && (u4Cnt--));
    }


    vVDecWriteVLD(u4VDecID, WO_VLD_SRST, (0x1 |(0x1<<8)));

    switch(u4VDecType)
    {        
    case VDEC_H264:
     u4VDecPDNCtrlSpec = 0x1F7;
     u4VDecPDNCtrlModule1 = 0x53E20180;
     u4VDecPDNCtrlModule2 = 0x60;  
     break;
    case VDEC_UNKNOWN:
    default:
        //Do nothing
        u4VDecPDNCtrlSpec = 0x00000000;
    break;
    }
    switch(u4VDecType)
    {        
    case VDEC_H264:
        u4VDecSysClk = 0x00000002;
        break;
    case VDEC_UNKNOWN:
    default:
        //Do nothing
        u4VDecSysClk = 0x00000000;
        break;
    }
    
    vVDecWriteDV( u4VDecID, RW_PDN_CRTL_SPEC, u4VDecPDNCtrlSpec); 
    vVDecWriteDV( u4VDecID, RW_PDN_CRTL_MODULE1, u4VDecPDNCtrlModule1); 
    vVDecWriteDV( u4VDecID, RW_PDN_CRTL_MODULE2, u4VDecPDNCtrlModule2); 
	
    vVDecWriteDV( u4VDecID, RW_SYS_CLK_SEL,  u4VDecSysClk);

    vVDecWriteVLD(u4VDecID, WO_VLD_SRST, 0);
}


// **************************************************************************************
// Function : BOOL fgVDec_HAL_COMMON_IsVLDFetchOk(UINT32 u4BSID, UINT32 u4VDecID)
// Description : Check if VLD fetch is done
// Parameter : None
// Return    : TRUE: VLD fetch OK, FALSE: not OK
// **************************************************************************************
BOOL fgVDec_HAL_COMMON_IsVLDFetchOk(UINT32 u4BSID, UINT32 u4VDecID)
{
    if ((u4VDecReadVLD(u4VDecID, RO_VLD_FETCHOK + (u4BSID << 10)) & VLD_FETCH_OK) == 0)
    {
        return (FALSE);
    }
    return (TRUE);
}

// **************************************************************************************
// Function : BOOL vVDec_HAL_COMMON_WaitVldFetchOk(UINT32 u4BSID, UINT32 u4VDecID)
// Description : Wait if VLD fetch is done
// Parameter : None
// Return    : TRUE: VLD fetch OK, FALSE: not OK
// **************************************************************************************
BOOL vVDec_HAL_COMMON_WaitVldFetchOk(UINT32 u4BSID, UINT32 u4VDecID)
{
    UINT32 u4Cnt = 0;

    if(_fgVdecSimDumpOpen) ///zhi0221,open vdec slim dump by cli cmd
    {
        //MMLOG_TRACE(LOG_MOD_VDEC, TEXT("////wait(`VDEC_INI_FETCH_RDY == 1);\n"));
    }

    if (!fgVDec_HAL_COMMON_IsVLDFetchOk(u4BSID, u4VDecID))
    {
        u4Cnt = 0;
        while (!fgVDec_HAL_COMMON_IsVLDFetchOk(u4BSID, u4VDecID))
        {
            u4Cnt++;
            if (u4Cnt >= 0x1000)
            {
                return (FALSE);
            }
        }
    }

    return (TRUE);
}


// **************************************************************************************
// Function : UINT32 dVLDGetBitS(UINT32 u4BSID, UINT32 u4VDecID, UINT32 dShiftBit)
// Description : Get Bitstream from VLD barrel shifter
// Parameter : dShiftBit: Bits to shift (0-32)
// Return    : barrel shifter
// **************************************************************************************
UINT32 vVDec_HAL_COMMON_VLDGetBits(UINT32 u4BSID, UINT32 u4VDecID, UINT32 u4ShiftBit)
{
    UINT32 u4RegVal;

    u4RegVal = u4VDecReadVLD(u4VDecID, RO_VLD_BARL + (u4BSID << 10) + (u4ShiftBit << 2));

    return (u4RegVal);
}

