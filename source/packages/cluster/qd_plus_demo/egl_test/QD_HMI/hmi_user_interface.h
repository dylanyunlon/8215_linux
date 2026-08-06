
/*****************************************************************************

File Name        :  hmi_user_interface.h
Organization    :  Zhuli Electronics Co.Ltd in Shanghai (www.shzldz.com)
******************************************************************************/

#ifndef _HMI_USER_INTERFACE_H
#define _HMI_USER_INTERFACE_H	

#define HMI_MAIN_PAGE_ACTIVE
//#define HMI_VIDEO_PAGE_ACTIVE
//#define HMI_ANIMAT_PAGE_ACTIVE
//#define HMI_CAR_PAGE_ACTIVE

#if 0
#define		HMI_NORMAL_BUTTON   			0    
#define		HMI_PRESS_BUTTON             	1       
#define		HMI_DISABLE_BUTTON  			2 
#define		HMI_HIGHTLIGHT_BUTTON  			3 
#define		HMI_HIGHTLIGHT_PRESS_BUTTON		4 
#define		HMI_GET_FOCUS_BUTTON			5
#define		HMI_KILL_FOCUS_BUTTON			6
#endif

typedef enum
{
	#if 0
	HMI_NORMAL_BUTTON=0,  			   
	HMI_PRESS_BUTTON=1,             	     
	HMI_DISABLE_BUTTON=2 , 			 
	HMI_HIGHTLIGHT_BUTTON=3,  			 
	HMI_HIGHTLIGHT_PRESS_BUTTON=4,	 
	HMI_GET_FOCUS_BUTTON=5,			
	HMI_KILL_FOCUS_BUTTON=6	
	#endif
	HMI_PRESS_BUTTON,	
	HMI_LONG_PRESS_BUTTON,
	HMI_REPEAT_BUTTON,
	HMI_UP_BUTTON,
}HMI_BUTTON_EVENT;

typedef enum
{
	HMI_SHOW_MENU_WIN 		= 0,
	HMI_SHOW_PHONE_WIN 	= 1,
	HMI_SHOW_ALARM_WIN		= 2
}hmiShowWindow;
typedef enum
{
	HMI_TIMING_INIT		= 0,
	HMI_TIMING_BEGIN 		= 1,
	HMI_TIMING_END		= 2
}HMI_TIME_EVENT;

typedef enum
{
	HMI_MODE_IN		= 0,
	HMI_MODE_ING 	= 1,
	HMI_MODE_OUT	= 2
}HMI_MODE_TYPE;

typedef enum
{
	HMI_RED 				= 0,
	HMI_LIGHT_RED 			= 1,
	HMI_WHITEORBLACK		= 2
}HMI_BAT_COLOR;
typedef enum
{
    front_one = 0,
    front_two = 1,
    left_front_one = 2,
    left_front_two = 3,
    right_front_one = 4,
    right_front_two = 5,
    front_ped_one = 6,
    front_ped_two = 7,
    left_front_ped_one = 8,
    left_front_ped_two = 9,
    right_front_ped_one = 10,
    right_front_ped_two = 11,
    front_bike_one = 12,
    front_bike_two = 13,
    left_front_bike_one = 14,
    left_front_bike_two = 15,
    right_front_bike_one = 16,
    right_front_bike_two = 17,
    front_cone_one = 18,
    front_cone_two = 19,
    left_front_cone_one = 20,
    left_front_cone_two = 21,
    right_front_cone_one = 22,
    right_front_cone_two = 23,
    front_tri_one = 24,
    front_tri_two = 25,
    left_front_tri_one = 26,
    left_front_tri_two = 27,
    right_front_tri_one = 28,
    right_front_tri_two = 29,
    a_target_id_max
}ADAS_TARGET_NB;

typedef enum
{
    a_target_type_none = 0,
    a_target_type_car = 1,
    a_target_type_pedestrian = 2,
    a_target_type_bikecycle = 3,
    a_target_type_truck = 4,
    a_target_type_show = 5,
    a_target_type_cone = 6,//added by pxguo 230719
    a_target_type_tri = 7//added by pxguo 230719
    
}ADAS_TARGET_TYPE;
typedef enum
{
    a_gray_color	= 0,
    a_red_color		= 1,
    a_yellow_color	= 2,
    a_blue_color	= 3,
    a_white_color	= 4,
    a_max_color		= 5
}ADAS_Color_TYPE;

typedef struct
{
	ADAS_TARGET_TYPE nTargetType;
	ADAS_Color_TYPE nTargetColor;
	uint8_t nTargetPositionX;/*0-160*/
	float nTargetPositionY;/*0-12*/
	uint8_t target_id;
}ADAS_TARGET_INFO;

typedef enum
{	
	LaneLine_Left = 0,
	LaneLine_Right = 1,
	LaneLine_LeftLeft = 2,
	LaneLine_RightRight = 3,
	LaneLine_Self = 4,
	LaneLine_ID_MAX
}LaneLine_ID;

typedef enum
{
	LaneLine_No_Display = 0,
	LaneLine_Solid_Lane = 1,
	LaneLine_Dash_Lane = 2 ,
	LaneLine_shape_nb
}ADAS_LINE_SHAPE;
typedef enum
{
	Line_gray,
	Line_white,
	Line_red,
	Line_green,
	Line_blue,
	LINE_COLOR_MAX
}ADAS_LINE_COLOR;

typedef struct
{
	ADAS_LINE_SHAPE	line_shape;
	ADAS_LINE_COLOR line_color;
	float			line_pos;
}ADAS_LINE_INFO;

typedef struct
{
	ADAS_LINE_INFO line[LaneLine_ID_MAX];
	float radius;
}ADAS_LINE_TYPE;

typedef struct {
	ADAS_TARGET_INFO nTargetInfo[a_target_id_max];
	ADAS_LINE_INFO line[LaneLine_ID_MAX];
	float radius;
}Adas_Type;

extern void hmi_gfx_cold_user(void);
extern void hmi_gfx_warm_user(void);
extern void hmi_call_back_button(HMI_OBJECT_ID_STR element_id,HMI_BUTTON_EVENT button_event);
extern void hmi_user_process(HMI_TIME dt,TOUCH_BUTTON_STR *pbutton);
extern void hmi_user_process_init(void);
extern void Hmi_KeyEventProcess(UINT8 KeyEvent);
extern void Hmi_CalcFPS(float dt);
#ifdef HMI_ANIMAT_PAGE_ACTIVE
void Hmi_PowerOnAnimatStart(void);
void Hmi_PowerOnAnimatBreak(void);
#endif
#if (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642)||\
	defined(HMI_GRAPHIC_OPENGLES)||\
	defined(HMI_GRAPHIC_RGL))
void hmi_status_action_call_func(HMI_OBJECT_ID_STR call_id);
#endif

void Hmi_Speed(UINT16 speed);
void hmiSetTemp(SINT16 value); 
void hmiSetOdo(UINT32 value);
void hmiSetBatteryPercentage(float value,UINT8 timeOutValue);
void hmiSetTrip(UINT32 value);
void hmiSetTripHour(UINT32 hour);
void hmiSetTripMin(UINT8 min);
void hmiSetTripAvgElecricity(SINT32 value);
void hmiSetTripAvgSpeed(UINT16 value);
void hmiSetThisTrip(UINT32 value);
void hmiSetThisTripHour(UINT32 hour);
void hmiSetThisTripMin(UINT8 min);
void hmiSetThisTripAvgElecricity(SINT32 value);
void hmiSetThisTripAvgSpeed(UINT16 value);
void hmiSetTripInsElecricity(SINT32 value);
void hmiSetBatteryPrecent(UINT8 value,BOOLEAN UnDisplay);
void hmiSetTotalOdo(UINT32 value);
void hmiSetTslSpeedLimit(UINT8 speed);
void hmiSetAccspeedLimit(UINT8 speed, UINT8 color);
void hmiSetPowerImageShow(INT16 power_value);
void hmiSetReadyState(BOOLEAN state);
void hmiSetChargingODO(UINT16 charge_odo);
void hmiSetMiddleMenu(void);
void hmiSetChargeParticleAnimation(HMI_TIME dt);
void hmiSetChargeProgressAnimation(UINT8 value); 
void hmiResetNavRoadToEmpty(uint8_t lineCount);

//extern void hmiSetTirePressAndTemptureState(en_warnid_t warn_id);


//void hmiSetTime(UINT32 hour,UINT8 minute,en_TimeFormat timeFormatClock,UINT8 timeAMPM);

extern void hmiSetWarnList(UINT16 warnId[], UINT8 number);
extern void hmiSwitchWarnListShow(void);
void hmiStopBootAnimation(void);
void hmiPlayBootAnimation(void);
BOOLEAN hmi_menu_is_trip(void);
BOOLEAN hmi_menu_is_this_trip(void);
BOOLEAN hmi_menu_is_totalodo(void);

void Hmi_d2_setIGNOFFTelltaleShowState(HMI_TIME dt);
HMI_TIME_EVENT hmi_charge_get_disp_st(void);
void hmISetPressWarnFlash(HMI_TIME dt,BOOLEAN leftFront, BOOLEAN leftRear, BOOLEAN rightFront, BOOLEAN rightRear);
HMI_BAT_COLOR hmiGetBatteryPercentageColor(UINT8 fill_w);
void hmiSetD2TtNum(BOOLEAN d2_tt_num);
void hmi_time_four_seconds(HMI_TIME dt);
void hmiSetMiddleMenuSlide(void);
void hmiClearOdoTime(HMI_TIME_EVENT odo_timer_st);
void hmiSetCollisionWarn(HMI_TIME dt);
void hmiSetBluePhoneTextScroll(HMI_TIME dt);
void hmiSetNaviTextScroll(HMI_TIME dt);
void hmiSetWarnShowAni(void);
void hmi_call_warn_back(void);
void hmi_set_test_page(UINT8 index);
void hmi_set_curise_speed_pop(HMI_TIME dt);
void hmiToMidDelay(HMI_TIME time);

//sen_right_warn_t hmiGetRightMenu(void);


//


//UINT8 utf8_to_unicode(UINT8 *utf8, UINT32_T *unicode);

//void hmiSetMenu(MENU_B_ENUM cur_menu);
//void hmiSetTcMenu(MENU_TC_ENUM cur_tc);

//void hmiSetTelltale(TELLTALE_INFO* ttInfo_str, UINT16 len);
void hmi_printf_time(const char* printf_string,SINT32 value1);
void hmi_printf_time_f(const char* printf_string,float value1);

#endif

