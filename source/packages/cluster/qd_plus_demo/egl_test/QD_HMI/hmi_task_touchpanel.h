/*****************************************************************************

File Name        :  hmi_task_toouchpanel.h
Organization    :  Zhuli Electronics Co.Ltd in Shanghai (www.shzldz.com)
******************************************************************************/

#ifndef _HMI_TASK_TOUCHPANEL_H
#define  _HMI_TASK_TOUCHPANEL_H

#define HMI_SUPPORT_CALIBRATION				HMI_NO
#define HMI_NO_TOUCH_KEY					HMI_ALL_OBJECT
#define HMI_SCREEN_BCK						1
#define HMI_DATALIST_LEN					5

#define	HMI_TP_X_SCALE_DEF					1
#define	HMI_TP_Y_SCALE_DEF					1
#define	HMI_TP_X_OFFSET_DEF					0
#define	HMI_TP_Y_OFFSET_DEF					0

#define	HMI_MINXSCALE						1
#define	HMI_MAXXSCALE						1
#define	HMI_MINYSCALE						1
#define	HMI_MAXYSCALE						1
#define	HMI_MINXOFFSET						1
#define	HMI_MINYOFFSET						1
#define	HMI_MAXXOFFSET						1
#define	HMI_MAXYOFFSET						1


#define	HMI_DISP_BUTTON_UP_PRESS			0x10
#define	HMI_DISP_BUTTON_UP_UNPRESS			0x11
#define	HMI_DISP_BUTTON_DN_PRESS			0x20
#define	HMI_DISP_BUTTON_DN_UNPRESS			0x21
#define	HMI_DISP_BUTTON_LEFT_PRESS			0x30
#define	HMI_DISP_BUTTON_LEFT_UNPRESS		0x31
#define	HMI_DISP_BUTTON_RIGHT_PRESS			0x40
#define	HMI_DISP_BUTTON_RIGHT_UNPRESS		0x41
#define	HMI_DISP_BUTTON_OK_PRESS			0x50
#define	HMI_DISP_BUTTON_OK_UNPRESS			0x51

#define	HMI_DISP_BUTTON_LEFTUP_PRESS		0x60
#define	HMI_DISP_BUTTON_LEFTUP_UNPRESS		0x61
#define	HMI_DISP_BUTTON_LEFTDN_PRESS		0x70
#define	HMI_DISP_BUTTON_LEFTDN_UNPRESS		0x71
#define	HMI_DISP_BUTTON_RIGHTUP_PRESS		0x80
#define	HMI_DISP_BUTTON_RIGHTUP_UNPRESS		0x81
#define	HMI_DISP_BUTTON_RIGHTDN_PRESS		0x90
#define	HMI_DISP_BUTTON_RIGHTDN_UNPRESS		0x91

#define  HMI_SHORT_POP_KEY_MSG  			0
#define	 HMI_LONG_KEY_MSG					1
#define  HMI_PRESS_KEY_MSG					2
#define  HMI_DRAG_DISTANCE					3  

#define	HMI_TP_INVALIDE_VALUE				(-1)

#define  HMI_TOUCH_MSG_LEN					200

#define  HMI_TOUCH_MAX_XY					0xffff
//#define HMI_IDLEKEY_OSD						(0x07)
#define	HMI_ROTATION_INVALIDE_ANGLE			(-7200.0f)

#define	HMI_MULTI_TP_MAX_DEVICE				10//5

#define HMI_NOT_FOUND						(~0x0)
#define HMI_MODE_TIMER3_EVENT				(0x20)
#define HMI_MODE_TIMER4_EVENT				(0x40)
#define HMI_CURSOR_MISS_TIME				(1000)
#define HMI_CURSOR_BUF_LEN     				(48)

#define HMI_NEW_XY_FLAG						(0x80)
#define HMI_TOUCH_PRESS						(0x01)
#define HMI_TOUCH_UP						(0x00)
#define HMI_TOUCH_MOVE						(0x02)
#define HMI_TOUCH_NO_ACTIVE					(0xff)

#define HMI_ONLY_DRAG_TYPE					(0x01)
#define HMI_ONLY_SLIDE_TYPE					(0x02)
//#define HMI_DRAG_SLIDE_TYPE					(0x03)
#define HMI_TOUCH_ELEMENT_FLAG				200
#define HMI_SUB_DIRTY_ZONE_FLAG				201





/*-----------------------Timer----------------------------*/
#define HMI_CLEAR_TIMER()					(hmi_button_status_timer=-1.0)
#define HMI_SET_TIMER(timer)				(hmi_button_status_timer=timer)
#define HMI_TIMER_OVER()					(hmi_button_status_timer==0)
/*------------------Type definition---------------------*/

typedef enum
{
	 HMI_KEY_STATUS_UNKNOWN_OSD=0
	,HMI_KEY_STATUS_IDLE_OSD
	,HMI_KEY_STATUS_WAIT_REPEAT_OSD
	,HMI_KEY_STATUS_WAIT_LONG_OSD
	,HMI_KEY_STATUS_REPEAT_OSD		
	/*Gesture*/
	,HMI_KEY_STATUS_FIRST_PRESS_OSD
	,HMI_KEY_STATUS_TRUE_PRESS_OSD	
	,HMI_KEY_STATUS_GESTURE_OSD 
	,HMI_KEY_STATUS_DRAGKEY_OSD	
	,HMI_KEY_STATUS_SCALE_OSD
	,HMI_KEY_STATUS_SLIDE_OSD
	,HMI_KEY_STATUS_ROTATION_OSD
} HMI_SPECIALKEY_TYPE;

typedef enum
{
	/*key */
	HMI_PRESSKEY_OSD,
	HMI_SHORTKEY_OSD,
	HMI_LONGKEY_OSD,
	HMI_LONG_RELEASEKEY_OSD,
	HMI_REPEATKEY_OSD,	
	HMI_REPEAT_RELEASEKEY_OSD,
	/*Gesture*/
	HMI_DRAGKEY_OSD,	
	HMI_SCALE_OSD,
	HMI_SLIDE_OSD,
	HMI_SCALE_ROTATION_OSD,
	HMI_GESTURE_OSD,
	HMI_ERRORKEY_OSD,
	/*Count*/
	HMI_KEY_CNT_OSD,
}HMI_BTN_STATUS_TYPE;

typedef enum
{
	HMI_GET_LONG_TIMER,	          		
	HMI_GET_REPEAT_FIRST_TIMER,
	HMI_GET_REPEAT_SECOND_TIMER,
	HMI_GET_DRAG
}HMI_BUTTON_ATTRIBUTE_TYPE;


typedef struct
{
	BOOLEAN				button;
	HMI_OBJECT_ID_STR	key;
}HMI_OBJECT_TOUCH_TYPE;


typedef struct
{
	SPOINT_TP	point;
	UINT8		press;
	UINT8		device_id;
	//BOOLEAN		hmi_is_button;
}HMI_TOUCH_STATUS_TYPE;

#if 0
typedef struct
{
	UINT16	x;
	UINT16	y;
}HMI_TOUCH_COORDINATE_TYPE;
#endif

#if 0
typedef struct
{
	POINT_TP				touch_xy_list[HMI_TOUCH_MSG_LEN];	
	UINT8					key_status;
	UINT8					fifo_in;				
}HMI_TOUCH_COORDINATE_FIFO_TYPE;
#endif

typedef struct
{
	SPOINT_TP				touch_xy;
	UINT8					press;
	UINT8					device_id;					
}HMI_MULTI_TP_TYPE;


typedef struct
{
	HMI_MULTI_TP_TYPE		touch_xy_list[HMI_TOUCH_MSG_LEN];		
	UINT8					head;	
	UINT8					tail;
}HMI_TOUCH_COORDINATE_FIFO_TYPE;


typedef struct
{
	HMI_OBJECT_ID_STR 		key;
	BOOLEAN					bbutton;/*key is button*/
	HMI_BTN_STATUS_TYPE		button_type;
	SPOINT32_TP				delta_s_speed;
	SPOINT32_TP				drag_position;/*relative to element zone*/
	U08						drag_slide_type;/*HMI_ONLY_DRAG_TYPE,HMI_ONLY_SLIDE_TYPE,HMI_DRAG_SLIDE_TYPE*/
	float_32				rotation_angle;	
	float_32				scale;	
	SPOINT32_TP				rotation_center;/*relative to element zone*/
	SPOINT32_TP				scale_center;/*relative to element zone*/
}TOUCH_BUTTON_STR;

#if HMI_LONG_KEY_TIMER <= 255
	typedef UINT8  HMI_TIMER_STR;
#else
	typedef UINT16 HMI_TIMER_STR;
#endif

typedef enum
{
	NON_CURSOR,
	INVALID_CURSOR,
	NORMAL_CURSOR
}CURSOR_STATE_T;

typedef enum
{
	CURSOR_CLOSE,
	CURSOR_OPEN
}CURSOR_SWITCH_T;

typedef enum
{
	HMI_2_STATUS_BUTTON,
	HMI_3_STATUS_BUTTON,
	HMI_4_STATUS_BUTTON,
	/****NOT BUTTON TYPE ****/
	HMI_NOT_BUTTON_TYPE
}HMI_BUTTON_TYPE_DEF;


/*Multi tp */
typedef enum
{
	HMI_MULTI_TP_NO_TOUCH,	          		
	HMI_MULTI_TP_FIRST_PRESS,
	HMI_MULTI_TP_TRUE_PRESS,
	HMI_MULTI_TP_GESTURE,
	HMI_MULTI_TP_SCALE,
	HMI_MULTI_TP_DRAG,
	HMI_MULTI_TP_SLIDE,
	HMI_MULTI_TP_ROTATION,
	/*Count*/
	HMI_MULTI_GESTURE_CNT
}HMI_GESTURE_STATUS_TYPE;

typedef enum
{
	HMI_MULTI_TP_FIRST_POINTS,
	HMI_MULTI_TP_PREVIOUS_POINTS,	          		
	HMI_MULTI_TP_CURRENT_POINTS,
	HMI_MULTI_TP_CURRENT_PREVIOUS_POINTS,	
	/*Count*/
	HMI_MULTI_CLEAR_CNT
}HMI_GESTURE_POINT_CLEAR_TYPE;


typedef struct
{
	SPOINT_TP				previous_points[HMI_MULTI_TP_MAX_DEVICE];
	U08						bprevious_active_device[HMI_MULTI_TP_MAX_DEVICE];
	SPOINT_TP				cur_points[HMI_MULTI_TP_MAX_DEVICE];
	U08						bcur_active_device[HMI_MULTI_TP_MAX_DEVICE];
	SPOINT_TP				pfirst_points[HMI_MULTI_TP_MAX_DEVICE];
	HMI_GESTURE_STATUS_TYPE	tp_status;
	U08						true_press_device;
	HMI_OBJECT_ID_STR 		true_press_key;	
	S3POINT_FLOAT_TP		last_drag_distance;
	HMI_OBJECT_ID_STR		last_drag_element;
}HMI_MULTI_TP_POINT_STR;

typedef struct
{
	DWORD	hmi_device_id;
	BOOLEAN	bused;
}HMI_DEVICE_ID_STR;




typedef struct
{
	POINT32_TP 		begin_point;/*drag point*/
	POINT32_TP 		rotation_begin_point;/*rotation center point*/
	POINT32_TP 		scale_point;/*scale center point*/
	POINT32_TP 		distance;
	float_32		scale;
	float_32		angle;
	BOOLEAN			bslide;
}HMI_GESTURE_INFO_STR;


#ifdef HMI_TOUCH_PANEL

#ifdef __cplusplus
extern "C"{
#endif
void hmi_send_xy(DWORD	device_ID,UINT16 x, UINT16 y,UINT8 press);

#ifdef __cplusplus
}
#endif



void hmi_touch_panel_cold_Init(void);
void hmi_touch_panel_warm_Init(void);
void hmi_touch_panel(HMI_TIME dt,TOUCH_BUTTON_STR *P_bt_status);

#endif

#endif


