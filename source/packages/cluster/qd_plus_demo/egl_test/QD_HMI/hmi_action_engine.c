/*****************************************************************************

File Name        :  hmi_action_engine.c
Organization    :  Zhuli Electronics Co.Ltd in Shanghai (www.shzldz.com)
******************************************************************************/


#ifndef HMI_ACTION_ENGINE_C
#define HMI_ACTION_ENGINE_C

#include "hmi_all_struct_include.h"
#if	HMI_ALL_EVENT_NUMBER+HMI_ACTION_GROUP_NUMBER+HMI_ANIM_NUMBER>0
//#include "hmi_res_sprite.c"
#include "HMI_Data/hmi_action_data.rom"

HMI_OBJECT_ID_STR hmi_active_action_group_table[HMI_MAX_RUN_ACTION_CNT];
#if HMI_ACTION_GROUP_NUMBER > 0
 /*
   Flag to indicate whether a action_group first do
  */
 #define HMI_NB_ACTION_GROUP_BYTE				((HMI_ACTION_GROUP_NUMBER+7)/8)
 static UINT8 hmi_do_action_group_flag[HMI_NB_ACTION_GROUP_BYTE];
 #define HMI_SET_DO_ACTION_GROUP_FLAG(id)		(hmi_do_action_group_flag[((id)>>3u)]=(hmi_do_action_group_flag[((id)>>3u)]|((UINT8) ( 0x01u<<(UINT8)((id)&0x07u)))))
 #define HMI_GET_DO_ACTION_GROUP_FLAG(id)		(hmi_do_action_group_flag[((id)>>3u)]&((UINT8) ( 0x01u<<(UINT8)((id)&0x07u))))
 #define HMI_CANCEL_DO_ACTION_GROUP_FLAG(id)	(hmi_do_action_group_flag[((id)>>3u)]=(hmi_do_action_group_flag[((id)>>3u)]&((UINT8)(~(0x01u<<(UINT8)((id)&0x07u))))))
#endif

UINT32 get_bezier_length(UINT32 *pSum,UINT32 len,UINT32 nPoint,CONST POINT32_TP *pPointArray);

void hmi_get_bezier_index(HMI_TIME			transDt,
								UINT32 *pPoint,UINT32 actLen,
								U16				bezierSum,
								HMI_TIME		duration,
								U08				point_cnt,
								POINT32_TP	CONST	*pPoint_list,
								HMI_BEZIER_NEW_POS_STR *pbezier_new_pos);


#if HMI_ACTION_GROUP_NUMBER >0
void hmi_add_action_group(HMI_OBJECT_ID_STR action_gp_id,HMI_OBJECT_DATA_STR excute_status,BOOLEAN run);
void hmi_get_action_group(HMI_OBJECT_TABLE_STR CONST *P_event_rel);
#endif

#if HMI_ACTION_GROUP_NUMBER >0
BOOLEAN hmi_do_action_group(HMI_OBJECT_ID_STR action_group_id,HMI_TIME dt);
#endif

#if HMI_ALL_ACTION_NUMBER>0
void hmi_step(HMI_OBJECT_ID_STR action_id,HMI_TIME dt,HMI_OBJECT_ID_STR parent_timer_id) REENTRANT;
#endif

#if HMI_SET_ACTION_NUMBER>0
void hmi_do_set_action(HMI_OBJECT_ID_STR action_id,HMI_TIME dt,HMI_OBJECT_ID_STR parent_timer_id);
#endif

#if HMI_SET_POS_NUMBER>0
void hmi_set_pos_action(HMI_SET_POS_STR CONST *P_set_pos);
#endif

#if HMI_SET_DELTA_POS_NUMBER>0
void hmi_set_delta_pos_action(HMI_SET_DELTA_POS_STR  CONST *P_set_delta_pos);
#endif

#if HMI_SET_W_H_NUMBER>0
void hmi_set_w_h_action(HMI_SET_W_H_STR CONST *P_set_w_h);
#endif

#if HMI_SET_DELTA_W_H_NUMBER>0
void hmi_set_delta_w_h_action(HMI_SET_DELTA_W_H_STR CONST *P_set_delta_w_h);
#endif

#if HMI_SET_DYN_CONTAINER_NUMBER>0
void hmi_set_dyn_container_action(HMI_SET_DYN_CONTAINER_STR CONST *P_set_dyn_container);
#endif

#if HMI_SET_PAGE_ON_OFF_NUMBER>0
void hmi_set_page_on_off_action(HMI_SET_PAGE_ON_OFF_STR CONST *P_set_page_on_off);
#endif

#if HMI_SET_EDIT_TEXT_NUMBER>0
void hmi_set_edit_text_action(HMI_SET_EDIT_TEXT_STR CONST *P_set_edit_text);
#endif

#if HMI_SET_TEXT_SCROLL_STEP_NUMBER>0
void hmi_set_text_scroll_step_action(HMI_SET_TEXT_SCROLL_STEP_STR CONST *P_set_text_scroll_step);
#endif

#if HMI_SET_FOR_COLOR_NUMBER>0			
void hmi_set_foreground_color_action(HMI_SET_COLOR_STR CONST* P_set_for_color_prop_table);
#endif

#if HMI_SET_BCK_COLOR_NUMBER>0		
void hmi_set_background_color_action(HMI_SET_COLOR_STR CONST* P_set_bck_color_prop_table);
#endif
#if 0
#if HMI_SET_BTN_STATUS_NUMBER>0		
void hmi_set_button_status_action(HMI_SET_BTN_STATUS_STR CONST* P_set_button_status_prop_table);
#endif
#endif

#if HMI_ANIM_STATIC_SET_POS_NUMBER+HMI_ANIM_DYN_SET_POS_NUMBER>0
void hmi_anim_set_pos_action(HMI_OBJECT_ID_STR obj_index,BOOLEAN dyn_anim,HMI_TIME dt,
							HMI_OBJECT_ID_STR parent_timer_id);
#endif

#if HMI_ANIM_STATIC_SET_W_H_NUMBER+HMI_ANIM_DYN_SET_W_H_NUMBER>0
void hmi_anim_set_wh_action(HMI_OBJECT_ID_STR obj_index,BOOLEAN dyn_anim,HMI_TIME dt,
							HMI_OBJECT_ID_STR parent_timer_id);
#endif

#if HMI_ANIM_STATIC_SET_FOR_COLOR_NUMBER+HMI_ANIM_DYN_SET_FOR_COLOR_NUMBER>0
void hmi_anim_set_fcolor_action(HMI_OBJECT_ID_STR obj_index,BOOLEAN dyn_anim,HMI_TIME dt,
							HMI_OBJECT_ID_STR parent_timer_id);
#endif
#if HMI_ANIM_STATIC_SET_P1_NUMBER+HMI_ANIM_DYN_SET_P1_NUMBER>0
void hmi_anim_set_p1_action(HMI_OBJECT_ID_STR obj_index,BOOLEAN dyn_anim,HMI_TIME dt,
							HMI_OBJECT_ID_STR parent_timer_id);
#endif
#if HMI_ANIM_STATIC_SET_P2_NUMBER+HMI_ANIM_DYN_SET_P2_NUMBER>0
void hmi_anim_set_p2_action(HMI_OBJECT_ID_STR obj_index,BOOLEAN dyn_anim,HMI_TIME dt,
								HMI_OBJECT_ID_STR parent_timer_id);
#endif
#if HMI_ANIM_STATIC_SET_P3_NUMBER+HMI_ANIM_DYN_SET_P3_NUMBER>0
void hmi_anim_set_p3_action(HMI_OBJECT_ID_STR obj_index,BOOLEAN dyn_anim,HMI_TIME dt,
							HMI_OBJECT_ID_STR parent_timer_id);
#endif

#if HMI_ANIM_STATIC_SET_IMAGELIST_INDEX_NUMBER+HMI_ANIM_DYN_SET_IMAGELIST_INDEX_NUMBER>0
void hmi_anim_set_imglist_action(HMI_OBJECT_ID_STR obj_index,BOOLEAN dyn_anim,HMI_TIME dt,
							HMI_OBJECT_ID_STR parent_timer_id);
#endif

#if HMI_ANIM_STATIC_SET_ALPHA_NUMBER+HMI_ANIM_DYN_SET_ALPHA_NUMBER>0
#ifndef HMI_GRAPHIC_ST7513
void hmi_anim_set_alpha_action(HMI_OBJECT_ID_STR obj_index,BOOLEAN dyn_anim,HMI_TIME dt,
							HMI_OBJECT_ID_STR parent_timer_id);
#endif
#endif
#if HMI_ANIM_STATIC_SET_SCALE_NUMBER+HMI_ANIM_DYN_SET_SCALE_NUMBER>0
#ifndef HMI_GRAPHIC_ST7513 // RGL opengl
void hmi_anim_set_scale_action(HMI_OBJECT_ID_STR obj_index,BOOLEAN dyn_anim,HMI_TIME dt,
							HMI_OBJECT_ID_STR parent_timer_id);
#endif
#endif

#if HMI_ANIM_STATIC_SET_ANGEL_NUMBER+HMI_ANIM_DYN_SET_ANGEL_NUMBER>0
#ifndef HMI_GRAPHIC_ST7513
void hmi_anim_set_angle_action(HMI_OBJECT_ID_STR obj_index,BOOLEAN dyn_anim,HMI_TIME dt,
							HMI_OBJECT_ID_STR parent_timer_id);
#endif
#endif

#if HMI_SET_SEND_EVENT_NUMBER > 0		
void hmi_set_send_event_action(HMI_SET_SEND_EVENT_STR CONST* P_set_send_event_prop_table);
#endif

#if HMI_SET_CALL_FUNC_NUMBER > 0		
void hmi_set_call_function_action(HMI_SET_CALL_FUNC_STR CONST* P_set_call_func_prop_table);
#endif
#if HMI_SET_IMAGELIST_INDEX_NUMBER > 0
void hmi_set_imagelist_index_action(HMI_SET_RANGE_STR CONST* P_set_imagelist_index_prop_table);	
#endif
#if HMI_SET_SCROLLBAR_RANGE_NUMBER > 0
void hmi_set_scrollbar_range_action(HMI_SET_RANGE_STR CONST* P_set_scrollbar_range_prop_table);	
#endif
#if HMI_SET_BUTTON_STATUS_NUMBER > 0
void hmi_set_button_status_action(HMI_SET_RANGE_STR CONST* P_set_button_status_prop_table);
#endif

#if HMI_TIMER_ACTION_NUMBER > 0
void hmi_reset_timer_action(HMI_OBJECT_ID_STR action_id);
#endif

#if HMI_ACTION_GROUP_NUMBER>0 && HMI_TIMER_ACTION_NUMBER>0
void hmi_reset_action_group(HMI_OBJECT_ID_STR action_group_id);
#endif

#if HMI_ACTION_GROUP_NUMBER > 0
void hmi_remove_action_group(HMI_OBJECT_ID_STR action_gp_id);
#endif

#if (HMI_ALL_EVENT_NUMBER+HMI_ALL_ACTION_NUMBER+HMI_ACTION_GROUP_NUMBER)>0
HMI_EVENT_ACTION_RES_STR CONST* hmi_get_action_list_point(HMI_OBJECT_ID_STR action_id);
#endif
#if HMI_ANIM_DYN_SET_SCALE_NUMBER>0
#ifndef HMI_GRAPHIC_TWLIB
#ifndef HMI_GRAPHIC_ST7513
void hmi_set_dyn_scale_s_e(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data,BOOLEAN start);
#endif
#endif
#endif



/*
Init action group
*/
void hmi_action_manager_init(void)
{
	UINT8 loop = 0U;
	
	for(loop = 0;loop < HMI_MAX_RUN_ACTION_CNT;loop++)
	{
		hmi_active_action_group_table[loop] = 0U;
	}	
}

#if HMI_EVENT_STAND_NUMBER > 0
void hmi_do_event(HMI_OBJECT_ID_STR element_id,UINT8 press_status)
{
	HMI_EVENT_ACTION_RES_STR CONST  *P_default_event_data = hmi_get_action_list_point(HMI_EVENT_ACT_BEGIN_INDEX);
	
	if(P_default_event_data != NULL)
	{
		#if ((HMI_EVENT_STAND_PRESS_NUMBER+HMI_EVENT_STAND_UP_NUMBER+	\
			HMI_EVENT_STAND_LONG_PRESS_NUMBER+HMI_EVENT_REPEAT_NUMBER+	\
			HMI_EVENT_PAGE_ON_NUMBER+HMI_EVENT_PAGE_OFF_NUMBER) > 0)
		HMI_OBJECT_ID_STR				object_id	= 0U;
		HMI_STAND_EVENT_PROP_STR 		*P_default_event_prop	= (HMI_STAND_EVENT_PROP_STR *)(P_default_event_data->P_object_prop);
		#endif
		#if ((HMI_EVENT_STAND_PRESS_NUMBER+HMI_EVENT_STAND_UP_NUMBER+	\
			HMI_EVENT_STAND_LONG_PRESS_NUMBER+HMI_EVENT_REPEAT_NUMBER+	\
			HMI_EVENT_PAGE_ON_NUMBER+HMI_EVENT_PAGE_OFF_NUMBER+	\
			HMI_EVENT_STAND_RUN_NUMBER) > 0)
		UINT8  							event_index	= 0U;
		#if HMI_ACTION_GROUP_NUMBER >0
		HMI_OBJECT_TABLE_STR CONST 		*P_default_event_rel	= P_default_event_data->P_object_relative;
		#endif
		#endif
		switch(press_status)
		{
			case HMI_EVENT_RUN:
				#if HMI_EVENT_STAND_RUN_NUMBER > 0
				for(event_index=0;event_index < HMI_EVENT_STAND_RUN_NUMBER;event_index++)
				{
					#if HMI_ACTION_GROUP_NUMBER > 0
					hmi_get_action_group(&P_default_event_rel[event_index]);
					#endif
				}
				#endif
				break;
			case HMI_PROGRAM_CTRL:
				#if HMI_EVENT_PROG_CONTROL_NUMBER>0
				#endif
				break;
			case HMI_BUTTON_PRESS:
				#if HMI_EVENT_STAND_PRESS_NUMBER > 0
				for(event_index=0;event_index < HMI_EVENT_STAND_PRESS_NUMBER;event_index++)
				{
					object_id = P_default_event_prop[event_index+HMI_EVENT_STAND_PROG_CTRL_MAX_ID].element_id;
					if(object_id == element_id)
					{
						#if HMI_ACTION_GROUP_NUMBER >0
						hmi_get_action_group(&P_default_event_rel[event_index+HMI_EVENT_STAND_PROG_CTRL_MAX_ID]);
						#endif
					}
				}
				#endif
				break;
			case HMI_BUTTON_UP:
				#if HMI_EVENT_STAND_UP_NUMBER > 0
				for(event_index=0;event_index < HMI_EVENT_STAND_UP_NUMBER;event_index++)
				{
					object_id = P_default_event_prop[event_index+HMI_EVENT_STAND_PRESS_MAX_ID].element_id;
					if(object_id == element_id)
					{
						#if HMI_ACTION_GROUP_NUMBER > 0
						hmi_get_action_group(&P_default_event_rel[event_index+HMI_EVENT_STAND_PRESS_MAX_ID]);
						#endif
					}
				}
				#endif
				break;
			case HMI_BUTTON_LONG_PRESS:
				#if HMI_EVENT_STAND_LONG_PRESS_NUMBER > 0
				for(event_index=0;event_index < HMI_EVENT_STAND_LONG_PRESS_NUMBER;event_index++)
				{
					object_id = P_default_event_prop[event_index+HMI_EVENT_STAND_UP_MAX_ID].element_id;
					if(object_id == element_id)
					{
						#if HMI_ACTION_GROUP_NUMBER >0
						hmi_get_action_group(&P_default_event_rel[event_index+HMI_EVENT_STAND_UP_MAX_ID]);
						#endif
					}
				}
				#endif
				break;
			case HMI_BUTTON_REPEAT:
				#if HMI_EVENT_REPEAT_NUMBER>0
				for(event_index=0;event_index<HMI_EVENT_REPEAT_NUMBER;event_index++)
				{
					object_id=P_default_event_prop[event_index+HMI_EVENT_STAND_LONG_PRESS_MAX_ID].element_id;
					if(object_id==element_id)
					{
						#if HMI_ACTION_GROUP_NUMBER >0
						hmi_get_action_group(&P_default_event_rel[event_index+HMI_EVENT_STAND_LONG_PRESS_MAX_ID]);
						#endif
					}
				}
				#endif
				break;
			case HMI_PAGE_ON:
				#if HMI_EVENT_PAGE_ON_NUMBER>0
				for(event_index=0;event_index<HMI_EVENT_PAGE_ON_NUMBER;event_index++)
				{
					object_id=P_default_event_prop[event_index+HMI_EVENT_STAND_REPEAT_MAX_ID].element_id;
					if(object_id==element_id)
					{
						#if HMI_ACTION_GROUP_NUMBER >0
						hmi_get_action_group(&P_default_event_rel[event_index+HMI_EVENT_STAND_REPEAT_MAX_ID]);
						#endif
					}
				}
				#endif
				break;
			case HMI_PAGE_OFF:
				#if HMI_EVENT_PAGE_OFF_NUMBER>0
				for(event_index=0;event_index<HMI_EVENT_PAGE_OFF_NUMBER;event_index++)
				{
					object_id=P_default_event_prop[event_index+HMI_EVENT_STAND_PAGE_ON_MAX_ID].element_id;
					if(object_id==element_id)
					{
						#if HMI_ACTION_GROUP_NUMBER >0
						hmi_get_action_group(&P_default_event_rel[event_index+HMI_EVENT_STAND_PAGE_ON_MAX_ID]);
						#endif
					}
				}
				#endif
				break;
			default:
				break;
		}
	}
}
#endif
#if HMI_ALL_EVENT_NUMBER>0
void hmi_action_send_event(HMI_OBJECT_ID_STR event_id)
{
	HMI_EVENT_ACTION_RES_STR CONST  *P_event_data = hmi_get_action_list_point(event_id);
	
	if(P_event_data != NULL)
	{
		#if HMI_ACTION_GROUP_NUMBER > 0
		HMI_OBJECT_TABLE_STR CONST 	*P_event_rel = P_event_data->P_object_relative;
		#endif
		if(IS_EVENT(event_id))
		{
			event_id = HMI_GET_EVENT_ID_INDEX(event_id);
			#if HMI_EVENT_STAND_MAX_ID >0
			if(event_id < HMI_EVENT_STAND_MAX_ID)
			{
				#if HMI_ACTION_GROUP_NUMBER > 0
				hmi_get_action_group(&P_event_rel[event_id]);
				#endif
			}
			else
			#endif
			if(event_id < HMI_EVENT_CUSTOM_MAX_ID)
			{
				event_id = HMI_GET_EVENT_CUSTOM_ID_INDEX(event_id);
				#if HMI_ACTION_GROUP_NUMBER >0
				hmi_get_action_group(&P_event_rel[event_id]);
				#endif
			}
			else
			{

			}
		}
	}
}
#endif
#if HMI_ACTION_GROUP_NUMBER>0
void hmi_get_action_group(HMI_OBJECT_TABLE_STR CONST *P_event_rel)
{
	UINT8 loop = 0U;
	
	for(loop=0;loop < P_event_rel->object_number/*action_group_number*/;loop++)
	{
		hmi_add_action_group((P_event_rel->p_object_table)[loop].object_id,HMI_ACTION_RUN,FALSE);
	}
}

void  hmi_add_action_group(HMI_OBJECT_ID_STR action_gp_id,HMI_OBJECT_DATA_STR excute_status,BOOLEAN run)
{	
	UINT8	index_reference	= 0U;
	BOOLEAN success			= FALSE;
	BOOLEAN action_active	= FALSE;
	HMI_OBJECT_ID_STR action_group_index = 0U;
	
	if((action_gp_id >= HMI_EVENT_MAX_ID)&&(action_gp_id < HMI_ACTION_GROUP_MAX_ID))
	{
		while( (index_reference < HMI_MAX_RUN_ACTION_CNT)&&(action_active == FALSE))
		{
			if(hmi_active_action_group_table[index_reference] == action_gp_id)
			{
				action_active = TRUE;
			}
			else
			{
				index_reference++;
			}
			
		}
		index_reference = 0;
		while((index_reference < HMI_MAX_RUN_ACTION_CNT)&&
			(success == FALSE)&&(action_active == FALSE))
		{
			if(hmi_active_action_group_table[index_reference] == 0U)
			{
				hmi_active_action_group_table[index_reference] = action_gp_id;
				success = TRUE;
			}
			else
			{
				index_reference++;
			}
		}
	}
	if((index_reference >= HMI_MAX_RUN_ACTION_CNT)&&
		(success == FALSE)&&(action_active == FALSE))
	{
		HMI_GFX_SET_STATUS(HMI_ACTION_GROUP_TABLE_OVERFLOW);/*set error flag*/
	}
	/*when user set action group status is HMI_ACTION_RUN,but action group is active,also need cancel action group flag*/
	if((action_active == TRUE)&&(excute_status == HMI_ACTION_RUN)&&
		(run == TRUE))
	{
		success = TRUE;
	}
	if((success == TRUE)&&(excute_status == HMI_ACTION_RUN))
	{
		action_group_index = HMI_GET_ACTION_GROUP_ID_INDEX(action_gp_id);
		#if HMI_ACTION_GROUP_NUMBER>0 && HMI_TIMER_ACTION_NUMBER>0
		hmi_reset_action_group(action_gp_id);
		#endif
		HMI_CANCEL_DO_ACTION_GROUP_FLAG(action_group_index);
	}		
}
#endif

#if HMI_ACTION_GROUP_NUMBER>0 && HMI_TIMER_ACTION_NUMBER>0
void hmi_reset_action_group(HMI_OBJECT_ID_STR action_group_id)
{
	static HMI_EVENT_ACTION_RES_STR 	CONST 	*P_action_group	= NULL;	
	UINT8 										loop			= 0U;
	UINT8 										nb_child		= 0U;
    HMI_OBJECT_PROP_STR 				CONST 	*P_child_t		= NULL;
	
	if((action_group_id >= HMI_EVENT_MAX_ID)&&(action_group_id < HMI_ACTION_GROUP_MAX_ID))
	{
		if(P_action_group == NULL)
		{
			P_action_group = hmi_get_action_list_point(action_group_id);
		}
		if(P_action_group != NULL)
		{			
			action_group_id = HMI_GET_ACTION_GROUP_ID_INDEX(action_group_id);
			if(action_group_id < HMI_ACTION_GROUP_NUMBER)
			{
				nb_child	= (P_action_group->P_object_relative)[action_group_id].object_number;
				P_child_t	= (P_action_group->P_object_relative)[action_group_id].p_object_table;
				for(loop=0U;loop < nb_child;loop++)
				{
					hmi_reset_timer_action(P_child_t[loop].object_id);
				}
			}
		}
	}
}
#endif


void hmi_action_manager(HMI_TIME dt)
{
#if HMI_ACTION_GROUP_NUMBER > 0
	UINT8				loop		= 0;
	BOOLEAN				finished	= FALSE;
	HMI_OBJECT_ID_STR	action_group_index=0U;
	
  	for(loop=0;loop < HMI_MAX_RUN_ACTION_CNT;loop++)
	{
		if(hmi_active_action_group_table[loop] != 0U)
		{
			action_group_index	= HMI_GET_ACTION_GROUP_ID_INDEX(hmi_active_action_group_table[loop]);
			finished			= hmi_do_action_group(hmi_active_action_group_table[loop],dt);
			if(action_group_index < HMI_ACTION_GROUP_NUMBER)
			{
				HMI_SET_DO_ACTION_GROUP_FLAG(action_group_index);
			}
			if(finished)
			{
				hmi_active_action_group_table[loop] = 0U;
			}
		}
	}
#endif
}

#if 0
typedef enum
{
	HMI_DT_START,
	HMI_DT_DUR,
	HMI_DT_ELAPSE,
	HMI_DT_DT,
	/*GET_TRANS_DT_CNT*/
}GET_TRANS_DT_STR;
#endif

//#if HMI_ANIM_NUMBER > 0
HMI_TIME get_trans_dt(HMI_TIME start_time,MOTION_TYPE_STR motion,
						HMI_TIME dur_time,HMI_TIME elapse_time,HMI_TIME dt)
{		
	HMI_TIME cur_time_next_time	= 0;	
	HMI_TIME mid_time			= start_time;
	HMI_TIME end_time			= 0;
	
	if(motion == HMI_LINEAR_ANIMATION )
	{		
		dt = elapse_time - start_time;		
	}
	else
	{
		end_time = start_time + dur_time;		
		cur_time_next_time = elapse_time;
		/*cur_time_next_time = cur_time_next_time+dt;lq 2017.4.3. elapse already is elapse+dt*/
		if(cur_time_next_time > end_time)
		{
			cur_time_next_time = end_time;
		}
		if(motion == HMI_SLOW_FAST_ANIMATION )
		{			
			elapse_time = (end_time-start_time)/((end_time-start_time)*(end_time-start_time));		
			cur_time_next_time-=start_time;
			cur_time_next_time = elapse_time*cur_time_next_time*cur_time_next_time+start_time;
			dt = cur_time_next_time-start_time;
		}
		else if(motion == HMI_FAST_SLOW_ANIMATION )
		{			
			elapse_time = (start_time-end_time)/((start_time-end_time)*(start_time-end_time));		
			cur_time_next_time-=end_time;
			cur_time_next_time = elapse_time*cur_time_next_time*cur_time_next_time+end_time;
			dt = cur_time_next_time-start_time;
		}
		else if(motion==HMI_F_S_F_ANIMATION)
		{
			mid_time = start_time+dur_time/2;
			if(cur_time_next_time > mid_time)
			{				
				elapse_time = (end_time-mid_time)/((end_time-mid_time)*(end_time-mid_time));		
				cur_time_next_time-=mid_time;
				cur_time_next_time = elapse_time*cur_time_next_time*cur_time_next_time+mid_time;
				dt = cur_time_next_time-start_time;
			}
			else
			{				
				elapse_time = (start_time-mid_time)/((start_time-mid_time)*(start_time-mid_time));		
				cur_time_next_time-=mid_time;
				cur_time_next_time = elapse_time*cur_time_next_time*cur_time_next_time+mid_time;
				dt = cur_time_next_time - start_time;
			}			
		}
		else if(motion == HMI_S_F_S_ANIMATION )
		{
			mid_time = start_time+dur_time/2;
			if(cur_time_next_time > mid_time)
			{				
				elapse_time = (mid_time-end_time)/((mid_time-end_time)*(mid_time-end_time));		
				cur_time_next_time-=end_time;
				cur_time_next_time = elapse_time*cur_time_next_time*cur_time_next_time+end_time;
				dt = cur_time_next_time-start_time;
			}
			else
			{				
				elapse_time = (mid_time-start_time)/((mid_time-start_time)*(mid_time-start_time));		
				cur_time_next_time-=start_time;
				cur_time_next_time = elapse_time*cur_time_next_time*cur_time_next_time+start_time;
				dt = cur_time_next_time-start_time;
			}			
		}
		else
		{			
			elapse_time = (end_time-start_time)/((end_time-start_time)*(end_time-start_time));		
			cur_time_next_time-=start_time;
			cur_time_next_time = elapse_time*cur_time_next_time*cur_time_next_time+start_time;
			dt = cur_time_next_time-start_time;
		}
		
	}
	
	return dt;
}


#if 0
HMI_TIME get_trans_dt(HMI_TIME start_time,MOTION_TYPE_STR motion,
						HMI_TIME dur_time,HMI_TIME elapse_time,HMI_TIME dt)
{
	//HMI_TIME cur_time=0;	
	HMI_TIME cur_time_next_time=0;	
	HMI_TIME mid_time=start_time;
	HMI_TIME trans_dt=dt;
	HMI_TIME end_time=0;
	HMI_TIME ver_xy=0;
	HMI_TIME no_ver_xy=0;
	HMI_TIME a=0;

	if(motion==HMI_LINEAR_ANIMATION )
	{		
		trans_dt=elapse_time-start_time;		
	}
	else
	{
		end_time=start_time+dur_time;		
		cur_time_next_time=elapse_time;
		cur_time_next_time=cur_time_next_time+dt;
		if(cur_time_next_time>end_time)
		{
			cur_time_next_time=end_time;
		}
		if(motion==HMI_SLOW_FAST_ANIMATION )
		{
			ver_xy=start_time;
			no_ver_xy=end_time;			
		}
		else if(motion==HMI_FAST_SLOW_ANIMATION )
		{
			ver_xy=end_time;
			no_ver_xy=start_time;
		}
		else if(motion==HMI_F_S_F_ANIMATION)
		{
			mid_time=start_time+dur_time/2;
			if(cur_time_next_time>mid_time)
			{
				ver_xy=mid_time;
				no_ver_xy=end_time;
			}
			else
			{
				ver_xy=mid_time;
				no_ver_xy=start_time;
			}
		}
		else if(motion==HMI_S_F_S_ANIMATION )
		{
			mid_time=start_time+dur_time/2;
			if(cur_time_next_time>mid_time)
			{
				ver_xy=end_time;;
				no_ver_xy=mid_time;
			}
			else
			{
				ver_xy=start_time;
				no_ver_xy=mid_time;
			}
		}
		else
		{
			ver_xy=start_time;
			no_ver_xy=end_time;
		}
		a=(no_ver_xy-ver_xy)/((no_ver_xy-ver_xy)*(no_ver_xy-ver_xy));		
		cur_time_next_time-=ver_xy;
		cur_time_next_time=a*cur_time_next_time*cur_time_next_time+ver_xy;
		trans_dt=cur_time_next_time-start_time;
	}
	
	return trans_dt;
}
#endif

BOOLEAN get_next_step_animfloat(POINT_FLOAT_TP *pPos /*POINT_FLOAT_TP *pnext_pos,POINT_FLOAT_TP *pstart,POINT_FLOAT_TP *pend*/,
									HMI_TIME tran_dt,HMI_TIME dur_time)
{	
	BOOLEAN			finished=FALSE;	
	float_32		next_dis_seq=0;	
	POINT_FLOAT_TP		av_speed_dis_next_dis={0,0};
	float_32		cur_dis_seq=0;
	
	if(pPos!=NULL)	
	{													
		HMI_SUB_POINT2(pPos[HMI_END_POS],pPos[HMI_START_POS],av_speed_dis_next_dis);		
		if(dur_time!=0)										
		{													
			HMI_DIV_POINT(dur_time,av_speed_dis_next_dis);			
		}												
		else											
		{												
			av_speed_dis_next_dis.x=HMI_AV_SPEED;					
			av_speed_dis_next_dis.y=HMI_AV_SPEED;					
		}												
		HMI_MUL_POINT(tran_dt,av_speed_dis_next_dis);				
		HMI_ADD_POINT2(pPos[HMI_START_POS],av_speed_dis_next_dis,pPos[HMI_NEXT_POS]);	
		HMI_SUB_POINT2(pPos[HMI_END_POS],pPos[HMI_NEXT_POS],av_speed_dis_next_dis);							
		if((av_speed_dis_next_dis.x<=HMI_FLOAT_DISTANCE)&&(av_speed_dis_next_dis.x>=(-HMI_FLOAT_DISTANCE))&&	
			(av_speed_dis_next_dis.y<=HMI_FLOAT_DISTANCE)&&(av_speed_dis_next_dis.y>=(-HMI_FLOAT_DISTANCE)))	
		{	
			HMI_DIS_SEQ(av_speed_dis_next_dis,next_dis_seq);	
			if(next_dis_seq<HMI_FLOAT_DISTANCE_SEQ)								
			{															
				HMI_CPY_POINT((pPos[HMI_NEXT_POS]),(pPos[HMI_END_POS]));
				finished=TRUE;
			}																
		}	
		if(!finished)
		{
			HMI_SUB_POINT2((pPos[HMI_END_POS]),(pPos[HMI_START_POS]),av_speed_dis_next_dis);
			HMI_DIS_SEQ(av_speed_dis_next_dis,cur_dis_seq);	
			HMI_SUB_POINT2((pPos[HMI_NEXT_POS]),(pPos[HMI_START_POS]),av_speed_dis_next_dis);
			HMI_DIS_SEQ(av_speed_dis_next_dis,next_dis_seq);	
			if(next_dis_seq>cur_dis_seq)
			{
				HMI_CPY_POINT((pPos[HMI_NEXT_POS]),(pPos[HMI_END_POS]));	
			}
		}		
		finished=TRUE;													
	}		
	return finished;
}


BOOLEAN get_next_step_anim32(POINT32_TP *pPos,
									HMI_TIME tran_dt,HMI_TIME dur_time)
{	
	BOOLEAN			finished	= FALSE;
	POINT_FLOAT_TP	av_speed	= {0,0};	
	POINT32_TP		distance_next_distance = {0,0};
	UINT32_T		next_dis_seq= 0;	
	UINT32_T		cur_dis_seq	= 0;
	
	if(pPos != NULL)	
	{													
		HMI_SUB_POINT2((pPos[HMI_END_POS] ),(pPos[HMI_START_POS ]),distance_next_distance/*distance*/);		
		if(dur_time != 0)										
		{													
			HMI_DIV_POINT3(dur_time,distance_next_distance/*distance*/,av_speed);			
		}												
		else											
		{												
			av_speed.x	= HMI_AV_SPEED;					
			av_speed.y	= HMI_AV_SPEED;					
		}												
		HMI_MUL_POINT(tran_dt,av_speed);				
		HMI_ADD_POINT2_F2I(pPos[HMI_START_POS],av_speed,pPos[HMI_NEXT_POS]);	
		HMI_SUB_POINT2(pPos[HMI_END_POS],pPos[HMI_NEXT_POS],distance_next_distance);						
		if((distance_next_distance.x <= HMI_DISTANCE)&&(distance_next_distance.x >= (-HMI_DISTANCE))&&	
			(distance_next_distance.y <= HMI_DISTANCE)&&(distance_next_distance.y >= (-HMI_DISTANCE)))	
		{	
			HMI_DIS_SEQ(distance_next_distance,next_dis_seq);	
			if(next_dis_seq < HMI_DISTANCE_SEQ)								
			{															
				HMI_CPY_POINT((pPos[HMI_NEXT_POS ]),(pPos[HMI_END_POS  ]));
				finished	= TRUE;
			}																
		}	
		if(!finished)
		{
			HMI_SUB_POINT2((pPos[HMI_END_POS]),(pPos[HMI_START_POS ]),distance_next_distance/*cur_distance*/);
			HMI_DIS_SEQ(distance_next_distance/*cur_distance*/,cur_dis_seq);	
			HMI_SUB_POINT2(pPos[HMI_NEXT_POS],pPos[HMI_START_POS],distance_next_distance);
			HMI_DIS_SEQ(distance_next_distance,next_dis_seq);	
			if(next_dis_seq > cur_dis_seq)
			{
				HMI_CPY_POINT(pPos[HMI_NEXT_POS],pPos[HMI_END_POS]);	
			}
		}		
		finished	= TRUE;													
	}	
	return finished;
}


//#endif

#if HMI_ACTION_GROUP_NUMBER >0
BOOLEAN hmi_do_action_group(HMI_OBJECT_ID_STR action_group_id,HMI_TIME dt)	
{
	BOOLEAN				finished		= FALSE;	
	UINT8				loop			= 0;
	UINT8				action_do_flag	= 0U;
	HMI_OBJECT_ID_STR	child_action_id	= 0U;	
	BOOLEAN				timer_finished	= FALSE;
	BOOLEAN				search_stop_flag= FALSE;
	UINT8				nb_group_child	= 0U;
	HMI_OBJECT_PROP_STR CONST	*P_group_child = NULL;
				
	if((action_group_id >= HMI_EVENT_MAX_ID)&&(action_group_id < HMI_ACTION_GROUP_MAX_ID))
	{
		action_group_id	= HMI_GET_ACTION_GROUP_ID_INDEX(action_group_id);
		if(action_group_id < HMI_ACTION_GROUP_NUMBER)
		{
			action_do_flag	= HMI_GET_DO_ACTION_GROUP_FLAG(action_group_id);
			nb_group_child	= hmi_action_group_relative_table[action_group_id].object_number;
			P_group_child	= hmi_action_group_relative_table[action_group_id].p_object_table;
			for(loop=0;loop < nb_group_child;loop++)
			{
				if(action_do_flag == 0U)
				{
					#if HMI_ALL_ACTION_NUMBER>0
					hmi_step(P_group_child[loop].object_id,dt,HMI_NB_ELEMENTS/*No parent*/);
					#endif
				}
				else
				{
					child_action_id	= HMI_GET_ACTION_ID_INDEX(P_group_child[loop].object_id);
					#if HMI_TIMER_ACTION_NUMBER > 0
					if((child_action_id < HMI_TIMER_ACTION_MAX_ID))
					{
						hmi_step(P_group_child[loop].object_id,dt,HMI_NB_ELEMENTS/*No parent*/);
					}
					#endif
				}
			}
			/*check finished*/
			loop = 0U;
			while((loop < nb_group_child)&&(search_stop_flag == FALSE))
			{
				child_action_id = HMI_GET_ACTION_ID_INDEX(P_group_child[loop].object_id);
				#if HMI_TIMER_ACTION_NUMBER > 0
				if((child_action_id < HMI_TIMER_ACTION_MAX_ID))
				{
					#if HMI_TIMER_ACTION_REPEAT_NUMBER>0
					if(child_action_id < HMI_TIMER_ACTION_REPEAT_MAX_ID)
					{
						search_stop_flag = TRUE;
					}
					else
					#endif
					#if HMI_TIMER_ACTION_ONETIME_NUMBER>0
					if(child_action_id < HMI_TIMER_ACTION_ONETIME_MAX_ID)
					{
						child_action_id = HMI_GET_TIMER_ACTION_ONETIME_ID_INDEX(child_action_id);
						{
							if(child_action_id < HMI_TIMER_ACTION_ONETIME_NUMBER)
							{
								#if 0
								elapse=hmi_onetime_elapse_prop[child_action_id];
								start=hmi_onetime_start_prop[child_action_id];
								#endif
								if(hmi_onetime_elapse_prop[child_action_id] > hmi_onetime_start_prop[child_action_id])
								{
									timer_finished=TRUE;
								}
								else
								{
									timer_finished=FALSE;
								}
							}
							/*oneTimeFinished=(BOOLEAN)(elapse>start);*/
							#if 0
							if(elapse>start)
							{
								timer_finished=TRUE;
							}
							else
							{
								timer_finished=FALSE;
							}
							#endif
							if(timer_finished)
							{
								loop++;
							}
							else
							{
								search_stop_flag=TRUE;
							}
						}
					}
					else
					#endif
					#if HMI_TIMER_ACTION_DURATION_NUMBER>0
					if(child_action_id<HMI_TIMER_ACTION_DURATION_MAX_ID)
					{
						if(child_action_id<HMI_TIMER_ACTION_S_DURATION_MAX_ID)

						{
							child_action_id=HMI_GET_TIMER_ACTION_S_DURATION_ID_INDEX(child_action_id);
							if(child_action_id<HMI_S_TIMER_ACTION_DURATION_NUMBER)
							{
								#if 0
								elapse=hmi_duration_elapse_prop[child_action_id];
								start=hmi_duration_static_prop[child_action_id].start;
								duration=hmi_duration_static_prop[child_action_id].duration;
								#endif 
								#if HMI_S_TIMER_ACTION_DURATION_NUMBER >0
								if(hmi_duration_elapse_prop[child_action_id]>
									(hmi_duration_static_prop[child_action_id].start+hmi_duration_static_prop[child_action_id].duration))
								{
									timer_finished=TRUE;
								}
								else
								{
									timer_finished=FALSE;
								}
								#endif
							}
						}
						#if HMI_DYN_TIMER_ACTION_DURATION_NUMBER > 0
						else if(child_action_id<HMI_TIMER_ACTION_D_DURATION_MAX_ID)/*dyn timer*/
						{
							child_action_id=HMI_GET_TIMER_ACTION_D_DURATION_ID_INDEX(child_action_id);
							if(child_action_id<HMI_DYN_TIMER_ACTION_DURATION_NUMBER)
							{
								#if 0
								elapse=hmi_dyn_duration_elapse_prop[child_action_id];
								start=hmi_dyn_duration_static_prop[child_action_id].start;
								duration=hmi_dyn_duration_static_prop[child_action_id].duration;
								#endif
								#if HMI_DYN_TIMER_ACTION_DURATION_NUMBER>0
								if(hmi_dyn_duration_elapse_prop[child_action_id]>
									(hmi_dyn_duration_static_prop[child_action_id].start+hmi_dyn_duration_static_prop[child_action_id].duration))
								{
									timer_finished=TRUE;
								}
								else
								{
									timer_finished=FALSE;
								}
								#endif
							}
						}
						#endif
						else
						{
						};	
						#if 0
						if(elapse>(start+duration))
						{
							timer_finished=TRUE;
						}
						else
						{
							timer_finished=FALSE;
						}
						#endif
						if(timer_finished)
						{
							loop++;
						}
						else
						{
							search_stop_flag=TRUE;
						}						
					}
					else
					#endif
					{
						loop++;
					}
				}
				else
				#endif
				{
					loop++;
				}
				
			}
			if(loop>=nb_group_child)/*finished all action*/
			{
				finished=TRUE;
			}
		}				
	}
	return finished; 
}
#endif
/****************************************************************************
Function Name        : hmi_step
Description          : 
Invocation           : 
Parameters           :
Return Value         : None
Critical Section     : None
External Interfaces  : None
******************************************************************************/
#if HMI_ALL_ACTION_NUMBER>0
void hmi_step(HMI_OBJECT_ID_STR action_id,HMI_TIME dt,HMI_OBJECT_ID_STR parent_timer_id) REENTRANT
{
	UINT8 	 		loop		= 0;		
	BOOLEAN 		repeat_DO	= FALSE;
	BOOLEAN 		duration_DO	= FALSE;
	BOOLEAN			onetime_DO	= FALSE;	
	BOOLEAN 		delta_T_child_finished = FALSE;	
	#if HMI_TIMER_ACTION_REPEAT_NUMBER>0
	HMI_OBJECT_ID_STR   	child_action_id	= 0U;
	#endif
	HMI_OBJECT_ID_STR   	action_id_index	= 0U;
	UINT8 					nb_child		= 0U;
    HMI_OBJECT_PROP_STR CONST 	*P_child_t	= NULL;
	HMI_TIME 				hmi_elapse =0.0f;
	
	if((action_id >= HMI_ACTION_GROUP_MAX_ID)&&(action_id < HMI_ACTION_MAX_ID))
	{
		action_id_index	= HMI_GET_ACTION_ID_INDEX(action_id);
		/*timer action*/
		#if HMI_TIMER_ACTION_NUMBER > 0
		if(action_id_index < HMI_TIMER_ACTION_MAX_ID)
		{
			#if HMI_TIMER_ACTION_REPEAT_NUMBER > 0
			if(action_id_index < HMI_TIMER_ACTION_REPEAT_MAX_ID)
			{				
				if(action_id_index < HMI_TIMER_ACTION_REPEAT_NUMBER)
				{					
					repeat_DO				= (hmi_repeat_elapse_prop[action_id_index] >= hmi_repeat_start_prop[action_id_index]);
					delta_T_child_finished	= (hmi_repeat_elapse_prop[action_id_index] <= hmi_repeat_start_prop[action_id_index]);
					if(delta_T_child_finished)
					{
						hmi_repeat_elapse_prop[action_id_index]+=dt;
					}
					nb_child = hmi_timer_action_repeat_relative[action_id_index].object_number;
					P_child_t= hmi_timer_action_repeat_relative[action_id_index].p_object_table;
					/*reset finished action of repeat child elapse to 0*/
					if(repeat_DO )
					{
						for(loop=0;loop < nb_child;loop++)
						{
							delta_T_child_finished	= FALSE;
							if((P_child_t[loop].object_id >= HMI_ACTION_GROUP_MAX_ID)&&
								(P_child_t[loop].object_id < HMI_ACTION_MAX_ID))
							{
								child_action_id	= HMI_GET_ACTION_ID_INDEX(P_child_t[loop].object_id);
								#if HMI_TIMER_ACTION_ONETIME_NUMBER>0
								if(HMI_IS_TIMER_ACTION_ONETIME(child_action_id))
								{
									child_action_id	= HMI_GET_TIMER_ACTION_ONETIME_ID_INDEX(child_action_id);
									if(child_action_id < HMI_TIMER_ACTION_ONETIME_NUMBER)
									{										
										delta_T_child_finished = (hmi_onetime_elapse_prop[child_action_id] > hmi_onetime_start_prop[child_action_id]);
									}
								}
								else
								#endif
								#if HMI_TIMER_ACTION_DURATION_NUMBER>0
								if(HMI_IS_TIMER_ACTION_DURATION(child_action_id))
								{
									if(HMI_IS_TIMER_ACTION_S_DURATION(child_action_id))
									{
										child_action_id	= HMI_GET_TIMER_ACTION_S_DURATION_ID_INDEX(child_action_id);									
										#if HMI_S_TIMER_ACTION_DURATION_NUMBER>0
										if(child_action_id < HMI_S_TIMER_ACTION_DURATION_NUMBER)
										{											
											delta_T_child_finished = (hmi_duration_elapse_prop[child_action_id] >
																(hmi_duration_static_prop[child_action_id].start+hmi_duration_static_prop[child_action_id].duration));
										}
										#endif
									}
									else if(HMI_IS_TIMER_ACTION_D_DURATION(child_action_id))
									{
										child_action_id = HMI_GET_TIMER_ACTION_D_DURATION_ID_INDEX(child_action_id);									
										#if HMI_DYN_TIMER_ACTION_DURATION_NUMBER > 0
										if(child_action_id < HMI_DYN_TIMER_ACTION_DURATION_NUMBER)
										{											
											delta_T_child_finished = (hmi_dyn_duration_elapse_prop[child_action_id] >
																	(hmi_dyn_duration_static_prop[child_action_id].start+hmi_dyn_duration_static_prop[child_action_id].duration));
										}
										#endif
									}
									else
									{
									}																																		
								}
								else
								#endif
								{
								}							
								if(delta_T_child_finished == TRUE)
								{
									hmi_reset_timer_action(P_child_t[loop].object_id);
								}
							}	
						}
					}
				}				
			}			
			else
			#endif
			#if HMI_TIMER_ACTION_ONETIME_NUMBER > 0
			if(action_id_index < HMI_TIMER_ACTION_ONETIME_MAX_ID)
			{
				action_id_index	= HMI_GET_TIMER_ACTION_ONETIME_ID_INDEX(action_id_index);				
				#if HMI_TIMER_ACTION_ONETIME_NUMBER > 0
				if(action_id_index < HMI_TIMER_ACTION_ONETIME_NUMBER)
				{					
					onetime_DO	= ((hmi_onetime_elapse_prop[action_id_index] <= hmi_onetime_start_prop[action_id_index])&&
									((hmi_onetime_elapse_prop[action_id_index]+dt) >= hmi_onetime_start_prop[action_id_index]));
					/*(hmi_onetime_elapse_prop[action_id_index]+dt > hmi_onetime_start_prop[action_id_index]));*///changed by pxguo 181208
					delta_T_child_finished = (hmi_onetime_elapse_prop[action_id_index] <= hmi_onetime_start_prop[action_id_index]);
					if(delta_T_child_finished == TRUE)
					{
						if(onetime_DO == TRUE)
						{
							hmi_onetime_elapse_prop[action_id_index]+=HMI_ONE_TIME_DELTA;
						}
						else
						{
							hmi_onetime_elapse_prop[action_id_index]+=dt;
						}
					}
					nb_child = hmi_timer_action_onetime_relative[action_id_index].object_number;
					P_child_t= hmi_timer_action_onetime_relative[action_id_index].p_object_table;
				}	
				#endif
			}
			else
			#endif
			#if HMI_TIMER_ACTION_DURATION_NUMBER>0
			if(action_id_index < HMI_TIMER_ACTION_DURATION_MAX_ID)
			{
				if(action_id_index < HMI_TIMER_ACTION_S_DURATION_MAX_ID)
				{
					action_id_index	= HMI_GET_TIMER_ACTION_S_DURATION_ID_INDEX(action_id_index);
					#if HMI_S_TIMER_ACTION_DURATION_NUMBER > 0
					if(action_id_index < HMI_S_TIMER_ACTION_DURATION_NUMBER)
					{						
						duration_DO	= ((hmi_duration_elapse_prop[action_id_index]>=hmi_duration_static_prop[action_id_index].start)&&
										(hmi_duration_elapse_prop[action_id_index]<=(hmi_duration_static_prop[action_id_index].start+hmi_duration_static_prop[action_id_index].duration)));					
						delta_T_child_finished=(hmi_duration_elapse_prop[action_id_index] <=
										(hmi_duration_static_prop[action_id_index].start + hmi_duration_static_prop[action_id_index].duration));
						if(delta_T_child_finished)
						{
							hmi_elapse =hmi_duration_elapse_prop[action_id_index];
							hmi_duration_elapse_prop[action_id_index]+=dt;
							if(duration_DO  == FALSE)
							{
								duration_DO	= ((hmi_elapse<=hmi_duration_static_prop[action_id_index].start)&&
											(hmi_duration_elapse_prop[action_id_index]>=(hmi_duration_static_prop[action_id_index].start+hmi_duration_static_prop[action_id_index].duration)));	
							}
						}
						nb_child	= hmi_timer_action_duration_relative[action_id_index].object_number;
						P_child_t	= hmi_timer_action_duration_relative[action_id_index].p_object_table;
					}
					#endif
				}
				else if(action_id_index < HMI_TIMER_ACTION_D_DURATION_MAX_ID)
				{	
					#if HMI_DYN_TIMER_ACTION_DURATION_NUMBER > 0
					action_id_index	= HMI_GET_TIMER_ACTION_D_DURATION_ID_INDEX(action_id_index);					
					duration_DO	= ((hmi_dyn_duration_elapse_prop[action_id_index] >= hmi_dyn_duration_static_prop[action_id_index].start)&&
						(hmi_dyn_duration_elapse_prop[action_id_index] <= (hmi_dyn_duration_static_prop[action_id_index].start+hmi_dyn_duration_static_prop[action_id_index].duration)));					
					delta_T_child_finished	= (hmi_dyn_duration_elapse_prop[action_id_index] <=
						(hmi_dyn_duration_static_prop[action_id_index].start+hmi_dyn_duration_static_prop[action_id_index].duration));
					if(delta_T_child_finished)
					{
						hmi_elapse =hmi_dyn_duration_elapse_prop[action_id_index];
						hmi_dyn_duration_elapse_prop[action_id_index]+=dt;
						if(duration_DO  == FALSE)
						{
							duration_DO	= ((hmi_elapse<=hmi_duration_static_prop[action_id_index].start)&&
										(hmi_duration_elapse_prop[action_id_index]>=(hmi_duration_static_prop[action_id_index].start+hmi_duration_static_prop[action_id_index].duration)));	
						}
					}
					nb_child = hmi_timer_action_dyn_duration_relative[action_id_index].object_number;
					P_child_t=hmi_timer_action_dyn_duration_relative[action_id_index].p_object_table;
					#endif
				}
				else
				{
					nb_child	= 0;
					P_child_t	= NULL;
				}																									
			}
			else
			#endif
			{
			}
			if((repeat_DO == TRUE)||(onetime_DO == TRUE)||(duration_DO == TRUE))
			{           
				/*do action*/
				if(P_child_t != NULL)
				{
					for(loop=0;loop < nb_child;loop++)
					{
						hmi_step(P_child_t[loop].object_id,dt,action_id);
					}
				}			
			}
		}
		else
		#endif
		#if HMI_SET_ACTION_NUMBER>0  
		if(action_id_index < HMI_SET_ACTION_MAX_ID)
		{
			hmi_do_set_action(action_id,dt,parent_timer_id);
		}
		else
		#endif
		{
		}
	}		
}
#endif


#if HMI_SET_ACTION_NUMBER>0
#if 0
void hmi_do_set_action(HMI_OBJECT_ID_STR action_id)
{
	/*static*/ HMI_EVENT_ACTION_RES_STR CONST	*P_set_action_data=NULL;
	HMI_SET_ACTION_PROP_STR		CONST		*P_set_action_prop=NULL;
	
	if((action_id>=HMI_ACTION_GROUP_MAX_ID)&&(action_id<HMI_ACTION_MAX_ID))
	{
		P_set_action_data=hmi_get_action_list_point(action_id);
		if(P_set_action_data !=NULL)
		{
			P_set_action_prop=(HMI_SET_ACTION_PROP_STR CONST *)(P_set_action_data->P_object_prop);
			action_id=HMI_GET_ACTION_ID_INDEX(action_id);
			if((action_id>=HMI_TIMER_ACTION_MAX_ID)&&(action_id < HMI_SET_ACTION_MAX_ID))
			{
				action_id=HMI_GET_SET_ACTION_ID_INDEX(action_id);
				#if HMI_SET_POS_NUMBER>0
				if(action_id< HMI_SET_POS_MAX_ID)
				{
					action_id=HMI_GET_SET_POS_ID_INDEX(action_id);
					hmi_set_pos_action((HMI_SET_POS_STR CONST *)(&P_set_action_prop->P_set_pos_prop_table[action_id]));
				}
				else
				#endif
				#if HMI_SET_DELTA_POS_NUMBER>0
				if(action_id< HMI_SET_DELTA_POS_MAX_ID)
				{
					action_id=HMI_GET_SET_DELTA_POS_ID_INDEX(action_id);
					hmi_set_delta_pos_action((HMI_SET_DELTA_POS_STR CONST *)(&P_set_action_prop->P_set_delta_pos_prop_table[action_id]));
				}
				else
				#endif
				#if HMI_SET_W_H_NUMBER>0
				if(action_id< HMI_SET_W_H_MAX_ID)
				{
					action_id=HMI_GET_SET_W_H_ID_INDEX(action_id);
					hmi_set_w_h_action((HMI_SET_W_H_STR CONST *)(&P_set_action_prop->P_set_w_h_prop_table[action_id]));
				}
				else
				#endif
				#if HMI_SET_DELTA_W_H_NUMBER>0
				if(action_id< HMI_SET_DELTA_W_H_MAX_ID)
				{
					action_id=HMI_GET_SET_DELTA_W_H_ID_INDEX(action_id);
					hmi_set_delta_w_h_action((HMI_SET_DELTA_W_H_STR CONST *)(&P_set_action_prop->P_set_delta_w_h_prop_table[action_id]));
				}
				else
				#endif
				#if HMI_SET_DYN_CONTAINER_NUMBER>0
				if(action_id< HMI_SET_DYN_CONTAINER_MAX_ID)
				{
					action_id=HMI_GET_SET_DYN_CONTAINER_ID_INDEX(action_id);
					hmi_set_dyn_container_action((HMI_SET_DYN_CONTAINER_STR CONST *)(&P_set_action_prop->P_set_dyn_container_prop_table[action_id]));
				}
				else
				#endif
				#if HMI_SET_PAGE_ON_OFF_NUMBER>0
				if(action_id< HMI_SET_PAGE_ON_OFF_MAX_ID)
				{
					action_id=HMI_GET_SET_PAGE_ON_OFF_ID_INDEX(action_id);
					hmi_set_page_on_off_action((HMI_SET_PAGE_ON_OFF_STR CONST *)(&P_set_action_prop->P_set_page_on_off_prop_table[action_id]));
				}
				else
				#endif
				#if HMI_SET_EDIT_TEXT_NUMBER>0
				if(action_id< HMI_SET_EDIT_TEXT_MAX_ID)
				{
					action_id=HMI_GET_SET_EDIT_TEXT_ID_INDEX(action_id);
					hmi_set_edit_text_action((HMI_SET_EDIT_TEXT_STR CONST *)(&P_set_action_prop->P_set_edit_text_prop_table[action_id]));
				}
				else
				#endif
				#if HMI_SET_TEXT_SCROLL_STEP_NUMBER>0
				if(action_id< HMI_SET_TEXT_SCROLL_STEP_MAX_ID)
				{
					action_id=HMI_GET_SET_TEXT_SCROLL_STEP_ID_INDEX(action_id);
					hmi_set_text_scroll_step_action((HMI_SET_TEXT_SCROLL_STEP_STR CONST *)(&P_set_action_prop->P_set_text_scroll_step_prop_table[action_id]));
				}
				else
				#endif
				#if HMI_SET_FOR_COLOR_NUMBER>0
				if(action_id< HMI_SET_FOR_COLOR_MAX_ID)
				{
					action_id=HMI_GET_SET_FOR_COLOR_ID_INDEX(action_id);
					hmi_set_foreground_color_action((HMI_SET_COLOR_STR CONST*)&P_set_action_prop->P_set_for_color_prop_table[action_id]);
				}
				else
				#endif
				#if HMI_SET_BCK_COLOR_NUMBER>0
				if(action_id< HMI_SET_BCK_COLOR_MAX_ID)
				{
					action_id=HMI_GET_SET_BCK_COLOR_ID_INDEX(action_id);
					hmi_set_background_color_action((HMI_SET_COLOR_STR CONST*)&P_set_action_prop->P_set_bck_color_prop_table[action_id]);
				}
				else
				#endif
				#if 0
				#if HMI_SET_BTN_STATUS_NUMBER>0
				if(action_id< HMI_SET_BTN_STATUS_MAX_ID)
				{
					action_id=HMI_GET_SET_BTN_STATUS_ID_INDEX(action_id);
					hmi_set_button_status_action((HMI_SET_BTN_STATUS_STR CONST*)&P_set_action_prop->P_set_button_status_prop_table[action_id]);
				}
				else
				#endif
				#endif
				#if HMI_SET_SEND_EVENT_NUMBER>0
				if(action_id< HMI_SET_SEND_EVENT_MAX_ID)
				{
					action_id=HMI_GET_SET_SEND_EVENT_ID_INDEX(action_id);
					hmi_set_send_event_action((HMI_SET_SEND_EVENT_STR CONST*)&P_set_action_prop->P_set_send_event_prop_table[action_id]);
				}
				else
				#endif
				#if HMI_SET_CALL_FUNC_NUMBER>0
				if(action_id< HMI_SET_CALL_FUNC_MAX_ID)
				{
					action_id=HMI_GET_SET_CALL_FUNC_ID_INDEX(action_id);
					hmi_set_call_function_action((HMI_SET_CALL_FUNC_STR CONST*)&P_set_action_prop->P_set_call_func_prop_table[action_id]);
				}
				else
				#endif
				#if HMI_SET_IMAGELIST_INDEX_NUMBER>0
				if(action_id< HMI_SET_IMAGELIST_INDEX_MAX_ID)
				{
					action_id=HMI_GET_SET_IMAGELIST_INDEX_ID_INDEX(action_id);
					hmi_set_imagelist_index_action((HMI_SET_RANGE_STR CONST*)&P_set_action_prop->P_set_imagelist_index_prop_table[action_id]);
				}
				else
				#endif
				#if HMI_SET_SCROLLBAR_RANGE_NUMBER>0
				if(action_id< HMI_SET_SCROLLBAR_RANGE_MAX_ID)
				{
					action_id=HMI_GET_SET_SCROLLBAR_RANGE_ID_INDEX(action_id);
					hmi_set_scrollbar_range_action((HMI_SET_RANGE_STR CONST*)&P_set_action_prop->P_set_scrollbar_range_prop_table[action_id]);
				}
				else
				#endif
				#if HMI_SET_BUTTON_STATUS_NUMBER>0
				if(action_id< HMI_SET_BUTTON_STATUS_MAX_ID)
				{
					action_id=HMI_GET_SET_BUTTON_STATUS_ID_INDEX(action_id);
					hmi_set_button_status_action((HMI_SET_RANGE_STR CONST*)&P_set_action_prop->P_set_button_status_prop_table[action_id]);
				}
				else
				#endif
				{
				}
			}
		}
	}
}
#endif

void hmi_do_set_action(HMI_OBJECT_ID_STR action_id,HMI_TIME dt,HMI_OBJECT_ID_STR parent_timer_id)
{
	HMI_EVENT_ACTION_RES_STR	CONST	*P_set_action_data	= NULL;
	HMI_SET_ACTION_PROP_STR		CONST	*P_set_action_prop	= NULL;
	HMI_OBJECT_ID_STR					action_index		= action_id;
	
	if((action_id >= HMI_TIMER_ACTION_ABS_MAX_ID)&&(action_id < HMI_ACTION_MAX_ID))/*is  set action*/
	{		
		#if HMI_ANIM_STATIC_SET_POS_NUMBER + HMI_ANIM_DYN_SET_POS_NUMBER > 0
		if(HMI_IS_ANIM_POS(action_index))
		{			
			if(action_index < HMI_SET_POS_ANIM_SXY_MAX_ID)
			{
				#if HMI_ANIM_STATIC_SET_POS_NUMBER > 0				
				hmi_anim_set_pos_action(action_index,FALSE,dt,parent_timer_id);
				#endif
			}
			else if(action_index < HMI_SET_POS_ANIM_DXY_MAX_ID)
			{
				#if HMI_ANIM_DYN_SET_POS_NUMBER > 0				
				hmi_anim_set_pos_action(action_index,TRUE,dt,parent_timer_id);
				#endif
			}
			else
			{

			}
		}
		else
		#endif
		#if HMI_ANIM_STATIC_SET_W_H_NUMBER + HMI_ANIM_DYN_SET_W_H_NUMBER > 0
		if(HMI_IS_W_H_ANIM(action_index))
		{
			if(action_index < HMI_SET_W_H_ANIM_SXY_MAX_ID)
			{
				#if HMI_ANIM_STATIC_SET_W_H_NUMBER > 0				
				hmi_anim_set_wh_action(action_index,FALSE,dt,parent_timer_id);
				#endif
			}
			else if(action_index < HMI_SET_W_H_ANIM_DXY_MAX_ID)
			{
				#if HMI_ANIM_DYN_SET_W_H_NUMBER > 0				
				hmi_anim_set_wh_action(action_index,TRUE,dt,parent_timer_id);
				#endif
			}
			else
			{

			}
		}
		else
		#endif
		#if HMI_ANIM_STATIC_SET_FOR_COLOR_NUMBER + HMI_ANIM_DYN_SET_FOR_COLOR_NUMBER > 0
		if(HMI_IS_FOR_COLOR_ANIM(action_index))
		{
			if(action_index < HMI_SET_FOR_COLOR_ANIM_SXY_MAX_ID)
			{
				#if HMI_ANIM_STATIC_SET_FOR_COLOR_NUMBER > 0				
				hmi_anim_set_fcolor_action(action_index,FALSE,dt,parent_timer_id);
				#endif
			}
			else if(action_index < HMI_SET_FOR_COLOR_ANIM_DXY_MAX_ID)
			{
				#if HMI_ANIM_DYN_SET_FOR_COLOR_NUMBER > 0				
				hmi_anim_set_fcolor_action(action_index,TRUE,dt,parent_timer_id);
				#endif
			}
			else
			{

			}
		}
		else
		#endif
		#if HMI_ANIM_STATIC_SET_P1_NUMBER + HMI_ANIM_DYN_SET_P1_NUMBER > 0
		if(HMI_IS_P1_INDEX_ANIM(action_index))
		{
			if(action_index < HMI_SET_P1_INDEX_ANIM_SXY_MAX_ID)
			{
				#if HMI_ANIM_STATIC_SET_P1_NUMBER > 0				
				hmi_anim_set_p1_action(action_index,FALSE,dt,parent_timer_id);
				#endif
			}
			else if(action_index < HMI_SET_P1_INDEX_ANIM_DXY_MAX_ID)
			{
				#if HMI_ANIM_DYN_SET_P1_NUMBER > 0				
				hmi_anim_set_p1_action(action_index,TRUE,dt,parent_timer_id);
				#endif
			}
			else
			{

			}
		}
		else
		#endif
		#if HMI_ANIM_STATIC_SET_P2_NUMBER + HMI_ANIM_DYN_SET_P2_NUMBER > 0
		if(HMI_IS_P2_INDEX_ANIM(action_index))
		{
			if(action_index < HMI_SET_P2_INDEX_ANIM_SXY_MAX_ID)
			{
				#if HMI_ANIM_STATIC_SET_P2_NUMBER > 0				
				hmi_anim_set_p2_action(action_index,FALSE,dt,parent_timer_id);
				#endif
			}
			else if(action_index < HMI_SET_P2_INDEX_ANIM_DXY_MAX_ID)
			{
				#if HMI_ANIM_DYN_SET_P2_NUMBER > 0				
				hmi_anim_set_p2_action(action_index,TRUE,dt,parent_timer_id);
				#endif
			}
			else
			{

			}
		}
		else
		#endif
		#if HMI_ANIM_STATIC_SET_P3_NUMBER + HMI_ANIM_DYN_SET_P3_NUMBER > 0
		if(HMI_IS_P3_INDEX_ANIM(action_index))
		{
			if(action_index < HMI_SET_P3_INDEX_ANIM_SXY_MAX_ID)
			{
				#if HMI_ANIM_STATIC_SET_P3_NUMBER > 0				
				hmi_anim_set_p3_action(action_index,FALSE,dt,parent_timer_id);
				#endif
			}
			else if(action_index < HMI_SET_P3_INDEX_ANIM_DXY_MAX_ID)
			{
				#if HMI_ANIM_DYN_SET_P3_NUMBER > 0				
				hmi_anim_set_p3_action(action_index,TRUE,dt,parent_timer_id);
				#endif
			}
			else
			{

			}
		}
		else
		#endif
		#if HMI_ANIM_STATIC_SET_IMAGELIST_INDEX_NUMBER + HMI_ANIM_DYN_SET_IMAGELIST_INDEX_NUMBER > 0
		if(HMI_IS_IMAGELIST_INDEX_ANIM(action_index))
		{
			if(action_index < HMI_SET_IMAGELIST_INDEX_ANIM_SXY_MAX_ID)
			{
				#if HMI_ANIM_STATIC_SET_IMAGELIST_INDEX_NUMBER>0				
				hmi_anim_set_imglist_action(action_index,FALSE,dt,parent_timer_id);
				#endif
			}
			else if(action_index < HMI_SET_IMAGELIST_INDEX_ANIM_DXY_MAX_ID)
			{
				#if HMI_ANIM_DYN_SET_IMAGELIST_INDEX_NUMBER > 0				
				hmi_anim_set_imglist_action(action_index,TRUE,dt,parent_timer_id);
				#endif
			}
			else
			{

			}
		}
		else
		#endif
		#if HMI_ANIM_STATIC_SET_ALPHA_NUMBER + HMI_ANIM_DYN_SET_ALPHA_NUMBER > 0
		#ifndef HMI_GRAPHIC_ST7513
		if(HMI_IS_ALPHA_INDEX_ANIM(action_index))
		{
			if(action_index < HMI_SET_ALPHA_INDEX_ANIM_SXY_MAX_ID)
			{
				#if HMI_ANIM_STATIC_SET_ALPHA_NUMBER > 0				
				hmi_anim_set_alpha_action(action_index,FALSE,dt,parent_timer_id);
				#endif
			}
			else if(action_index < HMI_SET_ALPHA_INDEX_ANIM_DXY_MAX_ID)
			{
				#if HMI_ANIM_DYN_SET_ALPHA_NUMBER > 0				
				hmi_anim_set_alpha_action(action_index,TRUE,dt,parent_timer_id);
				#endif
			}
			else
			{

			}
		}
		else
		#endif
		#endif
		#if HMI_ANIM_STATIC_SET_SCALE_NUMBER + HMI_ANIM_DYN_SET_SCALE_NUMBER > 0
		#ifndef HMI_GRAPHIC_ST7513 // RGL opengl
		if(HMI_IS_SCALE_INDEX_ANIM(action_index))
		{
			if(action_index < HMI_SET_SCALE_INDEX_ANIM_SXY_MAX_ID)
			{
				#if HMI_ANIM_STATIC_SET_SCALE_NUMBER > 0				
				hmi_anim_set_scale_action(action_index,FALSE,dt,parent_timer_id);
				#endif
			}
			else if(action_index < HMI_SET_SCALE_INDEX_ANIM_DXY_MAX_ID)
			{
				#if HMI_ANIM_DYN_SET_SCALE_NUMBER > 0				
				hmi_anim_set_scale_action(action_index,TRUE,dt,parent_timer_id);
				#endif
			}
			else
			{

			}
		}
		else
		#endif
		#endif
		#if HMI_ANIM_STATIC_SET_ANGEL_NUMBER + HMI_ANIM_DYN_SET_ANGEL_NUMBER>0
		#ifndef HMI_GRAPHIC_ST7513
		if(HMI_IS_ANGEL_INDEX_ANIM(action_index))
		{
			if(action_index < HMI_SET_ANGEL_INDEX_ANIM_SXY_MAX_ID)
			{
				#if HMI_ANIM_STATIC_SET_ANGEL_NUMBER > 0				
				hmi_anim_set_angle_action(action_index,FALSE,dt,parent_timer_id);
				#endif
			}
			else if(action_index<HMI_SET_ANGEL_INDEX_ANIM_DXY_MAX_ID)
			{
				#if HMI_ANIM_DYN_SET_ANGEL_NUMBER > 0				
				hmi_anim_set_angle_action(action_index,TRUE,dt,parent_timer_id);
				#endif
			}
			else
			{
			}
		}
		else
		#endif
		#endif		
		{	/*no animation*/
			P_set_action_data	= hmi_get_action_list_point(action_id);
			if(P_set_action_data != NULL)
			{
				P_set_action_prop	= (HMI_SET_ACTION_PROP_STR CONST *)(P_set_action_data->P_object_prop);
				action_index		= HMI_GET_ACTION_ID_INDEX(action_id);
				if((action_index >= HMI_TIMER_ACTION_MAX_ID)&&(action_index < HMI_SET_ACTION_MAX_ID))
				{
					action_index	= HMI_GET_SET_ACTION_ID_INDEX(action_index);
					#if HMI_NO_ANIM_SET_POS_NUMBER > 0
					if(action_id < HMI_SET_POS_NO_ANIM_MAX_ID)
					{
						action_id	= HMI_GET_SET_POS_NO_ANIM_ID_INDEX(action_id);
						hmi_set_pos_action((HMI_SET_POS_STR CONST *)(&P_set_action_prop->P_set_pos_prop_table[action_id]));
					}
					else
					#endif
					#if HMI_SET_DELTA_POS_NUMBER > 0
					if(action_id < HMI_SET_DELTA_POS_MAX_ID)
					{
						action_id	= HMI_GET_SET_DELTA_POS_ID_INDEX(action_id);
						hmi_set_delta_pos_action((HMI_SET_DELTA_POS_STR CONST *)(&P_set_action_prop->P_set_delta_pos_prop_table[action_id]));
					}
					else
					#endif
					#if HMI_SET_W_H_NO_ANIM_NUMBER > 0
					if(action_id < HMI_SET_W_H_NO_ANIM_MAX_ID)
					{
						action_id	= HMI_GET_SET_W_H_NO_ANIM_ID_INDEX(action_id);
						hmi_set_w_h_action((HMI_SET_W_H_STR CONST *)(&P_set_action_prop->P_set_w_h_prop_table[action_id]));
					}
					else
					#endif
					#if HMI_SET_DELTA_W_H_NUMBER > 0
					if(action_id < HMI_SET_DELTA_W_H_MAX_ID)
					{
						action_id	= HMI_GET_SET_DELTA_W_H_ID_INDEX(action_id);
						hmi_set_delta_w_h_action((HMI_SET_DELTA_W_H_STR CONST *)(&P_set_action_prop->P_set_delta_w_h_prop_table[action_id]));
					}
					else
					#endif
					#if HMI_SET_DYN_CONTAINER_NUMBER > 0
					if(action_id < HMI_SET_DYN_CONTAINER_MAX_ID)
					{
						action_id	= HMI_GET_SET_DYN_CONTAINER_ID_INDEX(action_id);
						hmi_set_dyn_container_action((HMI_SET_DYN_CONTAINER_STR CONST *)(&P_set_action_prop->P_set_dyn_container_prop_table[action_id]));
					}
					else
					#endif
					#if HMI_SET_PAGE_ON_OFF_NUMBER > 0
					if(action_id < HMI_SET_PAGE_ON_OFF_MAX_ID)
					{
						action_id	= HMI_GET_SET_PAGE_ON_OFF_ID_INDEX(action_id);
						hmi_set_page_on_off_action((HMI_SET_PAGE_ON_OFF_STR CONST *)(&P_set_action_prop->P_set_page_on_off_prop_table[action_id]));
					}
					else
					#endif
					#if HMI_SET_EDIT_TEXT_NUMBER > 0
					if(action_id < HMI_SET_EDIT_TEXT_MAX_ID)
					{
						action_id	= HMI_GET_SET_EDIT_TEXT_ID_INDEX(action_id);
						hmi_set_edit_text_action((HMI_SET_EDIT_TEXT_STR CONST *)(&P_set_action_prop->P_set_edit_text_prop_table[action_id]));
					}
					else
					#endif
					#if HMI_SET_TEXT_SCROLL_STEP_NUMBER > 0
					if(action_id < HMI_SET_TEXT_SCROLL_STEP_MAX_ID)
					{
						action_id	= HMI_GET_SET_TEXT_SCROLL_STEP_ID_INDEX(action_id);
						hmi_set_text_scroll_step_action((HMI_SET_TEXT_SCROLL_STEP_STR CONST *)(&P_set_action_prop->P_set_text_scroll_step_prop_table[action_id]));
					}
					else
					#endif
					#if HMI_SET_FOR_COLOR_NO_ANIM_NUMBER > 0
					if(action_id < HMI_SET_FOR_COLOR_NO_ANIM_MAX_ID)
					{
						action_id	= HMI_GET_SET_FOR_COLOR_NO_ANIM_ID_INDEX(action_id);
						hmi_set_foreground_color_action((HMI_SET_COLOR_STR CONST*)&P_set_action_prop->P_set_for_color_prop_table[action_id]);
					}
					else
					#endif
					#if HMI_SET_BCK_COLOR_NUMBER > 0
					if(action_id < HMI_SET_BCK_COLOR_MAX_ID)
					{
						action_id	= HMI_GET_SET_BCK_COLOR_ID_INDEX(action_id);
						hmi_set_background_color_action((HMI_SET_COLOR_STR CONST*)&P_set_action_prop->P_set_bck_color_prop_table[action_id]);
					}
					else
					#endif
					#if HMI_NO_ANIM_SET_P1_NUMBER > 0 
					if(action_id < HMI_SET_P1_INDEX_NO_ANIM_MAX_ID)
					{
						action_id	= HMI_GET_SET_P1_ID_INDEX(action_id);
						if(action_id < HMI_NO_ANIM_SET_P1_NUMBER)
						{							
							hmi_engine_set_object_info(hmi_set_p1_prop_table[action_id].object_id,
														hmi_set_p1_prop_table[action_id].p_value);
						}
					}
					else
					#endif
					#if HMI_NO_ANIM_SET_P2_NUMBER > 0 
					if(action_id < HMI_SET_P2_INDEX_NO_ANIM_MAX_ID)
					{
						action_id	= HMI_GET_SET_P2_ID_INDEX(action_id);
						if(action_id < HMI_NO_ANIM_SET_P2_NUMBER)
						{							
							hmi_engine_set_object_info(hmi_set_p2_prop_table[action_id].object_id,
														hmi_set_p2_prop_table[action_id].p_value);
						}
					}
					else
					#endif
					#if HMI_NO_ANIM_SET_P3_NUMBER > 0 
					if(action_id < HMI_SET_P3_INDEX_NO_ANIM_MAX_ID)
					{
						action_id	= HMI_GET_SET_P3_ID_INDEX(action_id);
						if(action_id < HMI_NO_ANIM_SET_P3_NUMBER)
						{							
							hmi_engine_set_object_info(hmi_set_p3_prop_table[action_id].object_id,
														hmi_set_p3_prop_table[action_id].p_value);
						}
					}
					else
					#endif

					#if 0
					#if HMI_SET_BTN_STATUS_NUMBER > 0
					if(action_id < HMI_SET_BTN_STATUS_MAX_ID)
					{
						action_id	= HMI_GET_SET_BTN_STATUS_ID_INDEX(action_id);
						hmi_set_button_status_action((HMI_SET_BTN_STATUS_STR CONST*)&P_set_action_prop->P_set_button_status_prop_table[action_id]);
					}
					else
					#endif
					#endif
					#if HMI_SET_SEND_EVENT_NUMBER > 0
					if(action_id < HMI_SET_SEND_EVENT_MAX_ID)
					{
						action_id	= HMI_GET_SET_SEND_EVENT_ID_INDEX(action_id);
						hmi_set_send_event_action((HMI_SET_SEND_EVENT_STR CONST*)&P_set_action_prop->P_set_send_event_prop_table[action_id]);
					}
					else
					#endif
					#if HMI_SET_CALL_FUNC_NUMBER > 0
					if(action_id < HMI_SET_CALL_FUNC_MAX_ID)
					{
						action_id	= HMI_GET_SET_CALL_FUNC_ID_INDEX(action_id);
						hmi_set_call_function_action((HMI_SET_CALL_FUNC_STR CONST*)&P_set_action_prop->P_set_call_func_prop_table[action_id]);
					}
					else
					#endif
					#if HMI_SET_IMAGELIST_INDEX_NO_ANIM_NUMBER > 0
					if(action_id < HMI_SET_IMAGELIST_INDEX_NO_ANIM_MAX_ID)
					{
						action_id	= HMI_GET_SET_IMAGELIST_INDEX_NO_ANIM_ID_INDEX(action_id);
						hmi_set_imagelist_index_action((HMI_SET_RANGE_STR CONST*)&P_set_action_prop->P_set_imagelist_index_prop_table[action_id]);
					}
					else
					#endif
					#if HMI_SET_SCROLLBAR_RANGE_NUMBER > 0
					if(action_id < HMI_SET_SCROLLBAR_RANGE_MAX_ID)
					{
						action_id	= HMI_GET_SET_SCROLLBAR_RANGE_ID_INDEX(action_id);
						hmi_set_scrollbar_range_action((HMI_SET_RANGE_STR CONST*)&P_set_action_prop->P_set_scrollbar_range_prop_table[action_id]);
					}
					else
					#endif
					#if HMI_SET_BUTTON_STATUS_NUMBER > 0 
					if(action_id < HMI_SET_BUTTON_STATUS_MAX_ID)
					{
						action_id	= HMI_GET_SET_BUTTON_STATUS_ID_INDEX(action_id); 
						hmi_set_button_status_action((HMI_SET_RANGE_STR CONST*)&P_set_action_prop->P_set_button_status_prop_table[action_id]);
					}
					else					
					#endif
					#if HMI_SET_ALPHA_NO_ANIM_NUMBER > 0 
					if(action_id < HMI_SET_ALPHA_INDEX_NO_ANIM_MAX_ID)
					{
						action_id	= HMI_GET_SET_ALPHA_NO_ANIM_ID_INDEX(action_id);
						if(action_id < HMI_SET_ALPHA_NO_ANIM_NUMBER)
						{							
							hmi_engine_set_object_info(hmi_set_alpha_prop_table[action_id].object_id_angel,
										(HMI_OBJECT_DATA_STR)(hmi_set_alpha_prop_table[action_id].alpha));
						}
					}
					else
					#endif
					#if HMI_SET_SCALE_NO_ANIM_NUMBER > 0 
					if(action_id < HMI_SET_SCALE_INDEX_NO_ANIM_MAX_ID)
					{
						action_id	= HMI_GET_SET_SCALE_NO_ANIM_ID_INDEX(action_id);
						if(action_id < HMI_SET_SCALE_NO_ANIM_NUMBER)
						{							
							hmi_engine_set_object_info(hmi_set_scale_prop_table[action_id].object_id_scale,
										(HMI_F32_TO_U32(hmi_set_scale_prop_table[action_id].scale)));
						}
					}
					else
					#endif
					#if HMI_SET_ANGEL_NO_ANIM_NUMBER>0 
					if(action_id < HMI_SET_ANGEL_INDEX_NO_ANIM_MAX_ID)
					{
						action_id	= HMI_GET_SET_ANGEL_NO_ANIM_ID_INDEX(action_id);
						if(action_id < HMI_SET_ANGEL_NO_ANIM_NUMBER)
						{							
							hmi_engine_set_object_info(hmi_set_angel_prop_table[action_id].object_id_angel,
									HMI_F32_TO_U32( (hmi_set_angel_prop_table[action_id].angel)));
						}
					}
					else
					#endif
					{
					}
				}
			}
		}
	}
}


#endif






UINT32 get_bezier_length(UINT32 *pSum,UINT32 len,UINT32 nPoint, POINT32_TP CONST *pPointArray)
{
	U32	actLen		= 0;
	U16	sum			= 0;
	U16	index		= 0;
	U16	indexSum	= 0;
	POINT32_TP CONST	*pPoint=NULL;
	POINT32_TP p[HMI_BEZIER_STEP+1];
	double_64	tt	= 0;
	double_64	step= 0;
	U16		j		= 0;
	U16		i		= 0;
	U16		dx		= 0;
	U16		dy		= 0;

	for(j=0;j < HMI_BEZIER_STEP+1;j++)
	{
		p[j].x	= 0;
		p[j].y	= 0;
	}
	
	pPoint	= pPointArray;	
	index	= (U16)nPoint ;
	
	step=(double_64)1.0/(double_64)HMI_BEZIER_STEP;		
	while(index >= 4)
	{
		j=0;
		for(tt=0.0;tt<1.01;tt=(tt+step))//get bezier point
		{
			if(j <= HMI_BEZIER_STEP)
			{
				p[j]	= hor(3,pPoint,tt);	
				j++;
			}
		}
		for(i=1;i < j;i++)
		{
			if(p[i-1].x > p[i].x)
			{
				dx	= (U16)(p[i-1].x - p[i].x);
			}
			else
			{
				dx	= (U16)(p[i].x - p[i-1].x);
			}
			if(p[i-1].y > p[i].y)
			{
				dy	= (U16)(p[i-1].y - p[i].y);
			}
			else
			{
				dy	= (U16)(p[i].y - p[i-1].y);
			}
			sum	= sum+dx+dy;
		}
		if(indexSum < len)
		{
			pSum[indexSum]	= sum;
			indexSum++;
			actLen++;
		}
		sum	= 0;
		index-=HMI_BEZIER_POINT_CNT;
		pPoint+=HMI_BEZIER_POINT_CNT;
		
	}
	
	return actLen;
}

UINT32 get_bezier_length_f(UINT32 *pSum,UINT32 len,UINT32 nPoint, POINT_FLOAT_TP CONST *pPointArray)
{
	U32	actLen		= 0;
	U16	sum			= 0;
	U16	index		= 0;
	U16	indexSum	= 0;
	POINT_FLOAT_TP CONST	*pPoint=NULL;
	POINT32_TP p[HMI_BEZIER_STEP+1];
	double_64	tt	= 0;
	double_64	step= 0;
	U16		j		= 0;
	U16		i		= 0;
	U16		dx		= 0;
	U16		dy		= 0;

	for(j=0;j < HMI_BEZIER_STEP+1;j++)
	{
		p[j].x	= 0;
		p[j].y	= 0;
	}
	
	pPoint	= pPointArray;	
	index	= (U16)nPoint ;
	
	step=(double_64)1.0/(double_64)HMI_BEZIER_STEP;		
	while(index >= 4)
	{
		j=0;
		for(tt=0.0;tt<1.01;tt=tt+step)//get bezier point
		{
			if(j <= HMI_BEZIER_STEP)
			{
				p[j]	= hor_f(3,pPoint,tt);
				j++;
			}
		}
		for(i=1;i < j;i++)
		{
			if(p[i-1].x > p[i].x)
			{
				dx	= (U16)(p[i-1].x - p[i].x);
			}
			else
			{
				dx	= (U16)(p[i].x - p[i-1].x);
			}
			if(p[i-1].y > p[i].y)
			{
				dy	= (U16)(p[i-1].y - p[i].y);
			}
			else
			{
				dy	= (U16)(p[i].y - p[i-1].y);
			}
			sum	= sum+dx+dy;
		}
		if(indexSum < len)
		{
			pSum[indexSum]	= sum;
			indexSum++;
			actLen++;
		}
		sum	= 0;
		index-=HMI_BEZIER_POINT_CNT;
		pPoint+=HMI_BEZIER_POINT_CNT;
		
	}
	
	return actLen;
}

void hmi_get_bezier_index(HMI_TIME			transDt,
							UINT32	*pPoint,UINT32 actLen,
							U16				bezierSum,
							HMI_TIME		duration,
							U08				point_cnt,
							CONST POINT32_TP		*pPoint_list,
							HMI_BEZIER_NEW_POS_STR *pbezier_new_pos)
{
	BYTE 	index		= 0;
	UINT32	curDistance	= 0;
	UINT32	bezierCnt	= 0;
	UINT32	j			= 0;
	UINT32	dx			= 0;
	UINT32	dy			= 0;
	UINT32	ds			= 0;
	
	double_64 tt		= 0;
	double_64 step		= 0;	
	POINT32_TP CONST	*pBezierPoint	= NULL;
	POINT32_TP p[HMI_BEZIER_STEP+1];
	float_32	averageSpeed	=0;
	
	if((pPoint != NULL)&&(pbezier_new_pos != NULL))
	{				
		if(duration != 0)
		{
			averageSpeed	= bezierSum / duration;
			curDistance		= (UINT32)(averageSpeed * transDt + 0.5f);
			if(curDistance > bezierSum)
			{
				curDistance = bezierSum;
			}
		}

		/*get Bezier Count*/
		bezierCnt=point_cnt/HMI_BEZIER_POINT_CNT;
		for(index=0;(index < bezierCnt)&&(index < actLen);index++)
		{
			if(curDistance <= (pPoint[index]))
			{
				/*index is Bezier NO*/
				break;
			}
			else
			{
				curDistance-=pPoint[index];
			}
		}

		//index is m_point array index
		if((index*HMI_BEZIER_POINT_CNT+HMI_BEZIER_POINT_CNT)>=point_cnt)
		{		
			if(point_cnt >= HMI_ONE_BEZIER_POINT_CNT)
			{
				index=point_cnt-HMI_ONE_BEZIER_POINT_CNT;
			}
			else
			{
				index=0;
			}
		}	
		else
		{
			index=index*HMI_BEZIER_POINT_CNT;
		}
		pBezierPoint=pPoint_list+index;		
		step=(double)1.0/((double)HMI_BEZIER_STEP);
		/*index is total bezier point number*/
		index=HMI_ONE_BEZIER_POINT_CNT;
		/*get one bezier all point*/
		tt	= 0.0;
		p[0]= hor(3,pBezierPoint,tt);
		j=1;
		for(tt=step/*0.0*/;tt<1.01;tt=tt+step)/*get bezier point*/
		{
			if(j<= HMI_BEZIER_STEP)
			{
				p[j]	= hor(3,pBezierPoint,tt);
				if(p[j].x > p[j-1].x)
				{
					dx	= p[j].x - p[j-1].x;
				}
				else
				{
					dx	= p[j-1].x - p[j].x;
				}
				if(p[j].y > p[j-1].y)
				{
					dy	= p[j].y - p[j-1].y;
				}
				else
				{
					dy	= p[j-1].y - p[j].y;
				}
				ds	= dx+dy;
				if(curDistance > ds)
				{
					curDistance-=ds;
				}
				else
				{
					break;
				}
				j++;
			}
		}	
		/*get line start and end point*/
		if(pbezier_new_pos != NULL)
		{
			pbezier_new_pos->lineStart.x	= p[j-1].x;
			pbezier_new_pos->lineStart.y	= p[j-1].y;

			pbezier_new_pos->lineEnd.x		= p[j].x;
			pbezier_new_pos->lineEnd.y		= p[j].y;

			pbezier_new_pos->newDur		= ds/averageSpeed;
			pbezier_new_pos->newElapse	= curDistance/averageSpeed;
		}		
	}	
}

void hmi_get_bezier_index_f(HMI_TIME			transDt,
							UINT32	*pPoint,UINT32 actLen,
							U16				bezierSum,
							HMI_TIME		duration,
							U08				point_cnt,
							CONST POINT_FLOAT_TP		*pPoint_list,
							HMI_BEZIER_NEW_POS_STR *pbezier_new_pos)
{
	BYTE 	index		= 0;
	UINT32	curDistance	= 0;
	UINT32	bezierCnt	= 0;
	UINT32	j			= 0;
	UINT32	dx			= 0;
	UINT32	dy			= 0;
	UINT32	ds			= 0;
	
	double_64 tt		= 0;
	double_64 step		= 0;	
	POINT_FLOAT_TP CONST	*pBezierPoint	= NULL;
	POINT32_TP p[HMI_BEZIER_STEP+1];
	float_32	averageSpeed	=0;
	
	if((pPoint != NULL)&&(pbezier_new_pos != NULL))
	{				
		if(duration != 0)
		{
			averageSpeed	= bezierSum / duration;
			curDistance		= (UINT32)(averageSpeed * transDt + 0.5f);
			if(curDistance > bezierSum)
			{
				curDistance = bezierSum;
			}
		}

		/*get Bezier Count*/
		bezierCnt=point_cnt/HMI_BEZIER_POINT_CNT;
		for(index=0;(index < bezierCnt)&&(index < actLen);index++)
		{
			if(curDistance <= (pPoint[index]))
			{
				/*index is Bezier NO*/
				break;
			}
			else
			{
				curDistance-=pPoint[index];
			}
		}

		//index is m_point array index
		if((index*HMI_BEZIER_POINT_CNT+HMI_BEZIER_POINT_CNT)>=point_cnt)
		{		
			if(point_cnt >= HMI_ONE_BEZIER_POINT_CNT)
			{
				index=point_cnt-HMI_ONE_BEZIER_POINT_CNT;
			}
			else
			{
				index=0;
			}
		}	
		else
		{
			index=index*HMI_BEZIER_POINT_CNT;
		}
		pBezierPoint=pPoint_list+index;		
		step=(double)1.0/((double)HMI_BEZIER_STEP);
		/*index is total bezier point number*/
		index=HMI_ONE_BEZIER_POINT_CNT;
		/*get one bezier all point*/
		tt	= 0.0;
		p[0]= hor_f(3,pBezierPoint,tt);
		j=1;
		for(tt=step/*0.0*/;tt<1.01;tt=tt+step)/*get bezier point*/
		{
			if(j<= HMI_BEZIER_STEP)
			{
				p[j]	= hor_f(3,pBezierPoint,tt);
				if(p[j].x > p[j-1].x)
				{
					dx	= p[j].x - p[j-1].x;
				}
				else
				{
					dx	= p[j-1].x - p[j].x;
				}
				if(p[j].y > p[j-1].y)
				{
					dy	= p[j].y - p[j-1].y;
				}
				else
				{
					dy	= p[j-1].y - p[j].y;
				}
				ds	= dx+dy;
				if(curDistance > ds)
				{
					curDistance-=ds;
				}
				else
				{
					break;
				}
				j++;
			}
		}	
		/*get line start and end point*/
		if(pbezier_new_pos != NULL)
		{
			pbezier_new_pos->lineStart.x	= p[j-1].x;
			pbezier_new_pos->lineStart.y	= p[j-1].y;

			pbezier_new_pos->lineEnd.x		= p[j].x;
			pbezier_new_pos->lineEnd.y		= p[j].y;

			pbezier_new_pos->newDur		= ds/averageSpeed;
			pbezier_new_pos->newElapse	= curDistance/averageSpeed;
		}		
	}	
}

#if ((HMI_ANIM_DYN_SET_POS_NUMBER>0) ||(HMI_ANIM_STATIC_SET_POS_NUMBER>0))
HMI_ANIM_POS_STR CONST * hmi_engine_get_act_set_bezier(HMI_OBJECT_ID_STR hmi_object_id)
{
	HMI_OBJECT_ID_STR	obj_index_offset		= 0;
	HMI_ANIM_POS_STR	CONST * pmotion_path	= NULL;

	#if(HMI_ANIM_DYN_SET_POS_NUMBER>0)
	if(HMI_IS_POS_ANIM_DXY_INDEX(hmi_object_id))
	{
		obj_index_offset	= HMI_GET_POS_ANIM_DXY_INDEX(hmi_object_id);
		if(obj_index_offset < HMI_ANIM_DYN_SET_POS_NUMBER)
		{
			pmotion_path =	&hmi_anim_dyn_set_pos_prop_table[obj_index_offset];
		}
	}
	else 
	#endif
	#if(HMI_ANIM_STATIC_SET_POS_NUMBER>0)
	if(HMI_IS_POS_ANIM_SXY_INDEX(hmi_object_id))
	{
		obj_index_offset= HMI_GET_POS_ANIM_SXY_INDEX(hmi_object_id);
		if((obj_index_offset < HMI_ANIM_STATIC_SET_POS_NUMBER))
		{
			pmotion_path =	&hmi_anim_static_set_pos_prop_table[obj_index_offset];
		}
	}
	else
	#endif
	{
	}
	return pmotion_path;
}

HMI_OBJECT_ID_STR hmi_engine_get_act_set_execute_id(HMI_OBJECT_ID_STR hmi_object_id)
{
	HMI_OBJECT_ID_STR	obj_index_offset		= 0;
	HMI_OBJECT_ID_STR	hmi_execute_id			= 0;
	
	#if(HMI_ANIM_DYN_SET_POS_NUMBER>0)
	if(HMI_IS_POS_ANIM_DXY_INDEX(hmi_object_id))
	{
		obj_index_offset	= HMI_GET_POS_ANIM_DXY_INDEX(hmi_object_id);
		if(obj_index_offset < HMI_ANIM_DYN_SET_POS_NUMBER)
		{
			hmi_execute_id =	hmi_anim_dyn_set_pos_execute_id[obj_index_offset];
		}
	}
	else
	#endif
	#if(HMI_ANIM_STATIC_SET_POS_NUMBER>0)
	if(HMI_IS_POS_ANIM_SXY_INDEX(hmi_object_id))
	{
		obj_index_offset= HMI_GET_POS_ANIM_SXY_INDEX(hmi_object_id);
		if((obj_index_offset < HMI_ANIM_STATIC_SET_POS_NUMBER))
		{
			hmi_execute_id =	hmi_anim_static_set_pos_execute_id[obj_index_offset];
		}
	}
	else
	#endif
	{
	}
	
	return hmi_execute_id;

}
#endif

#if HMI_ANIM_STATIC_SET_POS_NUMBER+HMI_ANIM_DYN_SET_POS_NUMBER>0
//#if bezier no>0




void hmi_anim_set_pos_action(HMI_OBJECT_ID_STR obj_index,BOOLEAN dyn_anim,HMI_TIME dt,
							HMI_OBJECT_ID_STR parent_timer_id)
{
	BOOLEAN				success			= FALSE;
	HMI_OBJECT_DATA_STR hmi_object_data	= 0;
	HMI_OBJECT_ID_STR	obj_index_offset= 0;
	HMI_OBJECT_ID_STR	obj_index_pro	= 0;
	HMI_OBJECT_ID_STR	obj_executer	= HMI_NB_ELEMENTS;	
	POINT32_TP			pos[HMI_ANIM_POS_CNT]	= {{0,0},{0,0},0,0};
	//#ifdef bezier
	UINT32	 bezier_length_list[HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT];
	HMI_BEZIER_NEW_POS_STR	bezier_new_pos={{0,0},{0,0},0,0};
	UINT32				act_len		= 0;
	HMI_TIME			trans_dt	= 0;
	//#endif
	parent_timer_id	= HMI_GET_ACTION_ID_INDEX(parent_timer_id);
	if(dyn_anim && (HMI_IS_POS_ANIM_DXY_INDEX(obj_index)))
	{
		if(HMI_IS_TIMER_ACTION_D_DURATION(parent_timer_id))/*dyn timer and dyn anim*/
		{
			#if (HMI_DYN_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_DYN_SET_POS_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_D_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_POS_ANIM_DXY_INDEX(obj_index);
			if((obj_index_offset < HMI_ANIM_DYN_SET_POS_NUMBER)&&(parent_timer_id < HMI_DYN_TIMER_ACTION_DURATION_NUMBER))
			{
				if(hmi_anim_dyn_set_pos_prop_table[obj_index_offset].point_cnt == HMI_LINE_PATH_CNT)
				{
					dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
										hmi_anim_dyn_set_pos_anim_type[obj_index_offset],
										hmi_dyn_duration_static_prop[parent_timer_id].duration,
										hmi_dyn_duration_elapse_prop[parent_timer_id],
										dt);				
					pos[HMI_START_POS].x	= hmi_anim_dyn_set_pos_prop_table[obj_index_offset].pPoint_list[0].x;
					pos[HMI_START_POS].y	= hmi_anim_dyn_set_pos_prop_table[obj_index_offset].pPoint_list[0].y;
					pos[HMI_END_POS].x		= hmi_anim_dyn_set_pos_prop_table[obj_index_offset].pPoint_list[1].x;
					pos[HMI_END_POS].y		= hmi_anim_dyn_set_pos_prop_table[obj_index_offset].pPoint_list[1].y;
					success		= get_next_step_anim32(pos,dt,
									hmi_dyn_duration_static_prop[parent_timer_id].duration);												
					obj_executer= hmi_anim_dyn_set_pos_execute_id[obj_index_offset];
				}
				else	/*Bezier path*/
				{
					/*transform dt*/
					trans_dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
									hmi_anim_dyn_set_pos_anim_type[obj_index_offset],
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_dyn_duration_elapse_prop[parent_timer_id],
									dt);
					/*get every bezier length*/
					act_len		= get_bezier_length(bezier_length_list,HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT,
									hmi_anim_dyn_set_pos_prop_table[obj_index_offset].point_cnt,
									hmi_anim_dyn_set_pos_prop_table[obj_index_offset].pPoint_list);
					hmi_get_bezier_index(trans_dt,bezier_length_list,act_len,
									hmi_anim_dyn_set_pos_prop_table[obj_index_offset].line_sum,
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_anim_dyn_set_pos_prop_table[obj_index_offset].point_cnt,
									hmi_anim_dyn_set_pos_prop_table[obj_index_offset].pPoint_list,
									&bezier_new_pos);
					#if 0
					if(bezier_new_pos.newElapse>dt)
					{
						bezier_new_pos.newElapse-=dt;
					}
					else
					{
						bezier_new_pos.newElapse=0;
					}
					#endif														
					trans_dt	= get_trans_dt(0/*start time*/,hmi_anim_dyn_set_pos_anim_type[obj_index_offset],
									bezier_new_pos.newDur,bezier_new_pos.newElapse,dt/*dt*/);
					pos[HMI_START_POS].x	= bezier_new_pos.lineStart.x;
					pos[HMI_START_POS].y	= bezier_new_pos.lineStart.y;
					pos[HMI_END_POS].x		= bezier_new_pos.lineEnd.x;
					pos[HMI_END_POS].y		= bezier_new_pos.lineEnd.y;
					success		= get_next_step_anim32(pos,trans_dt,
													bezier_new_pos.newDur);						
					obj_executer= hmi_anim_dyn_set_pos_execute_id[obj_index_offset];
				}
			}
			#endif
		}
		else if(HMI_IS_TIMER_ACTION_S_DURATION(parent_timer_id))/*static timer and dyn anim*/
		{
			#if (HMI_S_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_DYN_SET_POS_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_S_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_POS_ANIM_DXY_INDEX(obj_index);
			if((obj_index_offset < HMI_ANIM_DYN_SET_POS_NUMBER)&&(parent_timer_id < HMI_S_TIMER_ACTION_DURATION_NUMBER))
			{
				if(hmi_anim_dyn_set_pos_prop_table[obj_index_offset].point_cnt == HMI_LINE_PATH_CNT)
				{
					dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
										hmi_anim_dyn_set_pos_anim_type[obj_index_offset],
										hmi_duration_static_prop[parent_timer_id].duration,
										hmi_duration_elapse_prop[parent_timer_id],
										dt);				
					pos[HMI_START_POS].x= hmi_anim_dyn_set_pos_prop_table[obj_index_offset].pPoint_list[0].x;
					pos[HMI_START_POS].y= hmi_anim_dyn_set_pos_prop_table[obj_index_offset].pPoint_list[0].y;
					pos[HMI_END_POS].x	= hmi_anim_dyn_set_pos_prop_table[obj_index_offset].pPoint_list[1].x;
					pos[HMI_END_POS].y	= hmi_anim_dyn_set_pos_prop_table[obj_index_offset].pPoint_list[1].y;
					success		= get_next_step_anim32(pos,dt,
									hmi_duration_static_prop[parent_timer_id].duration);								
					obj_executer= hmi_anim_dyn_set_pos_execute_id[obj_index_offset];
				}
				else /*Bezier path*/
				{
					/*transform dt*/
					trans_dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
									hmi_anim_dyn_set_pos_anim_type[obj_index_offset],
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_duration_elapse_prop[parent_timer_id],
									dt);
					/*get every bezier length*/
					act_len	= get_bezier_length(bezier_length_list,HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT,
									hmi_anim_dyn_set_pos_prop_table[obj_index_offset].point_cnt,
									hmi_anim_dyn_set_pos_prop_table[obj_index_offset].pPoint_list);
					hmi_get_bezier_index(trans_dt,bezier_length_list,act_len,
									hmi_anim_dyn_set_pos_prop_table[obj_index_offset].line_sum,
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_anim_dyn_set_pos_prop_table[obj_index_offset].point_cnt,
									hmi_anim_dyn_set_pos_prop_table[obj_index_offset].pPoint_list,
									&bezier_new_pos);
					#if 0
					if(bezier_new_pos.newElapse>dt)
					{
						bezier_new_pos.newElapse-=dt;
					}
					else
					{
						bezier_new_pos.newElapse=0;
					}
					#endif														
					trans_dt	= get_trans_dt(0/*start time*/,hmi_anim_dyn_set_pos_anim_type[obj_index_offset],
									bezier_new_pos.newDur,bezier_new_pos.newElapse,dt/*dt*/);
					pos[HMI_START_POS].x	= bezier_new_pos.lineStart.x;
					pos[HMI_START_POS].y	= bezier_new_pos.lineStart.y;
					pos[HMI_END_POS].x		= bezier_new_pos.lineEnd.x;
					pos[HMI_END_POS].y		= bezier_new_pos.lineEnd.y;
					success		= get_next_step_anim32(pos,trans_dt,
									bezier_new_pos.newDur);						
					obj_executer= hmi_anim_dyn_set_pos_execute_id[obj_index_offset];
				}
			}
			#endif						
		}
		else
		{
		}				
	}
	else if((!dyn_anim)&&(HMI_IS_POS_ANIM_SXY_INDEX(obj_index)))
	{
		if(HMI_IS_TIMER_ACTION_D_DURATION(parent_timer_id))/*dyn timer and static anim*/
		{
			#if (HMI_DYN_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_STATIC_SET_POS_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_D_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_POS_ANIM_SXY_INDEX(obj_index);
			if((obj_index_offset < HMI_ANIM_STATIC_SET_POS_NUMBER)&&(parent_timer_id < HMI_DYN_TIMER_ACTION_DURATION_NUMBER))
			{
				if(hmi_anim_static_set_pos_prop_table[obj_index_offset].point_cnt == HMI_LINE_PATH_CNT)
				{
					dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
										hmi_anim_static_set_pos_anim_type[obj_index_offset],
										hmi_dyn_duration_static_prop[parent_timer_id].duration,
										hmi_dyn_duration_elapse_prop[parent_timer_id],
										dt);				
					pos[HMI_START_POS].x	= hmi_anim_static_set_pos_prop_table[obj_index_offset].pPoint_list[0].x;
					pos[HMI_START_POS].y	= hmi_anim_static_set_pos_prop_table[obj_index_offset].pPoint_list[0].y;
					pos[HMI_END_POS].x		= hmi_anim_static_set_pos_prop_table[obj_index_offset].pPoint_list[1].x;
					pos[HMI_END_POS].y		= hmi_anim_static_set_pos_prop_table[obj_index_offset].pPoint_list[1].y;
					success		= get_next_step_anim32(pos ,dt,
									hmi_dyn_duration_static_prop[parent_timer_id].duration);								
					obj_executer= hmi_anim_static_set_pos_execute_id[obj_index_offset];
				}
				else	/*Bezier*/
				{
					/*transform dt*/
					trans_dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
									hmi_anim_static_set_pos_anim_type[obj_index_offset],
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_dyn_duration_elapse_prop[parent_timer_id],
									dt);
					/*get every bezier length*/
					act_len	= get_bezier_length(bezier_length_list,HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT,
									hmi_anim_static_set_pos_prop_table[obj_index_offset].point_cnt,
									hmi_anim_static_set_pos_prop_table[obj_index_offset].pPoint_list);
					hmi_get_bezier_index(trans_dt,bezier_length_list,act_len,
									hmi_anim_static_set_pos_prop_table[obj_index_offset].line_sum,
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_anim_static_set_pos_prop_table[obj_index_offset].point_cnt,
									hmi_anim_static_set_pos_prop_table[obj_index_offset].pPoint_list,
									&bezier_new_pos);
					#if 0
					if(bezier_new_pos.newElapse>dt)
					{
						bezier_new_pos.newElapse-=dt;
					}
					else
					{
						bezier_new_pos.newElapse=0;
					}
					#endif														
					trans_dt	= get_trans_dt(0/*start time*/,hmi_anim_static_set_pos_anim_type[obj_index_offset],
									bezier_new_pos.newDur,bezier_new_pos.newElapse,dt/*dt*/);
					pos[HMI_START_POS].x	= bezier_new_pos.lineStart.x;
					pos[HMI_START_POS].y	= bezier_new_pos.lineStart.y;
					pos[HMI_END_POS].x		= bezier_new_pos.lineEnd.x;
					pos[HMI_END_POS].y		= bezier_new_pos.lineEnd.y;
					success		= get_next_step_anim32(pos,trans_dt,
									bezier_new_pos.newDur);						
					obj_executer= hmi_anim_static_set_pos_execute_id[obj_index_offset];
				}
			}
			#endif
		}
		else if(HMI_IS_TIMER_ACTION_S_DURATION(parent_timer_id))/*static timer and static anim*/
		{
			#if (HMI_S_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_STATIC_SET_POS_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_S_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_POS_ANIM_SXY_INDEX(obj_index);
			if((obj_index_offset < HMI_ANIM_STATIC_SET_POS_NUMBER)&&(parent_timer_id<HMI_S_TIMER_ACTION_DURATION_NUMBER))
			{
				if(hmi_anim_static_set_pos_prop_table[obj_index_offset].point_cnt==HMI_LINE_PATH_CNT)
				{
					dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
										hmi_anim_static_set_pos_anim_type[obj_index_offset],
										hmi_duration_static_prop[parent_timer_id].duration,
										hmi_duration_elapse_prop[parent_timer_id],
										dt);				
					pos[HMI_START_POS].x	= hmi_anim_static_set_pos_prop_table[obj_index_offset].pPoint_list[0].x;
					pos[HMI_START_POS].y	= hmi_anim_static_set_pos_prop_table[obj_index_offset].pPoint_list[0].y;
					pos[HMI_END_POS].x		= hmi_anim_static_set_pos_prop_table[obj_index_offset].pPoint_list[1].x;
					pos[HMI_END_POS].y		= hmi_anim_static_set_pos_prop_table[obj_index_offset].pPoint_list[1].y;
					success		= get_next_step_anim32(pos ,dt,
									hmi_duration_static_prop[parent_timer_id].duration);								
					obj_executer= hmi_anim_static_set_pos_execute_id[obj_index_offset];
				}
				else	/*Bezier*/
				{
					/*transform dt*/
					trans_dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
									hmi_anim_static_set_pos_anim_type[obj_index_offset],
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_duration_elapse_prop[parent_timer_id],
									dt);
					/*get every bezier length*/
					act_len	= get_bezier_length(bezier_length_list,HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT,
									hmi_anim_static_set_pos_prop_table[obj_index_offset].point_cnt,
									hmi_anim_static_set_pos_prop_table[obj_index_offset].pPoint_list);
					hmi_get_bezier_index(trans_dt,bezier_length_list,act_len,
									hmi_anim_static_set_pos_prop_table[obj_index_offset].line_sum,
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_anim_static_set_pos_prop_table[obj_index_offset].point_cnt,
									hmi_anim_static_set_pos_prop_table[obj_index_offset].pPoint_list,
									&bezier_new_pos);
					#if 0
					if(bezier_new_pos.newElapse>dt)
					{
						bezier_new_pos.newElapse-=dt;
					}
					else
					{
						bezier_new_pos.newElapse=0;
					}
					#endif														
					trans_dt	= get_trans_dt(0/*start time*/,hmi_anim_static_set_pos_anim_type[obj_index_offset],
									bezier_new_pos.newDur,bezier_new_pos.newElapse,dt/*dt*/);
					pos[HMI_START_POS].x	= bezier_new_pos.lineStart.x;
					pos[HMI_START_POS].y	= bezier_new_pos.lineStart.y;
					pos[HMI_END_POS].x		= bezier_new_pos.lineEnd.x;
					pos[HMI_END_POS].y		= bezier_new_pos.lineEnd.y;
					success		= get_next_step_anim32(pos,trans_dt,
									bezier_new_pos.newDur);						
					obj_executer= hmi_anim_static_set_pos_execute_id[obj_index_offset];
				}
			}
			#endif						
		}
		else
		{
		}
	}
	else
	{
	}
	if(success)
	{
		obj_index_pro	= hmi_get_obj_pro_id(obj_executer,HMI_ELEMENT_X);/*get obj_index property ID*/
		hmi_object_data	= pos[HMI_NEXT_POS].x ;
		hmi_engine_set_object_info(obj_index_pro,hmi_object_data);
		obj_index_pro	= hmi_get_obj_pro_id(obj_executer,HMI_ELEMENT_Y);/*get obj_index property ID*/
		hmi_object_data	= pos[HMI_NEXT_POS].y ;
		hmi_engine_set_object_info(obj_index_pro,hmi_object_data);
	}		
}
#endif


#if HMI_ANIM_STATIC_SET_W_H_NUMBER+HMI_ANIM_DYN_SET_W_H_NUMBER>0
void hmi_anim_set_wh_action(HMI_OBJECT_ID_STR obj_index,BOOLEAN dyn_anim,HMI_TIME dt,
							HMI_OBJECT_ID_STR parent_timer_id)
{
	BOOLEAN				success			= FALSE;
	HMI_OBJECT_DATA_STR hmi_object_data	= 0;
	HMI_OBJECT_ID_STR 	obj_index_offset= 0;	
	HMI_OBJECT_ID_STR 	obj_index_pro	= 0;
	HMI_OBJECT_ID_STR 	obj_executer	= HMI_NB_ELEMENTS;	
	POINT32_TP			pos[HMI_ANIM_POS_CNT]={{0,0},{0,0},{0,0}};
	//#ifdef bezier
	UINT32	 bezier_length_list[HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT];
	HMI_BEZIER_NEW_POS_STR	bezier_new_pos={{0,0},{0,0},0,0};
	UINT32				act_len		= 0;
	HMI_TIME			trans_dt	= 0;
	//#endif
	
	parent_timer_id	= HMI_GET_ACTION_ID_INDEX(parent_timer_id);
	if(dyn_anim&&(HMI_IS_W_H_ANIM_DXY_INDEX(obj_index)))
	{
		if(HMI_IS_TIMER_ACTION_D_DURATION(parent_timer_id))/*dyn timer and dyn anim*/
		{
			#if (HMI_DYN_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_DYN_SET_W_H_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_D_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_W_H_ANIM_DXY_INDEX(obj_index);
			if((obj_index_offset < HMI_ANIM_DYN_SET_W_H_NUMBER)&&(parent_timer_id < HMI_DYN_TIMER_ACTION_DURATION_NUMBER))
			{
				if(hmi_anim_dyn_set_w_h_prop_table[obj_index_offset].point_cnt == HMI_LINE_PATH_CNT)
				{
					dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
										hmi_anim_dyn_set_w_h_anim_type[obj_index_offset],
										hmi_dyn_duration_static_prop[parent_timer_id].duration,
										hmi_dyn_duration_elapse_prop[parent_timer_id],
										dt);				
					pos[HMI_START_POS].x =(INT32)(hmi_anim_dyn_set_w_h_prop_table[obj_index_offset].pPoint_list[0].x);
					pos[HMI_START_POS].y =(INT32)(hmi_anim_dyn_set_w_h_prop_table[obj_index_offset].pPoint_list[0].y);
					pos[HMI_END_POS].x =(INT32)(hmi_anim_dyn_set_w_h_prop_table[obj_index_offset].pPoint_list[1].x);
					pos[HMI_END_POS].y =(INT32)(hmi_anim_dyn_set_w_h_prop_table[obj_index_offset].pPoint_list[1].y);
					success		= get_next_step_anim32(pos ,dt,
									hmi_dyn_duration_static_prop[parent_timer_id].duration);												
					obj_executer= hmi_anim_dyn_w_h_execute_id[obj_index_offset];
				}
				else /*Bezier*/
				{
					/*transform dt*/
					trans_dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
									hmi_anim_dyn_set_w_h_anim_type[obj_index_offset],
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_dyn_duration_elapse_prop[parent_timer_id],
									dt);
					/*get every bezier length*/
					act_len	= get_bezier_length(bezier_length_list,HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT,
									hmi_anim_dyn_set_w_h_prop_table[obj_index_offset].point_cnt,
									hmi_anim_dyn_set_w_h_prop_table[obj_index_offset].pPoint_list);
					hmi_get_bezier_index(trans_dt,bezier_length_list,act_len,
									hmi_anim_dyn_set_w_h_prop_table[obj_index_offset].line_sum,
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_anim_dyn_set_w_h_prop_table[obj_index_offset].point_cnt,
									hmi_anim_dyn_set_w_h_prop_table[obj_index_offset].pPoint_list,
									&bezier_new_pos);
					#if 0
					if(bezier_new_pos.newElapse>dt)
					{
						bezier_new_pos.newElapse-=dt;
					}
					else
					{
						bezier_new_pos.newElapse=0;
					}
					#endif														
					trans_dt	= get_trans_dt(0/*start time*/,hmi_anim_dyn_set_w_h_anim_type[obj_index_offset],
											bezier_new_pos.newDur,bezier_new_pos.newElapse,dt/*dt*/);
					pos[HMI_START_POS].x	= bezier_new_pos.lineStart.x;
					pos[HMI_START_POS].y	= bezier_new_pos.lineStart.y;
					pos[HMI_END_POS].x		= bezier_new_pos.lineEnd.x;
					pos[HMI_END_POS].y		= bezier_new_pos.lineEnd.y;
					success		= get_next_step_anim32(pos,trans_dt,
											bezier_new_pos.newDur);						
					obj_executer= hmi_anim_dyn_w_h_execute_id[obj_index_offset];
				}
			}
			#endif
		}
		else if(HMI_IS_TIMER_ACTION_S_DURATION(parent_timer_id))/*static timer and dyn anim*/
		{
			#if (HMI_S_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_DYN_SET_W_H_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_S_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_W_H_ANIM_DXY_INDEX(obj_index);
			if((obj_index_offset < HMI_ANIM_DYN_SET_W_H_NUMBER)&&(parent_timer_id < HMI_S_TIMER_ACTION_DURATION_NUMBER))
			{
				if(hmi_anim_dyn_set_w_h_prop_table[obj_index_offset].point_cnt == HMI_LINE_PATH_CNT)
				{
					dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
										hmi_anim_dyn_set_w_h_anim_type[obj_index_offset],
										hmi_duration_static_prop[parent_timer_id].duration,
										hmi_duration_elapse_prop[parent_timer_id],
										dt);				
					pos[HMI_START_POS].x =(INT32)(hmi_anim_dyn_set_w_h_prop_table[obj_index_offset].pPoint_list[0].x);
					pos[HMI_START_POS].y =(INT32)(hmi_anim_dyn_set_w_h_prop_table[obj_index_offset].pPoint_list[0].y);
					pos[HMI_END_POS].x =(INT32)(hmi_anim_dyn_set_w_h_prop_table[obj_index_offset].pPoint_list[1].x);
					pos[HMI_END_POS].y =(INT32)(hmi_anim_dyn_set_w_h_prop_table[obj_index_offset].pPoint_list[1].y);
					success		= get_next_step_anim32(pos ,dt,
									hmi_duration_static_prop[parent_timer_id].duration);								
					obj_executer= hmi_anim_dyn_w_h_execute_id[obj_index_offset];
				}
				else /*Bezier*/
				{
					/*transform dt*/
					trans_dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
									hmi_anim_dyn_set_w_h_anim_type[obj_index_offset],
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_duration_elapse_prop[parent_timer_id],
									dt);
					/*get every bezier length*/
					act_len	= get_bezier_length(bezier_length_list,HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT,
									hmi_anim_dyn_set_w_h_prop_table[obj_index_offset].point_cnt,
									hmi_anim_dyn_set_w_h_prop_table[obj_index_offset].pPoint_list);
					hmi_get_bezier_index(trans_dt,bezier_length_list,act_len,
									hmi_anim_dyn_set_w_h_prop_table[obj_index_offset].line_sum,
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_anim_dyn_set_w_h_prop_table[obj_index_offset].point_cnt,
									hmi_anim_dyn_set_w_h_prop_table[obj_index_offset].pPoint_list,
									&bezier_new_pos);
					#if 0
					if(bezier_new_pos.newElapse>dt)
					{
						bezier_new_pos.newElapse-=dt;
					}
					else
					{
						bezier_new_pos.newElapse=0;
					}
					#endif														
					trans_dt	= get_trans_dt(0/*start time*/,hmi_anim_dyn_set_w_h_anim_type[obj_index_offset],
									bezier_new_pos.newDur,bezier_new_pos.newElapse,dt/*dt*/);
					pos[HMI_START_POS].x	= bezier_new_pos.lineStart.x;
					pos[HMI_START_POS].y	= bezier_new_pos.lineStart.y;
					pos[HMI_END_POS].x		= bezier_new_pos.lineEnd.x;
					pos[HMI_END_POS].y		= bezier_new_pos.lineEnd.y;
					success		= get_next_step_anim32(pos,trans_dt,
											bezier_new_pos.newDur);						
					obj_executer= hmi_anim_dyn_w_h_execute_id[obj_index_offset];
				}
			}
			#endif						
		}
		else
		{
		}				
	}
	else if((!dyn_anim)&&(HMI_IS_W_H_ANIM_SXY_INDEX(obj_index)))
	{
		if(HMI_IS_TIMER_ACTION_D_DURATION(parent_timer_id))/*dyn timer and static anim*/
		{
			#if (HMI_DYN_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_STATIC_SET_W_H_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_D_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_W_H_ANIM_SXY_INDEX(obj_index);
			if((obj_index_offset < HMI_ANIM_STATIC_SET_W_H_NUMBER)&&(parent_timer_id < HMI_DYN_TIMER_ACTION_DURATION_NUMBER))
			{
				if(hmi_anim_static_set_w_h_prop_table[obj_index_offset].point_cnt == HMI_LINE_PATH_CNT)
				{
					dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
										hmi_anim_static_set_w_h_anim_type[obj_index_offset],
										hmi_dyn_duration_static_prop[parent_timer_id].duration,
										hmi_dyn_duration_elapse_prop[parent_timer_id],
										dt);				
					pos[HMI_START_POS].x = hmi_anim_static_set_w_h_prop_table[obj_index_offset].pPoint_list[0].x; 
					pos[HMI_START_POS].y = hmi_anim_static_set_w_h_prop_table[obj_index_offset].pPoint_list[0].y;
					pos[HMI_END_POS].x = hmi_anim_static_set_w_h_prop_table[obj_index_offset].pPoint_list[1].x;
					pos[HMI_END_POS].y = hmi_anim_static_set_w_h_prop_table[obj_index_offset].pPoint_list[1].y;
					success		= get_next_step_anim32(pos ,dt,
									hmi_dyn_duration_static_prop[parent_timer_id].duration);								
					obj_executer= hmi_anim_static_w_h_execute_id[obj_index_offset];
				}
				else /*Bezier*/
				{
					/*transform dt*/
					trans_dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
									hmi_anim_static_set_w_h_anim_type[obj_index_offset],
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_dyn_duration_elapse_prop[parent_timer_id],
									dt);
					/*get every bezier length*/
					act_len	= get_bezier_length(bezier_length_list,HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT,
									hmi_anim_static_set_w_h_prop_table[obj_index_offset].point_cnt,
									hmi_anim_static_set_w_h_prop_table[obj_index_offset].pPoint_list);
					hmi_get_bezier_index(trans_dt,bezier_length_list,act_len,
									hmi_anim_static_set_w_h_prop_table[obj_index_offset].line_sum,
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_anim_static_set_w_h_prop_table[obj_index_offset].point_cnt,
									hmi_anim_static_set_w_h_prop_table[obj_index_offset].pPoint_list,
									&bezier_new_pos);
					#if 0
					if(bezier_new_pos.newElapse>dt)
					{
						bezier_new_pos.newElapse-=dt;
					}
					else
					{
						bezier_new_pos.newElapse=0;
					}
					#endif														
					trans_dt	= get_trans_dt(0/*start time*/,hmi_anim_static_set_w_h_anim_type[obj_index_offset],
									bezier_new_pos.newDur,bezier_new_pos.newElapse,dt/*dt*/);
					pos[HMI_START_POS].x	= bezier_new_pos.lineStart.x;
					pos[HMI_START_POS].y	= bezier_new_pos.lineStart.y;
					pos[HMI_END_POS].x		= bezier_new_pos.lineEnd.x;
					pos[HMI_END_POS].y		= bezier_new_pos.lineEnd.y;
					success		= get_next_step_anim32(pos,trans_dt,
									bezier_new_pos.newDur);						
					obj_executer= hmi_anim_static_w_h_execute_id[obj_index_offset];
				}
			}
			#endif
		}
		else if(HMI_IS_TIMER_ACTION_S_DURATION(parent_timer_id))/*static timer and static anim*/
		{
			#if (HMI_S_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_STATIC_SET_W_H_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_S_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_W_H_ANIM_SXY_INDEX(obj_index);
			if((obj_index_offset < HMI_ANIM_STATIC_SET_W_H_NUMBER)&&(parent_timer_id < HMI_S_TIMER_ACTION_DURATION_NUMBER))
			{
				if(hmi_anim_static_set_w_h_prop_table[obj_index_offset].point_cnt == HMI_LINE_PATH_CNT)
				{
					dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
										hmi_anim_static_set_w_h_anim_type[obj_index_offset],
										hmi_duration_static_prop[parent_timer_id].duration,
										hmi_duration_elapse_prop[parent_timer_id],
										dt);				
					pos[HMI_START_POS].x = hmi_anim_static_set_w_h_prop_table[obj_index_offset].pPoint_list[0].x;
					pos[HMI_START_POS].y = hmi_anim_static_set_w_h_prop_table[obj_index_offset].pPoint_list[0].y;
					pos[HMI_END_POS].x = hmi_anim_static_set_w_h_prop_table[obj_index_offset].pPoint_list[1].x;
					pos[HMI_END_POS].y = hmi_anim_static_set_w_h_prop_table[obj_index_offset].pPoint_list[1].y;
					success		= get_next_step_anim32(pos ,dt,
									hmi_duration_static_prop[parent_timer_id].duration);								
					obj_executer= hmi_anim_static_w_h_execute_id[obj_index_offset];
				}
				else /*Bezier*/
				{
					/*transform dt*/
					trans_dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
									hmi_anim_static_set_w_h_anim_type[obj_index_offset],
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_duration_elapse_prop[parent_timer_id],
									dt);
					/*get every bezier length*/
					act_len	= get_bezier_length(bezier_length_list,HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT,
									hmi_anim_static_set_w_h_prop_table[obj_index_offset].point_cnt,
									hmi_anim_static_set_w_h_prop_table[obj_index_offset].pPoint_list);
					hmi_get_bezier_index(trans_dt,bezier_length_list,act_len,
									hmi_anim_static_set_w_h_prop_table[obj_index_offset].line_sum,
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_anim_static_set_w_h_prop_table[obj_index_offset].point_cnt,
									hmi_anim_static_set_w_h_prop_table[obj_index_offset].pPoint_list,
									&bezier_new_pos);
					#if 0
					if(bezier_new_pos.newElapse>dt)
					{
						bezier_new_pos.newElapse-=dt;
					}
					else
					{
						bezier_new_pos.newElapse=0;
					}
					#endif														
					trans_dt	= get_trans_dt(0/*start time*/,hmi_anim_static_set_w_h_anim_type[obj_index_offset],
									bezier_new_pos.newDur,bezier_new_pos.newElapse,dt/*dt*/);
					pos[HMI_START_POS].x	= bezier_new_pos.lineStart.x;
					pos[HMI_START_POS].y	= bezier_new_pos.lineStart.y;
					pos[HMI_END_POS].x		= bezier_new_pos.lineEnd.x;
					pos[HMI_END_POS].y		= bezier_new_pos.lineEnd.y;
					success		= get_next_step_anim32(pos,trans_dt,
												bezier_new_pos.newDur);						
					obj_executer= hmi_anim_static_w_h_execute_id[obj_index_offset];
				}
			}
			#endif						
		}
		else
		{
		}
	}
	else
	{
	}
	if(success)
	{
		obj_index_pro	= hmi_get_obj_pro_id(obj_executer,HMI_ELEMENT_W);/*get obj_index property ID*/
		hmi_object_data	= (HMI_OBJECT_DATA_STR)(pos[HMI_NEXT_POS].x );
		hmi_engine_set_object_info(obj_index_pro,hmi_object_data);
		obj_index_pro	= hmi_get_obj_pro_id(obj_executer,HMI_ELEMENT_H);/*get obj_index property ID*/
		hmi_object_data	= (HMI_OBJECT_DATA_STR)(pos[HMI_NEXT_POS].y );
		hmi_engine_set_object_info(obj_index_pro,hmi_object_data);
	}		
}
#endif

#if HMI_ANIM_STATIC_SET_FOR_COLOR_NUMBER+HMI_ANIM_DYN_SET_FOR_COLOR_NUMBER>0
void hmi_anim_set_fcolor_action(HMI_OBJECT_ID_STR obj_index,BOOLEAN dyn_anim,HMI_TIME dt,
							HMI_OBJECT_ID_STR parent_timer_id)
{
	BOOLEAN				success			= FALSE;
	HMI_OBJECT_DATA_STR hmi_object_data	= 0;
	HMI_OBJECT_ID_STR 	obj_index_offset= 0;
	HMI_OBJECT_ID_STR 	obj_index_pro	= 0;
	HMI_OBJECT_ID_STR 	obj_executer	= HMI_NB_ELEMENTS;	
	POINT32_TP			pos[HMI_ANIM_POS_CNT]={{0,0},{0,0},{0,0}};
	
	parent_timer_id	= HMI_GET_ACTION_ID_INDEX(parent_timer_id);
	if(dyn_anim&&(HMI_IS_FOR_COLOR_ANIM_DXY(obj_index)))
	{
		if(HMI_IS_TIMER_ACTION_D_DURATION(parent_timer_id))/*dyn timer and dyn anim*/
		{
			#if (HMI_DYN_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_DYN_SET_FOR_COLOR_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_D_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_FOR_COLOR_ANIM_DXY_INDEX(obj_index);
			if((obj_index_offset < HMI_ANIM_DYN_SET_FOR_COLOR_NUMBER)&&(parent_timer_id < HMI_DYN_TIMER_ACTION_DURATION_NUMBER))
			{
				dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
									hmi_anim_dyn_set_for_color_anim_type[obj_index_offset],
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_dyn_duration_elapse_prop[parent_timer_id],
									dt);				
				pos[HMI_START_POS].x= (INT32)(hmi_anim_dyn_set_for_color_prop_table[obj_index_offset].pPoint_list[0].y);				
				pos[HMI_END_POS].x	= (INT32)(hmi_anim_dyn_set_for_color_prop_table[obj_index_offset].pPoint_list[1].y);				
				success		= get_next_step_anim32(pos ,dt,
								hmi_dyn_duration_static_prop[parent_timer_id].duration);												
				obj_executer= hmi_anim_dyn_for_color_execute_id[obj_index_offset];
							
			}
			#endif
		}
		else if(HMI_IS_TIMER_ACTION_S_DURATION(parent_timer_id))/*static timer and dyn anim*/
		{
			#if (HMI_S_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_DYN_SET_FOR_COLOR_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_S_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_FOR_COLOR_ANIM_DXY_INDEX(obj_index);
			if((obj_index_offset < HMI_ANIM_DYN_SET_FOR_COLOR_NUMBER)&&(parent_timer_id < HMI_S_TIMER_ACTION_DURATION_NUMBER))
			{
				dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
									hmi_anim_dyn_set_for_color_anim_type[obj_index_offset],
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_duration_elapse_prop[parent_timer_id],
									dt);				
				pos[HMI_START_POS].x	= (INT32)(hmi_anim_dyn_set_for_color_prop_table[obj_index_offset].pPoint_list[0].y);				
				pos[HMI_END_POS].x		= (INT32)(hmi_anim_dyn_set_for_color_prop_table[obj_index_offset].pPoint_list[1].y);				
				success		= get_next_step_anim32(pos ,dt,
								hmi_duration_static_prop[parent_timer_id].duration);												
				obj_executer= hmi_anim_dyn_for_color_execute_id[obj_index_offset];
			}
			#endif						
		}
		else
		{
		}				
	}
	else if((!dyn_anim)&&(HMI_IS_FOR_COLOR_ANIM_SXY(obj_index)))
	{
		if(HMI_IS_TIMER_ACTION_D_DURATION(parent_timer_id))/*dyn timer and static anim*/
		{
			#if (HMI_DYN_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_STATIC_SET_FOR_COLOR_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_D_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_FOR_COLOR_ANIM_SXY_INDEX(obj_index);
			if((obj_index_offset < HMI_ANIM_STATIC_SET_FOR_COLOR_NUMBER)&&(parent_timer_id < HMI_DYN_TIMER_ACTION_DURATION_NUMBER))
			{
				dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
									hmi_anim_static_set_for_color_anim_type[obj_index_offset],
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_dyn_duration_elapse_prop[parent_timer_id],
									dt);				
				pos[HMI_START_POS].x =(INT32)(hmi_anim_static_set_for_color_prop_table[obj_index_offset].pPoint_list[0].y);				
				pos[HMI_END_POS].x =(INT32)(hmi_anim_static_set_for_color_prop_table[obj_index_offset].pPoint_list[1].y);	
				success		= get_next_step_anim32(pos ,dt,
								hmi_dyn_duration_static_prop[parent_timer_id].duration);								
				obj_executer= hmi_anim_static_fcolor_execute_id[obj_index_offset];
			}
			#endif
		}
		else if(HMI_IS_TIMER_ACTION_S_DURATION(parent_timer_id))/*static timer and static anim*/
		{
			#if (HMI_S_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_STATIC_SET_FOR_COLOR_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_S_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_FOR_COLOR_ANIM_SXY_INDEX(obj_index);
			if((obj_index_offset < HMI_ANIM_STATIC_SET_FOR_COLOR_NUMBER)&&(parent_timer_id < HMI_S_TIMER_ACTION_DURATION_NUMBER))
			{
				dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
									hmi_anim_static_set_for_color_anim_type[obj_index_offset],
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_duration_elapse_prop[parent_timer_id],
									dt);				
				pos[HMI_START_POS].x	= (INT32)(hmi_anim_static_set_for_color_prop_table[obj_index_offset].pPoint_list[0].y);				
				pos[HMI_END_POS].x		= (INT32)(hmi_anim_static_set_for_color_prop_table[obj_index_offset].pPoint_list[1].y);	
				success		= get_next_step_anim32(pos ,dt,
								hmi_duration_static_prop[parent_timer_id].duration);								
				obj_executer= hmi_anim_static_fcolor_execute_id[obj_index_offset];
			}
			#endif						
		}
		else
		{
		}
	}
	else
	{
	}
	if(success)
	{
		obj_index_pro	= hmi_get_obj_pro_id(obj_executer,HMI_ELEMENT_FCOLOR);/*get obj_index property ID*/
		hmi_object_data	= (HMI_OBJECT_DATA_STR)(pos[HMI_NEXT_POS].x);
		hmi_engine_set_object_info(obj_index_pro,hmi_object_data);		
	}		
}
#endif

#if HMI_ANIM_STATIC_SET_IMAGELIST_INDEX_NUMBER+HMI_ANIM_DYN_SET_IMAGELIST_INDEX_NUMBER > 0
void hmi_anim_set_imglist_action(HMI_OBJECT_ID_STR obj_index,BOOLEAN dyn_anim,HMI_TIME dt,
							HMI_OBJECT_ID_STR parent_timer_id)
{
	BOOLEAN				success			= FALSE;
	HMI_OBJECT_DATA_STR hmi_object_data	= 0;
	HMI_OBJECT_ID_STR 	obj_index_offset= 0;
	HMI_OBJECT_ID_STR 	obj_executer	= HMI_NB_ELEMENTS;	
	POINT32_TP			pos[HMI_ANIM_POS_CNT]={{0,0},{0,0},{0,0}};

	parent_timer_id	= HMI_GET_ACTION_ID_INDEX(parent_timer_id);
	if(dyn_anim&&(HMI_IS_IMAGELIST_INDEX_ANIM_DXY(obj_index)))
	{
		if(HMI_IS_TIMER_ACTION_D_DURATION(parent_timer_id))/*dyn timer and dyn anim*/
		{
			#if (HMI_DYN_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_DYN_SET_IMAGELIST_INDEX_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_D_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_IMAGELIST_INDEX_ANIM_DXY(obj_index);
			if((obj_index_offset < HMI_ANIM_DYN_SET_IMAGELIST_INDEX_NUMBER)&&(parent_timer_id < HMI_DYN_TIMER_ACTION_DURATION_NUMBER))
			{
				dt=get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
									hmi_anim_dyn_set_imglist_index_anim_type[obj_index_offset],
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_dyn_duration_elapse_prop[parent_timer_id],
									dt);				
				pos[HMI_START_POS].x	= (INT32)(hmi_anim_dyn_set_imagelist_index_prop_table[obj_index_offset].pPoint_list[0].y);				
				pos[HMI_END_POS].x		= (INT32)(hmi_anim_dyn_set_imagelist_index_prop_table[obj_index_offset].pPoint_list[1].y);				
				success		= get_next_step_anim32(pos ,dt,
								hmi_dyn_duration_static_prop[parent_timer_id].duration);												
				obj_executer= hmi_anim_dyn_set_imglist_index_execute_id[obj_index_offset];
							
			}
			#endif
		}
		else if(HMI_IS_TIMER_ACTION_S_DURATION(parent_timer_id))/*static timer and dyn anim*/
		{
			#if (HMI_S_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_DYN_SET_IMAGELIST_INDEX_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_S_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_IMAGELIST_INDEX_ANIM_DXY(obj_index);
			if((obj_index_offset < HMI_ANIM_DYN_SET_IMAGELIST_INDEX_NUMBER)&&(parent_timer_id < HMI_S_TIMER_ACTION_DURATION_NUMBER))
			{
				dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
									hmi_anim_dyn_set_imglist_index_anim_type[obj_index_offset],
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_duration_elapse_prop[parent_timer_id],
									dt);				
				pos[HMI_START_POS].x	= (INT32)(hmi_anim_dyn_set_imagelist_index_prop_table[obj_index_offset].pPoint_list[0].y);				
				pos[HMI_END_POS].x		= (INT32)(hmi_anim_dyn_set_imagelist_index_prop_table[obj_index_offset].pPoint_list[1].y);				
				success		= get_next_step_anim32(pos ,dt,
								hmi_duration_static_prop[parent_timer_id].duration);												
				obj_executer= hmi_anim_dyn_set_imglist_index_execute_id[obj_index_offset];
			}
			#endif						
		}
		else
		{
		}				
	}
	else if((!dyn_anim)&&(HMI_IS_IMAGELIST_INDEX_ANIM_SXY(obj_index))) 
	{
		if(HMI_IS_TIMER_ACTION_D_DURATION(parent_timer_id))/*dyn timer and static anim*/
		{
			#if (HMI_DYN_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_STATIC_SET_IMAGELIST_INDEX_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_D_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_IMAGELIST_INDEX_ANIM_SXY(obj_index);
			if((obj_index_offset < HMI_ANIM_STATIC_SET_IMAGELIST_INDEX_NUMBER)&&(parent_timer_id < HMI_DYN_TIMER_ACTION_DURATION_NUMBER))
			{
				dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
									hmi_anim_static_setimglist_index_anim_type[obj_index_offset],
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_dyn_duration_elapse_prop[parent_timer_id],
									dt);				
				pos[HMI_START_POS].x =(INT32)(hmi_anim_static_set_imagelist_index_prop_table[obj_index_offset].pPoint_list[0].y);				
				pos[HMI_END_POS].x =(INT32)(hmi_anim_static_set_imagelist_index_prop_table[obj_index_offset].pPoint_list[1].y);				
				success=get_next_step_anim32(pos ,dt,
								hmi_dyn_duration_static_prop[parent_timer_id].duration);								
				obj_executer=hmi_anim_static_imglist_index_execute_id[obj_index_offset];
			}
			#endif
		}
		else if(HMI_IS_TIMER_ACTION_S_DURATION(parent_timer_id))/*static timer and static anim*/
		{
			#if (HMI_S_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_STATIC_SET_IMAGELIST_INDEX_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_S_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_IMAGELIST_INDEX_ANIM_SXY(obj_index);
			if((obj_index_offset < HMI_ANIM_STATIC_SET_IMAGELIST_INDEX_NUMBER)&&(parent_timer_id < HMI_S_TIMER_ACTION_DURATION_NUMBER))
			{
				dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
									hmi_anim_static_setimglist_index_anim_type[obj_index_offset],
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_duration_elapse_prop[parent_timer_id],
									dt);				
				pos[HMI_START_POS].x	= (INT32)(hmi_anim_static_set_imagelist_index_prop_table[obj_index_offset].pPoint_list[0].y);				
				pos[HMI_END_POS].x		= (INT32)(hmi_anim_static_set_imagelist_index_prop_table[obj_index_offset].pPoint_list[1].y);	
				success=get_next_step_anim32(pos ,dt,
								hmi_duration_static_prop[parent_timer_id].duration);								
				obj_executer=hmi_anim_static_imglist_index_execute_id[obj_index_offset];
			}
			#endif						
		}
		else
		{
		}
	}
	else
	{
	}
	if(success)
	{		
		hmi_object_data	= (HMI_OBJECT_DATA_STR)(pos[HMI_NEXT_POS].x );
		hmi_engine_set_object_info(obj_executer,hmi_object_data);/*obj_executer mean  image list index.*/
	}		
}
#endif
#if HMI_ANIM_STATIC_SET_SCALE_NUMBER+HMI_ANIM_DYN_SET_SCALE_NUMBER > 0
#ifndef HMI_GRAPHIC_ST7513 //RGL OPENGL
void hmi_anim_set_scale_action(HMI_OBJECT_ID_STR obj_index,BOOLEAN dyn_anim,HMI_TIME dt,
							HMI_OBJECT_ID_STR parent_timer_id)
{
	BOOLEAN				success			= FALSE;
	HMI_OBJECT_DATA_STR hmi_object_data	= 0U;
	HMI_OBJECT_ID_STR	obj_index_offset= 0;
	HMI_OBJECT_ID_STR	obj_index_pro	= 0;	
	HMI_OBJECT_ID_STR	obj_executer	= HMI_NB_ELEMENTS;	
	POINT_FLOAT_TP			pos[HMI_ANIM_POS_CNT]={{0,0},{0,0},{0,0}};
	//#ifdef bezier
	UINT32	 bezier_length_list[HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT];
	HMI_BEZIER_NEW_POS_STR	bezier_new_pos={{0,0},{0,0},0,0};
	UINT32				act_len		= 0;
	HMI_TIME			trans_dt	= 0;
	//#endif
	parent_timer_id	= HMI_GET_ACTION_ID_INDEX(parent_timer_id);
	if((dyn_anim==TRUE)&&(HMI_IS_SCALE_INDEX_ANIM_DXY(obj_index)))
	{
		if(HMI_IS_TIMER_ACTION_D_DURATION(parent_timer_id))/*dyn timer and dyn anim*/
		{
			#if (HMI_DYN_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_DYN_SET_SCALE_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_D_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_SCALE_INDEX_ANIM_DXY(obj_index);
			if((obj_index_offset < HMI_ANIM_DYN_SET_SCALE_NUMBER)&&(parent_timer_id < HMI_DYN_TIMER_ACTION_DURATION_NUMBER))
			{
				if(hmi_anim_dyn_set_scale_prop_table[obj_index_offset].point_cnt == HMI_LINE_PATH_CNT)
				{
					dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
										hmi_anim_dyn_set_scale_anim_type[obj_index_offset],
										hmi_dyn_duration_static_prop[parent_timer_id].duration,
										hmi_dyn_duration_elapse_prop[parent_timer_id],
										dt);				
					pos[HMI_START_POS].x	= (SCALE_TYPE)(hmi_anim_dyn_set_scale_prop_table[obj_index_offset].pPoint_list[0].y);				
					pos[HMI_END_POS].x		= (SCALE_TYPE)(hmi_anim_dyn_set_scale_prop_table[obj_index_offset].pPoint_list[1].y);				
					success		= get_next_step_animfloat(pos ,dt,
									hmi_dyn_duration_static_prop[parent_timer_id].duration);												
					obj_executer= hmi_anim_dyn_set_scale_execute_id[obj_index_offset];
				}
				else	/*Bezier*/
				{
					/*transform dt*/
					trans_dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
									hmi_anim_dyn_set_scale_anim_type[obj_index_offset],
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_dyn_duration_elapse_prop[parent_timer_id],
									dt);
					/*get every bezier length*/
					act_len	= get_bezier_length_f(bezier_length_list,HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT,
									hmi_anim_dyn_set_scale_prop_table[obj_index_offset].point_cnt,
									hmi_anim_dyn_set_scale_prop_table[obj_index_offset].pPoint_list);
					hmi_get_bezier_index_f(trans_dt,bezier_length_list,act_len,
									hmi_anim_dyn_set_scale_prop_table[obj_index_offset].line_sum,
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_anim_dyn_set_scale_prop_table[obj_index_offset].point_cnt,
									hmi_anim_dyn_set_scale_prop_table[obj_index_offset].pPoint_list,
									&bezier_new_pos);
					#if 0
					if(bezier_new_pos.newElapse>dt)
					{
						bezier_new_pos.newElapse-=dt;
					}
					else
					{
						bezier_new_pos.newElapse=0;
					}
					#endif														
					trans_dt	= get_trans_dt(0/*start time*/,hmi_anim_dyn_set_scale_anim_type[obj_index_offset],
									bezier_new_pos.newDur,bezier_new_pos.newElapse,dt/*dt*/);
					pos[HMI_START_POS].x	= (float_32)(bezier_new_pos.lineStart.x);
					pos[HMI_START_POS].y	= (float_32)(bezier_new_pos.lineStart.y);
					pos[HMI_END_POS].x		= (float_32)(bezier_new_pos.lineEnd.x);
					pos[HMI_END_POS].y		= (float_32)(bezier_new_pos.lineEnd.y);
					success		= get_next_step_animfloat(pos,trans_dt,
									bezier_new_pos.newDur);						
					obj_executer= hmi_anim_dyn_set_scale_execute_id[obj_index_offset];
				}
			}
			#endif
		}
		else if(HMI_IS_TIMER_ACTION_S_DURATION(parent_timer_id))/*static timer and dyn anim*/
		{
			#if (HMI_S_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_DYN_SET_SCALE_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_S_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_SCALE_INDEX_ANIM_DXY(obj_index);
			if((obj_index_offset < HMI_ANIM_DYN_SET_SCALE_NUMBER)&&(parent_timer_id < HMI_S_TIMER_ACTION_DURATION_NUMBER))
			{
				if(hmi_anim_dyn_set_scale_prop_table[obj_index_offset].point_cnt == HMI_LINE_PATH_CNT)
				{
					dt=get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
										hmi_anim_dyn_set_scale_anim_type[obj_index_offset],
										hmi_duration_static_prop[parent_timer_id].duration,
										hmi_duration_elapse_prop[parent_timer_id],
										dt);				
					pos[HMI_START_POS].x	= (SCALE_TYPE)(hmi_anim_dyn_set_scale_prop_table[obj_index_offset].pPoint_list[0].y);				
					pos[HMI_END_POS].x		= (SCALE_TYPE)(hmi_anim_dyn_set_scale_prop_table[obj_index_offset].pPoint_list[1].y);
					success		= get_next_step_animfloat(pos ,dt,
									hmi_duration_static_prop[parent_timer_id].duration);												
					obj_executer= hmi_anim_dyn_set_scale_execute_id[obj_index_offset];
				}
				else /*Bezier*/
				{
					/*transform dt*/
					trans_dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
									hmi_anim_dyn_set_scale_anim_type[obj_index_offset],
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_duration_elapse_prop[parent_timer_id],
									dt);
					/*get every bezier length*/
					act_len	= get_bezier_length_f(bezier_length_list,HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT,
									hmi_anim_dyn_set_scale_prop_table[obj_index_offset].point_cnt,
									hmi_anim_dyn_set_scale_prop_table[obj_index_offset].pPoint_list);
					hmi_get_bezier_index_f(trans_dt,bezier_length_list,act_len,
									hmi_anim_dyn_set_scale_prop_table[obj_index_offset].line_sum,
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_anim_dyn_set_scale_prop_table[obj_index_offset].point_cnt,
									hmi_anim_dyn_set_scale_prop_table[obj_index_offset].pPoint_list,
									&bezier_new_pos);
					#if 0
					if(bezier_new_pos.newElapse>dt)
					{
						bezier_new_pos.newElapse-=dt;
					}
					else
					{
						bezier_new_pos.newElapse=0;
					}
					#endif														
					trans_dt	= get_trans_dt(0/*start time*/,hmi_anim_dyn_set_scale_anim_type[obj_index_offset],
									bezier_new_pos.newDur,bezier_new_pos.newElapse,dt/*dt*/);
					pos[HMI_START_POS].x	= (float_32)(bezier_new_pos.lineStart.x);
					pos[HMI_START_POS].y	= (float_32)(bezier_new_pos.lineStart.y);
					pos[HMI_END_POS].x		= (float_32)(bezier_new_pos.lineEnd.x);
					pos[HMI_END_POS].y		= (float_32)(bezier_new_pos.lineEnd.y);
					success		= get_next_step_animfloat(pos,trans_dt,
									bezier_new_pos.newDur);						
					obj_executer= hmi_anim_dyn_set_scale_execute_id[obj_index_offset];
					/*convert y to alpha*/
					if(pos[HMI_NEXT_POS].y < 0)
					{
						pos[HMI_NEXT_POS].y	= 0;
					}
					pos[HMI_NEXT_POS].y	= (SCALE_TYPE)(((pos[HMI_NEXT_POS].y)/((float_32)HMI_LAYER_0_MAX_H_LENGTH))*HMI_MAX_ALPHA_VALUE+0.5f);
					if(pos[HMI_NEXT_POS].y > HMI_MAX_ALPHA_VALUE)
					{
						pos[HMI_NEXT_POS].y	= HMI_MAX_ALPHA_VALUE;
					}
				}
			}
			#endif						
		}
		else
		{
		}				
	}
	else if((!dyn_anim)&&(HMI_IS_SCALE_INDEX_ANIM_SXY(obj_index))) 
	{
		if(HMI_IS_TIMER_ACTION_D_DURATION(parent_timer_id))/*dyn timer and static anim*/
		{
			#if (HMI_DYN_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_STATIC_SET_SCALE_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_D_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_SCALE_INDEX_ANIM_SXY(obj_index);
			if((obj_index_offset < HMI_ANIM_STATIC_SET_SCALE_NUMBER)&&(parent_timer_id < HMI_DYN_TIMER_ACTION_DURATION_NUMBER))
			{
				if(hmi_anim_static_set_scale_prop_table[obj_index_offset].point_cnt == HMI_LINE_PATH_CNT )
				{
					dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
										hmi_anim_static_set_scale_anim_type[obj_index_offset],
										hmi_dyn_duration_static_prop[parent_timer_id].duration,
										hmi_dyn_duration_elapse_prop[parent_timer_id],
										dt);				
					pos[HMI_START_POS].x	= (SCALE_TYPE)(hmi_anim_static_set_scale_prop_table[obj_index_offset].pPoint_list[0].y);				
					pos[HMI_END_POS].x		= (SCALE_TYPE)(hmi_anim_static_set_scale_prop_table[obj_index_offset].pPoint_list[1].y);				
					success		= get_next_step_animfloat(pos ,dt,
									hmi_dyn_duration_static_prop[parent_timer_id].duration);								
					obj_executer= hmi_anim_static_set_scale_execute_id[obj_index_offset];
				}
				else /*Bezier*/
				{
					/*transform dt*/
					trans_dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
									hmi_anim_static_set_scale_anim_type[obj_index_offset],
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_dyn_duration_elapse_prop[parent_timer_id],
									dt);
					/*get every bezier length*/
					act_len	= get_bezier_length_f(bezier_length_list,HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT,
									hmi_anim_static_set_scale_prop_table[obj_index_offset].point_cnt,
									hmi_anim_static_set_scale_prop_table[obj_index_offset].pPoint_list);
					hmi_get_bezier_index_f(trans_dt,bezier_length_list,act_len,
									hmi_anim_static_set_scale_prop_table[obj_index_offset].line_sum,
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_anim_static_set_scale_prop_table[obj_index_offset].point_cnt,
									hmi_anim_static_set_scale_prop_table[obj_index_offset].pPoint_list,
									&bezier_new_pos);
					#if 0
					if(bezier_new_pos.newElapse>dt)
					{
						bezier_new_pos.newElapse-=dt;
					}
					else
					{
						bezier_new_pos.newElapse=0;
					}
					#endif														
					trans_dt	= get_trans_dt(0/*start time*/,hmi_anim_static_set_scale_anim_type[obj_index_offset],
												bezier_new_pos.newDur,bezier_new_pos.newElapse,dt/*dt*/);
					pos[HMI_START_POS].x	= (float_32)(bezier_new_pos.lineStart.x);//2020 05 09
					pos[HMI_START_POS].y	= (float_32)(bezier_new_pos.lineStart.y);//2020 05 09
					pos[HMI_END_POS].x		= (float_32)(bezier_new_pos.lineEnd.x);//2020 05 09
					pos[HMI_END_POS].y		= (float_32)(bezier_new_pos.lineEnd.y);//2020 05 09
					success		= get_next_step_animfloat(pos,trans_dt,
									bezier_new_pos.newDur);						
					obj_executer= hmi_anim_static_set_scale_execute_id[obj_index_offset];
					/*convert y to alpha*/
					if(pos[HMI_NEXT_POS].y < 0)
					{
						pos[HMI_NEXT_POS].y	= 0;
					}
					pos[HMI_NEXT_POS].y	= (SCALE_TYPE)(((pos[HMI_NEXT_POS].y)/((float_32)HMI_LAYER_0_MAX_H_LENGTH))*HMI_MAX_ALPHA_VALUE+0.5);
					if(pos[HMI_NEXT_POS].y > HMI_MAX_ALPHA_VALUE)
					{
						pos[HMI_NEXT_POS].y	= HMI_MAX_ALPHA_VALUE;
					}
				}
			}
			#endif
		}
		else if(HMI_IS_TIMER_ACTION_S_DURATION(parent_timer_id))/*static timer and static anim*/
		{
			#if (HMI_S_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_STATIC_SET_SCALE_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_S_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_SCALE_INDEX_ANIM_SXY(obj_index);
			if((obj_index_offset < HMI_ANIM_STATIC_SET_SCALE_NUMBER)&&(parent_timer_id < HMI_S_TIMER_ACTION_DURATION_NUMBER))
			{
				if(hmi_anim_static_set_scale_prop_table[obj_index_offset].point_cnt == HMI_LINE_PATH_CNT )
				{
					dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
										hmi_anim_static_set_scale_anim_type[obj_index_offset],
										hmi_duration_static_prop[parent_timer_id].duration,
										hmi_duration_elapse_prop[parent_timer_id],
										dt);				
					pos[HMI_START_POS].x	= (SCALE_TYPE)(hmi_anim_static_set_scale_prop_table[obj_index_offset].pPoint_list[0].y);				
					pos[HMI_END_POS].x		= (SCALE_TYPE)(hmi_anim_static_set_scale_prop_table[obj_index_offset].pPoint_list[1].y);	
					success		= get_next_step_animfloat(pos ,dt,
									hmi_duration_static_prop[parent_timer_id].duration);								
					obj_executer= hmi_anim_static_set_scale_execute_id[obj_index_offset];
				}
				else /*Bezier*/
				{
					/*transform dt*/
					trans_dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
									hmi_anim_static_set_scale_anim_type[obj_index_offset],
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_duration_elapse_prop[parent_timer_id],
									dt);
					/*get every bezier length*/
					act_len	= get_bezier_length_f(bezier_length_list,HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT,
									hmi_anim_static_set_scale_prop_table[obj_index_offset].point_cnt,
									hmi_anim_static_set_scale_prop_table[obj_index_offset].pPoint_list);
					hmi_get_bezier_index_f(trans_dt,bezier_length_list,act_len,
									hmi_anim_static_set_scale_prop_table[obj_index_offset].line_sum,
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_anim_static_set_scale_prop_table[obj_index_offset].point_cnt,
									hmi_anim_static_set_scale_prop_table[obj_index_offset].pPoint_list,
									&bezier_new_pos);
					#if 0
					if(bezier_new_pos.newElapse>dt)
					{
						bezier_new_pos.newElapse-=dt;
					}
					else
					{
						bezier_new_pos.newElapse=0;
					}
					#endif														
					trans_dt	= get_trans_dt(0/*start time*/,hmi_anim_static_set_scale_anim_type[obj_index_offset],
												bezier_new_pos.newDur,bezier_new_pos.newElapse,dt/*dt*/);
					
					pos[HMI_START_POS].x	= (float_32)(bezier_new_pos.lineStart.x);//2020 05 09
					pos[HMI_START_POS].y	= (float_32)(bezier_new_pos.lineStart.y);//2020 05 09
					pos[HMI_END_POS].x		= (float_32)(bezier_new_pos.lineEnd.x);//2020 05 09
					pos[HMI_END_POS].y		= (float_32)(bezier_new_pos.lineEnd.y);//2020 05 09
					success		= get_next_step_animfloat(pos,trans_dt,
														bezier_new_pos.newDur);						
					obj_executer= hmi_anim_static_set_scale_execute_id[obj_index_offset];
					/*convert y to alpha*/
					if(pos[HMI_NEXT_POS].y < 0)
					{
						pos[HMI_NEXT_POS].y	= 0;
					}
					pos[HMI_NEXT_POS].y	= (float_32)(((pos[HMI_NEXT_POS].y)/((float_32)HMI_LAYER_0_MAX_H_LENGTH))*HMI_MAX_ALPHA_VALUE+0.5f);//2020 05 09
					if(pos[HMI_NEXT_POS].y > HMI_MAX_ALPHA_VALUE)
					{
						pos[HMI_NEXT_POS].y	= HMI_MAX_ALPHA_VALUE;
					}
				}
			}
			#endif						
		}
		else
		{
		}
	}
	else
	{
	}
	if(success)
	{		
		obj_index_pro	= hmi_get_obj_pro_id(obj_executer,HMI_ELEMENT_SCALE);/*get obj_index property ID*/
		hmi_object_data	= (HMI_OBJECT_DATA_STR)(HMI_F32_TO_U32(pos[HMI_NEXT_POS].x));
		hmi_engine_set_object_info(obj_index_pro,hmi_object_data);
	}		
}
#endif
#endif

#if HMI_ANIM_DYN_SET_P1_NUMBER+HMI_ANIM_STATIC_SET_P1_NUMBER > 0
#ifndef HMI_GRAPHIC_ST7513 //RGL OPENGL
void hmi_anim_set_p1_action(HMI_OBJECT_ID_STR obj_index,BOOLEAN dyn_anim,HMI_TIME dt,
							HMI_OBJECT_ID_STR parent_timer_id)
{
	BOOLEAN				success			= FALSE;
	HMI_OBJECT_DATA_STR hmi_object_data	= 0;
	HMI_OBJECT_ID_STR	obj_index_offset= 0;
	HMI_OBJECT_ID_STR	obj_index_pro	= 0;
	HMI_OBJECT_ID_STR	obj_executer	= HMI_NB_ELEMENTS;	
	POINT_FLOAT_TP		pos[HMI_ANIM_POS_CNT]={{0,0},{0,0},{0,0}};
	UINT32	 bezier_length_list[HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT];
	HMI_BEZIER_NEW_POS_STR	bezier_new_pos={{0,0},{0,0},0,0};
	UINT32				act_len		= 0;
	HMI_TIME			trans_dt	= 0;
	HMI_CUSTOM_INFO_STR CONST*	phmi_custom_widget_info	= NULL;
	HMI_OBJECT_ID_STR	custom_lib_index			= 0U;
	HMI_OBJECT_ID_STR	hmi_object_index			= 0U;
	BOOLEAN 			get_success			= FALSE;
	
	parent_timer_id	= HMI_GET_ACTION_ID_INDEX(parent_timer_id);
	if(dyn_anim&&(HMI_IS_P1_INDEX_ANIM_DXY(obj_index)))
	{
		if(HMI_IS_TIMER_ACTION_D_DURATION(parent_timer_id))/*dyn timer and dyn anim*/
		{
			#if (HMI_DYN_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_DYN_SET_P1_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_D_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_P1_INDEX_ANIM_DXY(obj_index);
			if((obj_index_offset < HMI_ANIM_DYN_SET_P1_NUMBER)&&(parent_timer_id < HMI_DYN_TIMER_ACTION_DURATION_NUMBER))
			{
				if(hmi_anim_dyn_set_p1_prop_table[obj_index_offset].point_cnt == HMI_LINE_PATH_CNT )
				{
					dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
										hmi_anim_dyn_set_p1_anim_type[obj_index_offset],
										hmi_dyn_duration_static_prop[parent_timer_id].duration,
										hmi_dyn_duration_elapse_prop[parent_timer_id],
										dt);				
					pos[HMI_START_POS].x	= (ANGEL_TYPE)(hmi_anim_dyn_set_p1_prop_table[obj_index_offset].pPoint_list[0].y);				
					pos[HMI_END_POS].x		= (ANGEL_TYPE)(hmi_anim_dyn_set_p1_prop_table[obj_index_offset].pPoint_list[1].y);				
					success		= get_next_step_animfloat(pos ,dt,
									hmi_dyn_duration_static_prop[parent_timer_id].duration);												
					obj_executer= hmi_anim_dyn_set_p1_execute_id[obj_index_offset];
				}
				else /*Bezier*/
				{
					/*transform dt*/
					trans_dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
									hmi_anim_dyn_set_p1_anim_type[obj_index_offset],
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_dyn_duration_elapse_prop[parent_timer_id],
									dt);
					/*get every bezier length*/
					act_len	= get_bezier_length(bezier_length_list,HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT,
									hmi_anim_dyn_set_p1_prop_table[obj_index_offset].point_cnt,
									hmi_anim_dyn_set_p1_prop_table[obj_index_offset].pPoint_list);
					hmi_get_bezier_index(trans_dt,bezier_length_list,act_len,
									hmi_anim_dyn_set_p1_prop_table[obj_index_offset].line_sum,
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_anim_dyn_set_p1_prop_table[obj_index_offset].point_cnt,
									hmi_anim_dyn_set_p1_prop_table[obj_index_offset].pPoint_list,
									&bezier_new_pos);
					#if 0
					if(bezier_new_pos.newElapse>dt)
					{
						bezier_new_pos.newElapse-=dt;
					}
					else
					{
						bezier_new_pos.newElapse=0;
					}
					#endif														
					trans_dt	= get_trans_dt(0/*start time*/,hmi_anim_dyn_set_p1_anim_type[obj_index_offset],
									bezier_new_pos.newDur,bezier_new_pos.newElapse,dt/*dt*/);
					pos[HMI_START_POS].x	= bezier_new_pos.lineStart.x;
					pos[HMI_START_POS].y	= bezier_new_pos.lineStart.y;
					pos[HMI_END_POS].x		= bezier_new_pos.lineEnd.x;
					pos[HMI_END_POS].y		= bezier_new_pos.lineEnd.y;
					success		= get_next_step_animfloat(pos,trans_dt,
									bezier_new_pos.newDur);						
					obj_executer= hmi_anim_dyn_set_p1_execute_id[obj_index_offset];					
				}
			}
			#endif
		}
		else if(HMI_IS_TIMER_ACTION_S_DURATION(parent_timer_id))/*static timer and dyn anim*/
		{
			#if (HMI_S_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_DYN_SET_P1_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_S_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_P1_INDEX_ANIM_DXY(obj_index);
			if((obj_index_offset < HMI_ANIM_DYN_SET_P1_NUMBER)&&(parent_timer_id < HMI_S_TIMER_ACTION_DURATION_NUMBER))
			{
				if(hmi_anim_dyn_set_p1_prop_table[obj_index_offset].point_cnt == HMI_LINE_PATH_CNT )
				{
					dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
										hmi_anim_dyn_set_p1_anim_type[obj_index_offset],
										hmi_duration_static_prop[parent_timer_id].duration,
										hmi_duration_elapse_prop[parent_timer_id],
										dt);				
					pos[HMI_START_POS].x =(ANGEL_TYPE)(hmi_anim_dyn_set_p1_prop_table[obj_index_offset].pPoint_list[0].y);				
					pos[HMI_END_POS].x =(ANGEL_TYPE)(hmi_anim_dyn_set_p1_prop_table[obj_index_offset].pPoint_list[1].y);	
					success		= get_next_step_animfloat(pos ,dt,
									hmi_duration_static_prop[parent_timer_id].duration);												
					obj_executer= hmi_anim_dyn_set_p1_execute_id[obj_index_offset];
				}
				else /*Bezier*/
				{
					/*transform dt*/
					trans_dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
									hmi_anim_dyn_set_p1_anim_type[obj_index_offset],
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_duration_elapse_prop[parent_timer_id],
									dt);
					/*get every bezier length*/
					act_len	= get_bezier_length(bezier_length_list,HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT,
									hmi_anim_dyn_set_p1_prop_table[obj_index_offset].point_cnt,
									hmi_anim_dyn_set_p1_prop_table[obj_index_offset].pPoint_list);
					hmi_get_bezier_index(trans_dt,bezier_length_list,act_len,
									hmi_anim_dyn_set_p1_prop_table[obj_index_offset].line_sum,
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_anim_dyn_set_p1_prop_table[obj_index_offset].point_cnt,
									hmi_anim_dyn_set_p1_prop_table[obj_index_offset].pPoint_list,
									&bezier_new_pos);
					#if 0
					if(bezier_new_pos.newElapse>dt)
					{
						bezier_new_pos.newElapse-=dt;
					}
					else
					{
						bezier_new_pos.newElapse=0;
					}
					#endif														
					trans_dt	= get_trans_dt(0/*start time*/,hmi_anim_dyn_set_p1_anim_type[obj_index_offset],
									bezier_new_pos.newDur,bezier_new_pos.newElapse,dt/*dt*/);
					pos[HMI_START_POS].x	= bezier_new_pos.lineStart.x;
					pos[HMI_START_POS].y	= bezier_new_pos.lineStart.y;
					pos[HMI_END_POS].x		= bezier_new_pos.lineEnd.x;
					pos[HMI_END_POS].y		= bezier_new_pos.lineEnd.y;
					success		= get_next_step_animfloat(pos,trans_dt,
									bezier_new_pos.newDur);						
					obj_executer= hmi_anim_dyn_set_p1_execute_id[obj_index_offset];
				}
			}
			#endif						
		}
		else
		{
		}				
	}
	else if((!dyn_anim)&&(HMI_IS_P1_INDEX_ANIM_SXY(obj_index))) 
	{
		if(HMI_IS_TIMER_ACTION_D_DURATION(parent_timer_id))/*dyn timer and static anim*/
		{
			#if (HMI_DYN_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_STATIC_SET_P1_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_D_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_P1_INDEX_ANIM_SXY(obj_index);
			if((obj_index_offset < HMI_ANIM_STATIC_SET_P1_NUMBER)&&(parent_timer_id < HMI_DYN_TIMER_ACTION_DURATION_NUMBER))
			{
				if(hmi_anim_static_set_p1_prop_table[obj_index_offset].point_cnt == HMI_LINE_PATH_CNT)
				{
					dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
										hmi_anim_static_set_p1_anim_type[obj_index_offset],
										hmi_dyn_duration_static_prop[parent_timer_id].duration,
										hmi_dyn_duration_elapse_prop[parent_timer_id],
										dt);				
					pos[HMI_START_POS].x= (ANGEL_TYPE)(hmi_anim_static_set_p1_prop_table[obj_index_offset].pPoint_list[0].y);				
					pos[HMI_END_POS].x	= (ANGEL_TYPE)(hmi_anim_static_set_p1_prop_table[obj_index_offset].pPoint_list[1].y);					
					success		= get_next_step_animfloat(pos ,dt,
									hmi_dyn_duration_static_prop[parent_timer_id].duration);								
					obj_executer= hmi_anim_static_set_p1_execute_id[obj_index_offset];
				}
			}
			else /*Bezier*/
			{
				/*transform dt*/
					trans_dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
									hmi_anim_static_set_p1_anim_type[obj_index_offset],
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_dyn_duration_elapse_prop[parent_timer_id],
									dt);
					/*get every bezier length*/
					act_len	= get_bezier_length(bezier_length_list,HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT,
									hmi_anim_static_set_p1_prop_table[obj_index_offset].point_cnt,
									hmi_anim_static_set_p1_prop_table[obj_index_offset].pPoint_list);
					hmi_get_bezier_index(trans_dt,bezier_length_list,act_len,
									hmi_anim_static_set_p1_prop_table[obj_index_offset].line_sum,
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_anim_static_set_p1_prop_table[obj_index_offset].point_cnt,
									hmi_anim_static_set_p1_prop_table[obj_index_offset].pPoint_list,
									&bezier_new_pos);
					#if 0
					if(bezier_new_pos.newElapse>dt)
					{
						bezier_new_pos.newElapse-=dt;
					}
					else
					{
						bezier_new_pos.newElapse=0;
					}
					#endif														
					trans_dt	= get_trans_dt(0/*start time*/,hmi_anim_static_set_p1_anim_type[obj_index_offset],
									bezier_new_pos.newDur,bezier_new_pos.newElapse,dt/*dt*/);
					pos[HMI_START_POS].x	= (float_32)(bezier_new_pos.lineStart.x);
					pos[HMI_START_POS].y	= (float_32)(bezier_new_pos.lineStart.y);
					pos[HMI_END_POS].x		= (float_32)(bezier_new_pos.lineEnd.x);
					pos[HMI_END_POS].y		= (float_32)(bezier_new_pos.lineEnd.y);
					success		= get_next_step_animfloat(pos,trans_dt,
									bezier_new_pos.newDur);						
					obj_executer= hmi_anim_static_set_p1_execute_id[obj_index_offset];
			}
			#endif
		}
		else if(HMI_IS_TIMER_ACTION_S_DURATION(parent_timer_id))/*static timer and static anim*/
		{
			#if (HMI_S_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_STATIC_SET_P1_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_S_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_P1_INDEX_ANIM_SXY(obj_index);
			if((obj_index_offset < HMI_ANIM_STATIC_SET_P1_NUMBER)&&(parent_timer_id < HMI_S_TIMER_ACTION_DURATION_NUMBER))
			{
				if(hmi_anim_static_set_p1_prop_table[obj_index_offset].point_cnt == HMI_LINE_PATH_CNT)
				{
					dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
										hmi_anim_static_set_p1_anim_type[obj_index_offset],
										hmi_duration_static_prop[parent_timer_id].duration,
										hmi_duration_elapse_prop[parent_timer_id],
										dt);				
					pos[HMI_START_POS].x	= (ANGEL_TYPE)(hmi_anim_static_set_p1_prop_table[obj_index_offset].pPoint_list[0].y);				
					pos[HMI_END_POS].x		= (ANGEL_TYPE)(hmi_anim_static_set_p1_prop_table[obj_index_offset].pPoint_list[1].y);						
					success		= get_next_step_animfloat(pos ,dt,
									hmi_duration_static_prop[parent_timer_id].duration);								
					obj_executer= hmi_anim_static_set_p1_execute_id[obj_index_offset];
				}
				else /*Bezier*/
				{
					/*transform dt*/
					trans_dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
									hmi_anim_static_set_p1_anim_type[obj_index_offset],
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_duration_elapse_prop[parent_timer_id],
									dt);
					/*get every bezier length*/
					act_len	= get_bezier_length(bezier_length_list,HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT,
									hmi_anim_static_set_p1_prop_table[obj_index_offset].point_cnt,
									hmi_anim_static_set_p1_prop_table[obj_index_offset].pPoint_list);
					hmi_get_bezier_index(trans_dt,bezier_length_list,act_len,
									hmi_anim_static_set_p1_prop_table[obj_index_offset].line_sum,
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_anim_static_set_p1_prop_table[obj_index_offset].point_cnt,
									hmi_anim_static_set_p1_prop_table[obj_index_offset].pPoint_list,
									&bezier_new_pos);
					#if 0
					if(bezier_new_pos.newElapse>dt)
					{
						bezier_new_pos.newElapse-=dt;
					}
					else
					{
						bezier_new_pos.newElapse=0;
					}
					#endif														
					trans_dt	= get_trans_dt(0/*start time*/,hmi_anim_static_set_p1_anim_type[obj_index_offset],
									bezier_new_pos.newDur,bezier_new_pos.newElapse,dt/*dt*/);
					pos[HMI_START_POS].x	= (float_32)bezier_new_pos.lineStart.x;
					pos[HMI_START_POS].y	= (float_32)bezier_new_pos.lineStart.y;
					pos[HMI_END_POS].x		= (float_32)bezier_new_pos.lineEnd.x;
					pos[HMI_END_POS].y		= (float_32)bezier_new_pos.lineEnd.y;
					success		= get_next_step_animfloat(pos,trans_dt,
									bezier_new_pos.newDur);						
					obj_executer= hmi_anim_static_set_p1_execute_id[obj_index_offset];
				}
			}
			#endif						
		}
		else
		{
		}
	}
	else
	{
	}
	if(success)
	{		
		obj_index_pro	= hmi_get_obj_pro_id(obj_executer,HMI_ELEMENT_P1);/*get obj_index property ID*/
		#if HMI_DXY_CUSTOM_CNT>0
		if(HMI_IS_DYN_XY_CUSTOM(obj_executer))
		{
			if(obj_executer > HMI_SXY_SPLINE_MAX_ID)
			{
				hmi_object_index = HMI_GET_DXY_CUSTOM_INDEX(obj_executer);
			}
			else
			{
				hmi_object_index = 0U;
			}
			get_success 	 = hmi_engine_get_dyn_custom_index(hmi_object_index,&custom_lib_index);
			if(get_success == TRUE)
			{
				phmi_custom_widget_info =hmi_engine_get_dxy_custom_addr();
				phmi_custom_widget_info =&phmi_custom_widget_info[custom_lib_index];
			}
		}
		else
		#endif
		#if HMI_SXY_CUSTOM_CNT>0
		if(HMI_IS_CUSTOM_SXY(obj_executer))
		{
			hmi_object_index = HMI_GET_CUSTOM_SXY_ID_INDEX(obj_executer);
			get_success 	 = hmi_engine_get_static_custom_index(hmi_object_index,&custom_lib_index);
			if(get_success ==TRUE)
			{
				phmi_custom_widget_info =hmi_engine_get_sxy_custom_addr();
				phmi_custom_widget_info =&phmi_custom_widget_info[custom_lib_index];
			}
		}
		else
		#endif
		{
		}
		if(phmi_custom_widget_info !=NULL)
		{
			if(((phmi_custom_widget_info->attr_fun.attr) & HMI_P1) != 0)
			{
				if(((phmi_custom_widget_info->attr_fun.attr) & HMI_P1_F) != 0)
				{
					hmi_object_data	= (HMI_OBJECT_DATA_STR)HMI_F32_TO_U32(pos[HMI_NEXT_POS].x );
				}
				else
				{
					hmi_object_data	= (HMI_OBJECT_DATA_STR)(pos[HMI_NEXT_POS].x );
				}	
				hmi_engine_set_object_info(obj_index_pro,hmi_object_data);
			}
		}
	}		
}
#endif
#endif

#if HMI_ANIM_STATIC_SET_P2_NUMBER+HMI_ANIM_DYN_SET_P2_NUMBER > 0
#ifndef HMI_GRAPHIC_ST7513
void hmi_anim_set_p2_action(HMI_OBJECT_ID_STR obj_index,BOOLEAN dyn_anim,HMI_TIME dt,
							HMI_OBJECT_ID_STR parent_timer_id)
{
	BOOLEAN				success			= FALSE;
	HMI_OBJECT_DATA_STR hmi_object_data	= 0;
	HMI_OBJECT_ID_STR	obj_index_offset= 0;
	HMI_OBJECT_ID_STR	obj_index_pro	= 0;
	HMI_OBJECT_ID_STR	obj_executer	= HMI_NB_ELEMENTS;	
	POINT_FLOAT_TP		pos[HMI_ANIM_POS_CNT]={{0,0},{0,0},{0,0}};
	UINT32	 bezier_length_list[HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT];
	HMI_BEZIER_NEW_POS_STR	bezier_new_pos={{0,0},{0,0},0,0};
	UINT32				act_len		= 0;
	HMI_TIME			trans_dt	= 0;
	HMI_CUSTOM_INFO_STR CONST*	phmi_custom_widget_info	= NULL;
	HMI_OBJECT_ID_STR	custom_lib_index			= 0U;
	HMI_OBJECT_ID_STR	hmi_object_index			= 0U;
	BOOLEAN 			get_success			= FALSE;
	
	parent_timer_id	= HMI_GET_ACTION_ID_INDEX(parent_timer_id);
	if(dyn_anim&&(HMI_IS_P2_INDEX_ANIM_DXY(obj_index)))
	{
		if(HMI_IS_TIMER_ACTION_D_DURATION(parent_timer_id))/*dyn timer and dyn anim*/
		{
			#if (HMI_DYN_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_DYN_SET_P2_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_D_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_P2_INDEX_ANIM_DXY(obj_index);
			if((obj_index_offset < HMI_ANIM_DYN_SET_P2_NUMBER)&&(parent_timer_id < HMI_DYN_TIMER_ACTION_DURATION_NUMBER))
			{
				if(hmi_anim_dyn_set_p2_prop_table[obj_index_offset].point_cnt == HMI_LINE_PATH_CNT )
				{
					dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
										hmi_anim_dyn_set_p2_anim_type[obj_index_offset],
										hmi_dyn_duration_static_prop[parent_timer_id].duration,
										hmi_dyn_duration_elapse_prop[parent_timer_id],
										dt);				
					pos[HMI_START_POS].x	= (ANGEL_TYPE)(hmi_anim_dyn_set_p2_prop_table[obj_index_offset].pPoint_list[0].y);				
					pos[HMI_END_POS].x		= (ANGEL_TYPE)(hmi_anim_dyn_set_p2_prop_table[obj_index_offset].pPoint_list[1].y);				
					success		= get_next_step_animfloat(pos ,dt,
									hmi_dyn_duration_static_prop[parent_timer_id].duration);												
					obj_executer= hmi_anim_dyn_set_p2_execute_id[obj_index_offset];
				}
				else /*Bezier*/
				{
					/*transform dt*/
					trans_dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
									hmi_anim_dyn_set_p2_anim_type[obj_index_offset],
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_dyn_duration_elapse_prop[parent_timer_id],
									dt);
					/*get every bezier length*/
					act_len	= get_bezier_length(bezier_length_list,HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT,
									hmi_anim_dyn_set_p2_prop_table[obj_index_offset].point_cnt,
									hmi_anim_dyn_set_p2_prop_table[obj_index_offset].pPoint_list);
					hmi_get_bezier_index(trans_dt,bezier_length_list,act_len,
									hmi_anim_dyn_set_p2_prop_table[obj_index_offset].line_sum,
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_anim_dyn_set_p2_prop_table[obj_index_offset].point_cnt,
									hmi_anim_dyn_set_p2_prop_table[obj_index_offset].pPoint_list,
									&bezier_new_pos);
					#if 0
					if(bezier_new_pos.newElapse>dt)
					{
						bezier_new_pos.newElapse-=dt;
					}
					else
					{
						bezier_new_pos.newElapse=0;
					}
					#endif														
					trans_dt	= get_trans_dt(0/*start time*/,hmi_anim_dyn_set_p2_anim_type[obj_index_offset],
									bezier_new_pos.newDur,bezier_new_pos.newElapse,dt/*dt*/);
					pos[HMI_START_POS].x	= bezier_new_pos.lineStart.x;
					pos[HMI_START_POS].y	= bezier_new_pos.lineStart.y;
					pos[HMI_END_POS].x		= bezier_new_pos.lineEnd.x;
					pos[HMI_END_POS].y		= bezier_new_pos.lineEnd.y;
					success		= get_next_step_animfloat(pos,trans_dt,
									bezier_new_pos.newDur);						
					obj_executer= hmi_anim_dyn_set_p2_execute_id[obj_index_offset];					
				}
			}
			#endif
		}
		else if(HMI_IS_TIMER_ACTION_S_DURATION(parent_timer_id))/*static timer and dyn anim*/
		{
			#if (HMI_S_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_DYN_SET_P2_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_S_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_P2_INDEX_ANIM_DXY(obj_index);
			if((obj_index_offset < HMI_ANIM_DYN_SET_P2_NUMBER)&&(parent_timer_id < HMI_S_TIMER_ACTION_DURATION_NUMBER))
			{
				if(hmi_anim_dyn_set_p2_prop_table[obj_index_offset].point_cnt == HMI_LINE_PATH_CNT )
				{
					dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
										hmi_anim_dyn_set_p2_anim_type[obj_index_offset],
										hmi_duration_static_prop[parent_timer_id].duration,
										hmi_duration_elapse_prop[parent_timer_id],
										dt);				
					pos[HMI_START_POS].x =(ANGEL_TYPE)(hmi_anim_dyn_set_p2_prop_table[obj_index_offset].pPoint_list[0].y);				
					pos[HMI_END_POS].x =(ANGEL_TYPE)(hmi_anim_dyn_set_p2_prop_table[obj_index_offset].pPoint_list[1].y);	
					success		= get_next_step_animfloat(pos ,dt,
									hmi_duration_static_prop[parent_timer_id].duration);												
					obj_executer= hmi_anim_dyn_set_p2_execute_id[obj_index_offset];
				}
				else /*Bezier*/
				{
					/*transform dt*/
					trans_dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
									hmi_anim_dyn_set_p2_anim_type[obj_index_offset],
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_duration_elapse_prop[parent_timer_id],
									dt);
					/*get every bezier length*/
					act_len	= get_bezier_length(bezier_length_list,HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT,
									hmi_anim_dyn_set_p2_prop_table[obj_index_offset].point_cnt,
									hmi_anim_dyn_set_p2_prop_table[obj_index_offset].pPoint_list);
					hmi_get_bezier_index(trans_dt,bezier_length_list,act_len,
									hmi_anim_dyn_set_p2_prop_table[obj_index_offset].line_sum,
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_anim_dyn_set_p2_prop_table[obj_index_offset].point_cnt,
									hmi_anim_dyn_set_p2_prop_table[obj_index_offset].pPoint_list,
									&bezier_new_pos);
					#if 0
					if(bezier_new_pos.newElapse>dt)
					{
						bezier_new_pos.newElapse-=dt;
					}
					else
					{
						bezier_new_pos.newElapse=0;
					}
					#endif														
					trans_dt	= get_trans_dt(0/*start time*/,hmi_anim_dyn_set_p2_anim_type[obj_index_offset],
									bezier_new_pos.newDur,bezier_new_pos.newElapse,dt/*dt*/);
					pos[HMI_START_POS].x	= bezier_new_pos.lineStart.x;
					pos[HMI_START_POS].y	= bezier_new_pos.lineStart.y;
					pos[HMI_END_POS].x		= bezier_new_pos.lineEnd.x;
					pos[HMI_END_POS].y		= bezier_new_pos.lineEnd.y;
					success		= get_next_step_animfloat(pos,trans_dt,
									bezier_new_pos.newDur);						
					obj_executer= hmi_anim_dyn_set_p2_execute_id[obj_index_offset];
				}
			}
			#endif						
		}
		else
		{
		}				
	}
	else if((!dyn_anim)&&(HMI_IS_P2_INDEX_ANIM_SXY(obj_index))) 
	{
		if(HMI_IS_TIMER_ACTION_D_DURATION(parent_timer_id))/*dyn timer and static anim*/
		{
			#if (HMI_DYN_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_STATIC_SET_P2_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_D_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_P2_INDEX_ANIM_SXY(obj_index);
			if((obj_index_offset < HMI_ANIM_STATIC_SET_P2_NUMBER)&&(parent_timer_id < HMI_DYN_TIMER_ACTION_DURATION_NUMBER))
			{
				if(hmi_anim_static_set_p2_prop_table[obj_index_offset].point_cnt == HMI_LINE_PATH_CNT)
				{
					dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
										hmi_anim_static_set_p2_anim_type[obj_index_offset],
										hmi_dyn_duration_static_prop[parent_timer_id].duration,
										hmi_dyn_duration_elapse_prop[parent_timer_id],
										dt);				
					pos[HMI_START_POS].x= (ANGEL_TYPE)(hmi_anim_static_set_p2_prop_table[obj_index_offset].pPoint_list[0].y);				
					pos[HMI_END_POS].x	= (ANGEL_TYPE)(hmi_anim_static_set_p2_prop_table[obj_index_offset].pPoint_list[1].y);					
					success		= get_next_step_animfloat(pos ,dt,
									hmi_dyn_duration_static_prop[parent_timer_id].duration);								
					obj_executer= hmi_anim_static_set_p2_execute_id[obj_index_offset];
				}
			}
			else /*Bezier*/
			{
				/*transform dt*/
					trans_dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
									hmi_anim_static_set_p2_anim_type[obj_index_offset],
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_dyn_duration_elapse_prop[parent_timer_id],
									dt);
					/*get every bezier length*/
					act_len	= get_bezier_length(bezier_length_list,HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT,
									hmi_anim_static_set_p2_prop_table[obj_index_offset].point_cnt,
									hmi_anim_static_set_p2_prop_table[obj_index_offset].pPoint_list);
					hmi_get_bezier_index(trans_dt,bezier_length_list,act_len,
									hmi_anim_static_set_p2_prop_table[obj_index_offset].line_sum,
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_anim_static_set_p2_prop_table[obj_index_offset].point_cnt,
									hmi_anim_static_set_p2_prop_table[obj_index_offset].pPoint_list,
									&bezier_new_pos);
					#if 0
					if(bezier_new_pos.newElapse>dt)
					{
						bezier_new_pos.newElapse-=dt;
					}
					else
					{
						bezier_new_pos.newElapse=0;
					}
					#endif														
					trans_dt	= get_trans_dt(0/*start time*/,hmi_anim_static_set_p2_anim_type[obj_index_offset],
									bezier_new_pos.newDur,bezier_new_pos.newElapse,dt/*dt*/);
					pos[HMI_START_POS].x	= bezier_new_pos.lineStart.x;
					pos[HMI_START_POS].y	= bezier_new_pos.lineStart.y;
					pos[HMI_END_POS].x		= bezier_new_pos.lineEnd.x;
					pos[HMI_END_POS].y		= bezier_new_pos.lineEnd.y;
					success		= get_next_step_animfloat(pos,trans_dt,
									bezier_new_pos.newDur);						
					obj_executer= hmi_anim_static_set_p2_execute_id[obj_index_offset];
			}
			#endif
		}
		else if(HMI_IS_TIMER_ACTION_S_DURATION(parent_timer_id))/*static timer and static anim*/
		{
			#if (HMI_S_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_STATIC_SET_P2_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_S_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_P2_INDEX_ANIM_SXY(obj_index);
			if((obj_index_offset < HMI_ANIM_STATIC_SET_P2_NUMBER)&&(parent_timer_id < HMI_S_TIMER_ACTION_DURATION_NUMBER))
			{
				if(hmi_anim_static_set_p2_prop_table[obj_index_offset].point_cnt == HMI_LINE_PATH_CNT)
				{
					dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
										hmi_anim_static_set_p2_anim_type[obj_index_offset],
										hmi_duration_static_prop[parent_timer_id].duration,
										hmi_duration_elapse_prop[parent_timer_id],
										dt);				
					pos[HMI_START_POS].x	= (ANGEL_TYPE)(hmi_anim_static_set_p2_prop_table[obj_index_offset].pPoint_list[0].y);				
					pos[HMI_END_POS].x		= (ANGEL_TYPE)(hmi_anim_static_set_p2_prop_table[obj_index_offset].pPoint_list[1].y);						
					success		= get_next_step_animfloat(pos ,dt,
									hmi_duration_static_prop[parent_timer_id].duration);								
					obj_executer= hmi_anim_static_set_p2_execute_id[obj_index_offset];
				}
				else /*Bezier*/
				{
					/*transform dt*/
					trans_dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
									hmi_anim_static_set_p2_anim_type[obj_index_offset],
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_duration_elapse_prop[parent_timer_id],
									dt);
					/*get every bezier length*/
					act_len	= get_bezier_length(bezier_length_list,HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT,
									hmi_anim_static_set_p2_prop_table[obj_index_offset].point_cnt,
									hmi_anim_static_set_p2_prop_table[obj_index_offset].pPoint_list);
					hmi_get_bezier_index(trans_dt,bezier_length_list,act_len,
									hmi_anim_static_set_p2_prop_table[obj_index_offset].line_sum,
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_anim_static_set_p2_prop_table[obj_index_offset].point_cnt,
									hmi_anim_static_set_p2_prop_table[obj_index_offset].pPoint_list,
									&bezier_new_pos);
					#if 0
					if(bezier_new_pos.newElapse>dt)
					{
						bezier_new_pos.newElapse-=dt;
					}
					else
					{
						bezier_new_pos.newElapse=0;
					}
					#endif														
					trans_dt	= get_trans_dt(0/*start time*/,hmi_anim_static_set_p2_anim_type[obj_index_offset],
									bezier_new_pos.newDur,bezier_new_pos.newElapse,dt/*dt*/);
					pos[HMI_START_POS].x	= (float_32)bezier_new_pos.lineStart.x;
					pos[HMI_START_POS].y	= (float_32)bezier_new_pos.lineStart.y;
					pos[HMI_END_POS].x		= (float_32)bezier_new_pos.lineEnd.x;
					pos[HMI_END_POS].y		= (float_32)bezier_new_pos.lineEnd.y;
					success		= get_next_step_animfloat(pos,trans_dt,
									bezier_new_pos.newDur);						
					obj_executer= hmi_anim_static_set_p2_execute_id[obj_index_offset];
				}
			}
			#endif						
		}
		else
		{
		}
	}
	else
	{
	}
	if(success)
	{		
		obj_index_pro	= hmi_get_obj_pro_id(obj_executer,HMI_ELEMENT_P2);/*get obj_index property ID*/
		#if HMI_DXY_CUSTOM_CNT>0
		if(HMI_IS_DYN_XY_CUSTOM(obj_executer))
		{
			//hmi_object_index = HMI_GET_DXY_CUSTOM_INDEX(obj_executer);
			if(obj_executer > HMI_SXY_SPLINE_MAX_ID)
			{
				hmi_object_index = HMI_GET_DXY_CUSTOM_INDEX(obj_executer);
			}
			else
			{
				hmi_object_index = 0U;
			}
			get_success 	 = hmi_engine_get_dyn_custom_index(hmi_object_index,&custom_lib_index);
			if(get_success == TRUE)
			{
				phmi_custom_widget_info =hmi_engine_get_dxy_custom_addr();
				phmi_custom_widget_info =&phmi_custom_widget_info[custom_lib_index];
			}
		}
		else
		#endif
		#if HMI_SXY_CUSTOM_CNT>0
		if(HMI_IS_CUSTOM_SXY(obj_executer))
		{
			hmi_object_index = HMI_GET_CUSTOM_SXY_ID_INDEX(obj_executer);
			get_success 	 = hmi_engine_get_static_custom_index(hmi_object_index,&custom_lib_index);
			if(get_success ==TRUE)
			{
				phmi_custom_widget_info =hmi_engine_get_sxy_custom_addr();
				phmi_custom_widget_info =&phmi_custom_widget_info[custom_lib_index];
			}
		}
		else
		#endif
		{
		}
		if(phmi_custom_widget_info !=NULL)
		{
			if(((phmi_custom_widget_info->attr_fun.attr)& HMI_P2) != 0)
			{
				if(((phmi_custom_widget_info->attr_fun.attr) & HMI_P2_F) != 0)
				{
					hmi_object_data	= (HMI_OBJECT_DATA_STR)HMI_F32_TO_U32(pos[HMI_NEXT_POS].x );
				}
				else
				{
					hmi_object_data	= (HMI_OBJECT_DATA_STR)(pos[HMI_NEXT_POS].x );
				}	
				hmi_engine_set_object_info(obj_index_pro,hmi_object_data);
			}
		}
	}		
}
#endif
#endif

#if HMI_ANIM_STATIC_SET_P3_NUMBER+HMI_ANIM_DYN_SET_P3_NUMBER > 0
#ifndef HMI_GRAPHIC_ST7513
void hmi_anim_set_p3_action(HMI_OBJECT_ID_STR obj_index,BOOLEAN dyn_anim,HMI_TIME dt,
							HMI_OBJECT_ID_STR parent_timer_id)
{
	BOOLEAN				success			= FALSE;
	HMI_OBJECT_DATA_STR hmi_object_data	= 0;
	HMI_OBJECT_ID_STR	obj_index_offset= 0;
	HMI_OBJECT_ID_STR	obj_index_pro	= 0;
	HMI_OBJECT_ID_STR	obj_executer	= HMI_NB_ELEMENTS;	
	POINT_FLOAT_TP		pos[HMI_ANIM_POS_CNT]={{0,0},{0,0},{0,0}};
	UINT32	 bezier_length_list[HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT];
	HMI_BEZIER_NEW_POS_STR	bezier_new_pos={{0,0},{0,0},0,0};
	UINT32				act_len		= 0;
	HMI_TIME			trans_dt	= 0;
	HMI_CUSTOM_INFO_STR CONST*	phmi_custom_widget_info	= NULL;
	HMI_OBJECT_ID_STR	custom_lib_index			= 0U;
	HMI_OBJECT_ID_STR	hmi_object_index			= 0U;
	BOOLEAN 			get_success			= FALSE;
	
	parent_timer_id	= HMI_GET_ACTION_ID_INDEX(parent_timer_id);
	if(dyn_anim&&(HMI_IS_P3_INDEX_ANIM_DXY(obj_index)))
	{
		if(HMI_IS_TIMER_ACTION_D_DURATION(parent_timer_id))/*dyn timer and dyn anim*/
		{
			#if (HMI_DYN_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_DYN_SET_P3_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_D_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_P3_INDEX_ANIM_DXY(obj_index);
			if((obj_index_offset < HMI_ANIM_DYN_SET_P3_NUMBER)&&(parent_timer_id < HMI_DYN_TIMER_ACTION_DURATION_NUMBER))
			{
				if(hmi_anim_dyn_set_p3_prop_table[obj_index_offset].point_cnt == HMI_LINE_PATH_CNT )
				{
					dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
										hmi_anim_dyn_set_p3_anim_type[obj_index_offset],
										hmi_dyn_duration_static_prop[parent_timer_id].duration,
										hmi_dyn_duration_elapse_prop[parent_timer_id],
										dt);				
					pos[HMI_START_POS].x	= (ANGEL_TYPE)(hmi_anim_dyn_set_p3_prop_table[obj_index_offset].pPoint_list[0].y);				
					pos[HMI_END_POS].x		= (ANGEL_TYPE)(hmi_anim_dyn_set_p3_prop_table[obj_index_offset].pPoint_list[1].y);				
					success		= get_next_step_animfloat(pos ,dt,
									hmi_dyn_duration_static_prop[parent_timer_id].duration);												
					obj_executer= hmi_anim_dyn_set_p3_execute_id[obj_index_offset];
				}
				else /*Bezier*/
				{
					/*transform dt*/
					trans_dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
									hmi_anim_dyn_set_p3_anim_type[obj_index_offset],
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_dyn_duration_elapse_prop[parent_timer_id],
									dt);
					/*get every bezier length*/
					act_len	= get_bezier_length(bezier_length_list,HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT,
									hmi_anim_dyn_set_p3_prop_table[obj_index_offset].point_cnt,
									hmi_anim_dyn_set_p3_prop_table[obj_index_offset].pPoint_list);
					hmi_get_bezier_index(trans_dt,bezier_length_list,act_len,
									hmi_anim_dyn_set_p3_prop_table[obj_index_offset].line_sum,
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_anim_dyn_set_p3_prop_table[obj_index_offset].point_cnt,
									hmi_anim_dyn_set_p3_prop_table[obj_index_offset].pPoint_list,
									&bezier_new_pos);
					#if 0
					if(bezier_new_pos.newElapse>dt)
					{
						bezier_new_pos.newElapse-=dt;
					}
					else
					{
						bezier_new_pos.newElapse=0;
					}
					#endif														
					trans_dt	= get_trans_dt(0/*start time*/,hmi_anim_dyn_set_p3_anim_type[obj_index_offset],
									bezier_new_pos.newDur,bezier_new_pos.newElapse,dt/*dt*/);
					pos[HMI_START_POS].x	= bezier_new_pos.lineStart.x;
					pos[HMI_START_POS].y	= bezier_new_pos.lineStart.y;
					pos[HMI_END_POS].x		= bezier_new_pos.lineEnd.x;
					pos[HMI_END_POS].y		= bezier_new_pos.lineEnd.y;
					success		= get_next_step_animfloat(pos,trans_dt,
									bezier_new_pos.newDur);						
					obj_executer= hmi_anim_dyn_set_p3_execute_id[obj_index_offset];					
				}
			}
			#endif
		}
		else if(HMI_IS_TIMER_ACTION_S_DURATION(parent_timer_id))/*static timer and dyn anim*/
		{
			#if (HMI_S_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_DYN_SET_P3_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_S_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_P3_INDEX_ANIM_DXY(obj_index);
			if((obj_index_offset < HMI_ANIM_DYN_SET_P3_NUMBER)&&(parent_timer_id < HMI_S_TIMER_ACTION_DURATION_NUMBER))
			{
				if(hmi_anim_dyn_set_p3_prop_table[obj_index_offset].point_cnt == HMI_LINE_PATH_CNT )
				{
					dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
										hmi_anim_dyn_set_p3_anim_type[obj_index_offset],
										hmi_duration_static_prop[parent_timer_id].duration,
										hmi_duration_elapse_prop[parent_timer_id],
										dt);				
					pos[HMI_START_POS].x =(ANGEL_TYPE)(hmi_anim_dyn_set_p3_prop_table[obj_index_offset].pPoint_list[0].y);				
					pos[HMI_END_POS].x =(ANGEL_TYPE)(hmi_anim_dyn_set_p3_prop_table[obj_index_offset].pPoint_list[1].y);	
					success		= get_next_step_animfloat(pos ,dt,
									hmi_duration_static_prop[parent_timer_id].duration);												
					obj_executer= hmi_anim_dyn_set_p3_execute_id[obj_index_offset];
				}
				else /*Bezier*/
				{
					/*transform dt*/
					trans_dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
									hmi_anim_dyn_set_p3_anim_type[obj_index_offset],
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_duration_elapse_prop[parent_timer_id],
									dt);
					/*get every bezier length*/
					act_len	= get_bezier_length(bezier_length_list,HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT,
									hmi_anim_dyn_set_p3_prop_table[obj_index_offset].point_cnt,
									hmi_anim_dyn_set_p3_prop_table[obj_index_offset].pPoint_list);
					hmi_get_bezier_index(trans_dt,bezier_length_list,act_len,
									hmi_anim_dyn_set_p3_prop_table[obj_index_offset].line_sum,
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_anim_dyn_set_p3_prop_table[obj_index_offset].point_cnt,
									hmi_anim_dyn_set_p3_prop_table[obj_index_offset].pPoint_list,
									&bezier_new_pos);
					#if 0
					if(bezier_new_pos.newElapse>dt)
					{
						bezier_new_pos.newElapse-=dt;
					}
					else
					{
						bezier_new_pos.newElapse=0;
					}
					#endif														
					trans_dt	= get_trans_dt(0/*start time*/,hmi_anim_dyn_set_p3_anim_type[obj_index_offset],
									bezier_new_pos.newDur,bezier_new_pos.newElapse,dt/*dt*/);
					pos[HMI_START_POS].x	= bezier_new_pos.lineStart.x;
					pos[HMI_START_POS].y	= bezier_new_pos.lineStart.y;
					pos[HMI_END_POS].x		= bezier_new_pos.lineEnd.x;
					pos[HMI_END_POS].y		= bezier_new_pos.lineEnd.y;
					success		= get_next_step_animfloat(pos,trans_dt,
									bezier_new_pos.newDur);						
					obj_executer= hmi_anim_dyn_set_p3_execute_id[obj_index_offset];
				}
			}
			#endif						
		}
		else
		{
		}				
	}
	else if((!dyn_anim)&&(HMI_IS_P3_INDEX_ANIM_SXY(obj_index))) 
	{
		if(HMI_IS_TIMER_ACTION_D_DURATION(parent_timer_id))/*dyn timer and static anim*/
		{
			#if (HMI_DYN_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_STATIC_SET_P3_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_D_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_P3_INDEX_ANIM_SXY(obj_index);
			if((obj_index_offset < HMI_ANIM_STATIC_SET_P3_NUMBER)&&(parent_timer_id < HMI_DYN_TIMER_ACTION_DURATION_NUMBER))
			{
				if(hmi_anim_static_set_p3_prop_table[obj_index_offset].point_cnt == HMI_LINE_PATH_CNT)
				{
					dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
										hmi_anim_static_set_p3_anim_type[obj_index_offset],
										hmi_dyn_duration_static_prop[parent_timer_id].duration,
										hmi_dyn_duration_elapse_prop[parent_timer_id],
										dt);				
					pos[HMI_START_POS].x= (ANGEL_TYPE)(hmi_anim_static_set_p3_prop_table[obj_index_offset].pPoint_list[0].y);				
					pos[HMI_END_POS].x	= (ANGEL_TYPE)(hmi_anim_static_set_p3_prop_table[obj_index_offset].pPoint_list[1].y);					
					success		= get_next_step_animfloat(pos ,dt,
									hmi_dyn_duration_static_prop[parent_timer_id].duration);								
					obj_executer= hmi_anim_static_set_p3_execute_id[obj_index_offset];
				}
			}
			else /*Bezier*/
			{
				/*transform dt*/
					trans_dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
									hmi_anim_static_set_p3_anim_type[obj_index_offset],
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_dyn_duration_elapse_prop[parent_timer_id],
									dt);
					/*get every bezier length*/
					act_len	= get_bezier_length(bezier_length_list,HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT,
									hmi_anim_static_set_p3_prop_table[obj_index_offset].point_cnt,
									hmi_anim_static_set_p3_prop_table[obj_index_offset].pPoint_list);
					hmi_get_bezier_index(trans_dt,bezier_length_list,act_len,
									hmi_anim_static_set_p3_prop_table[obj_index_offset].line_sum,
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_anim_static_set_p3_prop_table[obj_index_offset].point_cnt,
									hmi_anim_static_set_p3_prop_table[obj_index_offset].pPoint_list,
									&bezier_new_pos);
					#if 0
					if(bezier_new_pos.newElapse>dt)
					{
						bezier_new_pos.newElapse-=dt;
					}
					else
					{
						bezier_new_pos.newElapse=0;
					}
					#endif														
					trans_dt	= get_trans_dt(0/*start time*/,hmi_anim_static_set_p3_anim_type[obj_index_offset],
									bezier_new_pos.newDur,bezier_new_pos.newElapse,dt/*dt*/);
					pos[HMI_START_POS].x	= bezier_new_pos.lineStart.x;
					pos[HMI_START_POS].y	= bezier_new_pos.lineStart.y;
					pos[HMI_END_POS].x		= bezier_new_pos.lineEnd.x;
					pos[HMI_END_POS].y		= bezier_new_pos.lineEnd.y;
					success		= get_next_step_animfloat(pos,trans_dt,
									bezier_new_pos.newDur);						
					obj_executer= hmi_anim_static_set_p3_execute_id[obj_index_offset];
			}
			#endif
		}
		else if(HMI_IS_TIMER_ACTION_S_DURATION(parent_timer_id))/*static timer and static anim*/
		{
			#if (HMI_S_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_STATIC_SET_P3_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_S_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_P3_INDEX_ANIM_SXY(obj_index);
			if((obj_index_offset < HMI_ANIM_STATIC_SET_P3_NUMBER)&&(parent_timer_id < HMI_S_TIMER_ACTION_DURATION_NUMBER))
			{
				if(hmi_anim_static_set_p3_prop_table[obj_index_offset].point_cnt == HMI_LINE_PATH_CNT)
				{
					dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
										hmi_anim_static_set_p3_anim_type[obj_index_offset],
										hmi_duration_static_prop[parent_timer_id].duration,
										hmi_duration_elapse_prop[parent_timer_id],
										dt);				
					pos[HMI_START_POS].x	= (ANGEL_TYPE)(hmi_anim_static_set_p3_prop_table[obj_index_offset].pPoint_list[0].y);				
					pos[HMI_END_POS].x		= (ANGEL_TYPE)(hmi_anim_static_set_p3_prop_table[obj_index_offset].pPoint_list[1].y);						
					success		= get_next_step_animfloat(pos ,dt,
									hmi_duration_static_prop[parent_timer_id].duration);								
					obj_executer= hmi_anim_static_set_p3_execute_id[obj_index_offset];
				}
				else /*Bezier*/
				{
					/*transform dt*/
					trans_dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
									hmi_anim_static_set_p3_anim_type[obj_index_offset],
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_duration_elapse_prop[parent_timer_id],
									dt);
					/*get every bezier length*/
					act_len	= get_bezier_length(bezier_length_list,HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT,
									hmi_anim_static_set_p3_prop_table[obj_index_offset].point_cnt,
									hmi_anim_static_set_p3_prop_table[obj_index_offset].pPoint_list);
					hmi_get_bezier_index(trans_dt,bezier_length_list,act_len,
									hmi_anim_static_set_p3_prop_table[obj_index_offset].line_sum,
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_anim_static_set_p3_prop_table[obj_index_offset].point_cnt,
									hmi_anim_static_set_p3_prop_table[obj_index_offset].pPoint_list,
									&bezier_new_pos);
					#if 0
					if(bezier_new_pos.newElapse>dt)
					{
						bezier_new_pos.newElapse-=dt;
					}
					else
					{
						bezier_new_pos.newElapse=0;
					}
					#endif														
					trans_dt	= get_trans_dt(0/*start time*/,hmi_anim_static_set_p3_anim_type[obj_index_offset],
									bezier_new_pos.newDur,bezier_new_pos.newElapse,dt/*dt*/);
					pos[HMI_START_POS].x	= (float_32)bezier_new_pos.lineStart.x;
					pos[HMI_START_POS].y	= (float_32)bezier_new_pos.lineStart.y;
					pos[HMI_END_POS].x		= (float_32)bezier_new_pos.lineEnd.x;
					pos[HMI_END_POS].y		= (float_32)bezier_new_pos.lineEnd.y;
					success		= get_next_step_animfloat(pos,trans_dt,
									bezier_new_pos.newDur);						
					obj_executer= hmi_anim_static_set_p3_execute_id[obj_index_offset];
				}
			}
			#endif						
		}
		else
		{
		}
	}
	else
	{
	}
	if(success)
	{		
		obj_index_pro	= hmi_get_obj_pro_id(obj_executer,HMI_ELEMENT_P3);/*get obj_index property ID*/
		#if HMI_DXY_CUSTOM_CNT>0
		if(HMI_IS_DYN_XY_CUSTOM(obj_executer))
		{
			//hmi_object_index = HMI_GET_DXY_CUSTOM_INDEX(obj_executer);
			if(obj_executer > HMI_SXY_SPLINE_MAX_ID)
			{
				hmi_object_index = HMI_GET_DXY_CUSTOM_INDEX(obj_executer);
			}
			else
			{
				hmi_object_index = 0U;
			}
			get_success 	 = hmi_engine_get_dyn_custom_index(hmi_object_index,&custom_lib_index);
			if(get_success == TRUE)
			{
				phmi_custom_widget_info =hmi_engine_get_dxy_custom_addr();
				phmi_custom_widget_info =&phmi_custom_widget_info[custom_lib_index];
			}
		}
		else
		#endif
		#if HMI_SXY_CUSTOM_CNT>0
		if(HMI_IS_CUSTOM_SXY(obj_executer))
		{
			hmi_object_index = HMI_GET_CUSTOM_SXY_ID_INDEX(obj_executer);
			get_success 	 = hmi_engine_get_static_custom_index(hmi_object_index,&custom_lib_index);
			if(get_success ==TRUE)
			{
				phmi_custom_widget_info =hmi_engine_get_sxy_custom_addr();
				phmi_custom_widget_info =&phmi_custom_widget_info[custom_lib_index];
			}
		}
		else
		#endif
		{
		}
		if(phmi_custom_widget_info !=NULL)
		{
			if(((phmi_custom_widget_info->attr_fun.attr) & HMI_P3) != 0)
			{
				if(((phmi_custom_widget_info->attr_fun.attr) & HMI_P3_F) != 0)
				{
					hmi_object_data	= (HMI_OBJECT_DATA_STR)HMI_F32_TO_U32(pos[HMI_NEXT_POS].x );
				}
				else
				{
					hmi_object_data	= (HMI_OBJECT_DATA_STR)(pos[HMI_NEXT_POS].x );
				}	
				hmi_engine_set_object_info(obj_index_pro,hmi_object_data);
			}
		}
	}		
}
#endif
#endif

#if HMI_ANIM_STATIC_SET_ALPHA_NUMBER+HMI_ANIM_DYN_SET_ALPHA_NUMBER > 0
#ifndef HMI_GRAPHIC_ST7513
void hmi_anim_set_alpha_action(HMI_OBJECT_ID_STR obj_index,BOOLEAN dyn_anim,HMI_TIME dt,
							HMI_OBJECT_ID_STR parent_timer_id)
{
	BOOLEAN				success			= FALSE;
	HMI_OBJECT_DATA_STR hmi_object_data	= 0;
	HMI_OBJECT_ID_STR	obj_index_offset= 0;
	HMI_OBJECT_ID_STR	obj_index_pro	= 0;	
	HMI_OBJECT_ID_STR	obj_executer	= HMI_NB_ELEMENTS;	
	POINT32_TP			pos[HMI_ANIM_POS_CNT]={{0,0},{0,0},{0,0}};
	//#ifdef bezier
	UINT32	 bezier_length_list[HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT];
	HMI_BEZIER_NEW_POS_STR	bezier_new_pos={{0,0},{0,0},0,0};
	UINT32				act_len		= 0;
	HMI_TIME			trans_dt	= 0;
	//#endif
	parent_timer_id	= HMI_GET_ACTION_ID_INDEX(parent_timer_id);
	if(dyn_anim&&(HMI_IS_ALPHA_INDEX_ANIM_DXY(obj_index)))
	{
		if(HMI_IS_TIMER_ACTION_D_DURATION(parent_timer_id))/*dyn timer and dyn anim*/
		{
			#if (HMI_DYN_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_DYN_SET_ALPHA_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_D_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_ALPHA_INDEX_ANIM_DXY(obj_index);
			if((obj_index_offset < HMI_ANIM_DYN_SET_ALPHA_NUMBER)&&(parent_timer_id < HMI_DYN_TIMER_ACTION_DURATION_NUMBER))
			{
				if(hmi_anim_dyn_set_alpha_prop_table[obj_index_offset].point_cnt == HMI_LINE_PATH_CNT)
				{
					dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
										hmi_anim_dyn_set_alpha_anim_type[obj_index_offset],
										hmi_dyn_duration_static_prop[parent_timer_id].duration,
										hmi_dyn_duration_elapse_prop[parent_timer_id],
										dt);				
					pos[HMI_START_POS].x	= (INT32)(hmi_anim_dyn_set_alpha_prop_table[obj_index_offset].pPoint_list[0].y);				
					pos[HMI_END_POS].x		= (INT32)(hmi_anim_dyn_set_alpha_prop_table[obj_index_offset].pPoint_list[1].y);				
					success		= get_next_step_anim32(pos ,dt,
									hmi_dyn_duration_static_prop[parent_timer_id].duration);												
					obj_executer= hmi_anim_dyn_set_alpha_execute_id[obj_index_offset];
				}
				else	/*Bezier*/
				{
					/*transform dt*/
					trans_dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
									hmi_anim_dyn_set_alpha_anim_type[obj_index_offset],
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_dyn_duration_elapse_prop[parent_timer_id],
									dt);
					/*get every bezier length*/
					act_len	= get_bezier_length(bezier_length_list,HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT,
									hmi_anim_dyn_set_alpha_prop_table[obj_index_offset].point_cnt,
									hmi_anim_dyn_set_alpha_prop_table[obj_index_offset].pPoint_list);
					hmi_get_bezier_index(trans_dt,bezier_length_list,act_len,
									hmi_anim_dyn_set_alpha_prop_table[obj_index_offset].line_sum,
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_anim_dyn_set_alpha_prop_table[obj_index_offset].point_cnt,
									hmi_anim_dyn_set_alpha_prop_table[obj_index_offset].pPoint_list,
									&bezier_new_pos);
					#if 0
					if(bezier_new_pos.newElapse>dt)
					{
						bezier_new_pos.newElapse-=dt;
					}
					else
					{
						bezier_new_pos.newElapse=0;
					}
					#endif														
					trans_dt	= get_trans_dt(0/*start time*/,hmi_anim_dyn_set_alpha_anim_type[obj_index_offset],
									bezier_new_pos.newDur,bezier_new_pos.newElapse,dt/*dt*/);
					pos[HMI_START_POS].x	= bezier_new_pos.lineStart.x;
					pos[HMI_START_POS].y	= bezier_new_pos.lineStart.y;
					pos[HMI_END_POS].x		= bezier_new_pos.lineEnd.x;
					pos[HMI_END_POS].y		= bezier_new_pos.lineEnd.y;
					success		= get_next_step_anim32(pos,trans_dt,
									bezier_new_pos.newDur);						
					obj_executer= hmi_anim_dyn_set_alpha_execute_id[obj_index_offset];
				}
			}
			#endif
		}
		else if(HMI_IS_TIMER_ACTION_S_DURATION(parent_timer_id))/*static timer and dyn anim*/
		{
			#if (HMI_S_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_DYN_SET_ALPHA_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_S_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_ALPHA_INDEX_ANIM_DXY(obj_index);
			if((obj_index_offset < HMI_ANIM_DYN_SET_ALPHA_NUMBER)&&(parent_timer_id < HMI_S_TIMER_ACTION_DURATION_NUMBER))
			{
				if(hmi_anim_dyn_set_alpha_prop_table[obj_index_offset].point_cnt == HMI_LINE_PATH_CNT)
				{
					dt=get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
										hmi_anim_dyn_set_alpha_anim_type[obj_index_offset],
										hmi_duration_static_prop[parent_timer_id].duration,
										hmi_duration_elapse_prop[parent_timer_id],
										dt);				
					pos[HMI_START_POS].x	= (INT32)(hmi_anim_dyn_set_alpha_prop_table[obj_index_offset].pPoint_list[0].y);				
					pos[HMI_END_POS].x		= (INT32)(hmi_anim_dyn_set_alpha_prop_table[obj_index_offset].pPoint_list[1].y);
					success		= get_next_step_anim32(pos ,dt,
									hmi_duration_static_prop[parent_timer_id].duration);												
					obj_executer= hmi_anim_dyn_set_alpha_execute_id[obj_index_offset];
				}
				else /*Bezier*/
				{
					/*transform dt*/
					trans_dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
									hmi_anim_dyn_set_alpha_anim_type[obj_index_offset],
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_duration_elapse_prop[parent_timer_id],
									dt);
					/*get every bezier length*/
					act_len	= get_bezier_length(bezier_length_list,HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT,
									hmi_anim_dyn_set_alpha_prop_table[obj_index_offset].point_cnt,
									hmi_anim_dyn_set_alpha_prop_table[obj_index_offset].pPoint_list);
					hmi_get_bezier_index(trans_dt,bezier_length_list,act_len,
									hmi_anim_dyn_set_alpha_prop_table[obj_index_offset].line_sum,
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_anim_dyn_set_alpha_prop_table[obj_index_offset].point_cnt,
									hmi_anim_dyn_set_alpha_prop_table[obj_index_offset].pPoint_list,
									&bezier_new_pos);
					#if 0
					if(bezier_new_pos.newElapse>dt)
					{
						bezier_new_pos.newElapse-=dt;
					}
					else
					{
						bezier_new_pos.newElapse=0;
					}
					#endif														
					trans_dt	= get_trans_dt(0/*start time*/,hmi_anim_dyn_set_alpha_anim_type[obj_index_offset],
									bezier_new_pos.newDur,bezier_new_pos.newElapse,dt/*dt*/);
					pos[HMI_START_POS].x	= bezier_new_pos.lineStart.x;
					pos[HMI_START_POS].y	= bezier_new_pos.lineStart.y;
					pos[HMI_END_POS].x		= bezier_new_pos.lineEnd.x;
					pos[HMI_END_POS].y		= bezier_new_pos.lineEnd.y;
					success		= get_next_step_anim32(pos,trans_dt,
									bezier_new_pos.newDur);						
					obj_executer= hmi_anim_dyn_set_alpha_execute_id[obj_index_offset];
					/*convert y to alpha*/
					if(pos[HMI_NEXT_POS].y < 0)
					{
						pos[HMI_NEXT_POS].y	= 0;
					}
					pos[HMI_NEXT_POS].y	= (INT32)(((pos[HMI_NEXT_POS].y)/((float_32)HMI_LAYER_0_MAX_H_LENGTH))*HMI_MAX_ALPHA_VALUE+0.5);
					if(pos[HMI_NEXT_POS].y > HMI_MAX_ALPHA_VALUE)
					{
						pos[HMI_NEXT_POS].y	= HMI_MAX_ALPHA_VALUE;
					}
				}
			}
			#endif						
		}
		else
		{
		}				
	}
	else if((!dyn_anim)&&(HMI_IS_ALPHA_INDEX_ANIM_SXY(obj_index))) 
	{
		if(HMI_IS_TIMER_ACTION_D_DURATION(parent_timer_id))/*dyn timer and static anim*/
		{
			#if (HMI_DYN_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_STATIC_SET_ALPHA_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_D_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_ALPHA_INDEX_ANIM_SXY(obj_index);
			if((obj_index_offset < HMI_ANIM_STATIC_SET_ALPHA_NUMBER)&&(parent_timer_id < HMI_DYN_TIMER_ACTION_DURATION_NUMBER))
			{
				if(hmi_anim_static_set_alpha_prop_table[obj_index_offset].point_cnt == HMI_LINE_PATH_CNT )
				{
					dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
										hmi_anim_static_set_alpha_anim_type[obj_index_offset],
										hmi_dyn_duration_static_prop[parent_timer_id].duration,
										hmi_dyn_duration_elapse_prop[parent_timer_id],
										dt);				
					pos[HMI_START_POS].x	= (INT32)(hmi_anim_static_set_alpha_prop_table[obj_index_offset].pPoint_list[0].y);				
					pos[HMI_END_POS].x		= (INT32)(hmi_anim_static_set_alpha_prop_table[obj_index_offset].pPoint_list[1].y);				
					success		= get_next_step_anim32(pos ,dt,
									hmi_dyn_duration_static_prop[parent_timer_id].duration);								
					obj_executer= hmi_anim_static_set_alpha_execute_id[obj_index_offset];
				}
				else /*Bezier*/
				{
					/*transform dt*/
					trans_dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
									hmi_anim_static_set_alpha_anim_type[obj_index_offset],
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_dyn_duration_elapse_prop[parent_timer_id],
									dt);
					/*get every bezier length*/
					act_len	= get_bezier_length(bezier_length_list,HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT,
									hmi_anim_static_set_alpha_prop_table[obj_index_offset].point_cnt,
									hmi_anim_static_set_alpha_prop_table[obj_index_offset].pPoint_list);
					hmi_get_bezier_index(trans_dt,bezier_length_list,act_len,
									hmi_anim_static_set_alpha_prop_table[obj_index_offset].line_sum,
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_anim_static_set_alpha_prop_table[obj_index_offset].point_cnt,
									hmi_anim_static_set_alpha_prop_table[obj_index_offset].pPoint_list,
									&bezier_new_pos);
					#if 0
					if(bezier_new_pos.newElapse>dt)
					{
						bezier_new_pos.newElapse-=dt;
					}
					else
					{
						bezier_new_pos.newElapse=0;
					}
					#endif														
					trans_dt	= get_trans_dt(0/*start time*/,hmi_anim_static_set_alpha_anim_type[obj_index_offset],
									bezier_new_pos.newDur,bezier_new_pos.newElapse,dt/*dt*/);
					pos[HMI_START_POS].x	= bezier_new_pos.lineStart.x;
					pos[HMI_START_POS].y	= bezier_new_pos.lineStart.y;
					pos[HMI_END_POS].x		= bezier_new_pos.lineEnd.x;
					pos[HMI_END_POS].y		= bezier_new_pos.lineEnd.y;
					success		= get_next_step_anim32(pos,trans_dt,
									bezier_new_pos.newDur);						
					obj_executer= hmi_anim_static_set_alpha_execute_id[obj_index_offset];
					/*convert y to alpha*/
					if(pos[HMI_NEXT_POS].y < 0)
					{
						pos[HMI_NEXT_POS].y	= 0;
					}
					pos[HMI_NEXT_POS].y	= (INT32)(((pos[HMI_NEXT_POS].y)/((float_32)HMI_LAYER_0_MAX_H_LENGTH))*HMI_MAX_ALPHA_VALUE+0.5);
					if(pos[HMI_NEXT_POS].y > HMI_MAX_ALPHA_VALUE)
					{
						pos[HMI_NEXT_POS].y	= HMI_MAX_ALPHA_VALUE;
					}
				}
			}
			#endif
		}
		else if(HMI_IS_TIMER_ACTION_S_DURATION(parent_timer_id))/*static timer and static anim*/
		{
			#if (HMI_S_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_STATIC_SET_ALPHA_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_S_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_ALPHA_INDEX_ANIM_SXY(obj_index);
			if((obj_index_offset < HMI_ANIM_STATIC_SET_ALPHA_NUMBER)&&(parent_timer_id < HMI_S_TIMER_ACTION_DURATION_NUMBER))
			{
				if(hmi_anim_static_set_alpha_prop_table[obj_index_offset].point_cnt == HMI_LINE_PATH_CNT )
				{
					dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
										hmi_anim_static_set_alpha_anim_type[obj_index_offset],
										hmi_duration_static_prop[parent_timer_id].duration,
										hmi_duration_elapse_prop[parent_timer_id],
										dt);				
					pos[HMI_START_POS].x	= (INT32)(hmi_anim_static_set_alpha_prop_table[obj_index_offset].pPoint_list[0].y);				
					pos[HMI_END_POS].x		= (INT32)(hmi_anim_static_set_alpha_prop_table[obj_index_offset].pPoint_list[1].y);	
					success		= get_next_step_anim32(pos ,dt,
									hmi_duration_static_prop[parent_timer_id].duration);								
					obj_executer= hmi_anim_static_set_alpha_execute_id[obj_index_offset];
				}
				else /*Bezier*/
				{
					/*transform dt*/
					trans_dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
									hmi_anim_static_set_alpha_anim_type[obj_index_offset],
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_duration_elapse_prop[parent_timer_id],
									dt);
					/*get every bezier length*/
					act_len	= get_bezier_length(bezier_length_list,HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT,
									hmi_anim_static_set_alpha_prop_table[obj_index_offset].point_cnt,
									hmi_anim_static_set_alpha_prop_table[obj_index_offset].pPoint_list);
					hmi_get_bezier_index(trans_dt,bezier_length_list,act_len,
									hmi_anim_static_set_alpha_prop_table[obj_index_offset].line_sum,
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_anim_static_set_alpha_prop_table[obj_index_offset].point_cnt,
									hmi_anim_static_set_alpha_prop_table[obj_index_offset].pPoint_list,
									&bezier_new_pos);
					#if 0
					if(bezier_new_pos.newElapse>dt)
					{
						bezier_new_pos.newElapse-=dt;
					}
					else
					{
						bezier_new_pos.newElapse=0;
					}
					#endif														
					trans_dt	= get_trans_dt(0/*start time*/,hmi_anim_static_set_alpha_anim_type[obj_index_offset],
									bezier_new_pos.newDur,bezier_new_pos.newElapse,dt/*dt*/);
					pos[HMI_START_POS].x	= bezier_new_pos.lineStart.x;
					pos[HMI_START_POS].y	= bezier_new_pos.lineStart.y;
					pos[HMI_END_POS].x		= bezier_new_pos.lineEnd.x;
					pos[HMI_END_POS].y		= bezier_new_pos.lineEnd.y;
					success		= get_next_step_anim32(pos,trans_dt,
									bezier_new_pos.newDur);						
					obj_executer= hmi_anim_static_set_alpha_execute_id[obj_index_offset];
					/*convert y to alpha*/
					if(pos[HMI_NEXT_POS].y < 0)
					{
						pos[HMI_NEXT_POS].y	= 0;
					}
					pos[HMI_NEXT_POS].y	= (INT32)(((pos[HMI_NEXT_POS].y)/((float_32)HMI_LAYER_0_MAX_H_LENGTH))*HMI_MAX_ALPHA_VALUE+0.5);
					if(pos[HMI_NEXT_POS].y > HMI_MAX_ALPHA_VALUE)
					{
						pos[HMI_NEXT_POS].y	= HMI_MAX_ALPHA_VALUE;
					}
				}
			}
			#endif						
		}
		else
		{
		}
	}
	else
	{
	}
	if(success)
	{		
		obj_index_pro	= hmi_get_obj_pro_id(obj_executer,HMI_ELEMENT_ALPHA);/*get obj_index property ID*/
		hmi_object_data	= (HMI_OBJECT_DATA_STR)(pos[HMI_NEXT_POS].x );
		hmi_engine_set_object_info(obj_index_pro,hmi_object_data);
	}		
}
#endif
#endif

#if HMI_ANIM_STATIC_SET_ANGEL_NUMBER+HMI_ANIM_DYN_SET_ANGEL_NUMBER > 0
#ifndef HMI_GRAPHIC_ST7513
void hmi_anim_set_angle_action(HMI_OBJECT_ID_STR obj_index,BOOLEAN dyn_anim,HMI_TIME dt,
							HMI_OBJECT_ID_STR parent_timer_id)
{
	BOOLEAN				success			= FALSE;
	HMI_OBJECT_DATA_STR hmi_object_data	= 0;
	HMI_OBJECT_ID_STR	obj_index_offset= 0;
	HMI_OBJECT_ID_STR	obj_index_pro	= 0;
	HMI_OBJECT_ID_STR	obj_executer	= HMI_NB_ELEMENTS;	
	POINT_FLOAT_TP		pos[HMI_ANIM_POS_CNT]={{0,0},{0,0},{0,0}};
	//#ifdef bezier
	UINT32	 bezier_length_list[HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT];
	HMI_BEZIER_NEW_POS_STR	bezier_new_pos={{0,0},{0,0},0,0};
	UINT32				act_len		= 0;
	HMI_TIME			trans_dt	= 0;
	//#endif
	
	parent_timer_id	= HMI_GET_ACTION_ID_INDEX(parent_timer_id);
	if(dyn_anim&&(HMI_IS_ANGEL_INDEX_ANIM_DXY(obj_index)))
	{
		if(HMI_IS_TIMER_ACTION_D_DURATION(parent_timer_id))/*dyn timer and dyn anim*/
		{
			#if (HMI_DYN_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_DYN_SET_ANGEL_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_D_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_ANGEL_INDEX_ANIM_DXY(obj_index);
			if((obj_index_offset < HMI_ANIM_DYN_SET_ANGEL_NUMBER)&&(parent_timer_id < HMI_DYN_TIMER_ACTION_DURATION_NUMBER))
			{
				if(hmi_anim_dyn_set_angel_prop_table[obj_index_offset].point_cnt == HMI_LINE_PATH_CNT )
				{
					dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
										hmi_anim_dyn_set_ange_anim_type[obj_index_offset],
										hmi_dyn_duration_static_prop[parent_timer_id].duration,
										hmi_dyn_duration_elapse_prop[parent_timer_id],
										dt);				
					pos[HMI_START_POS].x	= (ANGEL_TYPE)(hmi_anim_dyn_set_angel_prop_table[obj_index_offset].pPoint_list[0].y);				
					pos[HMI_END_POS].x		= (ANGEL_TYPE)(hmi_anim_dyn_set_angel_prop_table[obj_index_offset].pPoint_list[1].y);				
					success		= get_next_step_animfloat(pos ,dt,
									hmi_dyn_duration_static_prop[parent_timer_id].duration);												
					obj_executer= hmi_anim_dyn_set_angel_execute_id[obj_index_offset];
				}
				else /*Bezier*/
				{
					/*transform dt*/
					trans_dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
									hmi_anim_dyn_set_ange_anim_type[obj_index_offset],
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_dyn_duration_elapse_prop[parent_timer_id],
									dt);
					/*get every bezier length*/
					act_len	= get_bezier_length(bezier_length_list,HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT,
									hmi_anim_dyn_set_angel_prop_table[obj_index_offset].point_cnt,
									hmi_anim_dyn_set_angel_prop_table[obj_index_offset].pPoint_list);
					hmi_get_bezier_index(trans_dt,bezier_length_list,act_len,
									hmi_anim_dyn_set_angel_prop_table[obj_index_offset].line_sum,
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_anim_dyn_set_angel_prop_table[obj_index_offset].point_cnt,
									hmi_anim_dyn_set_angel_prop_table[obj_index_offset].pPoint_list,
									&bezier_new_pos);
					#if 0
					if(bezier_new_pos.newElapse>dt)
					{
						bezier_new_pos.newElapse-=dt;
					}
					else
					{
						bezier_new_pos.newElapse=0;
					}
					#endif														
					trans_dt	= get_trans_dt(0/*start time*/,hmi_anim_dyn_set_ange_anim_type[obj_index_offset],
									bezier_new_pos.newDur,bezier_new_pos.newElapse,dt/*dt*/);
					pos[HMI_START_POS].x	= bezier_new_pos.lineStart.x;
					pos[HMI_START_POS].y	= bezier_new_pos.lineStart.y;
					pos[HMI_END_POS].x		= bezier_new_pos.lineEnd.x;
					pos[HMI_END_POS].y		= bezier_new_pos.lineEnd.y;
					success		= get_next_step_animfloat(pos,trans_dt,
									bezier_new_pos.newDur);						
					obj_executer= hmi_anim_dyn_set_angel_execute_id[obj_index_offset];					
				}
			}
			#endif
		}
		else if(HMI_IS_TIMER_ACTION_S_DURATION(parent_timer_id))/*static timer and dyn anim*/
		{
			#if (HMI_S_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_DYN_SET_ANGEL_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_S_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_ANGEL_INDEX_ANIM_DXY(obj_index);
			if((obj_index_offset < HMI_ANIM_DYN_SET_ANGEL_NUMBER)&&(parent_timer_id < HMI_S_TIMER_ACTION_DURATION_NUMBER))
			{
				if(hmi_anim_dyn_set_angel_prop_table[obj_index_offset].point_cnt == HMI_LINE_PATH_CNT )
				{
					dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
										hmi_anim_dyn_set_ange_anim_type[obj_index_offset],
										hmi_duration_static_prop[parent_timer_id].duration,
										hmi_duration_elapse_prop[parent_timer_id],
										dt);				
					pos[HMI_START_POS].x =(ANGEL_TYPE)(hmi_anim_dyn_set_angel_prop_table[obj_index_offset].pPoint_list[0].y);				
					pos[HMI_END_POS].x =(ANGEL_TYPE)(hmi_anim_dyn_set_angel_prop_table[obj_index_offset].pPoint_list[1].y);	
					success		= get_next_step_animfloat(pos ,dt,
									hmi_duration_static_prop[parent_timer_id].duration);												
					obj_executer= hmi_anim_dyn_set_angel_execute_id[obj_index_offset];
				}
				else /*Bezier*/
				{
					/*transform dt*/
					trans_dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
									hmi_anim_dyn_set_ange_anim_type[obj_index_offset],
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_duration_elapse_prop[parent_timer_id],
									dt);
					/*get every bezier length*/
					act_len	= get_bezier_length(bezier_length_list,HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT,
									hmi_anim_dyn_set_angel_prop_table[obj_index_offset].point_cnt,
									hmi_anim_dyn_set_angel_prop_table[obj_index_offset].pPoint_list);
					hmi_get_bezier_index(trans_dt,bezier_length_list,act_len,
									hmi_anim_dyn_set_angel_prop_table[obj_index_offset].line_sum,
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_anim_dyn_set_angel_prop_table[obj_index_offset].point_cnt,
									hmi_anim_dyn_set_angel_prop_table[obj_index_offset].pPoint_list,
									&bezier_new_pos);
					#if 0
					if(bezier_new_pos.newElapse>dt)
					{
						bezier_new_pos.newElapse-=dt;
					}
					else
					{
						bezier_new_pos.newElapse=0;
					}
					#endif														
					trans_dt	= get_trans_dt(0/*start time*/,hmi_anim_dyn_set_ange_anim_type[obj_index_offset],
									bezier_new_pos.newDur,bezier_new_pos.newElapse,dt/*dt*/);
					pos[HMI_START_POS].x	= bezier_new_pos.lineStart.x;
					pos[HMI_START_POS].y	= bezier_new_pos.lineStart.y;
					pos[HMI_END_POS].x		= bezier_new_pos.lineEnd.x;
					pos[HMI_END_POS].y		= bezier_new_pos.lineEnd.y;
					success		= get_next_step_animfloat(pos,trans_dt,
									bezier_new_pos.newDur);						
					obj_executer= hmi_anim_dyn_set_angel_execute_id[obj_index_offset];
				}
			}
			#endif						
		}
		else
		{
		}				
	}
	else if((!dyn_anim)&&(HMI_IS_ANGEL_INDEX_ANIM_SXY(obj_index))) 
	{
		if(HMI_IS_TIMER_ACTION_D_DURATION(parent_timer_id))/*dyn timer and static anim*/
		{
			#if (HMI_DYN_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_STATIC_SET_ANGEL_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_D_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_ANGEL_INDEX_ANIM_SXY(obj_index);
			if((obj_index_offset < HMI_ANIM_STATIC_SET_ANGEL_NUMBER)&&(parent_timer_id < HMI_DYN_TIMER_ACTION_DURATION_NUMBER))
			{
				if(hmi_anim_static_set_angel_prop_table[obj_index_offset].point_cnt == HMI_LINE_PATH_CNT)
				{
					dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
										hmi_anim_static_set_ange_anim_type[obj_index_offset],
										hmi_dyn_duration_static_prop[parent_timer_id].duration,
										hmi_dyn_duration_elapse_prop[parent_timer_id],
										dt);				
					pos[HMI_START_POS].x= (ANGEL_TYPE)(hmi_anim_static_set_angel_prop_table[obj_index_offset].pPoint_list[0].y);				
					pos[HMI_END_POS].x	= (ANGEL_TYPE)(hmi_anim_static_set_angel_prop_table[obj_index_offset].pPoint_list[1].y);					
					success		= get_next_step_animfloat(pos ,dt,
									hmi_dyn_duration_static_prop[parent_timer_id].duration);								
					obj_executer= hmi_anim_static_set_angel_execute_id[obj_index_offset];
				}
			}
			else /*Bezier*/
			{
				/*transform dt*/
					trans_dt	= get_trans_dt(hmi_dyn_duration_static_prop[parent_timer_id].start,
									hmi_anim_static_set_ange_anim_type[obj_index_offset],
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_dyn_duration_elapse_prop[parent_timer_id],
									dt);
					/*get every bezier length*/
					act_len	= get_bezier_length(bezier_length_list,HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT,
									hmi_anim_static_set_angel_prop_table[obj_index_offset].point_cnt,
									hmi_anim_static_set_angel_prop_table[obj_index_offset].pPoint_list);
					hmi_get_bezier_index(trans_dt,bezier_length_list,act_len,
									hmi_anim_static_set_angel_prop_table[obj_index_offset].line_sum,
									hmi_dyn_duration_static_prop[parent_timer_id].duration,
									hmi_anim_static_set_angel_prop_table[obj_index_offset].point_cnt,
									hmi_anim_static_set_angel_prop_table[obj_index_offset].pPoint_list,
									&bezier_new_pos);
					#if 0
					if(bezier_new_pos.newElapse>dt)
					{
						bezier_new_pos.newElapse-=dt;
					}
					else
					{
						bezier_new_pos.newElapse=0;
					}
					#endif														
					trans_dt	= get_trans_dt(0/*start time*/,hmi_anim_static_set_ange_anim_type[obj_index_offset],
									bezier_new_pos.newDur,bezier_new_pos.newElapse,dt/*dt*/);
					pos[HMI_START_POS].x	= (float_32)(bezier_new_pos.lineStart.x);
					pos[HMI_START_POS].y	= (float_32)(bezier_new_pos.lineStart.y);
					pos[HMI_END_POS].x		= (float_32)(bezier_new_pos.lineEnd.x);
					pos[HMI_END_POS].y		= (float_32)(bezier_new_pos.lineEnd.y);
					success		= get_next_step_animfloat(pos,trans_dt,
									bezier_new_pos.newDur);						
					obj_executer= hmi_anim_static_set_angel_execute_id[obj_index_offset];
			}
			#endif
		}
		else if(HMI_IS_TIMER_ACTION_S_DURATION(parent_timer_id))/*static timer and static anim*/
		{
			#if (HMI_S_TIMER_ACTION_DURATION_NUMBER > 0)&&(HMI_ANIM_STATIC_SET_ANGEL_NUMBER > 0)
			parent_timer_id	= HMI_GET_TIMER_ACTION_S_DURATION_ID_INDEX(parent_timer_id);
			obj_index_offset= HMI_GET_ANGEL_INDEX_ANIM_SXY(obj_index);
			if((obj_index_offset < HMI_ANIM_STATIC_SET_ANGEL_NUMBER)&&(parent_timer_id < HMI_S_TIMER_ACTION_DURATION_NUMBER))
			{
				if(hmi_anim_static_set_angel_prop_table[obj_index_offset].point_cnt == HMI_LINE_PATH_CNT)
				{
					dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
										hmi_anim_static_set_ange_anim_type[obj_index_offset],
										hmi_duration_static_prop[parent_timer_id].duration,
										hmi_duration_elapse_prop[parent_timer_id],
										dt);				
					pos[HMI_START_POS].x	= (ANGEL_TYPE)(hmi_anim_static_set_angel_prop_table[obj_index_offset].pPoint_list[0].y);				
					pos[HMI_END_POS].x		= (ANGEL_TYPE)(hmi_anim_static_set_angel_prop_table[obj_index_offset].pPoint_list[1].y);						
					success		= get_next_step_animfloat(pos ,dt,
									hmi_duration_static_prop[parent_timer_id].duration);								
					obj_executer= hmi_anim_static_set_angel_execute_id[obj_index_offset];
				}
				else /*Bezier*/
				{
					/*transform dt*/
					trans_dt	= get_trans_dt(hmi_duration_static_prop[parent_timer_id].start,
									hmi_anim_static_set_ange_anim_type[obj_index_offset],
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_duration_elapse_prop[parent_timer_id],
									dt);
					/*get every bezier length*/
					act_len	= get_bezier_length(bezier_length_list,HMI_MAX_BEZIER_POINT/HMI_BEZIER_POINT_CNT,
									hmi_anim_static_set_angel_prop_table[obj_index_offset].point_cnt,
									hmi_anim_static_set_angel_prop_table[obj_index_offset].pPoint_list);
					hmi_get_bezier_index(trans_dt,bezier_length_list,act_len,
									hmi_anim_static_set_angel_prop_table[obj_index_offset].line_sum,
									hmi_duration_static_prop[parent_timer_id].duration,
									hmi_anim_static_set_angel_prop_table[obj_index_offset].point_cnt,
									hmi_anim_static_set_angel_prop_table[obj_index_offset].pPoint_list,
									&bezier_new_pos);
					#if 0
					if(bezier_new_pos.newElapse>dt)
					{
						bezier_new_pos.newElapse-=dt;
					}
					else
					{
						bezier_new_pos.newElapse=0;
					}
					#endif														
					trans_dt	= get_trans_dt(0/*start time*/,hmi_anim_static_set_ange_anim_type[obj_index_offset],
									bezier_new_pos.newDur,bezier_new_pos.newElapse,dt/*dt*/);
					pos[HMI_START_POS].x	= (float_32)bezier_new_pos.lineStart.x;
					pos[HMI_START_POS].y	= (float_32)bezier_new_pos.lineStart.y;
					pos[HMI_END_POS].x		= (float_32)bezier_new_pos.lineEnd.x;
					pos[HMI_END_POS].y		= (float_32)bezier_new_pos.lineEnd.y;
					success		= get_next_step_animfloat(pos,trans_dt,
									bezier_new_pos.newDur);						
					obj_executer= hmi_anim_static_set_angel_execute_id[obj_index_offset];
				}
			}
			#endif						
		}
		else
		{
		}
	}
	else
	{
	}
	if(success)
	{		
		obj_index_pro	= hmi_get_obj_pro_id(obj_executer,HMI_ELEMENT_ANGEL);/*get obj_index property ID*/
		if(pos[HMI_NEXT_POS].x < 0.0f)
		{
			pos[HMI_NEXT_POS].x+=360;
		}
		#if HMI_SUPPORT_FLOAT_ANGEL	== HMI_YES
		hmi_object_data	= (HMI_OBJECT_DATA_STR)HMI_F32_TO_U32(pos[HMI_NEXT_POS].x );	
		#else
		hmi_object_data	= (HMI_OBJECT_DATA_STR)(pos[HMI_NEXT_POS].x );
		#endif		
		hmi_engine_set_object_info(obj_index_pro,hmi_object_data);
	}		
}
#endif
#endif

#if HMI_SET_POS_NUMBER>0
void hmi_set_pos_action(HMI_SET_POS_STR CONST *P_set_pos)	
{		
	if(P_set_pos != NULL)
	{
		if( P_set_pos->object_id_x != HMI_NB_ELEMENTS)
		{
			hmi_engine_set_object_info(P_set_pos->object_id_x,  (HMI_OBJECT_DATA_STR)(P_set_pos->x));
		}
		if(P_set_pos->object_id_y  != HMI_NB_ELEMENTS)
		{
			hmi_engine_set_object_info(P_set_pos->object_id_y,  (HMI_OBJECT_DATA_STR)(P_set_pos->y));
		}
	}

}		
#endif

#if 0
#if HMI_SET_POS_NUMBER+HMI_ANIM_DYN_SET_POS_NUMBER>0
void hmi_set_anim_pos_action(HMI_SET_POS_STR CONST *P_set_pos)	
{
	UINT8 value_x=P_set_pos->x;
	UINT8 value_y=P_set_pos->y;
	HMI_OBJECT_ID_STR object_x=P_set_pos->object_id_x;
	HMI_OBJECT_ID_STR object_y=P_set_pos->object_id_y;
	if( object_x!=HMI_NB_ELEMENTS)
	{
		hmi_engine_set_object_info(object_x,  value_x);
	}
	if(object_y !=HMI_NB_ELEMENTS)
	{
		hmi_engine_set_object_info(object_y,  value_y);
	}

}		
#endif
#endif

#if HMI_SET_DELTA_POS_NUMBER>0
void hmi_set_delta_pos_action(HMI_SET_DELTA_POS_STR CONST *P_set_delta_pos)		
{
	#if HMI_GET_INFO_FUNC==YES
	HMI_OBJECT_DATA_STR fl_data	= 0U;	
	
	if(P_set_delta_pos->delta_x !=0)
	{
		hmi_engine_get_object_info(P_set_delta_pos->object_id_x, &fl_data);
		fl_data += P_set_delta_pos->delta_x;
		hmi_engine_set_object_info(P_set_delta_pos->object_id_x,  fl_data);
	}
	if(P_set_delta_pos->delta_y !=0)
	{
		hmi_engine_get_object_info(P_set_delta_pos->object_id_y, &fl_data);
		fl_data += P_set_delta_pos->delta_y;
		hmi_engine_set_object_info(P_set_delta_pos->object_id_y,  fl_data);
	}
	#endif
}
#endif


#if HMI_SET_W_H_NUMBER > 0
void hmi_set_w_h_action(HMI_SET_W_H_STR CONST *P_set_w_h)	
{		
	if(P_set_w_h->object_id_w != HMI_NB_ELEMENTS)
	{
		hmi_engine_set_object_info(P_set_w_h->object_id_w,  P_set_w_h->width);
	}
	if(P_set_w_h->object_id_h != HMI_NB_ELEMENTS)
	{
		hmi_engine_set_object_info(P_set_w_h->object_id_h,  P_set_w_h->height);
	}
}
#endif


#if HMI_SET_DELTA_W_H_NUMBER > 0
void hmi_set_delta_w_h_action(HMI_SET_DELTA_W_H_STR CONST *P_set_delta_w_h)	
{
	#if HMI_GET_INFO_FUNC==YES
	HMI_OBJECT_DATA_STR fl_data	= 0U;	
	
	if(P_set_delta_w_h->delta_w !=0)
	{
		hmi_engine_get_object_info(P_set_delta_w_h->object_id_w, &fl_data);
		fl_data+=P_set_delta_w_h->delta_w;
		hmi_engine_set_object_info(P_set_delta_w_h->object_id_w,  fl_data);
	}
	if(P_set_delta_w_h->delta_h !=0)
	{
		hmi_engine_get_object_info(P_set_delta_w_h->object_id_h, &fl_data);
		fl_data+=P_set_delta_w_h->delta_h;
		hmi_engine_set_object_info(P_set_delta_w_h->object_id_h,  fl_data);
	}
	#endif
}
#endif


#if HMI_SET_DYN_CONTAINER_NUMBER>0
void hmi_set_dyn_container_action(HMI_SET_DYN_CONTAINER_STR CONST  *P_set_dyn_container)	
{	
	if((P_set_dyn_container->object_id >= HMI_DYN_TEXTS_MAX_ID)&&
		(P_set_dyn_container->object_id < HMI_DYN_CONTAINERS_MAX_ID))
	{
		hmi_engine_set_object_info(P_set_dyn_container->object_id,P_set_dyn_container->child);
	}
}
#endif


#if HMI_SET_PAGE_ON_OFF_NUMBER>0
void hmi_set_page_on_off_action(HMI_SET_PAGE_ON_OFF_STR CONST *P_set_page_on_off)	
{	
	if(P_set_page_on_off->object_id < HMI_PAGE_SXY_MAX_ID)
	{
		if(P_set_page_on_off->page_on_off)
		{
			hmi_engine_set_object_info(P_set_page_on_off->object_id,HMI_ACTIVE_PAGE_BIT);
		}
		else
		{
			hmi_engine_set_object_info(P_set_page_on_off->object_id,HMI_REMOVE_PAGE_BIT);
		}
	}
}
#endif


#if HMI_SET_EDIT_TEXT_NUMBER > 0
void hmi_set_edit_text_action(HMI_SET_EDIT_TEXT_STR CONST *P_set_edit_text)	
{	
	if((P_set_edit_text->object_id >= HMI_DYN_BUTTON_SXY_MAX_ID) && 
		(P_set_edit_text->object_id < HMI_DYN_TEXTS_MAX_ID))
	{
		hmi_engine_edit_text(P_set_edit_text->object_id, 
							P_set_edit_text->changed_string);
	}
}	
#endif


#if HMI_SET_TEXT_SCROLL_STEP_NUMBER > 0
void hmi_set_text_scroll_step_action(HMI_SET_TEXT_SCROLL_STEP_STR CONST *P_set_text_scroll_step)	
{	
	if((P_set_text_scroll_step->object_id_scroll >= HMI_STATIC_TEXTS_MAX_ID) && 
		(P_set_text_scroll_step->object_id_scroll < HMI_SET_TEXT_SCROLL_STEP_MAX_ID/*need review */))
	{
	   hmi_engine_set_object_info(P_set_text_scroll_step->object_id_scroll, 
	   				(HMI_OBJECT_DATA_STR)(P_set_text_scroll_step->step));
	}
}
#endif

#if HMI_SET_FOR_COLOR_NUMBER > 0			
void hmi_set_foreground_color_action(HMI_SET_COLOR_STR CONST* P_set_for_color_prop_table)
{	
	hmi_engine_set_object_info(P_set_for_color_prop_table->object_id_color,(HMI_OBJECT_DATA_STR)(P_set_for_color_prop_table->color));
}
#endif

#if HMI_SET_BCK_COLOR_NUMBER > 0		
void hmi_set_background_color_action(HMI_SET_COLOR_STR CONST* P_set_bck_color_prop_table)
{	
	hmi_engine_set_object_info(P_set_bck_color_prop_table->object_id_color,P_set_bck_color_prop_table->color);
}
#endif
#if 0
#if HMI_SET_BTN_STATUS_NUMBER>0		
void hmi_set_button_status_action(HMI_SET_BTN_STATUS_STR CONST* P_set_button_status_prop_table)
{
	HMI_OBJECT_ID_STR button_id=P_set_button_status_prop_table->button_id;
	hmi_engine_set_object_info(button_id,P_set_button_status_prop_table->status);
}
#endif
#endif
#if HMI_SET_SEND_EVENT_NUMBER > 0		
void hmi_set_send_event_action(HMI_SET_SEND_EVENT_STR CONST* P_set_send_event_prop_table)
{
	hmi_action_send_event(P_set_send_event_prop_table->event_id);
}
#endif

#if HMI_SET_CALL_FUNC_NUMBER > 0		
void hmi_set_call_function_action(HMI_SET_CALL_FUNC_STR CONST* P_set_call_func_prop_table)
{	
	(P_set_call_func_prop_table->P_func)();
}
#endif
#if HMI_SET_IMAGELIST_INDEX_NUMBER > 0
void hmi_set_imagelist_index_action(HMI_SET_RANGE_STR CONST* P_set_imagelist_index_prop_table)
{	
	hmi_engine_set_object_info(P_set_imagelist_index_prop_table->object_id,
					P_set_imagelist_index_prop_table->index);
}
#endif
#if HMI_SET_SCROLLBAR_RANGE_NUMBER > 0
void hmi_set_scrollbar_range_action(HMI_SET_RANGE_STR CONST* P_set_scrollbar_range_prop_table)
{	
	hmi_engine_set_object_info(P_set_scrollbar_range_prop_table->object_id,P_set_scrollbar_range_prop_table->index);
}
#endif
#if HMI_SET_BUTTON_STATUS_NUMBER > 0
void hmi_set_button_status_action(HMI_SET_RANGE_STR CONST* P_set_button_status_prop_table)
{	
	hmi_engine_set_object_info(P_set_button_status_prop_table->object_id,
								P_set_button_status_prop_table->index);
}
#endif

#if 0
#if HMI_TIMER_ACTION_NUMBER>0
void hmi_reset_timer_action(HMI_OBJECT_ID_STR action_id)
{
	UINT8 									loop=0U;
	UINT8 									nb_child =0U;
    HMI_OBJECT_PROP_STR CONST 				*P_child_t=NULL;
	static HMI_EVENT_ACTION_RES_STR	CONST 	*P_repeat_data=NULL;
	HMI_REPEAT_ONETIME_PROP_STR 			*P_repeat_prop=NULL;
	HMI_OBJECT_TABLE_STR 			CONST	*P_repeat_rel=NULL;
	static HMI_EVENT_ACTION_RES_STR	CONST 	*P_onetime_data=NULL;
	HMI_REPEAT_ONETIME_PROP_STR 			*P_onetime_prop=NULL;
	HMI_OBJECT_TABLE_STR 			CONST	*P_onetime_rel=NULL;
	static HMI_EVENT_ACTION_RES_STR CONST	*P_duration_data=NULL;
	HMI_DURATION_PROP_STR					*P_duration_prop=NULL;
	HMI_OBJECT_TABLE_STR 			CONST	*P_duration_rel=NULL;
	HMI_OBJECT_ID_STR 						action_id_index=0U;
	
	
	if((action_id >=HMI_ACTION_GROUP_MAX_ID)&&(action_id < HMI_ACTION_MAX_ID))
	{
		action_id_index=HMI_GET_ACTION_ID_INDEX(action_id);
		if(action_id_index <HMI_TIMER_ACTION_MAX_ID)
		{
			#if HMI_TIMER_ACTION_REPEAT_NUMBER>0
			if(action_id_index < HMI_TIMER_ACTION_REPEAT_MAX_ID)
			{
				if(P_repeat_data==NULL)
				{
					P_repeat_data=hmi_get_action_list_point(action_id);
				}
				if(P_repeat_data !=NULL)
				{
					P_repeat_rel=P_repeat_data->P_object_relative;
					P_repeat_prop=(HMI_REPEAT_ONETIME_PROP_STR *)(P_repeat_data->P_object_prop);
					
					P_repeat_prop->P_elapse[action_id_index]=0U;
					nb_child=P_repeat_rel[action_id_index].object_number;
					P_child_t=P_repeat_rel[action_id_index].p_object_table;
				}
			}
			else
			#endif
			#if HMI_TIMER_ACTION_ONETIME_NUMBER>0
			if(action_id_index < HMI_TIMER_ACTION_ONETIME_MAX_ID)
			{
				action_id_index=HMI_GET_TIMER_ACTION_ONETIME_ID_INDEX(action_id_index);
				if(P_onetime_data==NULL)
				{
					P_onetime_data=hmi_get_action_list_point(action_id);
				}
				if(P_onetime_data !=NULL)
				{
					P_onetime_rel=P_onetime_data->P_object_relative;
					P_onetime_prop=(HMI_REPEAT_ONETIME_PROP_STR *)(P_onetime_data->P_object_prop);
					
					P_onetime_prop->P_elapse[action_id_index]=0U;
					nb_child=P_onetime_rel[action_id_index].object_number;
					P_child_t=P_onetime_rel[action_id_index].p_object_table;
				}
			}
			else
			#endif
			#if HMI_TIMER_ACTION_DURATION_NUMBER>0
			if(action_id_index < HMI_TIMER_ACTION_DURATION_MAX_ID)
			{
				action_id_index=HMI_GET_TIMER_ACTION_DURATION_ID_INDEX(action_id_index);
				if(P_duration_data ==NULL)
				{
					P_duration_data=hmi_get_action_list_point(action_id);
				}
				if(P_duration_data !=NULL)
				{
					P_duration_rel=P_duration_data->P_object_relative;
					P_duration_prop=(HMI_DURATION_PROP_STR*)(P_duration_data->P_object_prop);
					
					P_duration_prop->P_elapse[action_id_index]=0U;
					nb_child = P_duration_rel[action_id_index].object_number;
					P_child_t=P_duration_rel[action_id_index].p_object_table;
				}

			}
			else
			#endif
			{
			}
		}
	}

	for(loop=0;loop<nb_child;loop++)
	{
		hmi_reset_timer_action(P_child_t[loop].object_id);
	}
}
#endif
#endif

#if HMI_TIMER_ACTION_NUMBER>0
void hmi_reset_timer_action(HMI_OBJECT_ID_STR action_id)
{	
	UINT8 							nb_child	= 0U;
    HMI_OBJECT_PROP_STR CONST 		*P_child_t	= NULL;	
	HMI_OBJECT_ID_STR 				action_id_index = 0U;
		
	if((action_id >= HMI_ACTION_GROUP_MAX_ID)&&(action_id < HMI_ACTION_MAX_ID))
	{
		action_id_index	= HMI_GET_ACTION_ID_INDEX(action_id);
		if(action_id_index < HMI_TIMER_ACTION_MAX_ID)
		{
			#if HMI_TIMER_ACTION_REPEAT_NUMBER>0
			if(action_id_index < HMI_TIMER_ACTION_REPEAT_MAX_ID)
			{
				if(action_id_index < HMI_TIMER_ACTION_REPEAT_NUMBER)
				{
					hmi_repeat_elapse_prop[action_id_index]	= 0;
					nb_child	= hmi_timer_action_repeat_relative[action_id_index].object_number;
					P_child_t	= hmi_timer_action_repeat_relative[action_id_index].p_object_table;								
				}
			}
			else
			#endif
			#if HMI_TIMER_ACTION_ONETIME_NUMBER>0
			if(action_id_index < HMI_TIMER_ACTION_ONETIME_MAX_ID)
			{
				action_id_index	= HMI_GET_TIMER_ACTION_ONETIME_ID_INDEX(action_id_index);
				if(action_id_index < HMI_TIMER_ACTION_ONETIME_NUMBER)
				{
					hmi_onetime_elapse_prop[action_id_index]	= 0;
					nb_child	= hmi_timer_action_onetime_relative[action_id_index].object_number;
					P_child_t	= hmi_timer_action_onetime_relative[action_id_index].p_object_table;				
				}
			}
			else
			#endif
			#if HMI_S_TIMER_ACTION_DURATION_NUMBER>0
			if(action_id_index < HMI_TIMER_ACTION_S_DURATION_MAX_ID)
			{
				action_id_index	= HMI_GET_TIMER_ACTION_S_DURATION_ID_INDEX(action_id_index);
				if(action_id_index < HMI_S_TIMER_ACTION_DURATION_NUMBER)
				{
					hmi_duration_elapse_prop[action_id_index]	= 0;
					nb_child	= hmi_timer_action_duration_relative[action_id_index].object_number;
					P_child_t	= hmi_timer_action_duration_relative[action_id_index].p_object_table;
				}								
			}
			else
			#endif
			#if HMI_DYN_TIMER_ACTION_DURATION_NUMBER>0
			if(action_id_index < HMI_TIMER_ACTION_D_DURATION_MAX_ID)
			{
				action_id_index	= HMI_GET_TIMER_ACTION_D_DURATION_ID_INDEX(action_id_index);
				if(action_id_index < HMI_DYN_TIMER_ACTION_DURATION_NUMBER)
				{
					hmi_dyn_duration_elapse_prop[action_id_index]	= 0;
					nb_child	= hmi_timer_action_dyn_duration_relative[action_id_index].object_number;
					P_child_t	= hmi_timer_action_dyn_duration_relative[action_id_index].p_object_table;
				}								
			}
			else
			#endif
			{
			}
		}
	}
	if(P_child_t != NULL)
	{
		for(action_id_index=0;action_id_index < nb_child;action_id_index++)
		{
			hmi_reset_timer_action(P_child_t[action_id_index].object_id);
		}
	}
}
#endif



#if (HMI_ALL_EVENT_NUMBER+HMI_ALL_ACTION_NUMBER+HMI_ACTION_GROUP_NUMBER) > 0 
HMI_EVENT_ACTION_RES_STR CONST* hmi_get_action_list_point(HMI_OBJECT_ID_STR action_id)
{		
	HMI_EVENT_ACTION_RES_STR CONST		*P_action_list=NULL;
	
	if ((action_id >= HMI_EVENT_ACT_BEGIN_INDEX)&&(action_id < HMI_ACTION_MAX_ID))/*event,act,group*/
	{
		#if HMI_ALL_EVENT_NUMBER > 0
		if(action_id < HMI_EVENT_MAX_ID)
		{
			action_id	= HMI_GET_EVENT_ID_INDEX(action_id);
			#if HMI_EVENT_STAND_NUMBER > 0
			if(action_id < HMI_EVENT_STAND_MAX_ID)
			{
				P_action_list	= hmi_event_action_table->P_event->P_stand_event;
			}
			else 
			#endif
			#if HMI_EVENT_CUSTOM_NUMBER > 0
			if(action_id < HMI_EVENT_CUSTOM_MAX_ID)
			{
				P_action_list	= hmi_event_action_table->P_event->P_custom_event;
			}
			else
			#endif 
			{
			}
		}
		else
		#endif
		#if HMI_ACTION_GROUP_NUMBER > 0
		if(action_id < HMI_ACTION_GROUP_MAX_ID)
		{
			P_action_list	= hmi_event_action_table->P_action_group;
		}
		else
		#endif
		#if HMI_ALL_ACTION_NUMBER > 0
		if(action_id < HMI_ACTION_MAX_ID)
		{
			action_id	= HMI_GET_ACTION_ID_INDEX(action_id);
			#if HMI_TIMER_ACTION_NUMBER > 0
			if(action_id < HMI_TIMER_ACTION_MAX_ID)
			{
				#if HMI_TIMER_ACTION_REPEAT_NUMBER > 0
				if(action_id < HMI_TIMER_ACTION_REPEAT_MAX_ID)
				{
					P_action_list	= hmi_event_action_table->P_action->P_timer_action_table->P_repeat;
				}
				else
				#endif
				#if HMI_TIMER_ACTION_ONETIME_NUMBER > 0
				if(action_id < HMI_TIMER_ACTION_ONETIME_MAX_ID)
				{
					P_action_list	= hmi_event_action_table->P_action->P_timer_action_table->P_onetime;

				}
				else
				#endif
				#if HMI_S_TIMER_ACTION_DURATION_NUMBER > 0
				if(action_id < HMI_TIMER_ACTION_S_DURATION_MAX_ID)
				{
					P_action_list	= hmi_event_action_table->P_action->P_timer_action_table->P_duration;

				}
				else
				#endif
				#if HMI_DYN_TIMER_ACTION_DURATION_NUMBER > 0
				if(action_id < HMI_TIMER_ACTION_D_DURATION_MAX_ID)
				{
					P_action_list	= hmi_event_action_table->P_action->P_timer_action_table->P_dyn_duration;

				}
				else
				#endif
				{
				}
				
			}
			else
			#endif
			#if HMI_SET_ACTION_NUMBER > 0
			if(action_id < HMI_SET_ACTION_MAX_ID)
			{
				P_action_list	= hmi_event_action_table->P_action->P_set_action_table;
			}
			else
			#endif
			{
			}
			
		}
		else
		#endif
		{
		}
	}
	return (P_action_list) ;
}
#endif

/*Set dyn pos start and end*/
#if HMI_ANIM_DYN_SET_POS_NUMBER > 0
void hmi_set_dyn_pos_s_e(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data,BOOLEAN start)
{
	SINT16	x	= 0;
	SINT16	y	= 0;

	if(hmi_object_id < HMI_ANIM_DYN_SET_POS_NUMBER)
	{
		x	= (SINT16)hmi_object_data;
		y	= (SINT16)(HMI_U32_HIG16(hmi_object_data));	
		if(start)
		{
			if(x != HMI_SINT16_MAX)
			{				
				hmi_anim_dyn_set_pos_prop_table[hmi_object_id].pPoint_list[0/*start*/].x = x;
			}
			if(y != HMI_SINT16_MAX)
			{
				hmi_anim_dyn_set_pos_prop_table[hmi_object_id].pPoint_list[0/*start*/].y = y;
			}
		}
		else
		{
			if(x != HMI_SINT16_MAX)
			{
				hmi_anim_dyn_set_pos_prop_table[hmi_object_id].pPoint_list[1/*end*/].x = x;
			}
			if(y != HMI_SINT16_MAX)
			{
				hmi_anim_dyn_set_pos_prop_table[hmi_object_id].pPoint_list[1/*end*/].y = y;
			}
		}
	}
}
HMI_OBJECT_DATA_STR hmi_get_dyn_pos_s_e(HMI_OBJECT_ID_STR hmi_object_id,BOOLEAN start)
{
	HMI_OBJECT_DATA_STR hmi_object_data = 0;
	SINT16	x	= 0;
	SINT16	y	= 0;

	if(hmi_object_id < HMI_ANIM_DYN_SET_POS_NUMBER)
	{		
		if(start)
		{
			x	= hmi_anim_dyn_set_pos_prop_table[hmi_object_id].pPoint_list[0/*start*/].x;
			y	= hmi_anim_dyn_set_pos_prop_table[hmi_object_id].pPoint_list[0/*start*/].y;
		}
		else
		{
			x	= hmi_anim_dyn_set_pos_prop_table[hmi_object_id].pPoint_list[1/*end*/].x;
			y	= hmi_anim_dyn_set_pos_prop_table[hmi_object_id].pPoint_list[1/*end*/].y;
		}
	}
	hmi_object_data	= (HMI_OBJECT_DATA_STR)(HMI_2U16_TO_U32(x,y));
	return hmi_object_data;
}

#endif
/*Set dyn width heigh start and end*/
#if HMI_ANIM_DYN_SET_W_H_NUMBER > 0
void hmi_set_dyn_wh_s_e(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data,BOOLEAN start)
{
	UINT16	w	= 0;
	UINT16	h	= 0;

	if(hmi_object_id < HMI_ANIM_DYN_SET_W_H_NUMBER)
	{
		if(hmi_anim_dyn_set_w_h_prop_table[hmi_object_id].point_cnt == HMI_LINE_PATH_CNT )
		{
			w	= (UINT16)hmi_object_data;
			h	= (UINT16)(HMI_U32_HIG16(hmi_object_data));	
			if(start)
			{
				if(w != HMI_U16_MAX)
				{
					hmi_anim_dyn_set_w_h_prop_table[hmi_object_id].pPoint_list[0].x = w;
				}
				if(h != HMI_U16_MAX)
				{
					hmi_anim_dyn_set_w_h_prop_table[hmi_object_id].pPoint_list[0].x = h;
				}
			}
			else
			{
				if(w != HMI_U16_MAX)
				{
					hmi_anim_dyn_set_w_h_prop_table[hmi_object_id].pPoint_list[1].x = w;
				}
				if(h != HMI_U16_MAX)
				{
					hmi_anim_dyn_set_w_h_prop_table[hmi_object_id].pPoint_list[1].y = h;
				}
			}
		}
	}
}

HMI_OBJECT_DATA_STR hmi_get_dyn_wh_s_e(HMI_OBJECT_ID_STR hmi_object_id,BOOLEAN start)
{
	HMI_OBJECT_DATA_STR hmi_object_data = 0;
	UINT16	w	= 0;
	UINT16	h	= 0;

	if(hmi_object_id < HMI_ANIM_DYN_SET_W_H_NUMBER)
	{	
		if(hmi_anim_dyn_set_w_h_prop_table[hmi_object_id].point_cnt == HMI_LINE_PATH_CNT )
		{
			if(start)
			{			
				w	= hmi_anim_dyn_set_w_h_prop_table[hmi_object_id].pPoint_list[0].x;		
				h	= hmi_anim_dyn_set_w_h_prop_table[hmi_object_id].pPoint_list[0].y;			
			}
			else
			{			
				w	= hmi_anim_dyn_set_w_h_prop_table[hmi_object_id].pPoint_list[1].x;			
				h	= hmi_anim_dyn_set_w_h_prop_table[hmi_object_id].pPoint_list[1].y;			
			}
		}
	}
	hmi_object_data	= HMI_2U16_TO_U32(w,h);
	return hmi_object_data;
}

#endif
/*Set dyn fcolor start and end*/
#if HMI_ANIM_DYN_SET_FOR_COLOR_NUMBER > 0
void hmi_set_dyn_fcolor_s_e(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data,BOOLEAN start)
{
	HMI_COLOR_STR fcolor	= 0;

	if(hmi_object_id < HMI_ANIM_DYN_SET_FOR_COLOR_NUMBER)
	{
		fcolor	= (HMI_COLOR_STR)hmi_object_data;
		if(start)
		{
			hmi_anim_dyn_set_for_color_prop_table[hmi_object_id].pPoint_list[0].y	= (INT32)fcolor;
		}
		else
		{
			hmi_anim_dyn_set_for_color_prop_table[hmi_object_id].pPoint_list[1].y	= (INT32)fcolor;
		}
	}
}
HMI_OBJECT_DATA_STR hmi_get_dyn_fcolor_s_e(HMI_OBJECT_ID_STR hmi_object_id,BOOLEAN start)
{
	HMI_OBJECT_DATA_STR hmi_object_data	= 0;	

	if(hmi_object_id < HMI_ANIM_DYN_SET_FOR_COLOR_NUMBER)
	{		
		if(start)
		{
			hmi_object_data	= (HMI_OBJECT_DATA_STR)(hmi_anim_dyn_set_for_color_prop_table[hmi_object_id].pPoint_list[0].y);
		}
		else
		{
			hmi_object_data	= (HMI_OBJECT_DATA_STR)(hmi_anim_dyn_set_for_color_prop_table[hmi_object_id].pPoint_list[1].y);
		}
	}
	return hmi_object_data;
}

#endif
/*Set dyn image list start and end*/
#if HMI_ANIM_DYN_SET_IMAGELIST_INDEX_NUMBER > 0
void hmi_set_dyn_imglist_s_e(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data,BOOLEAN start)
{
	HMI_RANGE_STR index	= 0;

	if(hmi_object_id < HMI_ANIM_DYN_SET_IMAGELIST_INDEX_NUMBER)
	{
		index	= (HMI_RANGE_STR)hmi_object_data;
		if(start)
		{
			hmi_anim_dyn_set_imagelist_index_prop_table[hmi_object_id].pPoint_list[0].y	= index;
		}
		else
		{
			hmi_anim_dyn_set_imagelist_index_prop_table[hmi_object_id].pPoint_list[1].y	= index;
		}
	}
}
HMI_RANGE_STR hmi_get_dyn_imglist_s_e(HMI_OBJECT_ID_STR hmi_object_id,BOOLEAN start)
{
	HMI_RANGE_STR index	= 0;

	if(hmi_object_id < HMI_ANIM_DYN_SET_IMAGELIST_INDEX_NUMBER)
	{		
		if(start)
		{
			index	= hmi_anim_dyn_set_imagelist_index_prop_table[hmi_object_id].pPoint_list[0].y;
		}
		else
		{
			index	= hmi_anim_dyn_set_imagelist_index_prop_table[hmi_object_id].pPoint_list[1].y;
		}
	}
	return index;
}

#endif
/*Set dyn alpha start and end*/
#if HMI_ANIM_DYN_SET_ALPHA_NUMBER > 0
void hmi_set_dyn_alpha_s_e(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data,BOOLEAN start)
{
	ALPHA_TYPE alpha	= 0;

	if(hmi_object_id < HMI_ANIM_DYN_SET_ALPHA_NUMBER)
	{
		if(hmi_anim_dyn_set_alpha_prop_table[hmi_object_id].point_cnt == HMI_LINE_PATH_CNT)
		{
			alpha	= (ALPHA_TYPE)hmi_object_data;
			if(start)
			{
				hmi_anim_dyn_set_alpha_prop_table[hmi_object_id].pPoint_list[0].y	= alpha;
			}
			else
			{
				hmi_anim_dyn_set_alpha_prop_table[hmi_object_id].pPoint_list[1].y	= alpha;
			}
		}
	}
}
ALPHA_TYPE hmi_get_dyn_alpha_s_e(HMI_OBJECT_ID_STR hmi_object_id,BOOLEAN start)
{
	ALPHA_TYPE alpha	= 0;

	if(hmi_object_id < HMI_ANIM_DYN_SET_ALPHA_NUMBER)
	{	
		if(hmi_anim_dyn_set_alpha_prop_table[hmi_object_id].point_cnt == HMI_LINE_PATH_CNT)
		{
			if(start)
			{
				alpha	= hmi_anim_dyn_set_alpha_prop_table[hmi_object_id].pPoint_list[0].y;
			}
			else
			{
				alpha	= hmi_anim_dyn_set_alpha_prop_table[hmi_object_id].pPoint_list[1].y;
			}
		}
	}
	return alpha;
}
#endif
/*Set dyn angel start and end*/
#if HMI_ANIM_DYN_SET_ANGEL_NUMBER>0
#ifndef HMI_GRAPHIC_TWLIB
#ifndef HMI_GRAPHIC_ST7513
void hmi_set_dyn_angel_s_e(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data,BOOLEAN start)
{
	ANGEL_TYPE angel	= 0;

	if(hmi_object_id < HMI_ANIM_DYN_SET_ANGEL_NUMBER)
	{
		if(hmi_anim_dyn_set_angel_prop_table[hmi_object_id].point_cnt == HMI_LINE_PATH_CNT )
		{
			#if HMI_SUPPORT_FLOAT_ANGEL	== HMI_YES
			angel	= HMI_U32_TO_F32(hmi_object_data);	
			#else
			angel	= hmi_object_data;	
			#endif
			if(start)	
			{
				hmi_anim_dyn_set_angel_prop_table[hmi_object_id].pPoint_list[0].y	= angel;
			}
			else
			{
				hmi_anim_dyn_set_angel_prop_table[hmi_object_id].pPoint_list[1].y	= angel;
			}
		}
	}
}
#endif
#endif

#if HMI_ANIM_DYN_SET_ANGEL_NUMBER > 0
ANGEL_TYPE hmi_get_dyn_angel_s_e(HMI_OBJECT_ID_STR hmi_object_id,BOOLEAN start)
{
	ANGEL_TYPE angel	= 0;

	if(hmi_object_id < HMI_ANIM_DYN_SET_ANGEL_NUMBER)
	{		
		if(hmi_anim_dyn_set_angel_prop_table[hmi_object_id].point_cnt == HMI_LINE_PATH_CNT)
		{
			if(start)
			{
				angel	= hmi_anim_dyn_set_angel_prop_table[hmi_object_id].pPoint_list[0].y;
			}
			else
			{
				angel	= hmi_anim_dyn_set_angel_prop_table[hmi_object_id].pPoint_list[1].y;
			}
		}
	}
	return angel;
}
#endif
#endif
#if HMI_ANIM_DYN_SET_SCALE_NUMBER>0
#ifndef HMI_GRAPHIC_TWLIB
#ifndef HMI_GRAPHIC_ST7513
void hmi_set_dyn_scale_s_e(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data,BOOLEAN start)
{
	ANGEL_TYPE scale	= 0;

	if(hmi_object_id < HMI_ANIM_DYN_SET_SCALE_NUMBER)
	{
		if(hmi_anim_dyn_set_scale_prop_table[hmi_object_id].point_cnt == HMI_LINE_PATH_CNT )
		{
		#if HMI_SUPPORT_FLOAT_ANGEL	== HMI_YES
			scale	= HMI_U32_TO_F32(hmi_object_data);	
		#else
			scale	= hmi_object_data;	
		#endif
			if(start)	
			{
				hmi_anim_dyn_set_scale_prop_table[hmi_object_id].pPoint_list[0].y	= scale;
			}
			else
			{
				hmi_anim_dyn_set_scale_prop_table[hmi_object_id].pPoint_list[1].y	= scale;
			}
		}
	}
}
#endif
#endif
	
#if HMI_ANIM_DYN_SET_SCALE_NUMBER > 0
	ANGEL_TYPE hmi_get_dyn_scale_s_e(HMI_OBJECT_ID_STR hmi_object_id,BOOLEAN start)
	{
		ANGEL_TYPE scale	= 0;
	
		if(hmi_object_id < HMI_ANIM_DYN_SET_SCALE_NUMBER)
		{		
			if(hmi_anim_dyn_set_scale_prop_table[hmi_object_id].point_cnt == HMI_LINE_PATH_CNT)
			{
				if(start)
				{
					scale	= hmi_anim_dyn_set_scale_prop_table[hmi_object_id].pPoint_list[0].y;
				}
				else
				{
					scale	= hmi_anim_dyn_set_scale_prop_table[hmi_object_id].pPoint_list[1].y;
				}
			}
		}
		return scale;
	}
#endif
#endif


#if HMI_DYN_TIMER_ACTION_DURATION_NUMBER>0
void hmi_set_timer_s_e(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data,SET_TIMER_ENUM timer_prop )
{
	HMI_TIME time	= 0;
	
	time	= HMI_U32_TO_F32(hmi_object_data);
	if(timer_prop == HMI_SET_START)
	{
		if(hmi_object_id < HMI_DYN_TIMER_ACTION_DURATION_NUMBER)
		{
			hmi_dyn_duration_static_prop[hmi_object_id].start	= time;
		}
	}
	else if(timer_prop == HMI_SET_DURATION)
	{
		if(hmi_object_id < HMI_DYN_TIMER_ACTION_DURATION_NUMBER)
		{
			hmi_dyn_duration_static_prop[hmi_object_id].duration	= time;
		}
	}
	else if(timer_prop == HMI_SET_ELAPSE)
	{
		if(hmi_object_id < HMI_DYN_TIMER_ACTION_DURATION_NUMBER)
		{
			hmi_dyn_duration_elapse_prop[hmi_object_id]=time;
		}
	}
	else
	{
		;
	}
}
HMI_TIME hmi_get_timer_s_e(HMI_OBJECT_ID_STR hmi_object_id,SET_TIMER_ENUM timer_prop )
{
	HMI_TIME time	= 0;
		
	if(timer_prop == HMI_SET_START)
	{
		if(hmi_object_id < HMI_DYN_TIMER_ACTION_DURATION_NUMBER)
		{
			time	= hmi_dyn_duration_static_prop[hmi_object_id].start;
		}
	}
	else if(timer_prop == HMI_SET_DURATION)
	{
		if(hmi_object_id < HMI_DYN_TIMER_ACTION_DURATION_NUMBER)
		{
			time	= hmi_dyn_duration_static_prop[hmi_object_id].duration;
		}
	}
	else if(timer_prop == HMI_SET_ELAPSE)
	{
		if(hmi_object_id < HMI_DYN_TIMER_ACTION_DURATION_NUMBER)
		{
			time	= hmi_dyn_duration_elapse_prop[hmi_object_id];
		}
	}
	else
	{
		;
	}
	return time;
}

#endif


#if HMI_ACTION_GROUP_NUMBER > 0
void hmi_remove_action_group(HMI_OBJECT_ID_STR action_gp_id)
{
	UINT8 reference	= 0U;
	BOOLEAN success	= FALSE;
	
	if((action_gp_id >= HMI_EVENT_MAX_ID)&&(action_gp_id < HMI_ACTION_GROUP_MAX_ID))
	{
		while((reference < HMI_MAX_RUN_ACTION_CNT)&&(success == FALSE))
		{
			if(hmi_active_action_group_table[reference] == action_gp_id)
			{
				hmi_active_action_group_table[reference]	= 0U;
				success	= TRUE;
			}
			else
			{
				reference++;
			}			
		}
	}
}

#define		HMI_MAX_DELTA_TIME			999999.0f	/*lq 2018 11 13*/
void  hmi_set_action_status(HMI_OBJECT_ID_STR action_gp_id,HMI_OBJECT_DATA_STR status)
{
	
	if(status == HMI_ACTION_RUN ||status == HMI_ACTION_CONTINUE)
	{
		hmi_add_action_group(action_gp_id,status,TRUE);
	}
	else if(status == HMI_ACTION_PAUSE)
	{
		hmi_remove_action_group(action_gp_id);
	}
	else if(status == HMI_ACTION_FAST_FINISH)/*lq 2018 11 13*/
	{
		hmi_do_action_group(action_gp_id,HMI_MAX_DELTA_TIME);
		hmi_remove_action_group(action_gp_id);
	}
	else
	{
	}
}
#endif

#endif
POINT32_TP hor(U08 degree, POINT32_TP CONST *point, double_64 t)//hornbez
{
	SINT32		i	=0;
	SINT32	choose_i=0;
	double_64	fact=0;
	double_64	t1 	=0;
	double_64	x	=0;
	double_64	y	=0;
	POINT32_TP aux={0,0};
	
	t1	= 1.0 - t;
	fact= 1.0;
	choose_i = 1;
	x	= (double_64)point[0].x*t1;
	y	= (double_64)point[0].y*t1;
	for(i=1;i < degree;i++)
	{
		fact	= fact*t;
		choose_i= choose_i*(degree-i+1)/i;
		x		= (x+fact*choose_i*point[i].x)*t1;
		y		= (y+fact*choose_i*point[i].y)*t1;
	}
	x	= x + fact*t*(double_64)point[degree].x;
	y	= y + fact*t*(double_64)point[degree].y;
	
	aux.x	= (INT32)(x+0.5f);/*lq*/
	aux.y	= (INT32)(y+0.5f);/*lq*/
	
	return aux;
}


POINT32_TP hor_f(U08 degree, POINT_FLOAT_TP CONST *point, double_64 t)//hornbez
{
	SINT32		i	=0;
	SINT32	choose_i=0;
	double_64	fact=0;
	double_64	t1 	=0;
	double_64	x	=0;
	double_64	y	=0;
	POINT32_TP aux={0,0};
	
	t1	= 1.0 - t;
	fact= 1.0;
	choose_i = 1;
	x	= (double_64)point[0].x*t1;
	y	= (double_64)point[0].y*t1;
	for(i=1;i < degree;i++)
	{
		fact	= fact*t;
		choose_i= choose_i*(degree-i+1)/i;
		x		= (x+fact*choose_i*point[i].x)*t1;
		y		= (y+fact*choose_i*point[i].y)*t1;
	}
	x	= x + fact*t*(double_64)point[degree].x;
	y	= y + fact*t*(double_64)point[degree].y;
	
	aux.x	= (INT32)((float_32)(x+0.5f));/*lq*/
	aux.y	= (INT32)((float_32)(y+0.5f));/*lq*/
	
	return aux;
}
#if 1
POINT_FLOAT_TP hor_f2(U08 degree, POINT32_TP *point, double_64 t)//hornbez
{
	SINT32			i		= 0;
	SINT32			choose_i= 0;
	double_64		fact	= 0;
	double_64		t1 		= 0;
	double_64		x		= 0;
	double_64		y		= 0;
	POINT_FLOAT_TP	aux		= {0,0};
	
	t1			= 1.0 - t;
	fact		= 1.0;
	choose_i	= 1;
	x			= (double_64)point[0].x*t1;
	y			= (double_64)point[0].y*t1;
	for(i = 1;i < degree;i++)
	{
		fact		= fact*t;
		choose_i	= choose_i*(degree-i+1)/i;
		x			= (x+fact*choose_i*point[i].x)*t1;
		y			= (y+fact*choose_i*point[i].y)*t1;
	}
	x	= x + fact *t * (double_64)point[degree].x;
	y	= y + fact *t * (double_64)point[degree].y;
	
	aux.x	= (float_32)(x);
	aux.y	= (float_32)(y);
	
	return aux;
}
#endif
POINT_FLOAT_TP hor_f_point_f(U08 degree, POINT_FLOAT_TP *point, double_64 t)//hornbez
{
	SINT32			i		= 0;
	SINT32			choose_i= 0;
	double_64		fact	= 0;
	double_64		t1 		= 0;
	double_64		x		= 0;
	double_64		y		= 0;
	POINT_FLOAT_TP	aux		= {0,0};

	if(point != NULL)	/*lq*/
	{
		t1			= 1.0 - t;
		fact		= 1.0;
		choose_i	= 1;
		x			= (double_64)point[0].x*t1;
		y			= (double_64)point[0].y*t1;
		for(i = 1;i < degree;i++)
		{
			fact		= fact*t;
			choose_i	= choose_i*(degree-i+1)/i;
			x			= (x+fact*choose_i*point[i].x)*t1;
			y			= (y+fact*choose_i*point[i].y)*t1;
		}
		x	= x + fact *t * (double_64)point[degree].x;
		y	= y + fact *t * (double_64)point[degree].y;
		
		aux.x	= (float_32)(x);
		aux.y	= (float_32)(y);
	}
	
	return aux;
}

#endif
