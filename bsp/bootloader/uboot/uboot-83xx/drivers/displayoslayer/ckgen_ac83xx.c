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

//============================================================================
// Include files
//============================================================================
#include <linux/types.h>
//#include <stdarg.h>
#include "base_regs.h"
//#include "section.h"
#include "x_ckgen.h"
//#include "x_hal_ic.h"
//#include "x_printf.h"
//#include "x_debug.h"
//#include "drv_config.h"
//#include <linux/module.h>

//============================================================================
// Static function forward declarations
//============================================================================

#if 0 // to be deleted
static UINT32 _GetAPLL(void);
static UINT32 _GetARMPLL(void);
static UINT32 _GetDMPLL(void);
static UINT32 _GetSYSPLL1(void);
static UINT32 _GetSYSPLL2(void);
static UINT32 _GetUSBCK(void);

static BOOL _CalAPLL(UINT32 u4Clock);
static BOOL _CalARMPLL(UINT32 u4Clock);
static BOOL _CalDMPLL(UINT32 u4Clock);
static BOOL _CalSYSPLL1(UINT32 u4Clock);
static BOOL _CalSYSPLL2(UINT32 u4Clock);
static BOOL _CalMEMPLL(UINT32 u4Clock);

//============================================================================
// Static routines
//============================================================================
static UINT32 _GetAPLL(void)
{
    UINT32 u4Clock = 0;
    u4Clock =  294;  // 270

    return u4Clock;
}
#endif

extern UINT32 BSP_GetClock(SRC_CK_T eSource)
{
    UINT32 freq;
    UINT32 reg = 0;
    UINT32 regPwd = 0;
	UINT32 regPwdValue;
    UINT32 regValue;
    freq = 0;

    freq = CKGEN_PLLGP_FREF;

    switch (eSource) {
        case SRC_CK_APLL294:
		freq = CKGEN_PLLGP_SYSPLL_FREQ / (((CKGEN_READ32(REG_RW_ANA7_PLLGP_CFG8) >> 1)&0x7)+2);
		regPwd = 0;  // APLL294
		reg = REG_RW_ANA7_PLLGP_CFG7;
		break;

        case SRC_CK_ARM11PLL:
		regPwd = 0;  // ARMPLL
		reg = REG_RW_ANA7_PLLGP_CFG5;
		break;

        case SRC_CK_ARM9PLL:
		regPwd = 0;  // ARM9PLL
		reg = REG_RW_ANA7_PLLGP_CFG6;
		break;

        case SRC_CK_SYSPLL:
		regPwd = 0;  // SYSPLL1
		reg = REG_RW_ANA7_PLLGP_CFG0;
		break;

        case SRC_CK_APLL270:
		freq = CKGEN_PLLGP_SYSPLL_FREQ / (((CKGEN_READ32(REG_RW_ANA7_PLLGP_CFG8) >> 1)&0x7)+2);
		regPwd = 0;  // APLL270
		reg = REG_RW_ANA7_PLLGP_CFG11;
		break;

        case SRC_CK_APLL26:
		regPwd = 0;  // APLL26
		reg = REG_RW_ANA7_PLLGP_CFG21;
		freq = 1000000;
		break;

        case SRC_CK_MSDCPLL:
		regPwd = 0;  // MSDCPLL
		reg = REG_RW_ANA7_PLLGP_CFG27;
		break;

        case SRC_CK_VGAPLL:
		regPwd = 0;  // VGAPLL
		reg = REG_RW_ANA7_PLLGP_CFG28;
		break;

        case SRC_CK_HADDS2:
		regPwd = 0;  // HADDS2
		reg = REG_RW_ANA7_PLLGP_CFG24;
		break;

        case SRC_CK_MEMPLL:
		regPwd = 0;  // zplee
		reg = REG_RW_MEMPLL0;
		break;

        default:
            //Unknown source
            break;
    }

    if (0 == regPwd) return (freq);

    regPwdValue = CKGEN_READ32(regPwd);
    regValue = CKGEN_READ32(reg);

    if(eSource == SRC_CK_MEMPLL)
    {
      regValue &= 0xFFFF0000;
      regValue |= (regValue >> 31);
      regValue |= (((regValue >> 24)&0x7F) << 1);
      regValue |= (((regValue >> 22)&0x3) << 8);
      regValue |= (((regValue >> 20)&0x3) << 10);
      regValue |= (((regValue >> 18)&0x3) << 12);
      regValue |= (((regValue >> 16)&0x3) << 14);
    }


    if((regPwdValue & PLL_PWD) == 0)
    {

      freq = (freq >> PLL_GET_PREDIV(regValue));
      freq = (freq << PLL_GET_FBSEL(regValue));
      if(eSource != SRC_CK_MEMPLL)
      {
        freq = (freq >> PLL_GET_POSDIV(regValue));
      }

      freq = freq * (PLL_GET_FBDIV(regValue)+1);

    }

    return freq;
}

extern BOOL CKGEN_AgtOnClk(e_CLK_T eAgt)
{
    UINT32 u4Tmp, u4Reset;

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
    } else if (eAgt < e_CLK_AUDIO_B00) {    // CONFIG 1:
        u4Tmp = CKGEN_READ32(REG_RW_CLKGATE_CFG1);
        u4Reset = CKGEN_READ32(REG_RW_SYNC_RESET_CFG1);
        switch (eAgt) {
            case e_CLK_GFX:
                u4Tmp = u4Tmp | (CLK_PDN_GFX);
                u4Reset = u4Reset | (CLK_RESET_GFX);
                break;
            case e_CLK_DMARB:
                u4Tmp = u4Tmp | (CLK_PDN_DMARB);
                u4Reset = u4Reset | (CLK_RESET_DMARB);
                break;
            case e_CLK_PNG:
                u4Tmp = u4Tmp | (CLK_PDN_PNG);
                u4Reset = u4Reset | (CLK_RESET_PNG);
                break;
            case e_CLK_GIF:
                u4Tmp = u4Tmp | (CLK_PDN_GIF);
                u4Reset = u4Reset | (CLK_RESET_GIF);
                break;
            case e_CLK_IMG_RESZ:
                u4Tmp = u4Tmp | (CLK_PDN_IMG_RESZ);
                u4Reset = u4Reset | (CLK_RESET_IMG_RESZ);
                break;
            case e_CLK_OSD_RESZ:
                u4Tmp = u4Tmp | (CLK_PDN_OSD_RESZ);
                u4Reset = u4Reset | (CLK_RESET_OSD_RESZ);
                break;
            case e_CLK_JPGDEC:
                u4Tmp = u4Tmp | (CLK_PDN_JPGDEC);
                u4Reset = u4Reset | (CLK_RESET_JPGDEC);
                break;
            case e_CLK_DEMUX:
                u4Tmp = u4Tmp & (~CLK_PDN_DEMUX);
                u4Reset = u4Reset | (CLK_RESET_DEMUX);
                break;
            case e_CLK_DEMUX_TS0:
                u4Tmp = u4Tmp & (~CLK_PDN_DEMUX_TS0);
                u4Reset = u4Reset | (CLK_RESET_DEMUX);
                break;
            case e_CLK_DEMUX_TS1:
                u4Tmp = u4Tmp & (~CLK_PDN_DEMUX_TS1);
                u4Reset = u4Reset | (CLK_RESET_DEMUX);
                break;
            case e_CLK_DEMUX_27M:
                u4Tmp = u4Tmp & (~CLK_PDN_DEMUX_27M);
                u4Reset = u4Reset | (CLK_RESET_DEMUX);
                break;
            case e_CLK_NFI:
                u4Tmp = u4Tmp | (CLK_PDN_NFI);
                u4Reset = u4Reset | (CLK_RESET_NFI);
                break;
            case e_CLK_USB:
                u4Tmp = u4Tmp | (CLK_PDN_USB);
                u4Reset = u4Reset | (CLK_RESET_USB);
                break;
	    case e_CLK_IRT_DMA_WRAPPER:
                u4Tmp = u4Tmp | (CLK_PDN_IRT_DMA_WRAPPER);
                u4Reset = u4Reset | (CLK_RESET_IRT_DMA_WRAPPER);
                break;
	    case e_CLK_ARM9:
                u4Tmp = u4Tmp | (CLK_PDN_ARM9);
                //u4Reset = u4Reset | (CLK_RESET_ARM9);
                break;
            default:
                return FALSE;
        }
        CKGEN_WRITE32(REG_RW_CLKGATE_CFG1, u4Tmp);
		CKGEN_WRITE32(REG_RW_SYNC_RESET_CFG1, u4Reset);
    } else if (eAgt < e_CLK_MVDO) {    // CONFIG 2:
        u4Tmp = CKGEN_READ32(REG_RW_CLKGATE_CFG3);
        u4Reset = CKGEN_READ32(REG_RW_SYNC_RESET_CFG3);
        switch (eAgt) {
            case e_CLK_AUDIO_B00:
                u4Tmp = u4Tmp | (CLK_PDN_AUDIO_B00|CLK_PDN_AUDIO_B01|CLK_PDN_AUDIO_B02);
                u4Reset = u4Reset | (CLK_RESET_AUDIO_B00|CLK_RESET_AUDIO_B01|CLK_RESET_AUDIO_B02);
                break;
            case e_CLK_AUDIO_B01:
                u4Tmp = u4Tmp | (CLK_PDN_AUDIO_B01);
                u4Reset = u4Reset | (CLK_RESET_AUDIO_B01);
                break;
            case e_CLK_AUDIO_B02:
                u4Tmp = u4Tmp | (CLK_PDN_AUDIO_B02);
                u4Reset = u4Reset | (CLK_RESET_AUDIO_B02);
                break;
            case e_CLK_AUDIO_B03:
                u4Tmp = u4Tmp | (CLK_PDN_AUDIO_B03);
                u4Reset = u4Reset | (CLK_RESET_AUDIO_B03);
                break;
            case e_CLK_AUDIO_B04:
                u4Tmp = u4Tmp | (CLK_PDN_AUDIO_B04);
                u4Reset = u4Reset | (CLK_RESET_AUDIO_B04);
                break;
            case e_CLK_AUDIO_B05:
                u4Tmp = u4Tmp | (CLK_PDN_AUDIO_B05);
                u4Reset = u4Reset | (CLK_RESET_AUDIO_B05);
                break;
            case e_CLK_AUDIO_B06:
                u4Tmp = u4Tmp | (CLK_PDN_AUDIO_B06);
                u4Reset = u4Reset | (CLK_RESET_AUDIO_B06);
                break;
            case e_CLK_AUDIO_B07:
                u4Tmp = u4Tmp | (CLK_PDN_AUDIO_B07);
                u4Reset = u4Reset | (CLK_RESET_AUDIO_B07);
                break;
            case e_CLK_AUDIO_B08:
                u4Tmp = u4Tmp | (CLK_PDN_AUDIO_B08);
                u4Reset = u4Reset | (CLK_RESET_AUDIO_B08);
                break;
            case e_CLK_AUDIO_B09:
                u4Tmp = u4Tmp | (CLK_PDN_AUDIO_B09);
                u4Reset = u4Reset | (CLK_RESET_AUDIO_B09);
                break;
            case e_CLK_AUDIO_B10:
                u4Tmp = u4Tmp | (CLK_PDN_AUDIO_B10);
                u4Reset = u4Reset | (CLK_RESET_AUDIO_B10);
                break;
	    case e_CLK_AUDIO_B11:
                u4Tmp = u4Tmp | (CLK_PDN_AUDIO_B11);
                u4Reset = u4Reset | (CLK_RESET_AUDIO_B11);
                break;
	    case e_CLK_AUDIO_B12:
                u4Tmp = u4Tmp | (CLK_PDN_AUDIO_B12);
                u4Reset = u4Reset | (CLK_RESET_AUDIO_B12);
                break;
	    case e_CLK_AUDIO_B13:
		u4Tmp = u4Tmp | (CLK_PDN_AUDIO_B13);
		u4Reset = u4Reset | (CLK_RESET_AUDIO_B13);
		break;
	    case e_CLK_AUDIO_B14:
		u4Tmp = u4Tmp | (CLK_PDN_AUDIO_B14);
		u4Reset = u4Reset | (CLK_RESET_AUDIO_B14);
		break;
            case e_CLK_RFI_TOP:
                u4Tmp = u4Tmp | (CLK_PDN_RFI_TOP);
                u4Reset = u4Reset | (CLK_RESET_RFI_TOP);
                break;
            case e_CLK_MSDC_0:
                u4Tmp = u4Tmp | (CLK_PDN_MSDC_0);
                u4Reset = u4Reset | (CLK_RESET_MSDC_0);
                break;
            case e_CLK_MSDC_1:
                u4Tmp = u4Tmp | (CLK_PDN_MSDC_1);
                u4Reset = u4Reset | (CLK_RESET_MSDC_1);
                break;
            case e_CLK_MSDC_2:
                u4Tmp = u4Tmp | (CLK_PDN_MSDC_2);
                u4Reset = u4Reset | (CLK_RESET_MSDC_2);
                break;
	    case e_CLK_MSDC_SW_0:
		//u4Tmp = u4Tmp | (CLK_PDN_MSDC_SW_0);
		u4Reset = u4Reset | (CLK_RESET_MSDC_SW_0);
		break;
	    case e_CLK_MSDC_SW_1:
		//u4Tmp = u4Tmp | (CLK_PDN_MSDC_SW_1);
		u4Reset = u4Reset | (CLK_RESET_MSDC_SW_1);
		break;
	    case e_CLK_MSDC_SW_2:
		//u4Tmp = u4Tmp | (CLK_PDN_MSDC_SW_2);
		u4Reset = u4Reset | (CLK_RESET_MSDC_SW_2);
		break;
            case e_CLK_SPI_MOTO1:
                u4Tmp = u4Tmp | (CLK_PDN_SPI_MOTO1);
                u4Reset = u4Reset | (CLK_RESET_SPI_MOTO1);
                break;
            case e_CLK_SPI_MOTO2:
                u4Tmp = u4Tmp | (CLK_PDN_SPI_MOTO2);
                u4Reset = u4Reset | (CLK_RESET_SPI_MOTO2);
                break;
            case e_CLK_PWM0:
                u4Tmp = u4Tmp | (CLK_PDN_PWM0);
                u4Reset = u4Reset | (CLK_RESET_PWM0);
                break;
            case e_CLK_PWM1:
                u4Tmp = u4Tmp | (CLK_PDN_PWM1);
                u4Reset = u4Reset | (CLK_RESET_PWM1);
                break;
            case e_CLK_PWM2:
                u4Tmp = u4Tmp | (CLK_PDN_PWM2);
                u4Reset = u4Reset | (CLK_RESET_PWM2);
                break;
            case e_CLK_PWM3:
                u4Tmp = u4Tmp | (CLK_PDN_PWM3);
                u4Reset = u4Reset | (CLK_RESET_PWM3);
                break;
            case e_CLK_SIFM0:
                u4Tmp = u4Tmp | (CLK_PDN_SIFM0);
                u4Reset = u4Reset | (CLK_RESET_SIFM0);
                break;
            case e_CLK_SIFM1:
                u4Tmp = u4Tmp | (CLK_PDN_SIFM1);
                u4Reset = u4Reset | (CLK_RESET_SIFM1);
                break;
            case e_CLK_SIFS0:
                u4Tmp = u4Tmp | (CLK_PDN_SIFS0);
                u4Reset = u4Reset | (CLK_RESET_SIFS0);
                break;
            case e_CLK_SIFS1:
                u4Tmp = u4Tmp | (CLK_PDN_SIFS1);
                u4Reset = u4Reset | (CLK_RESET_SIFS1);
                break;
            default:
                return FALSE;
        }
        CKGEN_WRITE32(REG_RW_CLKGATE_CFG3, u4Tmp);
        CKGEN_WRITE32(REG_RW_SYNC_RESET_CFG3, u4Reset);
    } else if (eAgt < e_CLK_LVDS) {    // CONFIG 3:
        u4Tmp = CKGEN_READ32(REG_RW_CLKGATE_CFG4);
        u4Reset = CKGEN_READ32(REG_RW_SYNC_RESET_CFG4);
        switch (eAgt) {
            case e_CLK_MVDO:
                u4Tmp = u4Tmp | (CLK_PDN_MVDO);
                u4Reset = u4Reset | (CLK_RESET_MVDO);
                break;
            case e_CLK_DGO:
                u4Tmp = u4Tmp | (CLK_PDN_DGO);
                u4Reset = u4Reset | (CLK_RESET_DGO);
                break;
            case e_CLK_DACTST:
                u4Tmp = u4Tmp | (CLK_PDN_DACTST);
                //u4Reset = u4Reset | (CLK_RESET_DACTST);
                break;
            case e_CLK_DVD_OSD:
                u4Tmp = u4Tmp | (CLK_PDN_DVD_OSD);
                u4Reset = u4Reset | (CLK_RESET_DVD_OSD);
                break;
            case e_CLK_GRA:
                u4Tmp = u4Tmp | (CLK_PDN_GRA);
                u4Reset = u4Reset | (CLK_RESET_GRA);
                break;
            case e_CLK_BIM:
                u4Tmp = u4Tmp | (CLK_PDN_BIM);
                u4Reset = u4Reset | (CLK_RESET_BIM);
                break;
            case e_CLK_TURBO32:
                u4Tmp = u4Tmp | (CLK_PDN_TURBO32);
                u4Reset = u4Reset | (CLK_RESET_TURBO32);
                break;
            case e_CLK_VDEC:
                u4Tmp = u4Tmp | (CLK_PDN_VDEC);
                u4Reset = u4Reset | (CLK_RESET_VDEC);
                break;
            case e_CLK_PARSER:
                u4Tmp = u4Tmp | (CLK_PDN_PARSER);
                u4Reset = u4Reset | (CLK_RESET_PARSER);
                break;
            case e_CLK_RAMBUF:
                u4Tmp = u4Tmp | (CLK_PDN_RAMBUF);
                u4Reset = u4Reset | (CLK_RESET_RAMBUF);
                break;
            case e_CLK_PT110:
                u4Tmp = u4Tmp | (CLK_PDN_PT110);
                u4Reset = u4Reset | (CLK_RESET_PT110);
                break;
            case e_CLK_RS232:
                u4Tmp = u4Tmp | (CLK_PDN_RS232);
                u4Reset = u4Reset | (CLK_RESET_RS232);
                break;
            case e_CLK_CDDVD:
                u4Tmp = u4Tmp | (CLK_PDN_CDDVD);
                u4Reset = u4Reset | (CLK_RESET_CDDVD);
                break;
            case e_CLK_AUDIO:
                u4Tmp = u4Tmp | (CLK_PDN_AUDIO);
                u4Reset = u4Reset | (CLK_RESET_AUDIO);
                break;
            case e_CLK_SERVO_MISC:
                u4Tmp = u4Tmp | (CLK_PDN_SERVO_MISC);
                u4Reset = u4Reset | (CLK_RESET_SERVO_MISC);
                break;
	    case e_CLK_RAMBUF_APCTRL_TBUS3:
                u4Tmp = u4Tmp | (CLK_PDN_RAMBUF_APCTRL_TBUS3);
                u4Reset = u4Reset & (~CLK_RESET_RAMBUF_APCTRL_TBUS3);
                break;
           case e_CLK_RAMBUF_APCTRL_TBUS4:
                u4Tmp = u4Tmp | (CLK_PDN_RAMBUF_APCTRL_TBUS4);
                u4Reset = u4Reset & (~CLK_RESET_RAMBUF_APCTRL_TBUS4);
                break;
	   case e_CLK_RAMBUF_APCTRL_TBUS5:
                u4Tmp = u4Tmp | (CLK_PDN_RAMBUF_APCTRL_TBUS5);
                u4Reset = u4Reset & (~CLK_RESET_RAMBUF_APCTRL_TBUS5);
                break;
	   case e_CLK_MFG_TOP_PWR_WRAP:
		u4Tmp = u4Tmp & (~CLK_PDN_MFG_TOP_PWR_WRAP);
		u4Reset = u4Reset | (CLK_RESET_MFG_TOP_PWR_WRAP);
		break;
	   default:
                return FALSE;
        }
        CKGEN_WRITE32(REG_RW_CLKGATE_CFG4, u4Tmp);
		CKGEN_WRITE32(REG_RW_SYNC_RESET_CFG4, u4Reset);
    } else if (eAgt < e_CLK_SCLER) {    // CONFIG 3:
        u4Tmp = CKGEN_READ32(REG_RW_CLKGATE_CFG5);
        u4Reset = CKGEN_READ32(REG_RW_SYNC_RESET_CFG5);
        switch (eAgt) {
            case e_CLK_LVDS:
                u4Tmp = u4Tmp | (CLK_PDN_CLK_LVDS);
                u4Reset = u4Reset & (~CLK_RESET_CLK_LVDS);
                break;
	    case e_CLK_TP_TOP0:
                u4Tmp = u4Tmp | (CLK_PDN_CLK_TP_TOP0);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
                break;
	    case e_CLK_TP_TOP1:
                u4Tmp = u4Tmp | (CLK_PDN_CLK_TP_TOP1);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
                break;
	    case e_CLK_TP_TOP2:
                u4Tmp = u4Tmp  | (CLK_PDN_CLK_TP_TOP2);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
                break;
            case e_CLK_RFI_TOP1:
                u4Tmp = u4Tmp | (CLK_PDN_CLK_RFI_TOP1);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
                break;
	    case e_CLK_RFI_TOP2:
                u4Tmp = u4Tmp | (CLK_PDN_CLK_RFI_TOP2);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
                break;
	    case e_CLK_RFI_TOP3:
                u4Tmp = u4Tmp | (CLK_PDN_CLK_RFI_TOP3);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
                break;
            case e_CLK_RFI_TOP4:
                u4Tmp = u4Tmp | (CLK_PDN_CLK_RFI_TOP4);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
                break;
	    case e_CLK_RFI_TOP5:
		u4Tmp = u4Tmp | (CLK_PDN_CLK_RFI_TOP5);
		//u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
		break;
	    case e_CLK_RFI_TOP6:
                u4Tmp = u4Tmp | (CLK_PDN_CLK_RFI_TOP6);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
                break;
            default:
                return FALSE;
        }
        CKGEN_WRITE32(REG_RW_CLKGATE_CFG5, u4Tmp);
		CKGEN_WRITE32(REG_RW_SYNC_RESET_CFG5, u4Reset);
    } else if (eAgt < e_CLK_MAX) {    // CONFIG 6:
        u4Tmp = CKGEN_READ32(REG_RW_CLKGATE_CFG6);
        u4Reset = CKGEN_READ32(REG_RW_SYNC_RESET_CFG6);
        switch (eAgt) {
            case e_CLK_SCLER:
                u4Tmp = u4Tmp | (CLK_PDN_SCLER);
                u4Reset = u4Reset | (CLK_RESET_SCLER);
                break;
            case e_CLK_TVD1:
                u4Tmp = u4Tmp | (CLK_PDN_TVD1);
                u4Reset = u4Reset | (CLK_RESET_TVD);
                break;
            case e_CLK_TVD2:
                u4Tmp = u4Tmp | (CLK_PDN_TVD2);
                u4Reset = u4Reset | (CLK_RESET_TVD);
                break;
            case e_CLK_OSD:
                u4Tmp = u4Tmp | (CLK_PDN_OSD);
                u4Reset = u4Reset | (CLK_RESET_OSD);
                break;
            case e_CLK_OSD_R:
                u4Tmp = u4Tmp | (CLK_PDN_OSD_R);
                u4Reset = u4Reset | (CLK_RESET_OSD_R);
                break;
            case e_CLK_FPD:
                u4Tmp = u4Tmp | (CLK_PDN_FPD);
                u4Reset = u4Reset | (CLK_RESET_FPD);
                break;
            case e_CLK_FMT_VDO_F:
                u4Tmp = u4Tmp | (CLK_PDN_FMT_VDO_F);
                u4Reset = u4Reset | (CLK_RESET_FMT_VDO_F);
                break;
            case e_CLK_FMT_VDO_R:
                u4Tmp = u4Tmp | (CLK_PDN_FMT_VDO_R);
                u4Reset = u4Reset | (CLK_RESET_FMT_VDO_R);
                break;
            case e_CLK_WRITE_CHANEL:
                u4Tmp = u4Tmp | (CLK_PDN_WRITE_CHANEL);
                u4Reset = u4Reset | (CLK_RESET_WRITE_CHANEL);
                break;
            case e_CLK_FRAME_LOCK:
                u4Tmp = u4Tmp | (CLK_PDN_FRAME_LOCK);
                u4Reset = u4Reset | (CLK_RESET_FRAME_LOCK);
                break;
	case e_CLK_WRITE_CHANEL_2:
		u4Tmp = u4Tmp | (CLK_PDN_WRITE_CHANEL2);
		u4Reset = u4Reset | (CLK_RESET_WRITE_CHANEL2);
		break;
	case e_CLK_VGA:
		u4Tmp = u4Tmp | (CLK_PDN_VGA_EDID);
		u4Reset = u4Reset | (CLK_RESET_VGA_EDID);
		break;
	case e_CLK_YPBPR:
		u4Tmp = u4Tmp | (CLK_PDN_YPBPR_VGA);
		u4Reset = u4Reset | (CLK_RESET_YPBPR_VGA);
		break;
	case e_CLK_HDMI:
		u4Tmp = u4Tmp | (CLK_PDN_HDMI);
		u4Reset = u4Reset | (CLK_RESET_HDMI);
		break;
	case e_CLK_TVE:
		u4Tmp = u4Tmp | (CLK_PDN_TVE);
		u4Reset = u4Reset | (CLK_RESET_TVE);
		break;
	case e_CLK_DVD_MIX_2AP:
		u4Tmp = u4Tmp | (CLK_PDN_DVD_MIX_2AP);
		u4Reset = u4Reset | (CLK_RESET_DVD_MIX_2AP);
		break;
	case e_CLK_OSD_1:
		u4Tmp = u4Tmp | (CLK_PDN_OSD1);
		//u4Reset = u4Reset | (CLK_RESET_FRAME_LOCK);
		break;
	case e_CLK_OSD_2:
		u4Tmp = u4Tmp | (CLK_PDN_OSD2);
		//u4Reset = u4Reset | (CLK_RESET_FRAME_LOCK);
		break;
	case e_CLK_OSD_3:
		u4Tmp = u4Tmp | (CLK_PDN_OSD3);
		//u4Reset = u4Reset | (CLK_RESET_FRAME_LOCK);
		break;
	case e_CLK_OSD_4:
		u4Tmp = u4Tmp | (CLK_PDN_OSD4);
		//u4Reset = u4Reset | (CLK_RESET_FRAME_LOCK);
		break;
	case e_CLK_OSD_5:
		u4Tmp = u4Tmp | (CLK_PDN_OSD5);
		//u4Reset = u4Reset | (CLK_RESET_FRAME_LOCK);
		break;
	case e_CLK_OSD_R_2:
		u4Tmp = u4Tmp | (CLK_PDN_OSD_R2);
		//u4Reset = u4Reset | (CLK_RESET_FRAME_LOCK);
		break;
	case e_CLK_OSD_R_3:
		u4Tmp = u4Tmp | (CLK_PDN_OSD_R3);
		//u4Reset = u4Reset | (CLK_RESET_DVD_MIX_2AP);
		break;
	case e_CLK_SCLER_TG:
		u4Tmp = u4Tmp | (CLK_PDN_SCLER_TG);
		u4Reset = u4Reset | (CLK_RESET_SCLER_TG);
		break;
	case e_CLK_LCPROC_VDO:
		u4Tmp = u4Tmp | (CLK_PDN_LCPROC_VDO);
		u4Reset = u4Reset | (CLK_RESET_ICPROC_VDO);
		break;
            default:
                return FALSE;
        }
        CKGEN_WRITE32(REG_RW_CLKGATE_CFG6, u4Tmp);
		CKGEN_WRITE32(REG_RW_SYNC_RESET_CFG6, u4Reset);
    } else {
        return FALSE;
    }

    return TRUE;
}
//EXPORT_SYMBOL(CKGEN_AgtOnClk);

extern BOOL CKGEN_AgtOffClk(e_CLK_T eAgt)
{
    UINT32 u4Tmp, u4Reset;

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
    } else if (eAgt < e_CLK_AUDIO_B00) {    // CONFIG 1:
        u4Tmp = CKGEN_READ32(REG_RW_CLKGATE_CFG1);
        u4Reset = CKGEN_READ32(REG_RW_SYNC_RESET_CFG1);
        switch (eAgt) {
            case e_CLK_GFX:
                u4Tmp = u4Tmp & (~CLK_PDN_GFX);
                u4Reset = u4Reset & (~CLK_RESET_GFX);
                break;
            case e_CLK_DMARB:
                u4Tmp = u4Tmp & (~CLK_PDN_DMARB);
                u4Reset = u4Reset & (~CLK_RESET_DMARB);
                break;
            case e_CLK_PNG:
                u4Tmp = u4Tmp & (~CLK_PDN_PNG);
                u4Reset = u4Reset & (~CLK_RESET_PNG);
                break;
            case e_CLK_GIF:
                u4Tmp = u4Tmp & (~CLK_PDN_GIF);
                u4Reset = u4Reset & (~CLK_RESET_GIF);
                break;
            case e_CLK_IMG_RESZ:
                u4Tmp = u4Tmp & (~CLK_PDN_IMG_RESZ);
                u4Reset = u4Reset & (~CLK_RESET_IMG_RESZ);
                break;
            case e_CLK_OSD_RESZ:
                u4Tmp = u4Tmp & (~CLK_PDN_OSD_RESZ);
                u4Reset = u4Reset & (~CLK_RESET_OSD_RESZ);
                break;
            case e_CLK_JPGDEC:
                u4Tmp = u4Tmp & (~CLK_PDN_JPGDEC);
                u4Reset = u4Reset & (~CLK_RESET_JPGDEC);
                break;
            case e_CLK_DEMUX:
                u4Tmp = u4Tmp | (CLK_PDN_DEMUX);
                u4Reset = u4Reset & (~CLK_RESET_DEMUX);
                break;
            case e_CLK_DEMUX_TS0:
                u4Tmp = u4Tmp | (CLK_PDN_DEMUX_TS0);
                u4Reset = u4Reset & (~CLK_RESET_DEMUX);  //?
                break;
            case e_CLK_DEMUX_TS1:
                u4Tmp = u4Tmp | (CLK_PDN_DEMUX_TS1);
                u4Reset = u4Reset & (~CLK_RESET_DEMUX);  //?
                break;
            case e_CLK_DEMUX_27M:
                u4Tmp = u4Tmp | (CLK_PDN_DEMUX_27M);
                u4Reset = u4Reset & (~CLK_RESET_DEMUX);  //?
                break;
            case e_CLK_NFI:
                u4Tmp = u4Tmp | (CLK_PDN_NFI);
                u4Reset = u4Reset & (~CLK_RESET_NFI);  //?
                break;
            case e_CLK_USB:
                u4Tmp = u4Tmp & (~CLK_PDN_USB);
                u4Reset = u4Reset & (~CLK_RESET_USB);
                break;
            case e_CLK_IRT_DMA_WRAPPER:
                u4Tmp = u4Tmp & (~CLK_PDN_IRT_DMA_WRAPPER);
                u4Reset = u4Reset & (~CLK_RESET_IRT_DMA_WRAPPER);
                break;
	    case e_CLK_ARM9:
                u4Tmp = u4Tmp & (~CLK_PDN_ARM9);
                 break;
            default:
                return FALSE;
        }
        CKGEN_WRITE32(REG_RW_SYNC_RESET_CFG1, u4Reset);
        CKGEN_WRITE32(REG_RW_CLKGATE_CFG1, u4Tmp);
    } else if (eAgt < e_CLK_MVDO) {    // CONFIG 2:
        u4Tmp = CKGEN_READ32(REG_RW_CLKGATE_CFG3);
        u4Reset = CKGEN_READ32(REG_RW_SYNC_RESET_CFG3);
        switch (eAgt) {
            case e_CLK_AUDIO_B00:
                u4Tmp = u4Tmp & (~CLK_PDN_AUDIO_B00);
                u4Reset = u4Reset & (~CLK_RESET_AUDIO_B00);
                break;
            case e_CLK_AUDIO_B01:
                u4Tmp = u4Tmp & (~CLK_PDN_AUDIO_B01);
                u4Reset = u4Reset & (~CLK_RESET_AUDIO_B01);
                break;
            case e_CLK_AUDIO_B02:
                u4Tmp = u4Tmp & (~CLK_PDN_AUDIO_B02);
                u4Reset = u4Reset & (~CLK_RESET_AUDIO_B02);
                break;
            case e_CLK_AUDIO_B03:
                u4Tmp = u4Tmp & (~CLK_PDN_AUDIO_B03);
                u4Reset = u4Reset & (~CLK_RESET_AUDIO_B03);
                break;
            case e_CLK_AUDIO_B04:
                u4Tmp = u4Tmp & (~CLK_PDN_AUDIO_B04);
                u4Reset = u4Reset & (~CLK_RESET_AUDIO_B04);
                break;
            case e_CLK_AUDIO_B05:
                u4Tmp = u4Tmp & (~CLK_PDN_AUDIO_B05);
                u4Reset = u4Reset & (~CLK_RESET_AUDIO_B05);
                break;
            case e_CLK_AUDIO_B06:
                u4Tmp = u4Tmp & (~CLK_PDN_AUDIO_B06);
                u4Reset = u4Reset & (~CLK_RESET_AUDIO_B06);
                break;
            case e_CLK_AUDIO_B07:
                u4Tmp = u4Tmp & (~CLK_PDN_AUDIO_B07);
                u4Reset = u4Reset & (~CLK_RESET_AUDIO_B07);
                break;
            case e_CLK_AUDIO_B08:
                u4Tmp = u4Tmp & (~CLK_PDN_AUDIO_B08);
                u4Reset = u4Reset & (~CLK_RESET_AUDIO_B08);
                break;
            case e_CLK_AUDIO_B09:
                u4Tmp = u4Tmp & (~CLK_PDN_AUDIO_B09);
                u4Reset = u4Reset & (~CLK_RESET_AUDIO_B09);
                break;
            case e_CLK_AUDIO_B10:
                u4Tmp = u4Tmp & (~CLK_PDN_AUDIO_B10);
                u4Reset = u4Reset & (~CLK_RESET_AUDIO_B10);
                break;
	case e_CLK_AUDIO_B11:
		u4Tmp = u4Tmp & (~CLK_PDN_AUDIO_B11);
		u4Reset = u4Reset & (~CLK_RESET_AUDIO_B11);
		break;
	case e_CLK_AUDIO_B12:
		u4Tmp = u4Tmp & (~CLK_PDN_AUDIO_B12);
		u4Reset = u4Reset & (~CLK_RESET_AUDIO_B12);
		break;
	case e_CLK_AUDIO_B13:
		u4Tmp = u4Tmp & (~CLK_PDN_AUDIO_B13);
		u4Reset = u4Reset & (~CLK_RESET_AUDIO_B13);
		break;
	case e_CLK_AUDIO_B14:
		u4Tmp = u4Tmp & (~CLK_PDN_AUDIO_B14);
		u4Reset = u4Reset & (~CLK_RESET_AUDIO_B14);
		break;

            case e_CLK_RFI_TOP:
                u4Tmp = u4Tmp & (~CLK_PDN_RFI_TOP);
                u4Reset = u4Reset & (~CLK_RESET_RFI_TOP);
                break;
            case e_CLK_MSDC_0:
                u4Tmp = u4Tmp & (~CLK_PDN_MSDC_0);
                u4Reset = u4Reset & (~CLK_RESET_MSDC_0);
                break;
            case e_CLK_MSDC_1:
                u4Tmp = u4Tmp & (~CLK_PDN_MSDC_1);
                u4Reset = u4Reset & (~CLK_RESET_MSDC_1);
                break;
            case e_CLK_MSDC_2:
                u4Tmp = u4Tmp & (~CLK_PDN_MSDC_2);
                u4Reset = u4Reset & (~CLK_RESET_MSDC_2);
                break;
            case e_CLK_MSDC_SW_0:
		//u4Tmp = u4Tmp & (~CLK_PDN_MSDC_SW_0);
		u4Reset = u4Reset & (~CLK_RESET_MSDC_SW_0);
		break;
	    case e_CLK_MSDC_SW_1:
		//u4Tmp = u4Tmp & (~CLK_PDN_MSDC_SW_1);
		u4Reset = u4Reset & (~CLK_RESET_MSDC_SW_1);
		break;
	    case e_CLK_MSDC_SW_2:
		//u4Tmp = u4Tmp & (~CLK_PDN_MSDC_SW_2);
		u4Reset = u4Reset & (~CLK_RESET_MSDC_SW_2);
		break;
            case e_CLK_SPI_MOTO1:
                u4Tmp = u4Tmp & (~CLK_PDN_SPI_MOTO1);
                u4Reset = u4Reset & (~CLK_RESET_SPI_MOTO1);
                break;
            case e_CLK_SPI_MOTO2:
                u4Tmp = u4Tmp & (~CLK_PDN_SPI_MOTO2);
                u4Reset = u4Reset & (~CLK_RESET_SPI_MOTO2);
                break;
            case e_CLK_PWM0:
                u4Tmp = u4Tmp & (~CLK_PDN_PWM0);
                u4Reset = u4Reset & (~CLK_RESET_PWM0);
                break;
            case e_CLK_PWM1:
                u4Tmp = u4Tmp & (~CLK_PDN_PWM1);
                u4Reset = u4Reset & (~CLK_RESET_PWM1);
                break;
            case e_CLK_PWM2:
                u4Tmp = u4Tmp & (~CLK_PDN_PWM2);
                u4Reset = u4Reset & (~CLK_RESET_PWM2);
                break;
            case e_CLK_PWM3:
                u4Tmp = u4Tmp & (~CLK_PDN_PWM3);
                u4Reset = u4Reset & (~CLK_RESET_PWM3);
                break;
            case e_CLK_SIFM0:
                u4Tmp = u4Tmp & (~CLK_PDN_SIFM0);
                u4Reset = u4Reset & (~CLK_RESET_SIFM0);
                break;
            case e_CLK_SIFM1:
                u4Tmp = u4Tmp & (~CLK_PDN_SIFM1);
                u4Reset = u4Reset & (~CLK_RESET_SIFM1);
                break;
            case e_CLK_SIFS0:
                u4Tmp = u4Tmp & (~CLK_PDN_SIFS0);
                u4Reset = u4Reset & (~CLK_RESET_SIFS0);
                break;
            case e_CLK_SIFS1:
                u4Tmp = u4Tmp & (~CLK_PDN_SIFS1);
                u4Reset = u4Reset & (~CLK_RESET_SIFS1);
                break;
            default:
                return FALSE;
        }
        CKGEN_WRITE32(REG_RW_SYNC_RESET_CFG3, u4Reset);
        CKGEN_WRITE32(REG_RW_CLKGATE_CFG3, u4Tmp);
    } else if (eAgt < e_CLK_LVDS) {    // CONFIG 3:
        u4Tmp = CKGEN_READ32(REG_RW_CLKGATE_CFG4);
        u4Reset = CKGEN_READ32(REG_RW_SYNC_RESET_CFG4);
        switch (eAgt) {
            case e_CLK_MVDO:
                u4Tmp = u4Tmp & (~CLK_PDN_MVDO);
                u4Reset = u4Reset & (~CLK_RESET_MVDO);
                break;
            case e_CLK_DGO:
                u4Tmp = u4Tmp & (~CLK_PDN_DGO);
                u4Reset = u4Reset & (~CLK_RESET_DGO);
                break;
            case e_CLK_DACTST:
                u4Tmp = u4Tmp & (~CLK_PDN_DACTST);
                //u4Reset = u4Reset & (~CLK_RESET_DACTST);
                break;
            case e_CLK_DVD_OSD:
                u4Tmp = u4Tmp & (~CLK_PDN_DVD_OSD);
                u4Reset = u4Reset & (~CLK_RESET_DVD_OSD);
                break;
            case e_CLK_GRA:
                u4Tmp = u4Tmp & (~CLK_PDN_GRA);
                u4Reset = u4Reset & (~CLK_RESET_GRA);
                break;
            case e_CLK_BIM:
                u4Tmp = u4Tmp & (~CLK_PDN_BIM);
                u4Reset = u4Reset & (~CLK_RESET_BIM);
                break;
            case e_CLK_TURBO32:
                u4Tmp = u4Tmp & (~CLK_PDN_TURBO32);
                u4Reset = u4Reset & (~CLK_RESET_TURBO32);
                break;
            case e_CLK_VDEC:
                u4Tmp = u4Tmp & (~CLK_PDN_VDEC);
                u4Reset = u4Reset & (~CLK_RESET_VDEC);
                break;
            case e_CLK_PARSER:
                u4Tmp = u4Tmp & (~CLK_PDN_PARSER);
                u4Reset = u4Reset & (~CLK_RESET_PARSER);
                break;
            case e_CLK_RAMBUF:
                u4Tmp = u4Tmp & (~CLK_PDN_RAMBUF);
                u4Reset = u4Reset & (~CLK_RESET_RAMBUF);
                break;
            case e_CLK_PT110:
                u4Tmp = u4Tmp & (~CLK_PDN_PT110);
                u4Reset = u4Reset & (~CLK_RESET_PT110);
                break;
            case e_CLK_RS232:
                u4Tmp = u4Tmp & (~CLK_PDN_RS232);
                u4Reset = u4Reset & (~CLK_RESET_RS232);
                break;
            case e_CLK_CDDVD:
                u4Tmp = u4Tmp & (~CLK_PDN_CDDVD);
                u4Reset = u4Reset & (~CLK_RESET_CDDVD);
                break;
            case e_CLK_AUDIO:
                u4Tmp = u4Tmp & (~CLK_PDN_AUDIO);
                u4Reset = u4Reset & (~CLK_RESET_AUDIO);
                break;
            case e_CLK_SERVO_MISC:
                u4Tmp = u4Tmp & (~CLK_PDN_SERVO_MISC);
                u4Reset = u4Reset & (~CLK_RESET_SERVO_MISC);
                break;
            case e_CLK_RAMBUF_APCTRL_TBUS3:
                u4Tmp = u4Tmp & (~CLK_PDN_RAMBUF_APCTRL_TBUS3);
                u4Reset = u4Reset | (CLK_RESET_RAMBUF_APCTRL_TBUS3);
                break;
	    case e_CLK_RAMBUF_APCTRL_TBUS4:
                u4Tmp = u4Tmp & (~CLK_PDN_RAMBUF_APCTRL_TBUS4);
                u4Reset = u4Reset | (CLK_RESET_RAMBUF_APCTRL_TBUS4);
                break;
	    case e_CLK_RAMBUF_APCTRL_TBUS5:
                u4Tmp = u4Tmp & (~CLK_PDN_RAMBUF_APCTRL_TBUS5);
                u4Reset = u4Reset | (CLK_RESET_RAMBUF_APCTRL_TBUS5);
                break;
	    case e_CLK_MFG_TOP_PWR_WRAP:
		u4Tmp = u4Tmp | (CLK_PDN_MFG_TOP_PWR_WRAP);
		u4Reset = u4Reset & (~CLK_RESET_MFG_TOP_PWR_WRAP);
				break;
            default:
                return FALSE;
        }
        CKGEN_WRITE32(REG_RW_SYNC_RESET_CFG4, u4Reset);
        CKGEN_WRITE32(REG_RW_CLKGATE_CFG4, u4Tmp);
    } else if (eAgt < e_CLK_SCLER) {    // CONFIG 3:
        u4Tmp = CKGEN_READ32(REG_RW_CLKGATE_CFG5);
        u4Reset = CKGEN_READ32(REG_RW_SYNC_RESET_CFG5);
        switch (eAgt) {
            case e_CLK_LVDS:
                u4Tmp = u4Tmp & (~CLK_PDN_CLK_LVDS);
                u4Reset = u4Reset & (~CLK_RESET_CLK_LVDS);
                break;
	    case e_CLK_TP_TOP0:
                u4Tmp = u4Tmp & (~CLK_PDN_CLK_TP_TOP0);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
                break;
	    case e_CLK_TP_TOP1:
                u4Tmp = u4Tmp & (~CLK_PDN_CLK_TP_TOP1);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
                break;
	    case e_CLK_TP_TOP2:
                u4Tmp = u4Tmp  & (~CLK_PDN_CLK_TP_TOP2);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
                break;
            case e_CLK_RFI_TOP1:
                u4Tmp = u4Tmp & (~CLK_PDN_CLK_RFI_TOP1);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
                break;
	    case e_CLK_RFI_TOP2:
                u4Tmp = u4Tmp & (~CLK_PDN_CLK_RFI_TOP2);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
                break;
	    case e_CLK_RFI_TOP3:
                u4Tmp = u4Tmp & (~CLK_PDN_CLK_RFI_TOP3);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
                break;
	    case e_CLK_RFI_TOP4:
                u4Tmp = u4Tmp & (~CLK_PDN_CLK_RFI_TOP4);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
                break;
	    case e_CLK_RFI_TOP5:
		u4Tmp = u4Tmp & (~CLK_PDN_CLK_RFI_TOP5);
		//u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
		break;
	    case e_CLK_RFI_TOP6:
                u4Tmp = u4Tmp & (~CLK_PDN_CLK_RFI_TOP6);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
                break;
            default:
                return FALSE;
        }
        CKGEN_WRITE32(REG_RW_SYNC_RESET_CFG5, u4Reset);
        CKGEN_WRITE32(REG_RW_CLKGATE_CFG5, u4Tmp);
    } else if (eAgt < e_CLK_MAX) {    // CONFIG 6:
        u4Tmp = CKGEN_READ32(REG_RW_CLKGATE_CFG6);
        u4Reset = CKGEN_READ32(REG_RW_SYNC_RESET_CFG6);
        switch (eAgt) {
            case e_CLK_SCLER:
                u4Tmp = u4Tmp & (~CLK_PDN_SCLER);
                u4Reset = u4Reset & (~CLK_RESET_SCLER);
                break;
            case e_CLK_TVD1:
                u4Tmp = u4Tmp & (~CLK_PDN_TVD1);
                u4Reset = u4Reset & (~CLK_RESET_TVD);
                break;
            case e_CLK_TVD2:
                u4Tmp = u4Tmp & (~CLK_PDN_TVD2);
                u4Reset = u4Reset & (~CLK_RESET_TVD);  //?
                break;
            case e_CLK_OSD:
                u4Tmp = u4Tmp & (~CLK_PDN_OSD);
                u4Reset = u4Reset & (~CLK_RESET_OSD);
                break;
            case e_CLK_OSD_R:
                u4Tmp = u4Tmp & (~CLK_PDN_OSD_R);
                u4Reset = u4Reset & (~CLK_RESET_OSD_R);
                break;
            case e_CLK_FPD:
                u4Tmp = u4Tmp & (~CLK_PDN_FPD);
                u4Reset = u4Reset & (~CLK_RESET_FPD);
                break;
            case e_CLK_FMT_VDO_F:
                u4Tmp = u4Tmp & (~CLK_PDN_FMT_VDO_F);
                u4Reset = u4Reset & (~CLK_RESET_FMT_VDO_F);
                break;
            case e_CLK_FMT_VDO_R:
                u4Tmp = u4Tmp & (~CLK_PDN_FMT_VDO_R);
                u4Reset = u4Reset & (~CLK_RESET_FMT_VDO_R);
                break;
            case e_CLK_WRITE_CHANEL:
                u4Tmp = u4Tmp & (~CLK_PDN_WRITE_CHANEL);
                u4Reset = u4Reset & (~CLK_RESET_WRITE_CHANEL);
                break;
            case e_CLK_FRAME_LOCK:
                u4Tmp = u4Tmp & (~CLK_PDN_FRAME_LOCK);
                u4Reset = u4Reset & (~CLK_RESET_FRAME_LOCK);
                break;
	case e_CLK_WRITE_CHANEL_2:
		u4Tmp = u4Tmp & (~CLK_PDN_WRITE_CHANEL2);
		u4Reset = u4Reset & (~CLK_RESET_WRITE_CHANEL2);
		break;
	case e_CLK_VGA:
		u4Tmp = u4Tmp & (~CLK_PDN_VGA_EDID);
		u4Reset = u4Reset & (~CLK_RESET_VGA_EDID);
		break;
	case e_CLK_YPBPR:
		u4Tmp = u4Tmp & (~CLK_PDN_YPBPR_VGA);
		u4Reset = u4Reset & (~CLK_RESET_YPBPR_VGA);
		break;
	case e_CLK_HDMI:
		u4Tmp = u4Tmp & (~CLK_PDN_HDMI);
		u4Reset = u4Reset & (~CLK_RESET_HDMI);
		break;
	case e_CLK_TVE:
		u4Tmp = u4Tmp & (~CLK_PDN_TVE);
		u4Reset = u4Reset & (~CLK_RESET_TVE);
		break;
	case e_CLK_DVD_MIX_2AP:
		u4Tmp = u4Tmp & (~CLK_PDN_DVD_MIX_2AP);
		u4Reset = u4Reset & (~CLK_RESET_DVD_MIX_2AP);
		break;
	case e_CLK_OSD_1:
		u4Tmp = u4Tmp & (~CLK_PDN_OSD1);
		//u4Reset = u4Reset | (CLK_RESET_FRAME_LOCK);
		break;
	case e_CLK_OSD_2:
		u4Tmp = u4Tmp & (~CLK_PDN_OSD2);
		//u4Reset = u4Reset | (CLK_RESET_FRAME_LOCK);
		break;
	case e_CLK_OSD_3:
		u4Tmp = u4Tmp & (~CLK_PDN_OSD3);
		//u4Reset = u4Reset | (CLK_RESET_FRAME_LOCK);
		break;
	case e_CLK_OSD_4:
		u4Tmp = u4Tmp & (~CLK_PDN_OSD4);
		//u4Reset = u4Reset | (CLK_RESET_FRAME_LOCK);
		break;
	case e_CLK_OSD_5:
		u4Tmp = u4Tmp & (~CLK_PDN_OSD5);
		//u4Reset = u4Reset | (CLK_RESET_FRAME_LOCK);
		break;
	case e_CLK_OSD_R_2:
		u4Tmp = u4Tmp & (~CLK_PDN_OSD_R2);
		//u4Reset = u4Reset | (CLK_RESET_FRAME_LOCK);
		break;
	case e_CLK_OSD_R_3:
		u4Tmp = u4Tmp & (~CLK_PDN_OSD_R3);
		//u4Reset = u4Reset | (CLK_RESET_DVD_MIX_2AP);
		break;
	case e_CLK_SCLER_TG:
		u4Tmp = u4Tmp & (~CLK_PDN_SCLER_TG);
		u4Reset = u4Reset & (~CLK_RESET_SCLER_TG);
		break;
	case e_CLK_LCPROC_VDO:
		u4Tmp = u4Tmp & (~CLK_PDN_LCPROC_VDO);
		u4Reset = u4Reset & (~CLK_RESET_ICPROC_VDO);
		break;
            default:
                return FALSE;
        }
        CKGEN_WRITE32(REG_RW_SYNC_RESET_CFG6, u4Reset);
        CKGEN_WRITE32(REG_RW_CLKGATE_CFG6, u4Tmp);
    } else {
        return FALSE;
    }

    return TRUE;
}
// EXPORT_SYMBOL(CKGEN_AgtOffClk);

extern BOOL CKGEN_AgtSelClk(e_CLK_SEL_T eAgt, UINT32 u4Sel)
{
    UINT32 u4Tmp;

    if (eAgt < e_CLK_SEL_USB_27M) {    // CONFIG 0:
        u4Tmp = CKGEN_READ32(REG_RW_AP_REG0);
        switch (eAgt) {
            case e_CLK_SEL_RFI:
                u4Sel = (u4Sel << CLK_REG0_RFI_SEL_OFFSET) & CLK_REG0_RFI_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG0_RFI_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_TP_F32K:
                u4Sel = (u4Sel << CLK_REG0_TP_F32K_SEL_OFFSET) & CLK_REG0_TP_F32K_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG0_TP_F32K_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_TP:
                u4Sel = (u4Sel << CLK_REG0_TP_SEL_OFFSET) & CLK_REG0_TP_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG0_TP_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
			case e_CLK_SEL_VDO:  // zplee
                //u4Sel = (u4Sel << CLK_REG0_TP_SEL_OFFSET) & CLK_REG0_TP_SEL_MASK;
                //u4Tmp = u4Tmp & (~CLK_REG0_TP_SEL_MASK);
                //u4Tmp = u4Tmp | u4Sel;
                break;
			case e_CLK_SEL_RISC:  // zplee
                //u4Sel = (u4Sel << CLK_REG0_TP_SEL_OFFSET) & CLK_REG0_TP_SEL_MASK;
                //u4Tmp = u4Tmp & (~CLK_REG0_TP_SEL_MASK);
                //u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_DEMUX:
                u4Sel = (u4Sel << CLK_REG0_DEMUX_SEL_OFFSET) & CLK_REG0_DEMUX_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG0_DEMUX_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_DSP:
                u4Sel = (u4Sel << CLK_REG0_DSP_SEL_OFFSET) & CLK_REG0_DSP_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG0_DSP_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            default:
                return FALSE;
        }
        CKGEN_WRITE32(REG_RW_AP_REG0, u4Tmp);
    } else if (eAgt < e_CLK_SEL_AUD) {    // CONFIG 1:
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
    } else if (eAgt < e_CLK_SEL_DUTY) {    // CONFIG 2:
        u4Tmp = CKGEN_READ32(REG_RW_AP_REG2);
        switch (eAgt) {
            case e_CLK_SEL_AUD:
                u4Sel = (u4Sel << CLK_REG2_AUD_SEL_OFFSET) & CLK_REG2_AUD_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG2_AUD_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_G3D:
                u4Sel = (u4Sel << CLK_REG2_G3D_SEL_OFFSET) & CLK_REG2_G3D_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG2_G3D_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_FPD:
                u4Sel = (u4Sel << CLK_REG2_FPD_SEL_OFFSET) & CLK_REG2_FPD_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG2_FPD_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_SD11:
                u4Sel = (u4Sel << CLK_REG2_SD11_SEL_OFFSET) & CLK_REG2_SD11_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG2_SD11_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
		    case e_CLK_SEL_SD01:
                u4Sel = (u4Sel << CLK_REG2_SD01_SEL_OFFSET) & CLK_REG2_SD01_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG2_SD01_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
			case e_CLK_SEL_SD20:
                u4Sel = (u4Sel << CLK_REG2_SD20_SEL_OFFSET) & CLK_REG2_SD20_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG2_SD20_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_SD10:
                u4Sel = (u4Sel << CLK_REG2_SD10_SEL_OFFSET) & CLK_REG2_SD10_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG2_SD10_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_SD00:
                u4Sel = (u4Sel << CLK_REG2_SD00_SEL_OFFSET) & CLK_REG2_SD00_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG2_SD00_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_GRAPH:
                u4Sel = (u4Sel << CLK_REG2_GRAPH_SEL_OFFSET) & CLK_REG2_GRAPH_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG2_GRAPH_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            default:
                return FALSE;
        }
        CKGEN_WRITE32(REG_RW_AP_REG2, u4Tmp);
    } else if (eAgt < e_CLK_SEL_APLL_K8) {    // CONFIG 3:
        u4Tmp = CKGEN_READ32(REG_RW_AP_REG3);
        switch (eAgt) {
            case e_CLK_SEL_DUTY:
                u4Sel = (u4Sel << CLK_REG3_DUTY_SEL_OFFSET) & CLK_REG3_DUTY_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG3_DUTY_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_DEG:
                u4Sel = (u4Sel << CLK_REG3_DEG_SEL_OFFSET) & CLK_REG3_DEG_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG3_DEG_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_NF:
                u4Sel = (u4Sel << CLK_REG3_NF_SEL_OFFSET) & CLK_REG3_NF_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG3_NF_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_BT_MIC_AUD:
                u4Sel = (u4Sel << CLK_REG3_BT_MIC_AUD_SEL_OFFSET) & CLK_REG3_BT_MIC_AUD_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG3_BT_MIC_AUD_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_ARM_AUD:
                u4Sel = (u4Sel << CLK_REG3_ARM_AUD_SEL_OFFSET) & CLK_REG3_ARM_AUD_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG3_ARM_AUD_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_MPHON:
                u4Sel = (u4Sel << CLK_REG3_MPHON_SEL_OFFSET) & CLK_REG3_MPHON_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG3_MPHON_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_CPU2:
                u4Sel = (u4Sel << CLK_REG3_CPU2_SEL_OFFSET) & CLK_REG3_CPU2_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG3_CPU2_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_CPU1:
                u4Sel = (u4Sel << CLK_REG3_CPU1_SEL_OFFSET) & CLK_REG3_CPU1_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG3_CPU1_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_AUD2:
                u4Sel = (u4Sel << CLK_REG3_AUD2_SEL_OFFSET) & CLK_REG3_AUD2_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG3_AUD2_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            default:
                return FALSE;
        }
        CKGEN_WRITE32(REG_RW_AP_REG3, u4Tmp);
   } else if (eAgt < e_CLK_SEL_MLIN) {    // CONFIG 6:
        u4Tmp = CKGEN_READ32(REG_RW_AP_REG7);
        switch (eAgt) {
            case e_CLK_SEL_APLL_K8:
                u4Sel = (u4Sel << CLK_REG7_APLL_K8_SEL_OFFSET) & CLK_REG7_APLL_K8_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG7_APLL_K8_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_APLL_K7:
                u4Sel = (u4Sel << CLK_REG7_APLL_K7_SEL_OFFSET) & CLK_REG7_APLL_K7_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG7_APLL_K7_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_APLL_K6:
                u4Sel = (u4Sel << CLK_REG7_APLL_K6_SEL_OFFSET) & CLK_REG7_APLL_K6_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG7_APLL_K6_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_APLL_K5:
                u4Sel = (u4Sel << CLK_REG7_APLL_K5_SEL_OFFSET) & CLK_REG7_APLL_K5_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG7_APLL_K5_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_APLL_K4:
                u4Sel = (u4Sel << CLK_REG7_APLL_K4_SEL_OFFSET) & CLK_REG7_APLL_K4_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG7_APLL_K4_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_APLL_K3:
                u4Sel = (u4Sel << CLK_REG7_APLL_K3_SEL_OFFSET) & CLK_REG7_APLL_K3_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG7_APLL_K3_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_APLL_K2:
                u4Sel = (u4Sel << CLK_REG7_APLL_K2_SEL_OFFSET) & CLK_REG7_APLL_K2_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG7_APLL_K2_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_APLL_K1:
                u4Sel = (u4Sel << CLK_REG7_APLL_K1_SEL_OFFSET) & CLK_REG7_APLL_K1_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG7_APLL_K1_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            default:
                return FALSE;
        }
        CKGEN_WRITE32(REG_RW_AP_REG7, u4Tmp);
   } else if (eAgt < e_CLK_SEL_AUD_K5_TST) {    // CONFIG 8:
		u4Tmp = CKGEN_READ32(REG_RW_AP_REG8);
        switch (eAgt) {
            case e_CLK_SEL_MLIN:
                u4Sel = (u4Sel << CLK_REG8_MLIN_SEL_OFFSET) & CLK_REG8_MLIN_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG8_MLIN_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_AUD_MPH:
                u4Sel = (u4Sel << CLK_REG8_AUD_MPH_SEL_OFFSET) & CLK_REG8_AUD_MPH_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG8_AUD_MPH_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_MLIN2:
                u4Sel = (u4Sel << CLK_REG8_MLIN2_SEL_OFFSET) & CLK_REG8_MLIN2_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG8_MLIN2_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_AUD_PWM:
                u4Sel = (u4Sel << CLK_REG8_AUD_PWM_SEL_OFFSET) & CLK_REG8_AUD_PWM_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG8_AUD_PWM_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_AUD_ADC:
                u4Sel = (u4Sel << CLK_REG8_AUD_ADC_SEL_OFFSET) & CLK_REG8_AUD_ADC_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG8_AUD_ADC_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_PLL_TEST:
                u4Sel = (u4Sel << CLK_REG8_PLL_TEST_SEL_OFFSET) & CLK_REG8_PLL_TEST_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG8_PLL_TEST_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_APLL_A3:
                u4Sel = (u4Sel << CLK_REG8_APLL_A3_SEL_OFFSET) & CLK_REG8_APLL_A3_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG8_APLL_A3_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_APLL_A2:
                u4Sel = (u4Sel << CLK_REG8_APLL_A2_SEL_OFFSET) & CLK_REG8_APLL_A2_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG8_APLL_A2_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_APLL_A1:
                u4Sel = (u4Sel << CLK_REG8_APLL_A1_SEL_OFFSET) & CLK_REG8_APLL_A1_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG8_APLL_A1_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_APLL_K14:
                u4Sel = (u4Sel << CLK_REG8_APLL_K14_SEL_OFFSET) & CLK_REG8_APLL_K14_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG8_APLL_K14_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
	    case e_CLK_SEL_APLL_K13:
                u4Sel = (u4Sel << CLK_REG8_APLL_K13_SEL_OFFSET) & CLK_REG8_APLL_K13_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG8_APLL_K13_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
	case e_CLK_SEL_APLL_K12:
                u4Sel = (u4Sel << CLK_REG8_APLL_K12_SEL_OFFSET) & CLK_REG8_APLL_K12_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG8_APLL_K12_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
	case e_CLK_SEL_APLL_K11:
                u4Sel = (u4Sel << CLK_REG8_APLL_K11_SEL_OFFSET) & CLK_REG8_APLL_K11_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG8_APLL_K11_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
	case e_CLK_SEL_APLL_K10:
                u4Sel = (u4Sel << CLK_REG8_APLL_K10_SEL_OFFSET) & CLK_REG8_APLL_K10_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG8_APLL_K10_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
	case e_CLK_SEL_APLL_K9:
                u4Sel = (u4Sel << CLK_REG8_APLL_K9_SEL_OFFSET) & CLK_REG8_APLL_K9_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG8_APLL_K9_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            default:
                return FALSE;
        }
        CKGEN_WRITE32(REG_RW_AP_REG8, u4Tmp);
  } else if (eAgt < e_CLK_SEL_SIFS1) {    // CONFIG 9:
        u4Tmp = CKGEN_READ32(REG_RW_AP_REG9);
        switch (eAgt) {
            case e_CLK_SEL_AUD_K5_TST:
                u4Sel = (u4Sel << CLK_REG9_AUD_K5_TST_SEL_OFFSET) & CLK_REG9_AUD_K5_TST_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG9_AUD_K5_TST_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_AUD_A3_TST:
                u4Sel = (u4Sel << CLK_REG9_AUD_A3_TST_SEL_OFFSET) & CLK_REG9_AUD_A3_TST_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG9_AUD_A3_TST_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_AUD_A2_TST:
                u4Sel = (u4Sel << CLK_REG9_AUD_A2_TST_SEL_OFFSET) & CLK_REG9_AUD_A2_TST_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG9_AUD_A2_TST_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_AUD_A1_TST:
                u4Sel = (u4Sel << CLK_REG9_AUD_A1_TST_SEL_OFFSET) & CLK_REG9_AUD_A1_TST_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG9_AUD_A1_TST_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_MLIN_TST:
                u4Sel = (u4Sel << CLK_REG9_MLIN_TST_SEL_OFFSET) & CLK_REG9_MLIN_TST_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG9_MLIN_TST_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_DA_APLL1CK:
                u4Sel = (u4Sel << CLK_REG9_DA_APLL1CK_SEL_OFFSET) & CLK_REG9_DA_APLL1CK_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG9_DA_APLL1CK_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_DA_APLLCK:
                u4Sel = (u4Sel << CLK_REG9_DA_APLLCK_SEL_OFFSET) & CLK_REG9_DA_APLLCK_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG9_DA_APLLCK_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_MCLK_D2:
                u4Sel = (u4Sel << CLK_REG9_MCLK_D2_SEL_OFFSET) & CLK_REG9_MCLK_D2_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG9_MCLK_D2_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_SRAMIF:
                u4Sel = (u4Sel << CLK_REG9_CLK_SRAMIF_SEL_OFFSET) & CLK_REG9_CLK_SRAMIF_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG9_CLK_SRAMIF_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_TVD_MBIST:
                u4Sel = (u4Sel << CLK_REG9_TVD_MBIST_SEL_OFFSET) & CLK_REG9_TVD_MBIST_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG9_TVD_MBIST_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_SPI_MOTO:
                u4Sel = (u4Sel << CLK_REG9_SPI_MOTO_SEL_OFFSET) & CLK_REG9_SPI_MOTO_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG9_SPI_MOTO_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_PNG:
                u4Sel = (u4Sel << CLK_REG9_PNG_SEL_OFFSET) & CLK_REG9_PNG_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG9_PNG_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_TS1:
                u4Sel = (u4Sel << CLK_REG9_TS1_SEL_OFFSET) & CLK_REG9_TS1_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG9_TS1_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_TS0:
                u4Sel = (u4Sel << CLK_REG9_TS0_SEL_OFFSET) & CLK_REG9_TS0_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG9_TS0_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            default:
                return FALSE;
        }
        CKGEN_WRITE32(REG_RW_AP_REG9, u4Tmp);
  } else if (eAgt < e_CLK_SEL_MAX) {     // CONFIG 10:
        u4Tmp = CKGEN_READ32(REG_RW_AP_REG10);
        switch (eAgt) {
            case e_CLK_SEL_SIFS1:
                u4Sel = (u4Sel << CLK_REG10_SIFS1_SEL_OFFSET) & CLK_REG10_SIFS1_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG10_SIFS1_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_SIFS0:
                u4Sel = (u4Sel << CLK_REG10_SIFS0_SEL_OFFSET) & CLK_REG10_SIFS0_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG10_SIFS0_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_SIFM1:
                u4Sel = (u4Sel << CLK_REG10_SIFM1_SEL_OFFSET) & CLK_REG10_SIFM1_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG10_SIFM1_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_SIFM0:
                u4Sel = (u4Sel << CLK_REG10_SIFM0_SEL_OFFSET) & CLK_REG10_SIFM0_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG10_SIFM0_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
			case e_CLK_SEL_PWM3:
                u4Sel = (u4Sel << CLK_REG10_PWM3_SEL_OFFSET) & CLK_REG10_PWM3_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG10_PWM3_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_PWM2:
                u4Sel = (u4Sel << CLK_REG10_PWM2_SEL_OFFSET) & CLK_REG10_PWM2_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG10_PWM2_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_PWM1:
                u4Sel = (u4Sel << CLK_REG10_PWM1_SEL_OFFSET) & CLK_REG10_PWM1_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG10_PWM1_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_PWM0:
                u4Sel = (u4Sel << CLK_REG10_PWM0_SEL_OFFSET) & CLK_REG10_PWM0_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG10_PWM0_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_APLL_26M:
                u4Sel = (u4Sel << CLK_REG10_APLL_26M_SEL_OFFSET) & CLK_REG10_APLL_26M_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG10_APLL_26M_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_TWDS:
                u4Sel = (u4Sel << CLK_REG10_TWDS_SEL_OFFSET) & CLK_REG10_TWDS_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG10_TWDS_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            case e_CLK_SEL_LVDS:
                u4Sel = (u4Sel << CLK_REG10_LVDS_SEL_OFFSET) & CLK_REG10_LVDS_SEL_MASK;
                u4Tmp = u4Tmp & (~CLK_REG10_LVDS_SEL_MASK);
                u4Tmp = u4Tmp | u4Sel;
                break;
            default:
                return FALSE;
        }
        CKGEN_WRITE32(REG_RW_AP_REG10, u4Tmp);
    } else {
        return FALSE;
    }
    return TRUE;
}
//EXPORT_SYMBOL(CKGEN_AgtSelClk);

extern UINT32 CKGEN_AgtGetClk(e_CLK_SEL_T eAgt)
{
    UINT32 u4Tmp;
    UINT32 u4Sel = 0;
    UINT32 ret = 0;

    if (eAgt < e_CLK_SEL_USB_27M) {    // CONFIG 0:
        u4Tmp = CKGEN_READ32(REG_RW_AP_REG0);
        switch (eAgt) {
            case e_CLK_SEL_RFI:
                u4Sel = (u4Tmp& CLK_REG0_RFI_SEL_MASK) >> CLK_REG0_RFI_SEL_OFFSET;
                break;
            case e_CLK_SEL_TP_F32K:
                u4Sel = (u4Tmp& CLK_REG0_TP_F32K_SEL_MASK) >> CLK_REG0_TP_F32K_SEL_OFFSET;
                break;
            case e_CLK_SEL_TP:
                u4Sel = (u4Tmp& CLK_REG0_TP_SEL_MASK) >> CLK_REG0_TP_SEL_OFFSET;
                break;
            case e_CLK_SEL_VDO:
                //u4Sel = (u4Tmp& CLK_REG0_TP_SEL_MASK) >> CLK_REG0_TP_SEL_OFFSET;
                break;
	    case e_CLK_SEL_RISC:
                //u4Sel = (u4Tmp& CLK_REG0_TP_SEL_MASK) >> CLK_REG0_TP_SEL_OFFSET;
                break;
            case e_CLK_SEL_DEMUX:
                u4Sel = (u4Tmp& CLK_REG0_DEMUX_SEL_MASK) >> CLK_REG0_DEMUX_SEL_OFFSET;
                break;
            case e_CLK_SEL_DSP:
                u4Sel = (u4Tmp& CLK_REG0_DSP_SEL_MASK) >> CLK_REG0_DSP_SEL_OFFSET;
                break;
            default:
                ret = 1;
                break;
        }
    } else if (eAgt < e_CLK_SEL_AUD) {    // CONFIG 1:
        u4Tmp = CKGEN_READ32(REG_RW_AP_REG1);
        switch (eAgt) {
			case e_CLK_SEL_USB_27M:
                u4Sel = (u4Tmp& CLK_REG1_USB_27M_CLK_SEL_MASK) >> CLK_REG1_USB_27M_CLK_SEL_OFFSET;
                break;
            case e_CLK_SEL_OSD:
                u4Sel = (u4Tmp& CLK_REG1_OSD_SEL_MASK) >> CLK_REG1_OSD_SEL_OFFSET;
                break;
            case e_CLK_SEL_DRAM:
                u4Sel = (u4Tmp& CLK_REG1_DRAM_SEL_MASK) >> CLK_REG1_DRAM_SEL_OFFSET;
                break;
			case e_CLK_SEL_AXIM:
                u4Sel = (u4Tmp& CLK_REG1_CLK_AXIM_SEL_MASK) >> CLK_REG1_CLK_AXIM_SEL_OFFSET;
                break;
			case e_CLK_SEL_SPM:
                u4Sel = (u4Tmp& CLK_REG1_SPM_SEL_MASK) >> CLK_REG1_SPM_SEL_OFFSET;
                break;
            case e_CLK_SEL_VDEC_SYS:
                u4Sel = (u4Tmp& CLK_REG1_VDEC_SYS_SEL_MASK) >> CLK_REG1_VDEC_SYS_SEL_OFFSET;
                break;
            case e_CLK_SEL_JPEG:
                u4Sel = (u4Tmp& CLK_REG1_JPEG_SEL_MASK) >> CLK_REG1_JPEG_SEL_OFFSET;
                break;
            case e_CLK_SEL_RSZ:
                u4Sel = (u4Tmp& CLK_REG1_RSZ_SEL_MASK) >> CLK_REG1_RSZ_SEL_OFFSET;
                break;
            case e_CLK_SEL_FLASH:
                u4Sel = (u4Tmp& CLK_REG1_FLASH_SEL_MASK) >> CLK_REG1_FLASH_SEL_OFFSET;
                break;
            case e_CLK_SEL_BCLK:
                u4Sel = (u4Tmp& CLK_REG1_BCLK_SEL_MASK) >> CLK_REG1_BCLK_SEL_OFFSET;
                break;
            default:
                ret = 1;
                break;
        }
    } else if (eAgt < e_CLK_SEL_DUTY) {    // CONFIG 2:
        u4Tmp = CKGEN_READ32(REG_RW_AP_REG2);
        switch (eAgt) {
            case e_CLK_SEL_AUD:
                u4Sel = (u4Tmp& CLK_REG2_AUD_SEL_MASK) >> CLK_REG2_AUD_SEL_OFFSET;
                break;
            case e_CLK_SEL_G3D:
                u4Sel = (u4Tmp& CLK_REG2_G3D_SEL_MASK) >> CLK_REG2_G3D_SEL_OFFSET;
                break;
            case e_CLK_SEL_FPD:
                u4Sel = (u4Tmp& CLK_REG2_FPD_SEL_MASK) >> CLK_REG2_FPD_SEL_OFFSET;
                break;
            case e_CLK_SEL_SD11:
                u4Sel = (u4Tmp& CLK_REG2_SD11_SEL_MASK) >> CLK_REG2_SD11_SEL_OFFSET;
                break;
		    case e_CLK_SEL_SD01:
                u4Sel = (u4Tmp& CLK_REG2_SD01_SEL_MASK) >> CLK_REG2_SD01_SEL_OFFSET;
                break;
            case e_CLK_SEL_SD20:
                u4Sel = (u4Tmp& CLK_REG2_SD20_SEL_MASK) >> CLK_REG2_SD20_SEL_OFFSET;
                break;
            case e_CLK_SEL_SD10:
                u4Sel = (u4Tmp& CLK_REG2_SD10_SEL_MASK) >> CLK_REG2_SD10_SEL_OFFSET;
                break;
            case e_CLK_SEL_SD00:
                u4Sel = (u4Tmp& CLK_REG2_SD00_SEL_MASK) >> CLK_REG2_SD00_SEL_OFFSET;
                break;
            case e_CLK_SEL_GRAPH:
                u4Sel = (u4Tmp& CLK_REG2_GRAPH_SEL_MASK) >> CLK_REG2_GRAPH_SEL_OFFSET;
                break;
            default:
                ret = 1;
                break;
        }
    } else if (eAgt < e_CLK_SEL_APLL_K8) {    // CONFIG 3:
        u4Tmp = CKGEN_READ32(REG_RW_AP_REG3);
        switch (eAgt) {
            case e_CLK_SEL_DUTY:
                u4Sel = (u4Tmp& CLK_REG3_DUTY_SEL_MASK) >> CLK_REG3_DUTY_SEL_OFFSET;
                break;
            case e_CLK_SEL_DEG:
                u4Sel = (u4Tmp& CLK_REG3_DEG_SEL_MASK) >> CLK_REG3_DEG_SEL_OFFSET;
                break;
            case e_CLK_SEL_NF:
                u4Sel = (u4Tmp& CLK_REG3_NF_SEL_MASK) >> CLK_REG3_NF_SEL_OFFSET;
                break;
            case e_CLK_SEL_BT_MIC_AUD:
                u4Sel = (u4Tmp& CLK_REG3_BT_MIC_AUD_SEL_MASK) >> CLK_REG3_BT_MIC_AUD_SEL_OFFSET;
                break;
            case e_CLK_SEL_ARM_AUD:
                u4Sel = (u4Tmp& CLK_REG3_ARM_AUD_SEL_MASK) >> CLK_REG3_ARM_AUD_SEL_OFFSET;
                break;
            case e_CLK_SEL_MPHON:
                u4Sel = (u4Tmp& CLK_REG3_MPHON_SEL_MASK) >> CLK_REG3_MPHON_SEL_OFFSET;
                break;
            case e_CLK_SEL_CPU2:
                u4Sel = (u4Tmp& CLK_REG3_CPU2_SEL_MASK) >> CLK_REG3_CPU2_SEL_OFFSET;
                break;
            case e_CLK_SEL_CPU1:
                u4Sel = (u4Tmp& CLK_REG3_CPU1_SEL_MASK) >> CLK_REG3_CPU1_SEL_OFFSET;
                break;
            case e_CLK_SEL_AUD2:
                u4Sel = (u4Tmp& CLK_REG3_AUD2_SEL_MASK) >> CLK_REG3_AUD2_SEL_OFFSET;
                break;
            default:
                ret = 1;
                break;
        }
   } else if (eAgt < e_CLK_SEL_MLIN) {    // CONFIG 6:
        u4Tmp = CKGEN_READ32(REG_RW_AP_REG7);
        switch (eAgt) {
            case e_CLK_SEL_APLL_K8:
                u4Sel = (u4Tmp& CLK_REG7_APLL_K8_SEL_MASK) >> CLK_REG7_APLL_K8_SEL_OFFSET;
                break;
            case e_CLK_SEL_APLL_K7:
                u4Sel = (u4Tmp& CLK_REG7_APLL_K7_SEL_MASK) >> CLK_REG7_APLL_K7_SEL_OFFSET;
                break;
            case e_CLK_SEL_APLL_K6:
                u4Sel = (u4Tmp& CLK_REG7_APLL_K6_SEL_MASK) >> CLK_REG7_APLL_K6_SEL_OFFSET;
                break;
            case e_CLK_SEL_APLL_K5:
                u4Sel = (u4Tmp& CLK_REG7_APLL_K5_SEL_MASK) >> CLK_REG7_APLL_K5_SEL_OFFSET;
                break;
            case e_CLK_SEL_APLL_K4:
                u4Sel = (u4Tmp& CLK_REG7_APLL_K4_SEL_MASK) >> CLK_REG7_APLL_K4_SEL_OFFSET;
                break;
            case e_CLK_SEL_APLL_K3:
                u4Sel = (u4Tmp& CLK_REG7_APLL_K3_SEL_MASK) >> CLK_REG7_APLL_K3_SEL_OFFSET;
                break;
            case e_CLK_SEL_APLL_K2:
                u4Sel = (u4Tmp& CLK_REG7_APLL_K2_SEL_MASK) >> CLK_REG7_APLL_K2_SEL_OFFSET;
                break;
            case e_CLK_SEL_APLL_K1:
                u4Sel = (u4Tmp& CLK_REG7_APLL_K1_SEL_MASK) >> CLK_REG7_APLL_K1_SEL_OFFSET;
                break;
            default:
                ret = 1;
                break;
        }
   } else if (eAgt < e_CLK_SEL_AUD_K5_TST) {    // CONFIG 8:
        u4Tmp = CKGEN_READ32(REG_RW_AP_REG8);
        switch (eAgt) {
            case e_CLK_SEL_MLIN:
                u4Sel = (u4Tmp& CLK_REG8_MLIN_SEL_MASK) >> CLK_REG8_MLIN_SEL_OFFSET;
                break;
            case e_CLK_SEL_AUD_MPH:
                u4Sel = (u4Tmp& CLK_REG8_AUD_MPH_SEL_MASK) >> CLK_REG8_AUD_MPH_SEL_OFFSET;
                break;
            case e_CLK_SEL_MLIN2:
                u4Sel = (u4Tmp& CLK_REG8_MLIN2_SEL_MASK) >> CLK_REG8_MLIN2_SEL_OFFSET;
                break;
            case e_CLK_SEL_AUD_PWM:
                u4Sel = (u4Tmp& CLK_REG8_AUD_PWM_SEL_MASK) >> CLK_REG8_AUD_PWM_SEL_OFFSET;
                break;
            case e_CLK_SEL_AUD_ADC:
                u4Sel = (u4Tmp& CLK_REG8_AUD_ADC_SEL_MASK) >> CLK_REG8_AUD_ADC_SEL_OFFSET;
                break;
            case e_CLK_SEL_PLL_TEST:
                u4Sel = (u4Tmp& CLK_REG8_PLL_TEST_SEL_MASK) >> CLK_REG8_PLL_TEST_SEL_OFFSET;
                break;
            case e_CLK_SEL_APLL_A3:
                u4Sel = (u4Tmp& CLK_REG8_APLL_A3_SEL_MASK) >> CLK_REG8_APLL_A3_SEL_OFFSET;
                break;
            case e_CLK_SEL_APLL_A2:
                u4Sel = (u4Tmp& CLK_REG8_APLL_A2_SEL_MASK) >> CLK_REG8_APLL_A2_SEL_OFFSET;
                break;
            case e_CLK_SEL_APLL_A1:
                u4Sel = (u4Tmp& CLK_REG8_APLL_A1_SEL_MASK) >> CLK_REG8_APLL_A1_SEL_OFFSET;
                break;
            case e_CLK_SEL_APLL_K14:
                u4Sel = (u4Tmp& CLK_REG8_APLL_K14_SEL_MASK) >> CLK_REG8_APLL_K14_SEL_OFFSET;
                break;
	case e_CLK_SEL_APLL_K13:
                u4Sel = (u4Tmp& CLK_REG8_APLL_K13_SEL_MASK) >> CLK_REG8_APLL_K13_SEL_OFFSET;
                break;
	case e_CLK_SEL_APLL_K12:
                u4Sel = (u4Tmp& CLK_REG8_APLL_K12_SEL_MASK) >> CLK_REG8_APLL_K12_SEL_OFFSET;
                break;
	case e_CLK_SEL_APLL_K11:
                u4Sel = (u4Tmp& CLK_REG8_APLL_K11_SEL_MASK) >> CLK_REG8_APLL_K11_SEL_OFFSET;
                break;
	case e_CLK_SEL_APLL_K10:
                u4Sel = (u4Tmp& CLK_REG8_APLL_K10_SEL_MASK) >> CLK_REG8_APLL_K10_SEL_OFFSET;
                break;
	case e_CLK_SEL_APLL_K9:
                u4Sel = (u4Tmp& CLK_REG8_APLL_K9_SEL_MASK) >> CLK_REG8_APLL_K9_SEL_OFFSET;
                break;
            default:
                ret = 1;
                break;
        }
  } else if (eAgt < e_CLK_SEL_SIFS1) {    // CONFIG 9:
        u4Tmp = CKGEN_READ32(REG_RW_AP_REG9);
        switch (eAgt) {
            case e_CLK_SEL_AUD_K5_TST:
                u4Sel = (u4Tmp& CLK_REG9_AUD_K5_TST_SEL_MASK) >> CLK_REG9_AUD_K5_TST_SEL_OFFSET;
                break;
            case e_CLK_SEL_AUD_A3_TST:
                u4Sel = (u4Tmp& CLK_REG9_AUD_A3_TST_SEL_MASK) >> CLK_REG9_AUD_A3_TST_SEL_OFFSET;
                break;
            case e_CLK_SEL_AUD_A2_TST:
                u4Sel = (u4Tmp& CLK_REG9_AUD_A2_TST_SEL_MASK) >> CLK_REG9_AUD_A2_TST_SEL_OFFSET;
                break;
            case e_CLK_SEL_AUD_A1_TST:
                u4Sel = (u4Tmp& CLK_REG9_AUD_A1_TST_SEL_MASK) >> CLK_REG9_AUD_A1_TST_SEL_OFFSET;
                break;
            case e_CLK_SEL_MLIN_TST:
                u4Sel = (u4Tmp& CLK_REG9_MLIN_TST_SEL_MASK) >> CLK_REG9_MLIN_TST_SEL_OFFSET;
                break;
            case e_CLK_SEL_DA_APLL1CK:
                u4Sel = (u4Tmp& CLK_REG9_DA_APLL1CK_SEL_MASK) >> CLK_REG9_DA_APLL1CK_SEL_OFFSET;
                break;
            case e_CLK_SEL_DA_APLLCK:
                u4Sel = (u4Tmp& CLK_REG9_DA_APLLCK_SEL_MASK) >> CLK_REG9_DA_APLLCK_SEL_OFFSET;
                break;
            case e_CLK_SEL_MCLK_D2:
                u4Sel = (u4Tmp& CLK_REG9_MCLK_D2_SEL_MASK) >> CLK_REG9_MCLK_D2_SEL_OFFSET;
                break;
            case e_CLK_SEL_SRAMIF:
                u4Sel = (u4Tmp& CLK_REG9_CLK_SRAMIF_SEL_MASK) >> CLK_REG9_CLK_SRAMIF_SEL_OFFSET;
                break;
            case e_CLK_SEL_TVD_MBIST:
                u4Sel = (u4Tmp& CLK_REG9_TVD_MBIST_SEL_MASK) >> CLK_REG9_TVD_MBIST_SEL_OFFSET;
                break;
            case e_CLK_SEL_SPI_MOTO:
                u4Sel = (u4Tmp& CLK_REG9_SPI_MOTO_SEL_MASK) >> CLK_REG9_SPI_MOTO_SEL_OFFSET;
                break;
            case e_CLK_SEL_PNG:
                u4Sel = (u4Tmp& CLK_REG9_PNG_SEL_MASK) >> CLK_REG9_PNG_SEL_OFFSET;
                break;
            case e_CLK_SEL_TS1:
                u4Sel = (u4Tmp& CLK_REG9_TS1_SEL_MASK) >> CLK_REG9_TS1_SEL_OFFSET;
                break;
            case e_CLK_SEL_TS0:
                u4Sel = (u4Tmp& CLK_REG9_TS0_SEL_MASK) >> CLK_REG9_TS0_SEL_OFFSET;
                break;
            default:
                ret = 1;
                break;
        }
  } else if (eAgt < e_CLK_SEL_MAX) {     // CONFIG 10:
        u4Tmp = CKGEN_READ32(REG_RW_AP_REG10);
        switch (eAgt) {
            case e_CLK_SEL_SIFS1:
                u4Sel = (u4Tmp& CLK_REG10_SIFS1_SEL_MASK) >> CLK_REG10_SIFS1_SEL_OFFSET;
                break;
            case e_CLK_SEL_SIFS0:
                u4Sel = (u4Tmp& CLK_REG10_SIFS0_SEL_MASK) >> CLK_REG10_SIFS0_SEL_OFFSET;
                break;
            case e_CLK_SEL_SIFM1:
                u4Sel = (u4Tmp& CLK_REG10_SIFM1_SEL_MASK) >> CLK_REG10_SIFM1_SEL_OFFSET;
                break;
            case e_CLK_SEL_SIFM0:
                u4Sel = (u4Tmp& CLK_REG10_SIFM0_SEL_MASK) >> CLK_REG10_SIFM0_SEL_OFFSET;
                break;
			case e_CLK_SEL_PWM3:
                u4Sel = (u4Tmp& CLK_REG10_PWM3_SEL_MASK) >> CLK_REG10_PWM3_SEL_OFFSET;
                break;
            case e_CLK_SEL_PWM2:
                u4Sel = (u4Tmp& CLK_REG10_PWM2_SEL_MASK) >> CLK_REG10_PWM2_SEL_OFFSET;
                break;
            case e_CLK_SEL_PWM1:
                u4Sel = (u4Tmp& CLK_REG10_PWM1_SEL_MASK) >> CLK_REG10_PWM1_SEL_OFFSET;
                break;
            case e_CLK_SEL_PWM0:
                u4Sel = (u4Tmp& CLK_REG10_PWM0_SEL_MASK) >> CLK_REG10_PWM0_SEL_OFFSET;
                break;
            case e_CLK_SEL_APLL_26M:
                u4Sel = (u4Tmp& CLK_REG10_APLL_26M_SEL_MASK) >> CLK_REG10_APLL_26M_SEL_OFFSET;
                break;
            case e_CLK_SEL_TWDS:
                u4Sel = (u4Tmp& CLK_REG10_TWDS_SEL_MASK) >> CLK_REG10_TWDS_SEL_OFFSET;
                break;
            case e_CLK_SEL_LVDS:
                u4Sel = (u4Tmp& CLK_REG10_LVDS_SEL_MASK) >> CLK_REG10_LVDS_SEL_OFFSET;
                break;
            default:
                ret = 1;
                break;
        }
    } else {
        ret = 1;
    }

    if (ret) {
        return 0xFF;
    }	else {
        return u4Sel;
    }
}
//EXPORT_SYMBOL(CKGEN_AgtGetClk);


extern BOOL CKGEN_AgtOnClk_NoReset(e_CLK_T eAgt)
{
    UINT32 u4Tmp;

    if (eAgt < e_CLK_GFX) {    // CONFIG 0:
        u4Tmp = CKGEN_READ32(REG_RW_CLKGATE_CFG0);
        //u4Reset = CKGEN_READ32(REG_RW_SYNC_RESET_CFG0);
        switch (eAgt) {
            case e_CLK_VDEC_FULL:
                u4Tmp = u4Tmp | (CLK_PDN_VDEC_FULL_MASK);
                //u4Reset = u4Reset | (CLK_RESET_VDEC_FULL_MASK);
                break;
            default:
                return FALSE;
        }
        //CKGEN_WRITE32(REG_RW_SYNC_RESET_CFG0, u4Reset);
        CKGEN_WRITE32(REG_RW_CLKGATE_CFG0, u4Tmp);
    } else if (eAgt < e_CLK_DUTY_METER) {    // CONFIG 1:
        u4Tmp = CKGEN_READ32(REG_RW_CLKGATE_CFG1);
        //u4Reset = CKGEN_READ32(REG_RW_SYNC_RESET_CFG1);
        switch (eAgt) {
            case e_CLK_GFX:
                u4Tmp = u4Tmp | (CLK_PDN_GFX);
                //u4Reset = u4Reset | (CLK_RESET_GFX);
                break;
            case e_CLK_DMARB:
                u4Tmp = u4Tmp | (CLK_PDN_DMARB);
                //u4Reset = u4Reset | (CLK_RESET_DMARB);
                break;
            case e_CLK_PNG:
                u4Tmp = u4Tmp | (CLK_PDN_PNG);
                //u4Reset = u4Reset | (CLK_RESET_PNG);
                break;
            case e_CLK_GIF:
                u4Tmp = u4Tmp | (CLK_PDN_GIF);
                //u4Reset = u4Reset | (CLK_RESET_GIF);
                break;
            case e_CLK_IMG_RESZ:
                u4Tmp = u4Tmp | (CLK_PDN_IMG_RESZ);
                //u4Reset = u4Reset | (CLK_RESET_IMG_RESZ);
                break;
            case e_CLK_OSD_RESZ:
                u4Tmp = u4Tmp | (CLK_PDN_OSD_RESZ);
                //u4Reset = u4Reset | (CLK_RESET_OSD_RESZ);
                break;
            case e_CLK_JPGDEC:
                u4Tmp = u4Tmp | (CLK_PDN_JPGDEC);
                //u4Reset = u4Reset | (CLK_RESET_JPGDEC);
                break;
            case e_CLK_DEMUX:
                u4Tmp = u4Tmp & (~CLK_PDN_DEMUX);
                //u4Reset = u4Reset | (CLK_RESET_DEMUX);
                break;
            case e_CLK_DEMUX_TS0:
                u4Tmp = u4Tmp & (~CLK_PDN_DEMUX_TS0);
                //u4Reset = u4Reset | (CLK_RESET_DEMUX);
                break;
            case e_CLK_DEMUX_TS1:
                u4Tmp = u4Tmp & (~CLK_PDN_DEMUX_TS1);
                //u4Reset = u4Reset | (CLK_RESET_DEMUX);
                break;
            case e_CLK_DEMUX_27M:
                u4Tmp = u4Tmp & (~CLK_PDN_DEMUX_27M);
                //u4Reset = u4Reset | (CLK_RESET_DEMUX);
                break;
            case e_CLK_NFI:
                u4Tmp = u4Tmp | (CLK_PDN_NFI);
                //u4Reset = u4Reset | (CLK_RESET_NFI);
                break;
            case e_CLK_USB:
                u4Tmp = u4Tmp | (CLK_PDN_USB);
                //u4Reset = u4Reset | (CLK_RESET_USB);
                break;
	case e_CLK_IRT_DMA_WRAPPER:
                u4Tmp = u4Tmp | (CLK_PDN_IRT_DMA_WRAPPER);
                //u4Reset = u4Reset | (CLK_RESET_IRT_DMA_WRAPPER);
                break;
	case e_CLK_ARM9:
                u4Tmp = u4Tmp | (CLK_PDN_ARM9);
                //u4Reset = u4Reset | (CLK_RESET_ARM9);
                break;
            default:
                return FALSE;
        }
        //CKGEN_WRITE32(REG_RW_SYNC_RESET_CFG1, u4Reset);
        CKGEN_WRITE32(REG_RW_CLKGATE_CFG1, u4Tmp);
    } else if (eAgt < e_CLK_AUDIO_B00) {
		u4Tmp = CKGEN_READ32(REG_RW_CLKGATE_CFG2);
	    //u4Reset = CKGEN_READ32(REG_RW_SYNC_RESET_CFG1);
		switch (eAgt) {
			case e_CLK_DUTY_METER:
				u4Tmp = u4Tmp | (CLK_PDN_DUTY_METER_MASK);
				//u4Reset = u4Reset | (CLK_RESET_GFX);
				break;
			default:
                return FALSE;
        }
        //CKGEN_WRITE32(REG_RW_SYNC_RESET_CFG1, u4Reset);
        CKGEN_WRITE32(REG_RW_CLKGATE_CFG2, u4Tmp);
	}else if (eAgt < e_CLK_MVDO) {    // CONFIG 2:
        u4Tmp = CKGEN_READ32(REG_RW_CLKGATE_CFG3);
        //u4Reset = CKGEN_READ32(REG_RW_SYNC_RESET_CFG3);
        switch (eAgt) {
            case e_CLK_AUDIO_B00:
                u4Tmp = u4Tmp | (CLK_PDN_AUDIO_B00|CLK_PDN_AUDIO_B01|CLK_PDN_AUDIO_B02);
                //u4Reset = u4Reset | (CLK_RESET_AUDIO_B00|CLK_RESET_AUDIO_B01|CLK_RESET_AUDIO_B02);
                break;
            case e_CLK_AUDIO_B01:
                u4Tmp = u4Tmp | (CLK_PDN_AUDIO_B01);
                //u4Reset = u4Reset | (CLK_RESET_AUDIO_B01);
                break;
            case e_CLK_AUDIO_B02:
                u4Tmp = u4Tmp | (CLK_PDN_AUDIO_B02);
                //u4Reset = u4Reset | (CLK_RESET_AUDIO_B02);
                break;
            case e_CLK_AUDIO_B03:
                u4Tmp = u4Tmp | (CLK_PDN_AUDIO_B03);
                //u4Reset = u4Reset | (CLK_RESET_AUDIO_B03);
                break;
            case e_CLK_AUDIO_B04:
                u4Tmp = u4Tmp | (CLK_PDN_AUDIO_B04);
                //u4Reset = u4Reset | (CLK_RESET_AUDIO_B04);
                break;
            case e_CLK_AUDIO_B05:
                u4Tmp = u4Tmp | (CLK_PDN_AUDIO_B05);
                //u4Reset = u4Reset | (CLK_RESET_AUDIO_B05);
                break;
            case e_CLK_AUDIO_B06:
                u4Tmp = u4Tmp | (CLK_PDN_AUDIO_B06);
                //u4Reset = u4Reset | (CLK_RESET_AUDIO_B06);
                break;
            case e_CLK_AUDIO_B07:
                u4Tmp = u4Tmp | (CLK_PDN_AUDIO_B07);
                //u4Reset = u4Reset | (CLK_RESET_AUDIO_B07);
                break;
            case e_CLK_AUDIO_B08:
                u4Tmp = u4Tmp | (CLK_PDN_AUDIO_B08);
                //u4Reset = u4Reset | (CLK_RESET_AUDIO_B08);
                break;
            case e_CLK_AUDIO_B09:
                u4Tmp = u4Tmp | (CLK_PDN_AUDIO_B09);
                //u4Reset = u4Reset | (CLK_RESET_AUDIO_B09);
                break;
            case e_CLK_AUDIO_B10:
                u4Tmp = u4Tmp | (CLK_PDN_AUDIO_B10);
                //u4Reset = u4Reset | (CLK_RESET_AUDIO_B10);
                break;
	case e_CLK_AUDIO_B11:
                u4Tmp = u4Tmp | (CLK_PDN_AUDIO_B11);
                //u4Reset = u4Reset | (CLK_RESET_AUDIO_B11);
                break;
	case e_CLK_AUDIO_B12:
                u4Tmp = u4Tmp | (CLK_PDN_AUDIO_B12);
                //u4Reset = u4Reset | (CLK_RESET_AUDIO_B12);
                break;
	case e_CLK_AUDIO_B13:
		u4Tmp = u4Tmp | (CLK_PDN_AUDIO_B13);
		//u4Reset = u4Reset | (CLK_RESET_AUDIO_B13);
		break;
	case e_CLK_AUDIO_B14:
		u4Tmp = u4Tmp | (CLK_PDN_AUDIO_B14);
		//u4Reset = u4Reset | (CLK_RESET_AUDIO_B14);
		break;
            case e_CLK_RFI_TOP:
                u4Tmp = u4Tmp | (CLK_PDN_RFI_TOP);
                //u4Reset = u4Reset | (CLK_RESET_RFI_TOP);
                break;
            case e_CLK_MSDC_0:
                u4Tmp = u4Tmp | (CLK_PDN_MSDC_0);
                //u4Reset = u4Reset | (CLK_RESET_MSDC_0);
                break;
            case e_CLK_MSDC_1:
                u4Tmp = u4Tmp | (CLK_PDN_MSDC_1);
                //u4Reset = u4Reset | (CLK_RESET_MSDC_1);
                break;
            case e_CLK_MSDC_2:
                u4Tmp = u4Tmp | (CLK_PDN_MSDC_2);
                //u4Reset = u4Reset | (CLK_RESET_MSDC_2);
                break;
	case e_CLK_MSDC_SW_0:
		//u4Tmp = u4Tmp | (CLK_PDN_MSDC_SW_0);
		//u4Reset = u4Reset | (CLK_RESET_MSDC_SW_0);
		break;
	case e_CLK_MSDC_SW_1:
		//u4Tmp = u4Tmp | (CLK_PDN_MSDC_SW_1);
		//u4Reset = u4Reset | (CLK_RESET_MSDC_SW_1);
		break;
	case e_CLK_MSDC_SW_2:
		//u4Tmp = u4Tmp | (CLK_PDN_MSDC_SW_2);
		//u4Reset = u4Reset | (CLK_RESET_MSDC_SW_2);
		break;
            case e_CLK_SPI_MOTO1:
                u4Tmp = u4Tmp | (CLK_PDN_SPI_MOTO1);
                //u4Reset = u4Reset | (CLK_RESET_SPI_MOTO1);
                break;
            case e_CLK_SPI_MOTO2:
                u4Tmp = u4Tmp | (CLK_PDN_SPI_MOTO2);
                //u4Reset = u4Reset | (CLK_RESET_SPI_MOTO2);
                break;
            case e_CLK_PWM0:
                u4Tmp = u4Tmp | (CLK_PDN_PWM0);
                //u4Reset = u4Reset | (CLK_RESET_PWM0);
                break;
            case e_CLK_PWM1:
                u4Tmp = u4Tmp | (CLK_PDN_PWM1);
                //u4Reset = u4Reset | (CLK_RESET_PWM1);
                break;
            case e_CLK_PWM2:
                u4Tmp = u4Tmp | (CLK_PDN_PWM2);
                //u4Reset = u4Reset | (CLK_RESET_PWM2);
                break;
            case e_CLK_PWM3:
                u4Tmp = u4Tmp | (CLK_PDN_PWM3);
                //u4Reset = u4Reset | (CLK_RESET_PWM3);
                break;
            case e_CLK_SIFM0:
                u4Tmp = u4Tmp | (CLK_PDN_SIFM0);
                //u4Reset = u4Reset | (CLK_RESET_SIFM0);
                break;
            case e_CLK_SIFM1:
                u4Tmp = u4Tmp | (CLK_PDN_SIFM1);
                //u4Reset = u4Reset | (CLK_RESET_SIFM1);
                break;
            case e_CLK_SIFS0:
                u4Tmp = u4Tmp | (CLK_PDN_SIFS0);
                //u4Reset = u4Reset | (CLK_RESET_SIFS0);
                break;
            case e_CLK_SIFS1:
                u4Tmp = u4Tmp | (CLK_PDN_SIFS1);
                //u4Reset = u4Reset | (CLK_RESET_SIFS1);
                break;
            default:
                return FALSE;
        }
        CKGEN_WRITE32(REG_RW_CLKGATE_CFG3, u4Tmp);
        //CKGEN_WRITE32(REG_RW_SYNC_RESET_CFG3, u4Reset);
    } else if (eAgt < e_CLK_LVDS) {    // CONFIG 3:
        u4Tmp = CKGEN_READ32(REG_RW_CLKGATE_CFG4);
        //u4Reset = CKGEN_READ32(REG_RW_SYNC_RESET_CFG4);
        switch (eAgt) {
            case e_CLK_MVDO:
                u4Tmp = u4Tmp | (CLK_PDN_MVDO);
                //u4Reset = u4Reset | (CLK_RESET_MVDO);
                break;
            case e_CLK_DGO:
                u4Tmp = u4Tmp | (CLK_PDN_DGO);
                //u4Reset = u4Reset | (CLK_RESET_DGO);
                break;
            case e_CLK_DACTST:
                u4Tmp = u4Tmp | (CLK_PDN_DACTST);
                //u4Reset = u4Reset | (CLK_RESET_DACTST);
                break;
            case e_CLK_DVD_OSD:
                u4Tmp = u4Tmp | (CLK_PDN_DVD_OSD);
                //u4Reset = u4Reset | (CLK_RESET_DVD_OSD);
                break;
            case e_CLK_GRA:
                u4Tmp = u4Tmp | (CLK_PDN_GRA);
                //u4Reset = u4Reset | (CLK_RESET_GRA);
                break;
            case e_CLK_BIM:
                u4Tmp = u4Tmp | (CLK_PDN_BIM);
                //u4Reset = u4Reset | (CLK_RESET_BIM);
                break;
            case e_CLK_TURBO32:
                u4Tmp = u4Tmp | (CLK_PDN_TURBO32);
                //u4Reset = u4Reset | (CLK_RESET_TURBO32);
                break;
            case e_CLK_VDEC:
                u4Tmp = u4Tmp | (CLK_PDN_VDEC);
                //u4Reset = u4Reset | (CLK_RESET_VDEC);
                break;
            case e_CLK_PARSER:
                u4Tmp = u4Tmp | (CLK_PDN_PARSER);
                //u4Reset = u4Reset | (CLK_RESET_PARSER);
                break;
            case e_CLK_RAMBUF:
                u4Tmp = u4Tmp | (CLK_PDN_RAMBUF);
                //u4Reset = u4Reset | (CLK_RESET_RAMBUF);
                break;
            case e_CLK_PT110:
                u4Tmp = u4Tmp | (CLK_PDN_PT110);
                //u4Reset = u4Reset | (CLK_RESET_PT110);
                break;
            case e_CLK_RS232:
                u4Tmp = u4Tmp | (CLK_PDN_RS232);
                //u4Reset = u4Reset | (CLK_RESET_RS232);
                break;
            case e_CLK_CDDVD:
                u4Tmp = u4Tmp | (CLK_PDN_CDDVD);
                //u4Reset = u4Reset | (CLK_RESET_CDDVD);
                break;
            case e_CLK_AUDIO:
                u4Tmp = u4Tmp | (CLK_PDN_AUDIO);
                //u4Reset = u4Reset | (CLK_RESET_AUDIO);
                break;
            case e_CLK_SERVO_MISC:
                u4Tmp = u4Tmp | (CLK_PDN_SERVO_MISC);
                //u4Reset = u4Reset | (CLK_RESET_SERVO_MISC);
                break;
	case e_CLK_RAMBUF_APCTRL_TBUS3:
                u4Tmp = u4Tmp | (CLK_PDN_RAMBUF_APCTRL_TBUS3);
                //u4Reset = u4Reset & (~CLK_RESET_RAMBUF_APCTRL_TBUS3);
                break;
	case e_CLK_RAMBUF_APCTRL_TBUS4:
                u4Tmp = u4Tmp | (CLK_PDN_RAMBUF_APCTRL_TBUS4);
                //u4Reset = u4Reset & (~CLK_RESET_RAMBUF_APCTRL_TBUS4);
                break;
	case e_CLK_RAMBUF_APCTRL_TBUS5:
                u4Tmp = u4Tmp | (CLK_PDN_RAMBUF_APCTRL_TBUS5);
                //u4Reset = u4Reset & (~CLK_RESET_RAMBUF_APCTRL_TBUS5);
                break;
	case e_CLK_MFG_TOP_PWR_WRAP:
		u4Tmp = u4Tmp & (~CLK_PDN_MFG_TOP_PWR_WRAP);
		//u4Reset = u4Reset | (CLK_RESET_MFG_TOP_PWR_WRAP);
		break;
	default:
                return FALSE;
        }
        //CKGEN_WRITE32(REG_RW_SYNC_RESET_CFG4, u4Reset);
        CKGEN_WRITE32(REG_RW_CLKGATE_CFG4, u4Tmp);
    } else if (eAgt < e_CLK_SCLER) {    // CONFIG 3:
        u4Tmp = CKGEN_READ32(REG_RW_CLKGATE_CFG5);
        //u4Reset = CKGEN_READ32(REG_RW_SYNC_RESET_CFG5);
        switch (eAgt) {
            case e_CLK_LVDS:
                u4Tmp = u4Tmp | (CLK_PDN_CLK_LVDS);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_LVDS);
                break;
	case e_CLK_TP_TOP0:
                u4Tmp = u4Tmp | (CLK_PDN_CLK_TP_TOP0);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
                break;
	case e_CLK_TP_TOP1:
                u4Tmp = u4Tmp | (CLK_PDN_CLK_TP_TOP1);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
                break;
	case e_CLK_TP_TOP2:
                u4Tmp = u4Tmp  | (CLK_PDN_CLK_TP_TOP2);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
                break;
	case e_CLK_RFI_TOP1:
                u4Tmp = u4Tmp | (CLK_PDN_CLK_RFI_TOP1);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
                break;
	case e_CLK_RFI_TOP2:
                u4Tmp = u4Tmp | (CLK_PDN_CLK_RFI_TOP2);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
                break;
	case e_CLK_RFI_TOP3:
                u4Tmp = u4Tmp | (CLK_PDN_CLK_RFI_TOP3);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
                break;
	case e_CLK_RFI_TOP4:
                u4Tmp = u4Tmp | (CLK_PDN_CLK_RFI_TOP4);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
                break;
	case e_CLK_RFI_TOP5:
		u4Tmp = u4Tmp | (CLK_PDN_CLK_RFI_TOP5);
		//u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
		break;
	case e_CLK_RFI_TOP6:
                u4Tmp = u4Tmp | (CLK_PDN_CLK_RFI_TOP6);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
                break;
            default:
                return FALSE;
        }
        //CKGEN_WRITE32(REG_RW_SYNC_RESET_CFG5, u4Reset);
        CKGEN_WRITE32(REG_RW_CLKGATE_CFG5, u4Tmp);
    } else if (eAgt < e_CLK_MAX) {    // CONFIG 6:
        u4Tmp = CKGEN_READ32(REG_RW_CLKGATE_CFG6);
        //u4Reset = CKGEN_READ32(REG_RW_SYNC_RESET_CFG6);
        switch (eAgt) {
            case e_CLK_SCLER:
                u4Tmp = u4Tmp | (CLK_PDN_SCLER);
                //u4Reset = u4Reset | (CLK_RESET_SCLER);
                break;
            case e_CLK_TVD1:
                u4Tmp = u4Tmp | (CLK_PDN_TVD1);
                //u4Reset = u4Reset | (CLK_RESET_TVD);
                break;
            case e_CLK_TVD2:
                u4Tmp = u4Tmp | (CLK_PDN_TVD2);
                //u4Reset = u4Reset | (CLK_RESET_TVD);
                break;
            case e_CLK_OSD:
                u4Tmp = u4Tmp | (CLK_PDN_OSD);
                //u4Reset = u4Reset | (CLK_RESET_OSD);
                break;
            case e_CLK_OSD_R:
                u4Tmp = u4Tmp | (CLK_PDN_OSD_R);
                //u4Reset = u4Reset | (CLK_RESET_OSD_R);
                break;
            case e_CLK_FPD:
                u4Tmp = u4Tmp | (CLK_PDN_FPD);
                //u4Reset = u4Reset | (CLK_RESET_FPD);
                break;
            case e_CLK_FMT_VDO_F:
                u4Tmp = u4Tmp | (CLK_PDN_FMT_VDO_F);
                //u4Reset = u4Reset | (CLK_RESET_FMT_VDO_F);
                break;
            case e_CLK_FMT_VDO_R:
                u4Tmp = u4Tmp | (CLK_PDN_FMT_VDO_R);
                //u4Reset = u4Reset | (CLK_RESET_FMT_VDO_R);
                break;
            case e_CLK_WRITE_CHANEL:
                u4Tmp = u4Tmp | (CLK_PDN_WRITE_CHANEL);
                //u4Reset = u4Reset | (CLK_RESET_WRITE_CHANEL);
                break;
            case e_CLK_FRAME_LOCK:
                u4Tmp = u4Tmp | (CLK_PDN_FRAME_LOCK);
                //u4Reset = u4Reset | (CLK_RESET_FRAME_LOCK);
                break;
	case e_CLK_WRITE_CHANEL_2:
		u4Tmp = u4Tmp | (CLK_PDN_WRITE_CHANEL2);
		//u4Reset = u4Reset | (CLK_RESET_WRITE_CHANEL2);
		break;
	case e_CLK_VGA:
		u4Tmp = u4Tmp | (CLK_PDN_VGA_EDID);
		//u4Reset = u4Reset | (CLK_RESET_VGA_EDID);
		break;
	case e_CLK_YPBPR:
		u4Tmp = u4Tmp | (CLK_PDN_YPBPR_VGA);
		//u4Reset = u4Reset | (CLK_RESET_YPBPR_VGA);
		break;
	case e_CLK_HDMI:
		u4Tmp = u4Tmp | (CLK_PDN_HDMI);
		//u4Reset = u4Reset | (CLK_RESET_HDMI);
		break;
	case e_CLK_TVE:
		u4Tmp = u4Tmp | (CLK_PDN_TVE);
		//u4Reset = u4Reset | (CLK_RESET_TVE);
		break;
	case e_CLK_DVD_MIX_2AP:
		u4Tmp = u4Tmp | (CLK_PDN_DVD_MIX_2AP);
		//u4Reset = u4Reset | (CLK_RESET_DVD_MIX_2AP);
		break;
	case e_CLK_OSD_1:
		u4Tmp = u4Tmp | (CLK_PDN_OSD1);
		//u4Reset = u4Reset | (CLK_RESET_FRAME_LOCK);
		break;
	case e_CLK_OSD_2:
		u4Tmp = u4Tmp | (CLK_PDN_OSD2);
		//u4Reset = u4Reset | (CLK_RESET_FRAME_LOCK);
		break;
	case e_CLK_OSD_3:
		u4Tmp = u4Tmp | (CLK_PDN_OSD3);
		//u4Reset = u4Reset | (CLK_RESET_FRAME_LOCK);
		break;
	case e_CLK_OSD_4:
		u4Tmp = u4Tmp | (CLK_PDN_OSD4);
		//u4Reset = u4Reset | (CLK_RESET_FRAME_LOCK);
		break;
	case e_CLK_OSD_5:
		u4Tmp = u4Tmp | (CLK_PDN_OSD5);
		//u4Reset = u4Reset | (CLK_RESET_FRAME_LOCK);
		break;
	case e_CLK_OSD_R_2:
		u4Tmp = u4Tmp | (CLK_PDN_OSD_R2);
		//u4Reset = u4Reset | (CLK_RESET_FRAME_LOCK);
		break;
	case e_CLK_OSD_R_3:
		u4Tmp = u4Tmp | (CLK_PDN_OSD_R3);
		//u4Reset = u4Reset | (CLK_RESET_DVD_MIX_2AP);
		break;
	case e_CLK_SCLER_TG:
		u4Tmp = u4Tmp | (CLK_PDN_SCLER_TG);
		//u4Reset = u4Reset | (CLK_RESET_SCLER_TG);
		break;
	case e_CLK_LCPROC_VDO:
		u4Tmp = u4Tmp | (CLK_PDN_LCPROC_VDO);
		//u4Reset = u4Reset | (CLK_RESET_ICPROC_VDO);
		break;
            default:
                return FALSE;
        }
        //CKGEN_WRITE32(REG_RW_SYNC_RESET_CFG6, u4Reset);
        CKGEN_WRITE32(REG_RW_CLKGATE_CFG6, u4Tmp);
    } else {
        return FALSE;
    }

    return TRUE;
}

// EXPORT_SYMBOL(CKGEN_AgtOnClk_NoReset);

extern BOOL CKGEN_AgtOffClk_NoReset(e_CLK_T eAgt)
{
    UINT32 u4Tmp;

    if (eAgt < e_CLK_GFX) {    // CONFIG 0:
        u4Tmp = CKGEN_READ32(REG_RW_CLKGATE_CFG0);
        //u4Reset = CKGEN_READ32(REG_RW_SYNC_RESET_CFG0);
        switch (eAgt) {
            case e_CLK_VDEC_FULL:
                u4Tmp = u4Tmp & (~CLK_PDN_VDEC_FULL_MASK);
                //u4Reset = u4Reset & (~CLK_RESET_VDEC_FULL_MASK);
                break;
            default:
                return FALSE;
        }
        //CKGEN_WRITE32(REG_RW_SYNC_RESET_CFG0, u4Reset);
        CKGEN_WRITE32(REG_RW_CLKGATE_CFG0, u4Tmp);
    } else if (eAgt < e_CLK_AUDIO_B00) {    // CONFIG 1:
        u4Tmp = CKGEN_READ32(REG_RW_CLKGATE_CFG1);
        //u4Reset = CKGEN_READ32(REG_RW_SYNC_RESET_CFG1);
        switch (eAgt) {
            case e_CLK_GFX:
                u4Tmp = u4Tmp & (~CLK_PDN_GFX);
                //u4Reset = u4Reset & (~CLK_RESET_GFX);
                break;
            case e_CLK_DMARB:
                u4Tmp = u4Tmp & (~CLK_PDN_DMARB);
                //u4Reset = u4Reset & (~CLK_RESET_DMARB);
                break;
            case e_CLK_PNG:
                u4Tmp = u4Tmp & (~CLK_PDN_PNG);
                //u4Reset = u4Reset & (~CLK_RESET_PNG);
                break;
            case e_CLK_GIF:
                u4Tmp = u4Tmp & (~CLK_PDN_GIF);
                //u4Reset = u4Reset & (~CLK_RESET_GIF);
                break;
            case e_CLK_IMG_RESZ:
                u4Tmp = u4Tmp & (~CLK_PDN_IMG_RESZ);
                //u4Reset = u4Reset & (~CLK_RESET_IMG_RESZ);
                break;
            case e_CLK_OSD_RESZ:
                u4Tmp = u4Tmp & (~CLK_PDN_OSD_RESZ);
                //u4Reset = u4Reset & (~CLK_RESET_OSD_RESZ);
                break;
            case e_CLK_JPGDEC:
                u4Tmp = u4Tmp & (~CLK_PDN_JPGDEC);
                //u4Reset = u4Reset & (~CLK_RESET_JPGDEC);
                break;
            case e_CLK_DEMUX:
                u4Tmp = u4Tmp | (CLK_PDN_DEMUX);
                //u4Reset = u4Reset & (~CLK_RESET_DEMUX);
                break;
            case e_CLK_DEMUX_TS0:
                u4Tmp = u4Tmp | (CLK_PDN_DEMUX_TS0);
                //u4Reset = u4Reset & (~CLK_RESET_DEMUX);  //?
                break;
            case e_CLK_DEMUX_TS1:
                u4Tmp = u4Tmp | (CLK_PDN_DEMUX_TS1);
                //u4Reset = u4Reset & (~CLK_RESET_DEMUX);  //?
                break;
            case e_CLK_DEMUX_27M:
                u4Tmp = u4Tmp | (CLK_PDN_DEMUX_27M);
                //u4Reset = u4Reset & (~CLK_RESET_DEMUX);  //?
                break;
            case e_CLK_NFI:
                u4Tmp = u4Tmp | (CLK_PDN_NFI);
                //u4Reset = u4Reset & (~CLK_RESET_NFI);  //?
                break;
            case e_CLK_USB:
                u4Tmp = u4Tmp & (~CLK_PDN_USB);
                //u4Reset = u4Reset & (~CLK_RESET_USB);
                break;
            case e_CLK_IRT_DMA_WRAPPER:
                u4Tmp = u4Tmp & (~CLK_PDN_IRT_DMA_WRAPPER);
                //u4Reset = u4Reset & (~CLK_RESET_IRT_DMA_WRAPPER);
                break;
			case e_CLK_ARM9:
                u4Tmp = u4Tmp & (~CLK_PDN_ARM9);
                 break;
            default:
                return FALSE;
        }
        //CKGEN_WRITE32(REG_RW_SYNC_RESET_CFG1, u4Reset);
        CKGEN_WRITE32(REG_RW_CLKGATE_CFG1, u4Tmp);
    } else if (eAgt < e_CLK_MVDO) {    // CONFIG 2:
        u4Tmp = CKGEN_READ32(REG_RW_CLKGATE_CFG3);
        //u4Reset = CKGEN_READ32(REG_RW_SYNC_RESET_CFG3);
        switch (eAgt) {
            case e_CLK_AUDIO_B00:
                u4Tmp = u4Tmp & (~CLK_PDN_AUDIO_B00);
                //u4Reset = u4Reset & (~CLK_RESET_AUDIO_B00);
                break;
            case e_CLK_AUDIO_B01:
                u4Tmp = u4Tmp & (~CLK_PDN_AUDIO_B01);
                //u4Reset = u4Reset & (~CLK_RESET_AUDIO_B01);
                break;
            case e_CLK_AUDIO_B02:
                u4Tmp = u4Tmp & (~CLK_PDN_AUDIO_B02);
                //u4Reset = u4Reset & (~CLK_RESET_AUDIO_B02);
                break;
            case e_CLK_AUDIO_B03:
                u4Tmp = u4Tmp & (~CLK_PDN_AUDIO_B03);
                //u4Reset = u4Reset & (~CLK_RESET_AUDIO_B03);
                break;
            case e_CLK_AUDIO_B04:
                u4Tmp = u4Tmp & (~CLK_PDN_AUDIO_B04);
                //u4Reset = u4Reset & (~CLK_RESET_AUDIO_B04);
                break;
            case e_CLK_AUDIO_B05:
                u4Tmp = u4Tmp & (~CLK_PDN_AUDIO_B05);
                //u4Reset = u4Reset & (~CLK_RESET_AUDIO_B05);
                break;
            case e_CLK_AUDIO_B06:
                u4Tmp = u4Tmp & (~CLK_PDN_AUDIO_B06);
                //u4Reset = u4Reset & (~CLK_RESET_AUDIO_B06);
                break;
            case e_CLK_AUDIO_B07:
                u4Tmp = u4Tmp & (~CLK_PDN_AUDIO_B07);
                //u4Reset = u4Reset & (~CLK_RESET_AUDIO_B07);
                break;
            case e_CLK_AUDIO_B08:
                u4Tmp = u4Tmp & (~CLK_PDN_AUDIO_B08);
                //u4Reset = u4Reset & (~CLK_RESET_AUDIO_B08);
                break;
            case e_CLK_AUDIO_B09:
                u4Tmp = u4Tmp & (~CLK_PDN_AUDIO_B09);
                //u4Reset = u4Reset & (~CLK_RESET_AUDIO_B09);
                break;
            case e_CLK_AUDIO_B10:
                u4Tmp = u4Tmp & (~CLK_PDN_AUDIO_B10);
                //u4Reset = u4Reset & (~CLK_RESET_AUDIO_B10);
                break;
	case e_CLK_AUDIO_B11:
		u4Tmp = u4Tmp & (~CLK_PDN_AUDIO_B11);
		//u4Reset = u4Reset & (~CLK_RESET_AUDIO_B11);
		break;
	case e_CLK_AUDIO_B12:
		u4Tmp = u4Tmp & (~CLK_PDN_AUDIO_B12);
		//u4Reset = u4Reset & (~CLK_RESET_AUDIO_B12);
		break;
	case e_CLK_AUDIO_B13:
		u4Tmp = u4Tmp & (~CLK_PDN_AUDIO_B13);
		//u4Reset = u4Reset & (~CLK_RESET_AUDIO_B13);
		break;
	case e_CLK_AUDIO_B14:
		u4Tmp = u4Tmp & (~CLK_PDN_AUDIO_B14);
		//u4Reset = u4Reset & (~CLK_RESET_AUDIO_B14);
		break;

            case e_CLK_RFI_TOP:
                u4Tmp = u4Tmp & (~CLK_PDN_RFI_TOP);
                //u4Reset = u4Reset & (~CLK_RESET_RFI_TOP);
                break;
            case e_CLK_MSDC_0:
                u4Tmp = u4Tmp & (~CLK_PDN_MSDC_0);
                //u4Reset = u4Reset & (~CLK_RESET_MSDC_0);
                break;
            case e_CLK_MSDC_1:
                u4Tmp = u4Tmp & (~CLK_PDN_MSDC_1);
                //u4Reset = u4Reset & (~CLK_RESET_MSDC_1);
                break;
            case e_CLK_MSDC_2:
                u4Tmp = u4Tmp & (~CLK_PDN_MSDC_2);
                //u4Reset = u4Reset & (~CLK_RESET_MSDC_2);
                break;
            case e_CLK_MSDC_SW_0:
		//u4Tmp = u4Tmp & (~CLK_PDN_MSDC_SW_0);
		//u4Reset = u4Reset & (~CLK_RESET_MSDC_SW_0);
		break;
	case e_CLK_MSDC_SW_1:
		//u4Tmp = u4Tmp & (~CLK_PDN_MSDC_SW_1);
		//u4Reset = u4Reset & (~CLK_RESET_MSDC_SW_1);
		break;
	case e_CLK_MSDC_SW_2:
		//u4Tmp = u4Tmp & (~CLK_PDN_MSDC_SW_2);
		//u4Reset = u4Reset & (~CLK_RESET_MSDC_SW_2);
		break;
            case e_CLK_SPI_MOTO1:
                u4Tmp = u4Tmp & (~CLK_PDN_SPI_MOTO1);
                //u4Reset = u4Reset & (~CLK_RESET_SPI_MOTO1);
                break;
            case e_CLK_SPI_MOTO2:
                u4Tmp = u4Tmp & (~CLK_PDN_SPI_MOTO2);
                //u4Reset = u4Reset & (~CLK_RESET_SPI_MOTO2);
                break;
            case e_CLK_PWM0:
                u4Tmp = u4Tmp & (~CLK_PDN_PWM0);
                //u4Reset = u4Reset & (~CLK_RESET_PWM0);
                break;
            case e_CLK_PWM1:
                u4Tmp = u4Tmp & (~CLK_PDN_PWM1);
                //u4Reset = u4Reset & (~CLK_RESET_PWM1);
                break;
            case e_CLK_PWM2:
                u4Tmp = u4Tmp & (~CLK_PDN_PWM2);
                //u4Reset = u4Reset & (~CLK_RESET_PWM2);
                break;
            case e_CLK_PWM3:
                u4Tmp = u4Tmp & (~CLK_PDN_PWM3);
                //u4Reset = u4Reset & (~CLK_RESET_PWM3);
                break;
            case e_CLK_SIFM0:
                u4Tmp = u4Tmp & (~CLK_PDN_SIFM0);
                //u4Reset = u4Reset & (~CLK_RESET_SIFM0);
                break;
            case e_CLK_SIFM1:
                u4Tmp = u4Tmp & (~CLK_PDN_SIFM1);
                //u4Reset = u4Reset & (~CLK_RESET_SIFM1);
                break;
            case e_CLK_SIFS0:
                u4Tmp = u4Tmp & (~CLK_PDN_SIFS0);
                //u4Reset = u4Reset & (~CLK_RESET_SIFS0);
                break;
            case e_CLK_SIFS1:
                u4Tmp = u4Tmp & (~CLK_PDN_SIFS1);
                //u4Reset = u4Reset & (~CLK_RESET_SIFS1);
                break;
            default:
                return FALSE;
        }
        //CKGEN_WRITE32(REG_RW_SYNC_RESET_CFG3, u4Reset);
        CKGEN_WRITE32(REG_RW_CLKGATE_CFG3, u4Tmp);
    } else if (eAgt < e_CLK_LVDS) {    // CONFIG 3:
        u4Tmp = CKGEN_READ32(REG_RW_CLKGATE_CFG4);
        //u4Reset = CKGEN_READ32(REG_RW_SYNC_RESET_CFG4);
        switch (eAgt) {
            case e_CLK_MVDO:
                u4Tmp = u4Tmp & (~CLK_PDN_MVDO);
                //u4Reset = u4Reset & (~CLK_RESET_MVDO);
                break;
            case e_CLK_DGO:
                u4Tmp = u4Tmp & (~CLK_PDN_DGO);
                //u4Reset = u4Reset & (~CLK_RESET_DGO);
                break;
            case e_CLK_DACTST:
                u4Tmp = u4Tmp & (~CLK_PDN_DACTST);
                //u4Reset = u4Reset & (~CLK_RESET_DACTST);
                break;
            case e_CLK_DVD_OSD:
                u4Tmp = u4Tmp & (~CLK_PDN_DVD_OSD);
                //u4Reset = u4Reset & (~CLK_RESET_DVD_OSD);
                break;
            case e_CLK_GRA:
                u4Tmp = u4Tmp & (~CLK_PDN_GRA);
                //u4Reset = u4Reset & (~CLK_RESET_GRA);
                break;
            case e_CLK_BIM:
                u4Tmp = u4Tmp & (~CLK_PDN_BIM);
                //u4Reset = u4Reset & (~CLK_RESET_BIM);
                break;
            case e_CLK_TURBO32:
                u4Tmp = u4Tmp & (~CLK_PDN_TURBO32);
                //u4Reset = u4Reset & (~CLK_RESET_TURBO32);
                break;
            case e_CLK_VDEC:
                u4Tmp = u4Tmp & (~CLK_PDN_VDEC);
                //u4Reset = u4Reset & (~CLK_RESET_VDEC);
                break;
            case e_CLK_PARSER:
                u4Tmp = u4Tmp & (~CLK_PDN_PARSER);
                //u4Reset = u4Reset & (~CLK_RESET_PARSER);
                break;
            case e_CLK_RAMBUF:
                u4Tmp = u4Tmp & (~CLK_PDN_RAMBUF);
                //u4Reset = u4Reset & (~CLK_RESET_RAMBUF);
                break;
            case e_CLK_PT110:
                u4Tmp = u4Tmp & (~CLK_PDN_PT110);
                //u4Reset = u4Reset & (~CLK_RESET_PT110);
                break;
            case e_CLK_RS232:
                u4Tmp = u4Tmp & (~CLK_PDN_RS232);
                //u4Reset = u4Reset & (~CLK_RESET_RS232);
                break;
            case e_CLK_CDDVD:
                u4Tmp = u4Tmp & (~CLK_PDN_CDDVD);
                //u4Reset = u4Reset & (~CLK_RESET_CDDVD);
                break;
            case e_CLK_AUDIO:
                u4Tmp = u4Tmp & (~CLK_PDN_AUDIO);
                //u4Reset = u4Reset & (~CLK_RESET_AUDIO);
                break;
            case e_CLK_SERVO_MISC:
                u4Tmp = u4Tmp & (~CLK_PDN_SERVO_MISC);
                //u4Reset = u4Reset & (~CLK_RESET_SERVO_MISC);
                break;
	case e_CLK_RAMBUF_APCTRL_TBUS3:
                u4Tmp = u4Tmp & (~CLK_PDN_RAMBUF_APCTRL_TBUS3);
                //u4Reset = u4Reset | (CLK_RESET_RAMBUF_APCTRL_TBUS3);
                break;
	case e_CLK_RAMBUF_APCTRL_TBUS4:
                u4Tmp = u4Tmp & (~CLK_PDN_RAMBUF_APCTRL_TBUS4);
                //u4Reset = u4Reset | (CLK_RESET_RAMBUF_APCTRL_TBUS4);
                break;
	case e_CLK_RAMBUF_APCTRL_TBUS5:
                u4Tmp = u4Tmp & (~CLK_PDN_RAMBUF_APCTRL_TBUS5);
                //u4Reset = u4Reset | (CLK_RESET_RAMBUF_APCTRL_TBUS5);
                break;
	case e_CLK_MFG_TOP_PWR_WRAP:
		u4Tmp = u4Tmp | (CLK_PDN_MFG_TOP_PWR_WRAP);
		//u4Reset = u4Reset & (~CLK_RESET_MFG_TOP_PWR_WRAP);
		break;
            default:
                return FALSE;
        }
        //CKGEN_WRITE32(REG_RW_SYNC_RESET_CFG4, u4Reset);
        CKGEN_WRITE32(REG_RW_CLKGATE_CFG4, u4Tmp);
    } else if (eAgt < e_CLK_SCLER) {    // CONFIG 3:
        u4Tmp = CKGEN_READ32(REG_RW_CLKGATE_CFG5);
        //u4Reset = CKGEN_READ32(REG_RW_SYNC_RESET_CFG5);
        switch (eAgt) {
            case e_CLK_LVDS:
                u4Tmp = u4Tmp & (~CLK_PDN_CLK_LVDS);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_LVDS);
                break;
	case e_CLK_TP_TOP0:
                u4Tmp = u4Tmp & (~CLK_PDN_CLK_TP_TOP0);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
                break;
	case e_CLK_TP_TOP1:
                u4Tmp = u4Tmp & (~CLK_PDN_CLK_TP_TOP1);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
                break;
	case e_CLK_TP_TOP2:
                u4Tmp = u4Tmp  & (~CLK_PDN_CLK_TP_TOP2);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
                break;
	case e_CLK_RFI_TOP1:
                u4Tmp = u4Tmp & (~CLK_PDN_CLK_RFI_TOP1);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
                break;
	case e_CLK_RFI_TOP2:
                u4Tmp = u4Tmp & (~CLK_PDN_CLK_RFI_TOP2);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
                break;
	case e_CLK_RFI_TOP3:
                u4Tmp = u4Tmp & (~CLK_PDN_CLK_RFI_TOP3);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
                break;
	case e_CLK_RFI_TOP4:
                u4Tmp = u4Tmp & (~CLK_PDN_CLK_RFI_TOP4);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
                break;
	case e_CLK_RFI_TOP5:
		u4Tmp = u4Tmp & (~CLK_PDN_CLK_RFI_TOP5);
		//u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
		break;
	case e_CLK_RFI_TOP6:
                u4Tmp = u4Tmp & (~CLK_PDN_CLK_RFI_TOP6);
                //u4Reset = u4Reset & (~CLK_RESET_CLK_TP);
                break;
            default:
                return FALSE;
        }
        //CKGEN_WRITE32(REG_RW_SYNC_RESET_CFG5, u4Reset);
        CKGEN_WRITE32(REG_RW_CLKGATE_CFG5, u4Tmp);
    } else if (eAgt < e_CLK_MAX) {    // CONFIG 6:
        u4Tmp = CKGEN_READ32(REG_RW_CLKGATE_CFG6);
        //u4Reset = CKGEN_READ32(REG_RW_SYNC_RESET_CFG6);
        switch (eAgt) {
            case e_CLK_SCLER:
                u4Tmp = u4Tmp & (~CLK_PDN_SCLER);
                //u4Reset = u4Reset & (~CLK_RESET_SCLER);
                break;
            case e_CLK_TVD1:
                u4Tmp = u4Tmp & (~CLK_PDN_TVD1);
                //u4Reset = u4Reset & (~CLK_RESET_TVD1);
                break;
            case e_CLK_TVD2:
                u4Tmp = u4Tmp & (~CLK_PDN_TVD2);
                //u4Reset = u4Reset & (~CLK_RESET_TVD1);  //?
                break;
            case e_CLK_OSD:
                u4Tmp = u4Tmp & (~CLK_PDN_OSD);
                //u4Reset = u4Reset & (~CLK_RESET_OSD);
                break;
            case e_CLK_OSD_R:
                u4Tmp = u4Tmp & (~CLK_PDN_OSD_R);
                //u4Reset = u4Reset & (~CLK_RESET_OSD_R);
                break;
            case e_CLK_FPD:
                u4Tmp = u4Tmp & (~CLK_PDN_FPD);
                //u4Reset = u4Reset & (~CLK_RESET_FPD);
                break;
            case e_CLK_FMT_VDO_F:
                u4Tmp = u4Tmp & (~CLK_PDN_FMT_VDO_F);
                //u4Reset = u4Reset & (~CLK_RESET_FMT_VDO_F);
                break;
            case e_CLK_FMT_VDO_R:
                u4Tmp = u4Tmp & (~CLK_PDN_FMT_VDO_R);
                //u4Reset = u4Reset & (~CLK_RESET_FMT_VDO_R);
                break;
            case e_CLK_WRITE_CHANEL:
                u4Tmp = u4Tmp & (~CLK_PDN_WRITE_CHANEL);
                //u4Reset = u4Reset & (~CLK_RESET_WRITE_CHANEL);
                break;
            case e_CLK_FRAME_LOCK:
                u4Tmp = u4Tmp & (~CLK_PDN_FRAME_LOCK);
                //u4Reset = u4Reset & (~CLK_RESET_FRAME_LOCK);
                break;
	case e_CLK_WRITE_CHANEL_2:
		u4Tmp = u4Tmp & (~CLK_PDN_WRITE_CHANEL2);
		//u4Reset = u4Reset & (~CLK_RESET_WRITE_CHANEL2);
		break;
	case e_CLK_VGA:
		u4Tmp = u4Tmp & (~CLK_PDN_VGA_EDID);
		//u4Reset = u4Reset & (~CLK_RESET_VGA_EDID);
		break;
	case e_CLK_YPBPR:
		u4Tmp = u4Tmp & (~CLK_PDN_YPBPR_VGA);
		//u4Reset = u4Reset & (~CLK_RESET_YPBPR_VGA);
		break;
	case e_CLK_HDMI:
		u4Tmp = u4Tmp & (~CLK_PDN_HDMI);
		//u4Reset = u4Reset & (~CLK_RESET_HDMI);
		break;
	case e_CLK_TVE:
		u4Tmp = u4Tmp & (~CLK_PDN_TVE);
		//u4Reset = u4Reset & (~CLK_RESET_TVE);
		break;
	case e_CLK_DVD_MIX_2AP:
		u4Tmp = u4Tmp & (~CLK_PDN_DVD_MIX_2AP);
		//u4Reset = u4Reset & (~CLK_RESET_DVD_MIX_2AP);
		break;
	case e_CLK_OSD_1:
		u4Tmp = u4Tmp & (~CLK_PDN_OSD1);
		//u4Reset = u4Reset | (CLK_RESET_FRAME_LOCK);
		break;
	case e_CLK_OSD_2:
		u4Tmp = u4Tmp & (~CLK_PDN_OSD2);
		//u4Reset = u4Reset | (CLK_RESET_FRAME_LOCK);
		break;
	case e_CLK_OSD_3:
		u4Tmp = u4Tmp & (~CLK_PDN_OSD3);
		//u4Reset = u4Reset | (CLK_RESET_FRAME_LOCK);
		break;
	case e_CLK_OSD_4:
		u4Tmp = u4Tmp & (~CLK_PDN_OSD4);
		//u4Reset = u4Reset | (CLK_RESET_FRAME_LOCK);
		break;
	case e_CLK_OSD_5:
		u4Tmp = u4Tmp & (~CLK_PDN_OSD5);
		//u4Reset = u4Reset | (CLK_RESET_FRAME_LOCK);
		break;
	case e_CLK_OSD_R_2:
		u4Tmp = u4Tmp & (~CLK_PDN_OSD_R2);
		//u4Reset = u4Reset | (CLK_RESET_FRAME_LOCK);
		break;
	case e_CLK_OSD_R_3:
		u4Tmp = u4Tmp & (~CLK_PDN_OSD_R3);
		//u4Reset = u4Reset | (CLK_RESET_DVD_MIX_2AP);
		break;
	case e_CLK_SCLER_TG:
		u4Tmp = u4Tmp & (~CLK_PDN_SCLER_TG);
		//u4Reset = u4Reset & (~CLK_RESET_SCLER_TG);
		break;
	case e_CLK_LCPROC_VDO:
		u4Tmp = u4Tmp & (~CLK_PDN_LCPROC_VDO);
		//u4Reset = u4Reset & (~CLK_RESET_ICPROC_VDO);
		break;
            default:
                return FALSE;
        }
        //CKGEN_WRITE32(REG_RW_SYNC_RESET_CFG6, u4Reset);
        CKGEN_WRITE32(REG_RW_CLKGATE_CFG6, u4Tmp);
    } else {
        return FALSE;
    }

    return TRUE;
}


//EXPORT_SYMBOL(CKGEN_AgtOffClk_NoReset);

