/*****************************************************************************

File Name        :  hmi_driver_cfg.h
Organization    :  Zhuli Electronics Co.Ltd in Shanghai (www.shzldz.com)
******************************************************************************/

#ifndef HMI_DRIVER_CFG_H
#define HMI_DRIVER_CFG_H

#ifndef YES
   #define YES (1)
#endif
#ifndef NO
   #define NO  (0)
#endif




#define HMI_DRIVER_HORZ_FILL_FRAME_BUFFER    	(YES)
#define HMI_DRAW_BITMAP_CLIP_SUPPORT			(YES)

#define DISPLAY_SEGMENG                    (0)
#define HMI_GET_TEXT_FUNC					YES
#define HMI_GET_INFO_FUNC					YES

#define HMI_ZERO_BUFFER						0
#define HMI_ONE_BUFFER						1
#define HMI_TWO_BUFFER						2
#define HMI_OTF16_BUFFER					3
#define HMI_OTF32_BUFFER					4
#define HMI_OTF64_BUFFER					5
#define HMI_OTF128_BUFFER					6
#define HMI_OTF256_BUFFER					7
#define HMI_OTF16_SURFACE_BUFFER			8
#define HMI_OTF32_SURFACE_BUFFER			9
#define HMI_OTF64_SURFACE_BUFFER			10
#define HMI_OTF128_SURFACE_BUFFER			11
#define HMI_OTF256_SURFACE_BUFFER			12

#define HMI_DE_DRAW_OFF				YES



/***************************/

/*custom  may be modify the marco*/

/*********image data load vram************/
/*
HMI_LOAD_RES_ALL_AT_USED--load image data from flash to buffer when draw the image.
HMI_LOAD_RES_ALL_INIT-----load all image data from flash to buffer when init QD
HMI_LOAD_RES_ONLY_ROTATION-----load the only rotation image(the image angle not zero at QD Plus design) data from flash to buffer when init QD		
*/
	
#define HMI_LOAD_RES_ALL_INIT			1
#define HMI_LOAD_RES_ALL_AT_USED		2	
#define HMI_LOAD_RES_ONLY_ROTATION		3	
#define HMI_LOAD_RES_SEGMEN				4
#define HMI_LOAD_RES_SHEET				5
	
	
#define HMI_LOAD_SHEET_MAX_LEN			0x4000
#define HMI_LOAD_SHEET_MIN_LEN			0x80


#define HMI_LOAD_SOURCE_MODE			HMI_LOAD_RES_ALL_AT_USED	
/*cache font bitmap,only valide for RGL,s6j3200 .For VGLite,OpenVG must set YES*/
#define HMI_LOAD_FONT					NO

/*if define HMI_RENDER_FULL_SCREEN,draw full layer. or only draw dirty zone*/
//#define HMI_RENDER_FULL_SCREEN

/*if not define HMI_RENDER_FULL_SCREEN,
max dirty zone count*/
#define HMI_DIRTY_FIFO_LEN				1// 3

/*valide for VGLite*/
#define	HMI_LOAD_ONE_FRAME_IMGLIST		YES	/*YES ,use small SDRAM(display speed low).NO, use big SDRAM(display speed high)*/
/*
HMI_LOAD_RES_BUF_NO_INIT----build image data source buffer from flash to buffer when draw the image.
HMI_LOAD_RES_BUF_ALL_INI-----load all image data source buffer from flash to buffer when init QD
HMI_LOAD_RES_BUF_SEGMEN-----load the segment (many image at one BIN file) data from flash to buffer when init QD		

*/
#define HMI_LOAD_SOURCE_BUF_MODE		HMI_LOAD_RES_BUF_NO_INIT//HMI_LOAD_RES_BUF_NO_INIT//HMI_LOAD_RES_BUF_ALL_INIT//HMI_LOAD_RES_BUF_NO_INIT

/*if HMI_YES,after draw one image,GPU work immediately*/
#define HMI_GREEDY_MODE					NO//YES
/*if define HMI_RENDER_FULL_SCREEN,draw full layer. or only draw dirty zone*/
//#define HMI_RENDER_FULL_SCREEN

/*valide for vglite,OpenVG.
Either HMI_MEMORY_LONG_TIME_RELEASE or HMI_NOT_USED_THIS_TIME_RELEASE
must YES
1:(HMI_MEMORY_LONG_TIME_RELEASE YES,HMI_NOT_USED_THIS_TIME_RELEASE	 NO):when build image buffer is 
not failed,release long time no used buffer.
2:(HMI_MEMORY_LONG_TIME_RELEASE NO,HMI_NOT_USED_THIS_TIME_RELEASE	YES):when build image buffer is 
not failed,release all buffer which not draw at this time.
*/
#define HMI_MEMORY_LONG_TIME_RELEASE		YES
#define HMI_NOT_USED_THIS_TIME_RELEASE		NO 

/*HMI_MEMORY_LONG_TIME_RELEASE == YES,release HMI_LONG_TIME_RELEASE_CNT the oldest buffer*/
#define HMI_LONG_TIME_RELEASE_MAX_CNT		4 

/*after malloc video memory,if remain size of video memory < HMI_VGLITE_VIDEO_MEMORY_REMAIN,
release "HMI_LONG_TIME_RELEASE_MAX_CNT" buffer
*/
#define HMI_VGLITE_VIDEO_MEMORY_REMAIN		(512 * 1024 * 1024)//(6* 1024 * 1024)

/*valide only for vglite .if malloc source buffer failed,try count*/
#define	HMI_MAX_MALLOC_CNT					250

/*valide only for vglite .decode RLC image,first load to memory*/
#define HMI_DECODE_RLC_LOAD_MEMORY			NO

/*offset position BIN at flash 
1:RGL ,the base address is 0x10000000
2:s6j3200 ,the begin base address is 0x40000000
3:vglite ,the begin base address is 0x0x30002400
3:OpenVG ,the begin base address is 0x0x00000000

*/                                 
#define HMI_FLASH_OFFSET				(0)//cpu adr + flash offset + hmi offset  0x00340000



/*VGLite,OpenVG source buffer len*/
#if 0
#define	VGLITE_BUFFER_LEN				(HMI_DXY_BITMAPS_NUMBER + HMI_SXY_BITMAPS_NUMBER + \
														HMI_DXY_SCROLLBAR_NUMBER + HMI_SXY_SCROLLBAR_NUMBER + 
														HMI_DXY_IMAGELIST_NUMBER + HMI_SXY_IMAGELIST_NUMBER + 
														HMI_DXY_BUTTON_NUMBER + HMI_SXY_BUTTON_NUMBER + 1u/*not empty*/)
#else
#if	(HMI_LOAD_ONE_FRAME_IMGLIST	==	YES)
	#define	VGLITE_BUFFER_LEN				(200)
#else
	#define	VGLITE_BUFFER_LEN				(300)
#endif

#endif
#define	ADD_VGLITE_BUFFER_LEN			(100)

/*only AMT630H,Buffer for read flash*/
#define		HMI_READ_BUF_MIN_LEN	(1024 * 1024 * 2)


/******support touch panel********/
//#define HMI_TOUCH_PANEL
#define HMI_TOUCH_EXCLUDE_TEXT  NO  //?????????????

/*long button press time*/
#define  HMI_LONG_KEY_TIMER					1/*2 second*/
/*repeat button press time*/
#define  HMI_REPEAT_KEY_FIRST_TIMER			0.5f/*0.5 second */
/*first button press time for noise*/
#define  HMI_PRESS_FIRST_TIMER				0.1f/*0.1 second 0.06*/
/*fade parameter for slide*/
#define HMI_DRAG_SPEED_FADE					(0.9f)

/*MAX action,group count at one time*/
#if ( (defined(HMI_MCU_TW36))||(defined(HMI_MCU_TW25)))
#define HMI_MAX_RUN_ACTION_CNT					50/*(50)*/
#else
#define HMI_MAX_RUN_ACTION_CNT					50
#endif

/*support farsi language*/
#define HMI_FARSI_ENABLE				NO
#define HMI_FARSI_NEED_REVERSE			NO
/*support thai language*/
#define HMI_THAI_ENABLE					YES
/*valide for RGL*/
#define HMI_RGL_DMA_ENABLE				NO

#define	HMI_FARSI_CLIP_LEFT				NO	

/*****always not modify****/
/*load JPG to SDRAM.valide for  RGL,s6j3200.always HMI_NO,we always display LOGO JPG from flash*/
#define HMI_LOAD_JPG_IMAGE				NO
/*create window when init QD.always HMI_NO*/
#define HMI_CREATE_WIN_IN_INIT			NO
/*valide for RGL,always no*/
#define HMI_RENDER_ENABLE_FIFO			NO
/*valide for s6j3200*/
#define HMI_BUFFER_NUMBER				HMI_TWO_BUFFER
/*into endless loop when serious error*/
#define HMI_ENDLESS_LOOP				YES
#define HMI_QD_CREATE_WINDOW				NO

/*clear dirty zone*/
#define HMI_CLEAR_DIRTY_ENABLE 			YES

/*valide for RGL.init texture */
//#define HMI_INIT_TEXTURE_MEM_FIRST	

/*QD memory heap*/
#define HMI_USE_QD_HEAP					YES

#if(HMI_USE_QD_HEAP == NO)
#define	HMI_QD_PUBLIC_MIN_BLOCK_SIZE	256	/*QD memory block size*/
#define	HMI_QD_PUBLIC_BUFFER_CNT		64	/*QD memory block count*/
#define HMI_QD_MAX_HEAP_SIZE			(1024 * 1024 * 10)
#else
#define	HMI_QD_PUBLIC_MIN_BLOCK_SIZE	256	/*QD memory block size*/
#define	HMI_QD_PUBLIC_BUFFER_CNT		128	/*QD memory block count*/
#define HMI_QD_MAX_HEAP_SIZE			(1024 * 1024 * 160)

#endif


#define HMI_MAX_USE_BIG_IMG_CNT			(40)
#define HMI_REMAIN_USE_BIG_IMG_CNT		(8)

/*NO used*/
#define HMI_LOAD_RES_CREATE_TEXTURE		NO
#define HMI_LOAD_MULTI_PROCESS			NO

/*for jpeg*/
#define HMI_COVER_MODE					NO
#define HMI_RGB_565_LOW 				NO
/****************************/

/*cypress TVII*/
/**< Sets the color dither mode. The related parameter can be the following:
        - CYGFX_TRUE    Enable color dithering
        - CYGFX_FALSE   Disable color dithering (default) */

#define HMI_CTX_ATTR_DITHER_COLOR	CYGFX_TRUE

/**< Sets the filter mode. The related parameter can be the following:
        - ::CYGFX_GEN_FILTER_NEAREST
        - ::CYGFX_GEN_FILTER_BILINEAR (default)
        - ::CYGFX_GEN_FILTER_ANISOTROPIC */
#define HMI_CTX_ATTR_FILTER		CYGFX_GEN_FILTER_BILINEAR

/* for otf surface window,surface always malloc VRAM(video ram)*/
#define HMI_OTF_SURAFCE_ALWAYS_IN_VRAM		YES
#endif


