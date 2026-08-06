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
#include "vdec_info_h264.h"
#include "vdec_hw_h264.h"
#include "x_hal_ic.h"

void vVDecWriteAVCVLD(UINT32 u4VDecID, UINT32 u4Addr, UINT32 u4Val)
{
     u4VDecID = 0;

    if (u4VDecID == 0)
    {
        vWriteReg(AVC_VLD_REG_OFFSET0 + u4Addr, u4Val);
    }
    else
    {
        vWriteReg(AVC_VLD_REG_OFFSET1 + u4Addr, u4Val);
    }
}

UINT32 u4VDecReadAVCVLD(UINT32 u4VDecID, UINT32 u4Addr)
{
     u4VDecID = 0;

    if (u4VDecID == 0)
    {
        UINT32 u4Val = u4ReadReg(AVC_VLD_REG_OFFSET0 + u4Addr);
        return u4Val;
    }
    else
    {
        return (u4ReadReg(AVC_VLD_REG_OFFSET1 + u4Addr));
    }
}

// *********************************************************************
// Function : UINT32 dVLDGetBitS(UINT32 u4BSID,UINT32 u4VDecID,UINT32 dShiftBit)
// Description : Get Bitstream from VLD barrel shifter
// Parameter : dShiftBit: Bits to shift (0-32)
// Return    : barrel shifter
// *********************************************************************
UINT32 u4VDecAVCVLDGetBitS(UINT32 u4BSID,UINT32 u4VDecID,UINT32 dShiftBit)
{
    UINT32 u4RegVal;

    u4VDecID = 0;

    if (u4BSID == 0)
    {
        u4RegVal = u4VDecReadAVCVLD(u4VDecID, RO_VLD_AV_BARL  + (dShiftBit << 2));
    }
    else
    {
        u4RegVal = u4VDecReadAVCVLD(u4VDecID, RO_VLD_AV_2ND_BARL  + (dShiftBit << 2));
    }
    return (u4RegVal);
}

// *********************************************************************
// Function : UINT32 u4VDecAVCVLDShiftBits(UINT32 u4BSID, UINT32 u4VDecID)
// Description : Get AVCVLD shift bits %64
// Parameter : None
// Return    : VLD Sum
// *********************************************************************
UINT32 u4VDecAVCVLDShiftBits(UINT32 u4BSID, UINT32 u4VDecID)
{
    u4VDecID = 0;

    if (u4BSID==0)
    {
        return ((u4VDecReadAVCVLD(u4VDecID, RW_VLD_AV_CTRL) >> 16) & 0x3F);
    }
    else
    {
        return (u4VDecReadAVCVLD(u4VDecID, RW_VLD_AV_2ND_CTRL) & 0x3F);
    }
}

// *********************************************************************
// Function : BOOL fgH264VLDInitBarrelShifter1(UINT32 u4BSID, UINT32 u4VDecID, UINT32 u4VLDRdPtr, UINT32 u4VLDWrPtr)
// Description : Init HW Barrel Shifter
// Parameter : u4Ptr: Physical DRAM Start Address to fill Barrel Shifter
// Return    : TRUE: Initial Success, Fail: Initial Fail
// *********************************************************************
#ifdef H264_THRESHOLD_ENABLE
BOOL fgH264VLDInitBarrelShifter(UINT32 u4BSID, UINT32 u4VDecID, UINT32 u4FIFOSa, UINT32 u4VLDRdPtr, UINT32 u4VLDWrPtr, UINT32 u4VLDThreshold)
#else
BOOL fgH264VLDInitBarrelShifter(UINT32 u4BSID, UINT32 u4VDecID, UINT32 u4FIFOSa, UINT32 u4VLDRdPtr, UINT32 u4VLDWrPtr)
#endif
{
    UINT32 u4ByteAddr;
    UINT32 u4TgtByteAddr;
    //UINT32 u4Bits;
    INT32 i;
    BOOL fgFetchOK = FALSE;
    UINT32 u4Cnt;
    // prevent initialize barrel fail
    for (i = 0; i < 5; i++)
    {
      u4Cnt = 50000;
      if (u4VDecReadVLD(u4VDecID, RO_VLD_SRAMCTRL) & (1<<15))
      {
      	while((!(u4VDecReadVLD(u4VDecID, RO_VLD_SRAMCTRL)&0x1)) && (u4Cnt--));
      }

        vVDecWriteVLD(u4VDecID, WO_VLD_WPTR, u4VDecReadVLD(u4VDecID, WO_VLD_WPTR) | VLD_CLEAR_PROCESS_EN);
        vVDecWriteVLD(u4VDecID, RW_VLD_RPTR + (u4BSID << 10), u4VLDRdPtr);
        vVDecWriteVLD(u4VDecID, RW_VLD_RPTR + (u4BSID << 10), u4VLDRdPtr);

  vVDecWriteVLD(u4VDecID, WO_VLD_WPTR + (u4BSID << 10), u4VLDWrPtr);
#if defined(CC_MT3360)
  vVDecWriteVLD(u4VDecID, RW_VLD_ASYNC + (u4BSID << 10), u4VDecReadVLD(u4VDecID, RW_VLD_ASYNC) | VLD_WR_ENABLE);
#endif
        if (u4BSID == 0)
        {
            vVDecWriteAVCVLD(u4VDecID, RW_AVLD_RESET_SUM, AVLD_RESET_SUM_ON);
        }
        vVDecWriteVLD(u4VDecID, WO_VLD_SRST , 1 << 8);
        vVDecWriteVLD(u4VDecID, WO_VLD_SRST , 0);
        // start to fetch data
        vVDecWriteVLD(u4VDecID, RW_VLD_PROC + (u4BSID << 10), VLD_INIFET);
        if (vVDec_HAL_COMMON_WaitVldFetchOk(u4BSID, u4VDecID))
        {
            fgFetchOK = TRUE;
            break;
        }
    }

    if (!fgFetchOK)
    {
        Printf("[AVC SLT] fgH264VLDInitBarrelShifter FAIL\n");
        return (FALSE);
    }

    //mb();
    vVDecWriteVLD(u4VDecID, RW_VLD_PROC + (u4BSID << 10), VLD_INIBR);
    //mb();
    if (u4BSID == 0)
    {
      // HW workaround
      // can not reset sum off until barrel shifter finish initialization
        vVDecWriteAVCVLD(u4VDecID, RW_AVLD_RESET_SUM, AVLD_RESET_SUM_OFF);
    }

    //while (u4VDecReadH264VldRPtr(u4BSID, u4VDecID, &u4Bits, u4FIFOSa) < (u4VLDRdPtr - u4FIFOSa))
    //{
    //    u4VDecAVCVLDGetBitS(u4BSID, u4VDecID, 8);
    //}

    // move range 0~15 bytes
    u4TgtByteAddr = u4VLDRdPtr & 0xf;
    u4ByteAddr = u4VLDRdPtr & 0xfffffff0;
    while (u4TgtByteAddr)
    {
        u4TgtByteAddr --;
        if ( ((u4VDecReadAVCVLD(u4VDecID, RW_AVLD_RM03R)&RO_ALVD_FIND_03)) && u4TgtByteAddr)
        {
            u4TgtByteAddr --;
        }
        u4ByteAddr ++;
        u4VDecAVCVLDGetBitS(u4BSID, u4VDecID, 8);
    }

    return (TRUE);
}


// *********************************************************************
// Function : BOOL fgInitH264BarrelShift1(UINT32 u4VDecID, VDEC_INFO_H264_BS_INIT_PRM_T *prH264BSInitPrm)
// Description : Reset VLD2
// Parameter : None
// Return    : None
// *********************************************************************
BOOL fgInitH264BarrelShift1(UINT32 u4VDecID, VDEC_HAL_H264_BS_INIT_PRM_T *prH264BSInitPrm)
{
    vVDecWriteVLD(u4VDecID,RW_VLD_RDY_SWTICH, READY_TO_RISC_1);

    vVDecWriteAVCVLD(u4VDecID, RW_VLD_AV_CTRL, AVC_EN | AVC_RDY_WITH_CNT | AVC_RDY_CNT_THD | AVLD_MEM_PAUSE_MOD_EN
    | AVC_SUM6_APPEND_INV |AVC_NOT_CHK_DATA_VALID | AVC_RBSP_CHK_INV | AVC_ERR_BYPASS );
    vVDecWriteVLDTOP(u4VDecID, RW_VLD_TOP_TIMEOUT_SW, u4VDecReadVLDTOP(u4VDecID, RW_VLD_TOP_TIMEOUT_SW)|VLD_TOP_DEC_CYCLE_EN);

    vVDecWriteMC(u4VDecID, RW_MC_OPBUF, 6);
    vVDec_HAL_COMMON_SetVLDFIFO(0, u4VDecID, (prH264BSInitPrm->u4VFifoSa), ((UINT32) prH264BSInitPrm->u4VFifoEa));
    vVDecWriteVLDTOP(u4VDecID, RW_VLD_TOP_PRED_ADDR, (prH264BSInitPrm->u4PredSa ? ((UINT32) prH264BSInitPrm->u4PredSa) : 0));
    // Reset AVC VLD Sum
    vVDecWriteAVCVLD(u4VDecID, RW_AVLD_RESET_SUM, AVLD_RESET_SUM_ON);
    vVDecWriteAVCVLD(u4VDecID, RW_AVLD_RESET_SUM, AVLD_RESET_SUM_OFF);

    if (!fgH264VLDInitBarrelShifter(0, u4VDecID, ((UINT32) prH264BSInitPrm->u4VFifoSa), ((UINT32) prH264BSInitPrm->u4VLDRdPtr), (prH264BSInitPrm->u4VLDWrPtr)))
    {
        Printf("[AVC SLT] fgInitH264BarrelShift1 FAIL\n");
        return FALSE;
    }
    return TRUE;
}

// *********************************************************************
// Function : BOOL fgInitH264BarrelShift2(UINT32 u4RDPtrAddr)
// Description : Reset VLD2
// Parameter : None
// Return    : None
// *********************************************************************
BOOL fgInitH264BarrelShift2(UINT32 u4VDecID, VDEC_HAL_H264_BS_INIT_PRM_T *prH264BSInitPrm)
{
    // reset barrel shifter 2
    while (!(u4VDecReadVLD(u4VDecID, RO_VLD_SRAMCTRL | (1<<10))&0x10000));
    vVDecWriteVLD(u4VDecID, WO_VLD_SRST , 1 << 8);
    vVDecWriteVLD(u4VDecID, WO_VLD_SRST , 0);

    vVDecWriteVLD(u4VDecID,RW_VLD_2ND_RDY_SWTICH, READY_TO_RISC_1);

    // temporarily workaround, will be fixed by ECO
    vVDecWriteAVCVLD(u4VDecID, RW_VLD_AV_CTRL, 1 << 30);
    vVDecWriteAVCVLD(u4VDecID,RW_AVLD_2ND_BARL_CTRL, AVLD_2ND_BARL_EN);
    vVDec_HAL_COMMON_SetVLDFIFO(1, u4VDecID, (prH264BSInitPrm->u4VFifoSa), ((UINT32) prH264BSInitPrm->u4VFifoEa));

    if (!fgH264VLDInitBarrelShifter(1, u4VDecID, ((UINT32) prH264BSInitPrm->u4VFifoSa), ((UINT32) prH264BSInitPrm->u4VLDRdPtr), (prH264BSInitPrm->u4VLDWrPtr)))
    {
        return FALSE;
    }
    return TRUE;
}

UINT32 u4VDecReadH264VldRPtr(UINT32 u4BSID, UINT32 u4VDecID, UINT32 *pu4Bits, UINT32 u4VFIFOSa)
{
    UINT32 u4DramRptr=0;
    UINT32 u4SramRptr=0, u4SramWptr=0;
    UINT32 u4SramDataSz=0;
    UINT32 u4ByteAddr =0;
    UINT32 u4Cnt = 0;
    // HW issue, wait for read pointer stable
      u4Cnt = 50000;
    if (u4VDecReadVLD(u4VDecID, RO_VLD_SRAMCTRL) & (1<<15))
    {
        while((!(u4VDecReadVLD(u4VDecID, RO_VLD_SRAMCTRL)&0x1)) && (u4Cnt--));
    }

    if (0 == u4Cnt)
    {
        UINT32 u4VEnd = (u4VDecReadVLD(u4VDecID, RW_VLD_VEND + (u4BSID << 10)) << 6);
    }

  u4DramRptr = u4VDecReadVLD(u4VDecID, RO_VLD_VRPTR + (u4BSID << 10));
//  u4SramRptr = (u4VDecReadVLD(u4VDecID, RO_VLD_VBAR + (u4BSID << 10)) & 0xf) * 4; //count in 128bits
  u4SramRptr = ((u4VDecReadVLD(u4VDecID, RO_VLD_VBAR + (u4BSID << 10))) & 0xf) * 4 +
                     (((u4VDecReadVLD(u4VDecID, RO_VLD_VBAR + (u4BSID << 10))>>24)) & 0x3); //count in 128bits
  //u4SramRptrIn32Bits = 4 - ((u4VDecReadVLD(u4VDecID, RO_VLD_VBAR + (u4BSID << 10))>>24) & 0x3); //count in 32bits
  u4SramWptr = (((u4VDecReadVLD(u4VDecID, RO_VLD_VBAR + (u4BSID << 10))>>8)) & 0xf) *4;
  if (u4SramWptr > u4SramRptr)
  {
    u4SramDataSz = u4SramWptr - u4SramRptr;  // 128bits
  }
  else
  {
    u4SramDataSz = 64 - (u4SramRptr - u4SramWptr);
  }

  //*pu4Bits = u4VDecReadVLD(u4VDecID, RO_VLD_SUM)& 0x3f;
  *pu4Bits = u4VDecAVCVLDShiftBits(u4BSID, u4VDecID);

  u4ByteAddr = u4DramRptr - ((u4SramDataSz + 6) * 4) + ((*pu4Bits) / 8);

  
 /* Printf("[AVC SLT] u4ByteAddr = 0x%x,u4DramRptr= 0x%x, u4SramDataSz = 0x%x, (*pu4Bits)= 0x%x\n", 
    u4ByteAddr, u4DramRptr, u4SramDataSz, (*pu4Bits));*/

  if (u4ByteAddr < u4VFIFOSa)
  {
    u4ByteAddr = u4ByteAddr +
                ((u4VDecReadVLD(u4VDecID, RW_VLD_VEND + (u4BSID << 10)) << 6)- ((UINT32)u4VFIFOSa))
                - u4VFIFOSa;
    
   // Printf("[AVC SLT] u4VDecReadH264VldRPtr--0 = 0x%x\n", u4ByteAddr);
  }
  else
  {
    u4ByteAddr -= ((UINT32)u4VFIFOSa);
    
   // Printf("[AVC SLT] u4VDecReadH264VldRPtr--1 = 0x%x\n", u4ByteAddr);
  }
  *pu4Bits &= 0x7;
 // Printf("[AVC SLT] u4VDecReadH264VldRPtr---2 = 0x%x\n", u4ByteAddr);

    return (u4ByteAddr);

}

