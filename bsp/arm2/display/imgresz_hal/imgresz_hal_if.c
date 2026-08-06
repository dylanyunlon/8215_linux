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

//#include "x_os.h"

#include "x_assert.h"

/*#include "u_os.h"*/
#include "x_rtos.h"
#include "imgresz_drv.h"
#include "imgresz_hal_if.h"
#include "imgresz_hal.h"
#include "drv_imgresz_errcode.h"
#include "drv_thread.h"
#include "drv_config.h"
#include "x_hal_1176.h"
#include "sys_config.h"

#include "media/atc/display.h"

#include "x_hal_1176.h"
#include "irqs_vector.h"
#include "imgresz_hal_if.h"
#include "imgresz_hal_errcode.h"
#include "imgresz_hal.h"
#include "drv_config.h"
#include "chip_ver.h"

#include "x_iommu_3363.h"
#include "83xx_irqs_vector.h"

#include "imgresz_log.h"



typedef struct {
	IMGRESZ_HAL_RESIZE_MODE_T eResizeMode;            /*/ Resize mode*/
	IMGRESZ_HAL_RESAMPLE_METHOD_T eHResampleMethod;   /*/ Horizontal Resample method*/
	IMGRESZ_HAL_RESAMPLE_METHOD_T eVResampleMethod;   /*/ Vertical Resample method*/
	IMGRESZ_HAL_IMG_INFO_T rSrcImgInfo;               /*/ Source image infomation.*/
	IMGRESZ_HAL_IMG_INFO_T rDestImgInfo;              /*/ Destination image infomation.*/
	IMGRESZ_HAL_IMG_INFO_T rBldImgInfo;               /*/ Blending image infomation.*/
	IMGRESZ_HAL_PARTIAL_BUF_INFO_T rSrcRowBufInfo;    /*/ Source row buffer infomation.*/

	volatile BOOL fgResizeComplete;                            /*/ Resize complete flag.*/
	BOOL fgScaling;                                   /*/ Scaling.*/
	u32 u4TempBufSa;                               /*/ The start address of temporary buffer for partial mode.*/
	BOOL fgLumaKeyEnable;                             /*/ For luma key enable*/
	u8 u1LumaKey;                                  /*/ Luma key value*/
	IMGRESZ_HAL_JPEG_INFO_T rJpegInfo;                /*/ Jpeg information*/
	IMGRESZ_HAL_RM_INFO_T    rRMInfo;
	BOOL fgResume;                                    /*/ If break and resume.*/
	BOOL  fgUserTable;
	u32 u4MMUTable;
#if IMGRESZ_SUPPORT_RESET_DEST_BUFFER
	BOOL fgResetDstBuf;                               /*/ reset dest buffer*/
#endif

	BOOL fgScale1to1;                                  /*/ sun new*/
	BOOL fgScale4to1;                                  /*/ sun new*/
	IMGRESZ_HAL_NOTIFY_CB_REG_T rNofifyCallback;      /*/ Notify callback function*/
} IMGRESZ_HAL_INFO_T;

IMGRESZ_HAL_INFO_T _rImgReszHalInfo[HW_IMGRESZ_NUM] = {{0}, {0} };  /* HAL local information for each HW.*/

#define IMGRESZ_SUPPORT_AGENT_ON_OFF 1
 UINT32 u4HalGetTTB1(void){
	return 0;
}

/*extern int rand(void);*/
/*extern void srand(int seed);*/

#if 1/*IMGRESZ_IO_MMU_TEST*/
void vImgreszSetMMU(u32 u4ImgResizerID, BOOL fg2way)
{
	return;
}
#endif

#if 1/*IMGRESZ_IO_MMU_TEST*/
void vImgresz_MMUSet_After_Resize(u32 u4HwId)
{
	if(u4HwId == 0) {
		IOMMU_WRITE32(REG_RW_IOMMU_CFG4, IOMMU_RESZ,
			      (IOMMU_READ32(REG_RW_IOMMU_CFG4, IOMMU_RESZ) | 0x80000000));
		IOMMU_WRITE32(REG_RW_IOMMU_CFG4, IOMMU_RESZ,
			      (IOMMU_READ32(REG_RW_IOMMU_CFG4, IOMMU_RESZ) | 0x80800));

		while ((IOMMU_READ32(REG_RW_IOMMU_CFG8, IOMMU_RESZ) & 0x20000000) != 0)
			;
	}
}

void vImgreszSetUserTable(u32 u4MMUTable)

{
#if 0

	IOMMU_WRITE32(REG_RW_IOMMU_CFG1, IOMMU_RESZ, u4HalGetTTB0());
#else
	IOMMU_WRITE32(REG_RW_IOMMU_CFG1, IOMMU_RESZ, u4MMUTable);
#endif
}
#endif

#if 0/*(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_AC83XX)*/

#define CLK_RESZ_JPG_SEL_27M                            0 /*27MHz*/
#define CLK_RESZ_JPG_SEL_APLL                           1 /*270MHz/297MHz*/
#define CLK_RESZ_JPG_SEL_SYSPLL_D3                      2 /*144MHz*/
#define CLK_RESZ_JPG_SEL_ARMPLL_D3                      3 /*266.6MHz*/
#define CLK_RESZ_JPG_SEL_SYSPLL_D2                      4 /*216MHz*/
#define CLK_RESZ_JPG_SEL_ADPLL_324                      5 /*324MHz*/
#define CLK_RESZ_JPG_SEL_USBPLL_D2                      6 /*240MHz*/

#endif


void vHwImgReszClk(void)
{
	int ret = 0;
	UINT32 u4Value;
	//u4Value = CKGEN_AgtGetClk(e_CLK_SEL_RSZ);

	//if(u4Value != 0x5)
		//CKGEN_AgtSelClk(e_CLK_SEL_RSZ, 0x5);

}



void i4ImgResz_HAL_Set_Interupt_Enable(void)
{
	vImgResz_HAL_Set_Interupt(TRUE);
}

/*/ Initialize Image Resizer HAL when boot up*/
/*/ \return If return value < 0, it's failed. Please reference imgresz_hal_errcode.h.*/
s32 i4ImgResz_HAL_Boot_Init(
	void
)
{
#if 0
	x_os_isr_fct pf_old_isr = NULL;

	x_reg_isr(imgr0irq, vImgResz_HAL_ISR, &pf_old_isr);
	x_reg_isr(imgr1irq, vImgResz_HAL_ISR, &pf_old_isr);
#endif
	
	/* Image resizer and Graphics use the same clock setting,*/
	/* so the clock is set by Graphics driver.*/
	/* But, image resizer should check clock itself for verification.*/
	vHwImgReszClk();

	return S_IMGRESZ_HAL_OK;
}


/*/ Uninitialize Image Resizer HAL when boot down*/
/*/ \return If return value < 0, it's failed. Please reference imgresz_hal_errcode.h.*/
s32 i4ImgResz_HAL_Boot_Uninit(
	void
)
{
#if 0
	x_os_isr_fct pf_old_isr = NULL;

	x_reg_isr(imgr0irq, NULL, &pf_old_isr);
	x_reg_isr(imgr1irq, NULL, &pf_old_isr);
#endif
	
	return S_IMGRESZ_HAL_OK;
}

/*/ Initialize Image Resizer HAL*/
/*/ \return If return value < 0, it's failed. Please reference imgresz_hal_errcode.h.*/
s32 i4ImgResz_HAL_Init(
	u32 u4ImgReszID,                               /*/< [IN] Image Resizer hardware ID*/

	uintptr_t u4tableaddr
)
{
	BOOL fgMMUTable = (0 != u4tableaddr);

	/*IMGR_LOG(IMGR_LOG_LVL_DBG,"fgMMUTable is %d, u4tableaddr is %x!\r\n",fgMMUTable,u4tableaddr);*/

	if (u4ImgReszID >= HW_IMGRESZ_NUM) {
		return E_IMGRESZ_HAL_FAIL;
	}

#if IMGRESZ_SUPPORT_AGENT_ON_OFF
	vHwImgReszClk();

	if (u4ImgReszID == 0){
		
		//CKGEN_AgtOnClk(e_CLK_IMG_RESZ);//0xA0*/
		}
	 else{
		//CKGEN_AgtOnClk(e_CLK_OSD_RESZ);
		unsigned int add1=0xf00000a0;
		unsigned int add2=0xf00000bc;

		unsigned int value = *((volatile unsigned int*)(add1));
		value = value | 0x1<<5;
		*((volatile unsigned int*)(add1)) =value;

		value = *((volatile unsigned int*)(add2));
		value = value | 0x1<<5;
		*((volatile unsigned int*)(add2)) =value;

		IMGR_LOG(IMGR_LOG_LVL_DBG,"a0 is %x,bc is %x",*((volatile unsigned int*)(add1)),*((volatile unsigned int*)(add2)));

	 	}
#endif

	vHwImgReszEnable(u4ImgReszID);

	if (fgMMUTable) {
		vHwImgReszEnableMMU(u4ImgReszID);
	}

	vHwImgRezeReset(u4ImgReszID);
	vHwImgReszTempMMUSet(u4ImgReszID);
	/*i4HwImgReszSetDramReqBurstLimit(u4ImgReszID,8); // Set Dram Request Burst Limit.0*/
	vHwImgIntResize(u4ImgReszID);/*sun for turning on  interrupt*/

	vHwImgRezeSetWaitWRDone(u4ImgReszID);

	memset(&(_rImgReszHalInfo[u4ImgReszID]), 0, sizeof(IMGRESZ_HAL_INFO_T));

	if (fgMMUTable) {
                BOOL fg2way = TRUE;
                fg2way = FALSE;
		/*_rImgReszHalInfo[u4ImgReszID].fgUserTable = TRUE;*/
		vImgreszSetMMU(u4ImgReszID, fg2way);
		vImgreszSetUserTable(u4tableaddr);
	}

	/* Variable initialization*/
	_rImgReszHalInfo[u4ImgReszID].rJpegInfo.fgYExist = TRUE;
	_rImgReszHalInfo[u4ImgReszID].rJpegInfo.fgCbExist = TRUE;
	_rImgReszHalInfo[u4ImgReszID].rJpegInfo.fgCrExist = TRUE;
	_rImgReszHalInfo[u4ImgReszID].rBldImgInfo.u1Alpha = 0xFF;
#if IMGRESZ_SUPPORT_RESET_DEST_BUFFER
	_rImgReszHalInfo[u4ImgReszID].fgResetDstBuf = FALSE;
#endif
	_rImgReszHalInfo[u4ImgReszID].fgScale1to1 = FALSE;
	_rImgReszHalInfo[u4ImgReszID].fgScale4to1 = FALSE;
	_rImgReszHalInfo[u4ImgReszID].fgLumaKeyEnable = FALSE;
	return S_IMGRESZ_HAL_OK;
}


/*/ Uninitialize Image Resizer HAL*/
/*/ \return If return value < 0, it's failed. Please reference imgresz_hal_errcode.h.*/
s32 i4ImgResz_HAL_Uninit(
	u32 u4ImgReszID                               /*/< [IN] Image Resizer hardware ID*/
)
{
	if (u4ImgReszID >= HW_IMGRESZ_NUM) {
		return E_IMGRESZ_HAL_FAIL;
	}


#if 1/*IMGRESZ_IO_MMU_TEST*/
	vImgresz_MMUSet_After_Resize(u4ImgReszID);
	vHwImgReszDisableMMU(u4ImgReszID);
#endif

	vHwImgRezeReset(u4ImgReszID);
	vHwImgReszDisable(u4ImgReszID);

#if IMGRESZ_SUPPORT_AGENT_ON_OFF

	if (u4ImgReszID == 0)
		/*CKGEN_AgtOffClk(e_CLK_IMG_RESZ);*/
	{
/*		clk_disable_unprepare(clk_ac8317_imgr0);
		if ((*(u32 *)0xfd0000a0 & (1<<4)) !=0 || (*(u32 *)0xfd0000bc & (1<<4)) != 0)
			pr_err("IMGR0 unprepare error:0xA0=%x;0xBC=%x;\n", *(u32 *)0xfd0000a0, *(u32 *)0xfd0000bc);*/
	} else
		//CKGEN_AgtOffClk(e_CLK_OSD_RESZ);
	{
		unsigned int add1=0xf00000a0;
		unsigned int add2=0xf00000bc;

		unsigned int value = *((volatile unsigned int*)(add1));
		value = value | 0x0<<5;
		*((volatile unsigned int*)(add1)) =value;

		value = *((volatile unsigned int*)(add2));
		value = value | 0x0<<5;
		*((volatile unsigned int*)(add2)) =value;

		IMGR_LOG(IMGR_LOG_LVL_DBG,"a0 is %x,bc is %x",*((volatile unsigned int*)(add1)),*((volatile unsigned int*)(add2)));
	}

#endif

	return S_IMGRESZ_HAL_OK;
}

/*/ Gracefully reset*/
/*/ \return If return value < 0, it's failed. Please reference imgresz_hal_errcode.h.*/
s32 i4ImgResz_Gracefully_Reset(
	u32 u4ImgReszID                               /*/< [IN] Image Resizer hardware ID*/
)
{
	if (u4ImgReszID >= HW_IMGRESZ_NUM) {
		return E_IMGRESZ_HAL_FAIL;
	}


	vHwImgRezeDMAReset(u4ImgReszID);
#if 0

	while (TRUE) {
		if (fgHwImgReszDMARstFinish(u4ImgReszID)) {
			break;
		}
	}

#endif
	//WAIT_FOR_STATUS(fgHwImgReszDMARstFinish(u4ImgReszID), 20, "[ImgResz]DMA reset timeout \r\n");

	vHwImgRezeReset(u4ImgReszID);

	return S_IMGRESZ_HAL_OK;
}

/*/ Set Image Resizer HAL resize mode*/
/*/ \return If return value < 0, it's failed. Please reference imgresz_hal_errcode.h.*/
s32 i4ImgResz_HAL_Set_Resize_Mode(
	u32 u4ImgReszID,                              /*/< [IN] Image Resizer hardware ID*/

	IMGRESZ_HAL_RESIZE_MODE_T eResizeMode            /*/< [IN] Resize mode*/
)
{
	if (u4ImgReszID >= HW_IMGRESZ_NUM) {
		return E_IMGRESZ_HAL_FAIL;
	}

	_rImgReszHalInfo[u4ImgReszID].eResizeMode = eResizeMode;

	return S_IMGRESZ_HAL_OK;
}


/*/ Set Image Resizer HAL resample method*/
/*/ \return If return value < 0, it's failed. Please reference hal_img_resizer_errcode.h.*/
s32 i4ImgResz_HAL_Set_Resample_Method(
	u32 u4ImgReszID,                              /*/< [IN] Image Resizer hardware ID*/
	IMGRESZ_HAL_RESAMPLE_METHOD_T eHResampleMethod,  /*/< [IN] Horizontal Resample method*/

	IMGRESZ_HAL_RESAMPLE_METHOD_T eVResampleMethod   /*/< [IN] Vertical Resample method*/
)
{
	if (u4ImgReszID >= HW_IMGRESZ_NUM) {
		return E_IMGRESZ_HAL_FAIL;
	}

	_rImgReszHalInfo[u4ImgReszID].eHResampleMethod = eHResampleMethod;
	_rImgReszHalInfo[u4ImgReszID].eVResampleMethod = eVResampleMethod;

	return S_IMGRESZ_HAL_OK;
}


/*/ Set Image Resizer HAL source image info.*/
/*/ \return If return value < 0, it's failed. Please reference hal_img_resizer_errcode.h.*/
s32 i4ImgResz_HAL_Set_Source_Image_Info(
	u32 u4ImgReszID,                              /*/< [IN] Image Resizer hardware ID*/

	IMGRESZ_HAL_IMG_INFO_T *prSrcImgInfo             /*/< [IN] Source image infomation.*/
)
{
	if (u4ImgReszID >= HW_IMGRESZ_NUM) {
		return E_IMGRESZ_HAL_FAIL;
	}

	memcpy(&(_rImgReszHalInfo[u4ImgReszID].rSrcImgInfo), prSrcImgInfo, sizeof(IMGRESZ_HAL_IMG_INFO_T));

	return S_IMGRESZ_HAL_OK;
}


/*/ Set Image Resizer HAL destination image info.*/
/*/ \return If return value < 0, it's failed. Please reference hal_img_resizer_errcode.h.*/
s32 i4ImgResz_HAL_Set_Destination_Image_Info(
	u32 u4ImgReszID,                              /*/< [IN] Image Resizer hardware ID*/

	IMGRESZ_HAL_IMG_INFO_T *prDestImgInfo            /*/< [IN] Destination image infomation.*/
)
{
	if (u4ImgReszID >= HW_IMGRESZ_NUM) {
		return E_IMGRESZ_HAL_FAIL;
	}

	memcpy(&(_rImgReszHalInfo[u4ImgReszID].rDestImgInfo), prDestImgInfo, sizeof(IMGRESZ_HAL_IMG_INFO_T));

	return S_IMGRESZ_HAL_OK;
}


/*/ Set Image Resizer HAL blending image info.*/
/*/ \return If return value < 0, it's failed. Please reference hal_img_resizer_errcode.h.*/
s32 i4ImgResz_HAL_Set_Blending_Image_Info(
	u32 u4ImgReszID,                              /*/< [IN] Image Resizer hardware ID*/

	IMGRESZ_HAL_IMG_INFO_T *prBldImgInfo             /*/< [IN] Blending image infomation.*/
)
{
	if (u4ImgReszID >= HW_IMGRESZ_NUM) {
		return E_IMGRESZ_HAL_FAIL;
	}

	memcpy(&(_rImgReszHalInfo[u4ImgReszID].rBldImgInfo), prBldImgInfo, sizeof(IMGRESZ_HAL_IMG_INFO_T));

	return S_IMGRESZ_HAL_OK;
}


/*/ Set Image Resizer HAL partial mode information*/
/*/ \return If return value < 0, it's failed. Please reference hal_img_resizer_errcode.h.*/
s32 i4ImgResz_HAL_Set_Partial_Mode_Info(
	u32 u4ImgReszID,                              /*/< [IN] Image Resizer hardware ID*/

	IMGRESZ_HAL_PARTIAL_BUF_INFO_T *prSrcRowBufInfo, /*/< [IN] Source row buffer infomation.*/

	uintptr_t u4TempBufSa      /*/< [IN] The start address of temporary buffer for partial mode.*/
	/*/<      The size of temp buffer is destination image width * 1 bytes.*/
)
{
	if (u4ImgReszID >= HW_IMGRESZ_NUM) {
		return E_IMGRESZ_HAL_FAIL;
	}

	memcpy(&(_rImgReszHalInfo[u4ImgReszID].rSrcRowBufInfo),
		prSrcRowBufInfo, sizeof(IMGRESZ_HAL_PARTIAL_BUF_INFO_T));
	_rImgReszHalInfo[u4ImgReszID].u4TempBufSa = u4TempBufSa;

	return S_IMGRESZ_HAL_OK;
}

#if IMGRESZ_SUPPORT_RESET_DEST_BUFFER
s32 i4ImgResz_HAL_Set_DstBuf_Reset(
	u32 u4ImgReszID,

	BOOL fgReset
)
{
	if (u4ImgReszID >= HW_IMGRESZ_NUM) {
		return E_IMGRESZ_HAL_FAIL;
	}

	_rImgReszHalInfo[u4ImgReszID].fgResetDstBuf = fgReset;

	return S_IMGRESZ_HAL_OK;
}
#endif

/*/ Set Image Resizer HAL Jpeg information*/
/*/ \return If return value < 0, it's failed. Please reference hal_img_resizer_errcode.h.*/
s32 i4ImgResz_HAL_Set_Jpeg_Info(
	u32 u4ImgReszID,                              /*/< [IN] Image Resizer hardware ID*/

	IMGRESZ_HAL_JPEG_INFO_T *prJpegInfo              /*/< [IN] Jpeg information.*/
)
{
	if (u4ImgReszID >= HW_IMGRESZ_NUM) {
		return E_IMGRESZ_HAL_FAIL;
	}

	memcpy(&(_rImgReszHalInfo[u4ImgReszID].rJpegInfo), prJpegInfo, sizeof(IMGRESZ_HAL_JPEG_INFO_T));

	return S_IMGRESZ_HAL_OK;
}

/*/ Set Image Resizer HAL RM(RPR) information*/
/*/ \return If return value < 0, it's failed. Please reference hal_img_resizer_errcode.h.*/
s32 i4ImgResz_HAL_Set_RM_Info(
	u32 u4ImgReszID,                              /*/< [IN] Image Resizer hardware ID*/

	IMGRESZ_HAL_RM_INFO_T *prRMInfo              /*/< [IN] rm information.*/
)
{
	if (u4ImgReszID >= HW_IMGRESZ_NUM) {
		return E_IMGRESZ_HAL_FAIL;
	}

	memcpy(&(_rImgReszHalInfo[u4ImgReszID].rRMInfo), prRMInfo, sizeof(IMGRESZ_HAL_RM_INFO_T));

	return S_IMGRESZ_HAL_OK;
}


s32 i4ImgResz_HAL_Set_LumaKey(
	u32 u4ImgReszID,                              /*/< [IN] Image Resizer hardware ID*/

	BOOL fgEnable,                                   /*/< [IN] Luma key enable flag*/

	u8 u1LumaKey                                  /*/< [IN] Luma key value*/
)
{
	if (u4ImgReszID >= HW_IMGRESZ_NUM) {
		return E_IMGRESZ_HAL_FAIL;
	}

	_rImgReszHalInfo[u4ImgReszID].fgLumaKeyEnable = fgEnable;
	_rImgReszHalInfo[u4ImgReszID].u1LumaKey = u1LumaKey;

	return S_IMGRESZ_HAL_OK;
}

/*sun for encoder new application*/
s32 i4ImgResz_HAL_Set_Scale1to1(
	u32 u4ImgReszID,                              /*/< [IN] Image Resizer hardware ID*/

	BOOL fgEnable                                 /*/< [IN] scale1:1 enable flag*/
)
{
	if (u4ImgReszID >= HW_IMGRESZ_NUM) {
		return E_IMGRESZ_HAL_FAIL;
	}

	_rImgReszHalInfo[u4ImgReszID].fgScale1to1 = fgEnable;

	return S_IMGRESZ_HAL_OK;
}

/*sun for encoder new application*/
s32 i4ImgResz_HAL_Set_Scale4to1(
	u32 u4ImgReszID,                              /*/< [IN] Image Resizer hardware ID*/

	BOOL fgEnable                                 /*/< [IN] scale4:1 enable flag*/
)
{
	if (u4ImgReszID >= HW_IMGRESZ_NUM) {
		return E_IMGRESZ_HAL_FAIL;
	}

	_rImgReszHalInfo[u4ImgReszID].fgScale4to1 = fgEnable;

	return S_IMGRESZ_HAL_OK;
}



/*/ Image Resizer HAL do resize.*/
/*/ \return If return value < 0, it's failed. Please reference hal_img_resizer_errcode.h.*/
s32 i4ImgResz_HAL_Resize(
	u32 u4ImgReszID                               /*/< [IN] Image Resizer hardware ID*/
)
{
	/*u32 cnt = 0;*/
	/*u32 i;*/
	/*u32 j;*/
	IMGRESZ_HAL_INFO_T *prImgReszHalInfo;

	if (u4ImgReszID >= HW_IMGRESZ_NUM) {
		return E_IMGRESZ_HAL_FAIL;
	}

	prImgReszHalInfo = &(_rImgReszHalInfo[u4ImgReszID]);
	/*IMGR_LOG(IMGR_LOG_LVL_DBG, "\n i4ImgResz_HAL_Resize   start *********\n");*/

	/*0. Set RM mode to HW*/
	if (prImgReszHalInfo->rRMInfo.fgRPRMode) {
		if (prImgReszHalInfo->rRMInfo.fgRPRRacingModeEnable) {
			i4HwImgReszSetRPR(u4ImgReszID, TRUE, TRUE);
		} else {
			i4HwImgReszSetRPR(u4ImgReszID, TRUE, FALSE);
		}
	} else {
		i4HwImgReszSetRPR(u4ImgReszID, FALSE, FALSE);
	}

	/* FLIP Enable wchao wang*/
	i4HwImgReszSetFLIP(u4ImgReszID, FALSE);
	/*Swap Enable wchao wang 3360 don`t support*/
	/*i4HwImgReszSetSwap(u4ImgReszID,TRUE);*/
	/*IMGR_LOG(IMGR_LOG_LVL_DBG, "\n i4ImgResz_HAL_Resize   start 11111111111111111111111111111111111111**\n");*/

	if ((prImgReszHalInfo->eResizeMode == IMGRESZ_HAL_RESIZE_MODE_FRAME) ||
	    ((prImgReszHalInfo->eResizeMode == IMGRESZ_HAL_RESIZE_MODE_PARTIAL) &&
	     (prImgReszHalInfo->rSrcRowBufInfo.fgFirstRowBuf || prImgReszHalInfo->fgResume))) {

		/* 1. Set Resize Mode to HW*/
		i4HwImgReszSetResizeMode(u4ImgReszID, prImgReszHalInfo->eResizeMode,
					 &(prImgReszHalInfo->rSrcImgInfo.rBufferFormat));

		/* 2. Set Resample Method to HW*/
		i4HwImgReszSetResampleMethod(u4ImgReszID, prImgReszHalInfo->eHResampleMethod,
					     prImgReszHalInfo->eVResampleMethod);

		/* 3. Set Source Information to HW*/
		i4HwImgReszSetSrcBufFormat(u4ImgReszID, &(prImgReszHalInfo->rSrcImgInfo.rBufferFormat));

		if (prImgReszHalInfo->eResizeMode == IMGRESZ_HAL_RESIZE_MODE_FRAME) {
			i4HwImgReszSetSrcBufAddr(u4ImgReszID, prImgReszHalInfo->rSrcImgInfo.u4BufSA1,
						 prImgReszHalInfo->rSrcImgInfo.u4BufSA2,
						 prImgReszHalInfo->rSrcImgInfo.u4BufSA3);

			if (prImgReszHalInfo->rRMInfo.fgRPRRacingModeEnable) { /*sun new*/
				prImgReszHalInfo->rSrcImgInfo.u4BufHeight  = 0x00000020;
			}

			i4HwImgReszSetSrcRowBufHeight(u4ImgReszID, prImgReszHalInfo->rSrcImgInfo.u4BufHeight,
						      &(prImgReszHalInfo->rSrcImgInfo.rBufferFormat),
						      prImgReszHalInfo->rRMInfo.fgRPRRacingModeEnable);
		} else if (prImgReszHalInfo->eResizeMode == IMGRESZ_HAL_RESIZE_MODE_PARTIAL) {
			i4HwImgReszSetSrcBufAddr(u4ImgReszID, prImgReszHalInfo->rSrcRowBufInfo.u4CurRowBufSA1,
						 prImgReszHalInfo->rSrcRowBufInfo.u4CurRowBufSA2,
						 prImgReszHalInfo->rSrcRowBufInfo.u4CurRowBufSA3);
			i4HwImgReszSetSrcRowBufHeight(u4ImgReszID, prImgReszHalInfo->rSrcRowBufInfo.u4RowBufHeight,
						&(prImgReszHalInfo->rSrcImgInfo.rBufferFormat), FALSE); /*sun new*/

			if (prImgReszHalInfo->rJpegInfo.fgPictureMode || prImgReszHalInfo->fgResume) {
				i4HwImgReszSetSrcPrevRowBufAddr(u4ImgReszID,
								prImgReszHalInfo->rSrcRowBufInfo.u4PrevRowBufSA1,
								prImgReszHalInfo->rSrcRowBufInfo.u4PrevRowBufSA2,
								prImgReszHalInfo->rSrcRowBufInfo.u4PrevRowBufSA3);
			}
		}

		i4HwImgReszSetSrcBufWidth(u4ImgReszID, prImgReszHalInfo->rSrcImgInfo.u4BufWidth,
			&(prImgReszHalInfo->rSrcImgInfo.rBufferFormat));

		if (prImgReszHalInfo->rDestImgInfo.fgClipen == FALSE) {
			i4HwImgReszSetSrcImageWidthHeight(u4ImgReszID, prImgReszHalInfo->rSrcImgInfo.u4ImgWidth,
							  prImgReszHalInfo->rSrcImgInfo.u4ImgHeight,
							  &(prImgReszHalInfo->rSrcImgInfo.rBufferFormat));
		}

		i4HwImgReszSetSrcImageOffset(u4ImgReszID, prImgReszHalInfo->rSrcImgInfo.u4ImgXOff,
					     prImgReszHalInfo->rSrcImgInfo.u4ImgYOff,
					     &(prImgReszHalInfo->rSrcImgInfo.rBufferFormat),
					     prImgReszHalInfo->rSrcImgInfo.u4ImgHeight,
					     prImgReszHalInfo->rDestImgInfo.u4ImgHeight);

		if (prImgReszHalInfo->eResizeMode == IMGRESZ_HAL_RESIZE_MODE_FRAME) {
			i4HwImgReszSetSrcFirstRow(u4ImgReszID, TRUE);

			if (prImgReszHalInfo->rRMInfo.fgRPRRacingModeEnable) {
				i4HwImgReszSetSrcLastRow(u4ImgReszID, FALSE);
			} else {
				i4HwImgReszSetSrcLastRow(u4ImgReszID, TRUE);
			}
		} else if (prImgReszHalInfo->eResizeMode == IMGRESZ_HAL_RESIZE_MODE_PARTIAL) {
			i4HwImgReszSetSrcFirstRow(u4ImgReszID, prImgReszHalInfo->rSrcRowBufInfo.fgFirstRowBuf);
			i4HwImgReszSetSrcLastRow(u4ImgReszID, prImgReszHalInfo->rSrcRowBufInfo.fgLastRowBuf);
		}

		if (prImgReszHalInfo->rSrcImgInfo.rBufferFormat.eBufferMainFormat ==
			IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_INDEX_BUFFER) {
			i4HwImgReszSetIndexBufColorTranslation(u4ImgReszID);
			i4HwImgReszSetColorPalletTable(u4ImgReszID,
				(u32)prImgReszHalInfo->rSrcImgInfo.rBufferFormat.prColorPallet);
		}

		if (prImgReszHalInfo->fgScale4to1) { /*sun for encode new application*/
			i4HwImgReszSetJpegComponent(u4ImgReszID, TRUE, FALSE, FALSE);
		}

		/* 4. Set Destination Information to HW*/
		i4HwImgReszSetDestBufFormat(u4ImgReszID, &(prImgReszHalInfo->rSrcImgInfo.rBufferFormat),
					    &(prImgReszHalInfo->rDestImgInfo.rBufferFormat));
		i4HwImgReszSetDestBufAddr(u4ImgReszID, prImgReszHalInfo->rDestImgInfo.u4BufSA1,
					  prImgReszHalInfo->rDestImgInfo.u4BufSA2);

		if (prImgReszHalInfo->rDestImgInfo.fgClipen == TRUE) {
			i4HwImgReszSetDestBufWidth(u4ImgReszID, prImgReszHalInfo->rDestImgInfo.u4BufWidth);
			i4HwImgReszSetDestImageWidthHeight(u4ImgReszID, prImgReszHalInfo->rDestImgInfo.u4ClipImgWidth,
							   prImgReszHalInfo->rDestImgInfo.u4ClipImgHeight);
		} else {
			i4HwImgReszSetDestBufWidth(u4ImgReszID, prImgReszHalInfo->rDestImgInfo.u4BufWidth);
			i4HwImgReszSetDestImageWidthHeight(u4ImgReszID, prImgReszHalInfo->rDestImgInfo.u4ImgWidth,
							   prImgReszHalInfo->rDestImgInfo.u4ImgHeight);
		}

		i4HwImgReszSetDestImageOffset(u4ImgReszID, prImgReszHalInfo->rDestImgInfo.u4ImgXOff,
					      prImgReszHalInfo->rDestImgInfo.u4ImgYOff);

		/* 5. Set Blending Buffer Information to HW*/
		if ((prImgReszHalInfo->rSrcImgInfo.u1Alpha != 0xFF) &&
			(prImgReszHalInfo->rSrcImgInfo.fgPreloadBuf == TRUE)) {
			i4HwImgReszSetAlphaBlendingLevel(u4ImgReszID, (u32)(prImgReszHalInfo->rSrcImgInfo.u1Alpha));
			i4HwImgReszSetPreloadBufAddr(u4ImgReszID,
				prImgReszHalInfo->rBldImgInfo.u4BufSA1, prImgReszHalInfo->rBldImgInfo.u4BufSA2);
		}

		/* Clip wchao wang*/
		if (prImgReszHalInfo->rDestImgInfo.fgClipen == TRUE) {
			/*
			IMGR_LOG(IMGR_LOG_LVL_DBG, "rSrcImgInfo.u4ImgWidth = %d,  rDestImgInfo.u4ImgWidth = %d,
				rDestImgInfo.u4ClipImgWidth = %d, rDestImgInfo.u4ClipImgXOff = %d \r\n",
				 (int)prImgReszHalInfo->rSrcImgInfo.u4ImgWidth,
				 (int)prImgReszHalInfo->rDestImgInfo.u4ImgWidth,
				 (int)prImgReszHalInfo->rDestImgInfo.u4ClipImgWidth,
				 (int)prImgReszHalInfo->rDestImgInfo.u4ClipImgXOff);
			IMGR_LOG(IMGR_LOG_LVL_DBG, "rSrcImgInfo.u4ImgHeight = %d,  rDestImgInfo.u4ImgHeight = %d,
				rDestImgInfo.u4ClipImgHeight = %d, rDestImgInfo.u4ClipImgYOff = %d \r\n",
				 (int)prImgReszHalInfo->rSrcImgInfo.u4ImgHeight,
				 (int)prImgReszHalInfo->rDestImgInfo.u4ImgHeight,
				 (int)prImgReszHalInfo->rDestImgInfo.u4ClipImgHeight,
				 (int)prImgReszHalInfo->rDestImgInfo.u4ClipImgYOff);
					*/
			i4HwImgReszSetClipHScaleFactor(u4ImgReszID, prImgReszHalInfo->rSrcImgInfo.u4ImgWidth,
						       prImgReszHalInfo->rDestImgInfo.u4ImgWidth,
						       prImgReszHalInfo->rDestImgInfo.u4ClipImgWidth,
						       prImgReszHalInfo->rDestImgInfo.u4ClipImgXOff,
						       &(prImgReszHalInfo->rSrcImgInfo.u4SrcClipYWidth),
						       &(prImgReszHalInfo->rSrcImgInfo.u4SrcClipCWidth),
						       prImgReszHalInfo->eHResampleMethod,
						       &(prImgReszHalInfo->rSrcImgInfo.rBufferFormat),
						       &(prImgReszHalInfo->rDestImgInfo.rBufferFormat));

			i4HwImgReszSetClipVScaleFactor(u4ImgReszID, prImgReszHalInfo->rSrcImgInfo.u4ImgHeight,
						       prImgReszHalInfo->rDestImgInfo.u4ImgHeight,
						       prImgReszHalInfo->rDestImgInfo.u4ClipImgHeight,
						       prImgReszHalInfo->rDestImgInfo.u4ClipImgYOff,
						       &(prImgReszHalInfo->rSrcImgInfo.u4SrcClipYHeight),
						       &(prImgReszHalInfo->rSrcImgInfo.u4SrcClipCHeight),
						       prImgReszHalInfo->eVResampleMethod,
						       &(prImgReszHalInfo->rSrcImgInfo.rBufferFormat),
						       &(prImgReszHalInfo->rDestImgInfo.rBufferFormat));
			i4HwImgReszSetSrcImageClipWidthHeight(u4ImgReszID,
				prImgReszHalInfo->rSrcImgInfo.u4SrcClipYWidth,
				prImgReszHalInfo->rSrcImgInfo.u4SrcClipCWidth,
				prImgReszHalInfo->rSrcImgInfo.u4SrcClipYHeight,
				prImgReszHalInfo->rSrcImgInfo.u4SrcClipCHeight,
				&(prImgReszHalInfo->rSrcImgInfo.rBufferFormat));

		} else {
			/* 6. Set Scale Factor to HW*/
			if (prImgReszHalInfo->rRMInfo.fgRPRMode) { /*sun new*/
				i4HwImgReszSetRPRHScaleFactor(u4ImgReszID,
					(s32)prImgReszHalInfo->rSrcImgInfo.u4ImgWidth,
					(s32)prImgReszHalInfo->rDestImgInfo.u4ImgWidth);
				i4HwImgReszSetRPRVScaleFactor(u4ImgReszID,
					(s32)prImgReszHalInfo->rSrcImgInfo.u4ImgHeight,
					(s32)prImgReszHalInfo->rDestImgInfo.u4ImgHeight);
			} else {
				if (prImgReszHalInfo->fgScale1to1) { /*sun for encode new application*/
					i4HwImgReszSetScale1to1Factor(u4ImgReszID,
						&(prImgReszHalInfo->rSrcImgInfo.rBufferFormat));
				} else if (prImgReszHalInfo->fgScale4to1) {
					i4HwImgReszSetHScaleFactor(u4ImgReszID,
								   prImgReszHalInfo->rSrcImgInfo.u4ImgWidth,
								   (prImgReszHalInfo->rSrcImgInfo.u4ImgWidth / 4),
								   prImgReszHalInfo->eHResampleMethod,
								   &(prImgReszHalInfo->rSrcImgInfo.rBufferFormat),
								   &(prImgReszHalInfo->rDestImgInfo.rBufferFormat));
					i4HwImgReszSetVScaleFactor(u4ImgReszID,
								   prImgReszHalInfo->rSrcImgInfo.u4ImgHeight,
								   (prImgReszHalInfo->rSrcImgInfo.u4ImgHeight / 4),
								   prImgReszHalInfo->eVResampleMethod,
								   &(prImgReszHalInfo->rSrcImgInfo.rBufferFormat),
								   &(prImgReszHalInfo->rDestImgInfo.rBufferFormat));
				} else {

					i4HwImgReszSetHScaleFactor(u4ImgReszID,
								   prImgReszHalInfo->rSrcImgInfo.u4ImgWidth,
								   prImgReszHalInfo->rDestImgInfo.u4ImgWidth,
								   prImgReszHalInfo->eHResampleMethod,
								   &(prImgReszHalInfo->rSrcImgInfo.rBufferFormat),
								   &(prImgReszHalInfo->rDestImgInfo.rBufferFormat));
					i4HwImgReszSetVScaleFactor(u4ImgReszID,
								   prImgReszHalInfo->rSrcImgInfo.u4ImgHeight,
								   prImgReszHalInfo->rDestImgInfo.u4ImgHeight,
								   prImgReszHalInfo->eVResampleMethod,
								   &(prImgReszHalInfo->rSrcImgInfo.rBufferFormat),
								   &(prImgReszHalInfo->rDestImgInfo.rBufferFormat));
				}
			}
		}

		i4HwImgReszSetTmpLineBufLen(u4ImgReszID, prImgReszHalInfo->eVResampleMethod,
					    prImgReszHalInfo->rSrcImgInfo.u4ImgHeight,
					    prImgReszHalInfo->rDestImgInfo.u4ImgHeight,
					    prImgReszHalInfo->rDestImgInfo.u4ImgWidth,
					    prImgReszHalInfo->rRMInfo.fgRPRMode,
					    prImgReszHalInfo->fgLumaKeyEnable);


		/* 7. Jpeg info setting*/
		if (prImgReszHalInfo->eResizeMode == IMGRESZ_HAL_RESIZE_MODE_PARTIAL
		    || prImgReszHalInfo->rRMInfo.fgRPRMode == TRUE /*sun new*/
		   ) {
			i4HwImgReszSetTempBufAddr(u4ImgReszID, prImgReszHalInfo->u4TempBufSa);
		} else if ((!prImgReszHalInfo->rSrcImgInfo.rBufferFormat.fgJpg) &&
			   (prImgReszHalInfo->rSrcImgInfo.rBufferFormat.eBufferMainFormat ==
				IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_CB_CR_BUFFER)) {
			i4HwImgReszSetTempBufAddr(u4ImgReszID, prImgReszHalInfo->u4TempBufSa);
		}

		i4HwImgReszSetJpegPicMode(u4ImgReszID, prImgReszHalInfo->rJpegInfo.fgPictureMode);
		i4HwImgReszSetJpegPreloadMode(u4ImgReszID, prImgReszHalInfo->rJpegInfo.fgPreloadMode);

		if (prImgReszHalInfo->rRMInfo.fgRPRRacingModeEnable) { /*sun new*/
			i4HwImgReszSetJpegComponent(u4ImgReszID, TRUE, TRUE, TRUE);
			i4HwImgReszSetJpegComponentExt(u4ImgReszID, TRUE, TRUE, TRUE, TRUE);
		} else {
			i4HwImgReszSetJpegComponent(u4ImgReszID, prImgReszHalInfo->rJpegInfo.fgYExist,
						    prImgReszHalInfo->rJpegInfo.fgCbExist,
						    prImgReszHalInfo->rJpegInfo.fgCrExist);


			if ((prImgReszHalInfo->rJpegInfo.fgPictureMode) ||
			    (prImgReszHalInfo->rSrcImgInfo.rBufferFormat.eBufferMainFormat ==
				IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_CB_CR_BUFFER)) {
				i4HwImgReszSetJpegComponentExt(u4ImgReszID, prImgReszHalInfo->rJpegInfo.fgYExist,
							       prImgReszHalInfo->rJpegInfo.fgCbExist,
							       prImgReszHalInfo->rJpegInfo.fgCrExist,
							       FALSE);
			}
		}

		if (prImgReszHalInfo->rJpegInfo.fgPreloadMode) {
			i4HwImgReszSetPreloadBufAddr(u4ImgReszID, prImgReszHalInfo->rDestImgInfo.u4BufSA1,
						     prImgReszHalInfo->rDestImgInfo.u4BufSA2);
		}

		/* 8. Miscellaneous setting*/
		{
			static u32 u4DramBurstLimit = 8;

#if 0/*def IMGRESZ_HAL_EMU*/

			switch (u4DramBurstLimit) {
			case 8:
				u4DramBurstLimit = 4;
				break;

			case 4:
				u4DramBurstLimit = 2;
				break;

			case 2:
				u4DramBurstLimit = 8;
				break;
			}

#endif
			i4HwImgReszSetDramReqBurstLimit(u4ImgReszID, u4DramBurstLimit);
		}

		if (prImgReszHalInfo->rDestImgInfo.rBufferFormat.fgWT == FALSE &&
			prImgReszHalInfo->rDestImgInfo.rBufferFormat.fgWT == FALSE)
			/*vHwImgReszReadBurstLength(u4ImgReszID,TRUE);*/
		{
			vHwImgReszReadBurstLength(u4ImgReszID, FALSE);       /* for imgresz time out bug*/
		} else {
			vHwImgReszReadBurstLength(u4ImgReszID, FALSE);
		}

		if (prImgReszHalInfo->fgLumaKeyEnable) {
			vHwImgReszReadBurstLength(u4ImgReszID, FALSE);
		} else {
			vHwImgReszReadBurstLength(u4ImgReszID, FALSE);
		}

#if IMGRESZ_SUPPORT_RESET_DEST_BUFFER
		i4HwImgReszResetDestPartialBuf(u4ImgReszID, prImgReszHalInfo->fgResetDstBuf);
#endif
#ifndef IMGRESZ_HAL_EMU
		i4HwImgReszSetAlphaChangeScalingType(u4ImgReszID, 0);
#endif

	} else { /* Partial mode and not first row*/
		/* 3. Set Source Information to HW*/
		i4HwImgReszSetSrcBufAddr(u4ImgReszID, prImgReszHalInfo->rSrcRowBufInfo.u4CurRowBufSA1,
					 prImgReszHalInfo->rSrcRowBufInfo.u4CurRowBufSA2,
					 prImgReszHalInfo->rSrcRowBufInfo.u4CurRowBufSA3);
		i4HwImgReszSetSrcPrevRowBufAddr(u4ImgReszID, prImgReszHalInfo->rSrcRowBufInfo.u4PrevRowBufSA1,
						prImgReszHalInfo->rSrcRowBufInfo.u4PrevRowBufSA2,
						prImgReszHalInfo->rSrcRowBufInfo.u4PrevRowBufSA3);
		i4HwImgReszSetSrcFirstRow(u4ImgReszID, prImgReszHalInfo->rSrcRowBufInfo.fgFirstRowBuf);
		i4HwImgReszSetSrcLastRow(u4ImgReszID, prImgReszHalInfo->rSrcRowBufInfo.fgLastRowBuf);

#if 0/*IMGRESZ_SUPPORT_RESET_DEST_BUFFER*/

		/*IMGR_LOG(IMGR_LOG_LVL_DBG, "dst buffer1 = %x dst buffer2 = %x resetDstbuf = %d \r\n",*/
		/*            prImgReszHalInfo->rDestImgInfo.u4BufSA1,*/
		/*            prImgReszHalInfo->rDestImgInfo.u4BufSA2,*/
		/*            prImgReszHalInfo->fgResetDstBuf);*/
		if (prImgReszHalInfo->fgResetDstBuf) {
			i4HwImgReszSetDestBufAddr(u4ImgReszID, prImgReszHalInfo->rDestImgInfo.u4BufSA1,
						  prImgReszHalInfo->rDestImgInfo.u4BufSA2);
		}

		i4HwImgReszResetDestPartialBuf(u4ImgReszID, prImgReszHalInfo->fgResetDstBuf);
#endif
	}

	/* Alpha scaling type --- wang chao*/
	i4HwImgReszSetAlphaChangeScalingType(u4ImgReszID, prImgReszHalInfo->rSrcImgInfo.u4AlphaScalingType);
	i4HwImgReszSetAlphaChangeBilinearBoundary(u4ImgReszID, prImgReszHalInfo->rSrcImgInfo.fgBilinearBoundary);
	i4HwImgReszSetOnlyDistinguishAlpha(u4ImgReszID, prImgReszHalInfo->rSrcImgInfo.fgOnlyDistinquishAlpha);

	if (prImgReszHalInfo->fgLumaKeyEnable) {
		i4HwImgReszSetLumaKeyEnable(u4ImgReszID, TRUE);
		i4HwImgReszSetLumaKeyScalingType(u4ImgReszID, prImgReszHalInfo->rSrcImgInfo.fgBilinearBoundary);
		i4HwImgReszSetLumaKey(u4ImgReszID, prImgReszHalInfo->u1LumaKey);
	} else {
		i4HwImgReszSetLumaKeyEnable(u4ImgReszID, FALSE);
	}

	/* 9. Do resize*/
	prImgReszHalInfo->fgResizeComplete = FALSE;
	prImgReszHalInfo->fgResume = FALSE;
	prImgReszHalInfo->fgScaling = TRUE;
	//HalFlushInvalidateDCache();
	/*vHwImgReszPrinterRegister(u4ImgReszID);   // print register log*/
	vHwImgReszResize(u4ImgReszID);

	return S_IMGRESZ_HAL_OK;
}



/*/ Image Resizer HAL Get resize status.*/
/*/ \return If return value < 0, it's failed. Please reference hal_img_resizer_errcode.h.*/
s32 i4ImgResz_HAL_Get_Resize_Status(
	u32 u4ImgReszID                               /*/< [IN] Image Resizer hardware ID*/
)
{
	if (u4ImgReszID >= HW_IMGRESZ_NUM) {
		return E_IMGRESZ_HAL_FAIL;
	}


	if (!fgHwImgReszResizeFinish(u4ImgReszID)) {
		return E_IMGRESZ_HAL_FAIL;
	}

	/*if(!_rImgReszHalInfo[u4ImgReszID].fgResizeComplete)*/
	/*return E_IMGRESZ_HAL_FAIL;*/

	return S_IMGRESZ_HAL_OK;
}


/*/ Image Resizer HAL Get HW status.*/
/*/ \return If return value < 0, it's failed. Please reference hal_img_resizer_errcode.h.*/
s32 i4ImgResz_HAL_Get_HW_Status(
	u32 u4ImgReszID,                              /*/< [IN] Image Resizer hardware ID*/

	IMGRESZ_HAL_HW_STATUS_T * prHwStatus              /*/< [OUT] Hardware status*/
)
{
	if (u4ImgReszID >= HW_IMGRESZ_NUM) {
		return E_IMGRESZ_HAL_FAIL;
	}

	if (prHwStatus == NULL) {
		return E_IMGRESZ_HAL_FAIL;
	}

	i4HwImgReszGetHwStatus(u4ImgReszID, prHwStatus);

	return S_IMGRESZ_HAL_OK;
}


/*/ Image Resizer HAL Set HW status.*/
/*/ \return If return value < 0, it's failed. Please reference hal_img_resizer_errcode.h.*/
s32 i4ImgResz_HAL_Set_HW_Status(
	u32 u4ImgReszID,                              /*/< [IN] Image Resizer hardware ID*/

	IMGRESZ_HAL_HW_STATUS_T * prHwStatus              /*/< [IN] Hardware status*/
)
{
	if (u4ImgReszID >= HW_IMGRESZ_NUM) {
		return E_IMGRESZ_HAL_FAIL;
	}

	if (prHwStatus == NULL) {
		return E_IMGRESZ_HAL_FAIL;
	}

	i4HwImgReszSetHwStatus(u4ImgReszID, prHwStatus);
	_rImgReszHalInfo[u4ImgReszID].fgResume = TRUE;

	return S_IMGRESZ_HAL_OK;
}


/*/ Image Resizer HAL register notify callback function.*/
/*/ \return If return value < 0, it's failed. Please reference hal_img_resizer_errcode.h.*/
s32 i4ImgResz_HAL_Reg_Notify_Callback(
	u32 u4ImgReszID,                              /*/< [IN] Image Resizer hardware ID*/

	IMGRESZ_HAL_NOTIFY_CB_REG_T *prNofifyCallback    /*/< ]IN] Nofity callback function*/
)
{
	_rImgReszHalInfo[u4ImgReszID].rNofifyCallback = *prNofifyCallback;

	return S_IMGRESZ_HAL_OK;
}

#if 0
/*/ Image Resizer HAL unregister notify callback function.*/
/*/ \return If return value < 0, it's failed. Please reference hal_img_resizer_errcode.h.*/
s32 i4ImgResz_HAL_Unreg_Notify_Callback(
	u32 u4ImgReszID,                              /*/< [IN] Image Resizer hardware ID*/

	IMGRESZ_HAL_NOTIFY_CB_REG_T *prNofifyCallback    /*/< ]IN] Nofity callback function*/
)
{
	_rImgReszHalInfo[u4ImgReszID].rNofifyCallback.pvCallBackFunc = NULL;
	_rImgReszHalInfo[u4ImgReszID].rNofifyCallback.pvPrivData = NULL;

	return S_IMGRESZ_HAL_OK;
}
#endif



