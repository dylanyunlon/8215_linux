#ifndef __DISPLAY_IF_H__
#define __DISPLAY_IF_H__

#include "ac823x_display.h"

extern volatile HAL_PMX_DISP_MAIN_UNION_T*  _prPmxDispMainHw3365Reg;// = (HAL_PMX_DISP_MAIN_UNION_T*)HAL_PMX_DISP_MAIN_REG;
extern volatile HAL_PMX_DISP_MAIN_UNION_T*  _prPmxDispAuxHw3365Reg;// = (HAL_PMX_DISP_MAIN_UNION_T*)HAL_PMX_DISP_AUX_REG;
extern volatile HAL_PMX_DISP_MAIN_C_UNION_T*  _prPmxDispMainCHwReg;// = (HAL_PMX_DISP_MAIN_C_UNION_T*)HAL_PMX_DISP_MAIN_C_REG;
extern volatile HAL_PMX_DISP_MAIN_C_UNION_T*  _prPmxDispAuxCHwReg;// = (HAL_PMX_DISP_MAIN_C_UNION_T*)HAL_PMX_DISP_AUX_C_REG;
extern volatile PMX_HAL_MIX_UNION_T *  _rPmxHalMixHw3365Reg;
extern volatile PMX_HAL_MIX_UNION_T *  _rPmxHalMix2Hw3365Reg;// = (PMX_HAL_MIX_UNION_T *)PMX_HAL_MIX_REG;
void vPmxMixPlane_arm2(unsigned char ucPmxId, unsigned int u4Plane);
void vPmxVerifyHalLoadSetting_arm2(REG_SET_ARM2_T *prRegSet);
void PmxVerifyCAVSetup_arm2(unsigned int u4Mode);
void PmxVerifyCVBSSetup2_arm2(unsigned char ucVdoId, unsigned int ucPmxMode);
void PmxVerifySclerSetup_arm2(unsigned char ucVdoId, unsigned int ucPmxMode);
void PmxVerifySetMode_720480Vdo_arm2(unsigned char ucVdoId,
				unsigned int u4SrcFmt,
				unsigned int u4PmxFmt,
				unsigned int u4OutFmt,
				unsigned int u4CavFmt,
				unsigned char ucTvType,
				unsigned char ucFit,
				unsigned char ucInterlace,
				unsigned char ucVdoInterlace,
				unsigned char ucSrcType,
				unsigned char uc3d,
				unsigned int ucScanline,
				unsigned int u4AddrY,
				unsigned int u4AddrC
				);
#endif