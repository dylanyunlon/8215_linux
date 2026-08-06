/*****************************************************************************

File Name        :  hmi_engine_cfg.h
Organization    :  Zhuli Electronics Co.Ltd in Shanghai
******************************************************************************/

#ifndef HMI_ENGINE_CFG_H
#define HMI_ENGINE_CFG_H

/*
** HMI_UPDATE_BUSY    "Block all updates to a lower priority page whilea higher priority screen is active on the display. "
** HMI_NEED_REDRAW  "Update the low priority page while a higher priority screen is active."
**HMI_NONE			"Only refreshed the changed page."
*/
#define SW_MAIN_VERSION    						0x02
#define SW_SUB_VERSION        					0x05
#define SW_PATCH_VERSION      					0x04
#define HMI_ENGINE_LOW_PRIOR_PAGE_UPDATA		HMI_NEED_REDRAW

#define DISPLAY_SEGMENT_ROM 
#define	DISPLAY_SEGMENT_RAM 
#define DISPLAY_FLASH_ROM

#if ( (defined(HMI_MCU_TW36))||(defined(HMI_MCU_TW25)))
#define HMI_MAX_RUN_ACTION_CNT					20/*(50)*/
#else
#define HMI_MAX_RUN_ACTION_CNT					50
#endif


#define HMI_NEED_NESTED_DYN_CONTAINER_SUPPORT	(1)
#define HMI_DYN_CONTAINER_NESTED_DEPTH			50
#define HMI_INVALID_COOR						(-100)

#define HMI_FONT_CODE_UNICODE					(0)
#define HMI_FONT_CODE_GB2312					(1)
#define HMI_FONT_CODE_GBK						(2)
#define HMI_FONT_CODE_BIG5						(3)
#define HMI_FONT_CODE_IBM						(4)
#define HMI_FONT_CODE_SJIS						(5)


#endif
