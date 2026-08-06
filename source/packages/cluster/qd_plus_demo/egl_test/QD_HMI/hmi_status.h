/*****************************************************************************

File Name        :  hmi_status.h
Organization    :  Zhuli Electronics Co.Ltd in Shanghai (www.shzldz.com)
******************************************************************************/

#ifndef _HMI_STATUS_H
#define _HMI_STATUS_H
/*status*/
#define HMI_SEND_EVENT						BIT0
#define HMI_PART_RESOURCE_LOADED			BIT1
#define HMI_TOTOAL_SCREEN_ZONE_RENDER		BIT2
#define HMI_INSUFFICIENCY_MEMORY			BIT3
#define HMI_RELEASE_ALL_MEMEORY				BIT4
#define HMI_RESOURCE_MGR_FIFO_FULL			BIT5
#define HMI_RESOURCE_MGR_NULL_POINTER		BIT6
#define HMI_FILE_OPEN						BIT7
#define HMI_FILE_SYSTEM_BUSY				BIT8
#define HMI_RELEASE_USING_MEMEORY			BIT9
#define HMI_UNLOCK_DISPLAY_BUFFER			BIT10
#define HMI_FIND_NEW_DIRTY_ZONE				BIT11
#define HMI_ACTION_GROUP_TABLE_OVERFLOW		BIT12
#define HMI_TOUCH_MSG_OVERFLOW				BIT13	
#define HMI_DYN_CONTAINER_NESTED			BIT14	
#define HMI_DYN_DRAW_ERROR					BIT15	
#define HMI_DYN_TOP_API_ERROR				BIT16	
#define HMI_NOT_ENOUGH_WINDOW				BIT17
#define HMI_NOT_ENOUGH_FONT_WINDOW			BIT18
#define HMI_NOT_ENOUGH_FONT_ADDR_WINDOW		BIT19
#define HMI_COLD_RESET						BIT20
#define HMI_RGL_IMAGE_NOT_ENOUGH_MEM		BIT21
#define HMI_RGL_IMAGE_FORMAT_NOT_SUPPORT	BIT22
#define	HMI_RGL_MEM_INIT_ERROR				BIT23
#define	HMI_RGL_DRW2D_INIT_ERROR			BIT24
#define	HMI_RGL_DRW2D_DECODE_JPG_ERROR		BIT25
#define HMI_RGL_VIDEO_ERROR					BIT26
#define HMI_RGL_FINISH_FLAG					BIT27
#define HMI_RGL_PROP_EVENT					BIT28
#define HMI_RGL_GET_BUFFER_ERROR			BIT29
#define HMI_RELEASE_MEMORY_ERROR			BIT30		
/*set status*/
#define HMI_GFX_STATUS_INIT()			(hmi_gfx_status=0)
#define HMI_GFX_SET_STATUS(status)		(hmi_gfx_status|=status)
#define HMI_GFX_GET_STATUS(status)		(hmi_gfx_status&status)
#define HMI_GFX_CLEAR_STATUS(status)	(hmi_gfx_status=hmi_gfx_status&(~status))
/*status global variable*/
extern UINT32 hmi_gfx_status;

#endif

