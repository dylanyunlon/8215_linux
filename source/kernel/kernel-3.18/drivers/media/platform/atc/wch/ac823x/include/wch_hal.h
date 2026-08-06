/*
 *Department: ATC-SD-SD2
 *
 *Aurthor: yezhiqiang(atc0064)
 *
 *Date: 2016-12-05
 */

#ifndef _WCH_HAL_H_
#define _WCH_HAL_H_

#include "wch_drv.h"
#include "wch_priv.h"


typedef struct WCH_INPUT_INFO {
	unsigned char u1WchId;
	WCH_DATA_SRC_E eSrcType;
	WCH_DATA_FMT_E eSrcFmt;
	WCH_SRC_APP_ID_E eWchSrcId;
	unsigned int u4SrcWidth;
	unsigned int u4SrcHeight;
	unsigned int u4StartX;
	unsigned int u4StartYTop; 
	unsigned int u4StartYBot;
	bool fgProgressive;
	
	unsigned char u1YSel;
	unsigned char u1USel;
	unsigned char u1VSel;
	unsigned char u1CInDelay;
	
#if WCH_SUPPORT_AVM_480P
	bool fgSupportAVM480P;
#endif
} WCH_INPUT_INFO_T;


typedef struct WCH_OUTPUT_INFO {
	unsigned char u1WchId; 
	WCH_DATA_FMT_E eDstFmt;
	bool u4ScanLineMode; 
	bool fgProgressive;
	unsigned int u4DstWidth; 
	unsigned int u4DstHeight;
} WCH_OUTPUT_INFO_T;


void wchEnabelClk(unsigned char u1WchId);
void wchDisabelClk(unsigned char u1WchId);
void wchGetHwRegAddress(void);
void wchGetHwRegToSw(unsigned char u1WchId);
void wchHalInit(unsigned char u1WchId);
void wchHalDeInit(unsigned char u1WchId);
void wchHalSetYCAddress(unsigned char u1WchId, unsigned long u8YAddr, unsigned long u8CAddr);
void wchHalTopInit(unsigned char u1WchId, WCH_DATA_SRC_E eSrcType);
void wchHalVoutTopInit(WCH_VOUT_SRC_E eSrcType);
void wchHalSetInput(WCH_INPUT_INFO_T * pInputInfo);
void wchHalSetOutput(WCH_OUTPUT_INFO_T * pOutputInfo);
void wchHalStart(unsigned char u1WchId);
void wchHalStop(unsigned char u1WchId);

#endif

