/*****************************************************************************

File Name        :  hmi_action_engine.h
Organization    :  Zhuli Electronics Co.Ltd in Shanghai (www.shzldz.com)
******************************************************************************/


#ifndef _HMI_ACTION_ENGINE_H
#define _HMI_ACTION_ENGINE_H

#define HMI_ACTION_RUN				0
#define HMI_ACTION_CONTINUE			1
#define HMI_ACTION_PAUSE				2
#define HMI_ACTION_FAST_FINISH			3

#define HMI_SEND_EVENT_OFF			0
#define HMI_SEND_EVENT_ON			1

#define HMI_ONE_TIME_DELTA			10
#define HMI_MAX_TIME				0xffffffff		
#define	HMI_LINE_PATH_CNT			2
/*Bezier segment number*/
#define HMI_BEZIER_STEP		  		38
#define HMI_BEZIER_POINT_CNT		3
#define HMI_MAX_BEZIER_POINT		255
#define HMI_ONE_BEZIER_POINT_CNT	4
#define HMI_MAX_ALPHA_VALUE			255U

/*Point */
#define HMI_DISTANCE					1
#define HMI_DISTANCE_SEQ				(HMI_DISTANCE*HMI_DISTANCE)
#define HMI_AV_SPEED					1

#define HMI_FLOAT_DISTANCE				0.0001
#define HMI_FLOAT_DISTANCE_SEQ			0.0000001

#define HMI_MUL_POINT(k,point)			point.x*=k;\
										point.y*=k
										
#define HMI_MUL_POINT_f2i(k,point)		point.x=(INT32)(point.x*k);\
										point.y=(INT32)(point.y*k)

#define HMI_MUL_POINT_f2i16(k,point)	point.x=(INT16)(point.x*k);\
										point.y=(INT16)(point.y*k)


#define HMI_MUL_3D_POINT_f2i(k,point)		point.x=(INT32)(point.x*k);\
											point.y=(INT32)(point.y*k);\
											point.z=(INT32)(point.z*k)

										
#define  HMI_MUL_POINT_F(k,point)		point.x=point.x*k+0.5;\
										point.y=point.y*k+0.5
										
#define  HMI_MUL_POINT_FF(k,point)		point.x=(float_32)(point.x*k);\
												point.y=(float_32)(point.y*k)

#define  HMI_MUL_3D_POINT_F(k,point)	point.x=point.x*k+0.5;\
										point.y=point.y*k+0.5;\
										point.z=point.z*k+0.5


#define  HMI_MUL_POINT2(k,point,mul)	mul.x=point.x*k;\
										mul.y=point.y*k	

#define  HMI_MUL_3D_POINT2(k,point,mul)	mul.x=point.x*k;\
										mul.y=point.y*k;\
										mul.z=point.z*k

#define  HMI_NEGATE_3D_POINT2(point,negate)	negate.x= -point.x;\
											negate.y= -point.y;\
											negate.z= -point.z


#define  HMI_MUL_POINT2_F(k,point,mul)	mul.x=point.x*k+0.5;\
										mul.y=point.y*k+0.5										

#define  HMI_MUL_3D_POINT2_F(k,point,mul)	mul.x=point.x*k+0.5;\
											mul.y=point.y*k+0.5;\
											mul.z=point.z*k+0.5


#define HMI_DIV_POINT(k,point)			point.x/=k;\
										point.y/=k
#define HMI_DIV_3D_POINT(k,point)			point.x/=k;\
											point.y/=k;\
											point.z/=k

#define HMI_DIV_POINT3(k,point,sum)		sum.x=point.x/k;\
										sum.y=point.y/k

#define HMI_DIV_3D_POINT3(k,point,sum)		sum.x=point.x/k;\
											sum.y=point.y/k;\
											sum.z=point.z/k

#define HMI_DIV_POINT_F(k,point)		point.x=point.x/k+0.5;\
										point.y=point.y/k+0.5

#define HMI_DIV_3D_POINT_F(k,point)		point.x=point.x/k+0.5;\
										point.y=point.y/k+0.5;\
										point.z=point.z/k+0.5


#define HMI_DIV_POINT2(k,point,sum)		sum.x=point.x/k;\
										sum.y=point.y/k
#define HMI_DIV_3D_POINT2(k,point,sum)		sum.x=point.x/k;\
											sum.y=point.y/k;\
											sum.z=point.z/k;

									
#define HMI_DIV_POINT2_F(k,point,sum)	sum.x=point.x/k+0.5; \
										sum.y=point.y/k+0.5


#define HMI_DIV_3D_POINT2_F(k,point,sum)	sum.x=point.x/k+0.5;\
											sum.y=point.y/k+0.5;\
											sum.z=point.z/k+0.5


#define HMI_ADD_POINT(p1,p2)			p1.x+=p2.x;\
										p1.y+=p2.y

#define HMI_ADD_3D_POINT(p1,p2)			p1.x+=p2.x;\
										p1.y+=p2.y;\
										p1.z+=p2.z


#define HMI_ADD_POINT2(p1,p2,sum)		sum.x=p1.x+p2.x;\
										sum.y=p1.y+p2.y		


#define HMI_ADD_POINT2_IFF(p1,p2,sum)	sum.x=(float_32)(p1.x+p2.x);\
											sum.y=(float_32)(p1.y+p2.y)	
#define HMI_ADD_3D_POINT2(p1,p2,sum)		sum.x=p1.x+p2.x;\
											sum.y=p1.y+p2.y;\
											sum.z=p1.z+p2.z

										
#define HMI_ADD_POINT2_F(p1,p2,sum)		sum.x=p1.x+p2.x+0.5;\
										sum.y=p1.y+p2.y+0.5 

#define HMI_ADD_3D_POINT2_F(p1,p2,sum)	sum.x=p1.x+p2.x+0.5;\
										sum.y=p1.y+p2.y+0.5;\
										sum.z=p1.z+p2.z+0.5

										
#define HMI_ADD_POINT2_F2I(p1,p2,sum)	sum.x=(INT32)(p1.x+p2.x+0.5);\
										sum.y=(INT32)(p1.y+p2.y+0.5) 

#define HMI_ADD_3D_POINT2_F2I(p1,p2,sum)	sum.x=(INT32)(p1.x+p2.x+0.5);\
											sum.y=(INT32)(p1.y+p2.y+0.5);\
											sum.z=(INT32)(p1.z+p2.z+0.5)

#define HMI_POINT_F(p1)					p1.x+=0.5;\
										p1.y+=0.5

#define HMI_3D_POINT_F(p1)				p1.x+=0.5;\
										p1.y+=0.5;\
										p1.z+=0.5


#define HMI_SUB_POINT(p1,p2)			p1.x-=p2.x;\
										p1.y-=p2.y		

#define HMI_SUB_3D_POINT(p1,p2)			p1.x-=p2.x;\
										p1.y-=p2.y;\
										p1.z-=p2.z


#define HMI_SUB_POINT2(p1,p2,sub)		sub.x=p1.x-p2.x;\
										sub.y=p1.y-p2.y		

#define HMI_SUB_3D_POINT2(p1,p2,sub)	sub.x=p1.x-p2.x;\
										sub.y=p1.y-p2.y;\
										sub.z=p1.z-p2.z


#define HMI_SUB_POINT_IIF(p1,p2,sub)	sub.x=(float_32)(p1.x-p2.x);\
										sub.y=(float_32)(p1.y-p2.y)	

#define HMI_SUB_3D_POINT_IIF(p1,p2,sub)	sub.x=(float_32)(p1.x-p2.x);\
										sub.y=(float_32)(p1.y-p2.y);\
										sub.z=(float_32)(p1.z-p2.z)

#define HMI_CPY_POINT(p1,p2)			p1.x=p2.x;\
										p1.y=p2.y		

#define HMI_CPY_3D_POINT(p1,p2)			p1.x=p2.x;\
										p1.y=p2.y;\
										p1.z=p2.z

#define HMI_CPY_3D_POINT_I2F(p1,p2)		p1.x=(float_32)(p2.x);\
										p1.y=(float_32)(p2.y);\
										p1.z=(float_32)(p2.z)


#define HMI_DIS_SEQ(p1,k)				k=p1.x*p1.x+p1.y*p1.y
#define HMI_DIS_SEQ1(p1)				(p1.x*p1.x+p1.y*p1.y)

#define HMI_DIS_3D_SEQ(p1,k)			k=p1.x*p1.x+p1.y*p1.y+p1.z*p1.z


#define HMI_DIS_SEQ3(p1,p2,k)			k=(p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y)	
#define HMI_DIS_SEQ2(p1,p2)				((p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y))	

#define HMI_DIS_3D_SEQ2(p1,p2,k)		k=(p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y)+(p1.z-p2.z)*(p1.z-p2.z)	

/*
(x1,y1,z1)X(x2,y2,z2)=(y1z2-y2z1, z1x2-z2y1, x1y2-x2y1) 

*/
#define HMI_CROSS_MUL(p1,p2,sum)		sum.x= p1.y*p2.z - p2.y*p1.z;\
										sum.y= p1.z*p2.x - p2.z*p1.y;\
										sum.z= p1.x*p2.y - p2.x*p1.y

#define HMI_CROSS_MUL_FF(p1,p2,sum)	sum.x= (float_32)(p1.y*p2.z - p2.y*p1.z);\
										sum.y= (float_32)(p1.z*p2.x - p2.z*p1.y);\
										sum.z= (float_32)(p1.x*p2.y - p2.x*p1.y)



#define HMI_CROSS_MUL_Z(p1,p2)			(p1.x*p2.y - p2.x*p1.y)
#define HMI_CROSS_MUL2(p1,p2)			(p1.x * p2.y - p1.y * p2.x)


#define HMI_CROSS_MUL_3D(p1,p2,sum)		sum.x= p1.y*p2.z - p2.y*p1.z;\
										sum.y= p1.z*p2.x - p2.z*p1.y;\
										sum.z= p1.x*p2.y - p2.x*p1.y
#define HMI_DOT_MUL_XY(p1,p2)			(p1.x * p2.x + p1.y * p2.y)
#define HMI_DOT_MUL_F(p1,p2,sum)		sum	= (float_32)(p1.x * p2.x + p1.y * p2.y)
#define HMI_DOT_MUL(p1,p2,sum)			sum	= p1.x * p2.x + p1.y * p2.y
#define HMI_DOT_MUL2(p1,p2)				(p1.x * p2.x + p1.y * p2.y)

#define HMI_DOT_MUL_3D(p1,p2,sum)		sum	= p1.x * p2.x + p1.y * p2.y + p1.z * p2.z


#define HMI_DOT_MUL4(p1,p2,p3,p4)		((p2.x - p1.x)*(p4.y - p3.y) - (p2.y - p1.y)*(p4.x - p3.x))	
#define HMI_GET_AREA(p1,p2,p3)			HMI_DOT_MUL4(p1,p2,p1,p3)


typedef enum
{
	HMI_NO_ANIMATION,
	HMI_LINEAR_ANIMATION,
	HMI_FAST_SLOW_ANIMATION,
	HMI_SLOW_FAST_ANIMATION,
	HMI_S_F_S_ANIMATION,/*slow to fast ,then to slow*/
	HMI_F_S_F_ANIMATION,/*fast to slow   ,then to fast*/
	/*****motion end*****/
	HMI_NB_MOTION
}MOTION_TYPE_STR;


typedef enum 
{
	HMI_EVENT_RUN=0,
	HMI_PROGRAM_CTRL,
	HMI_BUTTON_PRESS,
	HMI_BUTTON_UP,
	HMI_BUTTON_LONG_PRESS,
	HMI_BUTTON_REPEAT,
	HMI_PAGE_ON,
	HMI_PAGE_OFF
}HMI_STAND_EVENT_TYPE;

typedef enum
{
	HMI_NEXT_POS,
	HMI_START_POS,
	HMI_END_POS,
	/*end*/
	HMI_ANIM_POS_CNT
}HMI_ANIM_POS_TYPE;

#if 0
typedef struct
{
	INT32 x;
	INT32 y;
}POINT32_TP;
#endif

typedef struct
{
	INT32 x;
	INT32 y;
	INT32 z;
}POINT32_3D_TP;



typedef struct
{
	float_32	x;
	float_32	y;
	float_32	z;
}POINT_FLOAT_3D_TP;


typedef struct
{
	void (*P_func)(void);
}HMI_SET_CALL_FUNC_STR;

typedef struct
{
	HMI_OBJECT_ID_STR event_id;
}HMI_SET_SEND_EVENT_STR;
#if 0
typedef struct
{
	UINT8 status;
	HMI_OBJECT_ID_STR button_id;
}HMI_SET_BTN_STATUS_STR;
#endif
typedef struct
{
	HMI_COLOR_STR   	color;
	HMI_OBJECT_ID_STR 	object_id_color;
}HMI_SET_COLOR_STR;


typedef struct
{
	SINT8 step;
	HMI_OBJECT_ID_STR 	object_id_scroll;
}HMI_SET_TEXT_SCROLL_STEP_STR;

typedef struct
{
	HMI_CHAR_STR const	*changed_string;
	HMI_OBJECT_ID_STR	object_id;
}HMI_SET_EDIT_TEXT_STR;

typedef struct
{
	BOOLEAN				page_on_off;
	HMI_OBJECT_ID_STR	object_id;
}HMI_SET_PAGE_ON_OFF_STR;

typedef struct
{
	HMI_OBJECT_ID_STR child;
	HMI_OBJECT_ID_STR object_id;
}HMI_SET_DYN_CONTAINER_STR;

typedef struct
{
	SINT8				delta_w;
	SINT8				delta_h;
	HMI_OBJECT_ID_STR	object_id_w;
	HMI_OBJECT_ID_STR	object_id_h;
}HMI_SET_DELTA_W_H_STR;

typedef UINT8			ALPHA_TYPE;
typedef struct
{
	ALPHA_TYPE			alpha;
	HMI_OBJECT_ID_STR	object_id_angel;
}HMI_SET_ALPHA_STR;
typedef struct
{
	#if 0
	ALPHA_TYPE			start_alpha;
	ALPHA_TYPE			end_alpha;
	#endif
	U16					line_sum;
	U08					point_cnt;		
	POINT32_TP			*pPoint_list;
}HMI_ANIM_ALPHA_STR;


typedef float_32		ANGEL_TYPE;
typedef struct
{
	ANGEL_TYPE			angel;
	HMI_OBJECT_ID_STR	object_id_angel;
}HMI_SET_ANGEL_STR;
typedef struct
{
	#if 0
	ANGEL_TYPE			start_angel;
	ANGEL_TYPE			end_angel;
	#endif
	U16					line_sum;
	U08					point_cnt;		
	//POINT32_TP			*pPoint_list;lq
	POINT32_TP			*pPoint_list;
}HMI_ANIM_ANGEL_STR;

typedef struct
{
#if 0 
	ANGEL_TYPE   start_angel; 
	ANGEL_TYPE   end_angel; 
#endif 
	U16     line_sum; 
	U08     point_cnt;   //POINT32_TP   *pPoint_list;lq 
	POINT_FLOAT_TP/*POINT32_TP 2019 11 30 lq*/   *pPoint_list;
}HMI_ANIM_SCALE_STR;

typedef float_32  SCALE_TYPE;

typedef struct
{ 
	SCALE_TYPE   scale; 
	HMI_OBJECT_ID_STR object_id_scale;
}HMI_SET_SCALE_STR;

typedef struct
{
	UINT16				width;
	UINT16				height;	
	HMI_OBJECT_ID_STR	object_id_w;
	HMI_OBJECT_ID_STR	object_id_h;
}HMI_SET_W_H_STR;


typedef struct
{
	UINT16				width;
	UINT16				height;	
}HMI_W_H_STR;
typedef struct
{
	#if 0
	UINT16				start_w;
	UINT16				start_h;
	UINT16				end_w;
	UINT16				end_h;
	#endif
	U16					line_sum;
	U08					point_cnt;	
	POINT32_TP			*pPoint_list;
}HMI_ANIM_W_H_STR;


typedef struct
{
	SINT8				delta_x;
	SINT8				delta_y;
	HMI_OBJECT_ID_STR	object_id_x;
	HMI_OBJECT_ID_STR	object_id_y;
}HMI_SET_DELTA_POS_STR;


typedef struct
{
	SINT16				x;
	SINT16				y;
	HMI_OBJECT_ID_STR	object_id_x;
	HMI_OBJECT_ID_STR	object_id_y;
}HMI_SET_POS_STR;

typedef struct
{
	#if 0/*for support Bezier path*/
	SINT16				start_x;
	SINT16				start_y;
	SINT16				end_x;
	SINT16				end_y;
	#endif
	U16					line_sum;
	U08					point_cnt;	
	POINT32_TP			*pPoint_list;
}HMI_ANIM_POS_STR;


typedef struct
{	
	POINT32_TP	lineStart;
	POINT32_TP	lineEnd;
	HMI_TIME	newDur;
	HMI_TIME	newElapse;
	//HMI_TIME	duration;	
}HMI_BEZIER_NEW_POS_STR;


typedef struct
{
	HMI_RANGE_STR		index;
	HMI_OBJECT_ID_STR	object_id;
}HMI_SET_RANGE_STR;




typedef struct
{
	#if 0
	HMI_RANGE_STR		start_range;
	HMI_RANGE_STR		end_range;
	#endif
	U16					line_sum;
	U08					point_cnt;		
	POINT32_TP			*pPoint_list;
}HMI_ANIM_RANGE_STR;

typedef struct
{
	#if 0
	HMI_COLOR_STR		start_color;
	HMI_COLOR_STR		end_color;
	#endif
	U16					line_sum;
	U08					point_cnt;		
	POINT32_TP			*pPoint_list;
}HMI_ANIM_COLOR_STR;

typedef struct
{
	#if 0
	HMI_COLOR_STR		start_color;
	HMI_COLOR_STR		end_color;
	#endif
	U16					line_sum;
	U08					point_cnt;		
	POINT32_TP			*pPoint_list;
}HMI_ANIM_P_STR;


typedef struct
{
	U16 				p_value;
	HMI_OBJECT_ID_STR	object_id;
}HMI_ANIM_CUSTOM_P_STR;

typedef struct
{
	HMI_SET_POS_STR 				const *P_set_pos_prop_table;
	HMI_SET_DELTA_POS_STR 			const *P_set_delta_pos_prop_table;
	HMI_SET_W_H_STR 				const *P_set_w_h_prop_table;
	HMI_SET_DELTA_W_H_STR 			const *P_set_delta_w_h_prop_table;
	HMI_SET_DYN_CONTAINER_STR 		const *P_set_dyn_container_prop_table;
	HMI_SET_PAGE_ON_OFF_STR 		const *P_set_page_on_off_prop_table;
	HMI_SET_EDIT_TEXT_STR  			const *P_set_edit_text_prop_table;
	HMI_SET_TEXT_SCROLL_STEP_STR 	const *P_set_text_scroll_step_prop_table;
	HMI_SET_COLOR_STR				const *P_set_for_color_prop_table;
	HMI_SET_COLOR_STR				const *P_set_bck_color_prop_table;
	/*HMI_SET_BTN_STATUS_STR			const *P_set_btn_status_prop_table;*/
	HMI_SET_SEND_EVENT_STR			const *P_set_send_event_prop_table;
	HMI_SET_CALL_FUNC_STR			const *P_set_call_func_prop_table;
	HMI_SET_RANGE_STR				const *P_set_imagelist_index_prop_table;
	HMI_SET_RANGE_STR				const *P_set_scrollbar_range_prop_table;
	HMI_SET_RANGE_STR				const *P_set_button_status_prop_table;
	HMI_ANIM_P_STR					const *P_set_anim_p1_prop_table;
	HMI_ANIM_P_STR					const *P_set_anim_p2_prop_table;
	HMI_ANIM_P_STR					const *P_set_anim_p3_prop_table;
}HMI_SET_ACTION_PROP_STR;


typedef struct
{
	HMI_TIME /*const*/	start;
	HMI_TIME		duration;

}HMI_DURATION_STATIC_PROP_STR;		

typedef struct
{
	HMI_TIME const	*P_start;
	HMI_TIME		*P_elapse;
}HMI_REPEAT_ONETIME_PROP_STR;

typedef struct
{
	HMI_DURATION_STATIC_PROP_STR const	*P_start_duration;
	HMI_TIME							*P_elapse;
}HMI_DURATION_PROP_STR;

typedef struct
{
	void const						*P_object_prop;
	HMI_OBJECT_TABLE_STR const		*P_object_relative;
}HMI_EVENT_ACTION_RES_STR;


typedef struct
{
	HMI_EVENT_ACTION_RES_STR const	*P_repeat;
	HMI_EVENT_ACTION_RES_STR const	*P_onetime;
	HMI_EVENT_ACTION_RES_STR const	*P_duration;
	HMI_EVENT_ACTION_RES_STR const	*P_dyn_duration;
}HMI_TIMER_ACTION_STR;


typedef struct
{
	HMI_TIMER_ACTION_STR const		*P_timer_action_table;
	HMI_EVENT_ACTION_RES_STR const	*P_set_action_table;
	
}HMI_ACTION_STR;



typedef struct
{
	HMI_OBJECT_ID_STR element_id;
}HMI_STAND_EVENT_PROP_STR;

typedef struct
{
	HMI_EVENT_ACTION_RES_STR const	*P_stand_event;
	HMI_EVENT_ACTION_RES_STR const	*P_custom_event;	
}HMI_EVENT_STR;

typedef struct
{
	HMI_EVENT_STR const				*P_event;
	HMI_EVENT_ACTION_RES_STR  const	*P_action_group;
	HMI_ACTION_STR   const			*P_action;
}HMI_EVENT_ACTION_STR;


void hmi_action_manager_init(void);
void hmi_action_manager(HMI_TIME dt);
#if HMI_EVENT_STAND_NUMBER>0
void hmi_do_event(HMI_OBJECT_ID_STR element_id,UINT8 press_status);
#endif
#if HMI_ACTION_GROUP_NUMBER>0
void  hmi_set_action_status(HMI_OBJECT_ID_STR action_gp_id,HMI_OBJECT_DATA_STR status);
#endif

#if HMI_ALL_EVENT_NUMBER>0
void hmi_action_send_event(HMI_OBJECT_ID_STR event_id);
#endif

#if HMI_DYN_TIMER_ACTION_DURATION_NUMBER>0
void hmi_set_timer_s_e(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data,SET_TIMER_ENUM timer_prop);
#endif

/*Set dyn pos start and end*/
#if HMI_ANIM_DYN_SET_POS_NUMBER>0
void hmi_set_dyn_pos_s_e(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data,BOOLEAN start);
HMI_OBJECT_DATA_STR hmi_get_dyn_pos_s_e(HMI_OBJECT_ID_STR hmi_object_id,BOOLEAN start);
#endif
/*Set dyn width heigh start and end*/
#if HMI_ANIM_DYN_SET_W_H_NUMBER>0
void hmi_set_dyn_wh_s_e(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data,BOOLEAN start);
HMI_OBJECT_DATA_STR hmi_get_dyn_wh_s_e(HMI_OBJECT_ID_STR hmi_object_id,BOOLEAN start);
#endif
/*Set dyn fcolor start and end*/
#if HMI_ANIM_DYN_SET_FOR_COLOR_NUMBER>0
void hmi_set_dyn_fcolor_s_e(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data,BOOLEAN start);
HMI_OBJECT_DATA_STR hmi_get_dyn_fcolor_s_e(HMI_OBJECT_ID_STR hmi_object_id,BOOLEAN start);
#endif
/*Set dyn image list start and end*/
#if HMI_ANIM_DYN_SET_IMAGELIST_INDEX_NUMBER>0
void hmi_set_dyn_imglist_s_e(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data,BOOLEAN start);
HMI_RANGE_STR hmi_get_dyn_imglist_s_e(HMI_OBJECT_ID_STR hmi_object_id,BOOLEAN start);
#endif
/*Set dyn alpha start and end*/
#if HMI_ANIM_DYN_SET_ALPHA_NUMBER>0
void hmi_set_dyn_alpha_s_e(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data,BOOLEAN start);
ALPHA_TYPE hmi_get_dyn_alpha_s_e(HMI_OBJECT_ID_STR hmi_object_id,BOOLEAN start);
#endif
/*Set dyn angel start and end*/
#if HMI_ANIM_DYN_SET_ANGEL_NUMBER>0
void hmi_set_dyn_angel_s_e(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data,BOOLEAN start);
ANGEL_TYPE hmi_get_dyn_angel_s_e(HMI_OBJECT_ID_STR hmi_object_id,BOOLEAN start);
#endif

#if HMI_DYN_TIMER_ACTION_DURATION_NUMBER>0
void hmi_set_timer_s_e(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data,SET_TIMER_ENUM timer_prop );
HMI_TIME hmi_get_timer_s_e(HMI_OBJECT_ID_STR hmi_object_id,SET_TIMER_ENUM timer_prop );
#endif

#if ((HMI_ANIM_DYN_SET_POS_NUMBER>0) ||(HMI_ANIM_STATIC_SET_POS_NUMBER>0))
HMI_OBJECT_ID_STR hmi_engine_get_act_set_execute_id(HMI_OBJECT_ID_STR hmi_object_id);
HMI_ANIM_POS_STR CONST * hmi_engine_get_act_set_bezier(HMI_OBJECT_ID_STR hmi_object_id);
#endif
POINT32_TP hor(U08 degree, POINT32_TP CONST *point, double_64 t);//hornbez
POINT32_TP hor_f(U08 degree, POINT_FLOAT_TP CONST *point, double_64 t);//hornbez
POINT_FLOAT_TP hor_f2(U08 degree, POINT32_TP *point, double_64 t);//hornbez
POINT_FLOAT_TP hor_f_point_f(U08 degree, POINT_FLOAT_TP *point, double_64 t);//hornbez
#if HMI_ANIM_DYN_SET_SCALE_NUMBER>0
#ifndef HMI_GRAPHIC_TWLIB
#ifndef HMI_GRAPHIC_ST7513
void hmi_set_dyn_scale_s_e(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data,BOOLEAN start);
#endif
#endif
#endif

#endif

