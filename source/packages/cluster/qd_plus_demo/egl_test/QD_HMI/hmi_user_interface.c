
/*****************************************************************************

File Name        :  hmi_user_interface.c
Organization    :  Zhuli Electronics Co.Ltd in Shanghai (www.shzldz.com)
******************************************************************************/
#include "hmi_all_struct_include.h"
#include <stdio.h>
#include "stdlib.h"
#include "math.h"


//Adas_Type s_AdasInfo;

void hmi_gfx_cold_user(void)
{
}

void hmi_gfx_warm_user(void)
{
}

void hmi_user_process_init(void)
{
	//hmi_engine_set_object_info(HMI_BACKGROUND, HMI_LOAD_IMAGE);
	//hmi_engine_set_object_info(HMI_POINTER_LEFT, HMI_LOAD_IMAGE);
	//hmi_engine_set_object_info(HMI_POINTER_RIGHT, HMI_LOAD_IMAGE);
	//hmi_engine_set_object_info(HMI_SMALLPOINTER_IMAGE, HMI_LOAD_IMAGE);
	//hmi_engine_set_object_info(HMI_SMALLPOINTERRIGHT_IMAGE, HMI_LOAD_IMAGE);
	//hmi_engine_set_object_info(HMI_WARNINGSIGN_IMAGE, HMI_LOAD_IMAGE);

	//hmi_engine_set_object_info(HMI_TRAFFICSIGNS_DEERCROSSING_IMAGE, HMI_LOAD_IMAGE);
	//hmi_engine_set_object_info(HMI_TRAFFICSIGNS_SPEED80_IMAGE, HMI_LOAD_IMAGE);
	//hmi_engine_set_object_info(HMI_TRAFFICSIGNS_TRAIN_IMAGE, HMI_LOAD_IMAGE);
	//hmi_engine_set_object_info(HMI_HANDBREAK_FULL_IMAGE, HMI_LOAD_IMAGE);
	//hmi_engine_set_object_info(HMI_AIRBAG_FULL_IMAGE, HMI_LOAD_IMAGE);
	//hmi_engine_set_object_info(HMI_BATTERY_FULL_IMAGE, HMI_LOAD_IMAGE);
	//hmi_engine_set_object_info(HMI_TIREPRESSURE_FULL_IMAGE, HMI_LOAD_IMAGE);
	//hmi_engine_set_object_info(HMI_PERFORMANCE_DISPLAY_IMAGE, HMI_LOAD_IMAGE);
	//hmi_engine_set_object_info(HMI_PERFORMANCE_CPU_IMAGE, HMI_LOAD_IMAGE);
	//hmi_engine_set_object_info(HMI_PERFORMANCE_RAM_IMAGE, HMI_LOAD_IMAGE);
	//hmi_engine_set_object_info(HMI_LIGHT_FULL_IMAGE, HMI_LOAD_IMAGE);
	//hmi_engine_set_object_info(HMI_MAIN_PAGE,HMI_ACTIVE_PAGE_BIT/*Page on*/ ); 
	//hmi_engine_set_object_info(HMI_NEW_PAGE4,HMI_ACTIVE_PAGE_BIT/*Page on*/ );

	//hmi_engine_set_object_info(HMI_WARNING_CONTAINER, HMI_WARNINGSIGN_CONTAINER);
	//hmi_engine_set_object_info(HMI_NEW_IMAGE4,HMI_LOAD_IMAGE );

}


static UINT32 hmi_adas_line_color[10] =
{
	0XFF777777,//GRAY
	0XFFFFFFFF,//WHITE
	0XFFE04638,//RED
	0XFF1CB978,//GREEN
	0XFF0C65DC//BLUE
};

void	f_set_total(void)
{
#if 0
	static INT32	tickets = 600;
	INT32	ticket_val = 600;
	HMI_CHAR_STR	total[5] = { 0,0,0,0,0 };
	INT32			val = 0;
	INT32			index = 0;
	BOOLEAN			begin = FALSE;

	tickets++;
	ticket_val = tickets;
	val = ticket_val / 1000;
	ticket_val = ticket_val % 1000;
	if (val > 0)
	{
		total[index++] = '0' + val;
		begin = TRUE;
	}
	else
	{
		if (begin == TRUE)
		{
			total[index++] = '0' + val;
		}
	}

	val = ticket_val / 100;
	ticket_val = ticket_val % 100;
	if (val > 0)
	{
		total[index++] = '0' + val;
		begin = TRUE;
	}
	else
	{
		if (begin == TRUE)
		{
			total[index++] = '0' + val;
		}
	}

	val = ticket_val / 10;
	ticket_val = ticket_val % 10;
	if (val > 0)
	{
		total[index++] = '0' + val;
		begin = TRUE;
	}
	else
	{
		if (begin == TRUE)
		{
			total[index++] = '0' + val;
		}
	}

	val = ticket_val;

	if (val > 0)
	{
		total[index++] = '0' + val;
		begin = TRUE;
	}
	else
	{
		if (begin == TRUE)
		{
			total[index++] = '0' + val;
		}
	}

	hmi_engine_edit_text(HMI_BOTTOM_TEXT, total);
#endif	
}

void set_timer(void)
{
#if 0
	static INT32	tickets = 0;
	U08				hour = 0u;
	U08				min = 0u;
	U08				second = 0u;
	HMI_CHAR_STR timer[9] = { '0','0',':','0','0',':','0','0',0 };

	tickets++;//second
	hour = tickets / 3600 / 60 % 60;
	min = tickets / 3600 % 60;
	second = tickets / 60 % 60;
	timer[0] += (hour / 10);
	timer[1] += (hour % 10);

	timer[3] += (min / 10);
	timer[4] += (min % 10);

	timer[6] += (second / 10);
	timer[7] += (second % 10);

	hmi_engine_edit_text(HMI_UPTIME_RUNTIME_TEXT, timer);
#endif
}


void	set_speed_imglist(void)
{
#if 1
	HMI_OBJECT_DATA_STR	x_speed = 0;
	INT32				speed = 0;

	hmi_engine_get_object_info(HMI_SPEED_FILL_X, &x_speed);
	speed = (INT32)x_speed;

	hmi_engine_set_object_info(HMI_SPEED_MOTION_100_IMAGE_LIST, speed / 100);

	speed = speed % 100;
	hmi_engine_set_object_info(HMI_SPEED_MOTION_10_IMAGE_LIST, speed / 10);

	speed = speed % 10;
	hmi_engine_set_object_info(HMI_SPEED_MOTION_1_IMAGE_LIST, speed);
#endif
}

void set_left_cluster_speed(void)
{
#if 1
	INT32	xy = 0;
	HMI_OBJECT_DATA_STR object_data = 0;
	hmi_engine_get_object_info(HMI_DXY_FILL_X, &object_data);
	xy = (INT32)object_data;

	object_data = xy % 10;
	hmi_engine_set_object_info(HMI_SPEED_ZERO_IMAGE_LIST, object_data);

	object_data = xy / 10;
	hmi_engine_set_object_info(HMI_SPEED_TEN_IMAGE_LIST, object_data);

	object_data = xy / 100;
	hmi_engine_set_object_info(HMI_SPEED_HUNDRED_IMAGE_LIST, object_data);

	object_data = xy / 30;
	hmi_engine_set_object_info(HMI_RPM_IMAGE_LIST, object_data);
#endif
}

#define LANE_LINE_MAX_DIS	1000//400
#define LANE_LINE_MIN_DIS	400//120
#define LANE_LINE_STEP		3150.0f/50.0f


static float_32 p3_test = 100.0f;
void hmi_update_line(float line_radius)
{
	float_32 active_radius = 0.0f;//line_radius /50.0;
	float_32 p3_value = 0.0f;
	SINT32   value_u32 = 0;
	float_32 step = LANE_LINE_STEP;

	if (line_radius > 3150.0f)
	{
		line_radius = 3150.0f;
	}
	if (line_radius < -3150.0f)
	{
		line_radius = -3150.0f;
	}
	active_radius = line_radius / 50.0f;
	if (active_radius != 0)
	{
		p3_value = LANE_LINE_MIN_DIS + ((LANE_LINE_MAX_DIS - LANE_LINE_MIN_DIS) / step * (fabs(active_radius)));
		if (active_radius < 0)
		{
			p3_value = -p3_value;
		}
		value_u32 = HMI_F32_TO_U32(p3_value);
		//hmi_engine_set_object_info(HMI_CUSTOM_LANE_LINE_LIB330_P3,value_u32);
	}
	else
	{
		//hmi_engine_set_object_info(HMI_CUSTOM_LANE_LINE_LIB330_P3,HMI_F32_TO_U32(0) );
	}

}

#define HMI_CAR_LINE_DOT_LEN		60//40
#define HMI_LEFT_LEFT_LINE_MAX_DISTANCE_PIXEL 140//200//220
#define HMI_LEFT_LINE_MAX_DISTANCE_PIXEL 90
#define HMI_ROAD_MAX_WIDTH				4//6.5//6.4
#define HMI_DRIVER_DISTANCE_PER_PIXEL	0.3
#if 0
void hmiSetLineAnimation(HMI_TIME dt)
{
	UINT16	cur_speed = 120;//getClusterInfo().nKMhx100;
	UINT32	offset_step = 0;
	UINT16	offset_max_len = 0;
	UINT32  pixel_distance = 0;
	static UINT32	offset_line = 0;
	static HMI_TIME 	tick = 0;

	tick += dt;
	if (cur_speed > 150)
	{
		cur_speed = 150;
	}
	/*set line move by speed*/
	hmi_engine_get_object_info(HMI_LEFT_LINE_FILL_X, &offset_step);
	offset_max_len += offset_step;
	hmi_engine_get_object_info(HMI_LEFT_LINE_FILL_Y, &offset_step);
	offset_max_len += offset_step;

	//if(offset_line < offset_max_len)
	{
		pixel_distance = tick * cur_speed;
		offset_step = (UINT32)pixel_distance / HMI_DRIVER_DISTANCE_PER_PIXEL;//offset_step;
		if (offset_step >= 1)
		{
			offset_line += offset_step;
			tick = 0;
		}
	}
#if 0
	else
	{
		offset_line = 0;
		tick = 0;
	}
#endif
	if (offset_line > 0xffffff00)
	{
		offset_line = 0;
	}

	hmi_engine_set_object_info(HMI_LEFT_LINE_FILL_Z, offset_line);
	hmi_engine_set_object_info(HMI_RIGHT_LINE_FILL_Z, offset_line);
	hmi_engine_set_object_info(HMI_LEFT_LEFT_LINE_FILL_Z, offset_line);
	hmi_engine_set_object_info(HMI_RIGHT_RIGHT_LINE_FILL_Z, offset_line);

}
#endif
#define HMI_LEFT_LANE_CHANGE_MIN (-50)
#define HMI_LEFT_LANE_CHANGE_MAX 140
#define HMI_LEFT_LANE_CHANGE_RANGE 190

#define HMI_RIGHT_LANE_CHANGE_MIN (-80)
#define HMI_RIGHT_LANE_CHANGE_MAX 110
#define HMI_RIGHT_LANE_CHANGE_RANGE 190
/*孙 - > 11.7*/
#if 0
void HmiSetAdasLine(ADAS_LINE_INFO* padas_line_info, float line_radius)
{
	UINT8	i = 0;
	UINT8	index = 0;
	float 	line_distance = 0.0f;
	UINT32	line_width = 0;
	UINT32	line_width2 = 0;
	BOOLEAN  left_line_disp_flag = TRUE;
	BOOLEAN  right_line_disp_flag = TRUE;
	BOOLEAN  left_left_line_disp_flag = TRUE;
	BOOLEAN  right_right_line_disp_flag = TRUE;
	SINT32	lane_change_left_x = 0;
	SINT32	lane_change_right_x = 0;

	static SINT32	last_lane_change_left_x = 2;
	static SINT32	last_lane_change_right_x = 2;

	hmi_update_line(line_radius);
	if (padas_line_info != NULL)
	{
		for (i = 0; i < LaneLine_Self; i++)
		{
			index = padas_line_info[i].line_color;
			if (index < LINE_COLOR_MAX)
			{
				hmi_engine_set_object_info(hmi_adas_line_id_color[i], hmi_adas_line_color[index]);
			}
			else
			{
				hmi_engine_set_object_info(hmi_adas_line_id_color[i], 0);
				if (i == 0)
				{
					left_line_disp_flag = FALSE;
				}
				if (i == 1)
				{
					right_line_disp_flag = FALSE;
				}
				if (i == 2)
				{
					left_left_line_disp_flag = FALSE;
				}
				if (i == 3)
				{
					right_right_line_disp_flag = FALSE;
				}

			}
			if (padas_line_info[i].line_shape == LaneLine_No_Display)
			{
				hmi_engine_set_object_info(hmi_adas_line_id_color[i], 0);
				if (i == 0)
				{
					left_line_disp_flag = FALSE;
				}
				if (i == 1)
				{
					right_line_disp_flag = FALSE;
				}
				if (i == 2)
				{
					left_left_line_disp_flag = FALSE;
				}
				if (i == 3)
				{
					right_right_line_disp_flag = FALSE;
				}
			}
			else if (padas_line_info[i].line_shape == LaneLine_Solid_Lane)
			{
				hmi_engine_set_object_info(hmi_adas_line_id_x[i], 0);
			}
			else if (padas_line_info[i].line_shape == LaneLine_Dash_Lane)
			{
				hmi_engine_set_object_info(hmi_adas_line_id_x[i], HMI_CAR_LINE_DOT_LEN);
				hmi_engine_set_object_info(hmi_adas_line_id_y[i], HMI_CAR_LINE_DOT_LEN);
			}
			else
			{

			}

			line_distance = padas_line_info[i].line_pos;
			if ((i == 0) || (i == 1))/*left,right*/
			{
				if ((line_distance > 6.21f) || (line_distance < 0.0f))
				{
					hmi_engine_set_object_info(hmi_adas_line_id_color[i], 0);
					if (i == 0)
					{
						left_line_disp_flag = FALSE;
					}
					if (i == 1)
					{
						right_line_disp_flag = FALSE;
					}
				}

				line_width = HMI_LEFT_LINE_MAX_DISTANCE_PIXEL / HMI_ROAD_MAX_WIDTH * line_distance;
				line_width2 = line_width + HMI_LEFT_LINE_MAX_DISTANCE_PIXEL / HMI_ROAD_MAX_WIDTH * 3.75;
				if (line_width2 > HMI_LEFT_LEFT_LINE_MAX_DISTANCE_PIXEL)
				{
					line_width2 = HMI_LEFT_LEFT_LINE_MAX_DISTANCE_PIXEL;
				}
				if (i == 0)
				{
					hmi_engine_set_object_info(HMI_LEFT_LINE_FILL_W, line_width);
					hmi_engine_set_object_info(HMI_LEFT_LEFT_LINE_FILL_W, line_width2);
				}
				if (i == 1)
				{
					hmi_engine_set_object_info(HMI_RIGHT_LINE_FILL_W, line_width);
					hmi_engine_set_object_info(HMI_RIGHT_RIGHT_LINE_FILL_W, line_width2);
				}



			}
		}
		if (left_line_disp_flag == FALSE)
		{
			hmi_engine_set_object_info(hmi_adas_line_id_color[2], 0);
			left_left_line_disp_flag = FALSE;

		}
		if (right_line_disp_flag == FALSE)
		{
			hmi_engine_set_object_info(hmi_adas_line_id_color[3], 0);
			right_right_line_disp_flag = FALSE;
		}

#if 0
		/*set road color*/
		if ((padas_line_info[LaneLine_Self].line_color == Line_blue) &&
			(left_line_disp_flag == TRUE) && (right_line_disp_flag == TRUE))
		{
			hmi_engine_set_object_info(HMI_DYN_ROAD_TEXTURE, HMI_ROAD_TEXTURE_BLUE);
		}
		else
		{
			hmi_engine_set_object_info(HMI_DYN_ROAD_TEXTURE, HMI_DYN_CONTAINER_IS_NULL);
		}
#endif

	}
}
#endif
static SINT32 target_y = 170;
static BOOLEAN target_set_flag = FALSE;
#if 0
void hmi_adas_set_target_y_init(void)
{
	target_y = 170;
	hmi_engine_set_object_info(HMI_ADAS_FRONT_1_Y, target_y);
}

void hmi_adas_set_target_start(BOOLEAN flag)
{
	target_set_flag = flag;
}
void hmi_adas_set_target_y(void)
{
	if (target_set_flag == TRUE)
	{
		if (target_y > 50)
		{
			target_y--;
		}
		hmi_engine_set_object_info(HMI_ADAS_FRONT_1_Y, target_y);
	}
}
#endif
#define	HMI_NUMBER_SCALE_TIME	0.31f
//uint8_t angle_flag_task = 0;
extern  uint32_t tick_1;
//uint8_t tick_1_flag = 0;
//uint32_t imge_list_flag = 0;
//uint8_t  scal_val = 0;
uint16_t str1[] = { 'a','b','c',0x9a7b, 0x5229,'\0' };
INT32			pcustom_data[124];

void hmi_user_process(HMI_TIME dt, TOUCH_BUTTON_STR* pbutton)
{
	//set_timer();
#if 0
	hmi_engine_set_object_info(HMI_LEFTPOINTER_CUSTOM_EVENT, HMI_SEND_EVENT_ON);
	hmi_engine_set_object_info(HMI_RIGHTPOINTER_CUSTOM_EVENT, HMI_SEND_EVENT_ON);
	hmi_engine_set_object_info(HMI_NAVIGATION_CUSTOM_EVENT, HMI_SEND_EVENT_ON);
	hmi_engine_set_object_info(HMI_TEXT_CUSTOM_EVENT, HMI_SEND_EVENT_ON);
	hmi_engine_set_object_info(HMI_INDOCATORRIGHT_CUSTOM_EVENT, HMI_SEND_EVENT_ON);
#endif
	// hmi_engine_set_object_info(HMI_NEW_PAGE3,HMI_ACTIVE_PAGE_BIT/*Page on*/ );
	 //hmi_engine_set_object_info(HMI_NEW_CUSTOM_EVENT9, HMI_SEND_EVENT_ON);

	 //hmi_engine_set_object_info(HMI_CLASSIC_PAGE,HMI_ACTIVE_PAGE_BIT/*Page on*/ );
	 //hmi_engine_set_object_info(HMI_MOTION_PAGE,HMI_ACTIVE_PAGE_BIT/*Page on*/ );
	 //hmi_engine_set_object_info(HMI_NEW_CUSTOM_EVENT3, HMI_SEND_EVENT_ON);
	 //hmi_engine_set_object_info(HMI_NEW_PAGE3,HMI_ACTIVE_PAGE_BIT/*Page on*/ );
#if 0
	static	float_32  power_needle_angel = 1.0f;
	hmi_engine_set_object_info(HMI_LEFT_FOLLOW_MOTION_LIB_P2, HMI_F32_TO_U32(power_needle_angel));
	if (power_needle_angel < 230.0f)
	{
		power_needle_angel += 1.0f;
	}
	else
	{
		power_needle_angel = 1.0f;
	}
#endif

	//float_32 temp_float = 20.0;

	//sprintf(str1,"%.1f",temp_float);
	//hmi_engine_edit_text(HMI_NEW_TEXT5, str1);

#if 0
	static UINT8 flag = 0;
	if (flag == 0)
	{
		hmi_engine_set_object_info(HMI_DYN_DISTORT_CONTAINER, HMI_DISTORT_CONTAINER);
		flag = 1;
	}
	else
	{
		hmi_engine_set_object_info(HMI_DYN_DISTORT_CONTAINER, HMI_DISTORT_CONTAINER2);
		flag = 0;
	}
	static UINT8 rightdownx = 124;
	static UINT8 rightdowny = 41;
	static UINT8 leftdownx = 0;
	static UINT8 leftdowny = 41;
	if (rightdownx < 250)
	{
		hmi_engine_set_object_info(HMI_NEW_CONTAINER1118_X, rightdownx);
		hmi_engine_set_object_info(HMI_NEW_CONTAINER1118_Y, rightdowny);
		rightdownx++;
		rightdowny++;
	}
	else
	{
		rightdownx = 124;
		rightdowny = 41;
	}
	if (leftdowny < 160)
	{
		//hmi_engine_set_object_info(HMI_NEW_CONTAINER1119_X, leftdownx);
		hmi_engine_set_object_info(HMI_NEW_CONTAINER1119_Y, leftdowny);
		//leftdownx++;
		leftdowny++;
	}
	else
	{
		//leftdownx = 0;
		leftdowny = 41;
	}

#endif

	//hmi_engine_set_object_info(HMI_CLASSIC_PAGE,HMI_ACTIVE_PAGE_BIT);

	//hmi_engine_set_object_info(HMI_NEW_PAGE4,HMI_ACTIVE_PAGE_BIT/*Page on*/ );
#if 1
	pcustom_data[0/*array element number*/] = 124;// element number
	pcustom_data[1/*message type*/] 		= 0;

	/*cure total number*/
	pcustom_data[2] = 2;

	/*cmd index*/
	//pcustom_data[3] = 70;
	/*data index*/
	//pcustom_data[4] = 110;

	/*cmd index*/
	pcustom_data[3] = 60;
	/*data index*/
	pcustom_data[4] = 100;

	/*cmd index*/
	pcustom_data[5] = 20;
	/*data index*/
	pcustom_data[6] = 80;
	/*cmd data*/
	pcustom_data[20]	= 6;//command number
	pcustom_data[21]	= 5;//data numbrt
	pcustom_data[22]	= 0xffffffff;//color
	pcustom_data[23]	= 6;//width
	pcustom_data[24]	= QD_PATH_MOVE_TO;
	pcustom_data[25]	= QD_PATH_LINE_TO;
	pcustom_data[26]	= QD_PATH_LINE_TO;
	pcustom_data[27]	= QD_PATH_LINE_TO;
	pcustom_data[28]	= QD_PATH_LINE_TO;
	pcustom_data[29]	= QD_PATH_END;

	/*new path*/
	pcustom_data[60]	= 3;//command number
	pcustom_data[61]	= 2/*2*/;//data numbrt
	pcustom_data[62]	= 0xff777777;//color;
	pcustom_data[63]	= 5;//width

	pcustom_data[64]	= QD_PATH_MOVE_TO;
	pcustom_data[65]	= QD_PATH_LINE_TO;
	pcustom_data[66]	= QD_PATH_END;

	/*new path*/
	pcustom_data[70]	= 4;//command number
	pcustom_data[71]	= 7;//data numbrt
	pcustom_data[72]	= 0xffffff00;//color;
	pcustom_data[73]	= 0;//width

	pcustom_data[74]	= QD_PATH_MOVE_TO;
	pcustom_data[75]	= QD_PATH_LINE_TO;
	pcustom_data[76]	= QD_PATH_LINE_TO;
	pcustom_data[77]	= QD_PATH_END;

	/*pointer data*/
	pcustom_data[80]	= 0;//pointer0 x
	pcustom_data[81]	= 0;//pointer0 y

	pcustom_data[82]	= 5;//pointer1 x
	pcustom_data[83]	= 20;//pointer1 y

	pcustom_data[84]	= -15;//pointer2 x
	pcustom_data[85]	= 30;//pointer2 y

	pcustom_data[86]	= -10;//pointer3 x
	pcustom_data[87]	= 45;//pointer3 y

	pcustom_data[88]	= 35;//pointer4x
	pcustom_data[89]	= 99;//pointer4 y

	/*new path */
	pcustom_data[100]	= 5;
	pcustom_data[101]	= 20;

	pcustom_data[102]	= 10;
	pcustom_data[103]	= 30;

	pcustom_data[104]	= 15;
	pcustom_data[105]	= 40;

	pcustom_data[106]	= 75;
	pcustom_data[107]	= 50;

	/*new path */
	pcustom_data[110]	= 4;
	pcustom_data[111]	= 18;

	pcustom_data[112]	= 2;
	pcustom_data[113]	= 34;

	pcustom_data[114]	= 15;
	pcustom_data[115]	= 33;

	pcustom_data[116]	= 20;
	pcustom_data[117]	= 25;

	pcustom_data[118]	= 23;
	pcustom_data[119]	= 13;

	pcustom_data[120]	= 14;
	pcustom_data[121]	= 10;

	pcustom_data[122]	= 9;
	pcustom_data[123]	= 15;

	hmi_engine_edit_text(HMI_SIMPLE_NAVI_LIB116, (void *)pcustom_data);
#endif


#if 1
	HMI_TIME time = 0;
	static U32 	tick = 0;
	static UINT8	scrollbar_index = 0;
	UINT16 a[] = { 0x4e8c,0x4e09,0x56db,0x4e94,0x516d,0x4e03,0x4e8c,0x4e09,0x56db,0x4e94,0x516d,0x4e03,0x4e8c,0x4e09,0x56db,0x4e94,0x516d,0x4e03,'/0' };
	float_32 start = 1.5;
	float_32 duration = 7;
	UINT32 ctrl_point = 0;
	static	BOOLEAN	first_run = TRUE;
	static  HMI_TIME	total_time = 0.0f;
	static  HMI_TIME	last_total_time = 0.0f;
	UINT8	navi_direct = 0;
	static UINT8	navi_menu = 0;
	HMI_OBJECT_DATA_STR float_int = 0;
	static float_32 angel = 0.0;
	static UINT8 alpha = 0;
	static HMI_TIME 	line_radius_tick = 0.0f;


#if 1
	line_radius_tick += dt;
	last_total_time = total_time;
	total_time += dt;


	//printf("user interface\n");

	//hmi_engine_set_object_info(HMI_CAR_3D_PAGE,HMI_ACTIVE_PAGE_BIT);
	//hmi_engine_set_object_info(HMI_CLASSIC_PAGE,HMI_ACTIVE_PAGE_BIT);

	//hmi_engine_set_object_info(HMI_RIGH_NUM_ACTION_ENTRY5S, );
	//hmi_engine_set_object_info(HMI_LEFT_BCK_FUEL_ACTION_ENTRY, );
#if 0

	hmi_engine_set_object_info(HMI_NEW_PAGE3, HMI_ACTIVE_PAGE_BIT);

#else

	/*switch page ,star action*/
	if (last_total_time == 0)// classic page
	{
		hmi_engine_set_object_info(HMI_CLASSIC_PAGE, HMI_ACTIVE_PAGE_BIT);
		hmi_engine_set_object_info(HMI_CLASSIC_PAGE_ACTION_ENTRY, HMI_ACTION_RUN);
	//hmi_engine_set_object_info(HMI_NEW_CUSTOM_EVENT4, HMI_SEND_EVENT_ON);

		hmi_engine_set_object_info(HMI_QD_TEXT_SCROLL, HMI_CLEAR_SCROLL_OFFSET);
		//s_AdasInfo.line[0].line_color	= Line_gray;
		//s_AdasInfo.line[1].line_color	= Line_gray;
		//s_AdasInfo.radius = 0.0f;
		//HmiSetAdasLine(s_AdasInfo.line, s_AdasInfo.radius);
	//hmi_engine_set_object_info(HMI_DYN_ROAD_TEXTURE, HMI_DYN_CONTAINER_IS_NULL);
		//hmi_adas_set_target_y_init();
		//hmi_adas_set_target_start(FALSE);
		//hmi_engine_set_object_info(HMI_ADAS_TARGET_CAR_INIT_ENTRY, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 20.0f) && (total_time > 20.0f))// motion page
	{
		hmi_engine_set_object_info(HMI_MOTION_PAGE, HMI_ACTIVE_PAGE_BIT);
		//hmi_engine_set_object_info(HMI_CLASSIC_PAGE, HMI_ACTIVE_PAGE_BIT);
		hmi_engine_set_object_info(HMI_LEFT_FOLLOW_MOTION_LIB_P1, 212/*poiner user set angle*/);

		float_int = (HMI_OBJECT_DATA_STR)(HMI_F32_TO_U32(-90.0f));
		hmi_engine_set_object_info(HMI_LEFT_MOTION_POINTER_IMAGE_ANGEL, float_int);
		//		hmi_engine_set_object_info(HMI_LEFT_MOTION_ACTION_ENTRY,HMI_ACTION_RUN );


				hmi_engine_set_object_info(HMI_RIGHT_FOLLOW_MOTION_LIB_P1, 3/*poiner user set angle*/);
		float_int = (HMI_OBJECT_DATA_STR)(HMI_F32_TO_U32(-90.0f));
		hmi_engine_set_object_info(HMI_RIGHT_WATER_POINTER_IMAGE_ANGEL, float_int);

		hmi_engine_set_object_info(HMI_QD_TEXT_SCROLL, HMI_CLEAR_SCROLL_OFFSET);
		//hmi_engine_set_object_info(HMI_DYN_MIDDLE_CONTAINER, HMI_MIDDLE_CONTAINER0);
		//hmi_adas_set_target_start(FALSE);
	}
	else
	{
	}



#if 1
	if ((last_total_time < 0.1f) && (total_time > 0.1f))
	{
		hmi_engine_set_object_info(HMI_POINTER_ACTION_ENTRY4, HMI_ACTION_RUN);

		//hmi_engine_set_object_info(HMI_DYN_MIDDLE_CONTAINER, HMI_MIDDLE_CONTAINER0);
	}
	else if ((last_total_time < 3.3f) && (total_time > 3.3f))
	{
		
	//hmi_engine_set_object_info(HMI_SIMPLE_NAVI_LIB116_P1, HMI_F32_TO_U32(2.0));
		//hmi_engine_set_object_info(HMI_POINTER_BCK_ACTION_ENTRY,HMI_ACTION_RUN);
		//hmi_engine_set_object_info(HMI_DYN_MIDDLE_CONTAINER, HMI_MIDDLE_CONTAINER1);
		//s_AdasInfo.radius = 2000.0f;
		//HmiSetAdasLine(s_AdasInfo.line, s_AdasInfo.radius);
	}
	//animation
	else if ((last_total_time < 5.0f) && (total_time > 5.0f))//speed,RMP animation
	{
		//hmi_engine_set_object_info(HMI_DYN_LEFT_CUBE_ACTION_ENTRY,HMI_ACTION_RUN);		
		//hmi_engine_set_object_info(HMI_DYN_RIGHT_CUBE_ACTION_ENTRY,HMI_ACTION_RUN);		
		hmi_engine_set_object_info(HMI_SCROLL_TEXT_QD_ACTION_ENTRY, HMI_ACTION_RUN);
		//s_AdasInfo.line[0].line_color	= Line_red;
		//s_AdasInfo.line[1].line_color	= Line_red;
		//s_AdasInfo.radius = 0.0f;
		//HmiSetAdasLine(s_AdasInfo.line, s_AdasInfo.radius);
	//hmi_engine_set_object_info(HMI_DYN_ROAD_TEXTURE, HMI_ROAD_TEXTURE_BLUE);
		//hmi_adas_set_target_start(TRUE);
		//hmi_engine_set_object_info(HMI_ADAS_TARGET_CAR_XY_ENTRY, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 6.7f) && (total_time > 6.7f))
	{
		hmi_engine_set_object_info(HMI_POINTER_ACTION_ENTRY4, HMI_ACTION_RUN);
		hmi_engine_set_object_info(HMI_NAVI_SCALE_ACTION_ENTRY, HMI_ACTION_RUN);
		//s_AdasInfo.line[0].line_color	= Line_gray;
		//s_AdasInfo.line[1].line_color	= Line_gray;
		//s_AdasInfo.radius = -2000.0f;
		//HmiSetAdasLine(s_AdasInfo.line, s_AdasInfo.radius);
	//hmi_engine_set_object_info(HMI_DYN_ROAD_TEXTURE, HMI_DYN_CONTAINER_IS_NULL);
	}
	else if ((last_total_time < 9.9f) && (total_time > 9.9f))
	{
		hmi_engine_set_object_info(HMI_POINTER_BCK_ACTION_ENTRY, HMI_ACTION_RUN);
		hmi_engine_set_object_info(HMI_NAVI_Y_ACTION_ENTRY, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 10.0f) && (total_time > 10.0f))//speed,RMP animation
	{
		//hmi_engine_set_object_info(HMI_DYN_LEFT_CUBE_ACTION_BCK_ENTRY,HMI_ACTION_RUN );			
		//hmi_engine_set_object_info(HMI_DYN_RIGHT_CUBE_ACTION_BCK_ENTRY,HMI_ACTION_RUN);
	}
	else if ((last_total_time < 13.0f) && (total_time > 13.0f))
	{
		hmi_engine_set_object_info(HMI_POINTER_ACTION_ENTRY4, HMI_ACTION_RUN);
		hmi_engine_set_object_info(HMI_NAVI_SCALE_ACTION_ENTRY, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 16.3f) && (total_time > 16.3f))
	{
		hmi_engine_set_object_info(HMI_POINTER_BCK_ACTION_ENTRY, HMI_ACTION_RUN);
		hmi_engine_set_object_info(HMI_NAVI_Y_ACTION_ENTRY, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 20.0f) && (total_time > 20.0f))//move navi 
	{
		hmi_engine_set_object_info(HMI_MOVE_MAP_ACTION_ENTRY, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 21.552f) && (total_time > 21.552f))
	{
		hmi_engine_set_object_info(HMI_LEFT_FOLLOW_MOTION_LIB_P1, 3/*x axis to rotation pointe*/);
		hmi_engine_set_object_info(HMI_RIGHT_FOLLOW_MOTION_LIB_P1, 5/*negative x axis to rotation pointe*/);
	}
	else if ((last_total_time < 22.0f) && (total_time > 22.0f))//water,fuel
	{
		hmi_engine_set_object_info(HMI_LEFT_FUEL_ACTION_ENTRY,HMI_ACTION_RUN );

		// Display navi tip info
		//hmi_engine_set_object_info(HMI_DYN_NAVI_DISTANCE_CONTAINER, HMI_DISTANCE_NAVI_CONTAINER);

		//              navi_direct	= tick % 3;
			  //if(navi_direct == 0)
			  //{
			  //	hmi_engine_set_object_info(HMI_DYN_NAVI_ARROW_CONTAINER,HMI_NAVI_ARROW_LEFT_CONTAINER);
			  //}
			  //else if(navi_direct == 1)
			  //{
			  //	hmi_engine_set_object_info(HMI_DYN_NAVI_ARROW_CONTAINER,HMI_NAVI_ARROW_RIGHT_CONTAINER);
			  //}
			  //else
			  //{
			  //	hmi_engine_set_object_info(HMI_DYN_NAVI_ARROW_CONTAINER,HMI_NAVI_ARROW_DIRECT_CONTAINER);
			  //}
	}
	else if ((last_total_time < 25.0f) && (total_time > 25.0f))
	{
		// off navi info tip
		hmi_engine_set_object_info(HMI_DYN_NAVI_ARROW_CONTAINER, HMI_DYN_CONTAINER_IS_NULL);
		//move navi  perspective
		hmi_engine_set_object_info(HMI_DYN_NAVI_DISTANCE_CONTAINER, HMI_DISTANCE_NAVI_CONTAINER_NULL);
		//hmi_engine_set_object_info(HMI_DYN_NAVI_CUBE_ACTION_ENTRY,HMI_ACTION_RUN );
		hmi_engine_set_object_info(HMI_LEFT_MOTION_ACTION_ENTRY, HMI_ACTION_RUN);
		hmi_engine_set_object_info(HMI_SCROLL_TEXT_QD_ACTION_ENTRY, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 26.0f) && (total_time > 26.0f))
	{
		//water,fuel
		hmi_engine_set_object_info(HMI_LEFT_BCK_FUEL_ACTION_ENTRY, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 28.20f) && (total_time > 28.2f))
	{
#if 0
		hmi_engine_set_object_info(HMI_LEFT_FOLLOW_MOTION_LIB_P1, 0/*poiner user set angle*/);

		float_int = (HMI_OBJECT_DATA_STR)(HMI_F32_TO_U32(90.0f));
		hmi_engine_set_object_info(HMI_LEFT_MOTION_POINTER_IMAGE_ANGEL, float_int);
#endif
	}
	else if ((last_total_time < 30.0f) && (total_time > 30.0f))
	{
		//hmi_engine_set_object_info(HMI_LEFT_FUEL_ACTION_ENTRY,HMI_ACTION_RUN );
		//menu navi
#if 1
	//navi_menu	= tick % 2;
		if (navi_menu == 0)
		{
			//hmi_engine_set_object_info(HMI_NAVI_MENU_PRISM_LIB_P3, HMI_NAVI_MENU__CONTAINER1);
			//hmi_engine_set_object_info(HMI_NAVI_MENU_PRISM_LIB_P1, HMI_NAVI_MENU__CONTAINER0);
			hmi_engine_set_object_info(HMI_DYN_MOTION_MENU_CONTAINER, HMI_MOTION_PRISM_MENU_CONTAINER);
			hmi_engine_set_object_info(HMI_NAVI_MENUPRISM_ACTION_ENTRY, HMI_ACTION_RUN);
		}
		else if (navi_menu == 1)
		{
			//hmi_engine_set_object_info(HMI_NAVI_MENUCIRCLE_LIB_P3, HMI_NAVI_MENU__CONTAINER1);
			//hmi_engine_set_object_info(HMI_NAVI_MENUCIRCLE_LIB_P1, HMI_NAVI_MENU__CONTAINER0);
			hmi_engine_set_object_info(HMI_DYN_MOTION_MENU_CONTAINER, HMI_MOTION_CIRCLE_MENU_CONTAINER);
			hmi_engine_set_object_info(HMI_NAVI_MENUCIRCLE_ACTION_ENTRY, HMI_ACTION_RUN);
		}
		else
		{
			//hmi_engine_set_object_info(HMI_NAVI_MENULAYEROUT_LIB_P3, HMI_NAVI_MENU__CONTAINER1);
			//hmi_engine_set_object_info(HMI_NAVI_MENULAYEROUT_LIB_P1, HMI_NAVI_MENU__CONTAINER0);
			hmi_engine_set_object_info(HMI_DYN_MOTION_MENU_CONTAINER, HMI_MOTION_LAYOUT_MENU_CONTAINER);
			hmi_engine_set_object_info(HMI_LAYOUT_ACTION_ENTRY, HMI_ACTION_RUN);
		}
		navi_menu++;
		if (navi_menu >= 2)
		{
			navi_menu = 0;
		}
#endif
	}
	else if ((last_total_time < 34.0f) && (total_time > 34.0f))
	{
		hmi_engine_set_object_info(HMI_DYN_MOTION_MENU_CONTAINER, HMI_MOTION_PRISM_MENU_CONTAINER_NULL);
		//hmi_engine_set_object_info(HMI_LEFT_BCK_FUEL_ACTION_ENTRY, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 35.0f) && (total_time > 35.0f))//move navi  not perspective
	{
	}

	//scale left number
	// 200
	if ((last_total_time < 21.0720882 - HMI_NUMBER_SCALE_TIME) && (total_time > 21.0720882 - HMI_NUMBER_SCALE_TIME))//left number scale
	{
		hmi_engine_set_object_info(HMI_LEFT_NUM_SCALE_ACTION_ENTRY200, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 21.0720882) && (total_time > 21.0720882))
	{
		hmi_engine_set_object_info(HMI_LEFT_NUM_SCALE_ACTION_ENTRY200S, HMI_ACTION_RUN);
	}	// 140
	else if ((last_total_time < 21.7441196 - HMI_NUMBER_SCALE_TIME) && (total_time > 21.7441196 - HMI_NUMBER_SCALE_TIME))//left number scale
	{
		hmi_engine_set_object_info(HMI_LEFT_NUM_SCALE_ACTION_ENTRY140, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 21.7441196) && (total_time > 21.7441196))
	{
		hmi_engine_set_object_info(HMI_LEFT_NUM_SCALE_ACTION_ENTRY140S, HMI_ACTION_RUN);
	}	// 100
	else if ((last_total_time < 22.2881451 - HMI_NUMBER_SCALE_TIME) && (total_time > 22.2881451 - HMI_NUMBER_SCALE_TIME))//left number scale
	{
		hmi_engine_set_object_info(HMI_LEFT_NUM_SCALE_ACTION_ENTRY100, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 22.2881451) && (total_time > 22.2881451))
	{
		hmi_engine_set_object_info(HMI_LEFT_NUM_SCALE_ACTION_ENTRY100S, HMI_ACTION_RUN);
	}//	60
	else if ((last_total_time < 22.6881638 - HMI_NUMBER_SCALE_TIME) && (total_time > 22.6881638 - HMI_NUMBER_SCALE_TIME))//left number scale
	{
		hmi_engine_set_object_info(HMI_LEFT_NUM_SCALE_ACTION_ENTRY60, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 22.6881638) && (total_time > 22.6881638))
	{
		hmi_engine_set_object_info(HMI_LEFT_NUM_SCALE_ACTION_ENTRY60S, HMI_ACTION_RUN);
	}// 40
	else if ((last_total_time < 23.0881824 - HMI_NUMBER_SCALE_TIME) && (total_time > 23.0881824 - HMI_NUMBER_SCALE_TIME))//left number scale
	{
		hmi_engine_set_object_info(HMI_LEFT_NUM_SCALE_ACTION_ENTRY40, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 23.0881824) && (total_time > 23.0881824))
	{
		hmi_engine_set_object_info(HMI_LEFT_NUM_SCALE_ACTION_ENTRY40S, HMI_ACTION_RUN);
	}// 20
	else if ((last_total_time < 23.6162071 - HMI_NUMBER_SCALE_TIME) && (total_time > 23.6162071 - HMI_NUMBER_SCALE_TIME))//left number scale
	{
		hmi_engine_set_object_info(HMI_LEFT_NUM_SCALE_ACTION_ENTRY20, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 23.6162071) && (total_time > 23.6162071))
	{
		hmi_engine_set_object_info(HMI_LEFT_NUM_SCALE_ACTION_ENTRY20S, HMI_ACTION_RUN);
	}// 0
	else if ((last_total_time < 24.5282497 - HMI_NUMBER_SCALE_TIME) && (total_time > 24.5282497 - HMI_NUMBER_SCALE_TIME))//left number scale
	{
		hmi_engine_set_object_info(HMI_LEFT_NUM_SCALE_ACTION_ENTRY0, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 24.5282497) && (total_time > 24.5282497))
	{
		hmi_engine_set_object_info(HMI_LEFT_NUM_SCALE_ACTION_ENTRY0S, HMI_ACTION_RUN);
	}
	else
	{
	}

	//scale right number
	// 6
	if ((last_total_time < 20.9440823 - HMI_NUMBER_SCALE_TIME) && (total_time > 20.9440823 - HMI_NUMBER_SCALE_TIME))//left number scale
	{
		hmi_engine_set_object_info(HMI_RIGH_NUM_ACTION_ENTRY6, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 20.9440823) && (total_time > 20.9440823))
	{
		hmi_engine_set_object_info(HMI_RIGH_NUM_ACTION_ENTRY6S, HMI_ACTION_RUN);
	}	// 5
	else if ((last_total_time < 21.8081226 - HMI_NUMBER_SCALE_TIME) && (total_time > 21.8081226 - HMI_NUMBER_SCALE_TIME))//left number scale
	{
		hmi_engine_set_object_info(HMI_RIGH_NUM_ACTION_ENTRY5, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 21.8081226) && (total_time > 21.8081226))
	{
		hmi_engine_set_object_info(HMI_RIGH_NUM_ACTION_ENTRY5S, HMI_ACTION_RUN);
	}	// 4
	else if ((last_total_time < 22.1281376 - HMI_NUMBER_SCALE_TIME) && (total_time > 22.1281376 - HMI_NUMBER_SCALE_TIME))//left number scale
	{
		hmi_engine_set_object_info(HMI_RIGH_NUM_ACTION_ENTRY4, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 22.1281376) && (total_time > 22.1281376))
	{
		hmi_engine_set_object_info(HMI_RIGH_NUM_ACTION_ENTRY4S, HMI_ACTION_RUN);
	}//	3
	else if ((last_total_time < 22.5601578 - HMI_NUMBER_SCALE_TIME) && (total_time > 22.5601578 - HMI_NUMBER_SCALE_TIME))//left number scale
	{
		hmi_engine_set_object_info(HMI_RIGH_NUM_ACTION_ENTRY3, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 22.5601578) && (total_time > 22.5601578))
	{
		hmi_engine_set_object_info(HMI_RIGH_NUM_ACTION_ENTRY3S, HMI_ACTION_RUN);
	}// 2
	else if ((last_total_time < 22.9761772 - HMI_NUMBER_SCALE_TIME) && (total_time > 22.9761772 - HMI_NUMBER_SCALE_TIME))//left number scale
	{
		hmi_engine_set_object_info(HMI_RIGH_NUM_ACTION_ENTRY2, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 22.9761772) && (total_time > 22.9761772))
	{
		hmi_engine_set_object_info(HMI_RIGH_NUM_ACTION_ENTRY2S, HMI_ACTION_RUN);
	}// 1
	else if ((last_total_time < 23.5202026 - HMI_NUMBER_SCALE_TIME) && (total_time > 23.5202026 - HMI_NUMBER_SCALE_TIME))//left number scale
	{
		hmi_engine_set_object_info(HMI_RIGH_NUM_ACTION_ENTRY1, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 23.5202026) && (total_time > 23.5202026))
	{
		hmi_engine_set_object_info(HMI_RIGH_NUM_ACTION_ENTRY1S, HMI_ACTION_RUN);
	}// 0
	else if ((last_total_time < 24.4642467 - HMI_NUMBER_SCALE_TIME) && (total_time > 24.4642467 - HMI_NUMBER_SCALE_TIME))//left number scale
	{
		hmi_engine_set_object_info(HMI_RIGH_NUM_ACTION_ENTRY0, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 24.4642467) && (total_time > 24.4642467))
	{
		hmi_engine_set_object_info(HMI_RIGH_NUM_ACTION_ENTRY0S, HMI_ACTION_RUN);
	}
	else
	{
	}



	//scale left number
	// 200
	if ((last_total_time < 28.9124546 - HMI_NUMBER_SCALE_TIME) && (total_time > 28.9124546 - HMI_NUMBER_SCALE_TIME))//left number scale
	{
		hmi_engine_set_object_info(HMI_LEFT_NUM_SCALE_ACTION_ENTRY200, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 28.9124546) && (total_time > 28.9124546))
	{
		hmi_engine_set_object_info(HMI_LEFT_NUM_SCALE_ACTION_ENTRY200S, HMI_ACTION_RUN);
	}	// 140
	else if ((last_total_time < 28.2404232 - HMI_NUMBER_SCALE_TIME) && (total_time > 28.2404232 - HMI_NUMBER_SCALE_TIME))//left number scale
	{
		hmi_engine_set_object_info(HMI_LEFT_NUM_SCALE_ACTION_ENTRY140, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 28.2404232) && (total_time > 28.2404232))
	{
		hmi_engine_set_object_info(HMI_LEFT_NUM_SCALE_ACTION_ENTRY140S, HMI_ACTION_RUN);
	}	// 100
	else if ((last_total_time < 27.6803970 - HMI_NUMBER_SCALE_TIME) && (total_time > 27.6803970 - HMI_NUMBER_SCALE_TIME))//left number scale
	{
		hmi_engine_set_object_info(HMI_LEFT_NUM_SCALE_ACTION_ENTRY100, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 27.6803970) && (total_time > 27.6803970))
	{
		hmi_engine_set_object_info(HMI_LEFT_NUM_SCALE_ACTION_ENTRY100S, HMI_ACTION_RUN);
	}//	60
	else if ((last_total_time < 27.2803783 - HMI_NUMBER_SCALE_TIME) && (total_time > 27.2803783 - HMI_NUMBER_SCALE_TIME))//left number scale
	{
		hmi_engine_set_object_info(HMI_LEFT_NUM_SCALE_ACTION_ENTRY60, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 27.2803783) && (total_time > 27.2803783))
	{
		hmi_engine_set_object_info(HMI_LEFT_NUM_SCALE_ACTION_ENTRY60S, HMI_ACTION_RUN);
	}// 40
	else if ((last_total_time < 26.8963604 - HMI_NUMBER_SCALE_TIME) && (total_time > 26.8963604 - HMI_NUMBER_SCALE_TIME))//left number scale
	{
		hmi_engine_set_object_info(HMI_LEFT_NUM_SCALE_ACTION_ENTRY40, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 26.8963604) && (total_time > 26.8963604))
	{
		hmi_engine_set_object_info(HMI_LEFT_NUM_SCALE_ACTION_ENTRY40S, HMI_ACTION_RUN);
	}// 20
	else if ((last_total_time < 26.3523350 - HMI_NUMBER_SCALE_TIME) && (total_time > 26.3523350 - HMI_NUMBER_SCALE_TIME))//left number scale
	{
		hmi_engine_set_object_info(HMI_LEFT_NUM_SCALE_ACTION_ENTRY20, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 26.3523350) && (total_time > 26.3523350))
	{
		hmi_engine_set_object_info(HMI_LEFT_NUM_SCALE_ACTION_ENTRY20S, HMI_ACTION_RUN);
	}// 0
	else if ((last_total_time < 25.4082909 - HMI_NUMBER_SCALE_TIME) && (total_time > 25.4082909 - HMI_NUMBER_SCALE_TIME))//left number scale
	{
		hmi_engine_set_object_info(HMI_LEFT_NUM_SCALE_ACTION_ENTRY0, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 25.4082909) && (total_time > 25.4082909))
	{
		hmi_engine_set_object_info(HMI_LEFT_NUM_SCALE_ACTION_ENTRY0S, HMI_ACTION_RUN);
	}
	else
	{
	}

	//scale right number
	// 6
	if ((last_total_time < 29.0244598 - HMI_NUMBER_SCALE_TIME) && (total_time > 29.0244598 - HMI_NUMBER_SCALE_TIME))//left number scale
	{
		hmi_engine_set_object_info(HMI_RIGH_NUM_ACTION_ENTRY6, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 29.0244598) && (total_time > 29.0244598))
	{
		hmi_engine_set_object_info(HMI_RIGH_NUM_ACTION_ENTRY6S, HMI_ACTION_RUN);
	}	// 5
	else if ((last_total_time < 28.1604195 - HMI_NUMBER_SCALE_TIME) && (total_time > 28.1604195 - HMI_NUMBER_SCALE_TIME))//left number scale
	{
		hmi_engine_set_object_info(HMI_RIGH_NUM_ACTION_ENTRY5, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 28.1604195) && (total_time > 28.1604195))
	{
		hmi_engine_set_object_info(HMI_RIGH_NUM_ACTION_ENTRY5S, HMI_ACTION_RUN);
	}	// 4,,27.8564053
	else if ((last_total_time < 27.5604053 - HMI_NUMBER_SCALE_TIME) && (total_time > 27.5604053 - HMI_NUMBER_SCALE_TIME))//left number scale
	{
		hmi_engine_set_object_info(HMI_RIGH_NUM_ACTION_ENTRY4, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 27.5604053) && (total_time > 27.5604053))
	{
		hmi_engine_set_object_info(HMI_RIGH_NUM_ACTION_ENTRY4S, HMI_ACTION_RUN);
	}//	3
	else if ((last_total_time < 27.4243851 - HMI_NUMBER_SCALE_TIME) && (total_time > 27.4243851 - HMI_NUMBER_SCALE_TIME))//left number scale
	{
		hmi_engine_set_object_info(HMI_RIGH_NUM_ACTION_ENTRY3, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 27.4243851) && (total_time > 27.4243851))
	{
		hmi_engine_set_object_info(HMI_RIGH_NUM_ACTION_ENTRY3S, HMI_ACTION_RUN);
	}// 2
	else if ((last_total_time < 27.0083656 - HMI_NUMBER_SCALE_TIME) && (total_time > 27.0083656 - HMI_NUMBER_SCALE_TIME))//left number scale
	{
		hmi_engine_set_object_info(HMI_RIGH_NUM_ACTION_ENTRY2, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 27.0083656) && (total_time > 27.0083656))
	{
		hmi_engine_set_object_info(HMI_RIGH_NUM_ACTION_ENTRY2S, HMI_ACTION_RUN);
	}// 1
	else if ((last_total_time < 26.4483395 - HMI_NUMBER_SCALE_TIME) && (total_time > 26.4483395 - HMI_NUMBER_SCALE_TIME))//left number scale
	{
		hmi_engine_set_object_info(HMI_RIGH_NUM_ACTION_ENTRY1, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 26.4483395) && (total_time > 26.4483395))
	{
		hmi_engine_set_object_info(HMI_RIGH_NUM_ACTION_ENTRY1S, HMI_ACTION_RUN);
	}// 0
	else if ((last_total_time < 25 - HMI_NUMBER_SCALE_TIME) && (total_time > 25 - HMI_NUMBER_SCALE_TIME))//left number scale
	{
		hmi_engine_set_object_info(HMI_RIGH_NUM_ACTION_ENTRY0, HMI_ACTION_RUN);
	}
	else if ((last_total_time < 25) && (total_time > 25))
	{
		hmi_engine_set_object_info(HMI_RIGH_NUM_ACTION_ENTRY0S, HMI_ACTION_RUN);
	}
	else
	{
	}
	/*adas line*/
	//hmiSetLineAnimation(dt);
#if 0
	if (line_radius_tick > 2.0f)
	{
		s_AdasInfo.radius += 500.0f;
		if (s_AdasInfo.radius > 3000.0f)
		{
			s_AdasInfo.radius = -3000.0f;
		}

		line_radius_tick = 0.0f;
	}
#endif
	//hmi_adas_set_target_y();

	if (total_time > 35.0f)
	{
		total_time = 0.0f;
	}
#endif	

	//tick++;

#if 0
#if HMI_IMAGE_PAGE_ON
	hmi_engine_set_object_info(HMI_IMAGE_PAGE, HMI_ACTIVE_PAGE_BIT);
	hmi_engine_set_object_info(HMI_NEEDLE_IMAGE_ANGEL, HMI_F32_TO_U32(angel));
#endif
#if 1//HMI_IMAGELIST_PAGE_ON
	if (tick == FALSE)
	{
		hmi_engine_set_object_info(HMI_IMAGELIST_PAGE, HMI_ACTIVE_PAGE_BIT);

		hmi_engine_set_object_info(HMI_NEW_ACTION_ENTRY4, HMI_ACTION_RUN);
		tick = TRUE;
	}
	//hmi_engine_set_object_info(HMI_NEW_IMAGE_LIST6,index );

	//hmi_engine_set_object_info(HMI_NEW_IMAGE_LIST7,index );

	//hmi_engine_set_object_info(HMI_NEW_IMAGE_LIST8,index );

	//hmi_engine_set_object_info(HMI_NEW_IMAGE_LIST8_X,x );
	//hmi_engine_set_object_info(HMI_NEW_CONTAINER5_SCALE,HMI_F32_TO_U32(scale) );
	//hmi_engine_set_object_info(HMI_DXY_IMAGE3_X,x);
	//hmi_engine_set_object_info(HMI_DXY_IMAGE3_Y,y);
#endif

#if HMI_SCROLLBAR_PAGE_ON

	hmi_engine_set_object_info(HMI_SCROLLBAR_PAGE, HMI_ACTIVE_PAGE_BIT);

	hmi_engine_set_object_info(HMI_NEW_SCROLL10, index);

	hmi_engine_set_object_info(HMI_NEW_SCROLL11, index);

	hmi_engine_set_object_info(HMI_NEW_SCROLL12, index);

	hmi_engine_set_object_info(HMI_NEW_SCROLL11_X, x);

	//hmi_engine_set_object_info(HMI_NEW_CONTAINER5_SCALE,HMI_F32_TO_U32(scale) );
#endif

//hmi_engine_set_object_info(HMI_NEW_IMAGE4_W,96*scale/16);
//hmi_engine_set_object_info(HMI_NEW_IMAGE4_H,80*scale/16);
#if HMI_TEXT_PAGE_ON
	if (tick == FALSE)
	{
		hmi_engine_set_object_info(HMI_NEW_PAGE, HMI_ACTIVE_PAGE_BIT);
		hmi_engine_set_object_info(HMI_NEW_ACTION_ENTRY19, HMI_ACTION_RUN);
		hmi_engine_set_object_info(HMI_NEW_PAGE18, HMI_ACTIVE_PAGE_BIT);
		hmi_engine_set_object_info(HMI_NEW_ACTION_ENTRY83, HMI_ACTION_RUN);
		hmi_engine_set_object_info(HMI_NEW_PAGE86, HMI_ACTIVE_PAGE_BIT);
		hmi_engine_set_object_info(HMI_NEW_ACTION_ENTRY96, HMI_ACTION_RUN);
		hmi_engine_set_object_info(HMI_NEW_PAGE54, HMI_ACTIVE_PAGE_BIT);
		hmi_engine_set_object_info(HMI_NEW_ACTION_ENTRY70, HMI_ACTION_RUN);
		hmi_engine_set_object_info(HMI_NEW_PAGE85, HMI_ACTIVE_PAGE_BIT);
		hmi_engine_set_object_info(HMI_NEW_ACTION_ENTRY111, HMI_ACTION_RUN);
		tick = = TRUE;

		hmi_engine_set_object_info(HMI_SCROLL_TEXT_SCROLL, 1);
		hmi_engine_set_object_info(HMI_LANGUAGE, HMI_CHINESE);

		hmi_engine_set_object_info(HMI_DYN_FONT_TEXT1, HMI_NEW_FONT19);
	}
	hmi_engine_set_object_info(HMI_DXY_PAGE38, HMI_ACTIVE_PAGE_BIT);
	hmi_engine_set_object_info(HMI_SXY_ACTION_ENTRY33, HMI_ACTION_RUN);
	hmi_engine_set_object_info(HMI_NEW_PAGE18, HMI_ACTIVE_PAGE_BIT);
	hmi_engine_set_object_info(HMI_NEW_ACTION_ENTRY87, HMI_ACTION_RUN);

	hmi_engine_set_object_info(HMI_DXY_PAGE38, HMI_ACTIVE_PAGE_BIT);
	hmi_engine_set_object_info(HMI_DXY_ACTION_ENTRY78, HMI_ACTION_RUN);
	hmi_engine_set_object_info(HMI_NEW_PAGE25, HMI_ACTIVE_PAGE_BIT);
	hmi_engine_edit_text(HMI_EDIT_TEXT, a);
	hmi_engine_set_object_info(HMI_DXY_PAGE3, HMI_ACTIVE_PAGE_BIT);
	hmi_engine_set_object_info(HMI_SXY_ACTION_ENTRY7, HMI_ACTION_RUN);
	hmi_engine_set_object_info(HMI_SXY_ACTION_ENTRY13, HMI_ACTION_RUN);
	hmi_engine_set_object_info(HMI_SXY_ACTION_ENTRY16, HMI_ACTION_RUN);
	hmi_engine_set_object_info(HMI_SXY_ACTION_ENTRY28, HMI_ACTION_RUN);

	hmi_engine_set_object_info(HMI_SXY_ACTION_ENTRY7, HMI_ACTION_RUN);
	hmi_engine_set_object_info(HMI_SXY_TIMER_ACTION8_START, HMI_F32_TO_U32(start));
	hmi_engine_set_object_info(HMI_SXY_TIMER_ACTION8_DUR, HMI_F32_TO_U32(duration));


	hmi_engine_set_object_info(HMI_DXY_PAGE3, HMI_ACTIVE_PAGE_BIT);
	hmi_engine_set_object_info(HMI_DXY_PAGE, HMI_REMOVE_PAGE_BIT);
	hmi_engine_set_object_info(HMI_FILL_PAGE, HMI_ACTIVE_PAGE_BIT);
	hmi_engine_set_object_info(HMI_SXY_FILL_COLOUR, 0Xff);////
	hmi_engine_set_object_info(HMI_FILL_ACTION_ENTRY, HMI_ACTION_RUN);
	hmi_engine_set_object_info(HMI_IMAGELIST_PAGE10, HMI_ACTIVE_PAGE_BIT);
	hmi_engine_set_object_info(HMI_IMAGELIST_PAGE10, HMI_ACTIVE_PAGE_BIT);
	hmi_engine_set_object_info(HMI_DXY_IMAGE_LIST11_ALPHA, 180);
	hmi_engine_set_object_info(HMI_NEW_IMAGE_LIST13_W, 96); .
		hmi_engine_set_object_info(HMI_NEW_IMAGE_LIST13_H, 96); hmi_engine_set_object_info(HMI_CONTAINER_PAGE, HMI_ACTIVE_PAGE_BIT);
	hmi_engine_set_object_info(HMI_DXY_CONTAINER16_X, 20);
	hmi_engine_set_object_info(HMI_DXY_CONTAINER16_Y, 20);
	hmi_engine_set_object_info(HMI_DXY_CONTAINER16_W, 80);
	hmi_engine_set_object_info(HMI_DXY_CONTAINER16_H, 80);
	hmi_engine_set_object_info(HMI_CONTAINER_PAGE, HMI_ACTIVE_PAGE_BIT);
	hmi_engine_set_object_info(HMI_DXY_CONTAINER31_X, 30);
	hmi_engine_set_object_info(HMI_DXY_CONTAINER31_Y, 160);
	hmi_engine_set_object_info(HMI_DXY_CONTAINER31_W, 80);
	hmi_engine_set_object_info(HMI_DXY_CONTAINER31_H, 80);
	hmi_engine_set_object_info(HMI_SXY_IMAGE_PAGE, HMI_ACTIVE_PAGE_BIT);
	hmi_engine_set_object_info(HMI_DXY_IMAGE_PAGE, HMI_ACTIVE_PAGE_BIT);
	hmi_engine_set_object_info(HMI_DXY_IMAGE_PAGE, HMI_ACTIVE_PAGE_BIT);
	hmi_engine_set_object_info(HMI_NEW_IMAGE6_X, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE6_Y, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE6_W, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE6_H, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE7_X, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE7_Y, 120);
	hmi_engine_set_object_info(HMI_NEW_IMAGE7_W, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE7_H, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE8_X, 60);
	hmi_engine_set_object_info(HMI_NEW_IMAGE8_Y, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE8_W, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE8_H, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE9_X, 90);
	hmi_engine_set_object_info(HMI_NEW_IMAGE9_Y, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE9_W, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE9_H, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE11_X, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE11_Y, 60);
	hmi_engine_set_object_info(HMI_NEW_IMAGE11_W, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE11_H, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE13_X, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE13_Y, 90);
	hmi_engine_set_object_info(HMI_NEW_IMAGE13_W, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE13_H, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE14_X, 120);
	hmi_engine_set_object_info(HMI_NEW_IMAGE14_Y, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE14_W, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE14_H, 30);
	hmi_engine_set_object_info(HMI_SXY_IMAGELIST_PAGE, HMI_ACTIVE_PAGE_BIT);
	hmi_engine_set_object_info(HMI_DXY_IMAGELIST_PAGE, HMI_ACTIVE_PAGE_BIT);
	hmi_engine_set_object_info(HMI_NEW_IMAGE_LIST27_X, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE_LIST27_Y, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE_LIST27_W, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE_LIST27_H, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE_LIST28_X, 60);
	hmi_engine_set_object_info(HMI_NEW_IMAGE_LIST28_Y, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE_LIST28_W, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE_LIST28_H, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE_LIST29_X, 90);
	hmi_engine_set_object_info(HMI_NEW_IMAGE_LIST29_Y, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE_LIST29_W, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE_LIST29_H, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE_LIST30_X, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE_LIST30_Y, 60);
	hmi_engine_set_object_info(HMI_NEW_IMAGE_LIST30_W, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE_LIST30_H, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE_LIST31_X, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE_LIST31_Y, 90);
	hmi_engine_set_object_info(HMI_NEW_IMAGE_LIST31_W, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE_LIST31_H, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE_LIST32_X, 120);
	hmi_engine_set_object_info(HMI_NEW_IMAGE_LIST32_Y, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE_LIST32_W, 30);
	hmi_engine_set_object_info(HMI_NEW_IMAGE_LIST32_H, 30);
	hmi_engine_set_object_info(HMI_SXY_SCROLL_BAR_PAGE, HMI_ACTIVE_PAGE_BIT);//SCROLL//
	hmi_engine_set_object_info(HMI_DXY_SCROLL_BAR_PAGE, HMI_ACTIVE_PAGE_BIT);
	hmi_engine_set_object_info(HMI_NEW_SCROLL39_X, 10);
	hmi_engine_set_object_info(HMI_NEW_SCROLL39_Y, 10);
	hmi_engine_set_object_info(HMI_NEW_SCROLL39_W, 100);
	hmi_engine_set_object_info(HMI_NEW_SCROLL39_H, 20);
	hmi_engine_set_object_info(HMI_NEW_SCROLL40_X, 10);
	hmi_engine_set_object_info(HMI_NEW_SCROLL40_Y, 30);
	hmi_engine_set_object_info(HMI_NEW_SCROLL40_W, 100);
	hmi_engine_set_object_info(HMI_NEW_SCROLL40_H, 20);
	hmi_engine_set_object_info(HMI_NEW_SCROLL41_X, 10);
	hmi_engine_set_object_info(HMI_NEW_SCROLL41_Y, 30);
	hmi_engine_set_object_info(HMI_NEW_SCROLL41_W, 100);
	hmi_engine_set_object_info(HMI_NEW_SCROLL41_H, 20);
	hmi_engine_set_object_info(HMI_NEW_SCROLL42_X, 10);
	hmi_engine_set_object_info(HMI_NEW_SCROLL42_Y, 50);
	hmi_engine_set_object_info(HMI_NEW_SCROLL42_W, 100);
	hmi_engine_set_object_info(HMI_NEW_SCROLL42_H, 20);
	hmi_engine_set_object_info(HMI_NEW_SCROLL43_X, 10);
	hmi_engine_set_object_info(HMI_NEW_SCROLL43_Y, 70);
	hmi_engine_set_object_info(HMI_NEW_SCROLL43_W, 100);
	hmi_engine_set_object_info(HMI_NEW_SCROLL43_H, 20);
	hmi_engine_set_object_info(HMI_NEW_SCROLL44_X, 10);
	hmi_engine_set_object_info(HMI_NEW_SCROLL44_Y, 90);
	hmi_engine_set_object_info(HMI_NEW_SCROLL44_W, 100);
	hmi_engine_set_object_info(HMI_NEW_SCROLL44_H, 20);
	hmi_engine_set_object_info(HMI_TRAIL_PAGE_1, HMI_ACTIVE_PAGE_BIT);////
	hmi_engine_set_object_info(HMI_TRAIL_PAGE_2, HMI_ACTIVE_PAGE_BIT);
	hmi_engine_set_object_info(HMI_TRAIL_CONTAINER_ALPHA, 127);
	ctrl_point = HMI_2U16_TO_U32(0, 0);
	hmi_engine_set_object_info(HMI_ONEPOINT_SPLINE5, ctrl_point);
	ctrl_point = HMI_2U16_TO_U32(20, 30);
	hmi_engine_set_object_info(HMI_ONEPOINT_SPLINE5, ctrl_point);
	ctrl_point = HMI_2U16_TO_U32(30, 50);
	hmi_engine_set_object_info(HMI_ONEPOINT_SPLINE5, ctrl_point);
	hmi_engine_set_object_info(HMI_NEW_IMAGE10_W, 250);
	hmi_engine_set_object_info(HMI_NEW_IMAGE10_H, 200);

	hmi_engine_set_object_info(HMI_SPLINE_PAGE, HMI_ACTIVE_PAGE_BIT);
	hmi_engine_set_object_info(HMI_MULTIPOINT_SPLINE4_X, 20);
	hmi_engine_set_object_info(HMI_MULTIPOINT_SPLINE4_Y, 20);
	hmi_engine_set_object_info(HMI_MULTIPOINT_SPLINE4_W, 120);
	hmi_engine_set_object_info(HMI_MULTIPOINT_SPLINE4_H, 120);
	hmi_engine_set_object_info(HMI_SPLINE_IMAGE_W, 240);////
	hmi_engine_set_object_info(HMI_SPLINE_IMAGE_H, 240);
	hmi_engine_set_object_info(HMI_SPLINE_PAGE, HMI_ACTIVE_PAGE_BIT);
	hmi_engine_set_object_info(HMI_SPLINE_CONTAINER_W, 10);//spline//
	hmi_engine_set_object_info(HMI_SPLINE_CONTAINER_W, 3);
	hmi_engine_set_object_info(HMI_SPLINE_CONTAINER_W, 2);
	hmi_engine_set_object_info(HMI_SXY_FILL_COLOUR, 0Xff);
	hmi_engine_set_object_info(HMI_SPLINE_PAGE, HMI_ACTIVE_PAGE_BIT); 5
		hmi_engine_set_object_info(HMI_NEW_CONTAINER4_Y, 0);
	hmi_engine_set_object_info(HMI_CUBE_ACTION_ENTRY, HMI_ACTION_RUN);
	hmi_engine_set_object_info(HMI_CAMER_FILL2_Z, 700);
	hmi_engine_set_object_info(HMI_CAMER_FILL2_X, 300);
	hmi_engine_set_object_info(HMI_CAMER_FILL2_Y, 300);
	hmi_engine_set_object_info(HMI_LIGHT_FILL2_X, 500);
	hmi_engine_set_object_info(HMI_LIGHT_FILL2_Y, 500);
	hmi_engine_set_object_info(HMI_CUBE_ACTION_ENTRY, HMI_ACTION_RUN);//  
	hmi_engine_set_object_info(HMI_CAMERA_LOOKUP_X, 1);
	hmi_engine_set_object_info(HMI_CAMERA_LOOKUP_Y, 0);
	hmi_engine_set_object_info(HMI_CAMERA_LOOKUP_Z, 0);
	hmi_engine_set_object_info(HMI_PRIVATE_AXIS2_X, 100);
	hmi_engine_set_object_info(HMI_AXIS_POINT_3_X, 80);
	hmi_engine_set_object_info(HMI_AXIS_POINT_2_X, 70);

	hmi_engine_set_object_info(HMI_NEW_CUBE8_SCALE, 1);
	hmi_engine_set_object_info(HMI_AXIS_POINT_2_X, 80);
	hmi_engine_set_object_info(HMI_NEW_CUBE8_X, 200);
	hmi_engine_set_object_info(HMI_NEW_CUBE8_Y, 60);
	hmi_engine_set_object_info(HMI_NEW_CUBE8_Z, 200);
	hmi_engine_set_object_info(HMI_NEW_CUBE8_ANGEL, -80);
	hmi_engine_set_object_info(HMI_NEW_CUBE8_PRI_ANGEL, -80);

	hmi_engine_set_object_info(HMI_CUBE_PAGE2, HMI_ACTIVE_PAGE_BIT);

	hmi_engine_set_object_info(HMI_SXY_ACTION_ENTRY, HMI_ACTION_RUN);
	hmi_engine_set_object_info(HMI_NEW_CONTAINER14_X, 30);
	hmi_engine_set_object_info(HMI_CUBE_ACTION_ENTRY3, HMI_ACTION_RUN);
	hmi_engine_set_object_info(HMI_DIF_COLOR3_COLOUR, 0Xff);
	hmi_engine_set_object_info(HMI_CUBE_PAGE3, HMI_ACTIVE_PAGE_BIT);
	hmi_engine_set_object_info(HMI_CUBE_PAGE3, HMI_ACTIVE_PAGE_BIT);
	hmi_engine_set_object_info(HMI_NEW_CUBE17_X, 100);
	hmi_engine_set_object_info(HMI_NEW_CUBE17_Y, 100);
	hmi_engine_set_object_info(HMI_CUBE_PAGE3, HMI_ACTIVE_PAGE_BIT);
	hmi_engine_set_object_info(HMI_NEW_CUBE17_SCALE, 50);

#endif
#endif

#endif
#else 
	if ((tick_1 % 600) == 0)
	{
		tick_1_flag++;
	}
	if (tick_1_flag == 1) {

		hmi_engine_set_object_info(HMI_CLASSIC_PAGE, HMI_ACTIVE_PAGE_BIT/*Page on*/);
		hmi_engine_set_object_info(HMI_POINTER_IMAGE_ANGEL, HMI_F32_TO_U32(angel/*angle*/));

		hmi_engine_set_object_info(HMI_RIGHT_POINTER_IMAGE_ANGEL, HMI_F32_TO_U32(angel/*angle*/));
		if (!angle_flag_task)
		{
			hmi_engine_set_object_info(HMI_DYN_LIGHT_CONTAINER, HMI_PRE_LIGHT_CONTAINER);

			angel += 5;
			if (angel > 270.0f)
			{
				angel = 270.0f;
				angle_flag_task = 1;
			}
		}
		else if (angle_flag_task)
		{
			hmi_engine_set_object_info(HMI_DYN_LIGHT_CONTAINER, HMI_REAR_LIGHT_CONTAINER);

			angel -= 20;
			if (angel <= 0)
			{
				angel = 0;
				angle_flag_task = 0;
			}


		}

	}
	else if (tick_1_flag >= 2)
	{
		uint8_t ge = 0;
		uint8_t shi = 0;
		uint8_t bai = 0;
		uint32_t imge_list_flag_val = imge_list_flag;
		hmi_engine_set_object_info(HMI_MOTION_PAGE, HMI_ACTIVE_PAGE_BIT/*Page on*/);
		hmi_engine_set_object_info(HMI_DYN_NAVI_MOTION_CONTAINER, HMI_NAVI_CONTAINER);
		for (uint8_t i = 0; i < 1; i++)
		{
			ge = imge_list_flag_val % 10;
			imge_list_flag_val = imge_list_flag_val / 10;
			shi = imge_list_flag_val % 10;
			imge_list_flag_val = imge_list_flag_val / 10;
			bai = imge_list_flag_val % 10;

		}
		hmi_engine_set_object_info(HMI_SPEED_MOTION_100_IMAGE_LIST, bai);
		hmi_engine_set_object_info(HMI_SPEED_MOTION_10_IMAGE_LIST, shi);
		hmi_engine_set_object_info(HMI_SPEED_MOTION_1_IMAGE_LIST, ge);
		if (shi == 0 && bai == 0)
		{
			hmi_engine_set_object_info(HMI_LEFT_MOTION_NUMBER_CONTAINER0_SCALE, HMI_F32_TO_U32(1.0));

		}
		else if (shi == 2 && bai == 0)
		{
			hmi_engine_set_object_info(HMI_LEFT_MOTION_NUMBER_CONTAINER0_SCALE, HMI_F32_TO_U32(0.6));
			hmi_engine_set_object_info(HMI_LEFT_MOTION_NUMBER_CONTAINER20_SCALE, HMI_F32_TO_U32(1.0));



		}
		else if (shi == 4 && bai == 0)
		{
			hmi_engine_set_object_info(HMI_LEFT_MOTION_NUMBER_CONTAINER0_SCALE, HMI_F32_TO_U32(0.6));
			hmi_engine_set_object_info(HMI_LEFT_MOTION_NUMBER_CONTAINER20_SCALE, HMI_F32_TO_U32(0.6));
			hmi_engine_set_object_info(HMI_LEFT_MOTION_NUMBER_CONTAINER40_SCALE, HMI_F32_TO_U32(1.0));



		}





		imge_list_flag++;

	}



#endif

}



#if 0
void hmi_page_change(void)
{
	static BOOLEAN change_flag = FALSE;
	if (change_flag == FALSE)
	{

		hmi_engine_set_object_info(HMI_THEME2, HMI_ACTIVE_PAGE_BIT);
		change_flag = TRUE;
	}

	else
	{
		hmi_engine_set_object_info(HMI_MAIN_PAGE, HMI_ACTIVE_PAGE_BIT);
		change_flag = FALSE;

	}

}
#endif
void hmi_needle_color_call_func(void)
{

}
#if 0

void hmi_one_time_over(void)
{
	hmi_engine_set_object_info(HMI_CIRCLE_PAGE, HMI_ACTIVE_PAGE_BIT);

	hmi_engine_set_object_info(HMI_CIRCLE_LAYOUOT_ACTION_ENTRY3, HMI_ACTION_RUN);
	hmi_engine_set_object_info(HMI_LOOP_ENTRY, HMI_ACTION_PAUSE);

}
void hmi_circle_function(void)
{
	static UINT16 cur_id = HMI_LIB_CONTAINER0;
	static UINT16 last_id = HMI_LIB_CONTAINER1;

	last_id = cur_id;
	if (cur_id < HMI_LIB_CONTAINER4)
	{
		cur_id++;
		hmi_engine_set_object_info(HMI_CIRCLE_LAYOUOT_ACTION_ENTRY3, HMI_ACTION_RUN);
	}
	else
	{
		cur_id = HMI_LIB_CONTAINER0;
		hmi_engine_set_object_info(HMI_MAIN_CLUST_PAGE, HMI_ACTIVE_PAGE_BIT);

		hmi_engine_set_object_info(HMI_LOOP_ENTRY, HMI_ACTION_RUN);
		hmi_engine_set_object_info(HMI_CIRCLE_LAYOUOT_ACTION_ENTRY3, HMI_ACTION_PAUSE);
	}
	hmi_engine_set_object_info(HMI_CIRCLE_LIB3_P1, cur_id);

}

void hmi_phone_acition_start(void)
{

	hmi_engine_set_object_info(HMI_NAVIGATION_PHONE_ACTION_ENTRY1123, HMI_ACTION_RUN);
}

void hmi_video_acition_start(void)
{
	hmi_engine_set_object_info(HMI_VIDEO_ACTION_ENTRY36, HMI_ACTION_RUN);

}

void hmi_set_acition_start(void)
{
	hmi_engine_set_object_info(HMI_SETTING_ACTION_ENTRY253, HMI_ACTION_RUN);

}

#endif
/*****************************************************************************/
/*   */
/*****************************************************************************/
void FuelChange(UINT32 fuel)
{
	return;
}


/*****************************************************************************/
/*   */
/*****************************************************************************/

void CoolandTemperatureTravelChange(UINT32 temp)
{
#if 0
	//hmi_engine_set_object_info(HMI_NEED_COLOR_FILL_COLOUR, 0xffffffff);
	switch (temp)
	{
	case 0:
		hmi_engine_set_object_info(HMI_COOLANDLIST, HMI_COOLAND0);
		break;
	case 1:
		hmi_engine_set_object_info(HMI_COOLANDLIST, HMI_COOLAND1);
		break;
	case 2:
		hmi_engine_set_object_info(HMI_COOLANDLIST, HMI_COOLAND2);
		break;
	case 3:
		hmi_engine_set_object_info(HMI_COOLANDLIST, HMI_COOLAND3);
		break;
	case 4:
		hmi_engine_set_object_info(HMI_COOLANDLIST, HMI_COOLAND4);
		break;
	case 5:
		hmi_engine_set_object_info(HMI_COOLANDLIST, HMI_COOLAND5);
		break;
	case 6:
		hmi_engine_set_object_info(HMI_COOLANDLIST, HMI_COOLAND6);
		break;
	case 7:
		hmi_engine_set_object_info(HMI_COOLANDLIST, HMI_COOLAND7);
		break;
	case 8:
		hmi_engine_set_object_info(HMI_COOLANDLIST, HMI_COOLAND8);
		break;
	default:
		break;
	}
	return;


#endif
}


/*****************************************************************************/
/*   */
/*****************************************************************************/

void TripChange(UINT32 trip)
{
#if 0
	char _buf[6] = { '0','0','0','0','0','0' };
	HMI_CHAR_STR totalMaile[7] = { '0','0','0','0','0','0',0 };
	sprintf(_buf, "%06d", trip);
	for (UINT32 i = 0; i < 6; i++)
	{
		totalMaile[i] = _buf[i];
	}
	hmi_engine_edit_text(HMI_TOTALMAILE_VALUE, totalMaile);
	return;
#endif
}


/*****************************************************************************/
/*? */
/*****************************************************************************/

void SubTripChange(UINT32 trip)
{
#if 0
	char _buf[3] = { '0','0','0' };
	HMI_CHAR_STR remainMaile[4] = { '0','0','0',0 };
	sprintf(_buf, "%06d", trip);
	for (UINT32 i = 0; i < 6; i++)
	{
		remainMaile[i] = _buf[i];
	}
	hmi_engine_edit_text(HMI_REMAINMAILE_VALUE, remainMaile);
	return;
#endif
}


/*****************************************************************************/
/* */
/*****************************************************************************/

void RemainTripChange(UINT32 trip)
{
	//hmi_engine_set_object_info(HMI_NEED_COLOR_FILL_COLOUR, 0xffffffff);

	return;
}


/*****************************************************************************/
/*   */
/*****************************************************************************/
void Menu_ODOChange(UINT32 tabindex, UINT32 pagindex, UINT32 listindex)
{
#if 0
	switch (tabindex)
	{
	case BDHMI_MENU_ODO:
		hmi_engine_set_object_info(HMI_MENU, HMI_MENU_CAR);
		hmi_engine_set_object_info(HMI_ODO, HMI_ODO_CAR);
		if (pagindex == 0)
		{
			hmi_engine_set_object_info(HMI_ODO_CAR, HMI_ODO_CAR_LIST1);
		}
		else if (pagindex == 1)
		{
			hmi_engine_set_object_info(HMI_ODO_CAR, HMI_ODO_CAR_TIRE);
		}
		break;
	case BDHMI_MENU_ADAS:
		hmi_engine_set_object_info(HMI_MENU, HMI_MENU_ADAS);
		break;
	case BDHMI_MENU_PHONE:
		hmi_engine_set_object_info(HMI_MENU, HMI_MENU_PHONE);
		break;
	case BDHMI_MENU_MEDIA:
		hmi_engine_set_object_info(HMI_MENU, HMI_MENU_MEDIA);
		break;
	case BDHMI_MENU_SETTING:
		hmi_engine_set_object_info(HMI_MENU, HMI_MENU_SETTING);
		break;
	case BDHMI_MENU_PROBLEMLIST:
		hmi_engine_set_object_info(HMI_MENU, HMI_MENU_PROBLEMLIST);
		break;
	default:
		hmi_engine_set_object_info(HMI_MENU, HMI_MENU_NULL);
		break;
	}
#endif
}




/*****************************************************************************/
/*   */
/*****************************************************************************/

void Power_ChargeValue(char _chargestate, UINT32 _powervalue)
{
#if 0
	if (_chargestate == 1)
	{
		//
		hmi_engine_set_object_info(HMI_POWERCHARGEICON, HMI_POWERCHARGE_CHARGE);
	}
	else if (_chargestate == 0)
	{
		//
		if (_powervalue == 0)
		{
			hmi_engine_set_object_info(HMI_POWERCHARGEICON, HMI_POWERCHARGE_NOMAL);
			hmi_engine_set_object_info(HMI_POWERCHARGE_NOMAL, HMI_POWERICON0);
		}
		else if (_powervalue < 10)
		{
			hmi_engine_set_object_info(HMI_POWERCHARGE_NOMAL, HMI_POWERICON0);
		}
		else if (_powervalue < 20)
		{
			hmi_engine_set_object_info(HMI_POWERCHARGE_NOMAL, HMI_POWERICON1);
		}
		else if (_powervalue < 30)
		{
			hmi_engine_set_object_info(HMI_POWERCHARGE_NOMAL, HMI_POWERICON2);
		}
		else if (_powervalue < 40)
		{
			hmi_engine_set_object_info(HMI_POWERCHARGE_NOMAL, HMI_POWERICON3);
		}
		else if (_powervalue < 50)
		{
			hmi_engine_set_object_info(HMI_POWERCHARGE_NOMAL, HMI_POWERICON4);
		}
		else if (_powervalue < 60)
		{
			hmi_engine_set_object_info(HMI_POWERCHARGE_NOMAL, HMI_POWERICON5);
		}
		else if (_powervalue < 70)
		{
			hmi_engine_set_object_info(HMI_POWERCHARGE_NOMAL, HMI_POWERICON6);
		}
		else if (_powervalue < 80)
		{
			hmi_engine_set_object_info(HMI_POWERCHARGE_NOMAL, HMI_POWERICON7);
		}
		else if (_powervalue < 90)
		{
			hmi_engine_set_object_info(HMI_POWERCHARGE_NOMAL, HMI_POWERICON8);
		}
		else if (_powervalue < 100)
		{
			hmi_engine_set_object_info(HMI_POWERCHARGE_NOMAL, HMI_POWERICON9);
		}
		else
		{
			hmi_engine_set_object_info(HMI_POWERCHARGE_NOMAL, HMI_POWERICON10);
		}

	}
#endif
}

/*****************************************************************************/
/* */
/*****************************************************************************/

void Gear(char gear)
{
#if 0
	switch (gear)
	{
	case GEAR_NOTSHOW:
		hmi_engine_set_object_info(HMI_GEARICON, HMI_GEARICON_NULL);
		break;
	case GEAR_P:
		hmi_engine_set_object_info(HMI_GEARICON, HMI_GEARICON_P);
		break;
	case GEAR_N:
		hmi_engine_set_object_info(HMI_GEARICON, HMI_GEARICON_N);
		break;
	default:
		break;
	}
#endif
}

void hmi_printf_time(const char* printf_string, SINT32 value1)
{
	if (printf_string != NULL)
	{
#ifdef HMI_WINDOWS
		SYSTEMTIME sysTime;
		GetLocalTime(&sysTime);
		printf("\nhmi time: %ld.%03ld>>>>>>>>>", sysTime.wMinute, sysTime.wSecond);
		printf("%s ,value is (%ld).\n", printf_string, value1);

#else
#if 0
		struct timespec ts;
		if (printf_string != NULL)
		{
			clock_gettime(CLOCK_MONOTONIC, &ts);
			//printf("\nhmi time: %ld.%03ld>>>>>>>>>", ts.tv_sec, ts.tv_nsec);
			//fprintf(stderr, "%s ,value is (%f).\n", printf_string, value1);
			syslog(LOG_INFO, "hmi time: %ld.%03ld>>>>>>>>>", ts.tv_sec, ts.tv_nsec);
		}
#endif
		fprintf(stderr, "[QD_HMI]%s ,value is (%ld).\n", printf_string, value1);
		//syslog(LOG_INFO, "[QD_HMI]%s,value is %d[DEBUG]",printf_string,value1);
#endif

	}
}

void hmi_printf_time_f(const char* printf_string, float value1)
{
	if (printf_string != NULL)
	{

#ifdef HMI_WINDOWS
		SYSTEMTIME sysTime;
		GetLocalTime(&sysTime);
		printf("\nhmi time: %ld.%03ld>>>>>>>>>", sysTime.wMinute, sysTime.wSecond);
		printf("%s ,value is (%ld).\n", printf_string, value1);
#else
#if 0
		struct timespec ts;
		if (printf_string != NULL)
		{
			clock_gettime(CLOCK_MONOTONIC, &ts);
			//printf("\nhmi time: %ld.%03ld>>>>>>>>>", ts.tv_sec, ts.tv_nsec);
			//fprintf(stderr, "%s ,value is (%f).\n", printf_string, value1);
			syslog(LOG_INFO, "hmi time: %ld.%03ld>>>>>>>>>", ts.tv_sec, ts.tv_nsec);
		}
#endif

		fprintf(stderr, "[QD_HMI]%s ,value is (%f).\n", printf_string, value1);
		//syslog(LOG_INFO, "[QD_HMI]%s,value is %f[DEBUG]",printf_string,value1);
#endif
	}

#endif
}

