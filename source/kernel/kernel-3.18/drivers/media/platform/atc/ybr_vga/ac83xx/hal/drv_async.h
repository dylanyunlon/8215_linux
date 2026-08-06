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

#ifndef DRV_ASYNC_H__
#define DRV_ASYNC_H__

#include "ybr_vga_util.h"
#include "drv_hdtv.h"
#include "drv_vga.h"
#include <linux/types.h>

#define wSP2VCompare_Num 2

// SP0 Extern  
extern u16   wSP0Hclk;
extern u8   bSP0Vclk;
extern u16   wSP0Vtotal;
extern u16  _wSP0StableVtotal;
extern u16   wSP0HLength;
extern u8   bSP0VCount;
extern u8   bSP0SigChk;
extern u8   fgSP0DIChk;
extern u8   fgSP0Hpol;
extern u8   fgSP0Vpol;
extern u16   wSP0VCompare[4];
extern u16   wSP0HCompare1;
extern u8  _bSP0Flag ;

extern u8   bSP0TrySignalState;
extern u32 wVGAADSpec;
extern u32 wHFHeight;
extern u32 wHFLow;
extern u32 wVFHeight;
extern u32 wVFLow;
extern u8 bModeIndex;


// SP0 Event Flag
#define vSetSP0Flg(arg) (_bSP0Flag |= (u8)(arg))
#define vClrSP0Flg(arg) (_bSP0Flag &= (u8)(~(arg)))
#define fgIsSP0FlgSet(arg) ((_bSP0Flag & (u8)(arg)) > 0)

#define SP0_MODE_DET_FLG (u32)(1U<<0)
#define SP0_VGA_AUTO_FLG (u32)(1U<<1)
#define SP0_AUTOCOLOR_FLG (u32)(1U<<2)
#define SP0_MCHG_BYPASS_FLG (u32)(1U<<7)


extern void vDrvAsyncMvDetectH (u16 wstart1, u16 wend1) ;
extern void vDrvAsyncMvDetectV (u16 wstart1, u16 wend1)  ;
extern u8 bDrvAsyncMvStatus (void)  ;
extern u8 _bHdtvMvOn ;

void vDrvAsyncHsyncEnable(u8 bEnable);


enum{   
            HDTV_NO_SIGNAL, 
            HDTV_CHK_MODECHG,   
            HDTV_WAIT_STABLE
         };

enum{   
            VGA_NO_SIGNAL,  
            VGA_CHK_MODECHG,    
            VGA_WAIT_STABLE
         };


enum{   
            MCHG_NO_CHG = 0,
            MDCHG_CON =1,   
            MCHG_NOSIG =2 ,  
            MCHG_SIGIN = 3,
            MCHG_UNLOCK =4,
            MCHG_ISR =5,
            MCHG_HVSYNC_LOSE =6,
            MCHG_POL_CHG =7,
            MCHG_HVLEN_CHG =8,
            MCHG_FLD_CHG =9,
            MCHG_HW_DET =10,
            MCHG_VGA_422=11
         };   


/*=====================================================================
      HARDWARE MUTE TYPE
  ====================================================================*/
#define AS_MUTE_HSACT (u32)(1U<<6)
#define AS_MUTE_VSACT (u32)(1U<<5)
#define AS_MUTE_CSACT (u32)(1U<<4)
#define AS_MUTE_HSPOL (u32)(1U<<3)
#define AS_MUTE_VSPOL (u32)(1U<<2)
#define AS_MUTE_HLEN  (u32)(1U<<1)
#define AS_MUTE_VLEN  (u32)(1U<<0)

/* ====================================================================
      SYNC TYPE SELECT
  ====================================================================*/
 #define SEPERATESYNC       0
 #define COMPOSITESYNC     (u32)1
 #define SYNCONGREEN        (u32)2

/*=====================================================================
            CS SEPARATOR THRESHOLD SELTECT
  ====================================================================*/
#define CS_SEPARATOR_THU_DEFAULT   0x01a4U
#define CS_SEPARATOR_THL_DEFAULT   0x00ffU
#define CS_SEPARATOR_THU_DEFAULT_1080   0x01c4U
#define CS_SEPARATOR_THL_DEFAULT_1080   0x00f0U
#define CS_SEPARATOR_THH_DEFAULT    0x19cU

#define vASPathReset()  vIO32WriteFldAlign(ASYNC_00,SEPERATESYNC,AS_SYNC_SEL)
#define vASSetSSync()   vIO32WriteFldAlign(ASYNC_00,SEPERATESYNC,AS_SYNC_SEL)
#define vASSetCSync()   vIO32WriteFldAlign(ASYNC_00,COMPOSITESYNC,AS_SYNC_SEL)
#define vASSetSOGSync() vIO32WriteFldAlign(ASYNC_00,SYNCONGREEN,AS_SYNC_SEL)



#define vDrvCsyncInvPol(bPol) vIO32WriteFldAlign(ASYNC_00, (bPol), AS_CSYNC_INV)
#define vDrvHsLockInv(bOn) vIO32WriteFldAlign(ASYNC_0E, (bOn), AS_HSYNC_LOCK_INV)
#define vDrvHsInv(bOn) vIO32WriteFldAlign(ASYNC_00, (bOn), AS_HSYNC_INV)
#define vDrvVsInv(bOn) vIO32WriteFldAlign(ASYNC_00, (bOn), AS_VSYNC_INV)
#define vDrvVsOutInvPol(bPol) vIO32WriteFldAlign(ASYNC_11,(bPol), AS_VSYNC_OUTP_INV)

#define bDrvASFieldAct()  (IO32ReadFldAlign(STA_SYNC0_00, AS_FIELD_ACT))
#define bDrvASFieldNum()  (IO32ReadFldAlign(STA_SYNC0_00, AS_FIELD_CK27_DET))

#define fgIsCLKLock() (IO32ReadFldAlign(VFE_STA_00,DDS_LOCK))
#define vDDS_MAX_PERR() (IO32ReadFldAlign(VFE_STA_00,DDS_MAX_PERR))


/*======================================================
              crystal number
  ====================================================*/
#define CRYSTAL_27M               27
#define CRYSTAL_48M                48

#define CRYSTAL                    CRYSTAL_27M


/*======================================================
              H/V Sync Internal Delay
  ====================================================*/
#define HSync_Auto_Delay 8


typedef struct // SP2MonStr
{
    u16 SP2_V_LEN_S;
    u16 SP2_H_LEN_S;
    u16 SP2_V_WIDTH_S;
}  SP2MonStr;

typedef enum
{
    SP2_Specific_LVL,
    SP2_Default_LVL
}  SP2Mon_LVL;



enum 
{
    ZERO,
    NOT_FINISHED,
    FINISHED,
    VALID,
    INVALID
};


enum 
{
    DOMAIN_27MHz,
    DOMAIN_PIXEL
};


#define VGA_DBG_MSG_DMP 1
extern u8   bVgadbgmask;

#define PRE_DOWN_OFST   2



//////////////////////////////////////////////////////////////////////////////////
extern void vDrvAsyncMvDetectH (u16 wstart1, u16 wend1);
extern void vDrvAsyncMvDetectV (u16 wstart1, u16 wend1);
extern u8 bDrvAsyncMvStatus (void);
extern u8 _bHdtvMvOn ;
void vDrvAsyncHsyncEnable(u8 bEnable);

extern void vResetVLen(void) ;

extern u8 fgASHPolarityMeasure(void) ;
extern u8 fgASVPolarityMeasure(void) ;
extern u16 wASHLenMeasure(void) ;
extern u16 wASHSyncWidthMeasure(void);
extern u16 wASVtotalMeasure(void) ;

#define wSP2VCompare_Num 2
extern SP2MonStr wSP2VCompare[wSP2VCompare_Num];
extern void vAS2SyncMeasure(SP2Mon_LVL type);

extern u16 wASTopBCLine(void);
extern u16 wASBottomBCLine(void);
extern u8 bASActiveChk(void) ;
extern u8 bASHDTVActiveChk(void);
extern void vASCSSeparatorThre(void);
extern void vSetAsyncMeasureBD(u8 bmode) ;
extern void vDrvAsyncVMask(u16 wstart1, u16 wend1) ;
extern void vDrvAsyncVMask2(u16 wstart1, u16 wend1) ;
extern void vDrvAsyncClampMask(u16 wstart1, u16 wend1) ;
extern void vDrvAsyncPreDataActive(u16 wstart,u16 wend);
extern void vDrvAsyncHBDMask(u16 Left, u16 Right);
extern void vDrvAsyncBDMask(u16 wstart1, u16 wend1)  ;
extern void  vDrvAsyncSetFieldDet(u16 wHLEN) ;

extern u8 bDrvAsyncMvStatus(void);

extern void vDrvAsyncVsyncStart(u16 wStart) ;
extern void vDrvAsyncVsyncOut(u16 wStart1, u16 wH1);

extern u16 wSP0IHSClock(u16 whlen) ;
extern u8 bSP0IVSClock(u16 whtotal, u16 wvtotal) ;
extern u8 bDrvAsGetActive(u8 VSyncNo) ;
extern u8 bDrvAsGetVtotal(void) ;
extern u8  fgSP0NewSync(void) ;

extern void SP0Initial(void) ;
extern u8 SP0SignalIdentify(u8 SigMode) ;
extern u8 bSP0VsyncValid(void)  ;
extern u16 wASHSyncWidthMeasure(void) ;
extern u16 wASVSyncWidthMeasure(void);
//extern u16 wDrvAsyncGetVactive (void) ;
extern void vASDeCompSel(u8 DeCompSel);


void vDrvAsyncSetMuteCriteria(u32 u4MuteFlag);


extern void vDrvRETIMEReset(void);

u8 bASGetSyncMode(void);
u8 bDrvASVsyncOutAct(void);
u16 wASHLenPixMeasure(void);


#endif
