#ifndef VGA_HAL_API_H_
#define VGA_HAL_API_H_

#include <generated/atc_project.h>
/*#include "x_bim.h"*/
#include "x_os.h"
#ifdef CONFIG_ATC_PLATFORM_ac83xx
#include "x_timer.h"
#endif
#ifndef __ARM2__
#include <linux/types.h>
#endif
/***Macro Define***/
#define SV_PORCHTUNE_DEC    0x1    /* only get/set decoder porch */
#define SV_PORCHTUNE_SCPOS  0x2    /* only get/set scaler porch */
#define SV_PORCHTUNE_MIX    (SV_PORCHTUNE_DEC|SV_PORCHTUNE_SCPOS)    /* only get/set mix dec/scpos porch */
#define SV_TRUE            	1
#define SV_FALSE           	0
#define SV_ON              	1
#define SV_OFF             	0
#define ON              	1
#define OFF             	0

/***Enum Define***/
enum
{
    SV_HPORCH_CURRENT,
    SV_HPORCH_DEFAULT,
    SV_HPORCH_MIN,
    SV_HPORCH_MAX,
    SV_VPORCH_CURRENT,
    SV_VPORCH_DEFAULT,
    SV_VPORCH_MIN,
    SV_VPORCH_MAX
} ;

enum VIDE_SIGNAL_STATUS_LIST/* Video Signal Status*/
{
    SV_VDO_NOSIGNAL        = 0,
    SV_VDO_NOSUPPORT,
    SV_VDO_UNKNOWN,        // Still doing mode detect
    SV_VDO_STABLE
};

enum IC_Input/* Input port of IC */
{
    P_C0 = 0,
    P_C1 = 1,
    P_C2 = 2,
    P_C3 = 3,
    P_C4 = 4, 
    P_SV0 = 5,
    P_SV1 = 6,
    P_SV2 = 7,
    P_YP0 = 8,
    P_YP1 = 9,
    P_VGA = 10,
    P_FB0 = 11,
    P_FB1 = 12,
    P_DVI = 13,
    P_CCIR = 14,
    P_DT1 = 15, 
    P_DT2 = 16, // for 5371
    P_VGACOMP = 17,
    P_MA = 18,
    P_FA = 0XFF
};

enum {
    DISABLE,
    ENABLE
};


/***Function Declaration***/
void vDrvVideoInit(void);
void initYPbPrVGA(void);
void vDrvVideoConnect(bool fgOnOff);
void vDrvVideoQueryInputTimingInfo(void);
u16 wDrvVideoGetPorch(u8 bPath, u8 bPorchType);
u16 wDrvVideoSetPorch(u8 bPath, u8 bPorchType, u16 wValue);
u16 wDrvVideoInputWidth(void);
u16 wDrvVideoInputHeight(void);
u8 bDrvVideoGetRefreshRate(void);
u8 bDrvVideoIsSrcInterlace(void);
u8 bDrvVideoSignalStatus(void);
u16 wDrvVideoGetHTotal(void);
u16 wDrvVideoGetVTotal(void);
u8 bDrvVideoGetTiming(void);
const CHAR* strDrvVideoGetTimingString(u8 bTiming);
u8 fgApiVideoVgaAuto(void);
void vDrvVideoAutoColor(void);
void vDrvVideoSyncMode(void);
u32 wDrvVideoGetTvType(u32 u4Height, u32 u4Width);
void   vDrvVideoMainLoop(void);
u8 bVGAMode_Detect(void);
u32 u4DrvVideoGetIrqStatus(void);
void vDrvVideoClearIrqStatus(u32 u4IrqStatus);
void vDrvVideoSuspend(void);
void vDrvVideoResume(void);
void vDrvVideoAuto(void);
void vVga_IRQ(u16 u2Vector);

/***Extern Function Decaration***/
extern void ybr_vga_autocolor(void);
extern void ybr_vga_auto(void);
extern void vVgaModeDetect(void) ;
extern void vVgaChkModeChange(void) ;
extern void vHdtvModeDetect(void) ;
extern void vHdtvChkModeChange(void) ;
extern void vHdtvISR(void) ;
extern void vVgaISR(void) ;
extern void vDrvEnableBlankLevelAdjust(void);
extern void vVdoSP0AutoState(void) ;
extern void vDrvAdjustBlankLevel(void);
extern void vDrvOnChipAutoColorIteration(void);
extern void vDrvPGALinearityVerify(void);

/***Extern Variable Declaration***/
extern u8 g_u1Timing;
extern u32 g_u4IrqEnable;
extern u32 g_u4IrqStatus; 
extern HANDLE_T g_hMLoopThread;
extern bool g_bStop;
extern u32 g_u4SrcType;
extern u8 _IsVgaDetectDone;
extern u8 _IsHdtvDetectDone;

#endif
