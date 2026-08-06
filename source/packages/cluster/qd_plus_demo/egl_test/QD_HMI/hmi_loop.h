
/*****************************************************************************

File Name        :  hmi_loop.h
Organization    :  Zhuli Electronics Co.Ltd in Shanghai (www.shzldz.com)
******************************************************************************/
#ifndef _HMI_LOOP_H
#define _HMI_LOOP_H


#define HMI_REND_MAX_TIME								100/*2 ms*/

#define HMI_REND_ONE_TEXT_RAM_TIME						70
#define HMI_REND_ONE_IMAGE_TIME							30
#define HMI_REND_ONE_IMAGE_LIST_TIME					30
#define HMI_REND_ONE_SCROLLBAR_TIME						100
#define HMI_REND_ONE_CUBE_TIME							100
#define HMI_REND_SWAP_BUFFER_TIME						HMI_REND_MAX_TIME
#define HMI_REND_FILL_TIME								10
#define HMI_REND_PAGE_ALPHA_TIME						30
#define HMI_REND_VIDEO_TIME								10

#define HMI_REND_ONE_TEXT_LOAD_RAM_TIME				50
#define HMI_REND_ONE_SMALL_IMAGE_LOAD_RAM_TIME		30
#define HMI_REND_ONE_MIDDLE_IMAGE_LOAD_RAM_TIME		60
#define HMI_REND_ONE_BIG_IMAGE_LOAD_RAM_TIME			80

//#define HMI_RENDER_ENABLE_FIFO							HMI_NO

#define HMI_SOFTKEY_LEN			2

#define HMI_RENDER_FULL_LAYER	NO
//#define HMI_CLEAR_DIRTY_ENABLE 		YES//YES   NO
//#define HMI_DIRTY_FIFO_LEN		1//10 lq
#define HMI_ALL_DIRTY_FIFO_LEN	(HMI_DIRTY_FIFO_LEN*HMI_LAYER_MAX_CNT+HMI_LAYER_MAX_CNT)
//#define HMI_ONE_BUFFER_DISP_PART

#define HMI_GFX_LOOP(dt)			hmi_gfx_update(dt);\
								hmi_gfx_render(/*dt*/);

#ifdef __cplusplus
extern "C"{
#endif
void hmi_gfx_loop(HMI_TIME dt);
void hmi_gfx_render(void);
void hmi_gfx_update(HMI_TIME dt);

#ifdef __cplusplus
}
#endif



#endif

