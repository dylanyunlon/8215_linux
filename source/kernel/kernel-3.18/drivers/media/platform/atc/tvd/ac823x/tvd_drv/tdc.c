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
#include"tvd_core.h"
#include"tvd_log.h"
#include"tdc.h"
#ifndef __ARM2__
#include <linux/printk.h>
#endif
#if 1
unsigned int TDC_FW_00 = 0;
#define TDC_US14_EN TDC_FIELD(1, 0)
#define TDC_COLOR_PATCH_EN TDC_FIELD(1, 1) 
#define TDC_CCS_ADAP_EN TDC_FIELD(1, 2) 

//YCDelay control by AP
static unsigned char _sbIsSetYCDelaybyAP = 0;
//CAGC control by AP
static unsigned char _sbIsSetCAGCbyAP = 0;
//AAF control by AP
static unsigned char _sbIsSetAAFbyAP = 0;
//Offset control by AP
static unsigned char _sbIsTVDOffsetbyAP = 0;

extern unsigned long TDC_DRAM_BASE;
unsigned int TDC_DRAM_SIZE = TDC_BUF_SIZE;
extern unsigned long  tvd_base[4];

REGTBL_T REGTBL_COMB_NTSC_AV[] = {

};
REGTBL_T  REGTBL_COMB_NTSC443[] = { 
    
};
REGTBL_T  REGTBL_COMB_PAL_60[] = {
    
};

REGTBL_T  REGTBL_COMB_PAL_AV[] = {

};

REGTBL_T  REGTBL_COMB_PAL_M_AV[] = {
    
};

REGTBL_T  REGTBL_COMB_PAL_N_AV[] = { 

};

REGTBL_T  REGTBL_COMB_SV[] = {

};

static unsigned int uffs(unsigned int x)
{
	int r = 1;

	if (!x)
		return 0;
	if (!(x & 0xffff)) {
		x >>= 16;
		r += 16;
	}
	if (!(x & 0xff)) {
		x >>= 8;
		r += 8;
	}
	if (!(x & 0xf)) {
		x >>= 4;
		r += 4;
	}
	if (!(x & 3)) {
		x >>= 2;
		r += 2;
	}
	if (!(x & 1)) {
		x >>= 1;
		r += 1;
	}
	return r;
}


void vDrvLoadRegTbl(REGTBL_T *pRegDwrdTbl) 
{
	// Store last TDC on/off status.
	unsigned int enable_3d = 0;
	TDC_GET_FIELD(COMB_CTRL_02, EN3D,enable_3d);
    vDrvTDCOnOff(SV_OFF);

    if (pRegDwrdTbl == NULL)
    {
        return;
    }

    while (pRegDwrdTbl->wReg != REG_END)
    {
        if (pRegDwrdTbl->dwMask == 0xFFFFFFFF)
        {
            TDC_WRITE32(pRegDwrdTbl->wReg, pRegDwrdTbl->dwValue);
        }
        else
        {
            TDC_WRITE32_MASK(pRegDwrdTbl->wReg, pRegDwrdTbl->dwValue,pRegDwrdTbl->dwMask);
        }
        pRegDwrdTbl++;
    }
	
	if(enable_3d == 1){
		vDrvTDCOnOff(SV_ON);
	}
	
}


bool fgDrvTDCEnableChk(void)
{
    unsigned char bTmp;

    bTmp = bHwTvdMode(tvd_base[0]);
    if (fgHwTvdSVID_TDC(tvd_base[0])||
        ((bTmp != AV_NTSC) && (bTmp != AV_PAL)
         && (bTmp != AV_PAL_M) && (bTmp != AV_PAL_N)))
    {

		TVD_LOG(TVD_LOG_LVL_INFO," source is other tvd mode,disable 3D filter\n");
		return false;
    }
    else
    {
        return true;
    }
}



/**
 * @brief TDC On/Off Routine
 * 	  //mtk01140 20090902 TDC onoff switchh control rule
 *		[condition] [TDC need to be set to]
 *	1.	[SCART mode change done, SVideo is ON(vTvd3dSVInit)]				[OFF]
 * 	2.	[dis-connect from a input with TDC enable(vTvd3dConnect)]			[OFF]
 *	3.	[Hardware init => TDC init(vDrvTDCInit)]					[OFF]
 *	4.	[Connet to SCART Svideo(vScartSvConnect)]					[OFF]
 *	5.	[FIFO error occur (bFIFOERR)]							[->OFF->ON] 
 *	6.	[before change TDC DRAM based(vDrvTDCSetDramBase), and (C_COMB_2A, EN3D) is ON]	[OFF]
 *	7.	[after change TDC DRAM based(vDrvTDCSetDramBase), and (C_COMB_2A, EN3D) is ON]	[ON]
 *	8.	[before Change setting of "TDC Bandwidth saving(vDrvTDCSaveBW)"]		[OFF]
 *	9.	[after Change setting of "TDC Bandwidth saving(vDrvTDCSaveBW)"]			[ON]
 *	10.	[before Change setting of "TDC DRAM bit(vDrvTDCSetDramMode)"]			[OFF]
 *	11.	[after Change setting of "TDC DRAM bit(vDrvTDCSetDramMode)"]			[ON] 
 * Switch TDC On/Off
 *
 * @param bOnOff: On/Off TDC
 * @return None
 * @warning
 * @todo
 *
 * @pre
 * @post
 */
void vDrvTDCOnOff(unsigned char bOnOff)
{
	unsigned int dram_address;
	
	TDC_GET_FIELD(COMB_CTRL_01, DRAMBASEADR,dram_address);
	TVD_LOG(TVD_LOG_LVL_INFO,"DRAM_BASE = 0x%x \n",dram_address);
    if (dram_address != 0)
    {
        if (bOnOff == SV_ON)
        {
            //turn off "force dram fifo idle"
            TVD_LOG(TVD_LOG_LVL_INFO,"turn on force enable 3D\n");
            TDC_SET_FIELD(COMB_CTRL_03, SV_OFF, FSKBACK);
            TDC_SET_FIELD(COMB_CTRL_02, SV_ON, EN3D);
        }
        else
        {
            //force dram fifo idle
            TVD_LOG(TVD_LOG_LVL_INFO,"force dram fifo idle\n");
            TDC_SET_FIELD(COMB_CTRL_03, SV_ON, FSKBACK);
            TDC_SET_FIELD(COMB_CTRL_02, SV_OFF, EN3D);
    	}
    }
    else
    {
		//force dram fifo idle
		TDC_SET_FIELD(COMB_CTRL_03, SV_ON, FSKBACK);
		TDC_SET_FIELD(COMB_CTRL_02, SV_OFF, EN3D);
		TVD_LOG(TVD_LOG_LVL_INFO,"Zero TDC memory address\n");
    }
} 


/**
 * @brief TDC active window size
 *
 * Set TDC active window
 *
 * @param bOnOff: Active/de-active TDC
 * @return None
 */

void vDrvTDCActive(unsigned char bOnOff,unsigned char tvd_mode)
{

	// Store last TDC on/off status.
	unsigned int enable_3d = 0;
	TDC_GET_FIELD(COMB_CTRL_02, EN3D,enable_3d);
    vDrvTDCOnOff(SV_OFF);

	if (bOnOff)
    {
        switch (tvd_mode)
        {
        case AV_PAL:
            // Set active window for PAL
            TDC_SET_FIELD(COMB_CTRL_02, ACTIVE_WIN_PAL_X_START, HCNT3D);       // Start pixel
            TDC_SET_FIELD(COMB_CTRL_02, ACTIVE_WIN_PAL_X_LENGTH, HLEN3D);      // h-length
            TDC_SET_FIELD(COMB_CTRL_03, ACTIVE_WIN_PAL_Y_START, VLINEST);      // start line
            TDC_SET_FIELD(COMB_CTRL_03, ACTIVE_WIN_PAL_Y_LENGTH, VLINECNT);    // v-length
            TDC_SET_FIELD(COMB_CTRL_05, ACTIVE_WIN_PAL_Y_LENGTH, VLEN3D_M);    // v-length
            break;
        case AV_PAL_M:
            // Set active window for PAL
            TDC_SET_FIELD(COMB_CTRL_02, ACTIVE_WIN_PAL_M_X_START, HCNT3D);     // Start pixel
            TDC_SET_FIELD(COMB_CTRL_02, ACTIVE_WIN_PAL_M_X_LENGTH, HLEN3D);    // h-length
            TDC_SET_FIELD(COMB_CTRL_03, ACTIVE_WIN_PAL_M_Y_START, VLINEST);    // start line
            TDC_SET_FIELD(COMB_CTRL_03, ACTIVE_WIN_PAL_M_Y_LENGTH, VLINECNT);  // v-length
            TDC_SET_FIELD(COMB_CTRL_05, ACTIVE_WIN_PAL_M_Y_LENGTH, VLEN3D_M);  // v-length
            break;
        case AV_PAL_N:
            // Set active window for PAL
            TDC_SET_FIELD(COMB_CTRL_02, ACTIVE_WIN_PAL_N_X_START, HCNT3D);     // Start pixel
            TDC_SET_FIELD(COMB_CTRL_02, ACTIVE_WIN_PAL_N_X_LENGTH, HLEN3D);    // h-length
            TDC_SET_FIELD(COMB_CTRL_03, ACTIVE_WIN_PAL_N_Y_START, VLINEST);    // start line
            TDC_SET_FIELD(COMB_CTRL_03, ACTIVE_WIN_PAL_N_Y_LENGTH, VLINECNT);  // v-length
            TDC_SET_FIELD(COMB_CTRL_05, ACTIVE_WIN_PAL_N_Y_LENGTH, VLEN3D_M);  // v-length
            break;
        case AV_NTSC:
        default:
            // Set active window for NTSC
            TDC_SET_FIELD(COMB_CTRL_02, ACTIVE_WIN_NTSC_X_START, HCNT3D);      // Start pixel
            TDC_SET_FIELD(COMB_CTRL_02, ACTIVE_WIN_NTSC_X_LENGTH, HLEN3D);     // h-length
            TDC_SET_FIELD(COMB_CTRL_03, ACTIVE_WIN_NTSC_Y_START, VLINEST);     // start line
            TDC_SET_FIELD(COMB_CTRL_03, ACTIVE_WIN_NTSC_Y_LENGTH, VLINECNT);   // v-length
            TDC_SET_FIELD(COMB_CTRL_05, ACTIVE_WIN_NTSC_Y_LENGTH, VLEN3D_M);   // v-length
            break;
        }
    }
    else
    {
        TDC_SET_FIELD(COMB_CTRL_02, 0, HCNT3D);        // Start pixel
        TDC_SET_FIELD(COMB_CTRL_02, 0, HLEN3D);        // h-length
        TDC_SET_FIELD(COMB_CTRL_03, 0, VLINEST);       // start line
        TDC_SET_FIELD(COMB_CTRL_03, 0, VLINECNT);      // v-length
    }

	if(enable_3d == 1){
		vDrvTDCOnOff(SV_ON);
	}
	
}


/**
 * @brief Set DRAM mode
 *
 * TDC can be 10-bit or 9-bit mode for DRAM access.
 *
 * @param b10BitMode SV_ON/SV_OFF
 */
void vDrvTDCSetDramMode(unsigned char b10BitMode)
{
   
    // Store last TDC on/off status.
	unsigned int enable_3d;
	TDC_GET_FIELD(COMB_CTRL_02, EN3D,enable_3d);
    vDrvTDCOnOff(SV_OFF);

    // Turn on 10-bit 3D comb.
    if (b10BitMode)
    {
        TDC_SET_FIELD(COMB_CTRL_02, SV_ON, B10MODE);
    }
    // Turn on 9-bit 3D comb.
    else
    {
        TDC_SET_FIELD(COMB_CTRL_02, SV_OFF, B10MODE);
    }

    // Re-enable TDC.
    if (enable_3d)
    {
        vDrvTDCOnOff(SV_ON);
    }

}

void vTvd3dSetAAF(unsigned char bValue)
{
    if(_sbIsSetAAFbyAP == SV_ON)
    {
		TVD_LOG(TVD_LOG_LVL_INFO," _sbIsSetAAFbyAP == SV_ON\n");
		return;
    }

    TDC_SET_FIELD(REG_VSRC_07, bValue, AAF_SEL);
}

void vTvd3dSetYCDelay(unsigned char bYCDelay)
{
    if(_sbIsSetYCDelaybyAP)
    {
		TVD_LOG(TVD_LOG_LVL_INFO,"_sbIsSetYCDelaybyAP\n");
		return;
    }

    TDC_SET_FIELD(CTG_07, (bYCDelay&0x3) ,Y_DELAY); //Y_DELAY
    TDC_SET_FIELD(CTG_07, (bYCDelay>>2) ,UV_DELAY); //UV_DELAY
}

void vTvd3dSetYCDelaybyAP(unsigned char bYDelay, unsigned char bCDelay)
{
    _sbIsSetYCDelaybyAP = SV_ON;
    TDC_SET_FIELD(CTG_07, bYDelay ,Y_DELAY); //Y_DELAY
    TDC_SET_FIELD(CTG_07, bCDelay ,UV_DELAY); //UV_DELAY
}


/**
 * @brief TDC Set Parameter
 *
 * Set TDC Parameter for different TV systems.
 *
 * @param None
 * @return None
 * @warning Remember to Off TDC before change setting
 * @todo Confirm TVD timgen setting for phase lock.
 *
 * @pre
 * @post
 */

void vDrvTDCSet(unsigned char tvd_mode)
{
	unsigned int sviedo_mode;
	
	// Store last TDC on/off status.
	unsigned int enable_3d = 0;
	TDC_GET_FIELD(COMB_CTRL_02, EN3D,enable_3d);
    vDrvTDCOnOff(SV_OFF);
    //AAF Setting---
    vTvd3dSetAAF(0);
    //YCDelay
    vTvd3dSetYCDelay(0);
	
	sviedo_mode = fgHwTvdSVID_TDC();
	
    if (sviedo_mode)
    {
        vDrvLoadRegTbl(REGTBL_COMB_SV);
        return;
    }
    switch (tvd_mode)
    {
    case AV_PAL:
        vDrvLoadRegTbl(REGTBL_COMB_PAL_AV);
        TDC_SET_FIELD(CTG_00, SV_OFF, BST_0DEG);     // Change TVD CGen color 45 degree .
        break;
    case AV_PAL_M:
        vDrvLoadRegTbl(REGTBL_COMB_PAL_M_AV);
        TDC_SET_FIELD(CTG_00, SV_OFF, BST_0DEG);     // Change TVD CGen color 45 degree .
        break;
    case AV_PAL_N:
        vDrvLoadRegTbl(REGTBL_COMB_PAL_N_AV);
        TDC_SET_FIELD(CTG_00, SV_OFF, BST_0DEG);     // Change TVD CGen color 45 degree .
        break;
    case AV_SECAM:
        //Enable CKill bypass Y/C seperation only under not SECAM (To reduce color fleshing).
        TDC_SET_FIELD(COMB_CTRL_03, SV_OFF, ENYCCKILL);
        TDC_SET_FIELD(CTG_00, SV_ON, BST_0DEG); 		// Nomatter
        break;
    case AV_NTSC443:
        vDrvLoadRegTbl(REGTBL_COMB_NTSC443);  
        TDC_SET_FIELD(CTG_00, SV_ON, BST_0DEG);     // Change TVD CGen color 0 degree .
        break;
    case AV_PAL_60:
        vDrvLoadRegTbl(REGTBL_COMB_PAL_60);
        TDC_SET_FIELD(CTG_00, SV_OFF, BST_0DEG);     // Change TVD CGen color 45 degree .
        break;
    case AV_NTSC:
    default:
        vDrvLoadRegTbl(REGTBL_COMB_NTSC_AV);
        TDC_SET_FIELD(CTG_00, SV_ON, BST_0DEG);     // Change TVD CGen color 0 degree .
        break;
    }

	// enable patch
    {
		TDC_SET_FIELD((unsigned int)&TDC_FW_00, SV_ON, TDC_US14_EN);	  //  		
        TDC_SET_FIELD((unsigned int)&TDC_FW_00, SV_ON, TDC_COLOR_PATCH_EN);     //cross color patch .    
    }
    // Re-enable TDC.        
    if (enable_3d)    
	{        
		vDrvTDCOnOff(SV_ON);    
	}   
}

void vTdcColorPatch(void)
{

	unsigned int perfieldchromavarsum;
	unsigned int bPatch_En;
	
	TDC_GET_FIELD(&TDC_FW_00, TDC_COLOR_PATCH_EN,bPatch_En);
	TDC_GET_FIELD(STA_COMB_06, PERFIELDCHROMAVARSUM,perfieldchromavarsum);
	
	if(bPatch_En)
	{
		if(perfieldchromavarsum>130)
		{
			TDC_SET_FIELD(COMB3D_1F, SV_OFF, CB_3D_CUBE_FILTER_Y_EN);
			TDC_SET_FIELD(COMB3D_1F, SV_OFF, CB_3D_CUBE_FILTER_C_EN);
		}
		else
		{
			TDC_SET_FIELD(COMB3D_1F, SV_ON, CB_3D_CUBE_FILTER_Y_EN);
			TDC_SET_FIELD(COMB3D_1F, SV_ON, CB_3D_CUBE_FILTER_C_EN);
		}
	}
}

void vTdcUS14Patch(void)
{
	unsigned int bPatch_En;
	unsigned int inpixcntsta;
	unsigned int mopixcntsta;
	
	TDC_GET_FIELD(STA_COMB_04, INPIXCNTSTA,inpixcntsta);
	TDC_GET_FIELD(&TDC_FW_00, TDC_US14_EN,bPatch_En);
	TDC_GET_FIELD(STA_COMB_05, MOPIXCNTSTA,mopixcntsta);
	
	if(bPatch_En)
	{
		if((inpixcntsta > 0x6000) && 
		   (mopixcntsta <0xC000))
		{
			TDC_SET_FIELD(COMB3D_0F, 2, REG_D3GAINCV);
			TDC_SET_FIELD(COMB3D_0F, 1, REG_D3GAINY);
		}
		else
		{
			TDC_SET_FIELD(COMB3D_0F, 4, REG_D3GAINCV);
			TDC_SET_FIELD(COMB3D_0F, 4, REG_D3GAINY);
		}
	}
}

void vTdcCrossColorProc(void)
{
	unsigned char bSAT_SUM; 
	unsigned char bCCS_ADAP;
	unsigned char bPerMotionPixCnt;
	unsigned char tvd_mode_detect;

	TDC_GET_FIELD(STA_CDET_00, MODE_TVD3D,tvd_mode_detect);
	TDC_GET_FIELD(COMB2D_1D, FRM_SAT_SUM,bSAT_SUM);
	TDC_GET_FIELD(STA_COMB_05, PERMOTIONPIXCOUNT,bPerMotionPixCnt);
	TDC_GET_FIELD(&TDC_FW_00, TDC_CCS_ADAP_EN,bCCS_ADAP);
	
	if(bCCS_ADAP)
	{
		if(tvd_mode_detect == AV_NTSC)
		{
			//cband_mode
			TDC_SET_FIELD(COMB2D_1C, ((bSAT_SUM<55 && bPerMotionPixCnt>5)?2:0), Y2D_CBAND_MODE);
			TDC_SET_FIELD(COMB2D_1B, ((bSAT_SUM<55 && bPerMotionPixCnt>5)?0:1), C2D_CBAND_FORCE_CBWGT);
			TDC_SET_FIELD(COMB2D_16, ((bSAT_SUM<190)?0:1), C2D_COLOR_FORCE_COLORWGTC);
			TDC_SET_FIELD(COMB2D_16, ((bSAT_SUM<190)?0:1), C2D_COLOR_FORCE_GRAYWGTC);
			//Saturation limit type & Chroma band weight
			TDC_SET_FIELD(COMB2D_19, ((bSAT_SUM<30)?2:1), C2D_COLOR_GWGT_SATLIMIT_TYPE);
			

			if(bSAT_SUM<59) // Aggressive
			{
				TDC_SET_FIELD(COMB2D_15, 24, C2D_COLOR_SIMILAR_TH);
				TDC_SET_FIELD(COMB2D_15, 20, C2D_COLOR_GSIMILAR_TH);
				TDC_SET_FIELD(COMB2D_15, 5, C2D_COLOR_SAT_RANGE);
				TDC_SET_FIELD(COMB2D_15, 20, C2D_COLOR_HUE_RANGE);
				TDC_SET_FIELD(COMB2D_16, 20, C2D_COLOR_GRAY_RANGE);
				TDC_SET_FIELD(COMB2D_16, 35, C2D_COLOR_UVVAR_TH);
				TDC_SET_FIELD(COMB2D_18, 70, C2D_COLOR_GRNG_BSUM);
				TDC_SET_FIELD(COMB2D_17, 5, C2D_COLOR_CBANDWGT_BSUMTH);
				TDC_SET_FIELD(COMB2D_17, 60, C2D_COLOR_GBANDWGT_BSUMTH);	
			}
			else // Conservative
			{
				TDC_SET_FIELD(COMB2D_15, 24, C2D_COLOR_SIMILAR_TH);
				TDC_SET_FIELD(COMB2D_15, 27, C2D_COLOR_GSIMILAR_TH);
				TDC_SET_FIELD(COMB2D_15, 5, C2D_COLOR_SAT_RANGE);
				TDC_SET_FIELD(COMB2D_15, 15, C2D_COLOR_HUE_RANGE);
				TDC_SET_FIELD(COMB2D_16, 20, C2D_COLOR_GRAY_RANGE);
				TDC_SET_FIELD(COMB2D_16, 25, C2D_COLOR_UVVAR_TH);
				TDC_SET_FIELD(COMB2D_18, 150, C2D_COLOR_GRNG_BSUM);
				TDC_SET_FIELD(COMB2D_17, 50, C2D_COLOR_CBANDWGT_BSUMTH);
				TDC_SET_FIELD(COMB2D_17, 150, C2D_COLOR_GBANDWGT_BSUMTH);
			}			
		}
		else if(tvd_mode_detect == AV_PAL)
		{
			//cband_mode
			TDC_SET_FIELD(COMB2D_1C, ((bSAT_SUM<40 && bPerMotionPixCnt>5)?2:
							(bSAT_SUM<80 && bPerMotionPixCnt>5)?1:0), Y2D_CBAND_MODE);
			
			TDC_SET_FIELD(COMB2D_1B, ((bSAT_SUM<88 && bPerMotionPixCnt>5)?0:1), C2D_CBAND_FORCE_CBWGT);
			TDC_SET_FIELD(COMB2D_16, ((bSAT_SUM<220)?0:1), C2D_COLOR_FORCE_COLORWGTC);
			TDC_SET_FIELD(COMB2D_16, ((bSAT_SUM<220)?0:1), C2D_COLOR_FORCE_GRAYWGTC);
			//Saturation limit type & Chroma band weight
			TDC_SET_FIELD(COMB2D_19, ((bSAT_SUM<48)?2:1), C2D_COLOR_GWGT_SATLIMIT_TYPE);
			

			if(bSAT_SUM<94) // Aggressive
			{
				TDC_SET_FIELD(COMB2D_15, 24, C2D_COLOR_SIMILAR_TH);
				TDC_SET_FIELD(COMB2D_15, 20, C2D_COLOR_GSIMILAR_TH);
				TDC_SET_FIELD(COMB2D_15, 5, C2D_COLOR_SAT_RANGE);
				TDC_SET_FIELD(COMB2D_15, 20, C2D_COLOR_HUE_RANGE);
				TDC_SET_FIELD(COMB2D_16, 30, C2D_COLOR_GRAY_RANGE);
				TDC_SET_FIELD(COMB2D_16, 35, C2D_COLOR_UVVAR_TH);
				TDC_SET_FIELD(COMB2D_18, 70, C2D_COLOR_GRNG_BSUM);
				TDC_SET_FIELD(COMB2D_17, 10, C2D_COLOR_CBANDWGT_BSUMTH);
				TDC_SET_FIELD(COMB2D_17, 60, C2D_COLOR_GBANDWGT_BSUMTH);	
			}
			else // Conservative
			{
				TDC_SET_FIELD(COMB2D_15, 24, C2D_COLOR_SIMILAR_TH);
				TDC_SET_FIELD(COMB2D_15, 27, C2D_COLOR_GSIMILAR_TH);
				TDC_SET_FIELD(COMB2D_15, 5, C2D_COLOR_SAT_RANGE);
				TDC_SET_FIELD(COMB2D_15, 15, C2D_COLOR_HUE_RANGE);
				TDC_SET_FIELD(COMB2D_16, 30, C2D_COLOR_GRAY_RANGE);
				TDC_SET_FIELD(COMB2D_16, 25, C2D_COLOR_UVVAR_TH);
				TDC_SET_FIELD(COMB2D_18, 150, C2D_COLOR_GRNG_BSUM);
				TDC_SET_FIELD(COMB2D_17, 50, C2D_COLOR_CBANDWGT_BSUMTH);
				TDC_SET_FIELD(COMB2D_17, 150, C2D_COLOR_GBANDWGT_BSUMTH);
			}
		}
	}
}


void vTdc3dProc(unsigned char tvd_mode)
{
    unsigned int  dwTdc3dLumasum;
    unsigned int  dwTdc3dMBPixCnt;
    unsigned int  dwTdc3dColorEdgeSum;
    unsigned int  dwTdc3dLumaEdgeSum;
    unsigned char  bMOTH4MB, bLSMOOTH;
    unsigned char  bSweepDet, bFIFOERR;

	unsigned char  full_status,empty_status,empty_status_a,empty_status_aa;
	unsigned int  tvd_mode_detect;

	
	bool fgTDCEnabled = false;
	
    TDC_GET_FIELD(STA_COMB_0C, SWEEP_DETECTION,bSweepDet);
    TDC_GET_FIELD(STA_COMB_07, LUMASUMSTA,dwTdc3dLumasum);
    TDC_GET_FIELD(STA_COMB_06, MBPIXCNTSTA,dwTdc3dMBPixCnt);
    TDC_GET_FIELD(STA_COMB_0B, COLOREDGESTA,dwTdc3dColorEdgeSum);
    TDC_GET_FIELD(STA_COMB_09, LUMAEDGESTA,dwTdc3dLumaEdgeSum);

    TDC_GET_FIELD(STA_COMB_0C, FIFO_EVERFULL,full_status);
    TDC_GET_FIELD(STA_COMB_0C, FIFOEMPTY,empty_status);
    TDC_GET_FIELD(STA_COMB_0C, FIFOEMPTY_A,empty_status_a);
    TDC_GET_FIELD(STA_COMB_0C, FIFOEMPTY_AA,empty_status_aa);

	bFIFOERR = full_status| empty_status | empty_status_a | empty_status_aa;

    //UNUSED(bTVDNoiseLevel);
    UNUSED(dwTdc3dLumaEdgeSum);
    UNUSED(dwTdc3dColorEdgeSum);
    UNUSED(dwTdc3dLumasum);
    UNUSED(dwTdc3dMBPixCnt);
    UNUSED(bMOTH4MB);
    UNUSED(bLSMOOTH);
    UNUSED(bSweepDet);  

	if (fgHwTvdSVID_TDC())
    {

		TVD_LOG(TVD_LOG_LVL_ERR, "source is SVideo\n");
		return;
    }

	if(bFIFOERR)
    {
		// Re-enable TDC. 
        vDrvTDCOnOff(SV_OFF);
        vDrvTDCOnOff(SV_ON);
    }

	vTdcColorPatch();

	TDC_GET_FIELD(STA_CDET_00, MODE_TVD3D,tvd_mode_detect);
		
	if(tvd_mode_detect== AV_NTSC)
	{
		vTdcUS14Patch();
	}
	vTdcCrossColorProc();
//    vAdaptive3DCombGain();

}


bool vDrvTDCSetDramBase(void)
{
	bool fgTDCEnabled = false;

	u32 dram_address = 0;
	u32 enable_3d = 0;

	TDC_GET_FIELD(COMB_CTRL_02, EN3D, enable_3d);

	// Store last TDC on/off status.
	fgTDCEnabled = enable_3d ? SV_ON : SV_OFF;

	dram_address = (TDC_DRAM_BASE >> 4);

	vDrvTDCOnOff(SV_OFF);

	TVD_LOG(TVD_LOG_LVL_INFO,"zhiDRAM_BASE = 0x%08x \n",dram_address);

	if(dram_address & 0xF0000000)
	{
		TVD_LOG(TVD_LOG_LVL_ERR,"TDC_DRAM_BASE is wrong\n");
		return false;
	}

	if (dram_address & 0x8000000)
	{
		TDC_SET_FIELD(REG_SYS_0A, 0xF, COMB3D_BIT27);
	}
	else
	{
		TDC_SET_FIELD(REG_SYS_0A, 0, COMB3D_BIT27);
	}

	if (dram_address & 0x4000000)
	{
		TDC_SET_FIELD(COMB2D_0D, 1, CHANNEL_B_SEL);
	}
	else
	{
		TDC_SET_FIELD(COMB2D_0D, 0, CHANNEL_B_SEL);
	}

	if (dram_address & 0x2000000)
	{
		TDC_SET_FIELD(COMB2D_0D, 1, DRAMBASEADR_MSB);
	}
	else
	{
		TDC_SET_FIELD(COMB2D_0D, 0, DRAMBASEADR_MSB);
	}

	// Update TDC Dram base.
	TDC_SET_FIELD(COMB_CTRL_01, dram_address, DRAMBASEADR);


	// Re-enable TDC.    
	if (fgTDCEnabled)
	{
		vDrvTDCOnOff(SV_ON);
	}

	return true;

}

void vDrvTDCInit(void)
{
	vDrvTDCOnOff(SV_ON);
}

#endif 
