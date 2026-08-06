#include "x_types.h"
#include "display.h"
#include "drv_osd_if.h"
#include "osd_inc.h"
#include "pmx_hal.h"
#include "log.h"
#include "reserve_memory.h"
#include "mrf.h"
#include "pmx_hw.h"
#include "pmx_vfy_hal.h"
#include "vdp_hal.h"
#include <generated/atc_project.h>

__u32 fb_log_lvl = FB_LOG_LVL_HAL;
__u8 *fb_lvl_str[] = {
	"[FB OFF]",
	"[FB ERR]",
	"[FB WARN]",
	"[FB CLI]",
	"[FB INFO]",
	"[FB HAL]",
	"[FB IRQ]",
	"[FB TRACE]",
	"[FB DBG]",
	"[FB REGRW]",
};

unsigned long fbm_base = 0x10000000 - 0xfc00000;
void* fbm_va;/*just for build arm2*/
unsigned long fbm_osd_base = 0x110000000;
unsigned long fbm_size = 0x1200000;
unsigned long logo_base = 0;
unsigned long logo_size = 0;
extern unsigned int gdty_cyc;
unsigned int z_order = 0x43210;
unsigned int fmtf_irq;
unsigned int fmtr_irq;

unsigned long IO_BASE_BRINGUP = 0x10000000;
static volatile unsigned int fpd_panel_size = 0;
unsigned int osdf_reg = 0x10000000 + 0x20000;
unsigned int osd1_reg = 0x10000000 + 0x20100;
unsigned int osd2_reg = 0x10000000 + 0x20200;
unsigned int osd3_reg = 0x10000000 + 0x20300;
unsigned int osd4_reg = 0x10000000 + 0x20a00;
unsigned int osd5_reg = 0x10000000 + 0x20b00;
unsigned int osdr_reg = 0x10000000 + 0xa3000;
unsigned int osdr1_reg = 0x10000000 + 0xa3100;
unsigned int osdr2_reg = 0x10000000 + 0xa3200;
unsigned int osdr3_reg = 0x10000000 + 0xa3300;
unsigned int vdof_reg = 0x10000000 + 0x42400;
unsigned int vdor_reg;
unsigned int tlcp_reg = 0x10000000 + 0xa4400;
unsigned int tcon_reg = 0x10000000 + 0xa4800;
unsigned int togc_reg = 0x10000000 + 0xa4700;
unsigned int scl_reg;/*just for build arm2*/
unsigned int sclf_reg;/*just for build arm2*/

FB_CONFIG_T  g_rFBConfig = {
	REAR_OUTPUT_MODE,
	TM070DDHG,
	{
		0,
		0,
		0,
		0,
	},
};

//extern void Flush_Cache(UINT32 u4Start, UINT32 u4Len);
extern void memcpy(void *dest, void *src, unsigned int count);

#define WriteREG(arg, val) (*(volatile unsigned int*)(IO_BASE_BRINGUP + (arg)) = val)
#define ReadREG(arg) (*(volatile unsigned int*)(IO_BASE_BRINGUP + (arg)))
#define WriteREGMsk(arg, val, msk)	WriteREG((arg), (ReadREG(arg) & (~(msk))) | ((val) & (msk)))

static void *_memset(void *s, int c, unsigned int count)
{
	char *tmp = (char *)s;

	while (count--)
		*tmp++ = c;
	return s;
}
static void _msleep(unsigned int cnt)
{
	int i =0, i4result =0;

	for (i =0; i< (cnt * 0x10000); i++)
	{
	   i4result ++;
	}
}

void PpVcpinit(unsigned char VideoPath, unsigned int DisplayMode)
{
	unsigned int Scaler_h, Scaler_v;
	unsigned int HActive, VActive;
	unsigned int HTotal, VTotal;
	unsigned int dwTmp;
	unsigned int PP_Delay, PP_Ctrl;

	if (0 == VideoPath) {
#ifndef CONFIG_ATC_PRJ_ac823x_adas
		WriteREGMsk(0xB4, 0x02000000, 0x02000000); //Bit 25
		WriteREGMsk(0xD0, 0x02000000, 0x02000000);
#endif

		WriteREGMsk(0x1f080, 0x1, 0x1); //enable PP&vcp

		Scaler_h = ReadREG(0xA46A0);
		Scaler_v = ReadREG(0xA46A4);
		HActive = (Scaler_h & 0x0000FFFF) - ((Scaler_h & 0xFFFF0000) >> 16) + 1;
		VActive = (Scaler_v & 0x0000FFFF) - ((Scaler_v & 0xFFFF0000) >> 16) + 1;

		dwTmp = ReadREG(0xA468C); 			//[27:16]:HTotal & [10:0]:VTotal
		HTotal = (dwTmp & 0x0FFF0000) >> 16;
		VTotal = (dwTmp & 0x000007FF) >> 0;

		PP_Ctrl = (((HActive)/2 -1) << 20) | (0x3 << 16) | (((HActive)/2 -1) << 2);
		WriteREGMsk(0x1f084, PP_Ctrl,0xffffffff); //pp_front_ctrl

		WriteREGMsk(0x42130, Scaler_h,0xffffffff); //PP_H_s_e
		WriteREGMsk(0x42134, Scaler_v,0xffffffff); //PP_V_s_e
		WriteREGMsk(0x42138, Scaler_v,0xffffffff); //PP_V_s_e

		PP_Delay = (0x80040000 | (HTotal - 0x59));
		WriteREGMsk(0x4213C, PP_Delay,0xffffffff);

		switch (DisplayMode) {
		case RES_1080P60HZ:	//1080P
			WriteREGMsk(0x42130, 0x00ba0839, 0xffffffff); //H/V_Sync Delay
			WriteREGMsk(0x420EC, 0xC00807eb, 0xffffffff); //H/V_Sync Delay
			break;				
			
		case RES_600P_1024:	//1024x600
			WriteREGMsk(0x42130, 0x00ae04ad, 0xffffffff); //H/V_Sync Delay
			WriteREGMsk(0x420E8, 0x800009b7, 0xffffffff); //H/V_Sync Delay
			WriteREGMsk(0x420EC, 0x800404b0, 0xffffffff); //H/V_Sync Delay
			break;
			
		case RES_480P_800:	//800x480
			WriteREGMsk(0x42130, 0x009603b5, 0xffffffff); //H/V_Sync Delay
			WriteREGMsk(0x420E8, 0x80040982, 0xffffffff); //H/V_Sync Delay
			WriteREGMsk(0x420EC, 0xC0020383, 0xffffffff); //H/V_Sync Delay
			break;
			
		case RES_600P_800:	//800x600
			WriteREGMsk(0x42130, 0x008203a1, 0xffffffff); //H/V_Sync Delay
			WriteREGMsk(0x4213c, 0x8004039a, 0xffffffff); //H/V_Sync Delay
			WriteREGMsk(0x420E8, 0x80000e38, 0xffffffff); //H/V_Sync Delay
			WriteREGMsk(0x420EC, 0xC004038f, 0xffffffff); //H/V_Sync Delay
			break;

		default:
			break;
		}
	} else if(1 == VideoPath){

	} else {
		Printf("[PpVcpinit] PpVcpinit wrong ChannelId:%d\n", VideoPath);

	}
}

void setPmxZorder(unsigned int z_order)
{
	_rPmxHalMixSwReg.rField.u4MIX_LAYER1_SEL = (z_order >> 0) & 0xF;
	_rPmxHalMixSwReg.rField.u4MIX_LAYER2_SEL = (z_order >> 4) & 0xF;
	_rPmxHalMixSwReg.rField.u4MIX_LAYER3_SEL = (z_order >> 8) & 0xF;
	_rPmxHalMixSwReg.rField.u4MIX_LAYER4_SEL = (z_order >> 12) & 0xF;
	_rPmxHalMixSwReg.rField.u4MIX_LAYER5_SEL = (z_order >> 16) & 0xF;
	_prPmxHalMixHwReg->au4Reg[0/4] = _rPmxHalMixSwReg.au4Reg[0/4];
}

void getpmxbaseaddr_arm2(void)
{
	_prPmxHalMixHwReg = (PMX_HAL_MIX_UNION_T *)(IO_BASE_BRINGUP + 0x1f000);// yzq hw
	setPmxZorder(z_order);

	_prPmxHalMix2HwReg = (PMX_HAL_MIX_UNION_T *)(IO_BASE_BRINGUP + 0x1f054);
	_prPmxDispMainHwReg = (HAL_PMX_DISP_MAIN_UNION_T *)(IO_BASE_BRINGUP + 0x42000);
	_prPmxDispAuxHwReg = (HAL_PMX_DISP_MAIN_UNION_T *)(IO_BASE_BRINGUP + 0x43000);
	_prPmxDispMainCHwReg = (HAL_PMX_DISP_MAIN_C_UNION_T*)(IO_BASE_BRINGUP + 0x42d00);
	_prPmxDispAuxCHwReg = (HAL_PMX_DISP_MAIN_C_UNION_T*)(IO_BASE_BRINGUP + 0x43d00);
}

extern bool EnPmxRearNewSD144Mode;
UCHAR _ucPmxVerifyMainIsrInit = 0;
static BOOL _fgPmxDrvInit = FALSE;
void PmxVerifyInitMainIsr(void)
{
	int pmxirq = 149 - 104;
	Printf("[PmxVerifyInitMainIsr] PmxVerifyInitMainIsr\n");

	if ((_ucPmxVerifyMainIsrInit == 0))
	{
		//v_enable_bim_irq(pmxirq);

		_ucPmxVerifyMainIsrInit = 1;
		Printf("[PmxVerifyInitMainIsr] PmxVerifyInitMainIsr OK\n");
	}
}

void PmxVerifyDrvInit_arm2(void)
{
        if (!_fgPmxDrvInit)
        {
                PMX_HalSetupSoftwareRegister();

                //not panel here.fixme vPmxVerifyHalSysInit();
                PmxVerifyInitMainIsr(); 
                //PmxVerifyInitAuxIsr();  //todo
                _fgPmxDrvInit = TRUE;
        }
}

void necessaryinit(void)
{
	getpmxbaseaddr_arm2();
	PmxVerifyDrvInit_arm2();
	PmxVerifySetMode_OSD(0, 600, 600, 600, 600, 13, 1, 0, 0, 1, 0, TRUE);
	//vPmxHalMainIsr(45,NULL);
}

static BYTE g_pbPanelGamma[64] = {
	0, 1, 3, 5, 7, 9, 11, 13,
	15, 17, 20, 23, 25, 28, 31, 34,
	37, 40, 43, 47, 51, 56, 61, 66,
	71, 76, 82, 87, 92, 97, 102, 107,
	112, 118, 123, 128, 133, 138, 143, 148,
	153, 158, 163, 167, 172, 177, 182, 186,
	191, 195, 200, 204, 208, 212, 216, 220,
	224, 228, 232, 236, 240, 244, 248, 252
};
bool  BL_DisplayInit(void)
{
	unsigned int u4Plane, u4RgnList, u4Rgn;
	unsigned int u4BitCount = 32;
	unsigned int *pbuffer = (unsigned int *)fbm_base;
	void * hMrf = NULL;
	void    *lpFB = NULL;
	unsigned int u4DisplayBitCnt;
	bool    bRet = FALSE;
	BITMAPOBJINFO   bitinfo;

	/*ddds/lvds/fpd/scl/pmx/pwm init*/
	necessaryinit();
	PpVcpinit(0, RES_600P_1024);

	logo_base = fbm_base;
	logo_size = fbm_size;

	MMInit();

	hMrf = LoadLogoMRF();
	Printf("[ARM2]mrf hMrf = %x \r\n",hMrf);
	MRFHEADER	  *pMrfHeader = (MRFHEADER *)hMrf;
	u4DisplayBitCnt = pMrfHeader->u4bitcount;
	Printf("[ARM2]mrf reserve count is %d \r\n",u4DisplayBitCnt);
	bRet = GetBitmapInfo(hMrf, 0, &bitinfo);
	Printf("[ARM2]mrf bRet is %d \r\n",bRet);

	if (bRet) {
		lpFB = GetRCObjectMemAddr(hMrf, &bitinfo);
		int upgrademode = fgDualHALGetUpgradeMode();
		Printf("[ARM2]upgrademode is %d \r\n",upgrademode);
		if(upgrademode == 0){
			memcpy((void *)(logo_base), lpFB,
		       bitinfo.u4Height * bitinfo.u4Width * bitinfo.u4BitCount / 8);
		}
	}

	Printf("[ARM2]bitinfo.u4Height(%d) bitinfo.u4Width(%d) bitinfo.u4BitCount(%d)\r\n",
	       bitinfo.u4Height, bitinfo.u4Width, bitinfo.u4BitCount);

	/*osd init*/
	OSD_Init(TRUE);

	u4Plane = PRIMARY_SURF_ID;
	i4OsdSetDisplayMode(u4Plane, RES_600P_1024);
	OSD_BASE_SetOsdPosition(u4Plane, 0, 0);
	OSD_SC_Scale(u4Plane, TRUE, 1024, 600, 1024, 600);
	OSD_RGN_LIST_Create(&u4RgnList);
	u4RgnList  = u4Plane;

	if (16 == u4BitCount) {
		OSD_RGN_Create(&u4Rgn, bitinfo.u4Width, bitinfo.u4Height, (void *)fbm_osd_base - 0x100000000, 11
			, (1024 * 2), 0, 0,  bitinfo.u4Width, bitinfo.u4Height);
	} else if (32 == u4BitCount) {
		OSD_RGN_Create(&u4Rgn, bitinfo.u4Width, bitinfo.u4Height, (void *)fbm_osd_base - 0x100000000, 14
			, (1024 * 4), 0, 0, bitinfo.u4Width, bitinfo.u4Height);
	}
	//Flush_Cache(0x67f8000, 0x20);
	OSD_RGN_Set(u4Rgn, OSD_RGN_MIX_SEL, OSD_BM_PLANE);
	OSD_RGN_LIST_DetachAll(u4RgnList);
	OSD_RGN_Insert(u4Rgn, u4RgnList);
	SetPlaneRgn(u4RgnList, u4Rgn);
	i4OsdPlaneFlipTo(u4Plane, u4RgnList);
	i4OsdPlaneEnble(u4Plane, TRUE);

        vVdpHalInit(VDP_1, TRUE);
	//vPanelBklControl(gdty_cyc, 100-gdty_cyc);
}

void  DisplayLighten(void)
{
    static BOOL s_fgFirstInit = FALSE;

	if(!s_fgFirstInit)
	{
		vPanelBklControl(gdty_cyc, 100-gdty_cyc);
		vTconSetBrightness(57);
		vTconSetContrast(18);
		vTconSetHue(50);
		vTconSetSaturation(50);
		vPanelSetGamma(g_pbPanelGamma);
		s_fgFirstInit = TRUE;
	}
		
}

void enableArm2LogoUI(void)
{
	static bool fglgoui = false;
	if (fglgoui == false) {
		vPmxMixPlane(PMX_1, PMX_HW_OSD2_MIX);
                vPmxHalMainIsr(45,NULL);
		fglgoui = true;
	}
}
