#include "drv_vdp.h"
#include "drv_vpq.h"
#include "drv_config.h"

#ifndef __BD_DRV_DCM_H_
#define __BD_DRV_DCM_H_

typedef enum
{
  H_MVDO0     = 0,
  H_MVDO1     = 1,
  H_NR        = 2,
  H_PP        = 3,
  H_DISPFMT0  = 4,
  H_DISPFMT1  = 5,
  H_VDOFMT0   = 6,
  H_VDOFMT1   = 7,
  H_MJC       = 8,
  H_MAX_HW    = 9,
  H_MVDO0_APQ = 10,
  H_MVDO1_APQ = 11, 
  H_MVDO0_ADV = 12,
  H_MVDO1_ADV = 13, 

  H_MVDO0_MEMA = 14,
  H_MVDO1_MEMA = 15, 
} DISP_MODULE_E;

typedef struct
{
  UINT32 Reg[64];
  UINT64 Mode;
} DCM_SW_REG_T;   

#define DCM_CHG_HW_ENABLE       ((UINT32)1 << 0)
#define DCM_CHG_REGION          ((UINT32)1 << 1)
#define DCM_CHG_RECONFIG_HAL    ((UINT32)1 << 2)
#define DCM_CHG_PICTURE         ((UINT32)1 << 3)
#define DCM_CHG_FIELD_FORWARD   ((UINT32)1 << 4)
#define DCM_CHG_DEINT_MODE      ((UINT32)1 << 5)

#define DCM_Q_DEPTH   16

typedef struct
{
#if CONFIG_DRV_ENABLE_DCM
  DCM_SW_REG_T  RegMVDO0[DCM_Q_DEPTH];
  DCM_SW_REG_T  RegMVDO1[DCM_Q_DEPTH];
  DCM_SW_REG_T  RegMVDO0Adv[DCM_Q_DEPTH];
  DCM_SW_REG_T  RegMVDO1Adv[DCM_Q_DEPTH];
  DCM_SW_REG_T  RegMVDO0Apq[DCM_Q_DEPTH];
  DCM_SW_REG_T  RegMVDO1Apq[DCM_Q_DEPTH];
  DCM_SW_REG_T  RegDISPFMT0[DCM_Q_DEPTH];
  DCM_SW_REG_T  RegDISPFMT1[DCM_Q_DEPTH];
  DCM_SW_REG_T  RegVDOFMT0[DCM_Q_DEPTH];
  DCM_SW_REG_T  RegVDOFMT1[DCM_Q_DEPTH];
  DCM_SW_REG_T  RegMVDO0Mema[DCM_Q_DEPTH];
  DCM_SW_REG_T  RegMVDO1Mema[DCM_Q_DEPTH];

  V_FRAME_INFO_T  VideoInfoQ[VDP_MAX_NS][DCM_Q_DEPTH];  
#endif

  UINT32        HwActiveMap;
  UINT32        HwActiveMapNext;
  UINT32        HwActiveMapPrev;
  UINT32        IsrPtr;
  UINT32        G1Ptr;
  UINT32        G2Ptr; 
} DCM_DATA_T;

typedef struct
{
  UINT32 ADJ_F;
  UINT32 V_Delay;
  UINT32 H_Delay;
} DCM_DELAY_VAL_T;

UINT32 DCM_Init(void);
UINT32 DCM_Uninit(void);
UINT32 DCM_Preprocess(void);
UINT32 DCM_EnableDispModule(DISP_MODULE_E eHw, UINT32 bIsEnable);
UINT32 DCM_IsDispModuleEnabledPrev(DISP_MODULE_E eHw);
UINT32 DCM_IsDispModuleEnabled(DISP_MODULE_E eHw);
UINT32 DCM_IsDispModuleEnabledNext(DISP_MODULE_E eHw);


UINT32 DCM_RequestRegAccess(void **pReg, UINT64 **pMode, DISP_MODULE_E eHw);
UINT32 DCM_RequestRegAccessISR(void **pReg, UINT64 **pMode, DISP_MODULE_E eHw);

UINT32 DCM_RequestFrameInfoForWrite(UINT8 ucVdpId, V_FRAME_INFO_T **ppFrameInfo);
UINT32 DCM_RequestFrameInfoForRead(UINT8 ucVdpId, V_FRAME_INFO_T **ppFrameInfo);
void DCM_TimingCorrection(void);
extern UINT32 DCM_debug_level;

#endif

