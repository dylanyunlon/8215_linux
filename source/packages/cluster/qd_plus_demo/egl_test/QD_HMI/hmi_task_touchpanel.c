/*****************************************************************************

File Name        :  hmi_task_toouchpanel.c
Organization    :  Zhuli Electronics Co.Ltd in Shanghai (www.shzldz.com)
******************************************************************************/
#include "hmi_all_struct_include.h"
#include <math.h>        /* sin and cos */

#ifdef HMI_TOUCH_PANEL

#define	HMI_TP_DRAG_DISTANCE_SEQ			0
#define	HMI_TP_INTO_DRAG_DISTANCE			15
#define	HMI_TP_INTO_DRAG_DISTANCE_SEQ		(HMI_TP_INTO_DRAG_DISTANCE*HMI_TP_INTO_DRAG_DISTANCE)

#define	HMI_TP_SCALE_DISTANCE_SEQ			2
#define	HMI_TP_INTO_SCALE_DISTANCE			15
#define	HMI_TP_INTO_SCALE_DISTANCE_SEQ		(HMI_TP_INTO_DRAG_DISTANCE*HMI_TP_INTO_DRAG_DISTANCE)
#define	HMI_TP_SCALE_PARAMETER				(0.05f)

#define	HMI_TP_MIN_FIRST_DISTANCE_SEQ		25
#define	HMI_TP_MIN_CUR_DISTANCE_SEQ			25


static	HMI_SPECIALKEY_TYPE		hmi_touch_status				= HMI_KEY_STATUS_UNKNOWN_OSD;
static	HMI_OBJECT_TOUCH_TYPE	hmi_current_process_touch		= {FALSE,HMI_ALL_OBJECT};
static	HMI_OBJECT_TOUCH_TYPE	hmi_previous_process_touch		= {FALSE,HMI_ALL_OBJECT};
static	TOUCH_BUTTON_STR		hmi_touch_button				= { HMI_NO_TOUCH_KEY,FALSE,HMI_KEY_CNT_OSD,{0,0},{0,0},FALSE,HMI_ROTATION_INVALIDE_ANGLE/*angle*/,-1.0f/*scale*/,{0,0},{0,0}};
#if	(HMI_SUPPORT_CALIBRATION==HMI_YES) 
UINT16 hmi_x_scale		= 0u;
UINT16 hmi_y_scale		= 0u;
UINT16 hmi_x_offset		= 0u;
UINT16 hmi_y_offset		= 0u;
UINT16 data_list[HMI_DATALIST_LEN];
#endif


static	HMI_TOUCH_COORDINATE_FIFO_TYPE hmi_tp_xy;
static	HMI_MULTI_TP_POINT_STR	hmi_multi_tp;
static	HMI_DEVICE_ID_STR		hmi_device_id_index[HMI_MULTI_TP_MAX_DEVICE];

static	HMI_TIME hmi_button_status_timer	= -1.0f;

static	BOOLEAN hmi_get_touch_xy(HMI_TOUCH_STATUS_TYPE *ptouch_status);
static	void	hmi_init_multi_tp_status(BOOLEAN bclear_last_drag);
static	void	hmi_init_multi_tp_point(HMI_GESTURE_POINT_CLEAR_TYPE	points);
static	BOOLEAN	hmi_gesture_status(HMI_TOUCH_STATUS_TYPE	*ptouch_data,TOUCH_BUTTON_STR *P_bt_status);
static	U08	hmi_active_device_multi_tp_point(HMI_GESTURE_POINT_CLEAR_TYPE	points);
static	void	hmi_return_gesture(HMI_GESTURE_INFO_STR *pgesture_info,TOUCH_BUTTON_STR	*P_bt_status);



#if defined (HMI_WINDOWS)	
void LogTrace(LPCTSTR pszFormat, ...)
{
    va_list pArgs;

	wchar_t szMessageBuffer[16380]={0};
    va_start( pArgs, pszFormat );
    _vsntprintf( szMessageBuffer, 16380, pszFormat, pArgs );
    va_end( pArgs );

    OutputDebugString(szMessageBuffer);
}
#else



#endif

/******************************************************************************
*
 *
 * Function:	PublicFunction
 *
 * Description: warm init the touch panel
 *				
 *				
 *
 * Invocation:	where this function is invoked
 *
 * Parameter:	parm1: none
 *				parm2: none
 *
 * Return:		none
 *				
 *
******************************************************************************/
void hmi_touch_panel_warm_init(void)
{	
	//UINT8		loopInit	= 0U;
	
	hmi_touch_status					= HMI_KEY_STATUS_UNKNOWN_OSD;	
	hmi_current_process_touch.button	= FALSE;
	hmi_current_process_touch.key		= HMI_ALL_OBJECT;
	hmi_previous_process_touch.button	= FALSE;
	hmi_previous_process_touch.key		= HMI_ALL_OBJECT;
	hmi_touch_button.button_type		= HMI_KEY_CNT_OSD;//HMI_KEY_STATUS_IDLE_OSD;
	hmi_touch_button.key				= HMI_ALL_OBJECT;
	#if(HMI_SUPPORT_CALIBRATION==HMI_YES) 
    hmi_eepread(EEP_ADDR_SYS_CAL_X_OFFSET,8, data_list);
    hmi_x_scale = data_list[5];
    hmi_x_scale = (hmi_x_scale << 8) | data_list[4];
    hmi_y_scale = data_list[7];
    hmi_y_scale = (hmi_y_scale << 8) | data_list[6];
    hmi_x_offset = data_list[1];
    hmi_x_offset = (hmi_x_offset << 8) | data_list[0];
    hmi_y_offset = data_list[3];
    hmi_y_offset = (hmi_y_offset << 8) | data_list[2];
//	cur_cursor = non_cursor; 
	//hmi_cursor_state = NORMAL_CURSOR;
	#endif
	/* Init touchPanel message fifo */
	hmi_button_status_timer		= -1.0f;
	/*tp message fifo init*/
	hmi_tp_xy.head	= 0u;
	hmi_tp_xy.tail	= 0u;
	hmi_init_multi_tp_status(TRUE);
	
}

/******************************************************************************
*
 *
 * Function:	PublicFunction
 *
 * Description: cold init the touch panel 
 *				
 *				
 *
 * Invocation:	where this function is invoked
 *
 * Parameter:	parm1: none
 *				parm2: none
 *
 * Return:		none
 *				
 *
 
******************************************************************************/
void hmi_touch_panel_cold_Init(void)
{	
    hmi_touch_panel_warm_init();
#if(HMI_SUPPORT_CALIBRATION == HMI_YES) 
 	hmi_calibate_para_init();
#endif
}


/******************************************************************************
*
 *
 *  Function:		PrivateFunction
 *
 *  Description:	set the button state by touch panel
 *
 *	Invocation:		where this function is invoked
 *
 *  Parameter:		private_parm1: button pointer
 *					private_parm2: touch coordinate pointer
 *
 *	Return:			void
 *					
 *
******************************************************************************/
void set_button_refresh_flag(HMI_OBJECT_ID_STR button_id,TOUCH_BUTTON_STR * p_touch_button)
{
	if(p_touch_button != NULL)
	{
		switch(p_touch_button->button_type)
		{			
			case HMI_PRESSKEY_OSD:				
				hmi_engine_set_object_info(button_id,HMI_BUTTON_PRESS_INDEX);
				break;			
			case HMI_SHORTKEY_OSD:
			case HMI_LONG_RELEASEKEY_OSD:
			case HMI_REPEAT_RELEASEKEY_OSD:
				hmi_engine_set_object_info(button_id,HMI_BUTTON_NORMAL_INDEX);
				break;
			default:
				;		
		}		
	}
}

/******************************************************************************
*
 *
 *  Function:		handle_falling_edge_of_button_press
 *
 *  Description:	handle the falling edge of button press
 *
 *	Invocation:		where this function is invoked
 *
 *  Parameter:		private_parm1: touch button 
 *					private_parm2: none
 *
 *	Return:			void
 *					
 *
******************************************************************************/
static void handle_falling_edge_of_button_press(TOUCH_BUTTON_STR * p_touch_button)
{	
	if(p_touch_button != NULL)
	{
		p_touch_button->button_type		= HMI_PRESSKEY_OSD;
		if(hmi_current_process_touch.button == TRUE)
		{
			set_button_refresh_flag(hmi_current_process_touch.key,
									p_touch_button);
		}
		/*Set long press timer*/
		HMI_CLEAR_TIMER();
		hmi_touch_status	= HMI_KEY_STATUS_WAIT_LONG_OSD;
		HMI_SET_TIMER(HMI_LONG_KEY_TIMER);
	}
}
/******************************************************************************
*
 *
 *  Function:		distinguish_drag_key_and_repeat_key
 *
 *  Description:	distinguish drag key and repeat_key
 *
 *	Invocation:		where this function is invoked
 *
 *  Parameter:		private_parm1: touch button 
 *					private_parm2: none
 *
 *	Return:			void
 *					
 *
******************************************************************************/
static void distinguish_drag_key_and_repeat_key(TOUCH_BUTTON_STR * p_touch_button)
{
	if(p_touch_button != NULL)
	{
		p_touch_button->button_type	= HMI_DRAGKEY_OSD;
	}
}

/******************************************************************************
*
 *
 *  Function:		announce_key_type
 *
 *  Description:	announce key type
 *
 *	Invocation:		where this function is invoked
 *
 *  Parameter:		private_parm1: touch button 
 *					private_parm2: none
 *
 *	Return:			void
 *					
 *
******************************************************************************/
static void announce_key_type(TOUCH_BUTTON_STR * p_touch_button)
{	
	if(p_touch_button != NULL)
	{
		if(hmi_touch_status == HMI_KEY_STATUS_WAIT_LONG_OSD)
		{
			p_touch_button->button_type	= HMI_SHORTKEY_OSD;
		}
		else if((hmi_touch_status == HMI_KEY_STATUS_WAIT_REPEAT_OSD)||
				(hmi_touch_status == HMI_KEY_STATUS_REPEAT_OSD))
		{
			p_touch_button->button_type	= HMI_LONG_RELEASEKEY_OSD;
		}
		else
		{
			p_touch_button->button_type	= HMI_ERRORKEY_OSD;
		}		
		if(hmi_previous_process_touch.button == TRUE)
		{
			set_button_refresh_flag(hmi_previous_process_touch.key, p_touch_button);
		}			  		 				 
		hmi_touch_status	= HMI_KEY_STATUS_IDLE_OSD;
		HMI_CLEAR_TIMER();
	}
}

/******************************************************************************
*
 *
 *  Function:		PrivateFunction
 *
 *  Description:	Judge the button type . e.g repeat button or long release
 *
 *	Invocation:		where this function is invoked
 *
 *  Parameter:		private_parm1: touch coordinate pointer
 *					private_parm2: none
 *
 *	Return:			void
 *					
 *
 
******************************************************************************/
static void   hmi_judge_buttontype(TOUCH_BUTTON_STR * p_touch_button,BOOLEAN hmi_button,TOUCH_BUTTON_STR *P_bt_status)
{
	//BOOLEAN		send_msg	= FALSE;
	
    hmi_current_process_touch.key		= p_touch_button->key;  
	hmi_current_process_touch.button	= hmi_button;
	
	if ((hmi_previous_process_touch.key != HMI_NO_TOUCH_KEY) && 
		(hmi_current_process_touch.key != HMI_NO_TOUCH_KEY) &&
		(hmi_previous_process_touch.key != hmi_current_process_touch.key))
	{	
		if(hmi_touch_status == HMI_KEY_STATUS_WAIT_LONG_OSD)
		{
			p_touch_button->button_type	= HMI_SHORTKEY_OSD;
		}
		else if((hmi_touch_status == HMI_KEY_STATUS_WAIT_REPEAT_OSD)||
				(hmi_touch_status == HMI_KEY_STATUS_REPEAT_OSD))
		{
			p_touch_button->button_type	= HMI_LONG_RELEASEKEY_OSD;
		}
		else
		{
			p_touch_button->button_type	= HMI_ERRORKEY_OSD;
		}
		if(hmi_previous_process_touch.button == TRUE)
		{
			set_button_refresh_flag(hmi_previous_process_touch.key, 
									p_touch_button);
		}
		P_bt_status[1].key					= hmi_previous_process_touch.key;
		P_bt_status[1].button_type			= p_touch_button->button_type;
		hmi_touch_status					= HMI_KEY_STATUS_IDLE_OSD;
		hmi_previous_process_touch.key		= HMI_NO_TOUCH_KEY;
		hmi_previous_process_touch.button	= FALSE;
	}

/**********************approximative falling edge of button press*********************************/
	if ((hmi_previous_process_touch.key == HMI_NO_TOUCH_KEY) && 
		(hmi_current_process_touch.key != HMI_NO_TOUCH_KEY)/*&&
		 (hmi_touch_status==HMI_KEY_STATUS_IDLE_OSD)*/)	
	{		
		handle_falling_edge_of_button_press(p_touch_button);
		P_bt_status[0].key			= hmi_current_process_touch.key;
		P_bt_status[0].button_type	= p_touch_button->button_type;
	}
		  		 
/******************************* distinguish held key or repeat key ******************************/
	
	if (( hmi_current_process_touch.key == hmi_previous_process_touch.key)
		&&( hmi_current_process_touch.key != HMI_NO_TOUCH_KEY ))
	{
        distinguish_drag_key_and_repeat_key(p_touch_button);
		P_bt_status[0].key			= hmi_current_process_touch.key;
		P_bt_status[0].button_type	= p_touch_button->button_type;
	}

/**************** At the rising edge of the key, it will announce the short key and long press release key ************/
	if ((hmi_previous_process_touch.key != HMI_NO_TOUCH_KEY) &&
		(hmi_current_process_touch.key == HMI_NO_TOUCH_KEY))
	{
	    announce_key_type(p_touch_button); 
		P_bt_status[0].key			= hmi_previous_process_touch.key;
		P_bt_status[0].button_type	= p_touch_button->button_type;
	}	
	hmi_previous_process_touch.key		= hmi_current_process_touch.key;	
	hmi_previous_process_touch.button	= hmi_current_process_touch.button;
}

/*
point compare current point
*/
static	BOOLEAN	hmi_is_move_tp(HMI_GESTURE_POINT_CLEAR_TYPE	points)
{
	BOOLEAN		move	= FALSE;
	U08			i		= 0u;

	if(points == HMI_MULTI_TP_PREVIOUS_POINTS)
	{
		for(i = 0u;(i < HMI_MULTI_TP_MAX_DEVICE)&&(move == FALSE);i++)
		{
			if((hmi_multi_tp.bprevious_active_device[i] < HMI_TOUCH_NO_ACTIVE)&&
				(hmi_multi_tp.bcur_active_device[i] < HMI_TOUCH_NO_ACTIVE))
			{
				if((hmi_multi_tp.previous_points[i].x != hmi_multi_tp.cur_points[i].x)||
					(hmi_multi_tp.previous_points[i].y != hmi_multi_tp.cur_points[i].y))
				{
					move	= TRUE;
				}
			}
		}
	}
	else if(points == HMI_MULTI_TP_FIRST_POINTS)
	{
		for(i = 0u;(i < HMI_MULTI_TP_MAX_DEVICE)&&(move == FALSE);i++)
		{						
			if((hmi_multi_tp.pfirst_points[i].x != hmi_multi_tp.cur_points[i].x)||
				(hmi_multi_tp.pfirst_points[i].y != hmi_multi_tp.cur_points[i].y))
			{
				move	= TRUE;
			}			
		}
	}
	else
	{
	}

	return		move;
}


static	BOOLEAN	hmi_get_tp_center(SPOINT32_TP *pcenter,HMI_GESTURE_POINT_CLEAR_TYPE	points)
{
	BOOLEAN			success			= FALSE;
	U08				i				= 0U;
	U08				cnt				= 0U;
	SPOINT32_TP		sum_point		= {0,0};
	
	if(pcenter != NULL)
	{
		if(points == HMI_MULTI_TP_PREVIOUS_POINTS)
		{
			for(i = 0u;i < HMI_MULTI_TP_MAX_DEVICE;i++)
			{
				if(hmi_multi_tp.bprevious_active_device[i] < HMI_TOUCH_NO_ACTIVE)
				{
					sum_point.x	+= hmi_multi_tp.previous_points[i].x;
					sum_point.y	+= hmi_multi_tp.previous_points[i].y;
					cnt++;
				}
			}
		}
		else if(points == HMI_MULTI_TP_CURRENT_POINTS)
		{
			for(i = 0u;i < HMI_MULTI_TP_MAX_DEVICE;i++)
			{
				if(hmi_multi_tp.bcur_active_device[i] < HMI_TOUCH_NO_ACTIVE)
				{
					/*refresh current point*/
					sum_point.x	+= hmi_multi_tp.cur_points[i].x;
					sum_point.y	+= hmi_multi_tp.cur_points[i].y;
					cnt++;
				}
				else if(hmi_multi_tp.bprevious_active_device[i] < HMI_TOUCH_NO_ACTIVE)
				{
					/*not refresh current point,use the previous point*/
					sum_point.x	+= hmi_multi_tp.previous_points[i].x;
					sum_point.y	+= hmi_multi_tp.previous_points[i].y;
					cnt++;
				}
				else
				{
				}
			}
		}
		else if(points == HMI_MULTI_TP_FIRST_POINTS)
		{
			for(i = 0u;i < HMI_MULTI_TP_MAX_DEVICE;i++)
			{
				if(hmi_multi_tp.bcur_active_device[i] < HMI_TOUCH_NO_ACTIVE)
				{
					/*refresh current point*/
					sum_point.x	+= hmi_multi_tp.pfirst_points[i].x;
					sum_point.y	+= hmi_multi_tp.pfirst_points[i].y;
					cnt++;
				}
				else if(hmi_multi_tp.bprevious_active_device[i] < HMI_TOUCH_NO_ACTIVE)
				{
					/*not refresh current point,use the previous point*/
					sum_point.x	+= hmi_multi_tp.pfirst_points[i].x;
					sum_point.y	+= hmi_multi_tp.pfirst_points[i].y;
					cnt++;
				}
				else
				{
				}
			}
		}
		else
		{
		}

		if(cnt != 0u)
		{
			pcenter->x	= (SINT32)(sum_point.x / ((float_32)cnt) + 0.5f);
			pcenter->y	= (SINT32)(sum_point.y / ((float_32)cnt) + 0.5f);
			success		= TRUE;
		}
	}

	return	success;
}



static	BOOLEAN	hmi_is_tp_drag(SPOINT32_TP *pbegin_point,SPOINT32_TP *pdistance,BOOLEAN *ptrue_press_end)
{
	BOOLEAN			success				= FALSE;
	BOOLEAN			pre_success			= FALSE;
	BOOLEAN			cur_success			= FALSE;
	SPOINT32_TP		previous_center		= {0,0};
	SPOINT32_TP		current_center		= {0,0};
	UINT32			distance_seq		= 0u;
	float_32		distance			= 0.0f;
	POINT_FLOAT_TP	normal				= {0.0f,0.0f};
	BOOLEAN			tp_move				= FALSE;

	/*init*/
	if(ptrue_press_end != NULL)
	{
		*ptrue_press_end	= FALSE;
	}
	
	tp_move	= hmi_is_move_tp(HMI_MULTI_TP_PREVIOUS_POINTS);
	if(tp_move == TRUE)
	{
		pre_success	= hmi_get_tp_center(&previous_center,HMI_MULTI_TP_PREVIOUS_POINTS);
		cur_success	= hmi_get_tp_center(&current_center,HMI_MULTI_TP_CURRENT_POINTS);

		if((pre_success == TRUE)&&(cur_success == TRUE)&&
			(pdistance != NULL)&&(pbegin_point != NULL))
		{
			distance_seq	= (UINT32)(HMI_DIS_SEQ2(current_center,previous_center));
			if(hmi_multi_tp.tp_status == HMI_MULTI_TP_DRAG)
			{
				if(distance_seq > HMI_TP_DRAG_DISTANCE_SEQ)
				{
					pbegin_point->x	= current_center.x;
					pbegin_point->y	= current_center.y;
					HMI_SUB_POINT(current_center,previous_center);				
					pdistance->x	= current_center.x;
					pdistance->y	= current_center.y;
					success	= TRUE;
				}
			}
			else 
			{
				if(distance_seq > HMI_TP_INTO_DRAG_DISTANCE_SEQ)
				{
					pbegin_point->x	= current_center.x;
					pbegin_point->y	= current_center.y;
					HMI_SUB_POINT(current_center,previous_center);				
					distance_seq	= (UINT32)(HMI_DIS_SEQ1(current_center));
					distance		= (float_32)(sqrt(distance_seq));
					if(distance > HMI_TP_INTO_DRAG_DISTANCE)
					{
						distance	-= HMI_TP_INTO_DRAG_DISTANCE;
					}
					else
					{
						distance	= 0.0f;
					}
					normal.x	= (float_32)(current_center.x);
					normal.y	= (float_32)(current_center.y);
					hmi_normalise_2d(&normal);
													
					pdistance->x	= (SINT32)(normal.x * distance +0.5f);
					pdistance->y	= (SINT32)(normal.y * distance +0.5f);

					/*end true press status*/
					if(ptrue_press_end != NULL)
					{
						if(hmi_multi_tp.tp_status == HMI_MULTI_TP_TRUE_PRESS)
						{
							*ptrue_press_end	= TRUE;
						}						
					}
					
					success	= TRUE;
				}
			}
		}
	}
	
	return	success;
}


static U08	hmi_point_vector_add_vector(SPOINT32_TP *ppoint,SPOINT32_TP *pvector,U08 vectore_array_length,HMI_GESTURE_POINT_CLEAR_TYPE	points)
{
	//BOOLEAN			success				= FALSE;
	U08				i					= 0u;	
	U08 			cnt					= 0u;

	if((ppoint != NULL)&&(pvector != NULL)&&
		(vectore_array_length >= HMI_MULTI_TP_MAX_DEVICE))
	{
		if(points == HMI_MULTI_TP_FIRST_POINTS)
		{
			for(i = 0U;i < HMI_MULTI_TP_MAX_DEVICE;i++)
			{
				if((hmi_multi_tp.bcur_active_device[i] == HMI_TOUCH_PRESS)||
					(hmi_multi_tp.bcur_active_device[i] == HMI_TOUCH_MOVE))
				{
					HMI_ADD_POINT2(hmi_multi_tp.pfirst_points[i],(*ppoint),pvector[i]);					
					cnt++;
				}	
				else if(hmi_multi_tp.bcur_active_device[i] == HMI_TOUCH_NO_ACTIVE)/*current point not refresh*/
				{
					if((hmi_multi_tp.bprevious_active_device[i] == HMI_TOUCH_PRESS)||
						(hmi_multi_tp.bprevious_active_device[i] == HMI_TOUCH_MOVE))/*check previous point*/
					{
						HMI_ADD_POINT2(hmi_multi_tp.pfirst_points[i],(*ppoint),pvector[i]);					
						cnt++;
					}
				}
				else
				{
				}								
			}
		}
		else if(points	== HMI_MULTI_TP_PREVIOUS_POINTS)
		{
			for(i = 0U;i < HMI_MULTI_TP_MAX_DEVICE;i++)
			{
				if(hmi_multi_tp.bprevious_active_device[i] != HMI_TOUCH_NO_ACTIVE)
				{
					HMI_ADD_POINT2(hmi_multi_tp.previous_points[i],(*ppoint),pvector[i]);					
					cnt++;
				}					
			}
		}
		else if(points	== HMI_MULTI_TP_CURRENT_POINTS)
		{
			for(i = 0U;i < HMI_MULTI_TP_MAX_DEVICE;i++)
			{	
				if((hmi_multi_tp.bcur_active_device[i] == HMI_TOUCH_PRESS)||
					(hmi_multi_tp.bcur_active_device[i] == HMI_TOUCH_MOVE))
				{
					HMI_ADD_POINT2(hmi_multi_tp.cur_points[i],(*ppoint),pvector[i]);					
					cnt++;
				}	
				else if(hmi_multi_tp.bcur_active_device[i] == HMI_TOUCH_NO_ACTIVE)/*current point not refresh*/
				{
					if((hmi_multi_tp.bprevious_active_device[i] == HMI_TOUCH_PRESS)||
						(hmi_multi_tp.bprevious_active_device[i] == HMI_TOUCH_MOVE))/*check previous point*/
					{
						HMI_ADD_POINT2(hmi_multi_tp.previous_points[i],(*ppoint),pvector[i]);					
						cnt++;
					}
				}
				else
				{
				}
			}
		}
		else
		{
		}
	}

	return		cnt;
}


static void	hmi_point_vector_add_vector2(SPOINT32_TP *ppoint,SPOINT32_TP *pvector_in,SPOINT32_TP *pvector_sum,HMI_GESTURE_POINT_CLEAR_TYPE	points)
{	
	U08				i		= 0u;

	if((ppoint != NULL)&&(pvector_in != NULL)&&(pvector_sum != NULL))		
	{	
		if(points	== HMI_MULTI_TP_CURRENT_POINTS)
		{
			for(i = 0U;i < HMI_MULTI_TP_MAX_DEVICE;i++)
			{	
				if((hmi_multi_tp.bcur_active_device[i] == HMI_TOUCH_PRESS)||
					(hmi_multi_tp.bcur_active_device[i] == HMI_TOUCH_MOVE))
				{
					HMI_ADD_POINT2(pvector_in[i],(*ppoint),pvector_sum[i]);						
				}	
				else if(hmi_multi_tp.bcur_active_device[i] == HMI_TOUCH_NO_ACTIVE)/*current point not refresh*/
				{
					if((hmi_multi_tp.bprevious_active_device[i] == HMI_TOUCH_PRESS)||
						(hmi_multi_tp.bprevious_active_device[i] == HMI_TOUCH_MOVE))/*check previous point*/
					{
						HMI_ADD_POINT2(pvector_in[i],(*ppoint),pvector_sum[i]);							
					}
				}
				else
				{
				}
			}
		}
		else
		{
		}

		
		
		
	}	
}



static	BOOLEAN	hmi_is_tp_scale(SPOINT32_TP *pbegin_point,float_32 *pscale)
{
	BOOLEAN			success					= FALSE;
	BOOLEAN			first_success			= FALSE;
	BOOLEAN			cur_success				= FALSE;
	SPOINT32_TP		first_center			= {0,0};
	SPOINT32_TP		current_center			= {0,0};
	UINT32			first_distance_seq		= 0u;
	UINT32			cur_distance_seq		= 0u;	
	//float_32		distance				= 0.0f;
	//POINT_FLOAT_TP	normal					= {0.0f,0.0f};
	U08				i						= 0u;
	float_32		scale					= 0.0f;
	U08				first_point_cnt			= 0u;
	U08				current_point_cnt		= 0u;
	U08				min_point_cnt			= 0u;
	BOOLEAN			tp_move					= FALSE;
	SPOINT32_TP		first_vectore[HMI_MULTI_TP_MAX_DEVICE]	= {0};
	SPOINT32_TP		cur_vectore[HMI_MULTI_TP_MAX_DEVICE]	= {0};
	SPOINT32_TP		current_to_first		= {0,0};

	tp_move	= hmi_is_move_tp(HMI_MULTI_TP_PREVIOUS_POINTS);
	if(tp_move == TRUE)
	{
		for(i = 0u;i < HMI_MULTI_TP_MAX_DEVICE;i++)
		{
			first_vectore[i].x	= 0;
			first_vectore[i].y	= 0;

			cur_vectore[i].x	= 0;
			cur_vectore[i].y	= 0;
		}

		first_success	= hmi_get_tp_center(&first_center,HMI_MULTI_TP_FIRST_POINTS);
		cur_success		= hmi_get_tp_center(&current_center,HMI_MULTI_TP_CURRENT_POINTS);

		if((first_success == TRUE)&&(cur_success == TRUE)&&
			(pbegin_point != NULL)&&(pscale != NULL))
		{				
			/*first center to current center vectore*/
			HMI_SUB_POINT2(first_center,current_center,current_to_first);
			/*move current vector center to first vector center*/
			current_point_cnt	= hmi_point_vector_add_vector(&current_to_first,cur_vectore,
										HMI_MULTI_TP_MAX_DEVICE,HMI_MULTI_TP_CURRENT_POINTS);
			/*get vector from ploy center to vector*/
			first_center.x	= -first_center.x;
			first_center.y	= -first_center.y;
			first_point_cnt	= hmi_point_vector_add_vector(&first_center,first_vectore,
									HMI_MULTI_TP_MAX_DEVICE,HMI_MULTI_TP_FIRST_POINTS);
			hmi_point_vector_add_vector2(&first_center,cur_vectore,
								cur_vectore,HMI_MULTI_TP_CURRENT_POINTS);
			if(first_point_cnt > current_point_cnt)
			{
				min_point_cnt	= current_point_cnt;
			}
			else
			{
				min_point_cnt	= first_point_cnt;
			}
			if(min_point_cnt >= 2/*scale,at least 2 point*/)
			{
				for(i = 0u;i < HMI_MULTI_TP_MAX_DEVICE;i++)
				{	
					first_distance_seq	= 0;
					cur_distance_seq	= 0;
					if((first_vectore[i].x != 0)||(first_vectore[i].y != 0))
					{
						first_distance_seq	= HMI_DIS_SEQ1(first_vectore[i]);						
					}
					
					if((cur_vectore[i].x != 0)||(cur_vectore[i].y != 0))
					{
						cur_distance_seq	= HMI_DIS_SEQ1(cur_vectore[i]);							
					}

					if((first_distance_seq > 0)&&(cur_distance_seq > 0))
					{
						scale	+= (float_32)(sqrt(cur_distance_seq) / sqrt(first_distance_seq));
					}
					
				}
				scale	= (float_32)(scale / min_point_cnt);
				*pscale	= scale;
				pbegin_point->x	= current_center.x;
				pbegin_point->y	= current_center.y;
				
				success	= TRUE;
			}		
		}
	}

	return	success;
}

/*
a to b angle
*/
static float_32 hmi_vector_angle(SPOINT32_TP *pfirst_vector,SPOINT32_TP *plast_vector,SPOINT32_TP *pcur_vector)
{
	float_32		angle			= 0.0f;
	float_32		cos_angle		= 0.0f;
	//SPOINT32_TP		cross_mul		= {0,0};
	SINT32			cross_mul_z		= 0;
	float_32		a_b_length		= 0.0f;
	SINT32			axb				= 0;
	BOOLEAN			ccw				= FALSE;
	BOOLEAN			valide			= TRUE;
	SINT32			first_cur_dot	= 0;
	
	if((pfirst_vector != NULL)&&(plast_vector != NULL)&&(pcur_vector != NULL))
	{
		axb		= (SINT32)(HMI_DOT_MUL2((*plast_vector),(*pcur_vector)));
		if(axb > 0)
		{
			ccw	= TRUE;
		}
		else if(axb < 0)
		{
			ccw	= FALSE;
		}
		else	/*a and b at one line*/
		{
			valide = FALSE;
		}
		
		if(valide == TRUE)
		{
			a_b_length	= (float_32)(sqrt(HMI_DIS_SEQ1((*pfirst_vector))) * 
									sqrt(HMI_DIS_SEQ1((*pcur_vector))));
			if(fabs(a_b_length) > HMI_FLOAT_TOLERANCE)
			{
				first_cur_dot	= (SINT32)(HMI_DOT_MUL2((*pfirst_vector),(*pcur_vector)));
				cos_angle		= (float_32)(first_cur_dot / a_b_length);
				if(cos_angle > 1.0f)
				{
					cos_angle	= 1.0f;
				}
				else if(angle < 0.0f)
				{
					cos_angle	= 0.0f;
				}
				else
				{
				}
				angle		= (float_32)(acos(cos_angle));
				
				cross_mul_z	= HMI_CROSS_MUL_Z((*pfirst_vector),(*pcur_vector));
				if(cross_mul_z > 0)
				{
					angle	= -angle;
				}
				angle	= (float_32)(angle / HMI_PI);
			}
		}
	}

	return		angle;
}

static	BOOLEAN	hmi_is_tp_rotation(SPOINT32_TP *pbegin_point,float_32 *pangle)
{
	BOOLEAN			success				= FALSE;
	BOOLEAN			first_success		= FALSE;
	BOOLEAN			cur_success			= FALSE;
	SPOINT32_TP		first_center		= {0,0};
	SPOINT32_TP		current_center		= {0,0};
	//UINT32			first_distance_seq	= 0u;
	UINT32			cur_distance_seq	= 0u;
	//float_32		distance			= 0.0f;
	//POINT_FLOAT_TP	normal				= {0.0f,0.0f};
	U08				i					= 0u;
	float_32		angle				= 0.0f;
	U08				first_point_cnt		= 0u;
	U08				current_point_cnt	= 0u;
	U08				min_point_cnt		= 0u;
	BOOLEAN			tp_move				= FALSE;
	SPOINT32_TP		first_vectore[HMI_MULTI_TP_MAX_DEVICE]	= {0};
	SPOINT32_TP		pre_vectore[HMI_MULTI_TP_MAX_DEVICE]	= {0};
	SPOINT32_TP		cur_vectore[HMI_MULTI_TP_MAX_DEVICE]	= {0};
	SPOINT32_TP		center_to_first		= {0,0};

	tp_move	= hmi_is_move_tp(HMI_MULTI_TP_PREVIOUS_POINTS);
	if(tp_move == TRUE)
	{
		for(i = 0u;i < HMI_MULTI_TP_MAX_DEVICE;i++)
		{
			first_vectore[i].x	= 0;
			first_vectore[i].y	= 0;

			cur_vectore[i].x	= 0;
			cur_vectore[i].y	= 0;
		}

		first_success	= hmi_get_tp_center(&first_center,HMI_MULTI_TP_FIRST_POINTS);
		cur_success		= hmi_get_tp_center(&current_center,HMI_MULTI_TP_CURRENT_POINTS);

		if((first_success == TRUE)&&(cur_success == TRUE)&&
			(pbegin_point != NULL)&&(pangle != NULL))
		{	
			/*current center to first center vectore*/
			HMI_SUB_POINT2(first_center,current_center,center_to_first);
			/*move current vector center to first vector center*/
			current_point_cnt	= hmi_point_vector_add_vector(&center_to_first,cur_vectore,
										HMI_MULTI_TP_MAX_DEVICE,HMI_MULTI_TP_CURRENT_POINTS);
			/*get vector from ploy center to vector*/
			first_center.x	= -first_center.x;
			first_center.y	= -first_center.y;
			first_point_cnt	= hmi_point_vector_add_vector(&first_center,first_vectore,
									HMI_MULTI_TP_MAX_DEVICE,HMI_MULTI_TP_FIRST_POINTS);
			hmi_point_vector_add_vector2(&first_center,cur_vectore,
									cur_vectore,HMI_MULTI_TP_CURRENT_POINTS);
			if(first_point_cnt > current_point_cnt)
			{
				min_point_cnt	= current_point_cnt;
			}
			else
			{
				min_point_cnt	= first_point_cnt;
			}
			if(min_point_cnt >= 2/*scale,at least 2 point*/)
			{
				for(i = 0u;i < HMI_MULTI_TP_MAX_DEVICE;i++)
				{
					angle	= hmi_vector_angle(&first_vectore[i],
								&pre_vectore[i],
								&cur_vectore[i]);									
				}
				angle	= (float_32)(angle / min_point_cnt);
				if(fabs(angle) > HMI_FLOAT_TOLERANCE)
				{
					*pangle	= angle;			
				}
				else
				{
					*pangle	= HMI_ROTATION_INVALIDE_ANGLE;
				}
				
				success	= TRUE;
			}	
			else
			{
				*pangle	= HMI_ROTATION_INVALIDE_ANGLE;
			}
		}
	}

	return	success;
}

/*
at true press status,drag to new elemet or all tp release
*/
static BOOLEAN hmi_true_press_drag(TOUCH_BUTTON_STR *P_bt_status)
{
	BOOLEAN			success				= FALSE;
	BOOLEAN			tp_move				= FALSE;
	BOOLEAN			pre_success			= FALSE;
	BOOLEAN			cur_success			= FALSE;
	BOOLEAN			current_button		= FALSE;		
	SPOINT32_TP		current_center		= {0,0};
	POINT_TP		hmi_point			= {0,0};
	UINT8			action_device_cnt	= 0u;
	HMI_OBJECT_ID_STR	new_key_id		= HMI_NO_TOUCH_KEY;

	if((P_bt_status != NULL)&&(hmi_multi_tp.tp_status == HMI_MULTI_TP_TRUE_PRESS))
	{
		tp_move	= hmi_is_move_tp(HMI_MULTI_TP_PREVIOUS_POINTS);
		if(tp_move == TRUE)
		{			
			cur_success	= hmi_get_tp_center(&current_center,HMI_MULTI_TP_CURRENT_POINTS);
			hmi_point.x	= (UINT16)(current_center.x);
			hmi_point.y	= (UINT16)(current_center.y);
			new_key_id	= hmi_search_page(&hmi_point,&current_button);
			if(hmi_touch_button.key	!= new_key_id)
			{
				hmi_touch_button.key	= HMI_NO_TOUCH_KEY;
				hmi_judge_buttontype(&hmi_touch_button,
									hmi_touch_button.bbutton,
									P_bt_status);

				hmi_multi_tp.tp_status	= HMI_MULTI_TP_GESTURE;
				hmi_touch_status		= HMI_KEY_STATUS_GESTURE_OSD;
				success					= TRUE;
			}
			else	/*all tp release*/
			{
				action_device_cnt	= hmi_active_device_multi_tp_point(HMI_MULTI_TP_CURRENT_POINTS);
				if(action_device_cnt == 0u)
				{
					hmi_touch_button.key	= HMI_NO_TOUCH_KEY;
					hmi_judge_buttontype(&hmi_touch_button,
										hmi_touch_button.bbutton,
										P_bt_status);

					hmi_multi_tp.tp_status	= HMI_MULTI_TP_GESTURE;
					hmi_touch_status		= HMI_KEY_STATUS_GESTURE_OSD;
					success					= TRUE;
				}
			}
		}
		else	/*all tp release*/
		{
			action_device_cnt	= hmi_active_device_multi_tp_point(HMI_MULTI_TP_CURRENT_POINTS);
			if(action_device_cnt == 0u)
			{
				hmi_touch_button.key	= HMI_NO_TOUCH_KEY;
				hmi_judge_buttontype(&hmi_touch_button,
									hmi_touch_button.bbutton,
									P_bt_status);

				hmi_multi_tp.tp_status	= HMI_MULTI_TP_GESTURE;
				hmi_touch_status		= HMI_KEY_STATUS_GESTURE_OSD;
				success					= TRUE;
			}
		}
	}

	return	success;
}

static BOOLEAN	hmi_get_tp_gesture(HMI_GESTURE_INFO_STR *pgesture_info,TOUCH_BUTTON_STR *P_bt_status)
{
	BOOLEAN		success						= FALSE;
	BOOLEAN		success_drag				= FALSE;
	BOOLEAN		success_scale				= FALSE;
	BOOLEAN		success_rotation			= FALSE;
	BOOLEAN		success_true_press			= FALSE;
	BOOLEAN		true_press_end				= FALSE;
	SPOINT32_TP	begin_point					= {0,0};	
	POINT32_TP	rotation_begin_point		= {0,0};	
	POINT32_TP	scale_point					= {0,0};
	SPOINT32_TP	distance					= {0,0};	
	float_32	scale						= -1.0f;
	float_32	angle						= HMI_ROTATION_INVALIDE_ANGLE;
	U08			action_device_cnt			= 0U;

	if(pgesture_info != NULL)
	{
		pgesture_info->begin_point.x			= HMI_TP_INVALIDE_VALUE;
		pgesture_info->begin_point.y			= HMI_TP_INVALIDE_VALUE;
		pgesture_info->rotation_begin_point.x	= HMI_TP_INVALIDE_VALUE;
		pgesture_info->rotation_begin_point.y	= HMI_TP_INVALIDE_VALUE;
		pgesture_info->scale_point.x			= HMI_TP_INVALIDE_VALUE;
		pgesture_info->scale_point.y			= HMI_TP_INVALIDE_VALUE;
		pgesture_info->distance.x				= 0;
		pgesture_info->distance.y				= 0;
		pgesture_info->scale					= -1.0f;
		pgesture_info->angle					= HMI_ROTATION_INVALIDE_ANGLE;
		
		if(hmi_multi_tp.tp_status != HMI_MULTI_TP_NO_TOUCH)
		{
			/*check drag*/
			success_drag	= hmi_is_tp_drag(&begin_point,
											&distance,
											&true_press_end);
			if(success_drag == FALSE)
			{
				begin_point.x	= HMI_TP_INVALIDE_VALUE;
				begin_point.y	= HMI_TP_INVALIDE_VALUE;
				distance.x		= 0;
				distance.y		= 0;
				
				/*when not generate drag message(drag distance too small),but at true press status,may drag to new element*/
				success_true_press	= hmi_true_press_drag(P_bt_status);
			}
			else /*get drag*/
			{
				/*from true press to drag,generate end true press key*/
				if(true_press_end == TRUE)
				{
					hmi_touch_button.key	= HMI_NO_TOUCH_KEY;
					hmi_judge_buttontype(&hmi_touch_button,
									hmi_touch_button.bbutton,
									P_bt_status);					
				}
				/*drag key message*/
				action_device_cnt	= hmi_active_device_multi_tp_point(HMI_MULTI_TP_CURRENT_POINTS);
				if(action_device_cnt == 0)/*all tp release*/
				{					
					hmi_multi_tp.tp_status	= HMI_MULTI_TP_NO_TOUCH;
					hmi_touch_status		= HMI_KEY_STATUS_IDLE_OSD;
				}
				else
				{										
					hmi_multi_tp.tp_status	= HMI_MULTI_TP_DRAG;
					hmi_touch_status		= HMI_KEY_STATUS_DRAGKEY_OSD;	
				}
				
							
			}

			/*check scale*/
			success_scale	= hmi_is_tp_scale(&scale_point,&scale);
			if(success_scale == FALSE)
			{
				scale_point.x	= HMI_TP_INVALIDE_VALUE;
				scale_point.y	= HMI_TP_INVALIDE_VALUE;
				scale			= -1.0f;
			}
			else
			{
				hmi_multi_tp.tp_status	= HMI_MULTI_TP_SCALE;
				hmi_touch_status		= HMI_KEY_STATUS_GESTURE_OSD;
			}

			/*check rotation*/
			success_rotation	= hmi_is_tp_rotation(&rotation_begin_point,&angle);
			if(success_rotation == FALSE)
			{
				rotation_begin_point.x	= HMI_TP_INVALIDE_VALUE;
				rotation_begin_point.y	= HMI_TP_INVALIDE_VALUE;
				angle					= HMI_ROTATION_INVALIDE_ANGLE;
			}
			else
			{
				hmi_multi_tp.tp_status	= HMI_MULTI_TP_ROTATION;
				hmi_touch_status		= HMI_KEY_STATUS_GESTURE_OSD;
			}
			
			pgesture_info->begin_point.x			= begin_point.x;
			pgesture_info->begin_point.y			= begin_point.y;
			pgesture_info->scale_point.x			= scale_point.x;
			pgesture_info->scale_point.y			= scale_point.y;
			pgesture_info->rotation_begin_point.x	= rotation_begin_point.x;
			pgesture_info->rotation_begin_point.y	= rotation_begin_point.y;
			pgesture_info->distance.x				= distance.x;
			pgesture_info->distance.y				= distance.y;
			pgesture_info->scale					= scale;
			pgesture_info->angle					= angle;
		}
	}

	success	= success_rotation || success_scale || success_drag || success_true_press;

	return	success;
}

static	void hmi_copy_current_tp_to_previous(void)
{
	U08		i	= 0u;
	
	for(i = 0u;i < HMI_MULTI_TP_MAX_DEVICE;i++)
	{
		hmi_multi_tp.bprevious_active_device[i]	= hmi_multi_tp.bcur_active_device[i];
		if(hmi_multi_tp.bprevious_active_device[i] == HMI_TOUCH_UP)
		{
			hmi_multi_tp.bprevious_active_device[i] = HMI_TOUCH_NO_ACTIVE;
		}
		hmi_multi_tp.previous_points[i].x		= hmi_multi_tp.cur_points[i].x;
		hmi_multi_tp.previous_points[i].y		= hmi_multi_tp.cur_points[i].y;
	}
}

static void	hmi_free_device_index(U08		device_ID_index)
{		
	if(device_ID_index < HMI_MULTI_TP_MAX_DEVICE)
	{						
		hmi_device_id_index[device_ID_index].bused 			= FALSE;
		hmi_device_id_index[device_ID_index].hmi_device_id	= 0U;		
	}	
}


void  hmi_touch_panel(HMI_TIME dt,TOUCH_BUTTON_STR *P_bt_status)    
{	
	HMI_OBJECT_ID_STR			object_id_hit		= HMI_ALL_OBJECT;		
	HMI_TOUCH_STATUS_TYPE		new_touch_data		= {{0,0},0,0};
	BOOLEAN						finished			= FALSE;
	BOOLEAN						get_tp_msg			= FALSE;	
	//HMI_GESTURE_STATUS_TYPE		last_tp_status		= HMI_MULTI_GESTURE_CNT;
	POINT_TP 					hmi_point			= {0,0};
	HMI_GESTURE_INFO_STR		gesture_info		= {0};
	BOOLEAN						tp_msg_fifo_empty	= TRUE;
	BOOLEAN						get_key				= FALSE;
	U08							i_msg				= 0u;
	

	/*init return key status*/
	P_bt_status[0].key					= HMI_NO_TOUCH_KEY;
	P_bt_status[0].button_type			= HMI_KEY_CNT_OSD;
	P_bt_status[0].delta_s_speed.x		= 0;
	P_bt_status[0].delta_s_speed.y		= 0;
	P_bt_status[0].drag_position.x		= -1;
	P_bt_status[0].drag_position.y		= -1;
	P_bt_status[0].rotation_center.x	= 0;
	P_bt_status[0].rotation_center.y	= 0;
	P_bt_status[0].rotation_angle		= HMI_ROTATION_INVALIDE_ANGLE;
	P_bt_status[0].scale_center.x		= 0;
	P_bt_status[0].scale_center.y		= 0;
		
	P_bt_status[1].key					= HMI_NO_TOUCH_KEY;
	P_bt_status[1].button_type			= HMI_KEY_CNT_OSD;
	P_bt_status[1].delta_s_speed.x		= 0;
	P_bt_status[1].delta_s_speed.y		= 0;
	P_bt_status[1].drag_position.x		= -1;
	P_bt_status[1].drag_position.y		= -1;
	P_bt_status[1].rotation_center.x	= 0;
	P_bt_status[1].rotation_center.y	= 0;
	P_bt_status[1].rotation_angle		= HMI_ROTATION_INVALIDE_ANGLE;
	P_bt_status[1].scale_center.x		= 0;
	P_bt_status[1].scale_center.y		= 0;	

	//last_tp_status	= hmi_multi_tp.tp_status;
	/*Timer elapse*/
	if(hmi_button_status_timer > 0.0f) /*enable button timer*/
	{
		hmi_button_status_timer	-= dt;
		if(hmi_button_status_timer < 0.0f)
		{
			hmi_button_status_timer	= 0.0f;
		}
	}
	/*Timer over ,key status change by timer*/
	if(HMI_TIMER_OVER())  
	{
		HMI_CLEAR_TIMER();
		switch(hmi_touch_status)
		{
			case HMI_KEY_STATUS_WAIT_LONG_OSD:				
				hmi_touch_button.button_type	= HMI_LONGKEY_OSD;
				hmi_touch_status				= HMI_KEY_STATUS_WAIT_REPEAT_OSD;
				HMI_SET_TIMER(HMI_REPEAT_KEY_FIRST_TIMER);
				P_bt_status[0].key				= hmi_touch_button.key;
				P_bt_status[0].button_type		= hmi_touch_button.button_type;
				get_tp_msg						= TRUE;				
				break;
			case HMI_KEY_STATUS_REPEAT_OSD:
			case HMI_KEY_STATUS_WAIT_REPEAT_OSD:				
				hmi_touch_button.button_type	= HMI_REPEATKEY_OSD;
				HMI_SET_TIMER(HMI_REPEAT_KEY_FIRST_TIMER);
				hmi_touch_status				= HMI_KEY_STATUS_REPEAT_OSD;
				P_bt_status[0].key				= hmi_touch_button.key;
				P_bt_status[0].button_type		= hmi_touch_button.button_type;
				get_tp_msg						= TRUE;				
				break;
			case HMI_KEY_STATUS_FIRST_PRESS_OSD:				
				hmi_touch_status				= HMI_KEY_STATUS_WAIT_LONG_OSD;
				hmi_multi_tp.tp_status			= HMI_MULTI_TP_TRUE_PRESS;
			#if HMI_PAGES_NUMBER > 0				
				hmi_point.x	= (UINT16)(hmi_touch_button.delta_s_speed.x/*first press point*/);
				hmi_point.y	= (UINT16)(hmi_touch_button.delta_s_speed.y/*first press point*/);
				/*get button or element ID*/
				hmi_touch_button.bbutton	= FALSE;
				object_id_hit	= hmi_search_page(&hmi_point,
												&(hmi_touch_button.bbutton));
			#endif
				if(object_id_hit != HMI_NO_TOUCH_KEY)/*get hit element id*/
				{
					hmi_touch_button.key	= object_id_hit;
				}
				else
				{
					hmi_touch_button.key	= HMI_NO_TOUCH_KEY;
				}

				hmi_judge_buttontype(&hmi_touch_button,
									hmi_touch_button.bbutton,P_bt_status);
				hmi_multi_tp.true_press_key		= hmi_touch_button.key;								
				get_tp_msg						= TRUE;				
				break;
			default:
				break;				
		}			
	}

	/*not get a key event by timer,get tp touch refresh tp xy,search slide key event*/
	if(get_tp_msg == FALSE)
	{
		while((hmi_get_touch_xy(&new_touch_data))&&
				get_tp_msg == FALSE)/*Get a tp event*/
		{			
			get_tp_msg	= hmi_gesture_status(&new_touch_data,P_bt_status);			
			if(new_touch_data.press == HMI_TOUCH_UP)
			{
				hmi_free_device_index(new_touch_data.device_id);
			}
			tp_msg_fifo_empty	= FALSE;					
		}

		/*gesture tp message*/		
		if((tp_msg_fifo_empty == FALSE)&&
			(get_tp_msg == FALSE))
		{
			finished	= hmi_get_tp_gesture(&gesture_info,P_bt_status);
			
			if(finished == TRUE)
			{
				/*return value*/
				hmi_return_gesture(&gesture_info,P_bt_status);	
				/*Save drag speed for next get slide speed*/
				if((gesture_info.distance.x != 0)&&(gesture_info.distance.y != 0))
				{
					hmi_multi_tp.last_drag_distance.x	= (float_32)(gesture_info.distance.x);
					hmi_multi_tp.last_drag_distance.y	= (float_32)(gesture_info.distance.y);
				}
				else
				{
					hmi_multi_tp.last_drag_distance.x	= 0.0f;
					hmi_multi_tp.last_drag_distance.y	= 0.0f;
				}
			}
		}

		/*fade drag speed for next slide speed*/
		if(hmi_multi_tp.last_drag_distance.x != 0)
		{
			hmi_multi_tp.last_drag_distance.x	= (float_32)(hmi_multi_tp.last_drag_distance.x * HMI_DRAG_SPEED_FADE);
		}

		if(hmi_multi_tp.last_drag_distance.y != 0)
		{
			hmi_multi_tp.last_drag_distance.y	= (float_32)(hmi_multi_tp.last_drag_distance.y * HMI_DRAG_SPEED_FADE);
		}
						
		if((tp_msg_fifo_empty == FALSE/*message tp not empty*/)&&
			((get_tp_msg == TRUE)||/*get key message*/
			(finished == TRUE/*get gesture message*/)||
			/*at true press,if move distance is small,copy current to previous */
			hmi_multi_tp.tp_status	== HMI_MULTI_TP_TRUE_PRESS))
		{
			/*copy current tp to previous*/
			hmi_copy_current_tp_to_previous();
			/*init current point to zeor*/
			hmi_init_multi_tp_point(HMI_MULTI_TP_CURRENT_POINTS);					
		}
	}			
	
	/*Printf key message*/
	for(i_msg = 0u;i_msg < HMI_SOFTKEY_LEN;i_msg++)
	{
		if(P_bt_status[i_msg].key != HMI_NO_TOUCH_KEY)
		{
			get_key	= TRUE;							
			switch(P_bt_status[i_msg].button_type)
			{
				case	HMI_PRESSKEY_OSD:
					#if HMI_EVENT_STAND_NUMBER > 0
						hmi_do_event(P_bt_status[i_msg].key,
									HMI_BUTTON_PRESS);/*2022 06 15*/
					#endif
					
					#if defined (HMI_WINDOWS)
						LogTrace(_T("%d key presskey\n"),P_bt_status[i_msg].key);
					#endif
						break;
				case	HMI_SHORTKEY_OSD:
					#if HMI_EVENT_STAND_NUMBER > 0
						hmi_do_event(P_bt_status[i_msg].key,
									HMI_BUTTON_UP);/*2022 06 15*/
					#endif
					
					#if defined (HMI_WINDOWS)
						LogTrace(_T("%d key shortkey\n"),P_bt_status[i_msg].key);
					#endif
						break;
				case	HMI_LONGKEY_OSD:
					#if HMI_EVENT_STAND_NUMBER > 0
						hmi_do_event(P_bt_status[i_msg].key,
									HMI_BUTTON_LONG_PRESS);/*2022 06 15*/
					#endif
					
					#if defined (HMI_WINDOWS)
						LogTrace(_T("%d key longkey\n"),P_bt_status[i_msg].key);
					#endif
						break;
				case	HMI_LONG_RELEASEKEY_OSD:
					#if HMI_EVENT_STAND_NUMBER > 0
						hmi_do_event(P_bt_status[i_msg].key,
									HMI_BUTTON_UP);/*2022 06 15*/
					#endif
					
					#if defined (HMI_WINDOWS)
						LogTrace(_T("%d key longReleasekey\n"),P_bt_status[i_msg].key);
					#endif
						break;
				case	HMI_REPEATKEY_OSD:	
					#if HMI_EVENT_STAND_NUMBER > 0
						hmi_do_event(P_bt_status[i_msg].key,
									HMI_BUTTON_REPEAT);/*2022 06 15*/
					#endif
					
					#if defined (HMI_WINDOWS)
						LogTrace(_T("%d key repeatkey\n"),P_bt_status[i_msg].key);
					#endif
						break;
				case	HMI_REPEAT_RELEASEKEY_OSD:
					#if HMI_EVENT_STAND_NUMBER > 0
						hmi_do_event(P_bt_status[i_msg].key,
									HMI_BUTTON_UP);/*2022 06 15*/
					#endif
					
					#if defined (HMI_WINDOWS)
						LogTrace(_T("%d key repeatReleasekey\n"),P_bt_status[i_msg].key);
					#endif
						break;
				case	HMI_GESTURE_OSD:
						/*drag*/
						if((P_bt_status[i_msg].drag_position.x >= 0)&&(P_bt_status[i_msg].drag_position.y >= 0))
						{	
							P_bt_status[i_msg].drag_slide_type	= HMI_ONLY_DRAG_TYPE;
						#if defined (HMI_WINDOWS)
							LogTrace(_T("Only Drag at (x=%d,y=%d) ,direction=(%d,%d) %d key.\n"),										
										P_bt_status[i_msg].drag_position.x,
										P_bt_status[i_msg].drag_position.y,
										P_bt_status[i_msg].delta_s_speed.x,
										P_bt_status[i_msg].delta_s_speed.y,
										P_bt_status[i_msg].key);
						#endif

							/*Save last drag info for next slide*/
							hmi_multi_tp.last_drag_element		= P_bt_status[i_msg].key;
							hmi_multi_tp.last_drag_distance.x	= (float_32)(P_bt_status[i_msg].delta_s_speed.x);
							hmi_multi_tp.last_drag_distance.y	= (float_32)(P_bt_status[i_msg].delta_s_speed.y);
																																		
						}		
						else /*may be slide*/
						{
						}
			#ifdef	HMI_WINDOWS							
						/*scale*/
						if(P_bt_status[i_msg].scale >= 0.0f)
						{
						#if defined (HMI_WINDOWS)
							LogTrace(_T("%d key.scale center x=%d,y=%d. Scale=%f \n"),
										P_bt_status[i_msg].key,
										P_bt_status[i_msg].scale_center.x,P_bt_status[i_msg].scale_center.y,
										P_bt_status[i_msg].scale);
						#endif
							
						}
						/*rotation*/
						if(P_bt_status[i_msg].rotation_angle > HMI_ROTATION_INVALIDE_ANGLE)
						{
						#if defined (HMI_WINDOWS)
							LogTrace(_T("%d key.rotation center x=%d,y=%d. Rotation=%f \n"),
										P_bt_status[i_msg].key,
										P_bt_status[i_msg].rotation_center.x,P_bt_status[i_msg].rotation_center.y,
										P_bt_status[i_msg].rotation_angle);
						#endif
							
						}
			#endif						
						
						break;
				default:
					;	
			}
		}
	}
	if(get_key == FALSE)
	{					
		if((hmi_multi_tp.last_drag_element != HMI_NO_TOUCH_KEY)&&
			(hmi_multi_tp.tp_status == HMI_MULTI_TP_NO_TOUCH))
		{
			P_bt_status[1].button_type	= HMI_SLIDE_OSD;
			P_bt_status[1].key	= hmi_multi_tp.last_drag_element;
			P_bt_status[1].drag_position.x	= -1;
			P_bt_status[1].drag_position.y	= -1;

			P_bt_status[1].delta_s_speed.x	= (SINT32)(hmi_multi_tp.last_drag_distance.x);
			P_bt_status[1].delta_s_speed.y	= (SINT32)(hmi_multi_tp.last_drag_distance.y);
#ifdef	HMI_WINDOWS				
			if((P_bt_status[1].delta_s_speed.x != 0)||
				(P_bt_status[1].delta_s_speed.y != 0))
			{
				LogTrace(_T("Only Slide=(%d,%d). %d key.\n"),																				
							P_bt_status[1].delta_s_speed.x,
							P_bt_status[1].delta_s_speed.y,
							P_bt_status[1].key);
			}	
#endif			
		}
	}
	

}

/******************************************************************************
*
 *
 * Function:	PublicFunction
 *
 * Description: convert the physical coordinate to screen logic coordinate
 *				
 *				
 *
 * Invocation:	where this function is invoked
 *
 * Parameter:	parm1: parameter point coordinate pointer
 *				parm2: none
 *
 * Return:		none
 *				
 *
 
******************************************************************************/
#if(HMI_SUPPORT_CALIBRATION==HMI_YES) 
void Disp_TP_convert_xy(POINT_TP * p_point)
{
#if 0
    // SINT32 temp_long;     
	 
     // cal. X co-ordinate
     //p_point->x= 256 - p_point->x; //255 ????
     p_point->x = p_point->x * hmi_x_scale/100;
    // p_point->x = p_point->x / 100;
     p_point->x += hmi_x_offset;
     p_point->x -= 128;
     //p_point->x=p_point->x+10;// for test
     if (p_point->x < 0)
     {
	 	p_point->x = 0;
     }
     else if (p_point->x > HMI_MAX_WIDTH/*GFX_MAX_W_LENGTH*/)
     {

		p_point->x= HMI_MAX_WIDTH/*GFX_MAX_W_LENGTH*/;
     }
     //TP_X = (unsigned int) (temp_long & 0xffff);
     // cal. Y co-ordinate   
     p_point->y= 0x3ff - p_point->y; //jwang6
     p_point->y = p_point->y * hmi_y_scale/100;
    // p_point->y = p_point->y / 100;
     p_point->y += hmi_y_offset;
     p_point->y -= 128;
     if (p_point->y < 0)
     {
		p_point->y = 0;
     }
     else if (p_point->y > HMI_MAX_HEIGHT/*GFX_MAX_H_LENGTH*/)
     {
 		p_point->y = HMI_MAX_HEIGHT/*GFX_MAX_H_LENGTH*/;
     }
     // TP_Y = (unsigned int) (temp_long & 0xffff);
     // keep_touch_key_count = 0;
#endif	
}
#endif


/******************************************************************************
*
 *
 * Function:	PrivateFunction
 *
 * Description: init the touch panel calibrate parameters
 *				
 *				
 *
 * Invocation:	where this function is invoked
 *
 * Parameter:	parm1: none
 *				parm2: none
 *
 * Return:		none
 *				
 *
 
******************************************************************************/
#if(HMI_SUPPORT_CALIBRATION==HMI_YES) 
void hmi_calibate_para_init(void)
{
#if 0	
	UINT8 change_flag=0;
	UINT8 data_list[8];
	
	if ((hmi_x_scale <HMI_MINXSCALE) || (hmi_x_scale >HMI_MAXXSCALE))
	{
		hmi_x_scale = HMI_TP_X_SCALE_DEF;
		change_flag=1;
	}
	if ((hmi_y_scale <HMI_MINYSCALE) || (hmi_y_scale >HMI_MAXYSCALE)) 
	{
		hmi_y_scale = HMI_TP_Y_SCALE_DEF;
		change_flag=1;
	}
	if ((hmi_x_offset <HMI_MINXOFFSET) || (hmi_x_offset >HMI_MAXXOFFSET))     
      	{
		hmi_x_offset = HMI_TP_X_OFFSET_DEF;
		change_flag=1;
	}
	if ((hmi_y_offset < HMI_MINYOFFSET ) || (hmi_y_offset >HMI_MAXYOFFSET ))    
 	{
		hmi_y_offset = HMI_TP_Y_OFFSET_DEF;
		change_flag=1;
 	}
	if(change_flag==1)
	{
		data_list[0] = hmi_x_offset;   
		data_list[1] = hmi_x_offset>>8;
		data_list[2] = hmi_y_offset;
		data_list[3] = hmi_y_offset>>8;
		data_list[4] = hmi_x_scale;
		data_list[5] = hmi_x_scale>>8;
		data_list[6] = hmi_y_scale;
		data_list[7] = hmi_y_scale>>8;            
		//EEPWrite(EEP_ADDR_SYS_CAL_X_OFFSET,8, data_list);
	}
	else
	{
		;
	}
#endif	
}
#endif

static	BOOLEAN hmi_get_touch_xy(HMI_TOUCH_STATUS_TYPE *ptouch_status)
{
	BOOLEAN		success		= FALSE;
	U08			head		= 0U;
	
	if(ptouch_status != NULL)
	{		
		if(hmi_tp_xy.head != hmi_tp_xy.tail)/*not full*/
		{
			head						= hmi_tp_xy.head;
			ptouch_status->point.x		= hmi_tp_xy.touch_xy_list[head].touch_xy.x;
			ptouch_status->point.y		= hmi_tp_xy.touch_xy_list[head].touch_xy.y;
			ptouch_status->press		= hmi_tp_xy.touch_xy_list[head].press;
			ptouch_status->device_id	= hmi_tp_xy.touch_xy_list[head].device_id;

			head++;
			if(head >= HMI_TOUCH_MSG_LEN)
			{
				head	= 0U;
			}
			hmi_tp_xy.head				= head;
			success						= TRUE;
		}				
	}
	
	return success;
}

static UINT8	hmi_get_device_index(DWORD	device_ID)
{
	UINT8	device_ID_index			= 0U;
	UINT8	device_ID_index_free	= 0xff;
	UINT8	i						= 0U;
	BOOLEAN	finished				= FALSE;

	/*search exist*/
	for(i = 0U; (i < HMI_MULTI_TP_MAX_DEVICE)&&(finished == FALSE );i++)
	{
		if(hmi_device_id_index[i].bused == TRUE)
		{
			if(hmi_device_id_index[i].hmi_device_id	== device_ID)
			{
				device_ID_index							= i;
				finished								= TRUE;
			}
		}
		else
		{
			device_ID_index_free	=	i;
		}
	}
	/*Get new index*/
	if(finished == FALSE)
	{
		if(device_ID_index_free < 0xff)
		{			
			device_ID_index											= device_ID_index_free;
			hmi_device_id_index[device_ID_index_free].hmi_device_id	= device_ID;
			hmi_device_id_index[device_ID_index_free].bused 		= TRUE;
			finished												= TRUE;
		}
		

		/*not find free*/
		if(finished == FALSE)
		{
			hmi_device_id_index[0].hmi_device_id	= device_ID;
			hmi_device_id_index[0].bused 			= TRUE;
			device_ID_index							= 0u;
			/*set error*/
		}
	}

	return	device_ID_index;
}


void hmi_send_xy(DWORD	device_ID,UINT16 x, UINT16 y,UINT8 press)
{	
	UINT8	tail_next		= 0U;
	UINT8	device_ID_U8	= 0U;
	#if(HMI_SUPPORT_CALIBRATION==HMI_YES) 
	/*convert xy to screen coordination*/
	#endif
	
	device_ID_U8	= hmi_get_device_index(device_ID);
	
	tail_next	= hmi_tp_xy.tail + 1;
	if(tail_next >= HMI_TOUCH_MSG_LEN)
	{
		tail_next	= 0U;
	}
	if(tail_next != hmi_tp_xy.head)
	{		
		hmi_tp_xy.touch_xy_list[hmi_tp_xy.tail].touch_xy.x	= x;
		hmi_tp_xy.touch_xy_list[hmi_tp_xy.tail].touch_xy.y	= y;				
		hmi_tp_xy.touch_xy_list[hmi_tp_xy.tail].device_id	= device_ID_U8;
		hmi_tp_xy.touch_xy_list[hmi_tp_xy.tail].press		= press;	
		hmi_tp_xy.tail										= tail_next;
	}
	else
	{
		HMI_GFX_SET_STATUS(HMI_TOUCH_MSG_OVERFLOW);
	}
}


/*Multi point*/
static	void	hmi_init_multi_tp_status(BOOLEAN bclear_last_drag)
{
	U08		i	= 0U;
	
	hmi_multi_tp.tp_status	= HMI_MULTI_TP_NO_TOUCH;
	for(i = 0U;i < HMI_MULTI_TP_MAX_DEVICE;i++)
	{
		hmi_multi_tp.previous_points[i].x	= HMI_TP_INVALIDE_VALUE;
		hmi_multi_tp.previous_points[i].y	= HMI_TP_INVALIDE_VALUE;
		hmi_multi_tp.bprevious_active_device[i]= HMI_TOUCH_NO_ACTIVE;

		hmi_multi_tp.cur_points[i].x		= HMI_TP_INVALIDE_VALUE;
		hmi_multi_tp.cur_points[i].y		= HMI_TP_INVALIDE_VALUE;
		hmi_multi_tp.bcur_active_device[i]	= HMI_TOUCH_NO_ACTIVE;

		hmi_multi_tp.pfirst_points[i].x		= HMI_TP_INVALIDE_VALUE;
		hmi_multi_tp.pfirst_points[i].y		= HMI_TP_INVALIDE_VALUE;

		hmi_multi_tp.true_press_device		= HMI_TOUCH_NO_ACTIVE;
		hmi_multi_tp.true_press_key			= HMI_NO_TOUCH_KEY;

		if(bclear_last_drag == TRUE)
		{
			hmi_multi_tp.last_drag_distance.x	= 0;
			hmi_multi_tp.last_drag_distance.y	= 0;
			hmi_multi_tp.last_drag_element		= HMI_NO_TOUCH_KEY;
		}

		hmi_device_id_index[i].hmi_device_id= 0;
		hmi_device_id_index[i].bused		= FALSE;
		
	}
}

static	void	hmi_init_multi_tp_point(HMI_GESTURE_POINT_CLEAR_TYPE	points)
{
	U08		i	= 0U;

	if(points	== HMI_MULTI_TP_CURRENT_POINTS)
	{
		for(i = 0U;i < HMI_MULTI_TP_MAX_DEVICE;i++)
		{			
			hmi_multi_tp.cur_points[i].x		= HMI_TP_INVALIDE_VALUE;
			hmi_multi_tp.cur_points[i].y		= HMI_TP_INVALIDE_VALUE;
			hmi_multi_tp.bcur_active_device[i]	= HMI_TOUCH_NO_ACTIVE;
		}
	}
	else	if(points	== HMI_MULTI_TP_CURRENT_PREVIOUS_POINTS)
	{
		for(i = 0U;i < HMI_MULTI_TP_MAX_DEVICE;i++)
		{
			hmi_multi_tp.previous_points[i].x	= HMI_TP_INVALIDE_VALUE;
			hmi_multi_tp.previous_points[i].y	= HMI_TP_INVALIDE_VALUE;
			hmi_multi_tp.bprevious_active_device[i] = HMI_TOUCH_NO_ACTIVE;

			hmi_multi_tp.cur_points[i].x		= HMI_TP_INVALIDE_VALUE;
			hmi_multi_tp.cur_points[i].y		= HMI_TP_INVALIDE_VALUE;
			hmi_multi_tp.bcur_active_device[i]	= HMI_TOUCH_NO_ACTIVE;
		}
	}
	else if(points	== HMI_MULTI_TP_PREVIOUS_POINTS)
	{
		for(i = 0U;i < HMI_MULTI_TP_MAX_DEVICE;i++)
		{
			hmi_multi_tp.previous_points[i].x	= HMI_TP_INVALIDE_VALUE;
			hmi_multi_tp.previous_points[i].y	= HMI_TP_INVALIDE_VALUE;
			hmi_multi_tp.bprevious_active_device[i] = HMI_TOUCH_NO_ACTIVE;
		}
	}	
	else if(points	== HMI_MULTI_TP_FIRST_POINTS)
	{
		for(i = 0U;i < HMI_MULTI_TP_MAX_DEVICE;i++)
		{			
			hmi_multi_tp.pfirst_points[i].x		= HMI_TP_INVALIDE_VALUE;
			hmi_multi_tp.pfirst_points[i].y		= HMI_TP_INVALIDE_VALUE;			
		}
	}
	else
	{
	}
}

BOOLEAN	hmi_exist_device(U08	device_id,HMI_GESTURE_POINT_CLEAR_TYPE	points)
{
	BOOLEAN		exist	= FALSE;	
	
	if(points	== HMI_MULTI_TP_CURRENT_PREVIOUS_POINTS)
	{
		if(device_id < HMI_MULTI_TP_MAX_DEVICE)
		{
			if(hmi_multi_tp.bprevious_active_device[device_id] < HMI_TOUCH_NO_ACTIVE)
			{
				exist	= TRUE;
			}
			else if(hmi_multi_tp.bcur_active_device[device_id] < HMI_TOUCH_NO_ACTIVE)
			{
				exist	= TRUE;
			}
			else
			{
			}
		}
	}
	else if(points	== HMI_MULTI_TP_PREVIOUS_POINTS)
	{
		if(hmi_multi_tp.bprevious_active_device[device_id] < HMI_TOUCH_NO_ACTIVE)
		{
			exist	= TRUE;
		}
	}
	else if(points	== HMI_MULTI_TP_CURRENT_POINTS)
	{
		if((hmi_multi_tp.bcur_active_device[device_id] == HMI_TOUCH_PRESS)||
			(hmi_multi_tp.bcur_active_device[device_id] == HMI_TOUCH_MOVE))
		{
			exist	= TRUE;
		}
		else if(hmi_multi_tp.bcur_active_device[device_id] == HMI_TOUCH_UP)
		{
		}
		else	/*HMI_TOUCH_NO_ACTIVE*/
		{
			if((hmi_multi_tp.bprevious_active_device[device_id] == HMI_TOUCH_PRESS)||
				(hmi_multi_tp.bprevious_active_device[device_id] == HMI_TOUCH_MOVE))
			{
				exist	= TRUE;
			}
		}
	}
	else
	{
	}

	return		exist;
}


void	hmi_clear_device(U08	device_id,HMI_GESTURE_POINT_CLEAR_TYPE	points)
{			
	if(points	== HMI_MULTI_TP_CURRENT_PREVIOUS_POINTS)
	{
		if(device_id < HMI_MULTI_TP_MAX_DEVICE)
		{			
			hmi_multi_tp.bprevious_active_device[device_id] = HMI_TOUCH_NO_ACTIVE;								
			hmi_multi_tp.bcur_active_device[device_id] 		= HMI_TOUCH_NO_ACTIVE;
		}
	}
	else if(points	== HMI_MULTI_TP_PREVIOUS_POINTS)
	{
		if(device_id < HMI_MULTI_TP_MAX_DEVICE)
		{			
			hmi_multi_tp.bprevious_active_device[device_id] = HMI_TOUCH_NO_ACTIVE;
		}
	}
	else if(points	== HMI_MULTI_TP_CURRENT_POINTS)
	{
		if(device_id < HMI_MULTI_TP_MAX_DEVICE)
		{			
			hmi_multi_tp.bcur_active_device[device_id] 	= HMI_TOUCH_NO_ACTIVE;
		}
	}
	else
	{
	}

}


static	void	hmi_refresh_multi_tp_point(HMI_TOUCH_STATUS_TYPE	*ptouch_data,HMI_GESTURE_POINT_CLEAR_TYPE	points,U08 press)
{
	U08		device_id	= 0U;
	
	if(ptouch_data != NULL)
	{
		device_id	= ptouch_data->device_id;
		if(points	== HMI_MULTI_TP_CURRENT_PREVIOUS_POINTS)
		{
			if(device_id < HMI_MULTI_TP_MAX_DEVICE)
			{
				hmi_multi_tp.previous_points[device_id].x			= ptouch_data->point.x;
				hmi_multi_tp.previous_points[device_id].y			= ptouch_data->point.y;
				hmi_multi_tp.bprevious_active_device[device_id]		= press;

				hmi_multi_tp.cur_points[device_id].x				= ptouch_data->point.x;
				hmi_multi_tp.cur_points[device_id].y				= ptouch_data->point.y;				
				hmi_multi_tp.bcur_active_device[device_id]			= press;
			}
		}
		else if(points	== HMI_MULTI_TP_PREVIOUS_POINTS)
		{
			hmi_multi_tp.previous_points[device_id].x			= ptouch_data->point.x;
			hmi_multi_tp.previous_points[device_id].y			= ptouch_data->point.y;
			hmi_multi_tp.bprevious_active_device[device_id]		= press;
		}
		else if(points	== HMI_MULTI_TP_CURRENT_POINTS)
		{
			hmi_multi_tp.cur_points[device_id].x				= ptouch_data->point.x;
			hmi_multi_tp.cur_points[device_id].y				= ptouch_data->point.y;
			hmi_multi_tp.bcur_active_device[device_id]			= press;
		}
		else if(points	== HMI_MULTI_TP_FIRST_POINTS)
		{
			hmi_multi_tp.pfirst_points[device_id].x				= ptouch_data->point.x;
			hmi_multi_tp.pfirst_points[device_id].y				= ptouch_data->point.y;			
		}
		else
		{
		}
	}
}

static	U08	hmi_active_device_multi_tp_point(HMI_GESTURE_POINT_CLEAR_TYPE	points)
{
	U08		act_device_cnt	= 0U;
	U08		device_id		= 0U;
		
	if(points	== HMI_MULTI_TP_CURRENT_PREVIOUS_POINTS)
	{
		for(device_id	= 0U;device_id < HMI_MULTI_TP_MAX_DEVICE;device_id++)
		{
			if(hmi_multi_tp.bprevious_active_device[device_id] < HMI_TOUCH_NO_ACTIVE)
			{
				act_device_cnt++;
			}

			if(hmi_multi_tp.bcur_active_device[device_id] < HMI_TOUCH_NO_ACTIVE)
			{
				act_device_cnt++;
			}						
		}
	}
	else if(points	== HMI_MULTI_TP_PREVIOUS_POINTS)
	{
		for(device_id	= 0U;device_id < HMI_MULTI_TP_MAX_DEVICE;device_id++)
		{
			if(hmi_multi_tp.bprevious_active_device[device_id] < HMI_TOUCH_NO_ACTIVE)
			{
				act_device_cnt++;
			}
		}
	}
	else if(points	== HMI_MULTI_TP_CURRENT_POINTS)
	{
		for(device_id	= 0U;device_id < HMI_MULTI_TP_MAX_DEVICE;device_id++)
		{
			if((hmi_multi_tp.bcur_active_device[device_id] == HMI_TOUCH_PRESS)||
				(hmi_multi_tp.bcur_active_device[device_id] == HMI_TOUCH_MOVE))
			{
				act_device_cnt++;
			}
			else if(hmi_multi_tp.bcur_active_device[device_id] == HMI_TOUCH_UP)
			{
			}
			else
			{
				/*current point not change*/
				if((hmi_multi_tp.bprevious_active_device[device_id] == HMI_TOUCH_PRESS)||
					(hmi_multi_tp.bprevious_active_device[device_id] == HMI_TOUCH_MOVE))
				{
					act_device_cnt++;
				}
				
			}
		}
	}
	else
	{
	}

	return	act_device_cnt;
}


static	BOOLEAN	hmi_tp_no_touch_status(HMI_TOUCH_STATUS_TYPE	*ptouch_data)
{	
	BOOLEAN		finished	= FALSE;
	UINT8		press		= HMI_TOUCH_NO_ACTIVE;

	if(ptouch_data != NULL)
	{
		press	= ptouch_data->press;
		switch(press)
		{
			case HMI_TOUCH_PRESS:
				hmi_refresh_multi_tp_point(ptouch_data,
											HMI_MULTI_TP_FIRST_POINTS,
											HMI_TOUCH_PRESS);
					
				hmi_refresh_multi_tp_point(ptouch_data,
											HMI_MULTI_TP_CURRENT_PREVIOUS_POINTS,
											HMI_TOUCH_PRESS);
				
				hmi_multi_tp.tp_status	= HMI_MULTI_TP_FIRST_PRESS;				
				hmi_touch_status		= HMI_KEY_STATUS_FIRST_PRESS_OSD;
				HMI_SET_TIMER(HMI_PRESS_FIRST_TIMER);
				/*save info for true press key*/
				hmi_multi_tp.true_press_device		= ptouch_data->device_id;

				hmi_touch_button.delta_s_speed.x	= (SINT32)(ptouch_data->point.x);
				hmi_touch_button.delta_s_speed.y	= (SINT32)(ptouch_data->point.y);				
				/*stop slide set speed to 0*/
				hmi_multi_tp.last_drag_distance.x	= 0;
				hmi_multi_tp.last_drag_distance.x	= 0;
				hmi_multi_tp.last_drag_element		= HMI_NO_TOUCH_KEY;
				break;
			case HMI_TOUCH_UP:
				break;
			case HMI_TOUCH_MOVE:
				break;
			default:
				break;
		}
	}

	return	finished;
}

static	void	hmi_return_gesture(HMI_GESTURE_INFO_STR *pgesture_info,TOUCH_BUTTON_STR	*P_bt_status)
{
	POINT_TP				hit_point			= {0,0};
	BOOLEAN					hmi_is_button		= FALSE;
	HMI_OBJECT_ID_STR		hit_id_element		= HMI_NO_TOUCH_KEY;
	
	if((pgesture_info != NULL)&&(P_bt_status != NULL))
	{
		/*first check drag*/
		if((pgesture_info->begin_point.x >= 0)&&(pgesture_info->distance.y >= 0))			
		{	
			P_bt_status[1].button_type	= HMI_GESTURE_OSD;			
			if(hit_id_element == HMI_NO_TOUCH_KEY)
			{
				hit_point.x					= (UINT16)(pgesture_info->begin_point.x);
				hit_point.y					= (UINT16)(pgesture_info->begin_point.y);
				hit_id_element				= hmi_search_page(&hit_point,&hmi_is_button);
			}
			if(hit_id_element != HMI_NO_TOUCH_KEY)
			{
				P_bt_status[1].key				= hit_id_element;
				P_bt_status[1].drag_position.x	= hit_point.x;
				P_bt_status[1].drag_position.y	= hit_point.y;
				
				P_bt_status[1].delta_s_speed.x	= pgesture_info->distance.x;
				P_bt_status[1].delta_s_speed.y	= pgesture_info->distance.y;

				P_bt_status[1].drag_slide_type	= HMI_ONLY_DRAG_TYPE;				
			}
		}

		/*Check rotation*/
		if(pgesture_info->angle > HMI_ROTATION_INVALIDE_ANGLE)			
		{	
			P_bt_status[1].button_type	= HMI_GESTURE_OSD;							
			
			if(hit_id_element == HMI_NO_TOUCH_KEY)
			{
				hit_point.x			= (UINT16)(pgesture_info->rotation_begin_point.x);
				hit_point.y			= (UINT16)(pgesture_info->rotation_begin_point.y);
			
				hit_id_element		= hmi_search_page(&hit_point,&hmi_is_button);				
			}
			if(hit_id_element != HMI_NO_TOUCH_KEY)
			{
				P_bt_status[1].key					= hit_id_element;
				P_bt_status[1].rotation_center.x	= hit_point.x;
				P_bt_status[1].rotation_center.y	= hit_point.y;
				
				P_bt_status[1].rotation_angle		= pgesture_info->angle;		
			}
		}

		/*Check scale*/
		if(pgesture_info->scale >= 0.0f)
		{	
			P_bt_status[1].button_type	= HMI_GESTURE_OSD;	
			if(hit_id_element == HMI_NO_TOUCH_KEY)
			{
				hit_point.x					= (UINT16)(pgesture_info->scale_point.x);
				hit_point.y					= (UINT16)(pgesture_info->scale_point.y);
				hit_id_element				= hmi_search_page(&hit_point,&hmi_is_button);
			}
			if(hit_id_element != HMI_NO_TOUCH_KEY)
			{
				P_bt_status[1].key				= hit_id_element;
				P_bt_status[1].scale_center.x	= hit_point.x;
				P_bt_status[1].scale_center.y	= hit_point.y;
				
				P_bt_status[1].scale			= pgesture_info->scale;													
			}
		}
	}
}


static	BOOLEAN	hmi_tp_first_press_status(HMI_TOUCH_STATUS_TYPE	*ptouch_data,TOUCH_BUTTON_STR *P_bt_status)
{	
	BOOLEAN					finished			= FALSE;
	UINT8					press				= 0u;
	UINT8					action_device_cnt	= 0u;
	BOOLEAN					active_device		= FALSE;	
	HMI_GESTURE_INFO_STR	gesture_info		= {{0,0},{0,0},{0,0},{0,0},0.0f,0.0f,FALSE};

	if(ptouch_data != NULL)
	{
		press	= ptouch_data->press;
		switch(press)
		{
			case HMI_TOUCH_PRESS:
				active_device	= hmi_exist_device(ptouch_data->device_id,
									HMI_MULTI_TP_PREVIOUS_POINTS);
				if(active_device == FALSE)
				{
					hmi_refresh_multi_tp_point(ptouch_data,
										HMI_MULTI_TP_FIRST_POINTS,HMI_TOUCH_PRESS);
					hmi_refresh_multi_tp_point(ptouch_data,
										HMI_MULTI_TP_CURRENT_PREVIOUS_POINTS,HMI_TOUCH_PRESS);
				}
				else
				{
					hmi_refresh_multi_tp_point(ptouch_data,
										HMI_MULTI_TP_CURRENT_POINTS,HMI_TOUCH_PRESS);
				}
				
				hmi_multi_tp.tp_status	= HMI_MULTI_TP_GESTURE;
				hmi_touch_status		= HMI_KEY_STATUS_UNKNOWN_OSD;				
				break;
			case HMI_TOUCH_UP:
				hmi_refresh_multi_tp_point(ptouch_data,
								HMI_MULTI_TP_CURRENT_POINTS,HMI_TOUCH_UP);
							
				action_device_cnt	= hmi_active_device_multi_tp_point(HMI_MULTI_TP_CURRENT_POINTS);
				if(action_device_cnt == 0u)
				{	
					finished	= hmi_get_tp_gesture(&gesture_info,P_bt_status);
					if(finished == TRUE)
					{
						hmi_return_gesture(&gesture_info,P_bt_status);
					}
										
					hmi_multi_tp.tp_status	= HMI_MULTI_TP_NO_TOUCH;
					hmi_touch_status		= HMI_KEY_STATUS_IDLE_OSD;
				}
				
				break;	
			case HMI_TOUCH_MOVE:
				hmi_refresh_multi_tp_point(ptouch_data,
								HMI_MULTI_TP_CURRENT_POINTS,HMI_TOUCH_MOVE);				
				break;	
		}
	}

	return	finished;
}

static	BOOLEAN	hmi_tp_true_press_status(HMI_TOUCH_STATUS_TYPE	*ptouch_data,TOUCH_BUTTON_STR *P_bt_status)
{	
	BOOLEAN					finished			= FALSE;
	UINT8					press				= 0u;
	UINT8					action_device_cnt	= 0u;
	BOOLEAN					active_device		= FALSE;
	HMI_GESTURE_INFO_STR	gesture_info		= {0};	

	if(ptouch_data != NULL)
	{
		press	= ptouch_data->press;
		switch(press)
		{
			case HMI_TOUCH_PRESS:
				active_device	= hmi_exist_device(ptouch_data->device_id,
									HMI_MULTI_TP_PREVIOUS_POINTS);
				if(active_device == FALSE)
				{
					hmi_refresh_multi_tp_point(ptouch_data,
										HMI_MULTI_TP_FIRST_POINTS,HMI_TOUCH_PRESS);
					hmi_refresh_multi_tp_point(ptouch_data,
										HMI_MULTI_TP_CURRENT_PREVIOUS_POINTS,HMI_TOUCH_PRESS);
					/*true press to gesture, mean release press key*/
					hmi_touch_button.key	= HMI_NO_TOUCH_KEY;
					hmi_judge_buttontype(&hmi_touch_button,
									hmi_touch_button.bbutton,
									P_bt_status);
					hmi_multi_tp.tp_status	= HMI_MULTI_TP_GESTURE;
					hmi_touch_status		= HMI_KEY_STATUS_GESTURE_OSD;
					finished	= TRUE;
				}
				else
				{
					hmi_refresh_multi_tp_point(ptouch_data,
										HMI_MULTI_TP_CURRENT_POINTS,HMI_TOUCH_PRESS);
				}				
				break;
			case HMI_TOUCH_UP:
				hmi_refresh_multi_tp_point(ptouch_data,
								HMI_MULTI_TP_CURRENT_POINTS,HMI_TOUCH_UP);
				action_device_cnt	= hmi_active_device_multi_tp_point(HMI_MULTI_TP_CURRENT_POINTS);
				if(action_device_cnt == 0u)
				{	
					finished	= hmi_get_tp_gesture(&gesture_info,P_bt_status);
					if(finished == TRUE)
					{
						hmi_return_gesture(&gesture_info,P_bt_status);
					}
										
					hmi_multi_tp.tp_status	= HMI_MULTI_TP_NO_TOUCH;
					hmi_touch_status		= HMI_KEY_STATUS_IDLE_OSD;
				}
							
				break;	
			case HMI_TOUCH_MOVE:
				hmi_refresh_multi_tp_point(ptouch_data,
								HMI_MULTI_TP_CURRENT_POINTS,HMI_TOUCH_MOVE);				
				break;	
		}
	}

	return	finished;
}


static	BOOLEAN	hmi_tp_drag_status(HMI_TOUCH_STATUS_TYPE	*ptouch_data,TOUCH_BUTTON_STR *P_bt_status)
{	
	BOOLEAN					finished			= FALSE;
	UINT8					press				= 0u;
	UINT8					action_device_cnt	= 0u;
	BOOLEAN					exist_device		= FALSE;
	HMI_GESTURE_INFO_STR	gesture_info		= {0};

	if(ptouch_data != NULL)
	{
		press	= ptouch_data->press;
		switch(press)
		{
			case HMI_TOUCH_PRESS:
				hmi_refresh_multi_tp_point(ptouch_data,
						HMI_MULTI_TP_FIRST_POINTS,HMI_TOUCH_PRESS);
				hmi_refresh_multi_tp_point(ptouch_data,
						HMI_MULTI_TP_CURRENT_PREVIOUS_POINTS,HMI_TOUCH_PRESS);				
				break;
			case HMI_TOUCH_UP:					
				exist_device		= hmi_exist_device(ptouch_data->device_id,HMI_MULTI_TP_CURRENT_POINTS);
				hmi_refresh_multi_tp_point(ptouch_data,
						HMI_MULTI_TP_CURRENT_PREVIOUS_POINTS,HMI_TOUCH_UP);	
				action_device_cnt	= hmi_active_device_multi_tp_point(HMI_MULTI_TP_CURRENT_POINTS);

				/*all tp up*/
				if((action_device_cnt == 0)&&(exist_device == TRUE))
				{
					finished	= hmi_get_tp_gesture(&gesture_info,P_bt_status);
					if(finished == TRUE)
					{
						hmi_return_gesture(&gesture_info,P_bt_status);
					}
					hmi_init_multi_tp_status(FALSE/*not clear last drag info*/);					
					hmi_multi_tp.tp_status	= HMI_MULTI_TP_NO_TOUCH;
					hmi_touch_status		= HMI_KEY_STATUS_IDLE_OSD;									
				}
				break;	
			case HMI_TOUCH_MOVE:
				hmi_refresh_multi_tp_point(ptouch_data,
							HMI_MULTI_TP_CURRENT_POINTS,HMI_TOUCH_MOVE);				
				break;	
		}
	}

	return	finished;
}


static	BOOLEAN	hmi_tp_scale_status(HMI_TOUCH_STATUS_TYPE	*ptouch_data,TOUCH_BUTTON_STR *P_bt_status)
{	
	BOOLEAN					finished			= FALSE;
	UINT8					press				= 0u;
	UINT8					action_device_cnt	= 0u;
	BOOLEAN					exist_device		= FALSE;
	HMI_GESTURE_INFO_STR	gesture_info	= {0};

	if(ptouch_data != NULL)
	{
		press	= ptouch_data->press;
		switch(press)
		{
			case HMI_TOUCH_PRESS:
				hmi_refresh_multi_tp_point(ptouch_data,
						HMI_MULTI_TP_FIRST_POINTS,HMI_TOUCH_PRESS);
				hmi_refresh_multi_tp_point(ptouch_data,
						HMI_MULTI_TP_CURRENT_PREVIOUS_POINTS,HMI_TOUCH_PRESS);				
				break;
			case HMI_TOUCH_UP:	
				exist_device		= hmi_exist_device(ptouch_data->device_id,HMI_MULTI_TP_CURRENT_POINTS);
				hmi_refresh_multi_tp_point(ptouch_data,
						HMI_MULTI_TP_CURRENT_POINTS,HMI_TOUCH_UP);
				action_device_cnt	= hmi_active_device_multi_tp_point(HMI_MULTI_TP_CURRENT_POINTS);				
				/*All tp up*/
				if((action_device_cnt == 0)&&(exist_device == TRUE))
				{		
					finished	= hmi_get_tp_gesture(&gesture_info,P_bt_status);
					if(finished == TRUE)
					{
						hmi_return_gesture(&gesture_info,P_bt_status);
					}										
					hmi_multi_tp.tp_status	= HMI_MULTI_TP_NO_TOUCH;
					hmi_touch_status		= HMI_KEY_STATUS_IDLE_OSD;											
					hmi_init_multi_tp_status(FALSE/*not clear last drag info*/);					
				}
				break;	
			case HMI_TOUCH_MOVE:
				hmi_refresh_multi_tp_point(ptouch_data,
							HMI_MULTI_TP_CURRENT_POINTS,HMI_TOUCH_MOVE);				
				break;	
		}
	}

	return	finished;
}


static	BOOLEAN	hmi_tp_rotation_status(HMI_TOUCH_STATUS_TYPE	*ptouch_data,TOUCH_BUTTON_STR *P_bt_status)
{	
	BOOLEAN					finished			= FALSE;
	UINT8					press				= 0u;
	UINT8					action_device_cnt	= 0u;
	//BOOLEAN					exist_device		= FALSE;
	HMI_GESTURE_INFO_STR	gesture_info		= {0};

	if(ptouch_data != NULL)
	{
		press	= ptouch_data->press;
		switch(press)
		{
			case HMI_TOUCH_PRESS:
				hmi_refresh_multi_tp_point(ptouch_data,
						HMI_MULTI_TP_FIRST_POINTS,HMI_TOUCH_PRESS);
				hmi_refresh_multi_tp_point(ptouch_data,
						HMI_MULTI_TP_CURRENT_PREVIOUS_POINTS,HMI_TOUCH_PRESS);				
				break;
			case HMI_TOUCH_UP:	
				hmi_refresh_multi_tp_point(ptouch_data,
						HMI_MULTI_TP_CURRENT_POINTS,HMI_TOUCH_UP);
				action_device_cnt	= hmi_active_device_multi_tp_point(HMI_MULTI_TP_CURRENT_POINTS);				
				if(action_device_cnt == 0)
				{		
					finished	= hmi_get_tp_gesture(&gesture_info,P_bt_status);
					if(finished == TRUE)
					{
						hmi_return_gesture(&gesture_info,P_bt_status);
					}	
					hmi_multi_tp.tp_status	= HMI_MULTI_TP_NO_TOUCH;
					hmi_touch_status		= HMI_KEY_STATUS_IDLE_OSD;
					hmi_init_multi_tp_status(FALSE/*not clear last drag info*/);					
				}
				break;	
			case HMI_TOUCH_MOVE:
				hmi_refresh_multi_tp_point(ptouch_data,
						HMI_MULTI_TP_CURRENT_POINTS,HMI_TOUCH_MOVE);				
				break;	
		}
	}

	return	finished;
}


static	BOOLEAN	hmi_tp_gesture_status(HMI_TOUCH_STATUS_TYPE	*ptouch_data,TOUCH_BUTTON_STR *P_bt_status)
{		
	BOOLEAN					success				= FALSE;
	UINT8					press				= 0u;
	UINT8					action_device_cnt	= 0u;
	BOOLEAN					exist_device		= FALSE;
	BOOLEAN					finished			= FALSE;
	HMI_GESTURE_INFO_STR	gesture_info		= {0};
	
	if(ptouch_data != NULL)
	{
		press	= ptouch_data->press;
		switch(press)
		{
			case HMI_TOUCH_PRESS:
				hmi_refresh_multi_tp_point(ptouch_data,
					HMI_MULTI_TP_FIRST_POINTS,HMI_TOUCH_PRESS);
				hmi_refresh_multi_tp_point(ptouch_data,
					HMI_MULTI_TP_CURRENT_PREVIOUS_POINTS,HMI_TOUCH_PRESS);				
				break;
			case HMI_TOUCH_UP:	
				hmi_refresh_multi_tp_point(ptouch_data,
					HMI_MULTI_TP_CURRENT_POINTS,HMI_TOUCH_UP);
				action_device_cnt	= hmi_active_device_multi_tp_point(HMI_MULTI_TP_CURRENT_POINTS);				
				if(action_device_cnt == 0)
				{		
					finished	= hmi_get_tp_gesture(&gesture_info,P_bt_status);
					if(finished == TRUE)
					{
						hmi_return_gesture(&gesture_info,P_bt_status);
					}	
					hmi_multi_tp.tp_status	= HMI_MULTI_TP_NO_TOUCH;
					hmi_touch_status		= HMI_KEY_STATUS_IDLE_OSD;
					hmi_init_multi_tp_status(FALSE/*not clear last drag info*/);					
				}
				break;	
			case HMI_TOUCH_MOVE:
				hmi_refresh_multi_tp_point(ptouch_data,
							HMI_MULTI_TP_CURRENT_POINTS,HMI_TOUCH_MOVE);				
				break;	
		}

		/**/
	}

	return	success;
}


static	BOOLEAN	hmi_gesture_status(HMI_TOUCH_STATUS_TYPE	*ptouch_data,TOUCH_BUTTON_STR *P_bt_status)
{
	BOOLEAN		finished	= FALSE;
	
	switch(hmi_multi_tp.tp_status)
	{
		case	HMI_MULTI_TP_NO_TOUCH:
				finished	= hmi_tp_no_touch_status(ptouch_data);
				break;
		case	HMI_MULTI_TP_FIRST_PRESS:
				finished	= hmi_tp_first_press_status(ptouch_data,P_bt_status);
				break;
		case	HMI_MULTI_TP_TRUE_PRESS :
				finished	= hmi_tp_true_press_status(ptouch_data,P_bt_status);
				break;
		case	HMI_MULTI_TP_DRAG :
				finished	= hmi_tp_drag_status(ptouch_data,P_bt_status);
				break;
		case	HMI_MULTI_TP_SCALE:
				finished	= hmi_tp_scale_status(ptouch_data,P_bt_status);
				break;		
		case	HMI_MULTI_TP_ROTATION :
				finished	= hmi_tp_rotation_status(ptouch_data,P_bt_status);
				break;
		case	HMI_MULTI_TP_GESTURE:
				finished	= hmi_tp_gesture_status(ptouch_data,P_bt_status);
				break;
		default:
			;
	}
	
	
	return	finished;
}


#endif

