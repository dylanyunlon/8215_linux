/*****************************************************************************

File Name        :  hmi_engine.c
Organization    :  Zhuli Electronics Co.Ltd in Shanghai (www.shzldz.com)
******************************************************************************/

#define HMI_ENGINE_C
#include "hmi_all_struct_include.h"
#include "HMI_Data/hmi_res_sprite.rom"
#include "HMI_Data/hmi_application_data.rom"
#include <math.h>        /* sin and cos */

//#include  "hmi_utils.rom"

#if defined(HMI_GRAPHIC_RGL) ||defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(S6J3200_GRAPHIC)
/*set layer draw first,if first draw the layer,clear the layer*/
#if 0
static	UINT8	first_draw_layer = 0xff;
#define	set_first_draw_all()		first_draw_layer = 0xff
#define set_first_draw(layer)		first_draw_layer = first_draw_layer|(UINT8(1<<layer))
#define clr_first_draw(layer)		first_draw_layer = first_draw_layer&(UINT8(~(1<<layer)))
#endif
#endif

#if		HMI_IMGLIST_MAX_STATUS<=2
	#define HMI_IMGLIST_MAX_STATUS_BIT_LEN	1
#elif	HMI_IMGLIST_MAX_STATUS<=4
	#define HMI_IMGLIST_MAX_STATUS_BIT_LEN	2
#elif	HMI_IMGLIST_MAX_STATUS<=16
	#define HMI_IMGLIST_MAX_STATUS_BIT_LEN	4
#else
	#define HMI_IMGLIST_MAX_STATUS_BIT_LEN	8
#endif

#if		HMI_BTN_MAX_STATUS<=2
	#define HMI_BTN_MAX_STATUS_BIT_LEN	1
#elif	HMI_BTN_MAX_STATUS<=4
	#define HMI_BTN_MAX_STATUS_BIT_LEN	2
#elif	HMI_BTN_MAX_STATUS<=16
	#define HMI_BTN_MAX_STATUS_BIT_LEN	4
#else
	#define HMI_BTN_MAX_STATUS_BIT_LEN	8
#endif

#if		((HMI_SCROLLBAR_MAX_STATUS+1)<=2)
	#define HMI_SCROLLBAR_MAX_STATUS_BIT_LEN	1
#elif	((HMI_SCROLLBAR_MAX_STATUS+1)<=4)
	#define HMI_SCROLLBAR_MAX_STATUS_BIT_LEN	2
#elif	((HMI_SCROLLBAR_MAX_STATUS+1)<=16)
	#define HMI_SCROLLBAR_MAX_STATUS_BIT_LEN	4
#elif	((HMI_SCROLLBAR_MAX_STATUS+1)<=256)
	#define HMI_SCROLLBAR_MAX_STATUS_BIT_LEN	8
#else
	#error ScrollBar 'MaxRange' must <=255
#endif

#define	HMI_TW_CONTAINER_COMPRESS_MAX_STATUS_BIT_LEN	1


#define HMI_RENDER_ALL_EXCEPT_BCK	NO		//-cxy
#define HMI_TEXT_COLOR_PALLETE_LEN	0		//-cxy
#define	HMI_DXY_GFILL_MAX_SON_CNT	NO		//-cxy

#define	 PI_HMI	  3.14159265358979323846

#define			HMI_CUBLIC_BEZIER_CTL_CNT		4
#define			HMI_HALF_CIRCLE_CTL_CNT			4
#define			HMI_EXPAND_HALF_CIRCLE_CTL_CNT	(HMI_HALF_CIRCLE_CTL_CNT + HMI_HALF_CIRCLE_CTL_CNT)
#define 		HMI_EXPAND_PATH					40
#define			HMI_CANE_SOLID_PATH_MAX_LEN		(30/*10*/ * HMI_CUBLIC_BEZIER_CTL_CNT)
#define 		HMI_LANE_SOLID_LINE_LEN			100
#define HMI_LANE_LINE_MAX_NB	2
#define	HMI_LANE_LINE_MIN_POINT_NB		8u
#define	HMI_LANE_LINE_POINT_NB			16u//8u
#define	HMI_ROAD_BEZIER_CTRL_CNT		(HMI_LANE_LINE_POINT_NB )
#define HMI_LANE_SLIOD_ZONE_LEN			14


HMI_OBJECT_ID_STR hmi_load_object_index = 0;

/*For MACROS too nested error*/
CONST HMI_OBJECT_ID_STR hmi_event_act_begin_index=(HMI_CONTAINERS_SXY_MAX_ID+HMI_LANGUAGE_NUMBER);
#if(HMI_LOAD_SOURCE_MODE ==HMI_LOAD_RES_SHEET)
#define HMI_LOAD_LIST_MAX_NB	(HMI_DXY_IMAGELIST_NUMBER+\
								HMI_SXY_IMAGELIST_NUMBER+\
								HMI_DXY_SCROLLBAR_NUMBER+\
								HMI_SXY_SCROLLBAR_NUMBER+\
								HMI_DXY_BUTTON_NUMBER+\
								HMI_SXY_BUTTON_NUMBER+\
								HMI_DXY_BITMAPS_NUMBER+\
								HMI_SXY_BITMAPS_NUMBER)

HMI_OBJECT_ID_STR	hmi_load_id_list[HMI_LOAD_LIST_MAX_NB]	= {0};
POINT_FIFO_TP		hmi_load_fifo ={0};
#endif

#if HMI_DYN_LANGUAGE_NUMBER>0
static HMI_OBJECT_ID_STR hmi_cur_language=0U;
#endif
#if HMI_ALL_DYN_OBJECTS_NUMBER > 0
#if	(defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))/*two buffer update*/
#define HMI_NB_ALL_DYN_OBJECT_BYTE					((HMI_ALL_DYN_OBJECTS_NUMBER+3)/4)
static UINT8 hmi_dynamic_object_changed_flag[HMI_NB_ALL_DYN_OBJECT_BYTE];

#define HMI_UPDATE_LEN								2u
#define FLAG_BIT_POS(flag_id)    					(flag_id &0x03)

#define HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(id)		(hmi_dynamic_object_changed_flag[((id)>>2u)]=(hmi_dynamic_object_changed_flag[((id)>>2u)]&( ~(UINT8)(0x03u<<((FLAG_BIT_POS(id))*HMI_UPDATE_LEN))))|((UINT8)(0x02u<<((FLAG_BIT_POS(id))*HMI_UPDATE_LEN))))
#define HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(id)		((hmi_dynamic_object_changed_flag[((id)>>2u)]>>((FLAG_BIT_POS(id))*HMI_UPDATE_LEN))&0x03u)
#define HMI_DYNAMIC_OBJECT_CANCEL_CHANGED_FLAG(id)	(hmi_dynamic_object_changed_flag[((id)>>2u)]=hmi_dynamic_object_changed_flag[((id)>>2u)]-((UINT8)((0x01u<<((FLAG_BIT_POS(id))*HMI_UPDATE_LEN)))))
#else
#define HMI_NB_ALL_DYN_OBJECT_BYTE					((HMI_ALL_DYN_OBJECTS_NUMBER+7)/8)
static UINT8 hmi_dynamic_object_changed_flag[HMI_NB_ALL_DYN_OBJECT_BYTE];


#define HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(id)		(hmi_dynamic_object_changed_flag[((id)>>3u)]=hmi_dynamic_object_changed_flag[((id)>>3u)]|( (UINT8)(0x01u<<((UINT8)((id)&0x07u)))))
#define HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(id)		(hmi_dynamic_object_changed_flag[((id)>>3u)]&((UINT8)(0x01u<<((UINT8)((id)&0x07u)))))
#define HMI_DYNAMIC_OBJECT_CANCEL_CHANGED_FLAG(id)	(hmi_dynamic_object_changed_flag[((id)>>3u)]=hmi_dynamic_object_changed_flag[((id)>>3u)]&((UINT8)(~(0x01u<<((UINT8)((id)&0x07u))))))
#endif
#endif
UINT8 hmi_driver_woking_status_flag = 0U;

#if HMI_ALL_LAYERS_NUMBER > 0
#if (HMI_LAYER_0_HIGHEST_PRIORITY > 1) ||(HMI_LAYER_1_HIGHEST_PRIORITY > 1)
static UINT8 hmi_highest_active_page_priority[HMI_ALL_LAYERS_NUMBER]	= 0;
#endif
#if HMI_LAYER_0_HIGHEST_PRIORITY>0
 static HMI_PRIOR_PAGE_STR DISPLAY_SEGMENT_RAM hmi_layer_0_active_page_id[HMI_LAYER_0_HIGHEST_PRIORITY];
#endif
#if HMI_ALL_LAYERS_NUMBER > 1
 static HMI_PRIOR_PAGE_STR DISPLAY_SEGMENT_RAM hmi_layer_1_active_page_id[HMI_LAYER_1_HIGHEST_PRIORITY]; 
#endif
#endif
HMI_LIGHT_POS_TYPE_STR	hmi_light_status;

/***element draw rect backups****/
#if (defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC))
#if	HMI_DXY_IMAGELIST_NUMBER	> 0
static HMI_RECT_STR  hmi_dxy_imagelist_rect_bck[HMI_DXY_IMAGELIST_NUMBER]=
{
	0//{HMI_INVALID_COOR,HMI_INVALID_COOR,0,0}
};
#endif

#if HMI_DXY_SCROLLBAR_NUMBER>0
/*Static scrollbar dyamic coordinate   */
 static HMI_RECT_STR hmi_dxy_scrollbar_rect_bck[HMI_DXY_SCROLLBAR_NUMBER]=
{
	0//{HMI_INVALID_COOR,HMI_INVALID_COOR,0,0}
};
#endif

#if HMI_DXY_PAGES_NUMBER>0
/*Dynamic xy page coordinate  */
 static HMI_RECT_STR  hmi_dxy_page_rect_bck[HMI_DXY_PAGES_NUMBER]=
{
	0//{HMI_INVALID_COOR,HMI_INVALID_COOR,0,0}
};
#endif

#if HMI_DXY_BUTTON_NUMBER>0
/*Static Button dyamic coordinate   */
 static HMI_RECT_STR hmi_dxy_button_rect_bck[HMI_DXY_BUTTON_NUMBER]=
{
	0//{HMI_INVALID_COOR,HMI_INVALID_COOR,0,0}
};
#endif
#if HMI_DYN_XY_EDIT_TEXTS_NUMBER>0
/*Editable text dynamic property   */
 static HMI_RECT_STR  hmi_dyn_xy_edit_text_prop_table_bck[HMI_DYN_XY_EDIT_TEXTS_NUMBER]=
{
	0//{HMI_INVALID_COOR,HMI_INVALID_COOR,0,0}
};
#endif
#if HMI_UNEDIT_TEXTS_DYN_XY_NUMBER>0
 static HMI_RECT_STR  hmi_dyn_xy_unedit_text_prop_table_bck[HMI_UNEDIT_TEXTS_DYN_XY_NUMBER]=
{
	0//{HMI_INVALID_COOR,HMI_INVALID_COOR,0,0}
};
#endif
#if HMI_DYN_CONTAINERS_NUMBER>0
/*Dynamic widget initial   */
 static HMI_DYN_CONTAINER_DATA_STR  hmi_dyn_container_table_bck[HMI_DYN_CONTAINERS_NUMBER]=
{
	0//HMI_NB_ELEMENTS
};
#endif
#if HMI_DYN_FILL_PAGES_NUMBER>0
/*Dynamic Fill coordinate  */
 static HMI_RECT_STR  hmi_fills_dyn_xy_rect_bck[HMI_DYN_FILL_PAGES_NUMBER]=
{
	0//{HMI_INVALID_COOR,HMI_INVALID_COOR,0,0}
};
#endif
#if HMI_DYN_GFILL_NUMBER>0
/*Dynamic Fill coordinate  */
 static HMI_RECT_STR  hmi_gradient_dxy_fill_rect_bck[HMI_DYN_GFILL_NUMBER]=
{
	0//{HMI_INVALID_COOR,HMI_INVALID_COOR,0,0}
};
#endif
#if HMI_DXY_CUBE_NUMBER>0
/*Dynamic Cube coordinate,width,angel,bump,face  */
 static HMI_RECT_STR  hmi_cubes_dyn_xy_rect_bck[HMI_DXY_CUBE_NUMBER]=
{
	0//{HMI_INVALID_COOR,HMI_INVALID_COOR,0,0}
};
#endif
#if HMI_DXY_3DCUBE_NUMBER>0
/*Dynamic Cube coordinate,width,angel,bump,face  */
 static HMI_RECT_STR  hmi_3Dcubes_dyn_xy_rect_bck[HMI_DXY_3DCUBE_NUMBER]=
{
	0//{HMI_INVALID_COOR,HMI_INVALID_COOR,0,0}
};
#endif

#if HMI_DXY_CONTAINERS_NUMBER>0
 static HMI_RECT_STR  hmi_dyn_xy_container_rect_bck[HMI_DXY_CONTAINERS_NUMBER]=
{
	0//{HMI_INVALID_COOR,HMI_INVALID_COOR,0,0}
};
#endif
#if HMI_DXY_BITMAPS_NUMBER>0
 static HMI_RECT_STR  hmi_bmp_dyn_xy_rect_bck[HMI_DXY_BITMAPS_NUMBER]=
{
	0//{HMI_INVALID_COOR,HMI_INVALID_COOR,0,0}
};
#endif
#if HMI_DXY_SPLINE_NUMBER>0
 static HMI_RECT_STR  hmi_spline_dyn_xy_rect_bck[HMI_DXY_SPLINE_NUMBER]=
{
	0//{HMI_INVALID_COOR,HMI_INVALID_COOR,0,0}
};
#endif
#if HMI_DXY_CUSTOM_CNT>0
 static HMI_RECT_STR  hmi_custom_dyn_xy_rect_bck[HMI_DXY_CUSTOM_CNT]=
{
	0//{HMI_INVALID_COOR,HMI_INVALID_COOR,0,0}
};
#endif

#if HMI_DXY_BITMAPS_NUMBER>0
 static float_32  hmi_bmp_dyn_angel_bck[HMI_DXY_BITMAPS_NUMBER]=
{
	0
};
#endif
HMI_INPUT_POLYGON_STR  hmi_spline_input_point={0};
//HMI_INPUT_POLYGON_STR  hmi_spline_expand_input_point={0};

#endif
#if HMI_MAX_SINGLE_STAUS_CNT>0
#if (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642)||defined(HMI_GRAPHIC_RGL)|| \
		defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG))
static HMI_OBJECT_ID_STR hmi_cur_table =0;
#endif
#endif

#if HMI_MAX_SUB_TABLE_CNT>0
 static HMI_OBJECT_ID_STR DISPLAY_SEGMENT_RAM hmi_last_table_status[HMI_MAX_SUB_TABLE_CNT ]={0};
#endif

/*static lq*/ HMI_PUBLIC_BUFFER_STR hmi_public_buffer		= {0,0};
void	hmi_init_qd_buffer_list(void);

#if HMI_ALL_STATIC_CONTAINERS_NUMBER > 0
  static BOOLEAN hmi_engine_check_container_changed(HMI_OBJECT_ID_STR hmi_object_id,HMI_RECT_STR CONST*pfarther_rect,
  							HMI_RECT_STR *pdirty_rect,UINT8 depth,HMI_RECT_STR * pcliped_farther_rect);
 #if (HMI_DYN_CONTAINERS_NUMBER > 0)
   static BOOLEAN hmi_engine_get_static_container_id(HMI_OBJECT_ID_STR * phmi_dyn_contain_id);
 #endif
#endif


#if HMI_ALL_DYN_OBJECTS_NUMBER> HMI_PAGES_NUMBER
static UINT8 hmi_engine_check_dynamic_object_changed(HMI_CONTAINER_STR CONST * phmi_container_info,
																	HMI_RECT_STR CONST *pfarther_rect,								
																	HMI_RECT_STR *pdirty_rect,
																	U08		depth,
																	HMI_RECT_STR *pcliped_farther_rect);
#endif	
/*static 2023 05 02*/ void  hmi_engine_draw_container(HMI_CONTAINER_STR CONST * phmi_container_info,HMI_RECT_STR CONST *pfarther_rect
										#if  (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513)||defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC))
										,HMI_RECT_STR *pdirty_rect,
										U08		depth,/*node of page NO*/
										HMI_RECT_STR *pcliped_farther_rect
										#if defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC)
										//,UINT8 father_alpha
										,HMI_ALPHA_SCALE_PT_STR *pfather_alpha_scale
										#endif
										#elif (defined(HMI_GRAPHIC_TWLIB))	
										,HMI_ELEMENT_TYPE parent_object_id_type,
										HMI_OBJECT_ID_STR hmi_object_id,
										BOOLEAN				bDrawBck
										#if HMI_RENDER_ALL_EXCEPT_BCK==NO
										,HMI_OBJECT_ID_STR parent_object_id
										,BOOLEAN				only_draw_flag
										#endif
										#elif	(defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
										,BOOLEAN				draw_flag
										#endif
										) REENTRANT;
static void  hmi_engine_draw_object(HMI_OBJECT_PROP_STR CONST * phmi_object_prop_table,HMI_RECT_STR CONST *pfarther_rect
									#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||	\
										defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||   \
									defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG))
									,HMI_RECT_STR *pdirty_rect,
									U08 	depth,
									HMI_RECT_STR *pcliped_farther_rect
									#if(defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC))									
									,HMI_ALPHA_SCALE_PT_STR *pfather_alpha_scale
									#endif
									#elif (defined(HMI_GRAPHIC_TW8836)) /*tw*/
									,HMI_ELEMENT_TYPE parent_object_id_type
									#if HMI_RENDER_ALL_EXCEPT_BCK==NO												
									,HMI_OBJECT_ID_STR parent_object_id
									,BOOLEAN		only_draw_flag
									#endif
									#elif	(defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
									,BOOLEAN				draw_flag
									#endif
									) REENTRANT;
										

HMI_OBJECT_ID_STR  hmi_search_container(HMI_CONTAINER_STR CONST * phmi_container_info,HMI_RECT_STR CONST *phmi_father_rect,POINT_TP *phmi_point,BOOLEAN *phmi_is_button) REENTRANT;
BOOLEAN hmi_judge_point_zone(HMI_RECT_STR CONST *rect_t,POINT_TP *point_t);
HMI_OBJECT_ID_STR  hmi_search_object(HMI_OBJECT_PROP_STR CONST * phmi_object_prop_table,HMI_RECT_STR CONST *phmi_father_rect,POINT_TP *press_point,BOOLEAN *phmi_is_button) REENTRANT;
void hmi_get_object_screen_coor(HMI_RECT_STR CONST*pfarther_rect,HMI_RECT_STR CONST*pobject_rect,HMI_RECT_STR *phmi_temp_rect);
void hmi_set_dirty_zone(HMI_RECT_STR *pfarther_rect,HMI_RECT_STR *pdirty_rect);
static void hmi_cube_get_container_point(HMI_OBJECT_ID_STR		hmi_object_id,								
											S3POINT_TP			*ppoint);
#if (HMI_DYN_CONTAINERS_NUMBER > 0)  && (HMI_ALL_STATIC_CONTAINERS_NUMBER > 0)
static BOOLEAN hmi_engine_get_bck_static_container_id(HMI_OBJECT_ID_STR * phmi_dyn_contain_id);
#endif
void hmi_load_file_to_vram(HMI_OBJECT_ID_STR	hmi_object_id);
void call_C_hmi_driver_load_rotation_file(void);
void	hmi_union_rect(HMI_RECT_STR *prect1,HMI_RECT_STR *prect2,HMI_RECT_STR *prect_union);
BOOLEAN hmi_get_rotation_pointer_dirty(HMI_RECT_STR *p_pointer_rect,
										HMI_RECT_STR *p_pointer_rect_old,
										HMI_ROTATION_STR *p_axis,HMI_ROTATION_STR *p_axis_old,
										float_32 new_angel,float_32 old_angel,
										HMI_RECT_STR *p_dirty,
										BOOLEAN	trail,HMI_RECT_STR *p_father_rect);

BOOLEAN hmi_get_rotation_rect(HMI_RECT_STR *p_rect,HMI_ROTATION_STR *p_axis,
									float_32 angel,HMI_RECT_STR *p_rect_rotation);

#if HMI_MAX_SINGLE_STAUS_CNT>0	
void hmi_set_status_list(HMI_OBJECT_ID_STR hmi_event_id);
HMI_OBJECT_ID_STR  hmi_get_next_status(HMI_OBJECT_ID_STR cur_statu,HMI_OBJECT_ID_STR event_id);
void hmi_engine_set_status_action(HMI_OBJECT_ID_STR status_id,HMI_ACTION_TYPE_STR cur_action_status);
#endif
#if HMI_DXY_SPLINE_NUMBER + HMI_SXY_SPLINE_NUMBER > 0U
static BOOLEAN hmi_check_container_child_changed(HMI_OBJECT_ID_STR hmi_object_id, BOOLEAN check_all_child);/*true is check all ,false is check one */
static BOOLEAN hmi_check_object_child_changed(HMI_OBJECT_ID_STR hmi_object_id);
void hmi_clear_spline_point_polyline(HMI_INPUT_POLYGON_STR *pspline_input_point);
#endif
#if HMI_DXY_CUSTOM_CNT + HMI_SXY_CUSTOM_CNT >0U
BOOLEAN hmi_engine_get_custom_prop(HMI_OBJECT_ID_STR custom_id,
										HMI_CUSTOM_PROP_STR *pobject_prop);
#endif
#ifdef HMI_INIT_TEXTURE_MEM_FIRST
void hmi_engine_init_texture_mem(void)
{
	call_C_hmi_driver_init_texture_mem();
	#if defined((HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC))
		//call_C_hmi_driver_init();
	#if (HMI_ENABLE_BIN == HMI_YES)
	#if (HMI_LOAD_SOURCE_MODE == HMI_LOAD_RES_ALL_INIT) 
		call_C_hmi_driver_load_all_file();
		#if defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)
			#if	(HMI_LOAD_SOURCE_BUF_MODE == HMI_LOAD_RES_BUF_ALL_INIT)
				hmi_load_all_to_buf_pixel();
			#endif
		#endif
	#elif(HMI_LOAD_SOURCE_MODE == HMI_LOAD_RES_ONLY_ROTATION)
		call_C_hmi_driver_load_rotation_file();
	#endif
	#endif
	#endif
}
#endif 
BOOLEAN	get_path_transform(ES_MATRIX_STR *pmatrix,POINT_FLOAT_TP	path[],U08 path_len,BOOLEAN bnormal);
void	hmi_two_rect_bound(POINT_FLOAT_TP pa[/*2*/],POINT_FLOAT_TP pb[/*2*/],POINT_FLOAT_TP bound[/*2*/]);

#ifdef HMI_LOAD_RES_SEPARATE
static void hmi_driver_load_one_file(HMI_OBJECT_ID_STR hmi_object_id);
#endif

BOOLEAN hmi_straddle_line(SPOINT_TP *p1,SPOINT_TP *p2,SPOINT_TP *q1,SPOINT_TP *q2);

BOOLEAN hmi_intersection_line(SPOINT_TP *p1,SPOINT_TP *p2,SPOINT_TP *q1,SPOINT_TP *q2);
extern void hmi_status_action_call_func(HMI_OBJECT_ID_STR call_id);
BOOLEAN	hmi_get_midpoint_ctrl(float_32 e,SPOINT32_TP	*pbegin,
									SPOINT32_TP	*pbeginControl,
									SPOINT32_TP	*pendControl,
									SPOINT32_TP	*pend,
									SPOINT32_TP	*pmid,
									SPOINT32_TP	*pmidControl1,
									SPOINT32_TP	*pmidControl2);
HMI_CONTAINER_STR CONST* hmi_engine_get_container_child_addr(HMI_OBJECT_ID_STR hmi_container_id);
void hmi_engine_get_object_prop2(HMI_OBJECT_ID_STR hmi_object_id,HMI_ELEMENT_PROP2_STR *pobject_prop);


void hmi_engine_init(void)
{
#if HMI_LAYER_0_HIGHEST_PRIORITY > 1
  	UINT16 hmi_count = 0U;
#endif
#if HMI_NB_ALL_DYN_OBJECT_BYTE>255
	UINT16 hmi_dyn_object_cf = 0U;
#else
	UINT8	hmi_dyn_object_cf = 0U;
#endif

#ifdef HMI_GRAPHIC_AGG
	UINT8 index	= 0U;
#endif
#if defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC)
HMI_OBJECT_ID_STR	hmi_bck_index	= 0U;
#endif
#if HMI_ALL_LAYERS_NUMBER >0U
	#if (HMI_LAYER_0_HIGHEST_PRIORITY > 1U) ||(HMI_LAYER_1_HIGHEST_PRIORITY > 1U)
	hmi_highest_active_page_priority[HMI_LAYER_SCREEN0] = HMI_LAYER_0_HIGHEST_PRIORITY;
	hmi_highest_active_page_priority[HMI_LAYER_SCREEN1] = HMI_LAYER_1_HIGHEST_PRIORITY;
	#endif
	#if HMI_LAYER_0_HIGHEST_PRIORITY > 1U
	for(hmi_count=0U; hmi_count < hmi_layer_table[HMI_LAYER_SCREEN0].max_priority; hmi_count++)
	#endif
	{
		#if (HMI_LAYER_0_HIGHEST_PRIORITY > 1U)   
		hmi_layer_0_active_page_id[hmi_count].new_page= HMI_PAGES_NUMBER;
		hmi_layer_0_active_page_id[hmi_count].old_page= HMI_PAGES_NUMBER;
		#elif (HMI_LAYER_0_HIGHEST_PRIORITY == 1U)
		hmi_layer_0_active_page_id[0].new_page= HMI_PAGES_NUMBER;
		hmi_layer_0_active_page_id[0].old_page= HMI_PAGES_NUMBER;
		#else
		#endif
	}
#if HMI_ALL_LAYERS_NUMBER > 1U
	#if HMI_LAYER_1_HIGHEST_PRIORITY > 1U
		for(hmi_count=0U; hmi_count < hmi_layer_table[HMI_LAYER_SCREEN1].max_priority; hmi_count++)
	#endif
		{
		#if (HMI_LAYER_1_HIGHEST_PRIORITY > 1U)   
			hmi_layer_1_active_page_id[hmi_count].new_page= HMI_PAGES_NUMBER;
			hmi_layer_1_active_page_id[hmi_count].old_page= HMI_PAGES_NUMBER;
		#elif (HMI_LAYER_1_HIGHEST_PRIORITY == 1U)
			hmi_layer_1_active_page_id[0].new_page= HMI_PAGES_NUMBER;
			hmi_layer_1_active_page_id[0].old_page= HMI_PAGES_NUMBER;
		#else
		#endif
		}
#endif
#endif
	hmi_driver_woking_status_flag = HMI_FRAMEBUFFER_FREE;
   	#if HMI_ALL_DYN_OBJECTS_NUMBER >0U
	for(hmi_dyn_object_cf=0U;hmi_dyn_object_cf<HMI_NB_ALL_DYN_OBJECT_BYTE;hmi_dyn_object_cf++)
	{
		hmi_dynamic_object_changed_flag[hmi_dyn_object_cf]=0U;
	}
	#endif
	#if HMI_DYN_LANGUAGE_NUMBER>0
	hmi_cur_language=HMI_DYN_LANGUAGE_INIT;
	#endif
	
#if (defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC))
#if	HMI_DXY_PAGES_NUMBER	> 0
	for(hmi_bck_index=0U; hmi_bck_index < HMI_DXY_PAGES_NUMBER; hmi_bck_index++)
	{
		hmi_dxy_page_rect_bck[hmi_bck_index].x	= HMI_INVALID_COOR;
		hmi_dxy_page_rect_bck[hmi_bck_index].y	= HMI_INVALID_COOR;
		hmi_dxy_page_rect_bck[hmi_bck_index].w	= 0;
		hmi_dxy_page_rect_bck[hmi_bck_index].h	= 0;
	}
#endif

#if	HMI_DXY_IMAGELIST_NUMBER	> 0
	for(hmi_bck_index=0U; hmi_bck_index < HMI_DXY_IMAGELIST_NUMBER; hmi_bck_index++)
	{
		hmi_dxy_imagelist_rect_bck[hmi_bck_index].x	= HMI_INVALID_COOR;
		hmi_dxy_imagelist_rect_bck[hmi_bck_index].y	= HMI_INVALID_COOR;
		hmi_dxy_imagelist_rect_bck[hmi_bck_index].w	= 0;
		hmi_dxy_imagelist_rect_bck[hmi_bck_index].h	= 0;
	}
#endif
	
#if HMI_DXY_SCROLLBAR_NUMBER>0
	for(hmi_bck_index=0U; hmi_bck_index < HMI_DXY_SCROLLBAR_NUMBER; hmi_bck_index++)
	{
		hmi_dxy_scrollbar_rect_bck[hmi_bck_index].x	= HMI_INVALID_COOR;
		hmi_dxy_scrollbar_rect_bck[hmi_bck_index].y	= HMI_INVALID_COOR;
		hmi_dxy_scrollbar_rect_bck[hmi_bck_index].w	= 0;
		hmi_dxy_scrollbar_rect_bck[hmi_bck_index].h	= 0;
	}
#endif
	
#if HMI_DXY_BUTTON_NUMBER>0
	for(hmi_bck_index=0U; hmi_bck_index < HMI_DXY_BUTTON_NUMBER; hmi_bck_index++)
	{
		hmi_dxy_button_rect_bck[hmi_bck_index].x	= HMI_INVALID_COOR;
		hmi_dxy_button_rect_bck[hmi_bck_index].y	= HMI_INVALID_COOR;
		hmi_dxy_button_rect_bck[hmi_bck_index].w	= 0;
		hmi_dxy_button_rect_bck[hmi_bck_index].h	= 0;
	}
#endif

#if HMI_DYN_XY_EDIT_TEXTS_NUMBER>0
	for(hmi_bck_index=0U; hmi_bck_index < HMI_DYN_XY_EDIT_TEXTS_NUMBER; hmi_bck_index++)
	{
		hmi_dyn_xy_edit_text_prop_table_bck[hmi_bck_index].x	= HMI_INVALID_COOR;
		hmi_dyn_xy_edit_text_prop_table_bck[hmi_bck_index].y	= HMI_INVALID_COOR;
		hmi_dyn_xy_edit_text_prop_table_bck[hmi_bck_index].w	= 0;
		hmi_dyn_xy_edit_text_prop_table_bck[hmi_bck_index].h	= 0;
	}
#endif
#if HMI_UNEDIT_TEXTS_DYN_XY_NUMBER>0
	for(hmi_bck_index=0U; hmi_bck_index < HMI_UNEDIT_TEXTS_DYN_XY_NUMBER; hmi_bck_index++)
	{
		hmi_dyn_xy_unedit_text_prop_table_bck[hmi_bck_index].x	= HMI_INVALID_COOR;
		hmi_dyn_xy_unedit_text_prop_table_bck[hmi_bck_index].y	= HMI_INVALID_COOR;
		hmi_dyn_xy_unedit_text_prop_table_bck[hmi_bck_index].w	= 0;
		hmi_dyn_xy_unedit_text_prop_table_bck[hmi_bck_index].h	= 0;
	}
#endif

#if HMI_DYN_CONTAINERS_NUMBER>0
	for(hmi_bck_index=0U; hmi_bck_index < HMI_DYN_CONTAINERS_NUMBER; hmi_bck_index++)
	{
		hmi_dyn_container_table_bck[hmi_bck_index]	= HMI_NB_ELEMENTS;
	}	
#endif
#if HMI_DYN_FILL_PAGES_NUMBER>0
	for(hmi_bck_index=0U; hmi_bck_index < HMI_DYN_FILL_PAGES_NUMBER; hmi_bck_index++)
	{
		hmi_fills_dyn_xy_rect_bck[hmi_bck_index].x	= HMI_INVALID_COOR;
		hmi_fills_dyn_xy_rect_bck[hmi_bck_index].y	= HMI_INVALID_COOR;
		hmi_fills_dyn_xy_rect_bck[hmi_bck_index].w	= 0;
		hmi_fills_dyn_xy_rect_bck[hmi_bck_index].h	= 0;
	}
#endif
#if HMI_DYN_GFILL_NUMBER>0
	for(hmi_bck_index=0U; hmi_bck_index < HMI_DYN_GFILL_NUMBER; hmi_bck_index++)
	{
		hmi_gradient_dxy_fill_rect_bck[hmi_bck_index].x	= HMI_INVALID_COOR;
		hmi_gradient_dxy_fill_rect_bck[hmi_bck_index].y	= HMI_INVALID_COOR;
		hmi_gradient_dxy_fill_rect_bck[hmi_bck_index].w	= 0;
		hmi_gradient_dxy_fill_rect_bck[hmi_bck_index].h	= 0;
	}
#endif
#if HMI_DXY_CUBE_NUMBER>0
	for(hmi_bck_index=0U; hmi_bck_index < HMI_DXY_CUBE_NUMBER; hmi_bck_index++)
	{
		hmi_cubes_dyn_xy_rect_bck[hmi_bck_index].x = HMI_INVALID_COOR;
		hmi_cubes_dyn_xy_rect_bck[hmi_bck_index].y = HMI_INVALID_COOR;
		hmi_cubes_dyn_xy_rect_bck[hmi_bck_index].w = 0;
		hmi_cubes_dyn_xy_rect_bck[hmi_bck_index].h = 0;
	}
#endif
#if HMI_DXY_CONTAINERS_NUMBER>0
	for(hmi_bck_index=0U; hmi_bck_index < HMI_DXY_CONTAINERS_NUMBER; hmi_bck_index++)
	{
		hmi_dyn_xy_container_rect_bck[hmi_bck_index].x = HMI_INVALID_COOR;
		hmi_dyn_xy_container_rect_bck[hmi_bck_index].y = HMI_INVALID_COOR;
		hmi_dyn_xy_container_rect_bck[hmi_bck_index].w = 0;
		hmi_dyn_xy_container_rect_bck[hmi_bck_index].h = 0;
	}
#endif
#if HMI_DXY_BITMAPS_NUMBER>0
	for(hmi_bck_index=0U; hmi_bck_index < HMI_DXY_BITMAPS_NUMBER; hmi_bck_index++)
	{
		hmi_bmp_dyn_xy_rect_bck[hmi_bck_index].x = HMI_INVALID_COOR;
		hmi_bmp_dyn_xy_rect_bck[hmi_bck_index].y = HMI_INVALID_COOR;
		hmi_bmp_dyn_xy_rect_bck[hmi_bck_index].w = 0;
		hmi_bmp_dyn_xy_rect_bck[hmi_bck_index].h = 0;
		hmi_bmp_dyn_angel_bck[hmi_bck_index]	= 0;
	}
#endif
#if HMI_DXY_SPLINE_NUMBER > 0U
	for(hmi_bck_index=0U; hmi_bck_index < HMI_DXY_SPLINE_NUMBER; hmi_bck_index++)
	{
		hmi_spline_dyn_xy_rect_bck[hmi_bck_index].x = HMI_INVALID_COOR;
		hmi_spline_dyn_xy_rect_bck[hmi_bck_index].y = HMI_INVALID_COOR;
		hmi_spline_dyn_xy_rect_bck[hmi_bck_index].w = 0;
		hmi_spline_dyn_xy_rect_bck[hmi_bck_index].h = 0;
	}
#endif
#if HMI_DXY_CUSTOM_CNT > 0U
	for(hmi_bck_index=0U; hmi_bck_index < HMI_DXY_CUSTOM_CNT; hmi_bck_index++)
	{
		hmi_custom_dyn_xy_rect_bck[hmi_bck_index].x = HMI_INVALID_COOR;
		hmi_custom_dyn_xy_rect_bck[hmi_bck_index].y = HMI_INVALID_COOR;
		hmi_custom_dyn_xy_rect_bck[hmi_bck_index].w = 0;
		hmi_custom_dyn_xy_rect_bck[hmi_bck_index].h = 0;
	}
#endif
#endif

#if HMI_DXY_SPLINE_NUMBER + HMI_SXY_SPLINE_NUMBER > 0U 
	hmi_spline_input_point.act_array_len					= 0U;
	hmi_spline_input_point.phmi_input_point_polygon			= NULL;
	hmi_spline_input_point.pnext_pre_list					= NULL;
	hmi_spline_input_point.head_index						= 0U;
	hmi_clear_spline_point_polyline(&hmi_spline_input_point);

	
	#if HMI_DXY_ONE_POINT_SPLINE_NUMBER >0U
	for(hmi_bck_index =0;hmi_bck_index < HMI_DXY_ONE_POINT_SPLINE_NUMBER;hmi_bck_index++)
	{
		dxy_spline_head_tail_offset_array[hmi_bck_index].head		= dxy_one_point_range_array[hmi_bck_index].begin;
		dxy_spline_head_tail_offset_array[hmi_bck_index].tail		= dxy_one_point_range_array[hmi_bck_index].begin;
		dxy_spline_head_tail_offset_array[hmi_bck_index].offset		= 0U;
	}
	#endif
	#if HMI_SXY_ONE_POINT_SPLINE_NUMBER >0U
	for(hmi_bck_index =0;hmi_bck_index < HMI_SXY_ONE_POINT_SPLINE_NUMBER;hmi_bck_index++)
	{
		sxy_spline_head_tail_offset_array[hmi_bck_index].head		= sxy_one_point_range_array[hmi_bck_index].begin;
		sxy_spline_head_tail_offset_array[hmi_bck_index].tail		= sxy_one_point_range_array[hmi_bck_index].begin;
		sxy_spline_head_tail_offset_array[hmi_bck_index].offset		= 0U;
	}
	#endif
#endif
#if HMI_MAX_SUB_TABLE_CNT>0
	for(hmi_bck_index =0;hmi_bck_index < HMI_MAX_SUB_TABLE_CNT;hmi_bck_index++)
	{
		hmi_last_table_status[hmi_bck_index ]=0;
	}
#endif
	
	hmi_public_buffer.ppublic_buffer	= NULL;
	hmi_public_buffer.buffer_len	= 0;
	memset(&hmi_light_status,0,sizeof(hmi_light_status));

	hmi_init_qd_buffer_list();/*2023 06 04*/
#ifdef HMI_GRAPHIC_AGG
   	call_C_hmi_driver_init();
	for(index=0;index < HMI_BBITMAP_NUMBER;index++)
	{
		call_C_hmi_driver_load_bigimage(hmi_bigbitmap_table[index]);
	}
	call_C_hmi_driver_load_file();
#endif

#if(HMI_CREATE_WIN_IN_INIT == HMI_YES)
	hmi_engine_set_object_info(0,HMI_ACTIVE_PAGE_BIT);/*added by pxguo 180723,create win in init*/
	#if (defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC))
	hmi_engine_create_rgl_window();
	#endif
#endif

#ifndef HMI_INIT_TEXTURE_MEM_FIRST
	#if (defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC))
	call_C_hmi_driver_init(
				#if defined(HMI_GRAPHIC_OPENGLES)
					HMI_VERTEX_SHADER_OFFSET,
					HMI_VERTEX_SHADER_LEN,
					HMI_FRAGMENT_SHADER_OFFSET,
					HMI_FRAGMENT_SHADER_LEN,
					0/*shader bin format*/,
					hmi_bmp_segment_tlb
				#endif
				);
	#ifdef HMI_RENESAS_L_CLUT_ENABLE
	call_C_hmi_driver_init_CLUT(hmi_clut);
	#endif
	
#if (HMI_ENABLE_BIN == HMI_YES)
	#if (HMI_LOAD_SOURCE_MODE == HMI_LOAD_RES_ALL_INIT)	
		#if ((HMI_DXY_IMAGELIST_NUMBER+HMI_SXY_IMAGELIST_NUMBER+	\
			HMI_DXY_SCROLLBAR_NUMBER+HMI_SXY_SCROLLBAR_NUMBER+	\
			HMI_DXY_BUTTON_NUMBER+HMI_SXY_BUTTON_NUMBER+	\
			HMI_DXY_BITMAPS_NUMBER+HMI_SXY_BITMAPS_NUMBER)>0) 
				call_C_hmi_driver_load_all_file();
		#endif
	#elif(HMI_LOAD_SOURCE_MODE == HMI_LOAD_RES_ONLY_ROTATION)
		call_C_hmi_driver_load_rotation_file();
	#elif(HMI_LOAD_SOURCE_MODE == HMI_LOAD_RES_SHEET)
		hmi_load_fifo.head	= 0;
		hmi_load_fifo.tail	= 0;
		for(hmi_bck_index = 0;hmi_bck_index < HMI_LOAD_LIST_MAX_NB;hmi_bck_index++)
		{
			hmi_load_id_list[hmi_bck_index]	= HMI_NB_ELEMENTS;
		}
		//hmi_load_sheet_add_all_id();
		//call_C_hmi_driver_load_sheet();		
	#endif
	
	#if defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)
		#if	(HMI_LOAD_SOURCE_BUF_MODE == HMI_LOAD_RES_BUF_ALL_INIT)
			hmi_load_all_to_buf_pixel();
		#endif
	#endif
	
#endif
#endif
#endif

	#ifdef HMI_GRAPHIC_ST7513
	call_C_hmi_driver_init();
	#endif
	#if (defined(HMI_MCU_TW36)||(defined(HMI_MCU_TW25)))
	call_C_hmi_driver_init();
	#endif	
	#if	(defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC))
	#if HMI_MAX_SUB_TABLE_CNT >0
	hmi_cur_table =HMI_DEFAULT_STATUS_TLB;
	#endif
	#endif
	#if	(defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
	#if HMI_SCENE_DYN_CONTAINERS_NUMBER >0
	call_C_hmi_driver_init(
							#if HMI_TEXT_COLOR_PALLETE_LEN>0
							hmi_text_color_pallete,
							#endif
							hmi_scene_data_table
							);
	#else
	call_C_hmi_driver_init(
							#if HMI_TEXT_COLOR_PALLETE_LEN>0
							hmi_text_color_pallete,
							#endif
							NULL
							);
	#endif
	#endif	

	//hmi_init_qd_buffer_list();
#if HMI_DXY_CUSTOM_CNT + HMI_SXY_CUSTOM_CNT >0U
	hmi_init_sub_model_3d_fifo();
#endif
}

void hmi_engine_deinit(void)
{
#if HMI_SXY_3DCUBE_NUMBER + HMI_DXY_3DCUBE_NUMBER > 0
	#if defined( HMI_GRAPHIC_OPENGLES )
	INT32	loop	= 0;
	INT32	j		= 0;	
	#endif
#endif
	
#if HMI_DXY_SPLINE_NUMBER + HMI_SXY_SPLINE_NUMBER > 0 
	hmi_dealloc_vram((void *)(hmi_spline_input_point.phmi_input_point_polygon));
	hmi_dealloc_vram((void *)(hmi_spline_input_point.pnext_pre_list));
	hmi_spline_input_point.phmi_input_point_polygon = NULL;
	hmi_spline_input_point.pnext_pre_list = NULL;
#endif
	if(hmi_public_buffer.ppublic_buffer != NULL)
	{
		hmi_dealloc_vram(hmi_public_buffer.ppublic_buffer);
		hmi_public_buffer.ppublic_buffer	= NULL;
	}
	hmi_public_buffer.buffer_len	= 0;

	/*Release 3D vertex buf */
#if HMI_SXY_3DCUBE_NUMBER + HMI_DXY_3DCUBE_NUMBER > 0
	#if defined( HMI_GRAPHIC_OPENGLES )
	for(loop = 0;loop < HMI_DXY_SXY_3D_VER_BUF_LEN;loop++)
	{	
		if((hmi_dxy_sxy_3d_ver_buf[loop].vboIds[0] > 0)||
			(hmi_dxy_sxy_3d_ver_buf[loop].vboIds[1] > 0)||
			(hmi_dxy_sxy_3d_ver_buf[loop].vboIds[2] > 0)||
			(hmi_dxy_sxy_3d_ver_buf[loop].vboIds[3] > 0))
		{
			glDeleteBuffers(HMI_VERTEX_BUF_CNT, hmi_dxy_sxy_3d_ver_buf[loop].vboIds);
			for(j = 0;j < HMI_VERTEX_BUF_CNT;j++)
			{
				hmi_dxy_sxy_3d_ver_buf[loop].vboIds[j] = 0;
			}
		}
	}
	#endif
#endif
}

#ifdef HMI_GRAPHIC_ST7513
HMI_HEIGHT_STR	hmi_get_layer_height(UINT8 layer)
{
	HMI_HEIGHT_STR h	= 0U;

	h	= HMI_MAX_HEIGHT;
	return h;
}
HMI_WIDTH_STR	hmi_get_layer_width(UINT8 layer)
{
	HMI_WIDTH_STR w	= 0;

	w	= HMI_MAX_WIDTH;
	return w;
}
#endif

#if HMI_VDXY_PAGES_NUMBER+HMI_VSXY_PAGES_NUMBER > 0
static void hmi_engine_draw_video(HMI_OBJECT_ID_STR video_page_object,BOOLEAN dxy)
{
	#ifndef HMI_GRAPHIC_ST7513
	HMI_OBJECT_ID_STR	video_page_index=0;		
	HMI_OBJECT_ID_STR	video_s_dxy_page_index=0;
	#if HMI_VDXY_PAGES_NUMBER>0
	if(dxy)
	{
		video_page_index		= HMI_GET_VPAGE_DXY_ID_INDEX(video_page_object);
		video_s_dxy_page_index	= HMI_GET_PAGE_DXY_ID_INDEX(video_page_object);
		if((video_page_index < HMI_VDXY_PAGES_NUMBER)&&
			(video_s_dxy_page_index < HMI_DXY_PAGES_NUMBER))
		{			
			hmi_display_video(&hmi_dxy_page_video_fmt[video_page_index],
							&hmi_dxy_page_rect[video_s_dxy_page_index],0,0);
		}
	}
	else
	#endif
	#if HMI_VSXY_PAGES_NUMBER>0
	{
		video_page_index		= HMI_GET_VPAGE_SXY_ID_INDEX(video_page_object);
		video_s_dxy_page_index	= HMI_GET_PAGE_SXY_ID_INDEX(video_page_object);
		if((video_page_index < HMI_VSXY_PAGES_NUMBER)&&
			(video_s_dxy_page_index < HMI_SXY_PAGES_NUMBER))
		{			
			hmi_display_video(&hmi_sxy_page_video_fmt[video_page_index],
							(HMI_RECT_STR *)(&hmi_sxy_page_rect[video_s_dxy_page_index]),0,0);
		}
	}
	#endif
	{
	}
	#endif
}
#endif

#if HMI_PAGES_NUMBER>0U
void hmi_engine_set_page(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data)
{	
	#if (HMI_LAYER_0_HIGHEST_PRIORITY > 1)||(HMI_LAYER_1_HIGHEST_PRIORITY > 1)
	UINT8 hmi_priority_cnt	= 0U;
	UINT8 				hmi_page_screen_id	= 0U;
	#endif
	UINT16 hmi_object_index	= 0U;
	HMI_PAGE_TABLE_STR CONST * phmi_page_info	= NULL;
	HMI_PRIOR_PAGE_STR *phmi_active_page_id	= NULL;
	
	if(HMI_IS_PAGE(hmi_object_id))
	{		
		#if HMI_DXY_PAGES_NUMBER > 0U
		if(HMI_IS_DXY_PAGE(hmi_object_id))
		{
			hmi_object_index	= HMI_GET_PAGE_DXY_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DXY_PAGES_NUMBER)
			{
		 		phmi_page_info= &hmi_dxy_page_table[hmi_object_index];
			}
		}
		else 
		#endif
		#if HMI_SXY_PAGES_NUMBER>0U
		if(HMI_IS_SXY_PAGE(hmi_object_id))
		{
			hmi_object_index	= HMI_GET_PAGE_SXY_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_SXY_PAGES_NUMBER)
			{
				phmi_page_info= &hmi_sxy_page_table[hmi_object_index];
			}
		}
		else
		#endif
		{
		}
		if(phmi_page_info != NULL)
		{
				if(phmi_page_info->screen_id == HMI_LAYER_SCREEN0)
				{
					phmi_active_page_id = hmi_layer_0_active_page_id;
				}
				#if HMI_ALL_LAYERS_NUMBER > 1
				else if(phmi_page_info->screen_id == HMI_LAYER_SCREEN1)
				{
					phmi_active_page_id = hmi_layer_1_active_page_id;
				}
				else
				{
					phmi_active_page_id = hmi_layer_0_active_page_id;
				}
				#endif
				if(phmi_active_page_id != NULL)
				{
				#if (HMI_LAYER_0_HIGHEST_PRIORITY > 1)||(HMI_LAYER_1_HIGHEST_PRIORITY > 1)
				hmi_page_screen_id	= phmi_page_info->screen_id;
				#endif
				if(hmi_object_data == HMI_REMOVE_PAGE_BIT)
				{
					if(phmi_active_page_id[phmi_page_info ->page_priority].new_page 
						== hmi_object_id)
				{
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
					#if HMI_EVENT_STAND_NUMBER>0
					hmi_do_event(hmi_object_id,HMI_PAGE_OFF);
					#endif
						phmi_active_page_id[phmi_page_info ->page_priority].old_page = 
									phmi_active_page_id[phmi_page_info ->page_priority].new_page;
						phmi_active_page_id[phmi_page_info ->page_priority].new_page = HMI_PAGES_NUMBER;
						
						#if(HMI_LAYER_1_HIGHEST_PRIORITY > 1)
						if(hmi_page_screen_id == HMI_LAYER_SCREEN1)
						{
							hmi_priority_cnt				= hmi_highest_active_page_priority[HMI_LAYER_SCREEN1];
							hmi_highest_active_page_priority[HMI_LAYER_SCREEN1]= HMI_LAYER_1_HIGHEST_PRIORITY;
							if(hmi_priority_cnt < HMI_LAYER_1_HIGHEST_PRIORITY)
							{
								hmi_priority_cnt++; 
								while(hmi_priority_cnt != 0)
								{
									hmi_priority_cnt--;
									hmi_object_id	= phmi_active_page_id[hmi_priority_cnt].new_page;
									if(hmi_object_id != HMI_PAGES_NUMBER)
									{
										if(hmi_highest_active_page_priority[HMI_LAYER_SCREEN1] == HMI_LAYER_1_HIGHEST_PRIORITY)
										{
											hmi_highest_active_page_priority[HMI_LAYER_SCREEN1] = hmi_priority_cnt;
										}
										/*HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
										HMI_GFX_SET_STATUS(HMI_SEND_EVENT);*/
									}
								}
							}
						}
						#endif
						#if (HMI_LAYER_0_HIGHEST_PRIORITY > 1)
						if(hmi_page_screen_id == HMI_LAYER_SCREEN0)
						{
							hmi_priority_cnt				= hmi_highest_active_page_priority[HMI_LAYER_SCREEN0];
							hmi_highest_active_page_priority[HMI_LAYER_SCREEN0]= HMI_LAYER_0_HIGHEST_PRIORITY;
							if(hmi_priority_cnt < HMI_LAYER_0_HIGHEST_PRIORITY)
							{
								hmi_priority_cnt++; 
								while(hmi_priority_cnt != 0)
								{
									hmi_priority_cnt--;
									hmi_object_id	= phmi_active_page_id[hmi_priority_cnt].new_page;
									if(hmi_object_id != HMI_PAGES_NUMBER)
									{
										if(hmi_highest_active_page_priority[HMI_LAYER_SCREEN0] == HMI_LAYER_0_HIGHEST_PRIORITY)
										{
											hmi_highest_active_page_priority[HMI_LAYER_SCREEN0] = hmi_priority_cnt;
										}
										/*HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
										HMI_GFX_SET_STATUS(HMI_SEND_EVENT);*/
									}
								}
							}
						}
						#endif 
					}
				}
				else if(phmi_active_page_id[phmi_page_info->page_priority].new_page 
						!= hmi_object_id)
				{
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
					#if (HMI_EVENT_STAND_NUMBER>0)
					hmi_do_event(phmi_active_page_id[phmi_page_info ->page_priority].new_page,HMI_PAGE_OFF);
					hmi_do_event(hmi_object_id,HMI_PAGE_ON);
					#endif
					phmi_active_page_id[phmi_page_info ->page_priority].old_page=phmi_active_page_id[phmi_page_info ->page_priority].new_page;
					phmi_active_page_id[phmi_page_info ->page_priority].new_page= (HMI_PAGE_ID_STR)hmi_object_id;
					#if defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG) || defined(S6J3200_GRAPHIC)				
					/*off old page video*/
					off_layer_video();		
					/*record new page layer and element id*/
					hmi_access_layer_video_status(hmi_object_id);		
					#endif
					
					#if HMI_LAYER_1_HIGHEST_PRIORITY > 1		
					if(hmi_page_screen_id == HMI_LAYER_SCREEN1)
					{
						hmi_priority_cnt = HMI_LAYER_1_HIGHEST_PRIORITY;
						if(hmi_highest_active_page_priority[HMI_LAYER_SCREEN1] == HMI_LAYER_1_HIGHEST_PRIORITY)
						{
							hmi_highest_active_page_priority[HMI_LAYER_SCREEN1] = phmi_page_info->page_priority;
						}
						else if(phmi_page_info ->page_priority > hmi_highest_active_page_priority[HMI_LAYER_SCREEN1])
						{
							hmi_highest_active_page_priority[HMI_LAYER_SCREEN1] = phmi_page_info ->page_priority;
						}
						else
						{
							hmi_priority_cnt = hmi_highest_active_page_priority[HMI_LAYER_SCREEN1];
						}
					}
					#endif
					#if HMI_LAYER_0_HIGHEST_PRIORITY > 1
					if(hmi_page_screen_id == HMI_LAYER_SCREEN0)
					{
						hmi_priority_cnt = HMI_LAYER_0_HIGHEST_PRIORITY;
						if(hmi_highest_active_page_priority[HMI_LAYER_SCREEN0] == HMI_LAYER_0_HIGHEST_PRIORITY)
						{
							hmi_highest_active_page_priority[HMI_LAYER_SCREEN0] = phmi_page_info->page_priority;
						}
						else if(phmi_page_info ->page_priority > hmi_highest_active_page_priority[HMI_LAYER_SCREEN0])
						{
							hmi_highest_active_page_priority[HMI_LAYER_SCREEN0] = phmi_page_info ->page_priority;
						}
						else
						{
							hmi_priority_cnt = hmi_highest_active_page_priority[HMI_LAYER_SCREEN0];
						}
					}
					#endif 
				}
				else if(hmi_object_data == HMI_REFRESH_PAGE_BIT)
				{
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
				else
				{
				}
			}
		}
	}
}
#endif

#if HMI_DXY_CONTAINERS_NUMBER > 0
void hmi_engine_set_dxy_container(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data)
{	
	
#if HMI_DXY_CONTAINERS_SCALE_ALPHA_NUMBER >0U
	float_32 container_scale = 0.0f;
#endif
	
	if(HMI_IS_DYN_XY_CONTAINER_PROPERTY(hmi_object_id))
	{		
		if(HMI_IS_DYN_X_CONTAINER(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_X_CONTAINER_INDEX(hmi_object_id);
			if(hmi_object_id <HMI_DXY_CONTAINERS_NUMBER)
			{
				if(hmi_dyn_xy_container_rect[hmi_object_id].x != (HMI_X_STR)hmi_object_data)
				{
					/*RGL container support video*/
					#if defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)
					if(hmi_dxy_container_video_fmt[hmi_object_id].video_fmt_channel&HMI_VIDEO_WINDOW)
					{
						/*set video xy change flag*/
						hmi_dxy_container_video_status[hmi_object_id]=hmi_dxy_container_video_status[hmi_object_id]|HMI_VIDEO_XY_FLAG;
					}
					#endif
					hmi_dyn_xy_container_rect[hmi_object_id].x  = (HMI_X_STR)hmi_object_data;
					hmi_object_id = HMI_DXY_CONTAINER_PRO2DXY_CONTAINER(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);					
				}
			}
		}
		else if(HMI_IS_DYN_Y_CONTAINER(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_Y_CONTAINER_INDEX(hmi_object_id);
			if(hmi_object_id <HMI_DXY_CONTAINERS_NUMBER)
			{
				if(hmi_dyn_xy_container_rect[hmi_object_id].y != (HMI_Y_STR)hmi_object_data)
				{
					/*RGL container support video*/
					#if defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)
					if(hmi_dxy_container_video_fmt[hmi_object_id].video_fmt_channel&HMI_VIDEO_WINDOW)
					{
						/*set video xy change flag*/
						hmi_dxy_container_video_status[hmi_object_id]=hmi_dxy_container_video_status[hmi_object_id]|HMI_VIDEO_XY_FLAG;
					}
					#endif
					hmi_dyn_xy_container_rect[hmi_object_id].y  = (HMI_Y_STR)hmi_object_data;
					hmi_object_id = HMI_DXY_CONTAINER_PRO2DXY_CONTAINER(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
					
				}
			}
		}
		else if(HMI_IS_DYN_W_CONTAINER(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_W_CONTAINER_INDEX(hmi_object_id);
			if(hmi_object_id <HMI_DXY_CONTAINERS_NUMBER)
			{
				if(hmi_dyn_xy_container_rect[hmi_object_id].w != (HMI_WIDTH_STR)hmi_object_data)
				{
					/*RGL container support video*/
					#if defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)
					if(hmi_dxy_container_video_fmt[hmi_object_id].video_fmt_channel&HMI_VIDEO_WINDOW)
					{
						/*set video wh change flag*/
						hmi_dxy_container_video_status[hmi_object_id]=hmi_dxy_container_video_status[hmi_object_id]|HMI_VIDEO_WH_FLAG;
					}
					#endif
					hmi_dyn_xy_container_rect[hmi_object_id].w  = (HMI_WIDTH_STR)hmi_object_data;
					hmi_object_id = HMI_DXY_CONTAINER_PRO2DXY_CONTAINER(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
					
				}
			}
		}
		else if(HMI_IS_DYN_H_CONTAINER(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_H_CONTAINER_INDEX(hmi_object_id);
			if(hmi_object_id <HMI_DXY_CONTAINERS_NUMBER)
			{
				if(hmi_dyn_xy_container_rect[hmi_object_id].h != (HMI_HEIGHT_STR)hmi_object_data)
				{
					/*RGL container support video*/
					#if defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)
					if(hmi_dxy_container_video_fmt[hmi_object_id].video_fmt_channel&HMI_VIDEO_WINDOW)
					{
						/*set video wh change flag*/
						hmi_dxy_container_video_status[hmi_object_id]=hmi_dxy_container_video_status[hmi_object_id]|((BYTE)HMI_VIDEO_WH_FLAG);
					}
					#endif
					hmi_dyn_xy_container_rect[hmi_object_id].h  = (HMI_HEIGHT_STR)hmi_object_data;
					hmi_object_id = HMI_DXY_CONTAINER_PRO2DXY_CONTAINER(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);					
				}
			}
		}
		else if(HMI_IS_DYN_ALPHA_CONTAINER(hmi_object_id))
		{
			
			#if HMI_DXY_CONTAINERS_SCALE_ALPHA_NUMBER >0U
			hmi_object_id = HMI_GET_DYN_ALPHA_CONTAINER_INDEX(hmi_object_id);
			if(hmi_object_id <HMI_DXY_CONTAINERS_SCALE_ALPHA_NUMBER)
			{
				if(hmi_dyn_xy_container_alpha_scale[hmi_object_id].alpha!= (UINT8)hmi_object_data)
				{
					hmi_dyn_xy_container_alpha_scale[hmi_object_id].alpha= (UINT8)hmi_object_data;
					hmi_object_id = HMI_DXY_CONTAINER_PRO2DXY_CONTAINER(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);					
				}
			}
			#endif
		}
		else if(HMI_IS_DYN_SCALE_CONTAINER(hmi_object_id))
		{
			#if HMI_DXY_CONTAINERS_SCALE_ALPHA_NUMBER >0U
			hmi_object_id = HMI_GET_DYN_SCALE_CONTAINER_INDEX(hmi_object_id);
			if(hmi_object_id <HMI_DXY_CONTAINERS_SCALE_ALPHA_NUMBER)
			{
				container_scale=HMI_U32_TO_F32(hmi_object_data);
				if(fabs(hmi_dyn_xy_container_alpha_scale[hmi_object_id].scale - container_scale)> HMI_FLOAT_TOLERANCE)
				{
					hmi_dyn_xy_container_alpha_scale[hmi_object_id].scale  = container_scale;
					hmi_object_id = HMI_DXY_CONTAINER_PRO2DXY_CONTAINER(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);					
				}
			}
			#endif
		}
		else
		{
			;
		}
	 }	
}
#endif
#if HMI_DXY_BITMAPS_NUMBER > 0U
void hmi_engine_set_dxy_bitmaps(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data)
{ 
	#if HMI_SUPPORT_FLOAT_ANGEL==HMI_YES
	float_32	value_angel=0;
	#endif
	#if HMI_DXY_ROTATION_BITMAPS_NUMBER > 0
	HMI_OBJECT_DATA_STR hmi_object_id_index	= 0;
	#endif
	if(HMI_IS_DYN_XY_BITMAP_PROPERTY(hmi_object_id))
	{
		if(HMI_IS_DYN_X_BITMAP(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_X_BITMAPS_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_BITMAPS_NUMBER)
			{				
				if(hmi_bmp_dyn_xy_rect[hmi_object_id].x != (HMI_X_STR)(hmi_object_data))
				{
					hmi_bmp_dyn_xy_rect[hmi_object_id].x = (HMI_X_STR)(hmi_object_data);
					hmi_object_id = HMI_DXY_BITMAP_PRO2DXY_BITMAP(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}				
			}
		}
		else if(HMI_IS_DYN_Y_BITMAP(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_Y_BITMAPS_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_BITMAPS_NUMBER)
			{
				if(hmi_bmp_dyn_xy_rect[hmi_object_id].y != (HMI_Y_STR)(hmi_object_data))
				{
					hmi_bmp_dyn_xy_rect[hmi_object_id].y = (HMI_Y_STR)(hmi_object_data);
					hmi_object_id = HMI_DXY_BITMAP_PRO2DXY_BITMAP(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_W_BITMAP(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_W_BITMAPS_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_BITMAPS_NUMBER)
			{
				if(hmi_bmp_dyn_xy_rect[hmi_object_id].w != (HMI_WIDTH_STR)(hmi_object_data))
				{
					hmi_bmp_dyn_xy_rect[hmi_object_id].w = (HMI_WIDTH_STR)(hmi_object_data);
					hmi_object_id = HMI_DXY_BITMAP_PRO2DXY_BITMAP(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_H_BITMAP(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_H_BITMAPS_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_BITMAPS_NUMBER)
			{
				if(hmi_bmp_dyn_xy_rect[hmi_object_id].h != (HMI_HEIGHT_STR)hmi_object_data)
				{
					hmi_bmp_dyn_xy_rect[hmi_object_id].h = (HMI_HEIGHT_STR)hmi_object_data;
					hmi_object_id = HMI_DXY_BITMAP_PRO2DXY_BITMAP(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||	\
			defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_YGV641)||	\
			defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||	\
			defined(HMI_GRAPHIC_YGV642)||defined(S6J3200_GRAPHIC))
		else if(HMI_IS_DYN_ALPHA_BITMAP(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_ALPHA_BITMAPS_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_BITMAPS_NUMBER)
			{
				if(hmi_bmp_dyn_xy_rect[hmi_object_id].alpha!= (UINT8)hmi_object_data)
				{
					hmi_bmp_dyn_xy_rect[hmi_object_id].alpha = (UINT8)hmi_object_data;
					hmi_object_id = HMI_DXY_BITMAP_PRO2DXY_BITMAP(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		#endif
		#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||	\
			defined(HMI_GRAPHIC_OPENGLES)||	defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||\
			defined(HMI_GRAPHIC_ST)||defined(S6J3200_GRAPHIC))
		else if(HMI_IS_DYN_ANGEL_BITMAP(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_ANGEL_BITMAPS_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_BITMAPS_NUMBER)
			{
				#if HMI_SUPPORT_FLOAT_ANGEL==HMI_YES
				value_angel	=HMI_U32_TO_F32(hmi_object_data);
				if(fabs(hmi_bmp_dyn_xy_rect[hmi_object_id].angel-value_angel)> HMI_FLOAT_TOLERANCE)
				#else
				if(hmi_bmp_dyn_xy_rect[hmi_object_id].angel != (hmi_object_data))
				#endif
				{
					#if HMI_SUPPORT_FLOAT_ANGEL==HMI_YES
					hmi_bmp_dyn_xy_rect[hmi_object_id].angel = value_angel;
					#else
					hmi_bmp_dyn_xy_rect[hmi_object_id].angel = (float_32)(hmi_object_data);
					#endif
					hmi_object_id = HMI_DXY_BITMAP_PRO2DXY_BITMAP(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		#endif
		#if (defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC))
		else if(HMI_IS_DYN_BLUR_BITMAP(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_BLUR_BITMAPS_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_BITMAPS_NUMBER)
			{
				if((hmi_bmp_dyn_xy_rect[hmi_object_id].attr & HMI_BLUR_FLAG)!= (UINT8)(hmi_object_data))
				{
					if(hmi_object_data==HMI_BLUR_FLAG)
					{
						hmi_bmp_dyn_xy_rect[hmi_object_id].attr |= (UINT8)HMI_BLUR_FLAG;
						hmi_object_id = HMI_DXY_BITMAP_PRO2DXY_BITMAP(hmi_object_id);
						HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
						HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
					}
					else
					{
						hmi_bmp_dyn_xy_rect[hmi_object_id].attr &= ((UINT8)(~HMI_BLUR_FLAG));
						hmi_object_id = HMI_DXY_BITMAP_PRO2DXY_BITMAP(hmi_object_id);
						HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
						HMI_GFX_SET_STATUS(HMI_SEND_EVENT);

					}
				}
			}
		}
		#endif
		#if (defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC))
		else if(HMI_IS_DYN_TRAIL_BITMAP(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_TRAIL_BITMAPS_INDEX(hmi_object_id);
			if(hmi_object_id >= HMI_DXY_CENTER_BITMAPS_NUMBER)
			{
				#if HMI_DXY_ROTATION_BITMAPS_NUMBER > 0
				hmi_object_id_index	= hmi_object_id - HMI_DXY_CENTER_BITMAPS_NUMBER;
				if(hmi_object_id_index < HMI_DXY_ROTATION_BITMAPS_NUMBER)
				{
					if((hmi_dxy_bitmap_rotation_trail_attr[hmi_object_id_index] & HMI_BMP_TRAIL) != (UINT8)(hmi_object_data))
					{
						if(hmi_object_data == HMI_ROT_IMAGE_TAIL_CW)
						{
							hmi_dxy_bitmap_rotation_trail_attr[hmi_object_id_index] |= (UINT8)HMI_ROT_IMAGE_TAIL_CW;
							hmi_object_id = HMI_DXY_BITMAP_PRO2DXY_BITMAP(hmi_object_id);
							HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
							HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
						}
						else
						{
							hmi_dxy_bitmap_rotation_trail_attr[hmi_object_id_index] &= ((UINT8)(~HMI_ROT_IMAGE_TAIL_CW));
							hmi_object_id = HMI_DXY_BITMAP_PRO2DXY_BITMAP(hmi_object_id);
							HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
							HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
						}
					}
				}
				#endif
			}
		}
#endif
		else
		{
			;
		}
	}
}
#endif
#if HMI_DXY_IMAGELIST_NUMBER >0U
void hmi_engine_set_dxy_imagelist(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data)
{ 
	if(HMI_IS_DYN_XY_IMGLIST_PROPERTY(hmi_object_id))
	{
		if(HMI_IS_DYN_X_IMAGELIST(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_X_IMAGELIST_INDEX(hmi_object_id);
			if(hmi_object_id<HMI_DXY_IMAGELIST_NUMBER)
			{
				if(hmi_dxy_imagelist_rect[hmi_object_id].x != (HMI_X_STR)hmi_object_data)
				{
					hmi_dxy_imagelist_rect[hmi_object_id].x  = (HMI_X_STR)hmi_object_data;
					hmi_object_id = HMI_DXY_IMGLIST_PRO2DXY_IMGLIST(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_Y_IMAGELIST(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_Y_IMAGELIST_INDEX(hmi_object_id);
			if(hmi_object_id<HMI_DXY_IMAGELIST_NUMBER)
			{
				if(hmi_dxy_imagelist_rect[hmi_object_id].y != (HMI_Y_STR)hmi_object_data)
				{
					hmi_dxy_imagelist_rect[hmi_object_id].y  = (HMI_Y_STR)hmi_object_data;
					hmi_object_id = HMI_DXY_IMGLIST_PRO2DXY_IMGLIST(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_W_IMAGELIST(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_W_IMAGELIST_INDEX(hmi_object_id);
			if(hmi_object_id<HMI_DXY_IMAGELIST_NUMBER)
			{
				if(hmi_dxy_imagelist_rect[hmi_object_id].w !=(HMI_WIDTH_STR) hmi_object_data)
				{
					hmi_dxy_imagelist_rect[hmi_object_id].w  = (HMI_WIDTH_STR)hmi_object_data;
					hmi_object_id = HMI_DXY_IMGLIST_PRO2DXY_IMGLIST(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_H_IMAGELIST(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_H_IMAGELIST_INDEX(hmi_object_id);
			if(hmi_object_id<HMI_DXY_IMAGELIST_NUMBER)
			{
				if(hmi_dxy_imagelist_rect[hmi_object_id].h != (HMI_HEIGHT_STR)hmi_object_data)
				{
					hmi_dxy_imagelist_rect[hmi_object_id].h  = (HMI_HEIGHT_STR)hmi_object_data;
					hmi_object_id = HMI_DXY_IMGLIST_PRO2DXY_IMGLIST(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_ALPHA_IMAGELIST(hmi_object_id))
		{
			#if	(defined(HMI_GRAPHIC_STGLIB)||defined(HMI_GRAPHIC_AGG)||	\
				defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_TWLIB)||		\
				defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||	\
				defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||	\
				defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))  	
			hmi_object_id = HMI_GET_DYN_ALPHA_IMAGELIST_INDEX(hmi_object_id);
			if(hmi_object_id<HMI_DXY_IMAGELIST_NUMBER)
			{
				if(hmi_dxy_imagelist_rect[hmi_object_id].alpha != (UINT8)hmi_object_data)
				{
					hmi_dxy_imagelist_rect[hmi_object_id].alpha  = (UINT8)hmi_object_data;
					hmi_object_id = HMI_DXY_IMGLIST_PRO2DXY_IMGLIST(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
			#endif
		}		
		else
		{
		}
	}
}
#endif
#if HMI_DXY_SCROLLBAR_NUMBER >0U
#if	(defined(HMI_GRAPHIC_STGLIB)||defined(HMI_GRAPHIC_AGG)||	\
	defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513)||		\
	defined(HMI_GRAPHIC_OPENGLES)|| defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||		\
	defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC)) 	
void hmi_engine_set_dxy_scrollbar(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data)
{ 
	if(HMI_IS_DYN_XY_SCROLLBAR_PROPERTY(hmi_object_id))
	{
		if(HMI_IS_DYN_X_SCROLLBAR(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_X_SCROLLBAR_INDEX(hmi_object_id);
			if(hmi_object_id<HMI_DXY_SCROLLBAR_NUMBER)
			{
				if(hmi_dxy_scrollbar_rect[hmi_object_id].x != (HMI_X_STR)hmi_object_data)
				{
					hmi_dxy_scrollbar_rect[hmi_object_id].x	= (HMI_X_STR)hmi_object_data;
					hmi_object_id=HMI_DXY_SCROLLBAR_PRO2DXY_SCROLLBAR(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_Y_SCROLLBAR(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_Y_SCROLLBAR_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_SCROLLBAR_NUMBER)
			{
				if(hmi_dxy_scrollbar_rect[hmi_object_id].y != (HMI_Y_STR)hmi_object_data)
				{
					hmi_dxy_scrollbar_rect[hmi_object_id].y	= (HMI_Y_STR)hmi_object_data;
					hmi_object_id = HMI_DXY_SCROLLBAR_PRO2DXY_SCROLLBAR(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_W_SCROLLBAR(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_W_SCROLLBAR_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_SCROLLBAR_NUMBER)
			{
				if(hmi_dxy_scrollbar_rect[hmi_object_id].w != (HMI_WIDTH_STR)hmi_object_data)
				{
					hmi_dxy_scrollbar_rect[hmi_object_id].w	= (HMI_WIDTH_STR)hmi_object_data;
					hmi_object_id = HMI_DXY_SCROLLBAR_PRO2DXY_SCROLLBAR(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_H_SCROLLBAR(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_H_SCROLLBAR_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_SCROLLBAR_NUMBER)
			{
				if(hmi_dxy_scrollbar_rect[hmi_object_id].h != (HMI_HEIGHT_STR)hmi_object_data)
				{
					hmi_dxy_scrollbar_rect[hmi_object_id].h	= (HMI_HEIGHT_STR)hmi_object_data;
					hmi_object_id = HMI_DXY_SCROLLBAR_PRO2DXY_SCROLLBAR(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_ALPHA_SCROLLBAR(hmi_object_id))
		{
			#if	(defined(HMI_GRAPHIC_STGLIB)||defined(HMI_GRAPHIC_AGG)||	\
				defined(HMI_GRAPHIC_OPENGLES)||	\
				defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_TWLIB)||defined(S6J3200_GRAPHIC))  	
			hmi_object_id = HMI_GET_DYN_ALPHA_SCROLLBAR_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_SCROLLBAR_NUMBER)
			{
				if(hmi_dxy_scrollbar_rect[hmi_object_id].alpha != (UINT8)hmi_object_data)
				{
					hmi_dxy_scrollbar_rect[hmi_object_id].alpha	= (UINT8)hmi_object_data;
					hmi_object_id = HMI_DXY_SCROLLBAR_PRO2DXY_SCROLLBAR(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
			#endif
		}
		else 
		{
		}
	}
}
#endif
#endif
#if HMI_DXY_BUTTON_NUMBER>0U
void hmi_engine_set_dxy_button(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data)
{
	if(HMI_IS_DYN_XY_BUTTON_PROPERTY(hmi_object_id))
		{
			if(HMI_IS_DYN_X_BUTTON(hmi_object_id))
			{
				hmi_object_id = HMI_GET_DYN_X_BUTTON_INDEX(hmi_object_id);
				if(hmi_object_id < HMI_DXY_BUTTON_NUMBER)
				{
					if(hmi_dxy_button_rect[hmi_object_id].x != (HMI_X_STR)hmi_object_data)
					{
						hmi_dxy_button_rect[hmi_object_id].x = (HMI_X_STR)hmi_object_data;
						hmi_object_id = HMI_DXY_BUTTON_PRO2DXY_BUTTON(hmi_object_id);
						HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
						HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
					}
				}
			}
			else if(HMI_IS_DYN_Y_BUTTON(hmi_object_id))
			{
				hmi_object_id = HMI_GET_DYN_Y_BUTTON_INDEX(hmi_object_id);
				if(hmi_object_id < HMI_DXY_BUTTON_NUMBER)
				{
					if(hmi_dxy_button_rect[hmi_object_id].y != (HMI_Y_STR)hmi_object_data)
					{
						hmi_dxy_button_rect[hmi_object_id].y = (HMI_Y_STR)hmi_object_data;
						hmi_object_id = HMI_DXY_BUTTON_PRO2DXY_BUTTON(hmi_object_id);
						HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
						HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
					}
				}
			}
			else if(HMI_IS_DYN_W_BUTTON(hmi_object_id))
			{
				hmi_object_id = HMI_GET_DYN_W_BUTTON_INDEX(hmi_object_id);
				if(hmi_object_id < HMI_DXY_BUTTON_NUMBER)
				{
					if(hmi_dxy_button_rect[hmi_object_id].w != (HMI_WIDTH_STR)hmi_object_data)
					{
						hmi_dxy_button_rect[hmi_object_id].w = (HMI_WIDTH_STR)hmi_object_data;
						hmi_object_id = HMI_DXY_BUTTON_PRO2DXY_BUTTON(hmi_object_id);
						HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
						HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
					}
				}
			}
			else if(HMI_IS_DYN_H_BUTTON(hmi_object_id))
			{
				hmi_object_id = HMI_GET_DYN_H_BUTTON_INDEX(hmi_object_id);
				if(hmi_object_id < HMI_DXY_BUTTON_NUMBER)
				{
					if(hmi_dxy_button_rect[hmi_object_id].h != (HMI_HEIGHT_STR)hmi_object_data)
					{
						hmi_dxy_button_rect[hmi_object_id].h = (HMI_HEIGHT_STR)hmi_object_data;
						hmi_object_id = HMI_DXY_BUTTON_PRO2DXY_BUTTON(hmi_object_id);
						HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
						HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
					}
				}
			}
			else if(HMI_IS_DYN_ALPHA_BUTTON(hmi_object_id))
			{
				#if	(defined(HMI_GRAPHIC_STGLIB)||defined(HMI_GRAPHIC_AGG)||	\
					defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_TWLIB)||		\
					defined(HMI_GRAPHIC_OPENGLES)||	defined(S6J3200_GRAPHIC)||	\
					defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||	\
					defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642)) 	
				hmi_object_id = HMI_GET_DYN_ALPHA_BUTTON_INDEX(hmi_object_id);
				if(hmi_object_id < HMI_DXY_BUTTON_NUMBER)
				{
					if(hmi_dxy_button_rect[hmi_object_id].alpha != (UINT8)hmi_object_data)
					{
						hmi_dxy_button_rect[hmi_object_id].alpha = (UINT8)hmi_object_data;
						hmi_object_id = HMI_DXY_BUTTON_PRO2DXY_BUTTON(hmi_object_id);
						HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
						HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
					}
				}
				#endif
			}
			else 
			{
			}
		}

}
#endif
#if HMI_DXY_PAGES_NUMBER>0U
void hmi_engine_set_dyn_pages(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data)
{ 
	if(HMI_IS_DYN_XY_PAGE_PROPERTY(hmi_object_id))
	{
		if(HMI_IS_DYN_X_PAGES(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_X_PAGES_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_PAGES_NUMBER)
			{
				if(hmi_dxy_page_rect[hmi_object_id].x != (HMI_X_STR)hmi_object_data)
				{
					hmi_dxy_page_rect[hmi_object_id].x  = (HMI_X_STR)hmi_object_data;
					hmi_object_id = HMI_DXY_PAGE_PRO2DXY_PAGE(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_Y_PAGES(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_Y_PAGES_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_PAGES_NUMBER)
			{
				if(hmi_dxy_page_rect[hmi_object_id].y != (HMI_Y_STR)hmi_object_data)
				{
					hmi_dxy_page_rect[hmi_object_id].y  = (HMI_Y_STR)hmi_object_data;
					hmi_object_id = HMI_DXY_PAGE_PRO2DXY_PAGE(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_W_PAGES(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_W_PAGES_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_PAGES_NUMBER)
			{
				if(hmi_dxy_page_rect[hmi_object_id].w != (HMI_WIDTH_STR)hmi_object_data)
				{
					hmi_dxy_page_rect[hmi_object_id].w  =(HMI_WIDTH_STR) hmi_object_data;
					hmi_object_id = HMI_DXY_PAGE_PRO2DXY_PAGE(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_H_PAGES(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_H_PAGES_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_PAGES_NUMBER)
			{
				if(hmi_dxy_page_rect[hmi_object_id].h != (HMI_HEIGHT_STR)hmi_object_data)
				{
					hmi_dxy_page_rect[hmi_object_id].h  = (HMI_HEIGHT_STR)hmi_object_data;
					hmi_object_id = HMI_DXY_PAGE_PRO2DXY_PAGE(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		#if (defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC))
		else if(HMI_IS_DYN_ALPHA_PAGES(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_ALPHA_PAGES_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_PAGES_NUMBER)
			{
				if(hmi_dxy_page_rect[hmi_object_id].alpha != (UINT8)hmi_object_data)
				{
					hmi_dxy_page_rect[hmi_object_id].alpha  = (UINT8)hmi_object_data;
					hmi_object_id = HMI_DXY_PAGE_PRO2DXY_PAGE(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					#if (defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC))
					/*page alpha change not draw page*/
					HMI_GFX_SET_STATUS(HMI_RGL_PROP_EVENT);
					#else
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
					#endif
				}
			}
		}
		#endif		
		else
		{
		}
	}
}
#endif

#if HMI_DYN_XY_EDIT_TEXTS_NUMBER > 0U
void hmi_engine_set_dxy_edit_texts(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data)
{ 
	if(HMI_IS_DYN_XY_DTEXT_PROPERTY(hmi_object_id))
	{
		if(HMI_IS_DYN_X_DTEXT(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_X_DTEXT_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DYN_XY_EDIT_TEXTS_NUMBER)
			{
				if((HMI_X_STR)(hmi_object_data) != hmi_dyn_xy_edit_text_prop_table[hmi_object_id].text_rect.x)
				{
					hmi_dyn_xy_edit_text_prop_table[hmi_object_id].text_rect.x = (HMI_X_STR)(hmi_object_data); 
					hmi_object_id = HMI_DXY_DTEXT_PRO2DXY_DTEXT(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_Y_DTEXT(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_Y_DTEXT_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DYN_XY_EDIT_TEXTS_NUMBER)
			{
				if((HMI_Y_STR)(hmi_object_data) != hmi_dyn_xy_edit_text_prop_table[hmi_object_id].text_rect.y)
				{
					hmi_dyn_xy_edit_text_prop_table[hmi_object_id].text_rect.y = (HMI_Y_STR)(hmi_object_data); 
					hmi_object_id = HMI_DXY_DTEXT_PRO2DXY_DTEXT(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_W_DTEXT(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_W_DTEXT_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DYN_XY_EDIT_TEXTS_NUMBER)
			{
				if((HMI_WIDTH_STR)hmi_object_data != hmi_dyn_xy_edit_text_prop_table[hmi_object_id].text_rect.w)
				{
					hmi_dyn_xy_edit_text_prop_table[hmi_object_id].text_rect.w = (HMI_WIDTH_STR)hmi_object_data; 
					hmi_object_id = HMI_DXY_DTEXT_PRO2DXY_DTEXT(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_H_DTEXT(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_H_DTEXT_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DYN_XY_EDIT_TEXTS_NUMBER)
			{
				if((HMI_HEIGHT_STR)hmi_object_data != hmi_dyn_xy_edit_text_prop_table[hmi_object_id].text_rect.h)
				{
					hmi_dyn_xy_edit_text_prop_table[hmi_object_id].text_rect.h = (HMI_HEIGHT_STR)hmi_object_data; 
					hmi_object_id = HMI_DXY_DTEXT_PRO2DXY_DTEXT(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_DCOLOR_DTEXT(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_DCOLOR_DTEXT_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DYN_XY_EDIT_TEXTS_NUMBER)
			{
				if((HMI_COLOR_STR)(hmi_object_data) != hmi_dyn_xy_edit_text_prop_table[hmi_object_id].color)
				{
					hmi_dyn_xy_edit_text_prop_table[hmi_object_id].color = (HMI_COLOR_STR)(hmi_object_data); 
					hmi_object_id = HMI_DXY_DTEXT_PRO2DXY_DTEXT(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}		
		else
		{
		}
	}
}
#endif


#if HMI_UNEDIT_TEXTS_DYN_XY_NUMBER > 0U
void hmi_engine_set_dxy_unedit_texts(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data)
{ 
	if(HMI_IS_DYN_XY_STEXT_PROPERTY(hmi_object_id))
	{
		if(HMI_IS_DYN_X_UNEDIT(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_X_STEXT_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_UNEDIT_TEXTS_DYN_XY_NUMBER)
			{
				if((HMI_X_STR)(hmi_object_data) != hmi_dyn_xy_unedit_text_prop_table[hmi_object_id].text_rect.x)
				{
					hmi_dyn_xy_unedit_text_prop_table[hmi_object_id].text_rect.x = (HMI_X_STR)(hmi_object_data); 
					hmi_object_id = HMI_DXY_STEXT_PRO2DXY_STEXT(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_Y_UNEDIT(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_Y_STEXT_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_UNEDIT_TEXTS_DYN_XY_NUMBER)
			{
				if((HMI_Y_STR)(hmi_object_data) != hmi_dyn_xy_unedit_text_prop_table[hmi_object_id].text_rect.y)
				{
					hmi_dyn_xy_unedit_text_prop_table[hmi_object_id].text_rect.y = (HMI_Y_STR)(hmi_object_data); 
					hmi_object_id = HMI_DXY_STEXT_PRO2DXY_STEXT(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_W_UNEDIT(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_W_STEXT_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_UNEDIT_TEXTS_DYN_XY_NUMBER)
			{
				if((HMI_WIDTH_STR)hmi_object_data != hmi_dyn_xy_unedit_text_prop_table[hmi_object_id].text_rect.w)
				{
					hmi_dyn_xy_unedit_text_prop_table[hmi_object_id].text_rect.w = (HMI_WIDTH_STR)hmi_object_data; 
					hmi_object_id = HMI_DXY_STEXT_PRO2DXY_STEXT(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_H_UNEDIT(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_H_STEXT_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_UNEDIT_TEXTS_DYN_XY_NUMBER)
			{
				if((HMI_HEIGHT_STR)hmi_object_data != hmi_dyn_xy_unedit_text_prop_table[hmi_object_id].text_rect.h)
				{
					hmi_dyn_xy_unedit_text_prop_table[hmi_object_id].text_rect.h = (HMI_HEIGHT_STR)hmi_object_data; 
					hmi_object_id = HMI_DXY_STEXT_PRO2DXY_STEXT(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_COLOR_UNEDIT(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_DCOLOR_STEXT_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_UNEDIT_TEXTS_DYN_XY_NUMBER)
			{
				if((HMI_COLOR_STR)(hmi_object_data) != hmi_dyn_xy_unedit_text_prop_table[hmi_object_id].color)
				{
					hmi_dyn_xy_unedit_text_prop_table[hmi_object_id].color = (HMI_COLOR_STR)(hmi_object_data); 
					hmi_object_id = HMI_DXY_STEXT_PRO2DXY_STEXT(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}		
		else
		{
		}
	}
}
#endif


#if HMI_DYN_FILL_PAGES_NUMBER > 0U
void hmi_engine_set_dyn_fill_pages(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data)
{ 
	if(HMI_IS_DYN_XY_FILL_PROPERTY(hmi_object_id))
	{
		if(HMI_IS_DYN_X_NFILL(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_X_NFILL_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DYN_FILL_PAGES_NUMBER)
			{
				if((HMI_X_STR)(hmi_object_data) != hmi_fills_dyn_xy_rect[hmi_object_id].x)
				{
					hmi_fills_dyn_xy_rect[hmi_object_id].x = (HMI_X_STR)(hmi_object_data);
					hmi_object_id = HMI_DXY_FILL_PRO2DXY_FILL(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_Y_NFILL(hmi_object_id)/*hmi_object_id < HMI_DYN_Y_NFILL_MAX_ID*/)
		{
			hmi_object_id = HMI_GET_DYN_Y_NFILL_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DYN_FILL_PAGES_NUMBER)
			{
				if((HMI_Y_STR)(hmi_object_data) != hmi_fills_dyn_xy_rect[hmi_object_id].y)
				{
					hmi_fills_dyn_xy_rect[hmi_object_id].y = (HMI_Y_STR)(hmi_object_data);
					hmi_object_id = HMI_DXY_FILL_PRO2DXY_FILL(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_W_NFILL(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_W_NFILL_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DYN_FILL_PAGES_NUMBER)
			{
				if((HMI_WIDTH_STR)(hmi_object_data) != hmi_fills_dyn_xy_rect[hmi_object_id].w)
				{
					hmi_fills_dyn_xy_rect[hmi_object_id].w = (HMI_WIDTH_STR)(hmi_object_data); 
					hmi_object_id = HMI_DXY_FILL_PRO2DXY_FILL(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}	
		else if(HMI_IS_DYN_H_NFILL(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_H_NFILL_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DYN_FILL_PAGES_NUMBER)
			{
				if((HMI_HEIGHT_STR)(hmi_object_data) != hmi_fills_dyn_xy_rect[hmi_object_id].h)
				{
					hmi_fills_dyn_xy_rect[hmi_object_id].h = (HMI_HEIGHT_STR)(hmi_object_data); 
					hmi_object_id = HMI_DXY_FILL_PRO2DXY_FILL(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_COLOR_NFILL(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_COLOR_NFILL_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DYN_FILL_PAGES_NUMBER)
			{
				if((HMI_COLOR_STR)(hmi_object_data) != hmi_fills_dyn_prop_table[hmi_object_id].color)
				{
					hmi_fills_dyn_prop_table[hmi_object_id].color = (HMI_COLOR_STR)(hmi_object_data); 
					hmi_object_id = HMI_DXY_FILL_PRO2DXY_FILL(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_Z_NFILL(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_Z_NFILL_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DYN_FILL_PAGES_NUMBER)
			{
				if((HMI_Z_STR)(hmi_object_data) != hmi_fills_dyn_xy_rect[hmi_object_id].z)
				{
					hmi_fills_dyn_xy_rect[hmi_object_id].z= (HMI_Z_STR)(hmi_object_data); 
					hmi_object_id = HMI_DXY_FILL_PRO2DXY_FILL(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else
		{
		}
	}
}
#endif

#if HMI_DYN_GFILL_NUMBER > 0U
#if	(defined(HMI_GRAPHIC_STGLIB)||defined(HMI_GRAPHIC_AGG)||	\
	defined(HMI_GRAPHIC_OPENGLES)||	\
	defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513)||		\
	defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(S6J3200_GRAPHIC)) 	
void hmi_engine_set_dyn_gfill_pages(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data)
{ 
	if(HMI_IS_DYN_XY_GFILL_PROPERTY(hmi_object_id))
	{
		if(HMI_IS_DYN_X_GFILL(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_X_GFILL_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DYN_GFILL_NUMBER)
			{
				if((HMI_X_STR)(hmi_object_data) != hmi_gradient_dxy_fill_rect[hmi_object_id].x)
				{
					hmi_gradient_dxy_fill_rect[hmi_object_id].x = (HMI_X_STR)(hmi_object_data); 
					hmi_object_id = HMI_DXY_GFILL_PRO2DXY_GFILL(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_Y_GFILL(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_Y_GFILL_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DYN_GFILL_NUMBER)
			{
				if((HMI_Y_STR)(hmi_object_data) != hmi_gradient_dxy_fill_rect[hmi_object_id].y)
				{
					hmi_gradient_dxy_fill_rect[hmi_object_id].y = (HMI_Y_STR)(hmi_object_data); 
					hmi_object_id = HMI_DXY_GFILL_PRO2DXY_GFILL(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_W_GFILL(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_W_GFILL_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DYN_GFILL_NUMBER)
			{
				if((HMI_WIDTH_STR)(hmi_object_data) != hmi_gradient_dxy_fill_rect[hmi_object_id].w)
				{
					hmi_gradient_dxy_fill_rect[hmi_object_id].w = (HMI_WIDTH_STR)(hmi_object_data); 
					hmi_object_id = HMI_DXY_GFILL_PRO2DXY_GFILL(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}	
		else if(HMI_IS_DYN_H_GFILL(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_H_GFILL_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DYN_GFILL_NUMBER)
			{
				if((HMI_HEIGHT_STR)(hmi_object_data) != hmi_gradient_dxy_fill_rect[hmi_object_id].h)
				{
					hmi_gradient_dxy_fill_rect[hmi_object_id].h = (HMI_HEIGHT_STR)(hmi_object_data); 
					hmi_object_id = HMI_DXY_GFILL_PRO2DXY_GFILL(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_C1_GFILL(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_C1_GFILL_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DYN_GFILL_NUMBER)
			{
				if((HMI_COLOR_STR)(hmi_object_data) != hmi_gradient_dxy_fill_table[hmi_object_id].color1)
				{
					hmi_gradient_dxy_fill_table[hmi_object_id].color1= (HMI_COLOR_STR)(hmi_object_data); 
					hmi_object_id = HMI_DXY_GFILL_PRO2DXY_GFILL(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_C2_GFILL(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_C2_GFILL_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DYN_GFILL_NUMBER)
			{
				if((HMI_COLOR_STR)(hmi_object_data) != hmi_gradient_dxy_fill_table[hmi_object_id].color2)
				{
					hmi_gradient_dxy_fill_table[hmi_object_id].color2= (HMI_COLOR_STR)(hmi_object_data); 
					hmi_object_id = HMI_DXY_GFILL_PRO2DXY_GFILL(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_Z_GFILL(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_Z_GFILL_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DYN_GFILL_NUMBER)
			{
				if((HMI_Z_STR)(hmi_object_data) != hmi_gradient_dxy_fill_rect[hmi_object_id].z)
				{
					hmi_gradient_dxy_fill_rect[hmi_object_id].z= (HMI_Z_STR)(hmi_object_data); 
					hmi_object_id = HMI_DXY_GFILL_PRO2DXY_GFILL(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else 
		{
		}
	}
}
#endif
#endif



#if HMI_SXY_ONE_POINT_SPLINE_NUMBER +HMI_DXY_ONE_POINT_SPLINE_NUMBER > 0

void one_point_overflow(SPOINT_TP * ppoint_array,POINT_RANGE_TP CONST *ppoint_range,
						POINT_FIFO_TP * phead_tail_range)
{
	UINT16 begin	= 0U;
	UINT16 end		= 0U;
	UINT16 loop		= 0U;
	UINT16 head		= 0U;
	UINT16 tail		= 0U;
	SINT16 offset	= 0U;

	if((ppoint_array != NULL)&&(ppoint_range != NULL)&&
		(phead_tail_range != NULL))
	{
		begin	= ppoint_range->begin;
		end		= ppoint_range->end;
		head	= phead_tail_range->head;
		tail	= phead_tail_range->tail;
		offset	= phead_tail_range->offset;
		
		if(head < tail)
		{
			for(loop = head;loop < tail;loop++)
			{
				ppoint_array[loop].x	+= offset;
			}
		}
		else
		{
			for(loop = begin;loop < tail;loop++)
			{
				ppoint_array[loop].x	+= offset;
			}

			for(loop = head;loop < end;loop++)
			{
				ppoint_array[loop].x	+= offset;
			}
		}
		phead_tail_range->offset	= 0;
	}
}


void hmi_spline_set_one_point(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data)
{
	SINT32	point_x		= 0;
	SINT16	data_x		= 0;
	SINT32	point_y		= 0;
	U16		tail		= 0U;/*2021 8 31 lq*/
	U16		pre_tail	= 0U;/*2021 8 31 lq*/
	BOOLEAN overflow	= FALSE;
	HMI_OBJECT_ID_STR	hmi_object_index 	= 0U;
	HMI_RECT_STR CONST	*phmi_spline_rect	= NULL;

	
	#if HMI_DXY_ONE_POINT_SPLINE_NUMBER > 0
	if(HMI_IS_DXY_ONE_SPLINE(hmi_object_id))
	{
		hmi_object_index	= HMI_GET_DXY_SPLINE_INDEX(hmi_object_id);	
		phmi_spline_rect	= (HMI_RECT_STR CONST	*)(&hmi_dxy_spline_rect[hmi_object_index]);
		hmi_object_index	= HMI_GET_DXY_ONE_SPLINE_INDEX(hmi_object_id);
		
		if(hmi_object_index < HMI_DXY_ONE_POINT_SPLINE_NUMBER)
		{
			#if 1
			if(HMI_CLEAR_ONE_POINT_FIFO == hmi_object_data)
			{
				dxy_spline_head_tail_offset_array[hmi_object_index].offset	= 0;
				dxy_spline_head_tail_offset_array[hmi_object_index].head	= dxy_one_point_range_array[hmi_object_index].begin;
				dxy_spline_head_tail_offset_array[hmi_object_index].tail	= dxy_one_point_range_array[hmi_object_index].begin;
			}
			else
			{
				if((dxy_one_point_range_array[hmi_object_index].begin+1) < 
					dxy_one_point_range_array[hmi_object_index].end)/*full*/
				{
					point_x	= (SINT16)HMI_U32_LOW16(hmi_object_data);
					data_x	= point_x;
					point_y	= (SINT16)(HMI_U32_HIG16(hmi_object_data));
					
					tail		= (dxy_spline_head_tail_offset_array[hmi_object_index].tail);
					point_x 	-= dxy_spline_head_tail_offset_array[hmi_object_index].offset;

				if(point_x <= HMI_MIN_INT16)
				{
					overflow	= TRUE;
				}
				else if(point_x >= HMI_MAX_INT16)
				{
					overflow	= TRUE;
				}
				else
				{
				}
				
				if(overflow == TRUE)
				{
					one_point_overflow(dxy_one_point_array,
										&(dxy_one_point_range_array[hmi_object_index]),
										&(dxy_spline_head_tail_offset_array[hmi_object_index]));
					point_x = data_x;
				}
				
				if(tail > dxy_one_point_range_array[hmi_object_index].begin)
				{
					pre_tail	= tail -1;
				}
				else
				{
					if(dxy_one_point_range_array[hmi_object_index].end >1)
					{
						pre_tail	= (dxy_one_point_range_array[hmi_object_index].end -1);
					}
					else
					{
						pre_tail	= (dxy_one_point_range_array[hmi_object_index].begin);
					}
				}

				

				if((point_x != dxy_one_point_array[pre_tail].x)||(point_y != dxy_one_point_array[pre_tail].y))
				{
					if(data_x > phmi_spline_rect->w)
					{ 
						dxy_spline_head_tail_offset_array[hmi_object_index].offset -= ( data_x - phmi_spline_rect->w);
					}
					else if(data_x < 0)
					{
						dxy_spline_head_tail_offset_array[hmi_object_index].offset += (-data_x);
					}
					else
					{
					}
				
					dxy_one_point_array[tail].x = point_x;
					dxy_one_point_array[tail].y = point_y;
					tail++;
					if(tail >= dxy_one_point_range_array[hmi_object_index].end)/*full*/
					{
						tail	= (dxy_one_point_range_array[hmi_object_index].begin);
					}
					dxy_spline_head_tail_offset_array[hmi_object_index].tail = tail;

					if(dxy_spline_head_tail_offset_array[hmi_object_index].head == tail)/*not full*/
					{
						dxy_spline_head_tail_offset_array[hmi_object_index].head++;
						if(dxy_spline_head_tail_offset_array[hmi_object_index].head >= dxy_one_point_range_array[hmi_object_index].end)/*full*/
						{
							dxy_spline_head_tail_offset_array[hmi_object_index].head = dxy_one_point_range_array[hmi_object_index].begin;
						}
					}
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
					}
				}
			}
			#endif
		}
	}
	else 
	#endif
	#if HMI_SXY_ONE_POINT_SPLINE_NUMBER > 0
	if(HMI_IS_SXY_ONE_SPLINE(hmi_object_id))
	{
		hmi_object_index	= HMI_GET_SXY_SPLINE_INDEX(hmi_object_id);	
		phmi_spline_rect	= &hmi_sxy_spline_rect[hmi_object_index];
		hmi_object_index	= HMI_GET_SXY_ONE_SPLINE_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_SXY_ONE_POINT_SPLINE_NUMBER)
		{
			if(HMI_CLEAR_ONE_POINT_FIFO == hmi_object_data)
			{
				sxy_spline_head_tail_offset_array[hmi_object_index].offset	= 0;
				sxy_spline_head_tail_offset_array[hmi_object_index].head	= sxy_one_point_range_array[hmi_object_index].begin;
				sxy_spline_head_tail_offset_array[hmi_object_index].tail	= sxy_one_point_range_array[hmi_object_index].begin;
			}
			else
			{
			if((sxy_one_point_range_array[hmi_object_index].begin+1) < sxy_one_point_range_array[hmi_object_index].end)/*full*/
			{
				point_x	= (SINT16)HMI_U32_LOW16(hmi_object_data);
				data_x	= point_x;
				point_y	= (SINT16)(HMI_U32_HIG16(hmi_object_data));
				
				point_x -= sxy_spline_head_tail_offset_array[hmi_object_index].offset;
				if(point_x <= HMI_MIN_INT16)
				{
					overflow	= TRUE;
				}
				else if(point_x >= HMI_MAX_INT16)
				{
					overflow	= TRUE;
				}
				else
				{
				}
				
				if(overflow == TRUE)
				{
					one_point_overflow(sxy_one_point_array,
										&(sxy_one_point_range_array[hmi_object_index]),
										&(sxy_spline_head_tail_offset_array[hmi_object_index]));
					point_x = data_x;
				}
				
				tail	= sxy_spline_head_tail_offset_array[hmi_object_index].tail;
				if(tail > sxy_one_point_range_array[hmi_object_index].begin)
				{
					pre_tail	= tail -1;
				}
				else
				{
					if(sxy_one_point_range_array[hmi_object_index].end >1)
					{
						pre_tail	= sxy_one_point_range_array[hmi_object_index].end -1;
					}
					else
					{
						pre_tail	= sxy_one_point_range_array[hmi_object_index].begin;
					}
				}

				if((point_x != sxy_one_point_array[pre_tail].x)||
					(point_y != sxy_one_point_array[pre_tail].y))
				{
					if(data_x > phmi_spline_rect->w)
					{ 
						sxy_spline_head_tail_offset_array[hmi_object_index].offset -= ( data_x - phmi_spline_rect->w);
					}
					else if(data_x < 0)
					{
						sxy_spline_head_tail_offset_array[hmi_object_index].offset += (-data_x);
					}
					else
					{
					}
					sxy_one_point_array[tail].x = point_x;
					sxy_one_point_array[tail].y = point_y;
					tail++;
					if(tail >= sxy_one_point_range_array[hmi_object_index].end)/*full*/
					{
						tail	= sxy_one_point_range_array[hmi_object_index].begin;
					}
					sxy_spline_head_tail_offset_array[hmi_object_index].tail = tail;

					if(sxy_spline_head_tail_offset_array[hmi_object_index].head == tail)/*not full*/
					{
						sxy_spline_head_tail_offset_array[hmi_object_index].head++;
						if(sxy_spline_head_tail_offset_array[hmi_object_index].head >= sxy_one_point_range_array[hmi_object_index].end)/*full*/
						{
							sxy_spline_head_tail_offset_array[hmi_object_index].head = sxy_one_point_range_array[hmi_object_index].begin;
							}
						}
					}
				}
			}
			HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
			HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
		}
	}
	else
	#endif
	{
	}
}
#endif
#define HMI_CUSTOM_INDEX_ERROR 0xff
#if HMI_SXY_CUSTOM_CNT > 0U
BOOLEAN hmi_engine_get_static_custom_index(HMI_OBJECT_ID_STR object_index,
													HMI_OBJECT_ID_STR	*pcustom_index)
{
	HMI_OBJECT_ID_STR index =0;
	BOOLEAN				search_success =FALSE;
	*pcustom_index = HMI_CUSTOM_INDEX_ERROR;
	while((index < HMI_SXY_CUSTOM_CNT)&&( search_success == FALSE))
	{
		if((hmi_sxy_custom_widget_info[index].begin <= object_index )&&
			(hmi_sxy_custom_widget_info[index].end >object_index ))
		{
			search_success  =  TRUE;
			*pcustom_index	= index;
		}
		index++;
	}
	return search_success;
}
#endif

#if HMI_DXY_CUSTOM_CNT > 0U
BOOLEAN hmi_engine_get_dyn_custom_index(HMI_OBJECT_ID_STR object_index,
													HMI_OBJECT_ID_STR	*pcustom_index)
{
	HMI_OBJECT_ID_STR index =0;
	BOOLEAN				search_success =FALSE;
	*pcustom_index = HMI_CUSTOM_INDEX_ERROR;
	while((index < HMI_DXY_CUSTOM_CNT)&&( search_success == FALSE))
	{
		if((hmi_dxy_custom_widget_info[index].begin <= object_index )&&
			(hmi_dxy_custom_widget_info[index].end >object_index ))
		{
			search_success  =  TRUE;
			*pcustom_index	= index;
		}
		index++;
	}
	return search_success;
}
void hmi_engine_set_dyn_custom(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data)
{ 
	HMI_OBJECT_ID_STR   custom_index	= 0;
	HMI_OBJECT_ID_STR	object_index	= 0;
	BOOLEAN				search_success	= FALSE;
	HMI_CUSTOM_P_INT*	pcustom_p_int	= NULL;
	HMI_CUSTOM_P_FLOAT*	pcustom_p_float	= NULL;
	float_32			hmi_object_data_f =0.0f;
	HMI_CUSTOM_XYZWH_FLOAT_STR	*pcustom_xyz_f	= NULL;
	HMI_CUSTOM_XYZWH_STR		*pcustom_xyz	= NULL;
	
	if(HMI_IS_DYN_XY_CUSTOM_PROPERTY(hmi_object_id))
	{
		if(HMI_IS_DYN_X_CUSTOM(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_X_CUSTOM_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_CUSTOM_CNT)
			{
				search_success =hmi_engine_get_dyn_custom_index(hmi_object_id,&custom_index);
				if(search_success == TRUE)
				{
					object_index	= hmi_object_id -hmi_dxy_custom_widget_info[custom_index].begin;
					if((hmi_dxy_custom_widget_info[custom_index].attr_fun.attr & HMI_XYZWH_F) == 0U)	/*2023 07 10.Custom x,y,z,w,h is float*/
					{
						pcustom_xyz	= (HMI_CUSTOM_XYZWH_STR *)hmi_dxy_custom_widget_info[custom_index].pxyzwh;
						if(pcustom_xyz[object_index].x != (SINT16)hmi_object_data)
						{
							pcustom_xyz[object_index].x = (SINT16)hmi_object_data;
							hmi_object_id = HMI_DXY_CUSTOM_PRO2DXY_CUSTOM(hmi_object_id);
							HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
							HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
						}
					}
					else 
					{
						pcustom_xyz_f	= (HMI_CUSTOM_XYZWH_FLOAT_STR *)hmi_dxy_custom_widget_info[custom_index].pxyzwh;						
						hmi_object_data_f	= HMI_U32_TO_F32(hmi_object_data);
						if(pcustom_xyz_f[object_index].x != hmi_object_data_f)
						{
							pcustom_xyz_f[object_index].x = hmi_object_data_f;
							hmi_object_id = HMI_DXY_CUSTOM_PRO2DXY_CUSTOM(hmi_object_id);
							HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
							HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
						}
					}
				}
			}
		}
		else if(HMI_IS_DYN_Y_CUSTOM(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_Y_CUSTOM_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_CUSTOM_CNT)
			{
				search_success =hmi_engine_get_dyn_custom_index(hmi_object_id,&custom_index);
				if(search_success == TRUE)
				{
					object_index	= hmi_object_id -hmi_dxy_custom_widget_info[custom_index].begin;
					if((hmi_dxy_custom_widget_info[custom_index].attr_fun.attr & HMI_XYZWH_F )== 0U)	/*2023 07 10.Custom x,y,z,w,h is float*/
					{
						pcustom_xyz	= (HMI_CUSTOM_XYZWH_STR *)hmi_dxy_custom_widget_info[custom_index].pxyzwh;
						if(pcustom_xyz[object_index].y != (SINT16)hmi_object_data)
						{
							pcustom_xyz[object_index].y = (SINT16)hmi_object_data;
							hmi_object_id = HMI_DXY_CUSTOM_PRO2DXY_CUSTOM(hmi_object_id);
							HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
							HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
						}
					}
					else
					{
						pcustom_xyz_f	= (HMI_CUSTOM_XYZWH_FLOAT_STR *)hmi_dxy_custom_widget_info[custom_index].pxyzwh;						
						hmi_object_data_f	= HMI_U32_TO_F32(hmi_object_data);
						if(pcustom_xyz_f[object_index].y != hmi_object_data_f)
						{
							pcustom_xyz_f[object_index].y = hmi_object_data_f;
							hmi_object_id = HMI_DXY_CUSTOM_PRO2DXY_CUSTOM(hmi_object_id);
							HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
							HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
						}
					}

				}
			}
		}
		else if(HMI_IS_DYN_Z_CUSTOM(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_Z_CUSTOM_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_CUSTOM_CNT)
			{
				search_success =hmi_engine_get_dyn_custom_index(hmi_object_id,&custom_index);
				if(search_success == TRUE)
				{
					object_index	= hmi_object_id -hmi_dxy_custom_widget_info[custom_index].begin;
					if((hmi_dxy_custom_widget_info[custom_index].attr_fun.attr & HMI_XYZWH_F) == 0U)	/*2023 07 10.Custom x,y,z,w,h is float*/
					{
						pcustom_xyz	= (HMI_CUSTOM_XYZWH_STR *)hmi_dxy_custom_widget_info[custom_index].pxyzwh;
						if(pcustom_xyz[object_index].z != (SINT16)hmi_object_data)
						{
							pcustom_xyz[object_index].z= (SINT16)hmi_object_data;
							hmi_object_id = HMI_DXY_CUSTOM_PRO2DXY_CUSTOM(hmi_object_id);
							HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
							HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
						}
					}
					else
					{
						pcustom_xyz_f	= (HMI_CUSTOM_XYZWH_FLOAT_STR *)hmi_dxy_custom_widget_info[custom_index].pxyzwh;						
						hmi_object_data_f	= HMI_U32_TO_F32(hmi_object_data);
						if(pcustom_xyz_f[object_index].z != hmi_object_data_f)
						{
							pcustom_xyz_f[object_index].z = hmi_object_data_f;
							hmi_object_id = HMI_DXY_CUSTOM_PRO2DXY_CUSTOM(hmi_object_id);
							HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
							HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
						}
					}
				}
			}
		}
		else if(HMI_IS_DYN_W_CUSTOM(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_W_CUSTOM_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_CUSTOM_CNT)
			{
				search_success =hmi_engine_get_dyn_custom_index(hmi_object_id,&custom_index);
				if(search_success == TRUE)
				{
					object_index	= hmi_object_id -hmi_dxy_custom_widget_info[custom_index].begin;
					if((hmi_dxy_custom_widget_info[custom_index].attr_fun.attr & HMI_XYZWH_F) == 0U)	/*2023 07 10.Custom x,y,z,w,h is float*/
					{
						pcustom_xyz	= (HMI_CUSTOM_XYZWH_STR *)hmi_dxy_custom_widget_info[custom_index].pxyzwh;
						if(pcustom_xyz[object_index].w != (U16)hmi_object_data)
						{
							pcustom_xyz[object_index].w = (U16)hmi_object_data;
							hmi_object_id = HMI_DXY_CUSTOM_PRO2DXY_CUSTOM(hmi_object_id);
							HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
							HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
						}
					}
					else
					{
						pcustom_xyz_f	= (HMI_CUSTOM_XYZWH_FLOAT_STR *)hmi_dxy_custom_widget_info[custom_index].pxyzwh;						
						hmi_object_data_f	= HMI_U32_TO_F32(hmi_object_data);
						if(pcustom_xyz_f[object_index].w != hmi_object_data_f)
						{
							pcustom_xyz_f[object_index].w = hmi_object_data_f;
							hmi_object_id = HMI_DXY_CUSTOM_PRO2DXY_CUSTOM(hmi_object_id);
							HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
							HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
						}
					}
				}
			}
		}
		else if(HMI_IS_DYN_H_CUSTOM(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_H_CUSTOM_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_CUSTOM_CNT)
			{
				search_success =hmi_engine_get_dyn_custom_index(hmi_object_id,&custom_index);
				if(search_success == TRUE)
				{
					object_index	= hmi_object_id -hmi_dxy_custom_widget_info[custom_index].begin;
					if((hmi_dxy_custom_widget_info[custom_index].attr_fun.attr & HMI_XYZWH_F) == 0U)	/*2023 07 10.Custom x,y,z,w,h is float*/
					{
						pcustom_xyz	= (HMI_CUSTOM_XYZWH_STR *)hmi_dxy_custom_widget_info[custom_index].pxyzwh;
						if(pcustom_xyz[object_index].h != (U16)hmi_object_data)
						{
							pcustom_xyz[object_index].h = (U16)hmi_object_data;
							hmi_object_id = HMI_DXY_CUSTOM_PRO2DXY_CUSTOM(hmi_object_id);
							HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
							HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
						}
					}
					else
					{
						pcustom_xyz_f	= (HMI_CUSTOM_XYZWH_FLOAT_STR *)hmi_dxy_custom_widget_info[custom_index].pxyzwh;						
						hmi_object_data_f	= HMI_U32_TO_F32(hmi_object_data);
						if(pcustom_xyz_f[object_index].h != hmi_object_data_f)
						{
							pcustom_xyz_f[object_index].h = hmi_object_data_f;
							hmi_object_id = HMI_DXY_CUSTOM_PRO2DXY_CUSTOM(hmi_object_id);
							HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
							HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
						}
					}
				}
			}
		}
		else if(HMI_IS_DYN_P1_CUSTOM(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_P1_CUSTOM_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_CUSTOM_CNT)
			{
				search_success =hmi_engine_get_dyn_custom_index(hmi_object_id,&custom_index);
				if(search_success == TRUE)
				{
					object_index	= hmi_object_id -hmi_dxy_custom_widget_info[custom_index].begin;
					if(hmi_dxy_custom_widget_info[custom_index].pp1 !=NULL)
					{
						if((hmi_dxy_custom_widget_info[custom_index].attr_fun.attr & HMI_P1_F) != 0U)
						{
							pcustom_p_float		= hmi_dxy_custom_widget_info[custom_index].pp1;
							hmi_object_data_f	= HMI_U32_TO_F32(hmi_object_data);
							if(pcustom_p_float[object_index] != hmi_object_data_f)
							{
								pcustom_p_float[object_index] = hmi_object_data_f;
								hmi_object_id = HMI_DXY_CUSTOM_PRO2DXY_CUSTOM(hmi_object_id);
								HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
								HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
							}
						}
						else
						{
							pcustom_p_int=hmi_dxy_custom_widget_info[custom_index].pp1;
							if(pcustom_p_int[object_index] != (HMI_CUSTOM_P_INT)hmi_object_data)
							{
								pcustom_p_int[object_index] = (HMI_CUSTOM_P_INT)hmi_object_data;
								hmi_object_id = HMI_DXY_CUSTOM_PRO2DXY_CUSTOM(hmi_object_id);
								HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
								HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
							}
						}
						
					}

				}
			}
		}
		else if(HMI_IS_DYN_P2_CUSTOM(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_P2_CUSTOM_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_CUSTOM_CNT)
			{
				search_success =hmi_engine_get_dyn_custom_index(hmi_object_id,&custom_index);
				if(search_success == TRUE)
				{
					object_index	= hmi_object_id -hmi_dxy_custom_widget_info[custom_index].begin;
					
					if(hmi_dxy_custom_widget_info[custom_index].pp2 !=NULL)
					{
						if((hmi_dxy_custom_widget_info[custom_index].attr_fun.attr & HMI_P2_F) != 0U)
						{
							pcustom_p_float		= hmi_dxy_custom_widget_info[custom_index].pp2;
							hmi_object_data_f	= HMI_U32_TO_F32(hmi_object_data);
							if(pcustom_p_float[object_index] != hmi_object_data_f)
							{
								pcustom_p_float[object_index] = hmi_object_data_f;
								hmi_object_id = HMI_DXY_CUSTOM_PRO2DXY_CUSTOM(hmi_object_id);
								HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
								HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
							}
						}
						else
						{
							pcustom_p_int=hmi_dxy_custom_widget_info[custom_index].pp2;
							if(pcustom_p_int[object_index] != (HMI_CUSTOM_P_INT)hmi_object_data)
							{
								pcustom_p_int[object_index] = (HMI_CUSTOM_P_INT)hmi_object_data;
								hmi_object_id = HMI_DXY_CUSTOM_PRO2DXY_CUSTOM(hmi_object_id);
								HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
								HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
							}
						}
						
					}

				}
			}
		}
		else if(HMI_IS_DYN_P3_CUSTOM(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_P3_CUSTOM_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_CUSTOM_CNT)
			{
				search_success =hmi_engine_get_dyn_custom_index(hmi_object_id,&custom_index);
				if(search_success == TRUE)
				{
					object_index	= hmi_object_id -hmi_dxy_custom_widget_info[custom_index].begin;
					if(hmi_dxy_custom_widget_info[custom_index].pp3 !=NULL)
					{
						if((hmi_dxy_custom_widget_info[custom_index].attr_fun.attr & HMI_P3_F) != 0U)
						{
							pcustom_p_float		= hmi_dxy_custom_widget_info[custom_index].pp3;
							hmi_object_data_f	= HMI_U32_TO_F32(hmi_object_data);
							if(pcustom_p_float[object_index] != hmi_object_data_f)
							{
								pcustom_p_float[object_index] = hmi_object_data_f;
								hmi_object_id = HMI_DXY_CUSTOM_PRO2DXY_CUSTOM(hmi_object_id);
								HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
								HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
							}
						}
						else
						{
							pcustom_p_int=hmi_dxy_custom_widget_info[custom_index].pp3;
							if(pcustom_p_int[object_index] != (HMI_CUSTOM_P_INT)hmi_object_data)
							{
								pcustom_p_int[object_index] = (HMI_CUSTOM_P_INT)hmi_object_data;
								hmi_object_id = HMI_DXY_CUSTOM_PRO2DXY_CUSTOM(hmi_object_id);
								HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
								HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
							}
						}
						
					}
				}
			}
		}
		else 
		{
		}
	}
}
#endif

#if HMI_DXY_SPLINE_NUMBER > 0U
void hmi_engine_set_dyn_spline(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data)
{ 
	HMI_SCALE_STR hmi_spline_scale	= 0.0;
	
	if(HMI_IS_DYN_XY_SPLINE_PROPERTY(hmi_object_id))
	{
		if(HMI_IS_DYN_X_SPLINE(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_X_SPLINE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_SPLINE_NUMBER)
			{
				if((HMI_X_STR)(hmi_object_data) != hmi_dxy_spline_rect[hmi_object_id].x)
				{
					hmi_dxy_spline_rect[hmi_object_id].x = (HMI_X_STR)(hmi_object_data); 
					hmi_object_id = HMI_DXY_SPLINE_PRO2DXY_SPLINE(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_Y_SPLINE(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_Y_SPLINE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_SPLINE_NUMBER)
			{
				if((HMI_Y_STR)(hmi_object_data) != hmi_dxy_spline_rect[hmi_object_id].y)
				{
					hmi_dxy_spline_rect[hmi_object_id].y = (HMI_Y_STR)(hmi_object_data); 
					hmi_object_id = HMI_DXY_SPLINE_PRO2DXY_SPLINE(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_W_SPLINE(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_W_SPLINE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_SPLINE_NUMBER)
			{
				if((HMI_WIDTH_STR)(hmi_object_data) != hmi_dxy_spline_rect[hmi_object_id].w)
				{
					hmi_dxy_spline_rect[hmi_object_id].w = (HMI_WIDTH_STR)(hmi_object_data); 
					hmi_object_id = HMI_DXY_SPLINE_PRO2DXY_SPLINE(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_H_SPLINE(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_H_SPLINE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_SPLINE_NUMBER)
			{
				if((HMI_HEIGHT_STR)(hmi_object_data) != hmi_dxy_spline_rect[hmi_object_id].h)
				{
					hmi_dxy_spline_rect[hmi_object_id].h = (HMI_HEIGHT_STR)(hmi_object_data); 
					hmi_object_id = HMI_DXY_SPLINE_PRO2DXY_SPLINE(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_SCALE_SPLINE(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_SCALE_SPLINE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_SPLINE_NUMBER)
			{
				hmi_spline_scale	= (HMI_SCALE_STR)(HMI_U32_TO_F32(hmi_object_data));
				if(hmi_spline_scale != hmi_dxy_spline_scale[hmi_object_id])
				{
					hmi_dxy_spline_scale[hmi_object_id] = hmi_spline_scale; 
					hmi_object_id = HMI_DXY_SPLINE_PRO2DXY_SPLINE(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else 
		{
		}
	}
}
#endif

#if HMI_DXY_CUBE_NUMBER > 0U
void hmi_engine_set_dyn_cube(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data)
{ 
	#if HMI_SUPPORT_FLOAT_ANGEL==HMI_YES
	float_32 value_angel=0;
	#endif
	float_32 cube_scale =0.0f;
	
	if(HMI_IS_DYN_XY_CUBE_PROPERTY(hmi_object_id))
	{
		if(HMI_IS_DYN_X_CUBE(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_X_CUBE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_CUBE_NUMBER)
			{
				if((HMI_X_STR)(hmi_object_data) != hmi_cubes_dyn_xy_rect[hmi_object_id].cube_rect.x)
				{
					hmi_cubes_dyn_xy_rect[hmi_object_id].cube_rect.x = (HMI_X_STR)(hmi_object_data); 
					hmi_object_id = HMI_DXY_CUBED_PRO2DXY_CUBE(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_Y_CUBE(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_Y_CUBE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_CUBE_NUMBER)
			{
				if((HMI_Y_STR)(hmi_object_data) != hmi_cubes_dyn_xy_rect[hmi_object_id].cube_rect.y)
				{
					hmi_cubes_dyn_xy_rect[hmi_object_id].cube_rect.y= (HMI_Y_STR)(hmi_object_data); 
					hmi_object_id = HMI_DXY_CUBED_PRO2DXY_CUBE(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_W_CUBE(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_W_CUBE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_CUBE_NUMBER)
			{
				if((HMI_WIDTH_STR)(hmi_object_data) != hmi_cubes_dyn_xy_rect[hmi_object_id].cube_rect.w)
				{
					hmi_cubes_dyn_xy_rect[hmi_object_id].cube_rect.w= (HMI_WIDTH_STR)(hmi_object_data); 
					hmi_object_id = HMI_DXY_CUBED_PRO2DXY_CUBE(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}	
		else if(HMI_IS_DYN_H_CUBE(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_H_CUBE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_CUBE_NUMBER)
			{
				if((HMI_WIDTH_STR)(hmi_object_data) != hmi_cubes_dyn_xy_rect[hmi_object_id].cube_rect.h)
				{
					hmi_cubes_dyn_xy_rect[hmi_object_id].cube_rect.h= (HMI_HEIGHT_STR)(hmi_object_data); 
					hmi_object_id = HMI_DXY_CUBED_PRO2DXY_CUBE(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_BUMP_CUBE(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_BUMP_CUBE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_CUBE_NUMBER)
			{
				if((BOOLEAN)(hmi_object_data) != hmi_cubes_dyn_xy_rect[hmi_object_id].bump)
				{
					hmi_cubes_dyn_xy_rect[hmi_object_id].bump= (BOOLEAN)(hmi_object_data); 
					hmi_object_id = HMI_DXY_CUBED_PRO2DXY_CUBE(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_ANGEL_CUBE(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_ANGEL_CUBE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_CUBE_NUMBER)
			{
				#if HMI_SUPPORT_FLOAT_ANGEL==HMI_YES
				value_angel	=HMI_U32_TO_F32(hmi_object_data);
				if(hmi_cubes_dyn_xy_rect[hmi_object_id].angel!=value_angel )
				#else
				if(hmi_cubes_dyn_xy_rect[hmi_object_id].angel!= (hmi_object_data))
				#endif
				{
					#if HMI_SUPPORT_FLOAT_ANGEL==HMI_YES
					hmi_cubes_dyn_xy_rect[hmi_object_id].angel = value_angel;
					#else
					hmi_cubes_dyn_xy_rect[hmi_object_id].angel = (float_32)(hmi_object_data);
					#endif
					hmi_object_id = HMI_DXY_CUBED_PRO2DXY_CUBE(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_PRI_AXIS_CUBE(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_PRI_AXIS_CUBE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_CUBE_NUMBER)
			{
				#if HMI_SUPPORT_FLOAT_ANGEL==HMI_YES
				value_angel	=HMI_U32_TO_F32(hmi_object_data);
				if(hmi_cubes_dyn_xy_rect[hmi_object_id].private_angel!=value_angel )
				#else
				if(hmi_cubes_dyn_xy_rect[hmi_object_id].private_angel!= (hmi_object_data))
				#endif
				{
					#if HMI_SUPPORT_FLOAT_ANGEL==HMI_YES
					hmi_cubes_dyn_xy_rect[hmi_object_id].private_angel= value_angel;
					#else
					hmi_cubes_dyn_xy_rect[hmi_object_id].private_angel = (float_32)(hmi_object_data);
					#endif
					hmi_object_id = HMI_DXY_CUBED_PRO2DXY_CUBE(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_Z_CUBE(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_Z_CUBE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_CUBE_NUMBER)
			{
				if(hmi_cubes_dyn_xy_rect[hmi_object_id].z!= (hmi_object_data))
				{
					hmi_cubes_dyn_xy_rect[hmi_object_id].z = (hmi_object_data);
					hmi_object_id = HMI_DXY_CUBED_PRO2DXY_CUBE(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_SCALE_CUBE(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_SCALE_CUBE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_CUBE_NUMBER)
			{
				cube_scale = HMI_U32_TO_F32(hmi_object_data);
				if(hmi_cubes_dyn_xy_rect[hmi_object_id].scale!= (cube_scale))
				{
					hmi_cubes_dyn_xy_rect[hmi_object_id].scale= (cube_scale);
					hmi_object_id = HMI_DXY_CUBED_PRO2DXY_CUBE(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else 
		{
		}
	}
}
#endif
#if HMI_DXY_3DCUBE_NUMBER > 0U
void hmi_engine_set_dyn_3dcube(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data)
{ 
	#if HMI_SUPPORT_FLOAT_ANGEL==HMI_YES
	float_32 value_angel=0;
	#endif
	float_32 cube_scale =0.0f;
	
	if(HMI_IS_DYN_XY_3DCUBE_PROPERTY(hmi_object_id))
	{
		if(HMI_IS_DYN_X_3DCUBE(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_X_3DCUBE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_3DCUBE_NUMBER)
			{
				if((HMI_X_STR)(hmi_object_data) != hmi_3dcubes_dyn_xy_rect[hmi_object_id].cube_rect.x)
				{
					hmi_3dcubes_dyn_xy_rect[hmi_object_id].cube_rect.x = (HMI_X_STR)(hmi_object_data); 
					hmi_object_id = HMI_DXY_3DCUBED_PRO2DXY_3DCUBE(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_Y_3DCUBE(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_Y_3DCUBE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_3DCUBE_NUMBER)
			{
				if((HMI_Y_STR)(hmi_object_data) != hmi_3dcubes_dyn_xy_rect[hmi_object_id].cube_rect.y)
				{
					hmi_3dcubes_dyn_xy_rect[hmi_object_id].cube_rect.y= (HMI_Y_STR)(hmi_object_data); 
					hmi_object_id = HMI_DXY_3DCUBED_PRO2DXY_3DCUBE(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_W_3DCUBE(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_W_3DCUBE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_CUBE_NUMBER)
			{
				if((HMI_WIDTH_STR)(hmi_object_data) != hmi_3dcubes_dyn_xy_rect[hmi_object_id].cube_rect.w)
				{
					hmi_3dcubes_dyn_xy_rect[hmi_object_id].cube_rect.w= (HMI_WIDTH_STR)(hmi_object_data); 
					hmi_object_id = HMI_DXY_3DCUBED_PRO2DXY_3DCUBE(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}	
		else if(HMI_IS_DYN_H_3DCUBE(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_H_3DCUBE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_3DCUBE_NUMBER)
			{
				if((HMI_WIDTH_STR)(hmi_object_data) != hmi_3dcubes_dyn_xy_rect[hmi_object_id].cube_rect.h)
				{
					hmi_3dcubes_dyn_xy_rect[hmi_object_id].cube_rect.h= (HMI_HEIGHT_STR)(hmi_object_data); 
					hmi_object_id = HMI_DXY_3DCUBED_PRO2DXY_3DCUBE(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_BUMP_3DCUBE(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_BUMP_3DCUBE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_3DCUBE_NUMBER)
			{
				if(hmi_object_data != hmi_3dcubes_dyn_xy_rect[hmi_object_id].bump)
				{
					hmi_3dcubes_dyn_xy_rect[hmi_object_id].bump	= (BOOLEAN)(hmi_object_data); 
					hmi_object_id = HMI_DXY_3DCUBED_PRO2DXY_3DCUBE(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_ANGEL_3DCUBE(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_ANGEL_3DCUBE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_3DCUBE_NUMBER)
			{
				#if HMI_SUPPORT_FLOAT_ANGEL==HMI_YES
				value_angel	=HMI_U32_TO_F32(hmi_object_data);
				if(hmi_3dcubes_dyn_xy_rect[hmi_object_id].angel!=value_angel )
				#else
				if(hmi_3dcubes_dyn_xy_rect[hmi_object_id].angel!= (hmi_object_data))
				#endif
				{
					#if HMI_SUPPORT_FLOAT_ANGEL==HMI_YES
					hmi_3dcubes_dyn_xy_rect[hmi_object_id].angel = value_angel;
					#else
					hmi_3dcubes_dyn_xy_rect[hmi_object_id].angel = (float_32)(hmi_object_data);
					#endif
					hmi_object_id = HMI_DXY_3DCUBED_PRO2DXY_3DCUBE(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_PRI_ANGEL_3DCUBE(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_PRI_ANGEL_3DCUBE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_3DCUBE_NUMBER)
			{
				#if HMI_SUPPORT_FLOAT_ANGEL==HMI_YES
				value_angel	= HMI_U32_TO_F32(hmi_object_data);
				if(hmi_3dcubes_dyn_xy_rect[hmi_object_id].private_angel != value_angel )
				#else
				if(hmi_3dcubes_dyn_xy_rect[hmi_object_id].private_angel != (hmi_object_data))
				#endif
				{
					#if HMI_SUPPORT_FLOAT_ANGEL==HMI_YES
					hmi_3dcubes_dyn_xy_rect[hmi_object_id].private_angel = value_angel;
					#else
					hmi_3dcubes_dyn_xy_rect[hmi_object_id].private_angel = (float_32)(hmi_object_data);
					#endif
					hmi_object_id = HMI_DXY_3DCUBED_PRO2DXY_3DCUBE(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_Z_3DCUBE(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_Z_3DCUBE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_3DCUBE_NUMBER)
			{
				if(hmi_object_data != hmi_3dcubes_dyn_xy_rect[hmi_object_id].z)
				{
					hmi_3dcubes_dyn_xy_rect[hmi_object_id].z	= (HMI_Z_STR)(hmi_object_data); 
					hmi_object_id = HMI_DXY_3DCUBED_PRO2DXY_3DCUBE(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else if(HMI_IS_DYN_SCALE_3DCUBE(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_SCALE_3DCUBE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_CUBE_NUMBER)
			{
				cube_scale = HMI_U32_TO_F32(hmi_object_data);
				if(hmi_3dcubes_dyn_xy_rect[hmi_object_id].scale!= (cube_scale))
				{
					hmi_3dcubes_dyn_xy_rect[hmi_object_id].scale= (cube_scale);
					hmi_object_id = HMI_DXY_3DCUBED_PRO2DXY_3DCUBE(hmi_object_id);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		else 
		{
		}
	}
}
#endif

#if HMI_MAX_SINGLE_STAUS_CNT>0
/*
****info***id**value**
****text***id**string**
****call***id**********
****event***id**********
*/

#if HMI_MAX_SINGLE_ACT_STAUS_CNT	> 0
void hmi_engine_set_status_action(HMI_OBJECT_ID_STR status_id,HMI_ACTION_TYPE_STR cur_action_status)
{
	HMI_ACTION_TYPE_STR  	action_type		= HMI_ACTION_CNT;
	UINT8					text_len		= 0;
	UINT16					action_start	= 0;
	UINT8					action_number	= 0;
	HMI_OBJECT_ID_STR		hmi_object_id 	= 0;
	HMI_OBJECT_DATA_STR		hmi_object_data = 0;
	HMI_STATUS_STR			hmi_action_index	= 0;
	HMI_STATUS_ACTION_STR * phmi_action     =NULL;
	if(HMI_IS_STATUS_ACTION(status_id))
	{
		hmi_action_index	=HMI_GET_STATUS_ACTION_INDEX(status_id);
		if(hmi_action_index < HMI_MAX_SINGLE_ACT_STAUS_CNT)
		{
			action_start		= hmi_status_action_index[hmi_action_index].start_offset;
			action_number		= hmi_status_action_index[hmi_action_index].offset_number;
			while(action_number > 0)
			{
				phmi_action		= (HMI_STATUS_ACTION_STR * )&hmi_status_action_table[action_start];
				if(phmi_action->action_info& HMI_IN_STATUS_ACTION)
				{
					action_type	= HMI_ACTION_IN;
				}
				else
				{
					action_type	= HMI_ACTION_OUT;
				}
				
				if(action_type==cur_action_status)
				{
					hmi_object_id	= phmi_action->object_id;
					if(phmi_action->action_info & HMI_SET_OBJ_INFO_ACTION)
					{
						hmi_object_data	= phmi_action->object_data;
						hmi_engine_set_object_info(hmi_object_id,hmi_object_data);
					} 
					else if(phmi_action->action_info & HMI_SET_TEXT_ACTION)
					{
						//#if HMI_DYN_EDIT_TEXTS_NUMBER > 0
						
						///#endif
					}
					else if(phmi_action->action_info & HMI_SET_CALL_FUNC_ACTION)
					{
						hmi_status_action_call_func(status_id);
					}
					else if(phmi_action->action_info & HMI_SET_SEND_EVENT_ACTION)
					{
						hmi_engine_set_object_info(hmi_object_id,0);
					}
					else
					{
					}
				}
				action_number--;
				action_start++;
			}
		}
	}
}
#endif

#if 0
HMI_OBJECT_ID_STR hmi_get_next_status(HMI_OBJECT_ID_STR cur_status,HMI_OBJECT_ID_STR event_id)
{
	HMI_OBJECT_ID_STR hmi_next_status	= 0;
	UINT16	hmi_cur_status_index		= 0;
	UINT16	hmi_offset					= 0;
	UINT8 	hmi_len						= 0;
	BOOLEAN success						= FALSE;
	if(HMI_IS_STATUS_DATA(cur_status))
	{
		hmi_cur_status_index	= HMI_GET_STATUS_DATA_INDEX(cur_status);
		if(hmi_cur_status_index < HMI_MAX_SINGLE_STAUS_CNT)
		{
			hmi_offset	= hmi_status_index[hmi_cur_status_index].start_offset;
			hmi_len		= hmi_status_index[hmi_cur_status_index].offset_number;
			
			while((!success)&&(hmi_len>0)) 
			{
				if(hmi_status_table[hmi_offset].event_id == event_id)
				{
					hmi_next_status=hmi_status_table[hmi_offset].next_st;
					success=TRUE;
				}
				hmi_offset++;
				hmi_len--;
			}
		}
	}
	return hmi_next_status;
}
#endif
HMI_OBJECT_ID_STR hmi_get_next_status(HMI_OBJECT_ID_STR cur_status,HMI_OBJECT_ID_STR event_id)
{
	HMI_OBJECT_ID_STR hmi_next_status	= HMI_NB_ELEMENTS;
	UINT16	hmi_cur_status_index		= 0;
	UINT16	hmi_offset					= 0;
	UINT8 	hmi_len						= 0;
	//BOOLEAN success						= FALSE;lq
	UINT16	hmi_mid						= 0;
	UINT16	hmi_low						= 0;
	UINT16	hmi_high					= 0;
	
	
	if(HMI_IS_STATUS_DATA(cur_status))
	{
		hmi_cur_status_index	= HMI_GET_STATUS_DATA_INDEX(cur_status);
		if(hmi_cur_status_index < HMI_MAX_SINGLE_STAUS_CNT)
		{
			hmi_offset	= hmi_status_index[hmi_cur_status_index].start_offset;
			hmi_len		= hmi_status_index[hmi_cur_status_index].offset_number;
			hmi_low		= hmi_offset;
			hmi_high	= hmi_offset + hmi_len;
			while(hmi_low <= hmi_high)
	        {
	            hmi_mid	=(hmi_low + hmi_high)>>1;/*(hmi_low + hmi_high)/2*/
	            if(hmi_status_table[hmi_mid].event_id > event_id)
	            {
	                hmi_high	= hmi_mid - 1;
	            }
	            else if(hmi_status_table[hmi_mid].event_id < event_id)
	            {
	            	hmi_low		= hmi_mid + 1;
	            }
	            else
	            {
					hmi_next_status	= hmi_status_table[hmi_mid].next_st;
					/*finished*/
					hmi_low = hmi_high + 1;
	            }	               
	        }	    							
		}
	}
	return hmi_next_status;
}


HMI_OBJECT_ID_STR hmi_get_last_status(void)
{
	HMI_OBJECT_ID_STR 		hmi_last_status	= 0;
	U16						hmi_cur_table_index	= 0;
	if(HMI_IS_STATUS_TABLE(hmi_cur_table))
	{
		hmi_cur_table_index = HMI_GET_STATUS_TABLE_INDEX(hmi_cur_table);
		if(hmi_cur_table_index < HMI_MAX_SUB_TABLE_CNT)
		{
			hmi_last_status = hmi_last_table_status[hmi_cur_table_index];
		}
	}
	return hmi_last_status;
}


void hmi_set_status_list(HMI_OBJECT_ID_STR hmi_event_id)
{
	HMI_OBJECT_ID_STR 		hmi_cur_status	= 0;
	HMI_OBJECT_ID_STR 		hmi_next_status	= 0;
	HMI_STATUS_STR 			hmi_next_index	= 0;
	HMI_STATUS_EVENT_STR 	hmi_event_index	= 0;
	U16						hmi_cur_table_index	= 0;
	
	if(HMI_IS_STATUS_EVENT(hmi_event_id))
	{
		hmi_event_index = HMI_GET_STATUS_EVENT_INDEX(hmi_event_id);
		if(HMI_IS_STATUS_TABLE(hmi_cur_table))
		{
			hmi_cur_table_index	= HMI_GET_STATUS_TABLE_INDEX(hmi_cur_table);
			if(hmi_cur_table_index < HMI_MAX_SUB_TABLE_CNT)
			{
				hmi_cur_status = hmi_cur_table_status[hmi_cur_table_index];

				hmi_next_status	= hmi_get_next_status(hmi_cur_status,hmi_event_id);
				
				if(HMI_IS_STATUS_TABLE(hmi_next_status))
				{
					hmi_cur_table	= hmi_next_status;
					#if HMI_MAX_SINGLE_ACT_STAUS_CNT	> 0
					hmi_engine_set_status_action(hmi_cur_status,HMI_ACTION_OUT);
					#endif
				}
				else if(HMI_IS_STATUS_DATA(hmi_next_status))
				{
					hmi_next_index=HMI_GET_STATUS_DATA_INDEX(hmi_next_status);
					/*get action to run*/
					hmi_cur_table_status[hmi_cur_table_index]	= hmi_next_status;
					hmi_last_table_status[hmi_cur_table_index]	= hmi_cur_status;
					#if HMI_MAX_SINGLE_ACT_STAUS_CNT	> 0
					hmi_engine_set_status_action(hmi_cur_status,HMI_ACTION_OUT);
					hmi_engine_set_status_action(hmi_next_status,HMI_ACTION_IN);
					#endif
				}
				else
				{
				}
			}
		}
	}
	
}
#endif


#if ((HMI_SCROLL_TEXT_SUPPORT != 0U) && (HMI_DYN_EDIT_TEXTS_NUMBER > 0U))
#ifndef HMI_GRAPHIC_TWLIB
void hmi_engine_set_text_scroll_step(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data)
{ 
	SINT16	hmi_step_value	= 0;
	SINT16	hmi_step_dt		= 0;
	UINT8  	hmi_cycle_flag	= 0;
	UINT8	hmi_cycle_step	= 0;
	UINT8	hmi_sign_bit	= 0;
	/****hmi_object_data bit15->+/-,bit14->cycle scroll or no ,bit13,12->cycle scroll len times******/
	if(HMI_IS_EDITSCROLL_PROPERTY(hmi_object_id))
	{
		hmi_object_id = HMI_GET_SCROLL_TEXT_ID_INDEX(hmi_object_id);
		if(hmi_object_id < HMI_DYN_EDIT_TEXTS_NUMBER)
		{
			HMI_TEXT_PROP_STR CONST * phmi_dyn_text_info = &hmi_edit_text_table[hmi_object_id];
			HMI_CHAR_STR             * phmi_data=NULL;
			HMI_CHAR_STR             * phmi_temp_data=NULL;
			HMI_OBJECT_DATA_STR		fl_data=0U;
			if((phmi_dyn_text_info->properties & HMI_TEXT_PROP_SCROLABLE) != 0)
			{
				phmi_data = &((HMI_CHAR_STR *)(phmi_dyn_text_info->hmi_string))[phmi_dyn_text_info->length+1];
				phmi_temp_data=&((HMI_CHAR_STR *)(phmi_dyn_text_info->hmi_string))[phmi_dyn_text_info->length+1];
				#if HMI_FONT_CODE==HMI_FONT_CODE_UNICODE
				fl_data    = *phmi_temp_data;
				#else
				fl_data    = *phmi_temp_data++;
				(fl_data)<<=8;
				fl_data  += *phmi_temp_data;
				#endif
				if(hmi_object_data==HMI_CLEAR_SCROLL_OFFSET)
				{
					hmi_object_data=0U;
				}
				else
				{
					hmi_step_value =HMI_GET_SCROLL_STEP_VALUE(fl_data);
					if(HMI_GET_SCROLL_SIGN_BIT(fl_data)!=0)/*data <0*/
					{
						hmi_step_value = -hmi_step_value;
					}

					hmi_cycle_flag	= HMI_GET_SCROLL_STEP_CYCLE_FLAG(hmi_object_data);
					hmi_cycle_step	= HMI_GET_SCROLL_STEP_CYCLE_STEP(hmi_object_data);
					hmi_step_dt = HMI_GET_SCROLL_STEP_VALUE(hmi_object_data);
					if(HMI_GET_SCROLL_SIGN_BIT(hmi_object_data)!=0)/*data <0*/
					{
						hmi_step_dt = -hmi_step_dt;
					}
					hmi_step_value += hmi_step_dt;
					if(hmi_step_value < 0)
					{
						hmi_sign_bit = 1;
						hmi_step_value = -hmi_step_value;
					}
					else
					{
						hmi_sign_bit=0;
					}
					hmi_object_data = HMI_SET_SCROLL_STEP_VALUE(hmi_sign_bit,hmi_cycle_flag,hmi_cycle_step,hmi_step_value);
				}
				hmi_object_id=HMI_EDITSCROLL_PRO2EDITSCROLL(hmi_object_id);
				#if HMI_FONT_CODE == HMI_FONT_CODE_UNICODE
				if(*phmi_data != (HMI_CHAR_STR)hmi_object_data)
				{
					*phmi_data =(HMI_CHAR_STR) hmi_object_data;					
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
				#else
				/*hmi_object_id=HMI_GET_SCROLL_PROP_EDIT_TEXT_ID_INDEX(hmi_object_id);*/				
				if(*phmi_data != (HMI_CHAR_STR)(hmi_object_data>>8))
				{
					*phmi_data = (HMI_CHAR_STR)(hmi_object_data>>8);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
				phmi_data++;
				if(*phmi_data != (HMI_CHAR_STR)(hmi_object_data))
				{
					*phmi_data = (HMI_CHAR_STR)(hmi_object_data);
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
				#endif
			}
		}
	}
}
#endif
#endif

#if (defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC))
void hmi_access_layer_video_status(HMI_OBJECT_ID_STR hmi_page_id)
{
	#if HMI_DXY_CONTAINERS_NUMBER > 0U
	HMI_PAGE_TABLE_STR CONST * 	phmi_page_info				= NULL;
	HMI_OBJECT_ID_STR			hmi_object_index			= 0;	
	UINT8 						hmi_number_object  			= 0;
	HMI_OBJECT_PROP_STR CONST * phmi_container_object_table	= NULL;
	UINT8 						hmi_number_object_const		= 0;
	U08							depth						= 0;
	HMI_OBJECT_ID_STR			hmi_object_id				= 0;
	clear_layer_video_status();//clear layer_video_element NULL;
	#if HMI_DXY_PAGES_NUMBER > 0U
	if(HMI_IS_DXY_PAGE(hmi_page_id))
	{
		hmi_object_index	= HMI_GET_PAGE_DXY_ID_INDEX(hmi_page_id);
		phmi_page_info		= &hmi_dxy_page_table[hmi_object_index];
	}
	else
	#endif
	#if HMI_SXY_PAGES_NUMBER>0U
	if(HMI_IS_SXY_PAGE(hmi_page_id))					
	{
		hmi_object_index	= HMI_GET_PAGE_SXY_ID_INDEX(hmi_page_id);
		phmi_page_info		= &hmi_sxy_page_table[hmi_object_index];
	}
	else
	#endif
	{
	}
	 
	if(phmi_page_info !=NULL)
	{
		phmi_container_object_table	= phmi_page_info->container.container_object_table.p_object_table;
		hmi_number_object			= phmi_page_info->container.container_object_table.object_number;
		hmi_number_object_const		= hmi_number_object;	
	}
	
	/*draw element*/
	
	while((hmi_number_object > 0)&&(phmi_container_object_table !=NULL))	
	{
		depth			= (hmi_number_object_const-hmi_number_object);/*node of page no*/
		hmi_object_id	= phmi_container_object_table->object_id;
		phmi_container_object_table++;
		hmi_object_index= 0;
     		 	
		 
		if(HMI_IS_DYN_XY_CONTAINER(hmi_object_id))
		{
			hmi_object_index	= HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_object_id );
			if(hmi_object_index < HMI_DXY_CONTAINERS_NUMBER)
			{
				if(hmi_dxy_container_video_fmt[hmi_object_index].video_fmt_channel
					&HMI_VIDEO_WINDOW)
				{
					/*record depth and video container relation*/
					set_layer_video_status(depth,hmi_object_id,
						&hmi_dxy_container_video_status[hmi_object_index]);
				}/*2016.9.7. for support video capture in*/
			}
		}		
				
	 	hmi_number_object--;
	}
	#endif
}

void  hmi_engine_create_rgl_window(void)
{	
#if(HMI_CREATE_WIN_IN_INIT == HMI_YES)
	#if (defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC))

	HMI_PAGE_TABLE_STR CONST * 	phmi_page_info				= NULL;
	HMI_OBJECT_ID_STR			hmi_object_index			= 0;	
	HMI_RECT_STR				hmi_screen_rect				= {HMI_INVALID_COOR,HMI_INVALID_COOR,0,0};
	HMI_RECT_STR				*pfarther_rect				= NULL;
	UINT8 						hmi_number_object  			= 0;
	HMI_OBJECT_PROP_STR CONST * phmi_container_object_table	= NULL;
	UINT8 						hmi_number_object_const		= 0;
	U08							depth						= 0;
	HMI_OBJECT_ID_STR			hmi_object_id				= 0;
	HMI_OBJECT_ID_STR			hmi_page_id = hmi_layer_0_active_page_id[0].new_page;
	HMI_RECT_STR				hmi_rect_temp				= {HMI_INVALID_COOR,HMI_INVALID_COOR,0,0};

	UINT8						hmi_screen_id	= 0U;
	
	hmi_screen_id =hmi_driver_get_render_screen();
	if(hmi_screen_id == HMI_LAYER_SCREEN0)
	{
		hmi_page_id = hmi_layer_0_active_page_id[0].new_page;
	}
	
#if HMI_ALL_LAYERS_NUMBER > 1
	else if(hmi_screen_id == HMI_LAYER_SCREEN1)
	{
		
		hmi_page_id = hmi_layer_1_active_page_id[0].new_page;
	}
	else
	{
		hmi_page_id = hmi_layer_0_active_page_id[0].new_page;
	}
#endif
	//hmi_driver_clear_rgl_window();//removed by pxguo 161031
	#if HMI_DXY_PAGES_NUMBER > 0U
	if(HMI_IS_DXY_PAGE(hmi_page_id))
	{
		pfarther_rect	= (HMI_RECT_STR *)(&hmi_dxy_page_rect[hmi_page_id]);
		phmi_page_info	= &hmi_dxy_page_table[hmi_page_id];
	}
	else
	#endif
	#if HMI_SXY_PAGES_NUMBER>0U
	if(HMI_IS_SXY_PAGE(hmi_page_id))					
	{
		hmi_object_index= HMI_GET_PAGE_SXY_ID_INDEX(hmi_page_id);
		pfarther_rect	= (HMI_RECT_STR *)&hmi_sxy_page_rect[hmi_object_index];
		phmi_page_info	= &hmi_sxy_page_table[hmi_object_index];		
	}
	else
	#endif
	{
	}
	if(phmi_page_info != NULL)
	{
		phmi_container_object_table	= phmi_page_info->container.container_object_table.p_object_table;
		hmi_number_object			= phmi_page_info->container.container_object_table.object_number;
		hmi_number_object_const		= hmi_number_object;	
	}
	
	/*draw element*/			
	while((hmi_number_object > 0)&&(phmi_container_object_table !=NULL)&&(pfarther_rect != NULL))	
	{
		depth			= (hmi_number_object_const-hmi_number_object);/*node of page no*/
		hmi_object_id	= phmi_container_object_table->object_id;
		phmi_container_object_table++;
		hmi_object_index= 0;
     	if(hmi_object_id  >= HMI_PAGE_SXY_MAX_ID)/*not a page*/
		{	 	
			#if HMI_ALL_DYN_OBJECTS_NUMBER >0U
			if(hmi_object_id  < HMI_ALL_DYN_OBJECTS_NUMBER)
			{			
			}
			#endif
			#if HMI_DXY_IMAGELIST_NUMBER>0
			if(HMI_IS_DYN_IMAGELIST_DXY(hmi_object_id))
			{
				hmi_object_index = HMI_GET_DYN_IMAGELIST_DXY_ID_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_DXY_IMAGELIST_NUMBER)
				{
					hmi_rect_temp.x	=	hmi_dxy_imagelist_rect[hmi_object_index].x;
					hmi_rect_temp.y	=	hmi_dxy_imagelist_rect[hmi_object_index].y;
					hmi_rect_temp.w	=	hmi_dxy_imagelist_rect[hmi_object_index].w;
					hmi_rect_temp.h	=	hmi_dxy_imagelist_rect[hmi_object_index].h;
					HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
						(&hmi_rect_temp),
						(&(hmi_screen_rect)));
				}			
				
				#if (defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG) ||defined(S6J3200_GRAPHIC))
				if(hmi_object_index < HMI_DXY_IMAGELIST_NUMBER)
				{
					hmi_driver_create_rgl_window((&(hmi_screen_rect)),depth);
				}
				#endif			
			}		
			else
			#endif
			#if HMI_SXY_IMAGELIST_NUMBER>0
			if(HMI_IS_DYN_IMAGELIST_SXY(hmi_object_id))
			{
				hmi_object_index = HMI_GET_DYN_IMAGELIST_SXY_ID_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_SXY_IMAGELIST_NUMBER)
				{
					hmi_rect_temp.x	=	hmi_sxy_imagelist_rect[hmi_object_index].x;
					hmi_rect_temp.y	=	hmi_sxy_imagelist_rect[hmi_object_index].y;
					hmi_rect_temp.w	=	hmi_sxy_imagelist_rect[hmi_object_index].w;
					hmi_rect_temp.h	=	hmi_sxy_imagelist_rect[hmi_object_index].h;
					HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
										(&hmi_rect_temp),
										(&hmi_screen_rect));
						
				}
				#if	defined(HMI_GRAPHIC_RGL) || defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG) || defined(S6J3200_GRAPHIC)
				if(hmi_object_index < HMI_SXY_IMAGELIST_NUMBER)
				{
					hmi_driver_create_rgl_window((&(hmi_screen_rect)),depth);
				}
				#endif			
			}
			else
			#endif
			#if HMI_DXY_SCROLLBAR_NUMBER>0
			if(HMI_IS_DYN_SCROLLBAR_DXY(hmi_object_id))
			{
				hmi_object_index = HMI_GET_DYN_SCROLLBAR_DXY_ID_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_DXY_SCROLLBAR_NUMBER)
				{
					hmi_rect_temp.x	=	hmi_dxy_scrollbar_rect[hmi_object_index].x;
					hmi_rect_temp.y	=	hmi_dxy_scrollbar_rect[hmi_object_index].y;
					hmi_rect_temp.w	=	hmi_dxy_scrollbar_rect[hmi_object_index].w;
					hmi_rect_temp.h	=	hmi_dxy_scrollbar_rect[hmi_object_index].h;
					HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
						(&hmi_rect_temp),
						(&(hmi_screen_rect)));
				}
				
				#if	defined(HMI_GRAPHIC_RGL) || defined(S6J3200_GRAPHIC)
				hmi_driver_create_rgl_window((&(hmi_screen_rect)),depth);
				#endif			
			}
			else		
			#endif
			#if HMI_SXY_SCROLLBAR_NUMBER>0
			if(HMI_IS_DYN_SCROLLBAR_SXY(hmi_object_id))
			{
				hmi_object_index = HMI_GET_DYN_SCROLLBAR_SXY_ID_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_SXY_SCROLLBAR_NUMBER)
				{
					hmi_rect_temp.x	=	hmi_sxy_scrollbar_rect[hmi_object_index].x;
					hmi_rect_temp.y	=	hmi_sxy_scrollbar_rect[hmi_object_index].y;
					hmi_rect_temp.w	=	hmi_sxy_scrollbar_rect[hmi_object_index].w;
					hmi_rect_temp.h	=	hmi_sxy_scrollbar_rect[hmi_object_index].h;
					HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
										(&hmi_rect_temp),
										(&(hmi_screen_rect)));				
				}
				#if	defined(HMI_GRAPHIC_RGL )|| defined(S6J3200_GRAPHIC)
				hmi_driver_create_rgl_window((&(hmi_screen_rect)),depth);
				#endif
			}
			else
			#endif
			#if HMI_DXY_BUTTON_NUMBER > 0  
			if(HMI_IS_DYN_BUTTON_DXY(hmi_object_id))
			{
				hmi_object_index = HMI_GET_DYN_BUTTON_DXY_ID_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_DXY_BUTTON_NUMBER)
				{
					hmi_rect_temp.x	=	hmi_dxy_button_rect[hmi_object_index].x;
					hmi_rect_temp.y	=	hmi_dxy_button_rect[hmi_object_index].y;
					hmi_rect_temp.w	=	hmi_dxy_button_rect[hmi_object_index].w;
					hmi_rect_temp.h	=	hmi_dxy_button_rect[hmi_object_index].h;
					HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
						(&hmi_rect_temp),
						(&(hmi_screen_rect)));				
				}

				#if	defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)
				hmi_driver_create_rgl_window((&(hmi_screen_rect)),depth);
				#endif			
			}		
			else
			#endif
			#if HMI_SXY_BUTTON_NUMBER>0 		
			if(HMI_IS_DYN_BUTTON_SXY(hmi_object_id))
			{
				hmi_object_index = HMI_GET_DYN_BUTTON_SXY_ID_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_SXY_BUTTON_NUMBER)
				{
					hmi_rect_temp.x	=	hmi_sxy_button_rect[hmi_object_index].x;
					hmi_rect_temp.y	=	hmi_sxy_button_rect[hmi_object_index].y;
					hmi_rect_temp.w	=	hmi_sxy_button_rect[hmi_object_index].w;
					hmi_rect_temp.h	=	hmi_sxy_button_rect[hmi_object_index].h;
					HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
												(&hmi_rect_temp),
												(&(hmi_screen_rect)));				
				}
				#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)
				hmi_driver_create_rgl_window((&(hmi_screen_rect)),depth);
				#endif
			}		
			else 		
			#endif
			#if HMI_DYN_EDIT_TEXTS_NUMBER/*editable text*/ > 0	   
			if(HMI_IS_DYN_TEXTS(hmi_object_id))
			{	         
				#if HMI_DYN_XY_EDIT_TEXTS_NUMBER > 0
				if(HMI_IS_DYN_TEXT_DXY(hmi_object_id))
			   	{
					hmi_object_index = HMI_GET_DYN_TEXTS_DXY_POS_INDEX(hmi_object_id);
					if(hmi_object_index < HMI_DYN_XY_EDIT_TEXTS_NUMBER)
					{
						hmi_get_object_screen_coor((HMI_RECT_STR CONST *)pfarther_rect,
												&hmi_dyn_xy_edit_text_prop_table[hmi_object_index].text_rect,
												&(hmi_screen_rect));					
					}
					#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)
					hmi_driver_create_rgl_window((&(hmi_screen_rect)),depth);
					#endif	
				}
				else
				#endif		 	
				#if HMI_S_XY_EDIT_TEXTS_NUMBER > 0
				if(HMI_IS_DYN_TEXT_SXY(hmi_object_id))
				{
					hmi_object_index = HMI_GET_DYN_TEXTS_SXY_POS_INDEX(hmi_object_id);
					if(hmi_object_index < HMI_S_XY_EDIT_TEXTS_NUMBER)
					{
						hmi_get_object_screen_coor((HMI_RECT_STR CONST *)pfarther_rect,
													&hmi_static_xy_edit_text_prop_table[hmi_object_index].text_rect,
													&(hmi_screen_rect));					
					}
					#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)
					hmi_driver_create_rgl_window((&(hmi_screen_rect)),depth);
					#endif	
				}
				else
				#endif
				{
				}
			}
			else
			#endif	 
			#if HMI_DYN_CONTAINERS_NUMBER > 0
			if(HMI_IS_DYN_CONTAINER(hmi_object_id))
			{		  	
				#if HMI_ALL_STATIC_CONTAINERS_NUMBER > 0  
				hmi_object_id  = HMI_GET_DYN_CONTAINERS_ID_INDEX(hmi_object_id);
			  	hmi_engine_get_static_container_id(&hmi_object_id);/*get static container ID*/			
				if(hmi_object_id == HMI_DYN_CONTAINER_IS_NULL)
				{
					
				}
				#if HMI_DXY_CONTAINERS_NUMBER>0U
				else if(HMI_IS_DYN_XY_CONTAINER(hmi_object_id))
				{
					hmi_object_id = HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_object_id );
					if(hmi_object_id< HMI_DXY_CONTAINERS_NUMBER)
					{
						hmi_get_object_screen_coor((HMI_RECT_STR CONST *)pfarther_rect,
							&hmi_dyn_xy_container_rect[hmi_object_id],
							&(hmi_screen_rect));					
					}
					#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)
					hmi_driver_create_rgl_window((&(hmi_screen_rect)),depth);
					#endif
				}
				#endif
				#if HMI_SXY_CONTAINERS_NUMBER>0
				else if(HMI_IS_CONTAINERS_SXY(hmi_object_id))
				{
					hmi_object_id = HMI_GET_CONTAINERS_SXY_ID_INDEX(hmi_object_id );
					if(hmi_object_id< HMI_SXY_CONTAINERS_NUMBER)
					{
						hmi_get_object_screen_coor((HMI_RECT_STR CONST *)pfarther_rect,
							&hmi_static_container_rect[hmi_object_id],&(hmi_screen_rect));					
					}				
					#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)
						hmi_driver_create_rgl_window((&(hmi_screen_rect)),depth);
					#endif
				}
				#endif
				else
				{
				}	
				#endif
				
			}
			else
			#endif
			#if HMI_DYN_FILL_PAGES_NUMBER > 0
			if(HMI_IS_DYN_NFILL(hmi_object_id))
			{
				hmi_object_index = HMI_GET_DYN_NFILL_INDEX(hmi_object_id );
				if(hmi_object_index< HMI_DYN_FILL_PAGES_NUMBER)
				{
					hmi_rect_temp.x	=	hmi_fills_dyn_xy_rect[hmi_object_index].x;
					hmi_rect_temp.y	=	hmi_fills_dyn_xy_rect[hmi_object_index].y;
					hmi_rect_temp.w	=	hmi_fills_dyn_xy_rect[hmi_object_index].w;
					hmi_rect_temp.h	=	hmi_fills_dyn_xy_rect[hmi_object_index].h;
					HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
						(&hmi_rect_temp)
						,(&(hmi_screen_rect)));				
				}
				#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)
				hmi_driver_create_rgl_window((&(hmi_screen_rect)),depth);
				#endif			
			}
			else
			#endif
			#if HMI_DYN_GFILL_NUMBER > 0
			if(HMI_IS_DYN_GFILL(hmi_object_id))
			{
			    hmi_object_index = HMI_GET_DYN_GFILL_INDEX(hmi_object_id );
				if(hmi_object_index < HMI_DYN_GFILL_NUMBER)
				{
					HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,\
						(&hmi_gradient_dxy_fill_rect[hmi_object_id]),(&hmi_screen_rect));				
				}
				#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)
				hmi_driver_create_rgl_window((&(hmi_screen_rect)),depth);
				#endif
			}
			else
			#endif
			#if HMI_DXY_CUBE_NUMBER > 0
			if(HMI_IS_DYN_CUBE(hmi_object_id))
			{
			    hmi_object_index = HMI_GET_DYN_CUBE_INDEX(hmi_object_id );
				if(hmi_object_index < HMI_DXY_CUBE_NUMBER)
				{
					hmi_get_object_screen_coor((HMI_RECT_STR CONST *)pfarther_rect,
						&hmi_cubes_dyn_xy_rect[hmi_object_index].cube_rect,&(hmi_screen_rect));				
				}
				#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)
				hmi_driver_create_rgl_window((&(hmi_screen_rect)),depth);
				#endif
			}
			else
			#endif
			#if HMI_DXY_CONTAINERS_NUMBER > 0U  
			if(HMI_IS_DYN_XY_CONTAINER(hmi_object_id))
			{
				hmi_object_index = HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_object_id );
				if(hmi_object_index < HMI_DXY_CONTAINERS_NUMBER)
				{
					hmi_get_object_screen_coor((HMI_RECT_STR CONST *)pfarther_rect,
						&hmi_dyn_xy_container_rect[hmi_object_index],
						&(hmi_screen_rect));				
				}
				#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)
				hmi_driver_create_rgl_window((&(hmi_screen_rect)),depth);
				#endif
			}
			else
			#endif
			#if HMI_DXY_BITMAPS_NUMBER> 0 
			if(HMI_IS_DYN_XY_BITMAP(hmi_object_id))
			{	  
				hmi_object_index = HMI_GET_DYN_XY_BITMAP_INDEX(hmi_object_id );
				hmi_rect_temp.x	=	hmi_bmp_dyn_xy_rect[hmi_object_index].x;
				hmi_rect_temp.y	=	hmi_bmp_dyn_xy_rect[hmi_object_index].y;
				hmi_rect_temp.w	=	hmi_bmp_dyn_xy_rect[hmi_object_index].w;
				hmi_rect_temp.h	=	hmi_bmp_dyn_xy_rect[hmi_object_index].h;
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,(&hmi_rect_temp),
												(&(hmi_screen_rect)));			
				#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)
				hmi_driver_create_rgl_window((&(hmi_screen_rect)),depth);
				#endif
			}
			else
			#endif
			#if HMI_STATIC_TEXTS_NUMBER/*uneditable text*/ > 0 
			if(HMI_IS_STATIC_TEXTS(hmi_object_id)) 
			{	         
				 #if HMI_UNEDIT_TEXTS_DYN_XY_NUMBER > 0
			     if(HMI_IS_UNEDIT_TEXT_DYN_XY(hmi_object_id)/*HMI_UNEDIT_TEXTS_DYN_XY_NUMBER*/)
			     {	
				 	hmi_object_index  = HMI_GET_DYN_XY_UNEDIT_POS_COLOR_INDEX(hmi_object_id);
					if(hmi_object_index < HMI_UNEDIT_TEXTS_DYN_XY_NUMBER)
					{
					 	hmi_get_object_screen_coor((HMI_RECT_STR CONST *)pfarther_rect,
													&hmi_dyn_xy_unedit_text_prop_table[hmi_object_index].text_rect,
													&(hmi_screen_rect));	
					}				
					#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)
					hmi_driver_create_rgl_window((&(hmi_screen_rect)),depth);
					#endif				
			     }
				 else 
				#endif
				{
					#if HMI_UNEDIT_TEXTS_STATIC_XY_NUMBER > 0					
					if(HMI_IS_UNEDIT_TEXT_STATIC_XY(hmi_object_id)/*hmi_object_index < HMI_UNEDIT_TEXTS_STATIC_XY_NUMBER*/)
					{
						hmi_object_index = HMI_GET_S_XY_UNEDIT_POS_COLOR_INDEX(hmi_object_id);
						if(hmi_object_index < HMI_UNEDIT_TEXTS_STATIC_XY_NUMBER)
						{
							hmi_get_object_screen_coor((HMI_RECT_STR CONST *)pfarther_rect,
												&hmi_static_xy_unedit_text_prop_table[hmi_object_index].text_rect,
												&(hmi_screen_rect));
						}										
						#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)
						hmi_driver_create_rgl_window((&(hmi_screen_rect)),depth);
						#endif
					}
				#endif
				}
			} 
			else
			#endif	     
			/*draw static element*/
			#if HMI_STATIC_FILL_PAGES_NUMBER > 0 
			if(HMI_IS_SXY_FILL_PAGES(hmi_object_id))
			{
				hmi_object_index  = HMI_GET_SXY_FILL_PAGES_ID_INDEX(hmi_object_id );
				if(hmi_object_index < HMI_STATIC_FILL_PAGES_NUMBER)
				{
					hmi_rect_temp.x	=	hmi_fills_static_xy_rect[hmi_object_index].x;
					hmi_rect_temp.y	=	hmi_fills_static_xy_rect[hmi_object_index].y;
					hmi_rect_temp.w	=	hmi_fills_static_xy_rect[hmi_object_index].w;
					hmi_rect_temp.h	=	hmi_fills_static_xy_rect[hmi_object_index].h;
					HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
						(&hmi_rect_temp),
						(&(hmi_screen_rect)));
				}
				#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)
				hmi_driver_create_rgl_window((&(hmi_screen_rect)),depth);
				#endif		 
			}
			else
			#endif
			#if HMI_STATIC_GFILL_NUMBER> 0
			if(HMI_IS_SXY_GFILL_PAGES(hmi_object_id))
			{
				hmi_object_index  = HMI_GET_SXY_GFILL_PAGES_ID_INDEX(hmi_object_id);
				hmi_rect_temp.x	=	hmi_gradient_sxy_fill_rect[hmi_object_index].x;
				hmi_rect_temp.y	=	hmi_gradient_sxy_fill_rect[hmi_object_index].y;
				hmi_rect_temp.w	=	hmi_gradient_sxy_fill_rect[hmi_object_index].w;
				hmi_rect_temp.h	=	hmi_gradient_sxy_fill_rect[hmi_object_index].h;
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
							(&hmi_gradient_sxy_fill_rect[hmi_object_id]),
							(&hmi_screen_rect));
				#ifdef HMI_GRAPHIC_RGL
				hmi_driver_create_rgl_window((&(hmi_screen_rect)),depth);
				#endif			 
			}
			else
			#endif
			#if HMI_SXY_CUBE_NUMBER > 0
			if(HMI_IS_SXY_CUBE(hmi_object_id))
			{
			    hmi_object_index = HMI_GET_SXY_CUBE_ID_INDEX(hmi_object_id );
				if(hmi_object_index < HMI_SXY_CUBE_NUMBER)
				{
					hmi_get_object_screen_coor((HMI_RECT_STR CONST *)pfarther_rect,
						&hmi_cubes_static_xy_rect[hmi_object_index].cube_rect,&(hmi_screen_rect));				
				}
				#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)
				hmi_driver_create_rgl_window((&(hmi_screen_rect)),depth);
				#endif
			}
			else
			#endif
			#if HMI_SXY_BITMAPS_NUMBER> 0
			if(HMI_IS_S_XY_BITMAP(hmi_object_id))
			{	         
			    hmi_object_index = HMI_GET_SXY_BITMAPS_ID_INDEX(hmi_object_id);			
			    if(hmi_object_index < HMI_SXY_BITMAPS_NUMBER)
			    {
					hmi_rect_temp.x	=	hmi_bmp_static_xy_rect[hmi_object_index].x;
					hmi_rect_temp.y	=	hmi_bmp_static_xy_rect[hmi_object_index].y;
					hmi_rect_temp.w	=	hmi_bmp_static_xy_rect[hmi_object_index].w;
					hmi_rect_temp.h	=	hmi_bmp_static_xy_rect[hmi_object_index].h;
			       	HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
						(&hmi_rect_temp),
						(&(hmi_screen_rect)));								
					#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)
					hmi_driver_create_rgl_window((&(hmi_screen_rect)),depth);
					#endif
			    }
			}
			else
			#endif
			#if HMI_SXY_CUSTOM_CNT > 0 
			if(HMI_IS_CUSTOM_SXY(hmi_object_id))
			{
			  	#if HMI_SXY_CUSTOM_CNT >0
				hmi_object_id = HMI_GET_CUSTOM_SXY_ID_INDEX(hmi_object_id );
				#endif
			}
			else
			#endif	
			#if HMI_SXY_CONTAINERS_NUMBER > 0 
			if(HMI_IS_CONTAINERS_SXY(hmi_object_id))
			{
			  	#if HMI_SXY_CONTAINERS_NUMBER >0
				hmi_object_index = HMI_GET_CONTAINERS_SXY_ID_INDEX(hmi_object_id );
				hmi_get_object_screen_coor((HMI_RECT_STR CONST *)pfarther_rect,
								&hmi_static_container_rect[hmi_object_index],
								&(hmi_screen_rect));
				#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)
				hmi_driver_create_rgl_window((&(hmi_screen_rect)),depth);
				#endif
				#endif
			}
			else
			#endif	  
			{
				;
			}	   	   
		}
	 hmi_number_object--;
	}
#endif
#endif
}
#endif

#if HMI_ALL_DYN_OBJECTS_NUMBER > 0   
void hmi_engine_set_object_info(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR hmi_object_data)
{
#if ((HMI_DXY_IMAGELIST_NUMBER > 0U) 	\
	||(HMI_SXY_IMAGELIST_NUMBER > 0U)	\
	||(HMI_DXY_SCROLLBAR_NUMBER > 0U)	\
	||(HMI_SXY_SCROLLBAR_NUMBER > 0U)	\
	||(HMI_DXY_BUTTON_NUMBER > 0U)		\
	||(HMI_SXY_BUTTON_NUMBER > 0U))
	BYTE	value_imglist	= 0;/*for btn,imglist*/
	UINT16	value_index		= 0;/*for btn,imglist*/
	BYTE	value_beg_bits	= 0;/*for btn,imglist*/
	BYTE	value_clr		= 0;/*for btn,imglist*/
	UINT16	imagelist_date	= 0;
#endif
#if HMI_ALL_DYN_OBJECTS_NUMBER > HMI_PAGES_NUMBER   
   HMI_OBJECT_ID_STR hmi_dyn_object_offset = 0U;	
#endif
#if HMI_LAYER_0_HIGHEST_PRIORITY > 1
	UINT8 hmi_priority_cnt = 0U;
#endif	
#if (defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC))
#if ((HMI_DXY_IMAGELIST_NUMBER+HMI_SXY_IMAGELIST_NUMBER+	\
	HMI_DXY_SCROLLBAR_NUMBER+HMI_SXY_SCROLLBAR_NUMBER+	\
	HMI_DXY_BUTTON_NUMBER+HMI_SXY_BUTTON_NUMBER+	\
	HMI_DXY_BITMAPS_NUMBER+HMI_SXY_BITMAPS_NUMBER)>0)

	HMI_CONTAINER_U16_STR 	hmi_container_info	=	{{0,0}};
	HMI_OBJECT_PROP_STR hmi_object_prop	=	{0};
#endif
#endif
	if(hmi_object_id < HMI_DYN_OBJECT_MAX_ID)
	{
		if(HMI_IS_PAGE(hmi_object_id))
		{
			#if HMI_PAGES_NUMBER > 0U
			hmi_engine_set_page(hmi_object_id,hmi_object_data);
			#endif
		}
		#if HMI_DXY_IMAGELIST_NUMBER > 0U
		else if(hmi_object_id < HMI_DYN_IMAGELIST_DXY_MAX_ID)
		{
			hmi_dyn_object_offset = HMI_GET_DYN_IMAGELIST_DXY_ID_INDEX(hmi_object_id);
			if(hmi_dyn_object_offset < HMI_DXY_IMAGELIST_NUMBER)
			{
				if(hmi_dxy_imagelist_table[hmi_dyn_object_offset].list_len >((UINT8)hmi_object_data))
				{
					#if (HMI_IMGLIST_MAX_STATUS!=0)
					imagelist_date=(hmi_dyn_object_offset*HMI_IMGLIST_MAX_STATUS_BIT_LEN);
					value_index= imagelist_date>> 3/*/8*/;
					value_beg_bits=imagelist_date&0x07/*%8*/;
					value_imglist=hmi_dxy_imagelist_index[value_index];
					value_imglist=value_imglist<<(8-(value_beg_bits+HMI_IMGLIST_MAX_STATUS_BIT_LEN));
					value_imglist=value_imglist>>(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
					if(value_imglist != (UINT8)hmi_object_data)
					{
						value_clr=0xff;
						value_clr=value_clr<<(8-(value_beg_bits+HMI_IMGLIST_MAX_STATUS_BIT_LEN));
						value_clr=value_clr>>(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
						value_clr=value_clr<<value_beg_bits;
						hmi_dxy_imagelist_index[value_index]&=(BYTE)(~value_clr);/*clear */
						hmi_object_data=hmi_object_data<<(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
						hmi_object_data=hmi_object_data&0xff;
						hmi_object_data=hmi_object_data>>(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
						hmi_object_data=hmi_object_data<<value_beg_bits;
						hmi_dxy_imagelist_index[value_index]|=hmi_object_data;																	
						HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
						HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
					}
					#else
					#error hmi_engine.c: The compiler, imageList length equal zero.
					#endif
				}
				else if((UINT8)hmi_object_data==HMI_IMAGELIST_NEXT)
				{
					#if (HMI_IMGLIST_MAX_STATUS!=0)
					imagelist_date =(hmi_dyn_object_offset*HMI_IMGLIST_MAX_STATUS_BIT_LEN);
					value_index=imagelist_date>>3/*/8*/;
					value_beg_bits=imagelist_date&0x07/*%8*/;
					hmi_object_data=hmi_dxy_imagelist_index[value_index];
					hmi_object_data=hmi_object_data<<(8-(value_beg_bits+HMI_IMGLIST_MAX_STATUS_BIT_LEN));
					hmi_object_data=hmi_object_data&0xff;
					hmi_object_data=hmi_object_data>>(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
					hmi_object_data++;
					if(hmi_dxy_imagelist_table[hmi_dyn_object_offset].list_len <= (UINT8)hmi_object_data)
					{
						value_clr=0xff;
						value_clr=value_clr<<(8-(value_beg_bits+HMI_IMGLIST_MAX_STATUS_BIT_LEN));
						value_clr=value_clr>>(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
						value_clr=value_clr<<value_beg_bits;
						hmi_dxy_imagelist_index[value_index]	&= (BYTE)(~value_clr);
					}
					else
					{
						value_clr=0xff;
						value_clr=value_clr<<(8-(value_beg_bits+HMI_IMGLIST_MAX_STATUS_BIT_LEN));
						value_clr=value_clr>>(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
						value_clr=value_clr<<value_beg_bits;
						hmi_dxy_imagelist_index[value_index]&= (BYTE)(~value_clr);/*clear*/
						hmi_object_data=hmi_object_data<<(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
						hmi_object_data=hmi_object_data&0xff;
						hmi_object_data=hmi_object_data>>(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
						hmi_object_data=hmi_object_data<<value_beg_bits;
						hmi_dxy_imagelist_index[value_index]|= (UINT8)hmi_object_data;
					}
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
					#else
					#error hmi_engine.c: The compiler, imageList length equal zero.
					#endif

				}
				else if((UINT8)hmi_object_data==HMI_IMAGELIST_PRE)
				{
					#if (HMI_IMGLIST_MAX_STATUS!=0)
					imagelist_date 	=(hmi_dyn_object_offset*HMI_IMGLIST_MAX_STATUS_BIT_LEN);
					value_index		=imagelist_date>>3/*/8*/;
					value_beg_bits	=imagelist_date&0x07/*%8*/;
					hmi_object_data=hmi_dxy_imagelist_index[value_index];
					hmi_object_data=hmi_object_data<<(8-(value_beg_bits+HMI_IMGLIST_MAX_STATUS_BIT_LEN));
					hmi_object_data=hmi_object_data&0xff;
					hmi_object_data=hmi_object_data>>(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
					/*clear*/
					value_clr=0xff;
					value_clr=value_clr<<(8-(value_beg_bits+HMI_IMGLIST_MAX_STATUS_BIT_LEN));
					value_clr=value_clr>>(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
					value_clr=value_clr<<value_beg_bits;
					hmi_dxy_imagelist_index[hmi_dyn_object_offset]	&= (BYTE)(~value_clr);
					/*set new value*/
					if((UINT8)hmi_object_data==0U )
					{
						hmi_object_data=hmi_dxy_imagelist_table[hmi_dyn_object_offset].list_len-1;
						hmi_object_data=hmi_object_data<<(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
						hmi_object_data=hmi_object_data&0xff;
						hmi_object_data=hmi_object_data>>(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
						hmi_object_data=hmi_object_data<<value_beg_bits;
						hmi_dxy_imagelist_index[value_index]	|= hmi_object_data;
					}
					else
					{
						hmi_object_data--;
						hmi_object_data=hmi_object_data<<(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
						hmi_object_data=hmi_object_data&0xff;
						hmi_object_data=hmi_object_data>>(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
						hmi_object_data=hmi_object_data<<value_beg_bits;
						hmi_dxy_imagelist_index[value_index]	|= (UINT8)hmi_object_data;
					}
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
					#else
					#error hmi_engine.c: The compiler, imageList length equal zero.
					#endif
				}
				#if defined(HMI_GRAPHIC_RGL	)|| defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)|| defined(S6J3200_GRAPHIC)
				else if((UINT8)hmi_object_data == HMI_LOAD_IMAGE)
				{
					#if(HMI_LOAD_SOURCE_MODE == HMI_LOAD_RES_SHEET)
					hmi_load_id_list[hmi_load_fifo.tail] = hmi_object_id;
					hmi_load_fifo.tail ++;
					if(hmi_load_fifo.tail >= HMI_LOAD_LIST_MAX_NB)
					{
						hmi_load_fifo.tail = 0;
					}
					if(hmi_load_fifo.tail == hmi_load_fifo.head)
					{
						hmi_load_fifo.head ++;
						if(hmi_load_fifo.head >= HMI_LOAD_LIST_MAX_NB)
						{
							hmi_load_fifo.head	= 0U;
						}
					}
					
					#else
					hmi_container_info.container_object_table.object_number	= 1;
					hmi_object_prop.object_id	= hmi_object_id;
					hmi_container_info.container_object_table.p_object_table	= (HMI_OBJECT_PROP_STR CONST	*)(&hmi_object_prop);
					call_C_hmi_driver_load_file(&hmi_container_info);
					#endif
				}
			#if defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)
				else if((UINT8)hmi_object_data == HMI_LOAD_IMAGE_BUF)/*added by lq 2021 1 12 lq*/
				{														
					hmi_load_file_to_pixel_buf(hmi_object_id,TRUE);
				}
			#endif
				else if((UINT8)hmi_object_data == HMI_REMOVE_IMAGE)
				{
					hmi_container_info.container_object_table.object_number	= 1;
					hmi_object_prop.object_id	= hmi_object_id;
					hmi_container_info.container_object_table.p_object_table	= (HMI_OBJECT_PROP_STR CONST	*)(&hmi_object_prop);
					call_C_hmi_driver_remove_load_file(&hmi_container_info);
				}
			#if defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)
				else if((UINT8)hmi_object_data == HMI_REMOVE_IMAGE_BUF)	/*added by lq 2021 1 12 lq*/
				{
					hmi_container_info.container_object_table.object_number	= 1;
					hmi_object_prop.object_id	= hmi_object_id;
					hmi_container_info.container_object_table.p_object_table	= (HMI_OBJECT_PROP_STR CONST	*)(&hmi_object_prop);
					call_C_hmi_driver_remove_buf_file(&hmi_container_info);
				}
			#endif
				#endif
				else
				{
				}
			}
		}
		#endif
		#if HMI_SXY_IMAGELIST_NUMBER>0U
		else if(hmi_object_id<HMI_DYN_IMAGELIST_SXY_MAX_ID)
		{			
			hmi_dyn_object_offset=HMI_GET_DYN_IMAGELIST_SXY_ID_INDEX(hmi_object_id);
			if(hmi_dyn_object_offset<HMI_SXY_IMAGELIST_NUMBER)
			{
				#if (HMI_IMGLIST_MAX_STATUS!=0)
				imagelist_date =(hmi_dyn_object_offset*HMI_IMGLIST_MAX_STATUS_BIT_LEN);
				value_index=imagelist_date>>3/*/8*/;
				value_beg_bits=imagelist_date&0x07/*%8*/;
				value_imglist=hmi_sxy_imagelist_index[value_index];
				value_imglist=value_imglist<<(8-(value_beg_bits+HMI_IMGLIST_MAX_STATUS_BIT_LEN));
				value_imglist=value_imglist>>(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
				if(hmi_sxy_imagelist_table[hmi_dyn_object_offset].list_len >((UINT8)hmi_object_data))
				{
					if(value_imglist != (UINT8)hmi_object_data)
					{
						value_clr=0xff;
						value_clr=value_clr<<(8-(value_beg_bits+HMI_IMGLIST_MAX_STATUS_BIT_LEN));
						value_clr=value_clr>>(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
						value_clr=value_clr<<value_beg_bits;
						hmi_sxy_imagelist_index[value_index]&=(BYTE)(~value_clr);/*clear*/
						hmi_object_data=hmi_object_data<<(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
						hmi_object_data=hmi_object_data&0xff;
						hmi_object_data=hmi_object_data>>(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
						hmi_object_data=hmi_object_data<<value_beg_bits;
						hmi_sxy_imagelist_index[value_index]	|= (UINT8)hmi_object_data;
						HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
						HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
					}
				}
				else if((UINT8)hmi_object_data==HMI_IMAGELIST_NEXT)
				{										
					hmi_object_data=hmi_sxy_imagelist_index[value_index];
					hmi_object_data=hmi_object_data<<(8-(value_beg_bits+HMI_IMGLIST_MAX_STATUS_BIT_LEN));
					hmi_object_data=hmi_object_data&0xff;
					hmi_object_data=hmi_object_data>>(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
					hmi_object_data++;
					value_clr=0xff;
					value_clr=value_clr<<(8-(value_beg_bits+HMI_IMGLIST_MAX_STATUS_BIT_LEN));
					value_clr=value_clr>>(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
					value_clr=value_clr<<value_beg_bits;
					hmi_sxy_imagelist_index[value_index]&=(BYTE)(~value_clr);/*clear*/
					if(hmi_sxy_imagelist_table[hmi_dyn_object_offset].list_len <= (UINT8)hmi_object_data)
					{
						/*hmi_sxy_imagelist_index[hmi_dyn_object_offset]	= 0;*/
					}
					else
					{
						hmi_object_data=hmi_object_data<<(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
						hmi_object_data=hmi_object_data&0xff;
						hmi_object_data=hmi_object_data>>(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
						hmi_object_data=hmi_object_data<<value_beg_bits;
						hmi_sxy_imagelist_index[value_index]	|= (UINT8)hmi_object_data;
					}
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
				else if((UINT8)hmi_object_data==HMI_IMAGELIST_PRE)
				{
					hmi_object_data=hmi_sxy_imagelist_index[value_index];
					hmi_object_data=hmi_object_data<<(8-(value_beg_bits+HMI_IMGLIST_MAX_STATUS_BIT_LEN));
					hmi_object_data=hmi_object_data&0xff;
					hmi_object_data=hmi_object_data>>(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
					value_clr=0xff;
					value_clr=value_clr<<(8-(value_beg_bits+HMI_IMGLIST_MAX_STATUS_BIT_LEN));
					value_clr=value_clr>>(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
					value_clr=value_clr<<value_beg_bits;	
					hmi_sxy_imagelist_index[value_index]&= (BYTE)(~value_clr);/*clear*/
					if((UINT8)hmi_object_data ==0U )
					{
						hmi_object_data=hmi_sxy_imagelist_table[hmi_dyn_object_offset].list_len-1;												
					}
					else
					{
						hmi_object_data--;												
					}
					hmi_object_data=hmi_object_data<<(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
					hmi_object_data=hmi_object_data&0xff;
					hmi_object_data=hmi_object_data>>(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
					hmi_object_data=hmi_object_data<<value_beg_bits;
					hmi_sxy_imagelist_index[value_index]	|= hmi_object_data;
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
				#if (defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC))
				else if((UINT8)hmi_object_data == HMI_LOAD_IMAGE)
				{
					hmi_container_info.container_object_table.object_number	= 1;
					hmi_object_prop.object_id	= hmi_object_id;
					hmi_container_info.container_object_table.p_object_table	= (HMI_OBJECT_PROP_STR CONST	*)(&hmi_object_prop);
					call_C_hmi_driver_load_file(&hmi_container_info);
				}
				else if((UINT8)hmi_object_data == HMI_REMOVE_IMAGE)
				{
					hmi_container_info.container_object_table.object_number	= 1;
					hmi_object_prop.object_id	= hmi_object_id;
					hmi_container_info.container_object_table.p_object_table	= (HMI_OBJECT_PROP_STR CONST	*)(&hmi_object_prop);
					call_C_hmi_driver_remove_load_file(&hmi_container_info);
				}
				#endif
				
				#if defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)
				else if((UINT8)hmi_object_data == HMI_LOAD_IMAGE_BUF)/*added by lq 2021 1 12 lq*/
				{														
					hmi_load_file_to_pixel_buf(hmi_object_id,TRUE);
				}
				else if((UINT8)hmi_object_data == HMI_REMOVE_IMAGE_BUF)	/*added by lq 2021 1 12 lq*/
				{
					hmi_container_info.container_object_table.object_number	= 1;
					hmi_object_prop.object_id	= hmi_object_id;
					hmi_container_info.container_object_table.p_object_table	= (HMI_OBJECT_PROP_STR CONST	*)(&hmi_object_prop);
					call_C_hmi_driver_remove_buf_file(&hmi_container_info);
				}
				#endif
				else
				{
				}
				#else
				#error hmi_engine.c: The compiler, imageList length equal zero.
				#endif
			}

		}
		#endif
		#if HMI_DXY_SCROLLBAR_NUMBER>0U
		else if(hmi_object_id<HMI_DYN_SCROLLBAR_DXY_MAX_ID)
		{
			hmi_dyn_object_offset = HMI_GET_DYN_SCROLLBAR_DXY_ID_INDEX(hmi_object_id);
			if(hmi_dyn_object_offset<HMI_DXY_SCROLLBAR_NUMBER)
			{
				if((hmi_dxy_scrollbar_table[hmi_dyn_object_offset].min_range<=(HMI_RANGE_STR)hmi_object_data) &&
					(hmi_dxy_scrollbar_table[hmi_dyn_object_offset].max_range>=(HMI_RANGE_STR)hmi_object_data))
				{
					imagelist_date=(hmi_dyn_object_offset*HMI_SCROLLBAR_MAX_STATUS_BIT_LEN);
					value_index=imagelist_date>>3/*/8*/;
					value_beg_bits=imagelist_date&0x07/*%8*/;
					value_imglist=hmi_dxy_scrollbar_cur_range[value_index];
					value_imglist=value_imglist<<(8-(value_beg_bits+HMI_SCROLLBAR_MAX_STATUS_BIT_LEN));
					value_imglist=value_imglist>>(8-HMI_SCROLLBAR_MAX_STATUS_BIT_LEN);
					
					if(value_imglist != (UINT8)hmi_object_data)
					{
						value_clr=0xff;
						value_clr=value_clr<<(8-(value_beg_bits+HMI_SCROLLBAR_MAX_STATUS_BIT_LEN));
						value_clr=value_clr>>(8-HMI_SCROLLBAR_MAX_STATUS_BIT_LEN);
						value_clr=value_clr<<value_beg_bits;
						hmi_dxy_scrollbar_cur_range[value_index]&=(BYTE)(~value_clr);/*clear*/
						hmi_object_data=hmi_object_data<<(8-HMI_SCROLLBAR_MAX_STATUS_BIT_LEN);
						hmi_object_data=hmi_object_data&0xff;
						hmi_object_data=hmi_object_data>>(8-HMI_SCROLLBAR_MAX_STATUS_BIT_LEN);
						hmi_object_data=hmi_object_data<<value_beg_bits;
						hmi_dxy_scrollbar_cur_range[value_index]	|= (UINT8)hmi_object_data;
						HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
						HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
					}
					#if 0
					if(hmi_dxy_scrollbar_cur_range[hmi_dyn_object_offset] != (HMI_RANGE_STR)hmi_object_data)
					{
						hmi_dxy_scrollbar_cur_range[hmi_dyn_object_offset] = (HMI_RANGE_STR)hmi_object_data;
						HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
						HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
					}
					#endif
				}
				#if (defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC))
				else if((UINT8)hmi_object_data == HMI_LOAD_IMAGE)
				{
					hmi_container_info.container_object_table.object_number	= 1;
					hmi_object_prop.object_id	= hmi_object_id;
					hmi_container_info.container_object_table.p_object_table	= (HMI_OBJECT_PROP_STR CONST	*)(&hmi_object_prop);
					call_C_hmi_driver_load_file(&hmi_container_info);
				}
				else if((UINT8)hmi_object_data == HMI_REMOVE_IMAGE)
				{
					hmi_container_info.container_object_table.object_number	= 1;
					hmi_object_prop.object_id	= hmi_object_id;
					hmi_container_info.container_object_table.p_object_table	= (HMI_OBJECT_PROP_STR CONST	*)(&hmi_object_prop);
					call_C_hmi_driver_remove_load_file(&hmi_container_info);
				}
				#endif
				#if defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)
				else if((UINT8)hmi_object_data == HMI_LOAD_IMAGE_BUF)/*added by lq 2021 1 12 lq*/
				{														
					hmi_load_file_to_pixel_buf(hmi_object_id,TRUE);
				}
				else if((UINT8)hmi_object_data == HMI_REMOVE_IMAGE_BUF)	/*added by lq 2021 1 12 lq*/
				{
					hmi_container_info.container_object_table.object_number	= 1;
					hmi_object_prop.object_id	= hmi_object_id;
					hmi_container_info.container_object_table.p_object_table	= (HMI_OBJECT_PROP_STR CONST	*)(&hmi_object_prop);
					call_C_hmi_driver_remove_buf_file(&hmi_container_info);
				}
				#endif
				else
				{

				}
			}
		}
		#endif
		#if HMI_SXY_SCROLLBAR_NUMBER>0U
		else if(hmi_object_id<HMI_DYN_SCROLLBAR_SXY_MAX_ID)
		{
			hmi_dyn_object_offset = HMI_GET_DYN_SCROLLBAR_SXY_ID_INDEX(hmi_object_id);
			if(hmi_dyn_object_offset<HMI_SXY_SCROLLBAR_NUMBER)
			{
				if((hmi_sxy_scrollbar_table[hmi_dyn_object_offset].min_range<=(HMI_RANGE_STR)hmi_object_data) &&
					(hmi_sxy_scrollbar_table[hmi_dyn_object_offset].max_range>=(HMI_RANGE_STR)hmi_object_data))
				{
					imagelist_date=(hmi_dyn_object_offset*HMI_SCROLLBAR_MAX_STATUS_BIT_LEN);
					value_index=imagelist_date>>3/*/8*/;
					value_beg_bits=imagelist_date&0x07/*%8*/;
					value_imglist=hmi_sxy_scrollbar_cur_range[value_index];
					value_imglist=value_imglist<<(8-(value_beg_bits+HMI_SCROLLBAR_MAX_STATUS_BIT_LEN));
					value_imglist=value_imglist>>(8-HMI_SCROLLBAR_MAX_STATUS_BIT_LEN);

					if(value_imglist != (UINT8)hmi_object_data)
					{
						value_clr=0xff;
						value_clr=value_clr<<(8-(value_beg_bits+HMI_SCROLLBAR_MAX_STATUS_BIT_LEN));
						value_clr=value_clr>>(8-HMI_SCROLLBAR_MAX_STATUS_BIT_LEN);
						value_clr=value_clr<<value_beg_bits;
						hmi_sxy_scrollbar_cur_range[value_index]&=(BYTE)(~value_clr);/*clear*/
						hmi_object_data=hmi_object_data<<(8-HMI_SCROLLBAR_MAX_STATUS_BIT_LEN);
						hmi_object_data=hmi_object_data&0xff;
						hmi_object_data=hmi_object_data>>(8-HMI_SCROLLBAR_MAX_STATUS_BIT_LEN);
						hmi_object_data=hmi_object_data<<value_beg_bits;
						hmi_sxy_scrollbar_cur_range[value_index]	|= (UINT8)hmi_object_data;
						HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
						HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
					}
					#if 0					
					if(hmi_sxy_scrollbar_cur_range[hmi_dyn_object_offset] != (HMI_RANGE_STR)hmi_object_data)
					{
						hmi_sxy_scrollbar_cur_range[hmi_dyn_object_offset] = (HMI_RANGE_STR)hmi_object_data;
						HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
						HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
					}
					#endif
				}
				#if (defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC))
				else if((UINT8)hmi_object_data == HMI_LOAD_IMAGE)
				{
					hmi_container_info.container_object_table.object_number	= 1;
					hmi_object_prop.object_id	= hmi_object_id;
					hmi_container_info.container_object_table.p_object_table	= (HMI_OBJECT_PROP_STR CONST	*)(&hmi_object_prop);
					call_C_hmi_driver_load_file(&hmi_container_info);
				}
				else if((UINT8)hmi_object_data == HMI_REMOVE_IMAGE)
				{
					hmi_container_info.container_object_table.object_number	= 1;
					hmi_object_prop.object_id	= hmi_object_id;
					hmi_container_info.container_object_table.p_object_table	= (HMI_OBJECT_PROP_STR CONST	*)(&hmi_object_prop);
					call_C_hmi_driver_remove_load_file(&hmi_container_info);
				}
				#endif
				#if defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)
				else if((UINT8)hmi_object_data == HMI_LOAD_IMAGE_BUF)/*added by lq 2021 1 12 lq*/
				{														
					hmi_load_file_to_pixel_buf(hmi_object_id,TRUE);
				}
				else if((UINT8)hmi_object_data == HMI_REMOVE_IMAGE_BUF)	/*added by lq 2021 1 12 lq*/
				{
					hmi_container_info.container_object_table.object_number	= 1;
					hmi_object_prop.object_id	= hmi_object_id;
					hmi_container_info.container_object_table.p_object_table	= (HMI_OBJECT_PROP_STR CONST	*)(&hmi_object_prop);
					call_C_hmi_driver_remove_buf_file(&hmi_container_info);
				}
				#endif
				else
				{

				}
			}
		}
		#endif
		#if HMI_DXY_BUTTON_NUMBER>0U
		else if(hmi_object_id<HMI_DYN_BUTTON_DXY_MAX_ID)
		{
			hmi_dyn_object_offset = HMI_GET_DYN_BUTTON_DXY_ID_INDEX(hmi_object_id);
			if(hmi_dyn_object_offset<HMI_DXY_BUTTON_NUMBER)
			{
				if(hmi_dxy_button_table[hmi_dyn_object_offset].button_image.list_len>(UINT8)(hmi_object_data))
				{
					#if (HMI_BTN_MAX_STATUS!=0)
					imagelist_date=(hmi_dyn_object_offset*HMI_BTN_MAX_STATUS_BIT_LEN);
					value_index=imagelist_date>>3/*/8*/;
					value_beg_bits=imagelist_date&0x07/*%8*/;
					value_imglist=hmi_dxy_button_press_status[value_index];
					value_imglist=value_imglist<<(8-(value_beg_bits+HMI_BTN_MAX_STATUS_BIT_LEN));
					value_imglist=value_imglist>>(8-HMI_BTN_MAX_STATUS_BIT_LEN);
					if(value_imglist!=(UINT8)(hmi_object_data))
					{					
						/*if((value_imglist!=HMI_BUTTON_HIGHTLIGHT_INDEX)&&
							(value_imglist!=HMI_BUTTON_DISABLE_INDEX))*/
						{
							/*clear*/
							value_clr=0xff;
							value_clr=(BYTE/*he2021-03-03*/)(value_clr<<(8-(value_beg_bits+HMI_BTN_MAX_STATUS_BIT_LEN)));
							value_clr=value_clr>>(8-HMI_BTN_MAX_STATUS_BIT_LEN);
							value_clr=value_clr<<value_beg_bits;
							hmi_dxy_button_press_status[value_index]	&= (BYTE)(~value_clr);
							
							hmi_object_data=hmi_object_data<<(8-HMI_BTN_MAX_STATUS_BIT_LEN);
							hmi_object_data=hmi_object_data&0xff;
							hmi_object_data=hmi_object_data>>(8-HMI_BTN_MAX_STATUS_BIT_LEN);
							hmi_object_data=hmi_object_data<<value_beg_bits;
							hmi_dxy_button_press_status[value_index]|=(UINT8)(hmi_object_data);
							HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
							HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
						}
					}
					#else
					#error hmi_engine.c: The compiler, imageList length equal zero.
					#endif
				}
				#if (defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC))
				else if((UINT8)hmi_object_data == HMI_LOAD_IMAGE)
				{
					hmi_container_info.container_object_table.object_number	= 1;
					hmi_object_prop.object_id	= hmi_object_id;
					hmi_container_info.container_object_table.p_object_table	= (HMI_OBJECT_PROP_STR CONST	*)(&hmi_object_prop);
					call_C_hmi_driver_load_file(&hmi_container_info);
				}
				else if((UINT8)hmi_object_data == HMI_REMOVE_IMAGE)
				{
					hmi_container_info.container_object_table.object_number	= 1;
					hmi_object_prop.object_id	= hmi_object_id;
					hmi_container_info.container_object_table.p_object_table	= (HMI_OBJECT_PROP_STR CONST	*)(&hmi_object_prop);
					call_C_hmi_driver_remove_load_file(&hmi_container_info);
				}
				#endif
				#if defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)
				else if((UINT8)hmi_object_data == HMI_LOAD_IMAGE_BUF)/*added by lq 2021 1 12 lq*/
				{														
					hmi_load_file_to_pixel_buf(hmi_object_id,TRUE);
				}
				else if((UINT8)hmi_object_data == HMI_REMOVE_IMAGE_BUF)	/*added by lq 2021 1 12 lq*/
				{
					hmi_container_info.container_object_table.object_number	= 1;
					hmi_object_prop.object_id	= hmi_object_id;
					hmi_container_info.container_object_table.p_object_table	= (HMI_OBJECT_PROP_STR CONST	*)(&hmi_object_prop);
					call_C_hmi_driver_remove_buf_file(&hmi_container_info);
				}
				#endif
				else
				{

				}
			}
		}
		#endif
		#if HMI_SXY_BUTTON_NUMBER>0U
		else if(hmi_object_id<HMI_DYN_BUTTON_SXY_MAX_ID)
		{
			hmi_dyn_object_offset = HMI_GET_DYN_BUTTON_SXY_ID_INDEX(hmi_object_id);
			if(hmi_dyn_object_offset<HMI_SXY_BUTTON_NUMBER)
			{
				if(hmi_sxy_button_table[hmi_dyn_object_offset].button_image.list_len>(UINT8)(hmi_object_data))
				{
					#if (HMI_BTN_MAX_STATUS!=0)
					imagelist_date=(hmi_dyn_object_offset*HMI_BTN_MAX_STATUS_BIT_LEN);
					value_index=imagelist_date>>3/*/8*/;
					value_beg_bits=imagelist_date&0x07/*%8*/;
					value_imglist=hmi_sxy_button_press_status[value_index];
					value_imglist=value_imglist<<(8-(value_beg_bits+HMI_BTN_MAX_STATUS_BIT_LEN));
					value_imglist=value_imglist>>(8-HMI_BTN_MAX_STATUS_BIT_LEN);
					if(value_imglist!=(UINT8)(hmi_object_data))
					{
						/*if((value_imglist!=HMI_BUTTON_HIGHTLIGHT_INDEX)&&
							(value_imglist!=HMI_BUTTON_DISABLE_INDEX))*/
						{
							/*clear*/
							value_clr=0xff;
							value_clr=(BYTE/*he2021-03-03*/)(value_clr<<(8-(value_beg_bits+HMI_BTN_MAX_STATUS_BIT_LEN)));
							value_clr=value_clr>>(8-HMI_BTN_MAX_STATUS_BIT_LEN);
							value_clr=value_clr<<value_beg_bits;
							hmi_sxy_button_press_status[value_index]	&= (BYTE)(~value_clr);
							
							hmi_object_data=hmi_object_data<<(8-HMI_BTN_MAX_STATUS_BIT_LEN);
							hmi_object_data=hmi_object_data&0xff;
							hmi_object_data=hmi_object_data>>(8-HMI_BTN_MAX_STATUS_BIT_LEN);
							hmi_object_data=hmi_object_data<<value_beg_bits;
							hmi_sxy_button_press_status[value_index]|=(UINT8)(hmi_object_data);
							HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
							HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
						}
					}
					#else
					#error hmi_engine.c: The compiler, imageList length equal zero.
					#endif
				}
				#if (defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC))
				else if((UINT8)hmi_object_data == HMI_LOAD_IMAGE)
				{
					hmi_container_info.container_object_table.object_number	= 1;
					hmi_object_prop.object_id	= hmi_object_id;
					hmi_container_info.container_object_table.p_object_table	= (HMI_OBJECT_PROP_STR CONST	*)(&hmi_object_prop);
					call_C_hmi_driver_load_file(&hmi_container_info);
				}
				else if((UINT8)hmi_object_data == HMI_REMOVE_IMAGE)
				{
					hmi_container_info.container_object_table.object_number	= 1;
					hmi_object_prop.object_id	= hmi_object_id;
					hmi_container_info.container_object_table.p_object_table	= (HMI_OBJECT_PROP_STR CONST	*)(&hmi_object_prop);
					call_C_hmi_driver_remove_load_file(&hmi_container_info);
				}
				#endif
				#if defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)
				else if((UINT8)hmi_object_data == HMI_LOAD_IMAGE_BUF)/*added by lq 2021 1 12 lq*/
				{														
					hmi_load_file_to_pixel_buf(hmi_object_id,TRUE);
				}
				else if((UINT8)hmi_object_data == HMI_REMOVE_IMAGE_BUF)	/*added by lq 2021 1 12 lq*/
				{
					hmi_container_info.container_object_table.object_number	= 1;
					hmi_object_prop.object_id	= hmi_object_id;
					hmi_container_info.container_object_table.p_object_table	= (HMI_OBJECT_PROP_STR CONST	*)(&hmi_object_prop);
					call_C_hmi_driver_remove_buf_file(&hmi_container_info);
				}
				#endif
				else
				{

				}
			}
		}
		#endif
		#if HMI_DYN_EDIT_TEXTS_NUMBER > 0U
		else if(hmi_object_id< HMI_DYN_TEXTS_MAX_ID)
		{
			hmi_dyn_object_offset = HMI_GET_DYN_TEXTS_INDEX(hmi_object_id);
			if(hmi_dyn_object_offset<HMI_DYN_EDIT_TEXTS_NUMBER)
			{
				if(hmi_object_data == (HMI_OBJECT_DATA_STR)HMI_CLEAR_TXT)/*clear text*/
				{
					if((*(HMI_CHAR_STR*)(hmi_edit_text_table[hmi_dyn_object_offset].hmi_string))!= 0)
					{
						(*(HMI_CHAR_STR*)(hmi_edit_text_table[hmi_dyn_object_offset].hmi_string)) = 0;
						HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
						HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
					}
				}
				else if(hmi_object_data == (HMI_OBJECT_DATA_STR)HMI_REFRESH_TXT)/*refresh text again*/
				{
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
				else /*set dyn font for editable text*/
				{
					if(hmi_object_data <HMI_ALL_FONT_NUMBER)
					{
						#if HMI_EDIT_TEXT_DXY_DYN_FONT_NUM > 0
						if(HMI_IS_DYN_TEXT_DXY_DYN_FONT(hmi_object_id))
						{
							hmi_dyn_object_offset	= HMI_GET_DYN_TEXTS_DXY_DYN_FONT_POS_INDEX(hmi_object_id);
							if(hmi_dyn_object_offset < HMI_EDIT_TEXT_DXY_DYN_FONT_NUM)
							{
								if(hmi_edit_text_dxy_dyn_font_table[hmi_dyn_object_offset]!=(U08)hmi_object_data)
								{
									hmi_edit_text_dxy_dyn_font_table[hmi_dyn_object_offset]=(U08)hmi_object_data;
									HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
									HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
								}
							}
						}
						#endif
						#if HMI_EDIT_TEXT_SXY_DYN_FONT_NUM > 0
						if(HMI_IS_DYN_TEXT_SXY_DYN_FONT(hmi_object_id))
						{
							hmi_dyn_object_offset	= HMI_GET_DYN_TEXTS_SXY_DYN_FONT_POS_INDEX(hmi_object_id);
							if(hmi_dyn_object_offset < HMI_EDIT_TEXT_SXY_DYN_FONT_NUM)
							{
								if(hmi_edit_text_sxy_dyn_font_table[hmi_dyn_object_offset]!=(U08)hmi_object_data)
								{
									hmi_edit_text_sxy_dyn_font_table[hmi_dyn_object_offset]=(U08)hmi_object_data;
									HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
									HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
								}
							}
						}
						#endif
					}
				}
			}
		}
		#endif
		#if HMI_DYN_CONTAINERS_NUMBER > 0
		else if(hmi_object_id < HMI_DYN_CONTAINERS_MAX_ID)
		{
			hmi_dyn_object_offset = HMI_GET_DYN_CONTAINERS_ID_INDEX(hmi_object_id);
			#if (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
			if(hmi_dyn_object_offset	< HMI_SCENE_DYN_CONTAINERS_NUMBER)
			{
				if((hmi_object_data==HMI_ANIMAT_STOP)||(hmi_object_data==HMI_ANIMAT_PLAY)||
					(hmi_object_data==HMI_ANIMAT_PAUSE)||(hmi_object_data==HMI_ANIMAT_TERMINATE))
				{
					if(hmi_dyn_container_table[hmi_dyn_object_offset] != hmi_object_data)
					{
						hmi_dyn_container_table[hmi_dyn_object_offset]  = (HMI_DYN_CONTAINER_DATA_STR)hmi_object_data;
						HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
						HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
					}
				}
			}
			else
			#endif
			if(hmi_dyn_object_offset<HMI_DYN_CONTAINERS_NUMBER)
			{
				if(hmi_dyn_container_table[hmi_dyn_object_offset] != hmi_object_data)
				{
					hmi_dyn_container_table[hmi_dyn_object_offset]  = (HMI_DYN_CONTAINER_DATA_STR)hmi_object_data;
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
			else
			{
			}
		}
   		#endif
		#if HMI_DYN_FILL_PAGES_NUMBER > 0U
		else if(hmi_object_id < HMI_DYN_NFILL_MAX_ID)
		{
			HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
			HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
		}
		#endif
		#if HMI_DYN_GFILL_NUMBER > 0U
		else if(hmi_object_id < HMI_DYN_GFILL_MAX_ID)
		{
			hmi_dyn_object_offset=HMI_GET_DYN_GFILL_INDEX(hmi_object_id);
			if(hmi_dyn_object_offset<HMI_DYN_GFILL_NUMBER)
			{
				if((UINT8)(hmi_object_data) != hmi_gradient_dxy_fill_table[hmi_dyn_object_offset].fill_type)
				{
					hmi_gradient_dxy_fill_table[hmi_dyn_object_offset].fill_type=(UINT8)(hmi_object_data); 
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		#endif
		#if HMI_DXY_CUBE_NUMBER > 0U
		else if(hmi_object_id < HMI_DYN_CUBE_MAX_ID)
		{
			HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
			HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
		}
		#endif
		#if HMI_DXY_3DCUBE_NUMBER > 0U
		else if(hmi_object_id < HMI_DYN_3DCUBE_MAX_ID)
		{
			HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
			HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
		}
		#endif
		#if HMI_DYN_LANGUAGE_NUMBER>0U
		else if(hmi_object_id < HMI_DYN_LANGUAGE_MAX_ID)
		{			
			hmi_dyn_object_offset=(HMI_OBJECT_ID_STR)HMI_GET_NB_LANGUAGE_ID_INDEX(hmi_object_data);
			if(hmi_dyn_object_offset <HMI_LANGUAGE_NUMBER)
			{
				if((HMI_OBJECT_ID_STR)hmi_object_data !=hmi_cur_language)
				{
					hmi_cur_language=(HMI_OBJECT_ID_STR)(hmi_object_data);
					//hmi_set_font_aligh(HMI_ALIGN_RIGHT);//HMI_ALIGN_LEFT or HMI_ALIGN_RIGHT
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		#endif
		#if HMI_DXY_CONTAINERS_NUMBER>0U
		else if(hmi_object_id < HMI_DYN_XY_CONTAINER_MAX_ID)
		{			
			#if (defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC))	/*2016.9.6 .support RGL video capture*/
			hmi_dyn_object_offset	= HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_object_id);
			if(hmi_object_data == HMI_VIDEO_CAPTURE_EN)				
			{	
				if(!(hmi_dxy_container_video_status[hmi_dyn_object_offset]&HMI_VIDEO_ENABLE_STATUS))
				{
					hmi_dxy_container_video_status[hmi_dyn_object_offset]	|= HMI_VIDEO_ENABLE_STATUS;
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
					/*set video enable flag*/
					hmi_dxy_container_video_status[hmi_dyn_object_offset]	|= HMI_VIDEO_ENABLE_FLAG;		
				}
			}
			else if(hmi_object_data == HMI_VIDEO_CAPTURE_DIS)
			{
				if(hmi_dxy_container_video_status[hmi_dyn_object_offset]&HMI_VIDEO_ENABLE_STATUS)
				{
					hmi_dxy_container_video_status[hmi_dyn_object_offset]	&= ((BYTE)(~HMI_VIDEO_ENABLE_STATUS));
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
					/*set video enable flag*/
					hmi_dxy_container_video_status[hmi_dyn_object_offset]	|= HMI_VIDEO_ENABLE_FLAG;	
				}
			}
			else if(hmi_object_data == HMI_VIDEO_CAPTURE_DESTORY)
			{
				if(hmi_dxy_container_video_status[hmi_dyn_object_offset]&HMI_VIDEO_CREATE_STATUS)
				{
					hmi_dxy_container_video_status[hmi_dyn_object_offset]	&= ((BYTE)(~HMI_VIDEO_CREATE_STATUS));
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
					/*set video create flag*/
					hmi_dxy_container_video_status[hmi_dyn_object_offset]	|= HMI_VIDEO_CREATE_FLAG;
				}
			}
			else if(hmi_object_data == HMI_VIDEO_CAPTURE_INIT)
			{
				if(!(hmi_dxy_container_video_status[hmi_dyn_object_offset]&HMI_VIDEO_CREATE_STATUS))
				{
					hmi_dxy_container_video_status[hmi_dyn_object_offset]	|= HMI_VIDEO_CREATE_STATUS;
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
					/*set video create flag*/
					hmi_dxy_container_video_status[hmi_dyn_object_offset]	|= HMI_VIDEO_CREATE_FLAG;
				}
			}
			else
			{
			}
			#endif
			
		}
		#endif
   		#if HMI_DXY_BITMAPS_NUMBER> 0U
		else if(hmi_object_id < HMI_DYN_XY_BITMAP_MAX_ID)
		{
			#if (defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC))
			if((UINT8)hmi_object_data == HMI_LOAD_IMAGE)
			{
				hmi_container_info.container_object_table.object_number	= 1;
				hmi_object_prop.object_id	= hmi_object_id;
				hmi_container_info.container_object_table.p_object_table	= (HMI_OBJECT_PROP_STR CONST	*)(&hmi_object_prop);
				call_C_hmi_driver_load_file(&hmi_container_info);
			}
			else if((UINT8)hmi_object_data == HMI_REMOVE_IMAGE)
			{
				hmi_container_info.container_object_table.object_number	= 1;
				hmi_object_prop.object_id	= hmi_object_id;
				hmi_container_info.container_object_table.p_object_table	= (HMI_OBJECT_PROP_STR CONST	*)(&hmi_object_prop);
				call_C_hmi_driver_remove_load_file(&hmi_container_info);
			}	
			else
			#endif
			#if defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)
			if((UINT8)hmi_object_data == HMI_LOAD_IMAGE_BUF)/*added by lq 2021 1 12 lq*/
			{														
				hmi_load_file_to_pixel_buf(hmi_object_id,TRUE);
			}
			else if((UINT8)hmi_object_data == HMI_REMOVE_IMAGE_BUF)	/*added by lq 2021 1 12 lq*/
			{
				hmi_container_info.container_object_table.object_number	= 1;
				hmi_object_prop.object_id	= hmi_object_id;
				hmi_container_info.container_object_table.p_object_table	= (HMI_OBJECT_PROP_STR CONST	*)(&hmi_object_prop);
				call_C_hmi_driver_remove_buf_file(&hmi_container_info);
			}
			#endif
			{
				HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
				HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
			}
		}
		#endif
		#if HMI_DXY_SPLINE_NUMBER > 0
		else if(HMI_IS_DXY_SPLINE(hmi_object_id))
		{		
			#if HMI_DXY_ONE_POINT_SPLINE_NUMBER > 0
			if(HMI_IS_DXY_ONE_SPLINE(hmi_object_id))
			{
				hmi_spline_set_one_point(hmi_object_id,hmi_object_data);
			}
			#endif
		}
		#endif
		#if HMI_SXY_SPLINE_NUMBER > 0
		else if(HMI_IS_SXY_SPLINE(hmi_object_id))
		{
			#if HMI_SXY_ONE_POINT_SPLINE_NUMBER > 0
			if(HMI_IS_SXY_ONE_SPLINE(hmi_object_id))
			{
				hmi_spline_set_one_point(hmi_object_id,hmi_object_data);
			}
			#endif
		}
		#endif
		#if HMI_DXY_CUSTOM_CNT > 0
		else if(HMI_IS_DYN_XY_CUSTOM(hmi_object_id))
		{
		}
		#endif
		
		/*set uneditable dxy text dyn font*/
		#if HMI_UNEDIT_TEXT_DXY_DYN_FONT_NUM > 0
		else if(HMI_IS_UNEDIT_TEXT_DYN_XY_DYN_FONT(hmi_object_id))
		{
			hmi_dyn_object_offset	= HMI_GET_DYN_XY_UNEDIT_DYN_FONT_INDEX(hmi_object_id);
			if(hmi_dyn_object_offset < HMI_UNEDIT_TEXT_DXY_DYN_FONT_NUM)
			{
				if(hmi_unedit_text_dxy_dyn_font_table[hmi_dyn_object_offset]!=(U08)hmi_object_data)
				{
					hmi_unedit_text_dxy_dyn_font_table[hmi_dyn_object_offset]=(U08)hmi_object_data;
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		#endif
		/*set uneditable sxy text dyn font*/
		#if HMI_UNEDIT_TEXT_SXY_DYN_FONT_NUM > 0
		else if(HMI_IS_UNEDIT_TEXT_STATIC_XY_DYN_FONT(hmi_object_id))
		{
			hmi_dyn_object_offset	= HMI_GET_S_XY_UNEDIT_DYN_FONT_INDEX(hmi_object_id);
			if(hmi_dyn_object_offset < HMI_UNEDIT_TEXT_SXY_DYN_FONT_NUM)
			{
				if(hmi_unedit_text_sxy_dyn_font_table[hmi_dyn_object_offset]!=(U08)hmi_object_data)
				{
					hmi_unedit_text_sxy_dyn_font_table[hmi_dyn_object_offset]=(U08)hmi_object_data;
					HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
					HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
				}
			}
		}
		#endif
		
		#if HMI_ENABLE_BIN>0
		#if HMI_SEGMENT_MAX>0
		else if(HMI_IS_SEGMENT(hmi_object_id))
		{	
			
			#if (HMI_LOAD_SOURCE_MODE == HMI_LOAD_RES_SEGMEN) || (HMI_LOAD_SOURCE_BUF_MODE == HMI_LOAD_RES_BUF_SEGMEN)
			hmi_dyn_object_offset	= HMI_GET_SEGMENT_INDEX(hmi_object_id);
			
			#if defined(HMI_GRAPHIC_OPENGLES)
			if(HMI_OPEN_FIL_RES == (U08)hmi_object_data)	/* lq add 2023 05 31*/
			{
				hmi_fopeng_seg_file((U08)(hmi_dyn_object_offset));
			}
			else if(HMI_CLOSE_FIL_RES == (U08)hmi_object_data)
/* lq add 2023 05 31*/
			{
				hmi_fclose_seg_file((U08)(hmi_dyn_object_offset));
			}
			else{}
			#endif
			if(hmi_dyn_object_offset < HMI_SEGMENT_MAX)
			{
				if(HMI_LOAD_IMAGE == (U08)hmi_object_data)
				{
				#if (defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(S6J3200_GRAPHIC))
					call_C_hmi_driver_load_file(&hmi_seg_info_list[hmi_dyn_object_offset]);
				#elif (defined(HMI_GRAPHIC_OPENGLES))
					hmi_load_bmp_segment((U08)(hmi_dyn_object_offset));
				#else

				#endif
				}
			#if defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)
				if(HMI_LOAD_IMAGE_BUF == (U08)hmi_object_data) /*lq 2021 02 05*/
				{				
				#if (HMI_LOAD_SOURCE_BUF_MODE == HMI_LOAD_RES_BUF_SEGMEN) 
					call_C_hmi_driver_load_buf_file(&hmi_seg_info_list[hmi_dyn_object_offset]);				
				#endif				
				}
				else if(HMI_REMOVE_IMAGE_BUF == (U08)hmi_object_data) /*lq 2021 02 05*/
				{				
				#if (HMI_LOAD_SOURCE_BUF_MODE == HMI_LOAD_RES_BUF_SEGMEN) 
					call_C_hmi_driver_remove_buf_file(&hmi_seg_info_list[hmi_dyn_object_offset]);				
				#endif				
				}
			#endif
				else if(HMI_REMOVE_IMAGE == (U08)hmi_object_data)
				{
				#if (defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(S6J3200_GRAPHIC))
					call_C_hmi_driver_remove_load_file(&hmi_seg_info_list[hmi_dyn_object_offset]);
				#elif (defined(HMI_GRAPHIC_OPENGLES))
					hmi_unload_bmp_segment((U08)(hmi_dyn_object_offset));
				#else

				#endif
				}
				else
				{
				}
			}
			#endif
		}
		#endif
		#endif
		/*Set element x,y,w,h,color,alpha,angel*/
		#if HMI_DXY_CONTAINERS_NUMBER > 0
		else if(HMI_IS_DYN_XY_CONTAINER_PROPERTY(hmi_object_id))
		{
			hmi_engine_set_dxy_container(hmi_object_id,hmi_object_data);
		}
   		#endif
		#if HMI_DXY_BITMAPS_NUMBER > 0U
		else if(HMI_IS_DYN_XY_BITMAP_PROPERTY(hmi_object_id))
		{
			hmi_engine_set_dxy_bitmaps(hmi_object_id,hmi_object_data);
		}
		#endif
		#if HMI_DXY_PAGES_NUMBER>0U
		else if(HMI_IS_DYN_XY_PAGE_PROPERTY(hmi_object_id))
		{
			hmi_engine_set_dyn_pages(hmi_object_id,hmi_object_data);
		}
		#endif
		#if HMI_DXY_IMAGELIST_NUMBER >0U
		else if(HMI_IS_DYN_XY_IMGLIST_PROPERTY(hmi_object_id))
		{
			hmi_engine_set_dxy_imagelist(hmi_object_id,hmi_object_data);
		}
		#endif
		#if HMI_DXY_SCROLLBAR_NUMBER >0U
		else if(HMI_IS_DYN_XY_SCROLLBAR_PROPERTY(hmi_object_id))
		{
			
			#if	(defined(HMI_GRAPHIC_STGLIB)||defined(HMI_GRAPHIC_AGG)||	\
				defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513)||		\
				defined(HMI_GRAPHIC_OPENGLES)||	defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||		\
				defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC))	
			hmi_engine_set_dxy_scrollbar(hmi_object_id,hmi_object_data);
			#endif
		}
   		#endif
		#if HMI_DXY_BUTTON_NUMBER>0U
		else if(HMI_IS_DYN_XY_BUTTON_PROPERTY(hmi_object_id))
		{
			hmi_engine_set_dxy_button(hmi_object_id,hmi_object_data);
		}
		#endif
		#if HMI_UNEDIT_TEXTS_DYN_XY_NUMBER > 0U
		else if(HMI_IS_DYN_XY_STEXT_PROPERTY(hmi_object_id))
		{
			hmi_engine_set_dxy_unedit_texts(hmi_object_id,hmi_object_data);
		}
		#endif
		#if HMI_DYN_XY_EDIT_TEXTS_NUMBER > 0U
		else if(HMI_IS_DYN_XY_DTEXT_PROPERTY(hmi_object_id))
		{
			hmi_engine_set_dxy_edit_texts(hmi_object_id,hmi_object_data);
		}
		#endif
		#if HMI_DYN_FILL_PAGES_NUMBER > 0U
		else if(HMI_IS_DYN_XY_FILL_PROPERTY(hmi_object_id))
		{
			hmi_engine_set_dyn_fill_pages(hmi_object_id,hmi_object_data);
		}
		#endif
		#if HMI_DYN_GFILL_NUMBER > 0U
		else if(HMI_IS_DYN_XY_GFILL_PROPERTY(hmi_object_id))
		{
			#ifndef HMI_GRAPHIC_TWLIB
			hmi_engine_set_dyn_gfill_pages(hmi_object_id,hmi_object_data);
			#endif
		}
		#endif
		#if HMI_DXY_CUBE_NUMBER > 0U
		else if(HMI_IS_DYN_XY_CUBE_PROPERTY(hmi_object_id))
		{
			hmi_engine_set_dyn_cube(hmi_object_id,hmi_object_data);
		}
		#endif
		#if HMI_DXY_3DCUBE_NUMBER > 0U
		else if(HMI_IS_DYN_XY_3DCUBE_PROPERTY(hmi_object_id))
		{
			hmi_engine_set_dyn_3dcube(hmi_object_id,hmi_object_data);
		}
		#endif
		#if ((HMI_SCROLL_TEXT_SUPPORT != 0U)&&(HMI_DYN_EDIT_TEXTS_NUMBER > 0U))
		else if(HMI_IS_EDITSCROLL_PROPERTY(hmi_object_id))
		{    
			#ifndef HMI_GRAPHIC_TWLIB
			hmi_engine_set_text_scroll_step(hmi_object_id,hmi_object_data);     
			#endif
		}
		#endif
		#if HMI_DXY_SPLINE_NUMBER > 0U
		else if(HMI_IS_DYN_XY_SPLINE_PROPERTY(hmi_object_id))
		{
			hmi_engine_set_dyn_spline(hmi_object_id,hmi_object_data);
		}
		#endif
		#if HMI_DXY_CUSTOM_CNT > 0U
		else if(HMI_IS_DYN_XY_CUSTOM_PROPERTY(hmi_object_id))
		{
			hmi_engine_set_dyn_custom(hmi_object_id,hmi_object_data);
		}
		#endif
   		#if HMI_ALL_DYN_OBJECTS_NUMBER > HMI_PAGES_NUMBER
		else
		{
		}
   		#endif
	}
	#if HMI_SXY_BITMAPS_NUMBER>0
	else if(HMI_IS_S_XY_BITMAP(hmi_object_id))
	{
			#if (defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC))
			if((UINT8)hmi_object_data == HMI_LOAD_IMAGE)
			{
				hmi_container_info.container_object_table.object_number	= 1;
				hmi_object_prop.object_id	= hmi_object_id;
				hmi_container_info.container_object_table.p_object_table	= (HMI_OBJECT_PROP_STR CONST	*)(&hmi_object_prop);
				call_C_hmi_driver_load_file(&hmi_container_info);
			}
			else if((UINT8)hmi_object_data == HMI_REMOVE_IMAGE)
			{
				hmi_container_info.container_object_table.object_number	= 1;
				hmi_object_prop.object_id	= hmi_object_id;
				hmi_container_info.container_object_table.p_object_table	= (HMI_OBJECT_PROP_STR CONST	*)(&hmi_object_prop);
				call_C_hmi_driver_remove_load_file(&hmi_container_info);
			}	
			#if defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)
			else if((UINT8)hmi_object_data == HMI_LOAD_IMAGE_BUF)/*added by lq 2021 1 12 lq*/
			{														
				hmi_load_file_to_pixel_buf(hmi_object_id,TRUE);
			}
			else if((UINT8)hmi_object_data == HMI_REMOVE_IMAGE_BUF)	/*added by lq 2021 1 12 lq*/
			{
				hmi_container_info.container_object_table.object_number	= 1;
				hmi_object_prop.object_id	= hmi_object_id;
				hmi_container_info.container_object_table.p_object_table	= (HMI_OBJECT_PROP_STR CONST	*)(&hmi_object_prop);
				call_C_hmi_driver_remove_buf_file(&hmi_container_info);
			}
			#endif
			else
			{
			}
			#endif
			
	}
	#endif
	#if HMI_ALL_EVENT_NUMBER>0
	else if(IS_EVENT(hmi_object_id))
	{
		if(hmi_object_data==HMI_SEND_EVENT_ON)
		{
			hmi_action_send_event(hmi_object_id);
		}
		else
		{
		}
	}
 	#endif
	#if HMI_ACTION_GROUP_NUMBER>0
	else if(IS_GROUP(hmi_object_id))
	{
		if((hmi_object_data==HMI_ACTION_RUN)||
			(hmi_object_data==HMI_ACTION_CONTINUE)||
			(hmi_object_data==HMI_ACTION_FAST_FINISH)||
			(hmi_object_data==HMI_ACTION_PAUSE))
		{
			hmi_set_action_status(hmi_object_id,hmi_object_data);
		}
		else
		{
		}
	}
	#endif
	/*Set dyn timer elapse*/ 
	#if 0/*HMI_DYN_TIMER_ACTION_DURATION_NUMBER>0*/
	else if(HMI_IS_TIMER_ACTION_D_DURATION_NO_OFFSET(hmi_object_id))
	{
		hmi_object_id=HMI_GET_ACTION_ID_INDEX(hmi_object_id);/*Get action begin offset*/
		hmi_object_id=HMI_GET_TIMER_ACTION_D_DURATION_ID_INDEX(hmi_object_id);
		hmi_set_timer_s_e(hmi_object_id,hmi_object_data,HMI_SET_ELAPSE);
	}
	#endif
	/*Set dyn timer start and duration*/
	#if HMI_DYN_TIMER_ACTION_DURATION_NUMBER>0
	else if(HMI_IS_DYN_TIMER_S_E(hmi_object_id)/*HMI_IS_DYN_TIMER_S_E_NO_OFFSET(hmi_object_id)*/)
	{		
		if(HMI_IS_DYN_TIMER_START(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_TIMER_S_INDEX(hmi_object_id);
			hmi_set_timer_s_e(hmi_object_id,hmi_object_data,HMI_SET_START);
		}
		else /*dyn timer end*/
		{
			hmi_object_id=HMI_GET_DYN_TIMER_E_INDEX(hmi_object_id);
			hmi_set_timer_s_e(hmi_object_id,hmi_object_data,HMI_SET_DURATION);
		}						
	}	
	#endif
	/*Set dyn pos start and end*/
	#if HMI_ANIM_DYN_SET_POS_NUMBER>0
	else if(HMI_IS_DYN_POS_S_E(hmi_object_id))
	{
		if(HMI_IS_DYN_POS_START(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_POS_START(hmi_object_id);
			hmi_set_dyn_pos_s_e(hmi_object_id,hmi_object_data,TRUE);
		}
		else /*end*/
		{
			hmi_object_id=HMI_GET_DYN_POS_END(hmi_object_id);
			hmi_set_dyn_pos_s_e(hmi_object_id,hmi_object_data,FALSE);
		}
	}
	#endif
	/*Set dyn width heigh start and end*/
	#if HMI_ANIM_DYN_SET_W_H_NUMBER>0
	else if(HMI_IS_DYN_WH_S_E(hmi_object_id))
	{
		if(HMI_IS_DYN_WH_START(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_WH_START(hmi_object_id);
			hmi_set_dyn_wh_s_e(hmi_object_id,hmi_object_data,TRUE);
		}
		else /*end*/
		{
			hmi_object_id=HMI_GET_DYN_WH_END(hmi_object_id);
			hmi_set_dyn_wh_s_e(hmi_object_id,hmi_object_data,FALSE);
		}
	}
	#endif
	/*Set dyn fcolor start and end*/
	#if HMI_ANIM_DYN_SET_FOR_COLOR_NUMBER>0
	else if(HMI_IS_DYN_FCOLOR_S_E(hmi_object_id))
	{
		if(HMI_IS_DYN_FCOLOR_START(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_FCOLOR_START(hmi_object_id);
			hmi_set_dyn_fcolor_s_e(hmi_object_id,hmi_object_data,TRUE);
		}
		else /*end*/
		{
			hmi_object_id=HMI_GET_DYN_FCOLOR_END(hmi_object_id);
			hmi_set_dyn_fcolor_s_e(hmi_object_id,hmi_object_data,FALSE);
		}
	}
	#endif
	/*Set dyn image list start and end*/
	#if HMI_ANIM_DYN_SET_IMAGELIST_INDEX_NUMBER>0
	else if(HMI_IS_DYN_IMGLST_S_E(hmi_object_id))
	{
		if(HMI_IS_DYN_IMGLST_START(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_IMGLST_START(hmi_object_id);
			hmi_set_dyn_imglist_s_e(hmi_object_id,hmi_object_data,TRUE);
		}
		else /*end*/
		{
			hmi_object_id=HMI_GET_DYN_IMGLST_END(hmi_object_id);
			hmi_set_dyn_imglist_s_e(hmi_object_id,hmi_object_data,FALSE);
		}
	}
	#endif
	/*Set dyn alpha start and end*/
	#if HMI_ANIM_DYN_SET_ALPHA_NUMBER>0
	else if(HMI_IS_DYN_ALPHA_S_E(hmi_object_id))
	{
		if(HMI_IS_DYN_ALPHA_START(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_ALPHA_START(hmi_object_id);
			hmi_set_dyn_alpha_s_e(hmi_object_id,hmi_object_data,TRUE);
		}
		else /*end*/
		{
			hmi_object_id=HMI_GET_DYN_ALPHA_END(hmi_object_id);
			hmi_set_dyn_alpha_s_e(hmi_object_id,hmi_object_data,FALSE);
		}
	}
	#endif
	
	/*Set dyn SCALE start and end*/
#if HMI_ANIM_DYN_SET_SCALE_NUMBER>0
	else if(HMI_IS_DYN_SCALE_S_E(hmi_object_id))
	{
	#if (defined(HMI_GRAPHIC_STGLIB)||defined(HMI_GRAPHIC_AGG)|| \
			defined(HMI_GRAPHIC_OPENGLES)|| \
			defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_OPENVG)) 	
		if(HMI_IS_DYN_SCALE_START(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_SCALE_START(hmi_object_id);
			hmi_set_dyn_scale_s_e(hmi_object_id,hmi_object_data,TRUE);
		}
		else /*end*/
		{
			hmi_object_id=HMI_GET_DYN_SCALE_END(hmi_object_id);
			hmi_set_dyn_scale_s_e(hmi_object_id,hmi_object_data,FALSE);
		}
	#endif
	}
#endif
	/*Set dyn angel start and end*/
	#if HMI_ANIM_DYN_SET_ANGEL_NUMBER>0
	else if(HMI_IS_DYN_ANGEL_S_E(hmi_object_id))
	{
		#if	(defined(HMI_GRAPHIC_STGLIB)||defined(HMI_GRAPHIC_AGG)|| \
			defined(HMI_GRAPHIC_OPENGLES)|| defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)|| \
			defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC)) 	
		if(HMI_IS_DYN_ANGEL_START(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_ANGEL_START(hmi_object_id);
			hmi_set_dyn_angel_s_e(hmi_object_id,hmi_object_data,TRUE);
		}
		else /*end*/
		{
			hmi_object_id=HMI_GET_DYN_ANGEL_END(hmi_object_id);
			hmi_set_dyn_angel_s_e(hmi_object_id,hmi_object_data,FALSE);
		}
		#endif
	}
	#endif
	#if (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642)|| \
		defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)|| \
		defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC))
	#if HMI_MAX_STATUS_EVENT_CNT>0
	else if(HMI_IS_STATUS_EVENT(hmi_object_id))
	{
		#if HMI_MAX_SINGLE_STAUS_CNT>0
		hmi_set_status_list(hmi_object_id);
		#endif
	}
	#endif
	#endif
	else
	{
	}
}
#endif

#if HMI_DYN_EDIT_TEXTS_NUMBER+						\
	HMI_DXY_CENTER_MUL_TEXTURE_BITMAPS_NUMBER+		\
	HMI_DXY_ROTATION_MUL_TEXTURE_BITMAPS_NUMBER+	\
	HMI_SXY_MUL_TEXTURE_BITMAPS_NUMBER +			\
	HMI_DXY_CUSTOM_CNT> 0
void hmi_engine_edit_text(HMI_OBJECT_ID_STR hmi_object_id,void/*HMI_CHAR_STR*/ CONST * phmi_dst_string)
{
	HMI_OBJECT_ID_STR		hmi_dyn_object_offset	=0U;
	HMI_CHAR_STR         *	phmi_src_string	=NULL;
	HMI_TEXT_LENGTH_STR		hmi_src_string_length	=0U;
	UINT8					hmi_string_changed_flag	=FALSE;
	#if HMI_DXY_CENTER_MUL_TEXTURE_BITMAPS_NUMBER+		\
		HMI_DXY_ROTATION_MUL_TEXTURE_BITMAPS_NUMBER > 0
	UINT8					hmi_mul_bmp_index =0;
	HMI_TEXTURE_RECT_STR *	phmi_src_rect	= NULL;
	HMI_TEXTURE_RECT_STR *	phmi_dst_rect	= NULL;
	#endif
	HMI_CHAR_STR	CONST*	phmi_dst_new_string	=(HMI_CHAR_STR	CONST*)phmi_dst_string;
	#if HMI_DXY_CUSTOM_CNT > 0
	BOOLEAN					get_success			= TRUE;
	HMI_CUSTOM_PROP_STR		custom_object_prop  = {0U};
	INT32					*pcustom_data_int32	= NULL;
	float_32				*pcustom_data_float	= NULL;
	UINT32					custom_index		= 0;
	SINT16 					head_index 			= 0;
    SINT16 					tail_index 			= 0;
	INT32 					length				= 0;
	INT32 					custom_length		= 0;
	INT32 					i					= 0;
	#endif
	#if HMI_DYN_EDIT_TEXTS_NUMBER >0
	if(HMI_IS_DYN_TEXTS(hmi_object_id))
	{
		if(phmi_dst_new_string !=NULL)
		{
			hmi_dyn_object_offset = HMI_GET_DYN_TEXTS_INDEX(hmi_object_id);
			phmi_src_string         = (HMI_CHAR_STR *)hmi_edit_text_table[hmi_dyn_object_offset].hmi_string/*.p_dyn_data*/;
			hmi_src_string_length     = hmi_edit_text_table[hmi_dyn_object_offset].length;
			hmi_string_changed_flag  = FALSE;
			while((hmi_src_string_length != 0) && (*phmi_dst_new_string != 0))
			{
				if(*phmi_dst_new_string != *phmi_src_string)
				{
					*phmi_src_string = *phmi_dst_new_string;
					hmi_string_changed_flag =TRUE;
				}
				phmi_src_string++;
				phmi_dst_new_string++;
				hmi_src_string_length--;
			}
			if(*phmi_src_string != 0U)
			{
				*phmi_src_string = 0U;
				hmi_string_changed_flag = TRUE;
			}
			if( hmi_string_changed_flag)
			{
				HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
				HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
			}
		}
	}
	else 
	#endif
	#if HMI_DXY_CENTER_MUL_TEXTURE_BITMAPS_NUMBER >0
	if(HMI_IS_DXY_CENTER_MUL_TEXTURE_BITAMP(hmi_object_id))
	{
		if(phmi_dst_string !=NULL)
		{
			hmi_dyn_object_offset = HMI_GET_DXY_CENTER_MUL_TEXTURE_BITMAP_INDEX(hmi_object_id);
			hmi_mul_bmp_index		=hmi_dxy_center_texture_rect_list[hmi_dyn_object_offset].mul_bmp_index;
			hmi_src_string_length   = hmi_dxy_center_texture_rect_list[hmi_dyn_object_offset].mul_bmp_len;
			phmi_src_rect         = (HMI_TEXTURE_RECT_STR *)(&hmi_texture_rect_list[hmi_mul_bmp_index]);
			phmi_dst_rect		 =(HMI_TEXTURE_RECT_STR *)phmi_dst_string;
			hmi_string_changed_flag  = FALSE;
			while((hmi_src_string_length != 0) && 
				(phmi_dst_rect != NULL) &&
				(phmi_dst_rect->texture_w != 0) &&
				(phmi_dst_rect->texture_h != 0))
			{
				if((phmi_dst_rect->texture_x != phmi_src_rect->texture_x)||
					(phmi_dst_rect->texture_y != phmi_src_rect->texture_y)||
					(phmi_dst_rect->texture_w != phmi_src_rect->texture_w)||
					(phmi_dst_rect->texture_h != phmi_src_rect->texture_h))
				{
					phmi_src_rect->texture_x= phmi_dst_rect->texture_x;
					phmi_src_rect->texture_y= phmi_dst_rect->texture_y;
					phmi_src_rect->texture_w= phmi_dst_rect->texture_w;
					phmi_src_rect->texture_h= phmi_dst_rect->texture_h;
					hmi_string_changed_flag =TRUE;
				}
				phmi_src_rect++;
				phmi_dst_rect++;
				hmi_src_string_length--;
			}
			if(hmi_src_string_length != 0U)
			{
				phmi_src_rect->texture_w= 0U;
				phmi_src_rect->texture_h= 0U;
				//hmi_string_changed_flag = TRUE;
			}
			if( hmi_string_changed_flag)
			{
				HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
				HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
			}
		}
	}
	else 
	#endif
	#if HMI_DXY_ROTATION_MUL_TEXTURE_BITMAPS_NUMBER	> 0
	if(HMI_IS_DXY_ROTATION_MUL_TEXTURE_BITAMP(hmi_object_id))
	{
		if(phmi_dst_string !=NULL)
		{
			hmi_dyn_object_offset = HMI_GET_DXY_ROTATION_MUL_TEXTURE_BITMAP_INDEX(hmi_object_id);
			hmi_mul_bmp_index		=hmi_dxy_roation_texture_rect_list[hmi_dyn_object_offset].mul_bmp_index;
			hmi_src_string_length   = hmi_dxy_roation_texture_rect_list[hmi_dyn_object_offset].mul_bmp_len;
			phmi_src_rect         = (HMI_TEXTURE_RECT_STR *)(&hmi_texture_rect_list[hmi_mul_bmp_index]);
			phmi_dst_rect		 =(HMI_TEXTURE_RECT_STR *)phmi_dst_string;
			hmi_string_changed_flag  = FALSE;
			while((hmi_src_string_length != 0) && 
				(phmi_dst_rect != NULL) &&
				(phmi_dst_rect->texture_w != 0) &&
				(phmi_dst_rect->texture_h != 0))
			{
				if((phmi_dst_rect->texture_x != phmi_src_rect->texture_x)||
					(phmi_dst_rect->texture_y != phmi_src_rect->texture_y)||
					(phmi_dst_rect->texture_w != phmi_src_rect->texture_w)||
					(phmi_dst_rect->texture_h != phmi_src_rect->texture_h)
				{
					phmi_src_rect->texture_x= phmi_dst_rect->texture_x;
					phmi_src_rect->texture_y= phmi_dst_rect->texture_y;
					phmi_src_rect->texture_w= phmi_dst_rect->texture_w;
					phmi_src_rect->texture_h= phmi_dst_rect->texture_h;
					hmi_string_changed_flag =TRUE;
				}
				phmi_src_rect++;
				phmi_dst_rect++;
				hmi_src_string_length--;
			}
			if(hmi_src_string_length != 0U)
			{
				phmi_src_rect->texture_w= 0U;
				phmi_src_rect->texture_h= 0U;
				//hmi_string_changed_flag = TRUE;
			}
			if( hmi_string_changed_flag)
			{
				HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
				HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
			}
		}
	}
	else 
	#endif
	#if HMI_DXY_CUSTOM_CNT > 0
	if(HMI_IS_CUSTOM_SXY(hmi_object_id) || HMI_IS_DYN_XY_CUSTOM(hmi_object_id))
	{

		get_success = hmi_engine_get_custom_prop(hmi_object_id,
										&custom_object_prop);
		if((custom_object_prop.attr & HMI_FIFO_EN) != 0u)
		{			
			if(HMI_IS_DYN_XY_CUSTOM(hmi_object_id))
			{	
				if ((custom_object_prop.attr & HMI_FIFO_F) == 0u)
				{
					pcustom_data_int32 = (INT32	*)(custom_object_prop.pcustom_data);
				
					custom_index	= (UINT32)HMI_GET_DXY_CUSTOM_INDEX(hmi_object_id);
					if(custom_index < HMI_DXY_CUSTOM_CNT)
					{
						pcustom_data_int32	= &pcustom_data_int32[custom_index];
						custom_length		= pcustom_data_int32[0];
					}
				}
				else
				{
					pcustom_data_float = (float_32	*)(custom_object_prop.pcustom_data);
				
					custom_index	= (UINT32)HMI_GET_DXY_CUSTOM_INDEX(hmi_object_id);
					if(custom_index < HMI_DXY_CUSTOM_CNT)
					{
						pcustom_data_float	= &pcustom_data_float[custom_index];
						custom_length		= ((INT32)pcustom_data_float[0]);
					}
				}
			}
			
			
			if(((pcustom_data_int32 != NULL) || (pcustom_data_float != NULL))&&
				(custom_object_prop.pbeg_end != NULL)&&(phmi_dst_string != NULL))
			{
				head_index 	= custom_object_prop.pbeg_end->head_index;
				tail_index 	= custom_object_prop.pbeg_end->tail_index;
				if(pcustom_data_int32 != NULL)
				{
					custom_length	= pcustom_data_int32[0];
					length			= ((INT32 *)phmi_dst_string)[0];
				}
				else if(pcustom_data_float != NULL)
				{
					custom_length	= (INT32)pcustom_data_float[0];
					length			= (INT32)(((float_32 *)phmi_dst_string)[0]);
				}
				else{}
				if(custom_length != length)
				{
					hmi_string_changed_flag	= TRUE;
				}
				else
				{
					if(pcustom_data_int32 != NULL)
					{
						for(i = 0;i < length;i++)
						{
							if(pcustom_data_int32[i] != ((INT32 *)phmi_dst_string)[i])
							{
								hmi_string_changed_flag = TRUE;
								i			= length;
							}
						}
					}
					else if(pcustom_data_float != NULL)
					{
						for(i = 0;i < length;i++)
						{
							if(pcustom_data_float[i] != ((float_32 *)phmi_dst_string)[i])
							{
								hmi_string_changed_flag = TRUE;
								i			= length;
							}
						}
					}
					else{}
				}
				if(hmi_string_changed_flag	== TRUE)
				{
					if ((length > 0) && (length <= (tail_index - head_index))) 
					{
						if(pcustom_data_int32 != NULL)
						{
				        	memcpy(pcustom_data_int32, phmi_dst_string, length * sizeof(INT32));
						}
						else if(pcustom_data_float != NULL)
						{
				        	memcpy(pcustom_data_float, phmi_dst_string, length * sizeof(float_32));
						}
						else
						{
							hmi_string_changed_flag = FALSE;
						}
				    }
					else
					{
						hmi_string_changed_flag = FALSE;
					}
				}
			}	
		}
		if( hmi_string_changed_flag)
		{
			HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
			HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
		}
	}
	#if 0
	else
	#endif
	#if HMI_SXY_MUL_TEXTURE_BITMAPS_NUMBER
	if(HMI_IS_SXY_MUL_TEXTURE_BITAMP(hmi_object_id))
	{
		if(phmi_dst_string !=NULL)
		{
			hmi_dyn_object_offset = HMI_GET_SXY_MUL_TEXTURE_BITMAP_ID_INDEX(hmi_object_id);
			hmi_mul_bmp_index		=hmi_sxy_texture_rect_list[hmi_dyn_object_offset].mul_bmp_index;
			hmi_src_string_length   = hmi_sxy_texture_rect_list[hmi_dyn_object_offset].mul_bmp_len;
			phmi_src_rect         = (HMI_TEXTURE_RECT_STR *)(&hmi_texture_rect_list[hmi_mul_bmp_index]);
			phmi_dst_rect		 =(HMI_TEXTURE_RECT_STR *)phmi_dst_string;
			hmi_string_changed_flag  = FALSE;
			while((hmi_src_string_length != 0) && 
				(phmi_dst_rect != NULL) &&
				(phmi_dst_rect->texture_w != 0) &&
				(phmi_dst_rect->texture_h != 0))
			{
				if((phmi_dst_rect->texture_x != phmi_src_rect->texture_x)||
					(phmi_dst_rect->texture_y != phmi_src_rect->texture_y)||
					(phmi_dst_rect->texture_w != phmi_src_rect->texture_w)||
					(phmi_dst_rect->texture_h != phmi_src_rect->texture_h))
				{
					phmi_src_rect->texture_x= phmi_dst_rect->texture_x;
					phmi_src_rect->texture_y= phmi_dst_rect->texture_y;
					phmi_src_rect->texture_w= phmi_dst_rect->texture_w;
					phmi_src_rect->texture_h= phmi_dst_rect->texture_h;
					hmi_string_changed_flag =TRUE;
				}
				phmi_src_rect++;
				phmi_dst_rect++;
				hmi_src_string_length--;
			}
			if(hmi_src_string_length != 0U)
			{
				phmi_src_rect->texture_w= 0U;
				phmi_src_rect->texture_h= 0U;
				//hmi_string_changed_flag = TRUE;
			}
			if( hmi_string_changed_flag)
			{
				HMI_DYNAMIC_OBJECT_SET_CHANGED_FLAG(hmi_object_id);
				HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
			}
		}
	}
	else
	#endif
	#endif
	{
		
	}
}
#endif 

#if HMI_ALL_DYN_OBJECTS_NUMBER > HMI_PAGES_NUMBER
#if HMI_GET_INFO_FUNC==YES
BOOLEAN hmi_engine_get_object_info(HMI_OBJECT_ID_STR hmi_object_id,HMI_OBJECT_DATA_STR * phmi_object_data)
{
	BOOLEAN	hmi_get_status	= FALSE;
	#if ((HMI_DYN_TIMER_ACTION_DURATION_NUMBER > 0) 	\
		|| (HMI_ANIM_DYN_SET_ANGEL_NUMBER > 0))
	HMI_TIME	time_angel	= 0;
	#endif
#if ((HMI_DXY_IMAGELIST_NUMBER+HMI_SXY_IMAGELIST_NUMBER+ 	\
	HMI_DXY_SCROLLBAR_NUMBER+HMI_SXY_SCROLLBAR_NUMBER+		\
	HMI_DXY_BUTTON_NUMBER+HMI_SXY_BUTTON_NUMBER) > 0U)
	BYTE	value_imglist	= 0;/*for btn,imglist*/
	BYTE	value_index		= 0;/*for btn,imglist*/
	BYTE	value_beg_bits	= 0;/*for btn,imglist*/
	UINT16  imagelist_date	= 0;
#endif
	HMI_CHAR_STR  * 	phmi_string 	= 0U;
	U08 				font_id 		= HMI_INVALIATE_FONT_ID;
	HMI_OBJECT_ID_STR	hmi_object_index	= 0;
	HMI_OBJECT_ID_STR	hmi_object_index2	= 0;
#if HMI_DYN_EDIT_TEXTS_NUMBER > 0
	SINT16				hmi_scroll_offset		= 0;/*text offset which user set the value*/
	UINT8				hmi_cycle_flag			= 0;
	UINT8				hmi_cycle_step			= 0;
	HMI_TEXT_PROP_STR CONST*	phmi_text_prop_info 	= NULL;	
#endif
	#if HMI_DXY_SPLINE_NUMBER +HMI_SXY_SPLINE_NUMBER >0
	HMI_SCALE_STR		spline_scale	= 0;
	#endif
	#if HMI_DXY_CUSTOM_CNT > 0
	HMI_OBJECT_ID_STR   custom_index	= 0;
	HMI_OBJECT_ID_STR	object_index	= 0;
	BOOLEAN				search_success	= FALSE;
	float_32			hmi_object_data_f =0.0f;
	HMI_CUSTOM_P_INT*	pcustom_p_int	= NULL;
	HMI_CUSTOM_P_FLOAT*	pcustom_p_float	= NULL;
	#endif
	#if HMI_S_XY_EDIT_TEXTS_NUMBER + HMI_DYN_XY_EDIT_TEXTS_NUMBER+HMI_STATIC_TEXTS_NUMBER > 0
	HMI_TEXT_RECT_STR	CONST  *	 phmi_text_rect = NULL;
	HMI_TEXT_PROP_STR	text_info		= {0,0,0,NULL};
	#endif
#if (HMI_STATIC_TEXTS_NUMBER > 0U)
		HMI_CHAR_STR		**pp_str_list	= NULL;
#endif
	
	if(hmi_object_id < HMI_DYN_OBJECT_MAX_ID) 
	{
		if(hmi_object_id < HMI_PAGE_SXY_MAX_ID)
		{
			*phmi_object_data=hmi_layer_0_active_page_id[0].new_page ;//current active page 
			hmi_get_status = TRUE;
		}
		#if HMI_DXY_IMAGELIST_NUMBER>0
		else if(HMI_IS_DYN_IMAGELIST_DXY(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_IMAGELIST_DXY_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_IMAGELIST_NUMBER)
			{
				imagelist_date=(hmi_object_id*HMI_IMGLIST_MAX_STATUS_BIT_LEN);
				value_index=imagelist_date>>3;
				value_beg_bits=imagelist_date%8;
				value_imglist=hmi_dxy_imagelist_index[value_index];
				value_imglist=value_imglist<<(8-(value_beg_bits+HMI_IMGLIST_MAX_STATUS_BIT_LEN));
				value_imglist=value_imglist>>(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
				
				*phmi_object_data  = value_imglist;
				hmi_get_status		= TRUE;
			}
		}
		#endif
		#if HMI_SXY_IMAGELIST_NUMBER>0
		else if(HMI_IS_DYN_IMAGELIST_SXY(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_IMAGELIST_SXY_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_SXY_IMAGELIST_NUMBER)
			{
				imagelist_date=(hmi_object_id*HMI_IMGLIST_MAX_STATUS_BIT_LEN);
				value_index		= (imagelist_date / 8);
				value_beg_bits=imagelist_date%8;
				value_imglist=hmi_sxy_imagelist_index[value_index];
				value_imglist=value_imglist<<(8-(value_beg_bits+HMI_IMGLIST_MAX_STATUS_BIT_LEN));
				value_imglist=value_imglist>>(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
				*phmi_object_data  = value_imglist;
				hmi_get_status = TRUE;
			}
		}
		#endif
		#if HMI_DXY_SCROLLBAR_NUMBER>0
		else if(HMI_IS_DYN_SCROLLBAR_DXY(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_SCROLLBAR_DXY_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_SCROLLBAR_NUMBER)
			{
			   *phmi_object_data  = hmi_dxy_scrollbar_cur_range[hmi_object_id];
			   hmi_get_status = TRUE;
			}
		}
		#endif
		#if HMI_SXY_SCROLLBAR_NUMBER>0
		else if(HMI_IS_DYN_SCROLLBAR_SXY(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_SCROLLBAR_SXY_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_SXY_SCROLLBAR_NUMBER)
			{
			   *phmi_object_data  = hmi_sxy_scrollbar_cur_range[hmi_object_id];
			   hmi_get_status = TRUE;
			}
		}
		#endif
		#if HMI_DXY_BUTTON_NUMBER>0
		else if(HMI_IS_DYN_BUTTON_DXY(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_BUTTON_DXY_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_BUTTON_NUMBER)
			{
				imagelist_date=(hmi_object_id*HMI_BTN_MAX_STATUS_BIT_LEN);
				value_index=imagelist_date>>3/*/8*/;
				value_beg_bits=imagelist_date&0x07/*%8*/;
				value_imglist=hmi_dxy_button_press_status[value_index];
				value_imglist=value_imglist<<(8-(value_beg_bits+HMI_BTN_MAX_STATUS_BIT_LEN));
				value_imglist=value_imglist>>(8-HMI_BTN_MAX_STATUS_BIT_LEN);
				*phmi_object_data  = value_imglist;
				hmi_get_status = TRUE;
			}
		}
		#endif
		#if HMI_SXY_BUTTON_NUMBER>0
		else if(HMI_IS_DYN_BUTTON_SXY(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_BUTTON_SXY_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_SXY_BUTTON_NUMBER)
			{
				imagelist_date=(hmi_object_id*HMI_BTN_MAX_STATUS_BIT_LEN);
				value_index=imagelist_date>>3/*/8*/;
				value_beg_bits=imagelist_date&0x07/*%8*/;
				value_imglist=hmi_sxy_button_press_status[value_index];
				value_imglist=value_imglist<<(8-(value_beg_bits+HMI_BTN_MAX_STATUS_BIT_LEN));
				value_imglist=value_imglist>>(8-HMI_BTN_MAX_STATUS_BIT_LEN);
				*phmi_object_data  = value_imglist;
				hmi_get_status = TRUE;
			}
		}
		#endif
		#if HMI_DYN_EDIT_TEXTS_NUMBER > 0
		else if(HMI_IS_DYN_TEXTS(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_TEXTS_INDEX(hmi_object_id);
			if(hmi_object_index	< HMI_DYN_EDIT_TEXTS_NUMBER)	
			{
				phmi_string	=	(HMI_CHAR_STR *)hmi_edit_text_table[hmi_object_index].hmi_string;
				font_id		=	hmi_edit_text_table[hmi_object_index].font_id;
				phmi_text_prop_info = &hmi_edit_text_table[hmi_object_index];
				
				#if HMI_EDIT_TEXT_DXY_DYN_FONT_NUM > 0
				if(HMI_IS_DYN_TEXT_DXY_DYN_FONT(hmi_object_id))
				{
					hmi_object_index2	= HMI_GET_DYN_TEXTS_DXY_DYN_FONT_POS_INDEX(hmi_object_id);
					if(hmi_object_index2 < HMI_EDIT_TEXT_DXY_DYN_FONT_NUM)
					{
						font_id	= hmi_edit_text_dxy_dyn_font_table[hmi_object_index2];
					}
				}
				else
				#endif
				#if HMI_EDIT_TEXT_SXY_DYN_FONT_NUM > 0
				if(HMI_IS_DYN_TEXT_SXY_DYN_FONT(hmi_object_id))
				{
					hmi_object_index2	= HMI_GET_DYN_TEXTS_SXY_DYN_FONT_POS_INDEX(hmi_object_id);
					if(hmi_object_index2 < HMI_EDIT_TEXT_SXY_DYN_FONT_NUM)
					{
						font_id	= hmi_edit_text_sxy_dyn_font_table[hmi_object_index2];
					}
				}
				else
				#endif
				{
				}
				if(phmi_object_data	!=NULL)
				{
					
					if((*phmi_object_data) == HMI_TEXT_GET_MULLINE)
					{
						#if HMI_DYN_XY_EDIT_TEXTS_NUMBER > 0
						if(HMI_IS_DYN_TEXT_DXY(hmi_object_id))
					   	{
							hmi_object_index2 = HMI_GET_DYN_TEXTS_DXY_POS_INDEX(hmi_object_id);
							if(hmi_object_index2 < HMI_DYN_XY_EDIT_TEXTS_NUMBER)
							{
								phmi_text_rect	= &hmi_dyn_xy_edit_text_prop_table[hmi_object_index2];						
							}
						}
						else
						#endif
						#if HMI_S_XY_EDIT_TEXTS_NUMBER > 0
						if(HMI_IS_DYN_TEXT_SXY(hmi_object_id))
						{
							hmi_object_index2 = HMI_GET_DYN_TEXTS_SXY_POS_INDEX(hmi_object_id);
							if(hmi_object_index2 < HMI_S_XY_EDIT_TEXTS_NUMBER)
							{
								phmi_text_rect	= &hmi_static_xy_edit_text_prop_table[hmi_object_index2];																	
							}
						}
						#endif
						#if HMI_S_XY_EDIT_TEXTS_NUMBER + HMI_DYN_XY_EDIT_TEXTS_NUMBER > 0
							*phmi_object_data	= hmi_get_line_counts(phmi_text_rect,
                                                                                                      &hmi_edit_text_table[hmi_object_index],
                                                                      font_id);
							hmi_get_status 		= TRUE;
						#endif
					}
					else if((*phmi_object_data) ==HMI_TEXT_GET_SCROLL_LEN)
					{
						#if HMI_SCROLL_TEXT_SUPPORT != 0U
						if(phmi_text_prop_info != NULL)
						{
							if(HMI_TEXT_PROP_IS_SCROLABLE(phmi_text_prop_info->properties))
							{
								/*get scroll text offset*/
								#if HMI_FONT_CODE == HMI_FONT_CODE_UNICODE
								hmi_scroll_offset	= phmi_string[phmi_text_prop_info->length+1];
								#else
								hmi_scroll_offset	= (phmi_string[phmi_text_prop_info->length+1]<<8);
								hmi_scroll_offset	+= phmi_string[phmi_text_prop_info->length+2];
								#endif
								hmi_cycle_flag = HMI_GET_SCROLL_STEP_CYCLE_FLAG(hmi_scroll_offset);
								hmi_cycle_step=HMI_GET_SCROLL_STEP_CYCLE_STEP(hmi_scroll_offset);
								
								if(HMI_GET_SCROLL_SIGN_BIT(hmi_scroll_offset))
								{
									hmi_scroll_offset =-(HMI_GET_SCROLL_STEP_VALUE(hmi_scroll_offset));
								}
								else
								{
									hmi_scroll_offset =HMI_GET_SCROLL_STEP_VALUE(hmi_scroll_offset);
								}
								*phmi_object_data	= hmi_scroll_offset;
								hmi_get_status 		= TRUE;
							}
							else
							{
								*phmi_object_data = 0u;
								hmi_get_status 	= FALSE;
							}
						}
						else
						{
							*phmi_object_data = 0u;
							hmi_get_status 	= FALSE;
						}
						#endif
					}
					else if((*phmi_object_data) == 0U)
					{
						if((phmi_string	!= NULL))
						{
							*phmi_object_data	= hmi_driver_get_string_len(phmi_string,font_id);
							hmi_get_status 		= TRUE;
						}
					}
					else
					{
						*phmi_object_data = 1u;

					}
				}
				
			}
			
		}
		#endif
   		#if HMI_DYN_CONTAINERS_NUMBER > 0
		else if(HMI_IS_DYN_CONTAINER(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_CONTAINERS_ID_INDEX(hmi_object_id);
			if(hmi_object_id<HMI_DYN_CONTAINERS_NUMBER)
			{
				*phmi_object_data   = hmi_dyn_container_table[hmi_object_id];
				hmi_get_status = TRUE;
			}
		}
		#endif
		#if HMI_DYN_FILL_PAGES_NUMBER > 0
		else if(HMI_IS_DYN_NFILL(hmi_object_id))
		{
		}
		#endif
		#if HMI_DYN_GFILL_NUMBER>0
		else if(HMI_IS_DYN_GFILL(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_GFILL_INDEX(hmi_object_id);
			if(hmi_object_id<HMI_DYN_GFILL_NUMBER)
			{
				*phmi_object_data	= hmi_gradient_dxy_fill_table[hmi_object_id].fill_type;
				hmi_get_status	= TRUE;
			}
		}
		#endif
		#if HMI_DXY_CUBE_NUMBER>0
		else if(HMI_IS_DYN_CUBE(hmi_object_id))
		{
		}
		#endif
		#if HMI_DXY_3DCUBE_NUMBER>0
		else if(HMI_IS_DYN_3DCUBE(hmi_object_id))
		{
		}
		#endif
		#if HMI_DYN_LANGUAGE_NUMBER>0U
		else if(HMI_IS_DYN_LANGUAGE(hmi_object_id))
		{
			*phmi_object_data=(HMI_OBJECT_DATA_STR)hmi_cur_language;
			hmi_get_status	= TRUE;
		}
		#endif
		#if HMI_DXY_CONTAINERS_NUMBER>0
		else if(HMI_IS_DYN_XY_CONTAINER(hmi_object_id))
		{
			/*2016.9.6 support RGL video capture.get video enable/disable*/
			#if (defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC))
			hmi_object_id		= HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_object_id);		
			*phmi_object_data	= (HMI_OBJECT_DATA_STR)((hmi_dxy_container_video_status[hmi_object_id]&
									HMI_VIDEO_ENABLE_CREATE_BIT));
			hmi_get_status		= TRUE;
			#endif
		}
		#endif
		#if HMI_DXY_BITMAPS_NUMBER > 0
		else if(HMI_IS_DYN_XY_BITMAP(hmi_object_id))
		{
		}
		#endif
		#if HMI_DXY_SPLINE_NUMBER > 0
		else if(HMI_IS_DXY_SPLINE(hmi_object_id))
		{
		}
		#endif
		#if HMI_SXY_SPLINE_NUMBER > 0
		else if(HMI_IS_SXY_SPLINE(hmi_object_id))
		{
		}
		#endif
		#if HMI_DXY_CUSTOM_CNT > 0
		else if(HMI_IS_DYN_XY_CUSTOM(hmi_object_id))
		{
		}
		#endif
		#if HMI_STATIC_TEXTS_NUMBER/*uneditable text*/ > 0 
		else if(HMI_IS_STATIC_TEXTS(hmi_object_id)) 
		{			 
		#if HMI_UNEDIT_TEXTS_DYN_XY_NUMBER > 0
		if(HMI_IS_UNEDIT_TEXT_DYN_XY(hmi_object_id)/*HMI_UNEDIT_TEXTS_DYN_XY_NUMBER*/)
		{	
			hmi_object_index = HMI_GET_STATIC_TEXT_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_STATIC_TEXTS_NUMBER)
			{
				font_id	=hmi_unedit_text_table[hmi_object_index].font_id;
				pp_str_list=(HMI_CHAR_STR **)(hmi_unedit_text_table[hmi_object_index].hmi_string);
				if(HMI_IS_NB_LANGUAGE(hmi_cur_language))
				{
					hmi_object_index=HMI_GET_NB_LANGUAGE_ID_INDEX(hmi_cur_language);
					if(hmi_object_index < HMI_LANGUAGE_NUMBER)
					{									
						phmi_string=(void *)pp_str_list[hmi_object_index];
					}
				}
			}
			#if HMI_UNEDIT_TEXT_DXY_DYN_FONT_NUM > 0
			if(HMI_IS_UNEDIT_TEXT_DYN_XY_DYN_FONT(hmi_object_id))
			{
				hmi_object_index	= HMI_GET_DYN_XY_UNEDIT_DYN_FONT_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_UNEDIT_TEXT_DXY_DYN_FONT_NUM)
				{
					font_id = hmi_unedit_text_dxy_dyn_font_table[hmi_object_index];
				}
			}
			#endif
			if((phmi_string	!= NULL))
			{
				if((*phmi_object_data) == HMI_TEXT_GET_MULLINE)
				{
					hmi_object_index2 = HMI_GET_DYN_XY_UNEDIT_POS_COLOR_INDEX(hmi_object_id);
					hmi_object_index = HMI_GET_STATIC_TEXT_ID_INDEX(hmi_object_id);
					if(hmi_object_index2 < HMI_UNEDIT_TEXTS_DYN_XY_NUMBER)
					{
						phmi_text_rect	= &hmi_dyn_xy_unedit_text_prop_table[hmi_object_index2];																	
					}
					text_info.properties=hmi_unedit_text_table[hmi_object_index].properties;
					text_info.font_id	=hmi_unedit_text_table[hmi_object_index].font_id;
					text_info.length	=hmi_unedit_text_table[hmi_object_index].length;
					text_info.hmi_string	=(void*)phmi_string;
					*phmi_object_data	= hmi_get_line_counts(phmi_text_rect,
                                                              &text_info,
                                                              font_id);
					hmi_get_status 		= TRUE;
					
				}
				else if((*phmi_object_data) == 0U)
				{
					if((phmi_string	!= NULL))
					{
						*phmi_object_data	= hmi_driver_get_string_len(phmi_string,font_id);
						hmi_get_status 		= TRUE;
					}
				}
				else
				{
					*phmi_object_data = 1u;

				}
			}

		}
		else 
		#endif
		{
			#if HMI_UNEDIT_TEXTS_STATIC_XY_NUMBER > 0					
			if(HMI_IS_UNEDIT_TEXT_STATIC_XY(hmi_object_id)/*hmi_object_index < HMI_UNEDIT_TEXTS_STATIC_XY_NUMBER*/)
			{
				hmi_object_index = HMI_GET_STATIC_TEXT_ID_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_STATIC_TEXTS_NUMBER)
				{
					font_id	=hmi_unedit_text_table[hmi_object_index].font_id;
					pp_str_list=(HMI_CHAR_STR **)(hmi_unedit_text_table[hmi_object_index].hmi_string);
					if(HMI_IS_NB_LANGUAGE(hmi_cur_language))
					{
						hmi_object_index	= HMI_GET_NB_LANGUAGE_ID_INDEX(hmi_cur_language);
						if(hmi_object_index < HMI_LANGUAGE_NUMBER)
						{		
							phmi_string=(void *)pp_str_list[hmi_object_index];
						}
					}
				}
				
				#if HMI_UNEDIT_TEXT_SXY_DYN_FONT_NUM > 0
				if(HMI_IS_UNEDIT_TEXT_STATIC_XY_DYN_FONT(hmi_object_id))
				{
					hmi_object_index	= HMI_GET_S_XY_UNEDIT_DYN_FONT_INDEX(hmi_object_id);
					if(hmi_object_index < HMI_UNEDIT_TEXT_SXY_DYN_FONT_NUM)
					{
						font_id = hmi_unedit_text_sxy_dyn_font_table[hmi_object_index];
					}
				}
				#endif
				if((phmi_string != NULL))
				{
					if((*phmi_object_data) == HMI_TEXT_GET_MULLINE)
					{
						hmi_object_index2 = HMI_GET_S_XY_UNEDIT_POS_COLOR_INDEX(hmi_object_id);
						hmi_object_index = HMI_GET_STATIC_TEXT_ID_INDEX(hmi_object_id);
						if(hmi_object_index2 < HMI_UNEDIT_TEXTS_STATIC_XY_NUMBER)
						{
							phmi_text_rect	= &hmi_static_xy_unedit_text_prop_table[hmi_object_index2];																	
						}
						text_info.properties=hmi_unedit_text_table[hmi_object_index].properties;
						text_info.font_id	=hmi_unedit_text_table[hmi_object_index].font_id;
						text_info.length	=hmi_unedit_text_table[hmi_object_index].length;
						text_info.hmi_string	=(void*)phmi_string;
						*phmi_object_data	= hmi_get_line_counts(phmi_text_rect,
                                                                  &text_info,
                                                                  font_id);
						hmi_get_status 		= TRUE;
						
					}
					else if((*phmi_object_data) == 0U)
					{
						if((phmi_string	!= NULL))
						{
							*phmi_object_data	= hmi_driver_get_string_len(phmi_string,font_id);
							hmi_get_status 		= TRUE;
						}
					}
					else
					{
						*phmi_object_data = 1u;

					}
				}	
				
			}
			#endif
		}
		} 
	#endif
		#if HMI_SEGMENT_MAX>0
		else if(HMI_IS_SEGMENT(hmi_object_id))
		{
		}
		#endif
		/*Get element x,y,w,h,color,alpha,angel*/
	   #if HMI_DXY_CONTAINERS_NUMBER>0
		else if(HMI_IS_DYN_X_CONTAINER(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_X_CONTAINER_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_CONTAINERS_NUMBER)
			{
				*phmi_object_data	= hmi_dyn_xy_container_rect[hmi_object_id].x;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_Y_CONTAINER(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_Y_CONTAINER_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_CONTAINERS_NUMBER)
			{
				*phmi_object_data	= hmi_dyn_xy_container_rect[hmi_object_id].y;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_W_CONTAINER(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_W_CONTAINER_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_CONTAINERS_NUMBER)
			{
				*phmi_object_data	= hmi_dyn_xy_container_rect[hmi_object_id].w;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_H_CONTAINER(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_H_CONTAINER_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_CONTAINERS_NUMBER)
			{
			   *phmi_object_data	= hmi_dyn_xy_container_rect[hmi_object_id].h;
			   hmi_get_status = TRUE;
			}
		}
		#if HMI_DXY_CONTAINERS_SCALE_ALPHA_NUMBER>0
		else if(HMI_IS_DYN_ALPHA_CONTAINER(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_ALPHA_CONTAINER_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_CONTAINERS_NUMBER)
			{
			   *phmi_object_data	= hmi_dyn_xy_container_alpha_scale[hmi_object_id].alpha;
			   hmi_get_status = TRUE;
			}
		}
		#endif
   		#endif
		#if HMI_DXY_BITMAPS_NUMBER > 0
		else if(HMI_IS_DYN_X_BITMAP(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_X_BITMAPS_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_BITMAPS_NUMBER)
			{
			   *phmi_object_data	= hmi_bmp_dyn_xy_rect[hmi_object_id].x;
			   hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_Y_BITMAP(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_Y_BITMAPS_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_BITMAPS_NUMBER)
			{
			   *phmi_object_data= hmi_bmp_dyn_xy_rect[hmi_object_id].y;
			   hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_W_BITMAP(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_W_BITMAPS_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_BITMAPS_NUMBER)
			{
			   *phmi_object_data= hmi_bmp_dyn_xy_rect[hmi_object_id].w;
			   hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_H_BITMAP(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_H_BITMAPS_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_BITMAPS_NUMBER)
			{
			   *phmi_object_data= hmi_bmp_dyn_xy_rect[hmi_object_id].h;
			   hmi_get_status = TRUE;
			}
		}
		#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||		\
		defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_TWLIB)||			\
		defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||		\
		defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)|| 		\
		defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
		else if(HMI_IS_DYN_ALPHA_BITMAP(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_ALPHA_BITMAPS_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_BITMAPS_NUMBER)
			{
			   *phmi_object_data= hmi_bmp_dyn_xy_rect[hmi_object_id].alpha;
			   hmi_get_status = TRUE;
			}
		}
		#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||\
			defined(HMI_GRAPHIC_OPENGLES)||\
			defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||\
			defined(HMI_GRAPHIC_ST)||defined(S6J3200_GRAPHIC))
		else if(HMI_IS_DYN_ANGEL_BITMAP(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_ANGEL_BITMAPS_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_BITMAPS_NUMBER)
			{
				#if HMI_SUPPORT_FLOAT_ANGEL	== HMI_YES
				*phmi_object_data= (HMI_OBJECT_DATA_STR)(HMI_F32_TO_U32(hmi_bmp_dyn_xy_rect[hmi_object_id].angel));
				#else
				*phmi_object_data= (HMI_OBJECT_DATA_STR/*float_32 lq*/)(hmi_bmp_dyn_xy_rect[hmi_object_id].angel);
				#endif
				hmi_get_status = TRUE;
			}
		}
		#endif
		#if (defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC))
		else if(HMI_IS_DYN_BLUR_BITMAP(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_BLUR_BITMAPS_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_BITMAPS_NUMBER)
			{
				*phmi_object_data= (hmi_bmp_dyn_xy_rect[hmi_object_id].attr&HMI_BLUR_FLAG);
				hmi_get_status = TRUE;
			}
		}
		#endif
		#endif
   		#endif
		#if HMI_DXY_PAGES_NUMBER>0U
		else if(HMI_IS_DYN_X_PAGES(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_X_PAGES_INDEX(hmi_object_id);
			if(hmi_object_id<HMI_DXY_PAGES_NUMBER)
			{
				*phmi_object_data= hmi_dxy_page_rect[hmi_object_id].x ;
				hmi_get_status = TRUE; 
			}
		}
		else if(HMI_IS_DYN_Y_PAGES(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_Y_PAGES_INDEX(hmi_object_id);
			if(hmi_object_id<HMI_DXY_PAGES_NUMBER)
			{
				*phmi_object_data= hmi_dxy_page_rect[hmi_object_id].y ;
				 hmi_get_status = TRUE; 
			}
		}
		else if(HMI_IS_DYN_W_PAGES(hmi_object_id))
		 {
			hmi_object_id = HMI_GET_DYN_W_PAGES_INDEX(hmi_object_id);
			if(hmi_object_id<HMI_DXY_PAGES_NUMBER)
			{
				*phmi_object_data= hmi_dxy_page_rect[hmi_object_id].w ;
				hmi_get_status = TRUE; 
			}
		}
		else if(HMI_IS_DYN_H_PAGES(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_H_PAGES_INDEX(hmi_object_id);
			if(hmi_object_id<HMI_DXY_PAGES_NUMBER)
			{
				*phmi_object_data= hmi_dxy_page_rect[hmi_object_id].h ;
				hmi_get_status = TRUE; 
			}
		}
		#endif
		#if HMI_DXY_IMAGELIST_NUMBER>0U
		else if(HMI_IS_DYN_X_IMAGELIST(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_X_IMAGELIST_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_IMAGELIST_NUMBER)
			{
				*phmi_object_data	= hmi_dxy_imagelist_rect[hmi_object_id].x;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_Y_IMAGELIST(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_Y_IMAGELIST_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_IMAGELIST_NUMBER)
			{
				*phmi_object_data	= hmi_dxy_imagelist_rect[hmi_object_id].y;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_W_IMAGELIST(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_W_IMAGELIST_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_IMAGELIST_NUMBER)
			{
				*phmi_object_data	= hmi_dxy_imagelist_rect[hmi_object_id].w;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_H_IMAGELIST(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_H_IMAGELIST_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_IMAGELIST_NUMBER)
			{
				*phmi_object_data	= hmi_dxy_imagelist_rect[hmi_object_id].h;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_ALPHA_IMAGELIST(hmi_object_id))
		{
			#if	(defined(HMI_GRAPHIC_STGLIB)||defined(HMI_GRAPHIC_AGG)||	\
				defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_TWLIB)||defined(S6J3200_GRAPHIC)||		\
				defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||		\
				defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642)) 
			hmi_object_id=HMI_GET_DYN_ALPHA_IMAGELIST_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_IMAGELIST_NUMBER)
			{
				*phmi_object_data	= hmi_dxy_imagelist_rect[hmi_object_id].alpha;
				hmi_get_status = TRUE;
			}
			#endif
		}
		#endif
		#if HMI_DXY_SCROLLBAR_NUMBER>0U
		else if(HMI_IS_DYN_X_SCROLLBAR(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_X_SCROLLBAR_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_SCROLLBAR_NUMBER)
			{
				*phmi_object_data	= hmi_dxy_scrollbar_rect[hmi_object_id].x;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_Y_SCROLLBAR(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_Y_SCROLLBAR_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_SCROLLBAR_NUMBER)
			{
				*phmi_object_data	= hmi_dxy_scrollbar_rect[hmi_object_id].y;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_W_SCROLLBAR(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_W_SCROLLBAR_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_SCROLLBAR_NUMBER)
			{
				*phmi_object_data	= hmi_dxy_scrollbar_rect[hmi_object_id].w;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_H_SCROLLBAR(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_H_SCROLLBAR_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_SCROLLBAR_NUMBER)
			{
				*phmi_object_data	= hmi_dxy_scrollbar_rect[hmi_object_id].h;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_ALPHA_SCROLLBAR(hmi_object_id))
		{
			#if	(defined(HMI_GRAPHIC_STGLIB)||defined(HMI_GRAPHIC_AGG)||	\
				defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_TWLIB)||		\
				defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||	\
				defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||						\
				defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))  	
			hmi_object_id=HMI_GET_DYN_ALPHA_SCROLLBAR_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_SCROLLBAR_NUMBER)
			{
				*phmi_object_data	= hmi_dxy_scrollbar_rect[hmi_object_id].alpha;
				hmi_get_status = TRUE;
			}
			#endif
		}
		#endif
		#if HMI_DXY_BUTTON_NUMBER>0
		else if(HMI_IS_DYN_X_BUTTON(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_X_BUTTON_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_BUTTON_NUMBER)
			{
				*phmi_object_data	= hmi_dxy_button_rect[hmi_object_id].x;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_Y_BUTTON(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_Y_BUTTON_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_BUTTON_NUMBER)
			{
				*phmi_object_data	= hmi_dxy_button_rect[hmi_object_id].y;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_W_BUTTON(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_W_BUTTON_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_BUTTON_NUMBER)
			{
				*phmi_object_data	= hmi_dxy_button_rect[hmi_object_id].w;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_H_BUTTON(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_H_BUTTON_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_BUTTON_NUMBER)
			{
				*phmi_object_data	= hmi_dxy_button_rect[hmi_object_id].h;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_ALPHA_BUTTON(hmi_object_id))
		{
			#if	(defined(HMI_GRAPHIC_STGLIB)||defined(HMI_GRAPHIC_AGG)||	\
				defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_TWLIB)||		\
				defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||		\
				defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||		\
				defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))  	
			hmi_object_id=HMI_GET_DYN_ALPHA_BUTTON_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_BUTTON_NUMBER)
			{
				*phmi_object_data	= hmi_dxy_button_rect[hmi_object_id].alpha;
				hmi_get_status = TRUE;
			}
			#endif
		}
		#endif
		#if HMI_DYN_XY_EDIT_TEXTS_NUMBER > 0
		else if(HMI_IS_DYN_X_DTEXT(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_X_DTEXT_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DYN_XY_EDIT_TEXTS_NUMBER)
			{
				*phmi_object_data= hmi_dyn_xy_edit_text_prop_table[hmi_object_id].text_rect.x;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_Y_DTEXT(hmi_object_id))
		{
			 hmi_object_id=HMI_GET_DYN_Y_DTEXT_ID_INDEX(hmi_object_id);
			 if(hmi_object_id < HMI_DYN_XY_EDIT_TEXTS_NUMBER)
			 {
			    *phmi_object_data= hmi_dyn_xy_edit_text_prop_table[hmi_object_id].text_rect.y;
			    hmi_get_status = TRUE;
			 }
		}
		else if(HMI_IS_DYN_W_DTEXT(hmi_object_id))
		{
			 hmi_object_id=HMI_GET_DYN_W_DTEXT_ID_INDEX(hmi_object_id);
			 if(hmi_object_id < HMI_DYN_XY_EDIT_TEXTS_NUMBER)
			 {
			    *phmi_object_data = hmi_dyn_xy_edit_text_prop_table[hmi_object_id].text_rect.w;
			    hmi_get_status = TRUE;
			 }
		}
		else if(HMI_IS_DYN_H_DTEXT(hmi_object_id))
		{
			 hmi_object_id=HMI_GET_DYN_H_DTEXT_ID_INDEX(hmi_object_id);
			 if(hmi_object_id < HMI_DYN_XY_EDIT_TEXTS_NUMBER)
			 {
			    *phmi_object_data = hmi_dyn_xy_edit_text_prop_table[hmi_object_id].text_rect.h;
			    hmi_get_status = TRUE;
			 }
		}
		else if(HMI_IS_DYN_DCOLOR_DTEXT(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_DCOLOR_DTEXT_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DYN_XY_EDIT_TEXTS_NUMBER)
			{
				*phmi_object_data = hmi_dyn_xy_edit_text_prop_table[hmi_object_id].color;
				hmi_get_status = TRUE;
			}
		}
		#endif
		#if HMI_UNEDIT_TEXTS_DYN_XY_NUMBER > 0
		else if(HMI_IS_DYN_X_UNEDIT(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_X_STEXT_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_UNEDIT_TEXTS_DYN_XY_NUMBER)
			{
				*phmi_object_data = hmi_dyn_xy_unedit_text_prop_table[hmi_object_id].text_rect.x;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_Y_UNEDIT(hmi_object_id))
		{
			 hmi_object_id=HMI_GET_DYN_Y_STEXT_ID_INDEX(hmi_object_id);
			 if(hmi_object_id < HMI_UNEDIT_TEXTS_DYN_XY_NUMBER)
			 {
			    *phmi_object_data= hmi_dyn_xy_unedit_text_prop_table[hmi_object_id].text_rect.y;
			    hmi_get_status = TRUE;
			 }
		}
		else if(HMI_IS_DYN_W_UNEDIT(hmi_object_id))
		{
			 hmi_object_id=HMI_GET_DYN_W_STEXT_ID_INDEX(hmi_object_id);
			 if(hmi_object_id < HMI_UNEDIT_TEXTS_DYN_XY_NUMBER)
			 {
			    *phmi_object_data = hmi_dyn_xy_unedit_text_prop_table[hmi_object_id].text_rect.w;
			    hmi_get_status = TRUE;
			 }
		}
		else if(HMI_IS_DYN_H_UNEDIT(hmi_object_id))
		{
			 hmi_object_id=HMI_GET_DYN_H_STEXT_ID_INDEX(hmi_object_id);
			 if(hmi_object_id < HMI_UNEDIT_TEXTS_DYN_XY_NUMBER)
			 {
			    *phmi_object_data = hmi_dyn_xy_unedit_text_prop_table[hmi_object_id].text_rect.h;
			    hmi_get_status = TRUE;
			 }
		}
		else if(HMI_IS_DYN_COLOR_UNEDIT(hmi_object_id))
		{
			 hmi_object_id=HMI_GET_DYN_DCOLOR_STEXT_ID_INDEX(hmi_object_id);
			 if(hmi_object_id < HMI_UNEDIT_TEXTS_DYN_XY_NUMBER)
			 {
			    *phmi_object_data= hmi_dyn_xy_unedit_text_prop_table[hmi_object_id].color;
			    hmi_get_status = TRUE;
			 }
		}
		#endif
		
		#if HMI_DYN_FILL_PAGES_NUMBER > 0
		else if(HMI_IS_DYN_X_NFILL(hmi_object_id))
		{
			 hmi_object_id=HMI_GET_DYN_X_NFILL_ID_INDEX(hmi_object_id);

			 if(hmi_object_id < HMI_DYN_FILL_PAGES_NUMBER)
			 {
			    *phmi_object_data           = hmi_fills_dyn_xy_rect[hmi_object_id].x;
			    hmi_get_status = TRUE;
			 }
		}
		else if(HMI_IS_DYN_Y_NFILL(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_Y_NFILL_ID_INDEX(hmi_object_id);

			if(hmi_object_id < HMI_DYN_FILL_PAGES_NUMBER)
			{
				*phmi_object_data          = hmi_fills_dyn_xy_rect[hmi_object_id].y;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_W_NFILL(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_W_NFILL_ID_INDEX(hmi_object_id);

			if(hmi_object_id< HMI_DYN_FILL_PAGES_NUMBER)
			{
				*phmi_object_data         = hmi_fills_dyn_xy_rect[hmi_object_id].w;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_H_NFILL(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_H_NFILL_ID_INDEX(hmi_object_id);

			if(hmi_object_id < HMI_DYN_FILL_PAGES_NUMBER)
			{
				*phmi_object_data           = hmi_fills_dyn_xy_rect[hmi_object_id].h;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_COLOR_NFILL(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_COLOR_NFILL_ID_INDEX(hmi_object_id);

			if(hmi_object_id < HMI_DYN_FILL_PAGES_NUMBER)
			{
				*phmi_object_data           = hmi_fills_dyn_prop_table[hmi_object_id].color;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_Z_NFILL(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_Z_NFILL_ID_INDEX(hmi_object_id);

			if(hmi_object_id < HMI_DYN_FILL_PAGES_NUMBER)
			{
				*phmi_object_data           = hmi_fills_dyn_xy_rect[hmi_object_id].z;
				hmi_get_status = TRUE;
			}
		}
		#endif
		#if HMI_DYN_GFILL_NUMBER>0U
		else if(HMI_IS_DYN_X_GFILL(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_X_GFILL_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DYN_GFILL_NUMBER)
			{
				*phmi_object_data           = hmi_gradient_dxy_fill_rect[hmi_object_id].x;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_Y_GFILL(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_Y_GFILL_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DYN_GFILL_NUMBER)
			{
				*phmi_object_data           = hmi_gradient_dxy_fill_rect[hmi_object_id].y;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_W_GFILL(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_W_GFILL_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DYN_GFILL_NUMBER)
			{
				*phmi_object_data           = hmi_gradient_dxy_fill_rect[hmi_object_id].w;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_H_GFILL(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_H_GFILL_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DYN_GFILL_NUMBER)
			{
				*phmi_object_data           = hmi_gradient_dxy_fill_rect[hmi_object_id].h;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_C1_GFILL(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_C1_GFILL_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DYN_GFILL_NUMBER)
			{
				*phmi_object_data           = hmi_gradient_dxy_fill_table[hmi_object_id].color1;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_C2_GFILL(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_C2_GFILL_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DYN_GFILL_NUMBER)
			{
				*phmi_object_data           = hmi_gradient_dxy_fill_table[hmi_object_id].color2;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_Z_GFILL(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_Z_GFILL_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DYN_GFILL_NUMBER)
			{
				*phmi_object_data           = hmi_gradient_dxy_fill_rect[hmi_object_id].z;
				hmi_get_status = TRUE;
			}
		}
		#endif
		#if HMI_DXY_CUBE_NUMBER>0U
		else if(HMI_IS_DYN_X_CUBE(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_X_CUBE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_CUBE_NUMBER)
			{
				*phmi_object_data           = hmi_cubes_dyn_xy_rect[hmi_object_id].cube_rect.x;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_Y_CUBE(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_Y_CUBE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_CUBE_NUMBER)
			{
				*phmi_object_data           = hmi_cubes_dyn_xy_rect[hmi_object_id].cube_rect.y;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_W_CUBE(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_W_CUBE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_CUBE_NUMBER)
			{
				*phmi_object_data           = hmi_cubes_dyn_xy_rect[hmi_object_id].cube_rect.w;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_H_CUBE(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_H_CUBE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_CUBE_NUMBER)
			{
				*phmi_object_data           = hmi_cubes_dyn_xy_rect[hmi_object_id].cube_rect.h;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_BUMP_CUBE(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_BUMP_CUBE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_CUBE_NUMBER)
			{
				*phmi_object_data           = hmi_cubes_dyn_xy_rect[hmi_object_id].bump;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_ANGEL_CUBE(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_ANGEL_CUBE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_CUBE_NUMBER)
			{
				*phmi_object_data           = (HMI_OBJECT_DATA_STR)(hmi_cubes_dyn_xy_rect[hmi_object_id].angel);
				hmi_get_status = TRUE;
			}
		}
		#endif
		#if HMI_DXY_SPLINE_NUMBER > 0
		else if(HMI_IS_DYN_X_SPLINE(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_X_SPLINE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_SPLINE_NUMBER)
			{
				*phmi_object_data= hmi_dxy_spline_rect[hmi_object_id].x;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_Y_SPLINE	(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_Y_SPLINE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_SPLINE_NUMBER)
			{
				*phmi_object_data= hmi_dxy_spline_rect[hmi_object_id].y;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_W_SPLINE(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_W_SPLINE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_SPLINE_NUMBER)
			{
				*phmi_object_data= hmi_dxy_spline_rect[hmi_object_id].w;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_H_SPLINE(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_H_SPLINE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_SPLINE_NUMBER)
			{
				*phmi_object_data= hmi_dxy_spline_rect[hmi_object_id].h;
				hmi_get_status = TRUE;
			}
		}
		else if(HMI_IS_DYN_SCALE_SPLINE(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_SCALE_SPLINE_ID_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_SPLINE_NUMBER)
			{
				spline_scale	= hmi_dxy_spline_scale[hmi_object_id];
				*phmi_object_data= (HMI_OBJECT_DATA_STR)(HMI_F32_TO_U32(spline_scale));
				hmi_get_status = TRUE;
			}
		}
		#endif
		#if HMI_DXY_CUSTOM_CNT > 0U
		else if(HMI_IS_DYN_XY_CUSTOM_PROPERTY(hmi_object_id))
		{
			if(HMI_IS_DYN_X_CUSTOM(hmi_object_id))
			{
				hmi_object_id = HMI_GET_DYN_X_CUSTOM_ID_INDEX(hmi_object_id);
				if(hmi_object_id < HMI_DXY_CUSTOM_CNT)
				{
					search_success = hmi_engine_get_dyn_custom_index(hmi_object_id,&custom_index);
					if(search_success == TRUE)
					{
						object_index	= hmi_object_id -hmi_dxy_custom_widget_info[custom_index].begin;
						if(((hmi_dxy_custom_widget_info[custom_index].attr_fun.attr) & HMI_XYZWH_F) == 0U)	/*2023 07 10.Custom x,y,z,w,h is float*/
						{
							*phmi_object_data	= ((HMI_CUSTOM_XYZWH_STR*)(hmi_dxy_custom_widget_info[custom_index].pxyzwh))[object_index].x;
						}
						else
						{
							
							*phmi_object_data	= (HMI_OBJECT_DATA_STR)(HMI_F32_TO_U32(((HMI_CUSTOM_XYZWH_FLOAT_STR*)(hmi_dxy_custom_widget_info[custom_index].pxyzwh))[object_index].x));						
						}
					}
				}
			}
			else if(HMI_IS_DYN_Y_CUSTOM(hmi_object_id))
			{
				hmi_object_id = HMI_GET_DYN_Y_CUSTOM_ID_INDEX(hmi_object_id);
				if(hmi_object_id < HMI_DXY_CUSTOM_CNT)
				{
					search_success =hmi_engine_get_dyn_custom_index(hmi_object_id,&custom_index);
					if(search_success == TRUE)
					{
						object_index	= hmi_object_id -hmi_dxy_custom_widget_info[custom_index].begin;
						//*phmi_object_data	= hmi_dxy_custom_widget_info[custom_index].pxyzwh[object_index].y;
						if((hmi_dxy_custom_widget_info[custom_index].attr_fun.attr & HMI_XYZWH_F) == 0U)	/*2023 07 10.Custom x,y,z,w,h is float*/
						{
							*phmi_object_data	= ((HMI_CUSTOM_XYZWH_STR*)(hmi_dxy_custom_widget_info[custom_index].pxyzwh))[object_index].y;
						}
						else
						{
							
							*phmi_object_data	= (HMI_OBJECT_DATA_STR)(HMI_F32_TO_U32(((HMI_CUSTOM_XYZWH_FLOAT_STR*)(hmi_dxy_custom_widget_info[custom_index].pxyzwh))[object_index].y));						
						}
					}
				}
			}
			else if(HMI_IS_DYN_Z_CUSTOM(hmi_object_id))
			{
				hmi_object_id = HMI_GET_DYN_Z_CUSTOM_ID_INDEX(hmi_object_id);
				if(hmi_object_id < HMI_DXY_CUSTOM_CNT)
				{
					search_success =hmi_engine_get_dyn_custom_index(hmi_object_id,&custom_index);
					if(search_success == TRUE)
					{
						object_index	= hmi_object_id -hmi_dxy_custom_widget_info[custom_index].begin;
						//*phmi_object_data	= hmi_dxy_custom_widget_info[custom_index].pxyzwh[object_index].z;
						if((hmi_dxy_custom_widget_info[custom_index].attr_fun.attr & HMI_XYZWH_F) == 0U)	/*2023 07 10.Custom x,y,z,w,h is float*/
						{
							*phmi_object_data	= ((HMI_CUSTOM_XYZWH_STR*)(hmi_dxy_custom_widget_info[custom_index].pxyzwh))[object_index].z;
						}
						else
						{
							*phmi_object_data	= (HMI_OBJECT_DATA_STR)(HMI_F32_TO_U32(((HMI_CUSTOM_XYZWH_FLOAT_STR*)(hmi_dxy_custom_widget_info[custom_index].pxyzwh))[object_index].z));						
						}

					}
				}
			}
			else if(HMI_IS_DYN_W_CUSTOM(hmi_object_id))
			{
				hmi_object_id = HMI_GET_DYN_W_CUSTOM_ID_INDEX(hmi_object_id);
				if(hmi_object_id < HMI_DXY_CUSTOM_CNT)
				{
					search_success =hmi_engine_get_dyn_custom_index(hmi_object_id,&custom_index);
					if(search_success == TRUE)
					{
						object_index	= hmi_object_id -hmi_dxy_custom_widget_info[custom_index].begin;
						//*phmi_object_data	= hmi_dxy_custom_widget_info[custom_index].pxyzwh[object_index].w;
						if((hmi_dxy_custom_widget_info[custom_index].attr_fun.attr & HMI_XYZWH_F) == 0U)	/*2023 07 10.Custom x,y,z,w,h is float*/
						{
							*phmi_object_data	= ((HMI_CUSTOM_XYZWH_STR*)(hmi_dxy_custom_widget_info[custom_index].pxyzwh))[object_index].w;
						}
						else
						{
							*phmi_object_data	= (HMI_OBJECT_DATA_STR)(HMI_F32_TO_U32(((HMI_CUSTOM_XYZWH_FLOAT_STR*)(hmi_dxy_custom_widget_info[custom_index].pxyzwh))[object_index].w));						
						}
					}
				}
			}
			else if(HMI_IS_DYN_H_CUSTOM(hmi_object_id))
			{
				hmi_object_id = HMI_GET_DYN_H_CUSTOM_ID_INDEX(hmi_object_id);
				if(hmi_object_id < HMI_DXY_CUSTOM_CNT)
				{
					search_success =hmi_engine_get_dyn_custom_index(hmi_object_id,&custom_index);
					if(search_success == TRUE)
					{
						object_index	= hmi_object_id -hmi_dxy_custom_widget_info[custom_index].begin;
						//*phmi_object_data	= hmi_dxy_custom_widget_info[custom_index].pxyzwh[object_index].h;
						if((hmi_dxy_custom_widget_info[custom_index].attr_fun.attr & HMI_XYZWH_F) == 0U)	/*2023 07 10.Custom x,y,z,w,h is float*/
						{
							*phmi_object_data	= ((HMI_CUSTOM_XYZWH_STR*)(hmi_dxy_custom_widget_info[custom_index].pxyzwh))[object_index].h;
						}
						else
						{
							*phmi_object_data	= (HMI_OBJECT_DATA_STR)(HMI_F32_TO_U32(((HMI_CUSTOM_XYZWH_FLOAT_STR*)(hmi_dxy_custom_widget_info[custom_index].pxyzwh))[object_index].h));						
						}
					}
				}
			}
			else if(HMI_IS_DYN_P1_CUSTOM(hmi_object_id))
			{
				hmi_object_id = HMI_GET_DYN_P1_CUSTOM_ID_INDEX(hmi_object_id);
				if(hmi_object_id < HMI_DXY_CUSTOM_CNT)
				{
					search_success =hmi_engine_get_dyn_custom_index(hmi_object_id,&custom_index);
					if(search_success == TRUE)
					{
						object_index	= hmi_object_id -hmi_dxy_custom_widget_info[custom_index].begin;
						if(hmi_dxy_custom_widget_info[custom_index].pp1 !=NULL)
						{
							if((hmi_dxy_custom_widget_info[custom_index].attr_fun.attr & HMI_P1_F) != 0U)
							{
								pcustom_p_float	= hmi_dxy_custom_widget_info[custom_index].pp1;
								*phmi_object_data	= (HMI_OBJECT_DATA_STR)(HMI_F32_TO_U32(pcustom_p_float[object_index]));
							}
							else
							{
								
								pcustom_p_int	= hmi_dxy_custom_widget_info[custom_index].pp1;
								*phmi_object_data	= pcustom_p_int[object_index];
							}
							
						}

					}
				}
			}
			else if(HMI_IS_DYN_P2_CUSTOM(hmi_object_id))
			{
				hmi_object_id = HMI_GET_DYN_P2_CUSTOM_ID_INDEX(hmi_object_id);
				if(hmi_object_id < HMI_DXY_CUSTOM_CNT)
				{
					search_success =hmi_engine_get_dyn_custom_index(hmi_object_id,&custom_index);
					if(search_success == TRUE)
					{
						object_index	= hmi_object_id -hmi_dxy_custom_widget_info[custom_index].begin;
						
						if(hmi_dxy_custom_widget_info[custom_index].pp2 !=NULL)
						{
							if((hmi_dxy_custom_widget_info[custom_index].attr_fun.attr & HMI_P2_F) != 0U)
							{
								pcustom_p_float	= hmi_dxy_custom_widget_info[custom_index].pp2;
								hmi_object_data_f	= pcustom_p_float[object_index];
								*phmi_object_data	= (HMI_OBJECT_DATA_STR)(HMI_F32_TO_U32(hmi_object_data_f));
								
							}
							else
							{
								pcustom_p_int	= hmi_dxy_custom_widget_info[custom_index].pp2;
								*phmi_object_data	= pcustom_p_int[object_index];
							}
						}

					}
				}
			}
			else if(HMI_IS_DYN_P3_CUSTOM(hmi_object_id))
			{
				hmi_object_id = HMI_GET_DYN_P3_CUSTOM_ID_INDEX(hmi_object_id);
				if(hmi_object_id < HMI_DXY_CUSTOM_CNT)
				{
					search_success =hmi_engine_get_dyn_custom_index(hmi_object_id,&custom_index);
					if(search_success == TRUE)
					{
						object_index	= hmi_object_id -hmi_dxy_custom_widget_info[custom_index].begin;
						if(hmi_dxy_custom_widget_info[custom_index].pp3 !=NULL)
						{
							if((hmi_dxy_custom_widget_info[custom_index].attr_fun.attr & HMI_P3_F) != 0U)
							{
								pcustom_p_float	= hmi_dxy_custom_widget_info[custom_index].pp3;
								hmi_object_data_f	= pcustom_p_float[object_index];
								*phmi_object_data	= (HMI_OBJECT_DATA_STR)(HMI_F32_TO_U32(hmi_object_data_f));
							}
							else
							{
								pcustom_p_int	= hmi_dxy_custom_widget_info[custom_index].pp3;
								*phmi_object_data	= pcustom_p_int[object_index];
							}
							
						}
					}
				}
			}
			else 
			{
			}
		}
		#endif
		else
		{
		}
   } 
   else
   {
		#if (HMI_SCROLL_TEXT_SUPPORT != 0) && (HMI_DYN_EDIT_TEXTS_NUMBER > 0)
		if(HMI_IS_EDIT_DXY_TEXT_SCROLL(hmi_object_id))
		{
			hmi_object_id=HMI_GET_SCROLL_TEXT_ID_INDEX(hmi_object_id);

			if(hmi_object_id < HMI_DYN_EDIT_TEXTS_NUMBER)
			{
				HMI_TEXT_PROP_STR CONST * phmi_dyn_text_info = &hmi_edit_text_table[hmi_object_id];
				HMI_CHAR_STR            * hmi_scroll_text_offset=NULL;
				if((phmi_dyn_text_info->properties & HMI_TEXT_PROP_SCROLABLE) != 0)
				{
					hmi_scroll_text_offset  = &((HMI_CHAR_STR *)(phmi_dyn_text_info->hmi_string))[phmi_dyn_text_info->length+1];
					#if HMI_FONT_CODE==HMI_FONT_CODE_UNICODE
					*phmi_object_data    = *hmi_scroll_text_offset;
					#else
					*phmi_object_data    = *hmi_scroll_text_offset++;
					(*phmi_object_data)<<=8;
					*phmi_object_data  += *hmi_scroll_text_offset;
					#endif
					hmi_get_status = TRUE;
				}
			}		
		}
		else
		#endif	  	
		/*Set dyn timer elapse*/
		#if HMI_DYN_TIMER_ACTION_DURATION_NUMBER>0
		if(HMI_IS_TIMER_ACTION_D_DURATION(hmi_object_id))
		{
			hmi_object_id=HMI_GET_TIMER_ACTION_D_DURATION_ID_INDEX(hmi_object_id);
			time_angel=hmi_get_timer_s_e(hmi_object_id,HMI_SET_ELAPSE);
			*phmi_object_data=(HMI_OBJECT_DATA_STR)(HMI_F32_TO_U32(time_angel));
			hmi_get_status = TRUE;
		}
		else
		#endif
		/*Set dyn timer start and duration*/
		#if HMI_DYN_TIMER_ACTION_DURATION_NUMBER>0
		if(HMI_IS_DYN_TIMER_S_E(hmi_object_id))
		{
			if(HMI_IS_DYN_TIMER_START(hmi_object_id))
			{
				hmi_object_id=HMI_GET_DYN_TIMER_S_INDEX(hmi_object_id);
				time_angel=hmi_get_timer_s_e(hmi_object_id,HMI_SET_START);
				*phmi_object_data= (HMI_OBJECT_DATA_STR)(HMI_F32_TO_U32(time_angel));
				hmi_get_status = TRUE;
			}
			else /*dyn timer end*/
			{
				hmi_object_id=HMI_GET_DYN_TIMER_E_INDEX(hmi_object_id);
				time_angel=hmi_get_timer_s_e(hmi_object_id,HMI_SET_ELAPSE);
				*phmi_object_data= (HMI_OBJECT_DATA_STR)(HMI_F32_TO_U32(time_angel));
				hmi_get_status = TRUE;
			}						
		}
		else
		#endif
		/*Set dyn pos start and end*/
		#if HMI_ANIM_DYN_SET_POS_NUMBER>0
		if(HMI_IS_DYN_POS_S_E(hmi_object_id))
		{
			if(HMI_IS_DYN_POS_START(hmi_object_id))
			{
				hmi_object_id=HMI_GET_DYN_POS_START(hmi_object_id);
				*phmi_object_data=hmi_get_dyn_pos_s_e(hmi_object_id,TRUE);
				hmi_get_status = TRUE;
			}
			else /*end*/
			{
				hmi_object_id=HMI_GET_DYN_POS_END(hmi_object_id);
				*phmi_object_data=hmi_get_dyn_pos_s_e(hmi_object_id,FALSE);
				hmi_get_status = TRUE;
			}
		}
		else
		#endif
		/*Set dyn width heigh start and end*/
		#if HMI_ANIM_DYN_SET_W_H_NUMBER>0
		if(HMI_IS_DYN_WH_S_E(hmi_object_id))
		{
			if(HMI_IS_DYN_WH_START(hmi_object_id))
			{
				hmi_object_id=HMI_GET_DYN_WH_START(hmi_object_id);
				*phmi_object_data=hmi_get_dyn_wh_s_e(hmi_object_id,TRUE);
				hmi_get_status = TRUE;
			}
			else /*end*/
			{
				hmi_object_id=HMI_GET_DYN_WH_END(hmi_object_id);
				*phmi_object_data=hmi_get_dyn_wh_s_e(hmi_object_id,FALSE);
				hmi_get_status = TRUE;
			}
		}
		else
		#endif
		/*Set dyn fcolor start and end*/
		#if HMI_ANIM_DYN_SET_FOR_COLOR_NUMBER>0
		if(HMI_IS_DYN_FCOLOR_S_E(hmi_object_id))
		{
			if(HMI_IS_DYN_FCOLOR_START(hmi_object_id))
			{
				hmi_object_id=HMI_GET_DYN_FCOLOR_START(hmi_object_id);
				*phmi_object_data=hmi_get_dyn_fcolor_s_e(hmi_object_id,TRUE);
				hmi_get_status = TRUE;
			}
			else /*end*/
			{
				hmi_object_id=HMI_GET_DYN_FCOLOR_END(hmi_object_id);
				*phmi_object_data=hmi_get_dyn_fcolor_s_e(hmi_object_id,FALSE);
				hmi_get_status = TRUE;
			}
		}
		else
		#endif
		/*Set dyn image list start and end*/
		#if HMI_ANIM_DYN_SET_IMAGELIST_INDEX_NUMBER>0
		if(HMI_IS_DYN_IMGLST_S_E(hmi_object_id))
		{
			if(HMI_IS_DYN_IMGLST_START(hmi_object_id))
			{
				hmi_object_id=HMI_GET_DYN_IMGLST_START(hmi_object_id);
				*phmi_object_data=hmi_get_dyn_imglist_s_e(hmi_object_id,TRUE);
				hmi_get_status = TRUE;
			}
			else /*end*/
			{
				hmi_object_id=HMI_GET_DYN_IMGLST_END(hmi_object_id);
				*phmi_object_data=hmi_get_dyn_imglist_s_e(hmi_object_id,FALSE);
				hmi_get_status = TRUE;
			}
		}
		else
		#endif
		/*Set dyn alpha start and end*/
		#if HMI_ANIM_DYN_SET_ALPHA_NUMBER>0
		if(HMI_IS_DYN_ALPHA_S_E(hmi_object_id))
		{
			if(HMI_IS_DYN_ALPHA_START(hmi_object_id))
			{
				hmi_object_id=HMI_GET_DYN_ALPHA_START(hmi_object_id);
				*phmi_object_data=hmi_get_dyn_alpha_s_e(hmi_object_id,TRUE);
				hmi_get_status = TRUE;
			}
			else /*end*/
			{
				hmi_object_id=HMI_GET_DYN_ALPHA_END(hmi_object_id);
				*phmi_object_data=hmi_get_dyn_alpha_s_e(hmi_object_id,FALSE);
				hmi_get_status = TRUE;
			}
		}
		else
		#endif
		/*Set dyn angel start and end*/
		#if HMI_ANIM_DYN_SET_ANGEL_NUMBER>0
		if(HMI_IS_DYN_ANGEL_S_E(hmi_object_id))
		{
			if(HMI_IS_DYN_ANGEL_START(hmi_object_id))
			{
				hmi_object_id=HMI_GET_DYN_ANGEL_START(hmi_object_id);
				
				#if	(defined(HMI_GRAPHIC_STGLIB)||defined(HMI_GRAPHIC_AGG)||	\
					defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513)||		\
					defined(HMI_GRAPHIC_OPENGLES)||	defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||	\
					defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC))	
				
				time_angel=hmi_get_dyn_angel_s_e(hmi_object_id,TRUE);

				#if HMI_SUPPORT_FLOAT_ANGEL	== HMI_YES
				*phmi_object_data	= HMI_F32_TO_U32(time_angel);
				#else
				*phmi_object_data	= time_angel;
				#endif

				#else
				time_angel			= 0;/*tw not support*/
				*phmi_object_data	= time_angel;
				#endif

				hmi_get_status		= TRUE;
			}
			else /*end*/
			{
				hmi_object_id		= HMI_GET_DYN_ANGEL_END(hmi_object_id);
				time_angel			= hmi_get_dyn_angel_s_e(hmi_object_id,FALSE);
				#if HMI_SUPPORT_FLOAT_ANGEL	== HMI_YES
				*phmi_object_data	= HMI_F32_TO_U32(time_angel);
				#else
				*phmi_object_data	= (time_angel);
				#endif			
				hmi_get_status		= TRUE;
			}
		}
		#endif
		{
		}
   }

   return	hmi_get_status;
}
#endif
#endif

#if HMI_DYN_EDIT_TEXTS_NUMBER+HMI_STATIC_TEXTS_NUMBER > 0
#if  HMI_GET_TEXT_FUNC==YES
HMI_CHAR_STR CONST * hmi_engine_get_text(HMI_OBJECT_ID_STR hmi_object_id)
{
	HMI_CHAR_STR CONST * phmi_string = 0;//0U;	-cxy
	#if (HMI_STATIC_TEXTS_NUMBER > 0U)
	HMI_OBJECT_ID_STR 	 hmi_object_index =0;
	HMI_CHAR_STR		**pp_str_list	= NULL;
	#endif
	#if HMI_DYN_EDIT_TEXTS_NUMBER/*uneditable text*/ > 0 
	if(HMI_IS_DYN_TEXTS(hmi_object_id))
	{		
		hmi_object_id = HMI_GET_DYN_TEXTS_INDEX(hmi_object_id);
		if(hmi_object_id<HMI_DYN_EDIT_TEXTS_NUMBER)	
		{
			phmi_string    = (HMI_CHAR_STR *)hmi_edit_text_table[hmi_object_id].hmi_string;
		}
	}
	else
	#endif
 	#if HMI_STATIC_TEXTS_NUMBER/*uneditable text*/ > 0 
	if(HMI_IS_STATIC_TEXTS(hmi_object_id)) 
	{			  
		hmi_object_index = HMI_GET_STATIC_TEXT_ID_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_STATIC_TEXTS_NUMBER)
		{
			pp_str_list=(HMI_CHAR_STR **)(hmi_unedit_text_table[hmi_object_index].hmi_string);
			if(HMI_IS_NB_LANGUAGE(hmi_cur_language))
			{
				hmi_object_index=HMI_GET_NB_LANGUAGE_ID_INDEX(hmi_cur_language);
				if(hmi_object_index < HMI_LANGUAGE_NUMBER)
				{									 
				 	phmi_string=pp_str_list[hmi_object_index];
				}
			}
		}
	} 
	 else
	#endif
	{

	}
	return(phmi_string);
}
#endif
#endif

/*
intersection p_r1 and p_r2 zont to phmi_temp_rect
*/
#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)|| \
	defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)|| \
	defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)|| \
	defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))
void hmi_get_intersec_rect(HMI_RECT_STR CONST *p_r1,HMI_RECT_STR CONST *p_r2,HMI_RECT_STR *phmi_temp_rect)
{		
	HMI_X_STR	min_right	= 0;
	HMI_Y_STR	min_bottom	= 0;
		
	if((p_r1 != NULL)&&(p_r2 != NULL)&&(phmi_temp_rect != NULL))
	{	
		phmi_temp_rect->x	= HMI_INVALID_COOR;
		phmi_temp_rect->y	= HMI_INVALID_COOR;
		phmi_temp_rect->w	= 0;
		phmi_temp_rect->h	= 0;
		
		if((p_r1->x) > (p_r2->x))
		{
			phmi_temp_rect->x	= p_r1->x;
		}
		else
		{
			phmi_temp_rect->x	= p_r2->x;
		}
		if((p_r1->y) > (p_r2->y))
		{
			phmi_temp_rect->y	= p_r1->y;
		}
		else
		{
			phmi_temp_rect->y	= p_r2->y;
		}		
		if((p_r1->x + p_r1->w) < (p_r2->x + p_r2->w))
		{
			min_right	= p_r1->x + p_r1->w;
		}
		else
		{
			min_right	= p_r2->x + p_r2->w;
		}
		if((p_r1->y + p_r1->h)<(p_r2->y + p_r2->h))
		{
			min_bottom	= p_r1->y + p_r1->h;
		}
		else
		{
			min_bottom	= p_r2->y + p_r2->h;
		}
		if(phmi_temp_rect->x <= min_right)
		{
			phmi_temp_rect->w	= min_right-phmi_temp_rect->x;
		}
		else
		{
			phmi_temp_rect->x	= HMI_INVALID_COOR;
			phmi_temp_rect->y	= HMI_INVALID_COOR;
			phmi_temp_rect->w	= 0;
			phmi_temp_rect->h	= 0;
		}
		if(phmi_temp_rect->y <= min_bottom)
		{
			phmi_temp_rect->h	= min_bottom-phmi_temp_rect->y;
		}
		else
		{
			phmi_temp_rect->x	= HMI_INVALID_COOR;
			phmi_temp_rect->y	= HMI_INVALID_COOR;
			phmi_temp_rect->w	= 0;
			phmi_temp_rect->h	= 0;
		}
	}	
}

/*
p_node_pos intersec with p_dirty[]

*/
void hmi_get_intersec_rect_depth(HMI_RECT_STR CONST *p_node_screen_pos,U08 depth,HMI_RECT_STR *p_dirty,HMI_RECT_STR *p_insect)
{	
	#if  defined(HMI_GRAPHIC_ST7513)
	UINT8			gdi_layer = 0;
	#endif

	if((p_node_screen_pos != NULL)&&(p_dirty != NULL)&&(p_insect != NULL))
	{
		#if  (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||	\
				defined(HMI_GRAPHIC_OPENGLES)||					\
				defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||					\
				defined(HMI_GRAPHIC_ST)||defined(S6J3200_GRAPHIC))		
		//gdi_layer	= get_layer_from_depth(depth);//removed by pxguo
		if(depth < HMI_LAYER_MAX_CNT)
		{
			hmi_get_intersec_rect(p_node_screen_pos,
				&(p_dirty[depth]/*&(p_dirty[gdi_layer]*/),
				p_insect);
		}
		else
		{
			hmi_get_intersec_rect(p_node_screen_pos,
				&(p_dirty[0]/*&(p_dirty[gdi_layer]*/),
				p_insect);
		}
		#endif
		
		#if  defined(HMI_GRAPHIC_ST7513)
		gdi_layer	= 0;		
		hmi_get_intersec_rect(p_node_screen_pos,
						&p_dirty[gdi_layer],
						p_insect);		
		#endif
	}	
}


#endif

/*
get screen coordinate
*/
void hmi_get_object_screen_coor(HMI_RECT_STR CONST *pfarther_rect,HMI_RECT_STR CONST *pobject_rect,HMI_RECT_STR *phmi_temp_rect)
{
	
	phmi_temp_rect->x=pfarther_rect->x+pobject_rect->x;
	phmi_temp_rect->y=pfarther_rect->y+pobject_rect->y;
	phmi_temp_rect->w=pobject_rect->w;
	phmi_temp_rect->h=pobject_rect->h;
        
}

/*
union pfarther_rect and  pdirty_rect to pdirty_rect zone
*/
#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)|| \
	defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)|| \
	defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)|| \
	defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))
void hmi_set_dirty_zone(HMI_RECT_STR *pfarther_rect,HMI_RECT_STR *pdirty_rect)
{
	HMI_X_STR	max_right		= 0;
	HMI_Y_STR	max_bottom		= 0;
	HMI_X_STR	parent_right	= 0;
	HMI_Y_STR	parent_bottom	= 0;
	HMI_X_STR	dirty_right		= 0;
	HMI_Y_STR	dirty_bottom	= 0;
	
	if((pfarther_rect != NULL)&&(pdirty_rect != NULL))
	{
		if((pdirty_rect->x == HMI_INVALID_COOR)&& 
			(pdirty_rect->y == HMI_INVALID_COOR)&& 
			(pdirty_rect->w == 0)&&
			(pdirty_rect->h == 0))
		{
			pdirty_rect->x	= pfarther_rect->x;
			pdirty_rect->y	= pfarther_rect->y;
			pdirty_rect->w	= pfarther_rect->w;
			pdirty_rect->h	= pfarther_rect->h;
		}
		else
		{
			/*union pfarther_rect and  pdirty_rect to pdirty_rect zone*/
			parent_right	= pfarther_rect->x + pfarther_rect->w;
			dirty_right		= pdirty_rect->x + pdirty_rect->w;
			parent_bottom	= pfarther_rect->y + pfarther_rect->h;
			dirty_bottom	= pdirty_rect->y + pdirty_rect->h;
			if(parent_right > dirty_right)
			{
				max_right	= parent_right;
			}
			else
			{
				max_right	= dirty_right;
			}
			if(parent_bottom > dirty_bottom)
			{
				max_bottom	= parent_bottom;
			}
			else
			{
				max_bottom	= dirty_bottom;
			}
			if(pfarther_rect->x < pdirty_rect->x)
			{
				pdirty_rect->x	= pfarther_rect->x;
			}
			if(pfarther_rect->y < pdirty_rect->y)
			{
				pdirty_rect->y	= pfarther_rect->y;
			}
			pdirty_rect->w	= max_right - pdirty_rect->x;
			pdirty_rect->h	= max_bottom - pdirty_rect->y;
		}
	}
			
}

void hmi_set_dirty_zone_layer(HMI_RECT_STR *pnode_rect,HMI_RECT_STR *pdirty_rect,
									U08 depth/*page node depth*/)
{	
	U08			layer	= 0;
	U08			i		= 0;
	U08		buffer_layer= FALSE;
	HMI_RECT_STR full_screen	= {0,0,HMI_MAX_WIDTH,HMI_MAX_HEIGHT};
	
	#if (defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG))
	depth	= 0U;
	#endif
	#if (defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC))
	{
		layer	= HMI_LAYER_MAX_CNT;
	}
	#else
	{
		layer	= 1;
	}
	#endif
	if(depth == HMI_PAGE_ALL_LAYER)/*all layer*/
	{
		for(i=0;i < layer;i++)
		{	
			buffer_layer	= is_buffer_layer(i);
			if(buffer_layer == TRUE)
			{
				hmi_add_dirty(pnode_rect,
					pdirty_rect,i);		
			}
			else
			{
				hmi_add_dirty(&full_screen,
					pdirty_rect,i);	
			}
		}
	}
	else	
	{			
		if(depth < layer)
		{
			#ifdef HMI_RENDER_FULL_SCREEN
			hmi_add_dirty(&full_screen,
					pdirty_rect,depth);
			#else
			buffer_layer	= is_buffer_layer(depth);
			if(buffer_layer == TRUE)
			{
				hmi_add_dirty(pnode_rect,
					pdirty_rect,depth);			
			}
			else
			{
				hmi_add_dirty(&full_screen,
					pdirty_rect,depth);	
			}
			#endif
		}
	}
			
}

#endif

#if 0
UINT8 hmi_engine_draw_page(UINT8 cur_step
							#ifndef HMI_GRAPHIC_TWLIB
							,HMI_RECT_STR *pdirty_zone
							#endif
							)
{	
	BOOLEAN						is_video_page=FALSE;
	HMI_RECT_STR CONST			*phmi_page_rect=NULL;
	#if HMI_LAYER_0_HIGHEST_PRIORITY> 1U
	UINT8                 		hmi_priority_cnt=0U;
	UINT8                 		hmi_layer_highest_active_priority=0U;
	#endif
	UINT8                 		hmi_object_index = 0U;
	HMI_LAYER_TABLE_STR CONST	*phmi_active_layer_info = &hmi_layer_table[0];
	#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_ST))
	
	HMI_RECT_STR				hmi_temp_rect={HMI_INVALID_COOR,HMI_INVALID_COOR,0,0};
	HMI_RECT_STR				hmi_union_rect ={HMI_INVALID_COOR,HMI_INVALID_COOR,0,0};
	HMI_RECT_STR CONST			*pold_page_rect=NULL;
	HMI_RECT_STR				hmi_screen_rect={0,0,HMI_MAX_WIDTH,HMI_MAX_HEIGHT};
	#else /*tw*/	
	HMI_ELEMENT_TYPE			element_type=HMI_ELEM_TYPE_SPAGE;
	BOOLEAN						bDrawBck=FALSE;
	#endif
	//HMI_RECT_STR				hmi_screen_rect={0,0,HMI_MAX_WIDTH,HMI_MAX_HEIGHT};
	hmi_driver_woking_status_flag = 0U;
	#ifdef HMI_GRAPHIC_RGL
	set_first_draw_all();
	#endif
	#if 0
	#if HMI_LAYER_0_HIGHEST_PRIORITY > 1
	hmi_layer_highest_active_priority = HMI_LAYER_0_HIGHEST_PRIORITY;
	#endif
	#endif
	#if 0
	if(hmi_driver_check_busy_status() == TRUE)
	{
		HMI_GFX_SET_STATUS(HMI_SEND_EVENT);
	}
	else
	#endif
	#if HMI_LAYER_0_HIGHEST_PRIORITY > 1
	{
		hmi_layer_highest_active_priority = hmi_highest_active_page_priority;
	}
	if(hmi_layer_highest_active_priority < HMI_LAYER_0_HIGHEST_PRIORITY)
	#endif
	{
		#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_ST))
		#if HMI_DYN_LANGUAGE_NUMBER>0
		if((cur_step==HMI_GET_DIRTY_ZONE)&&(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(HMI_LANGUAGE)!=0U))
		{
				hmi_set_dirty_zone(&hmi_screen_rect,pdirty_zone);
				HMI_DYNAMIC_OBJECT_CANCEL_CHANGED_FLAG(HMI_LANGUAGE);
		}
		else
		#endif
		
		#endif
		{			
			#if HMI_LAYER_0_HIGHEST_PRIORITY > 1  
			/*if get dirty zone ,search all priority page table.*/
			if(cur_step==HMI_GET_DIRTY_ZONE)
			{
				hmi_layer_highest_active_priority=HMI_LAYER_0_HIGHEST_PRIORITY-1;
			}
			for(hmi_priority_cnt=0U; hmi_priority_cnt <= hmi_layer_highest_active_priority; hmi_priority_cnt++)
			#endif
			{
				#if HMI_LAYER_0_HIGHEST_PRIORITY > 1
				HMI_PAGE_ID_STR hmi_page_id = hmi_layer_0_active_page_id[hmi_priority_cnt].new_page;
				HMI_PAGE_ID_STR hmi_old_page = hmi_layer_0_active_page_id[hmi_priority_cnt].old_page;
				#else
				HMI_PAGE_ID_STR hmi_page_id = hmi_layer_0_active_page_id[0].new_page;
				HMI_PAGE_ID_STR hmi_old_page =hmi_layer_0_active_page_id[0].old_page;
				#endif
				HMI_PAGE_TABLE_STR CONST * phmi_page_info = NULL;
				#if HMI_DXY_PAGES_NUMBER>0U
				if(HMI_IS_DXY_PAGE(hmi_page_id))
				{
					if(HMI_IS_VIDEO_DXY_PAGE(hmi_page_id))
					{
						is_video_page=TRUE;
						phmi_page_info=NULL;
						hmi_engine_draw_video();
					}
					else
					{
						phmi_page_rect=&hmi_dxy_page_rect[hmi_page_id];
						phmi_page_info = &hmi_dxy_page_table[hmi_page_id];
					}
					#ifdef HMI_GRAPHIC_TWLIB
					element_type=HMI_ELEM_TYPE_DPAGE;
					#endif
				}
				else
				#endif
				#if HMI_SXY_PAGES_NUMBER>0U
				if(HMI_IS_SXY_PAGE(hmi_page_id))					
				{
					if(HMI_IS_VIDEO_SXY_PAGE(hmi_page_id))
					{
						is_video_page=TRUE;
						phmi_page_info=NULL;
						hmi_engine_draw_video();
					}
					else
					{
						hmi_object_index=HMI_GET_PAGE_SXY_ID_INDEX(hmi_page_id);
						phmi_page_rect=&hmi_sxy_page_rect[hmi_object_index];
						phmi_page_info = &hmi_sxy_page_table[hmi_object_index];
					}
					#ifdef HMI_GRAPHIC_TWLIB
					element_type=HMI_ELEM_TYPE_SPAGE;
					#endif
				}
				else
				#endif
				{
				}
				#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_ST))
				if(cur_step==HMI_GET_DIRTY_ZONE)
				{
					/*old page rect is dirty zone*/
					if(hmi_old_page!=HMI_PAGES_NUMBER)
					{
						pold_page_rect=NULL;
						
						#if HMI_NDXY_PAGES_NUMBER>0U
						if(HMI_IS_NORMAL_DXY_PAGE(hmi_old_page)/*hmi_old_page<HMI_DXY_PAGES_NUMBER*/)
						{
							pold_page_rect=&hmi_dxy_page_rect[hmi_old_page];
						}
						else
						#endif
						#if HMI_NSXY_PAGES_NUMBER>0U
						if(HMI_IS_NORMAL_SXY_PAGE/*hmi_old_page<HMI_PAGE_SXY_MAX_ID*/)
						{
							hmi_object_index=HMI_GET_PAGE_SXY_ID_INDEX(hmi_old_page);
							pold_page_rect=&hmi_sxy_page_rect[hmi_object_index];
						}
						else
						#endif
						{
						}
						hmi_get_intersec_rect((HMI_RECT_STR CONST *)pold_page_rect,(HMI_RECT_STR CONST *)&hmi_screen_rect,&hmi_union_rect);
						if((hmi_union_rect.x==HMI_INVALID_COOR)&&(hmi_union_rect.y==HMI_INVALID_COOR)&&(hmi_union_rect.w==0)&&(hmi_union_rect.h==0))
						{}
						else
						{
							if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_old_page)!=0)
							{
								hmi_set_dirty_zone(&hmi_union_rect,pdirty_zone);
								HMI_DYNAMIC_OBJECT_CANCEL_CHANGED_FLAG(hmi_old_page); 
							}
						}
					}
				}
				#endif
				if(phmi_page_info!=NULL)
				{
					if(cur_step==HMI_DRAW_PAGE)
					{
						if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_page_id)!=0)
						{
							HMI_DYNAMIC_OBJECT_CANCEL_CHANGED_FLAG(hmi_page_id);
							#ifdef HMI_GRAPHIC_TWLIB
							bDrawBck=TRUE;
							#endif
						}
						#if defined(HMI_GRAPHIC_TWLIB)
						else
						{
							bDrawBck=FALSE;
						}
						#endif
						#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_ST))
						hmi_get_intersec_rect((HMI_RECT_STR CONST *)phmi_page_rect,(HMI_RECT_STR CONST *)&hmi_screen_rect,&hmi_union_rect);
						hmi_engine_draw_container(&phmi_page_info->container,(HMI_RECT_STR CONST *)phmi_page_rect,&hmi_union_rect,pdirty_zone);
						#else	/*tw*/						
						hmi_engine_draw_container(&phmi_page_info->container,
													(HMI_RECT_STR CONST *)phmi_page_rect,
													element_type,hmi_page_id,bDrawBck);
						#endif
					}
					#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_ST))
					else if(cur_step==HMI_GET_DIRTY_ZONE)
					{
						/*new page dirty zone*/
						hmi_get_intersec_rect((HMI_RECT_STR CONST *)phmi_page_rect,(HMI_RECT_STR CONST *)&hmi_screen_rect,&hmi_union_rect);
						if((hmi_union_rect.x==HMI_INVALID_COOR)&&(hmi_union_rect.y==HMI_INVALID_COOR)&&(hmi_union_rect.w==0)&&(hmi_union_rect.h==0))
						{}
						else
						{
							if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_page_id)!=0)
							{ 
								#if HMI_DXY_PAGES_NUMBER>0U
								if(hmi_page_id<HMI_DXY_PAGES_NUMBER)
								{
									/*dyn page dirty zone is buffer rect*/
									hmi_set_dirty_zone(&hmi_screen_rect,pdirty_zone);
								}
								else
								#endif
								#if HMI_SXY_PAGES_NUMBER>0U
								if(hmi_page_id<HMI_PAGE_SXY_MAX_ID)
								{
									/*static page dirty zone is page rect*/
									hmi_set_dirty_zone(&hmi_union_rect,pdirty_zone);
								}
								else
								#endif
								{
								}
							}
							#if HMI_ALL_DYN_OBJECTS_NUMBER > HMI_PAGES_NUMBER
							#if (HMI_ENGINE_LOW_PRIOR_PAGE_UPDATA == HMI_UPDATE_BUSY) && (HMI_LAYER_0_HIGHEST_PRIORITY > 1)
							else if(hmi_priority_cnt >= hmi_highest_active_page_priority)
							#else
							else
							#endif
							{
								hmi_engine_check_dynamic_object_changed(&phmi_page_info->container,(HMI_RECT_STR CONST *)phmi_page_rect,&hmi_union_rect,pdirty_zone);
							}
							#if (HMI_ENGINE_LOW_PRIOR_PAGE_UPDATA == HMI_UPDATE_BUSY) && (HMI_LAYER_0_HIGHEST_PRIORITY > 1)
							else
							{
							}
							#endif
							#endif
						}
					}
					else
					{}
					#endif
				}
			}
		}
	}
	if((cur_step==HMI_DRAW_PAGE)&&(!is_video_page))
	{
		#if defined(HMI_GRAPHIC_TWLIB)	
		call_C_hmi_driver_refresh_LCD(bDrawBck);
		#else
		call_C_hmi_driver_refresh_LCD();
		#endif
	}
	return(hmi_driver_woking_status_flag);
}

#endif

#if HMI_PAGES_NUMBER>0
#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||\
	defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||\
	defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||\
	defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))
UINT8 hmi_engine_get_dirty_zone_page(HMI_RECT_STR *pdirty_zone)
{	
	BOOLEAN						is_video_page	= FALSE;
	#if (defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_RGL))
	BOOLEAN						exist_video_page= FALSE;
	#endif
	BOOLEAN						dxy_page		= FALSE;
	HMI_RECT_STR 				hmi_page_rect 	= {0};
	#if (HMI_LAYER_0_HIGHEST_PRIORITY > 1)||(HMI_LAYER_1_HIGHEST_PRIORITY > 1)
	UINT8                 		hmi_priority_cnt= 0U;
	UINT8                 		hmi_layer_highest_active_priority = 0U;
	#endif
	UINT8                 		hmi_object_index		= 0U;
	#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||\
		defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||\
		defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||\
		defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))	
	HMI_RECT_STR				hmi_insec_rect		= {HMI_INVALID_COOR,HMI_INVALID_COOR,0,0};
	HMI_RECT_STR 				old_page_rect		= {0};
	HMI_RECT_STR				hmi_screen_rect		= {0,0,HMI_MAX_WIDTH,HMI_MAX_HEIGHT};	
	HMI_RECT_STR				cliped_farther_rect	= {HMI_INVALID_COOR,HMI_INVALID_COOR,0,0};
	#endif
	HMI_PAGE_TABLE_STR CONST * phmi_page_info	= NULL;	
	HMI_PAGE_ID_STR				hmi_page_id		= 0;
	HMI_PAGE_ID_STR				hmi_old_page	= 0;
	UINT8						hmi_screen_id	= 0U;
	HMI_PRIOR_PAGE_STR			*phmi_active_page_id = NULL;
	
	#if (HMI_LAYER_0_HIGHEST_PRIORITY > 1)||(HMI_LAYER_1_HIGHEST_PRIORITY > 1)
	UINT8						pop_up_window_layer	= 0;
	UINT8						hmi_layer_highest_prior	=0U;
	#endif
	
	hmi_screen_id =hmi_driver_get_render_screen();
	
	#if (HMI_LAYER_0_HIGHEST_PRIORITY > 1)||(HMI_LAYER_1_HIGHEST_PRIORITY > 1)
	if(hmi_screen_id ==  HMI_LAYER_SCREEN1)
	{
		hmi_layer_highest_prior = HMI_LAYER_1_HIGHEST_PRIORITY;
	}
	else
	{
		hmi_layer_highest_prior = HMI_LAYER_0_HIGHEST_PRIORITY;
	}
	{
		hmi_layer_highest_active_priority = hmi_highest_active_page_priority[hmi_screen_id];
	}
	if(hmi_layer_highest_active_priority < hmi_layer_highest_prior)
	#endif
	{		
		#if HMI_DYN_LANGUAGE_NUMBER > 0	/*language number >1*/
		if((HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(HMI_LANGUAGE) != 0U))/*if change language setting,all active page need refresh*/
		{				
			hmi_set_dirty_zone_layer(&hmi_screen_rect,pdirty_zone,HMI_PAGE_ALL_LAYER);
			HMI_DYNAMIC_OBJECT_CANCEL_CHANGED_FLAG(HMI_LANGUAGE);
		}
		else		
		#endif
		{	
			if(hmi_screen_id == HMI_LAYER_SCREEN0)
			{
				phmi_active_page_id = hmi_layer_0_active_page_id;
			}
			
#if HMI_ALL_LAYERS_NUMBER > 1
			else if(hmi_screen_id == HMI_LAYER_SCREEN1)
			{

				phmi_active_page_id = hmi_layer_1_active_page_id;
			}
			else
			{
				phmi_active_page_id = hmi_layer_0_active_page_id;
			}
#endif
			
			if(phmi_active_page_id != NULL)
			{
				#if (HMI_LAYER_0_HIGHEST_PRIORITY > 1) ||(HMI_LAYER_1_HIGHEST_PRIORITY > 1)
				if (hmi_layer_highest_prior > 0)
				{
				/*Search all priority page table.*/			
					hmi_layer_highest_active_priority	= hmi_layer_highest_prior-1;
				}
				for(hmi_priority_cnt=0U; hmi_priority_cnt <= hmi_layer_highest_active_priority; hmi_priority_cnt++)
				#endif
				{
					#if (HMI_LAYER_0_HIGHEST_PRIORITY > 1) ||(HMI_LAYER_1_HIGHEST_PRIORITY > 1)
					hmi_page_id		= phmi_active_page_id[hmi_priority_cnt].new_page;
					hmi_old_page	= phmi_active_page_id[hmi_priority_cnt].old_page;
					phmi_active_page_id[hmi_priority_cnt].old_page	= HMI_PAGES_NUMBER;
					#else
					hmi_page_id		= phmi_active_page_id[0].new_page;
					hmi_old_page	= phmi_active_page_id[0].old_page;
					phmi_active_page_id[0].old_page	= HMI_PAGES_NUMBER;
				#endif				
				#if HMI_DXY_PAGES_NUMBER > 0U	/*Get current dxy page info*/
				if(HMI_IS_DXY_PAGE(hmi_page_id))
				{
					dxy_page	= TRUE;
					if(HMI_IS_VIDEO_DXY_PAGE(hmi_page_id))
					{
						is_video_page	= TRUE;
						phmi_page_info	= NULL;						
						#if (defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_RGL))
						exist_video_page= TRUE;
						#endif
					}
					else /*dxy page,not video page*/
					{
						hmi_object_index	= HMI_GET_PAGE_DXY_ID_INDEX(hmi_page_id);
						if(hmi_object_index < HMI_DXY_PAGES_NUMBER)
						{							
							hmi_page_rect.x = hmi_dxy_page_rect[hmi_object_index].x;
							hmi_page_rect.y = hmi_dxy_page_rect[hmi_object_index].y;
							hmi_page_rect.w = hmi_dxy_page_rect[hmi_object_index].w;
							hmi_page_rect.h = hmi_dxy_page_rect[hmi_object_index].h;
							phmi_page_info 	= &hmi_dxy_page_table[hmi_object_index];
						}
					}					
				}
				else
				#endif
				#if HMI_SXY_PAGES_NUMBER > 0U	/*Get current static xy page info*/
				if(HMI_IS_SXY_PAGE(hmi_page_id))					
				{
					dxy_page	= FALSE;
					if(HMI_IS_VIDEO_SXY_PAGE(hmi_page_id))
					{
						is_video_page	= TRUE;
						phmi_page_info	= NULL;
						#if (defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_RGL))
						exist_video_page= TRUE;
						#endif
					}
					else	/*sxy page,not video page*/
					{
						hmi_object_index	= HMI_GET_PAGE_SXY_ID_INDEX(hmi_page_id);
						if(hmi_object_index < HMI_SXY_PAGES_NUMBER)
						{
							hmi_page_rect.x = hmi_sxy_page_rect[hmi_object_index].x;
							hmi_page_rect.y = hmi_sxy_page_rect[hmi_object_index].y;
							hmi_page_rect.w = hmi_sxy_page_rect[hmi_object_index].w;
							hmi_page_rect.h = hmi_sxy_page_rect[hmi_object_index].h;
							phmi_page_info 	= &hmi_sxy_page_table[hmi_object_index];
						}
					}					
				}
				else
				#endif
				{
				}
				if(hmi_page_id != HMI_PAGES_NUMBER)
				{
					if((HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_page_id) != 0))					
					{
						if((!is_video_page))
						{							
							if(dxy_page)
							{
								hmi_page_rect.x = hmi_screen_rect.x;/*dxy page dirty zone is screen zone*/
								hmi_page_rect.y = hmi_screen_rect.y;
								hmi_page_rect.w = hmi_screen_rect.w;
								hmi_page_rect.h = hmi_screen_rect.h;
							}
							
							hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_page_rect),
												(HMI_RECT_STR CONST *)(&hmi_screen_rect),
												&hmi_insec_rect);/*clip phmi_page_rect width screen zone*/
								#if (HMI_LAYER_0_HIGHEST_PRIORITY > 1) ||(HMI_LAYER_1_HIGHEST_PRIORITY > 1)
							if(hmi_priority_cnt != 0)/*only pop up window layer need refresh*/
							{
								#if defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(S6J3200_GRAPHIC)
								pop_up_window_layer = get_popup_window_layer();/*RGL not support pop up page*/
								#else
								pop_up_window_layer = 0;/*one layer of GDI*/
								#endif
								hmi_set_dirty_zone_layer((HMI_RECT_STR *)(&hmi_page_rect),pdirty_zone,
														pop_up_window_layer);
							}
							else
							{
								hmi_set_dirty_zone_layer((HMI_RECT_STR *)(&hmi_page_rect),pdirty_zone,
														0);
							}
							#else
							{						
								hmi_set_dirty_zone_layer((HMI_RECT_STR *)(&hmi_page_rect),pdirty_zone,
														HMI_PAGE_ALL_LAYER);/*all layer is dirty*/
							}												
							#endif
						}
					}
					else	/*search every element refresh flag and get union element dirty zone*/
					{
						if(!is_video_page)
						{						
							hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_screen_rect),
									(HMI_RECT_STR CONST *)(&hmi_page_rect),
									&cliped_farther_rect);
							hmi_engine_check_dynamic_object_changed((HMI_CONTAINER_STR CONST *)(&(phmi_page_info->container)),
														(HMI_RECT_STR CONST *)(&hmi_page_rect),
														pdirty_zone,
														HMI_PAGE_BEGIN_DEPTH/*page node tree depth*/,
														&cliped_farther_rect);
						}
					}
				}
								
				if((hmi_old_page!=HMI_PAGES_NUMBER)	)/*Get old page info*/
				{	
										
					{
						#if HMI_DXY_PAGES_NUMBER>0U					
						if(HMI_IS_DXY_PAGE(hmi_old_page))					
						{
							dxy_page = TRUE;
							if(HMI_IS_VIDEO_DXY_PAGE(hmi_old_page))
							{
								is_video_page	= TRUE;
								old_page_rect.x	= 0;
								old_page_rect.y	= 0;
								old_page_rect.w	= 0;
								old_page_rect.h	= 0;
							}
							else
							{
								is_video_page=FALSE;							
								hmi_object_index=HMI_GET_PAGE_DXY_ID_INDEX(hmi_old_page);
								if(hmi_object_index<HMI_DXY_PAGES_NUMBER)
								{
									if((HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_old_page)!=0U))/*set ,refresh flag not render*/
									{
										HMI_DYNAMIC_OBJECT_CANCEL_CHANGED_FLAG(hmi_old_page);
										old_page_rect.x=hmi_screen_rect.x;
										old_page_rect.y=hmi_screen_rect.y;
										old_page_rect.w=hmi_screen_rect.w;
										old_page_rect.h=hmi_screen_rect.h;
									}
									else
									{
										old_page_rect.x=hmi_dxy_page_rect[hmi_object_index].x;
										old_page_rect.y=hmi_dxy_page_rect[hmi_object_index].y;
										old_page_rect.w=hmi_dxy_page_rect[hmi_object_index].w;
										old_page_rect.h=hmi_dxy_page_rect[hmi_object_index].h;
									}
								}
							}
						}
						#endif

						#if HMI_SXY_PAGES_NUMBER>0U
						if(HMI_IS_SXY_PAGE(hmi_old_page))					
						{
							dxy_page = FALSE;
							if(HMI_IS_VIDEO_SXY_PAGE(hmi_old_page))
							{
								is_video_page	= TRUE;
								old_page_rect.x	= 0;
								old_page_rect.y	= 0;
								old_page_rect.w	= 0;
								old_page_rect.h	= 0;
							}
							else
							{
								is_video_page=FALSE;
								hmi_object_index=HMI_GET_PAGE_SXY_ID_INDEX(hmi_old_page);
								if(hmi_object_index<HMI_SXY_PAGES_NUMBER)
								{
									old_page_rect.x=hmi_sxy_page_rect[hmi_object_index].x;
									old_page_rect.y=hmi_sxy_page_rect[hmi_object_index].y;
									old_page_rect.w=hmi_sxy_page_rect[hmi_object_index].w;
									old_page_rect.h=hmi_sxy_page_rect[hmi_object_index].h;
								}
							}
						}
						#endif
						if(old_page_rect.w	!= 0)
						{
								#if (HMI_LAYER_0_HIGHEST_PRIORITY > 1) ||(HMI_LAYER_1_HIGHEST_PRIORITY > 1)
							if(hmi_priority_cnt!=0)/*only pop up window layer need refresh*/
							{
								#if (defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
								pop_up_window_layer = get_popup_window_layer();
								#else
								pop_up_window_layer = 0;/*0 layer of GDI*/
								#endif
								hmi_set_dirty_zone_layer((HMI_RECT_STR *)(&old_page_rect),pdirty_zone,
														pop_up_window_layer);
							}
							else
							{						
								hmi_set_dirty_zone_layer((HMI_RECT_STR *)(&old_page_rect),pdirty_zone,
														HMI_PAGE_ALL_LAYER);/*all layer is dirty*/
							}
							#else
							hmi_set_dirty_zone_layer((HMI_RECT_STR *)(&old_page_rect),pdirty_zone,
														HMI_PAGE_ALL_LAYER);/*all layer is dirty*/
							#endif
						}
					}
				}
				}
			}
		}
	}
	#if (defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_RGL))
	if(exist_video_page == TRUE)
	{
		#if 0
		hmi_enable_video_layer(TRUE);
		#endif
	}
	else
	{
		
	}
	#endif
	return(hmi_driver_woking_status_flag);
}
#endif


#if 0
UINT8 hmi_engine_draw_page(UINT8 cur_step
							#ifndef HMI_GRAPHIC_TWLIB
							,HMI_RECT_STR *pdirty_zone
							#endif
							)
{	
	BOOLEAN						is_video_page=FALSE;
	HMI_RECT_STR CONST			*phmi_page_rect=NULL;
	#if HMI_LAYER_0_HIGHEST_PRIORITY> 1U
	UINT8                 		hmi_priority_cnt=0U;
	UINT8                 		hmi_layer_highest_active_priority=0U;
	#endif
	UINT8                 		hmi_object_index = 0U;
	HMI_LAYER_TABLE_STR CONST	*phmi_active_layer_info = &hmi_layer_table[0];
	#if (defined(HMI_GRAPHIC_AGG)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_ST))
	
	HMI_RECT_STR				hmi_temp_rect={HMI_INVALID_COOR,HMI_INVALID_COOR,0,0};
	HMI_RECT_STR				hmi_insec_rect ={HMI_INVALID_COOR,HMI_INVALID_COOR,0,0};
	HMI_RECT_STR CONST			*pold_page_rect=NULL;
	HMI_RECT_STR				hmi_screen_rect={0,0,HMI_MAX_WIDTH,HMI_MAX_HEIGHT};
	#else /*tw*/	
	HMI_ELEMENT_TYPE			element_type=HMI_ELEM_TYPE_SPAGE;
	BOOLEAN						bDrawBck=FALSE;
	#endif
	//HMI_RECT_STR				hmi_screen_rect={0,0,HMI_MAX_WIDTH,HMI_MAX_HEIGHT};
	hmi_driver_woking_status_flag = 0U;
	#ifdef HMI_GRAPHIC_RGL
	set_first_draw_all();
	#endif
	
	#if HMI_LAYER_0_HIGHEST_PRIORITY > 1
	{
		hmi_layer_highest_active_priority = hmi_highest_active_page_priority;
	}
	if(hmi_layer_highest_active_priority < HMI_LAYER_0_HIGHEST_PRIORITY)
	#endif
	{
		#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)
			||defined(HMI_GRAPHIC_ST)) /*support multilanguage*/
		#if HMI_DYN_LANGUAGE_NUMBER>0	/*language number >1*/
		if((cur_step==HMI_GET_DIRTY_ZONE)&&
			(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(HMI_LANGUAGE)!=0U))
		{
				//hmi_set_dirty_zone(&hmi_screen_rect,pdirty_zone);
				hmi_set_dirty_zone_layer(&hmi_screen_rect,pdirty_zone,HMI_PAGE_ALL_LAYER);
				HMI_DYNAMIC_OBJECT_CANCEL_CHANGED_FLAG(HMI_LANGUAGE);
		}
		else
		#endif		
		#endif
		{			
			#if HMI_LAYER_0_HIGHEST_PRIORITY > 1  
			/*if get dirty zone ,search all priority page table.*/
			if(cur_step==HMI_GET_DIRTY_ZONE)
			{
				hmi_layer_highest_active_priority=HMI_LAYER_0_HIGHEST_PRIORITY-1;
			}
			for(hmi_priority_cnt=0U; hmi_priority_cnt <= hmi_layer_highest_active_priority; hmi_priority_cnt++)
			#endif
			{
				#if HMI_LAYER_0_HIGHEST_PRIORITY > 1
				HMI_PAGE_ID_STR hmi_page_id = hmi_layer_0_active_page_id[hmi_priority_cnt].new_page;
				HMI_PAGE_ID_STR hmi_old_page = hmi_layer_0_active_page_id[hmi_priority_cnt].old_page;
				#else
				HMI_PAGE_ID_STR hmi_page_id = hmi_layer_0_active_page_id[0].new_page;
				HMI_PAGE_ID_STR hmi_old_page =hmi_layer_0_active_page_id[0].old_page;
				#endif
				HMI_PAGE_TABLE_STR CONST * phmi_page_info = NULL;
				#if HMI_DXY_PAGES_NUMBER>0U
				if(HMI_IS_DXY_PAGE(hmi_page_id))
				{
					if(HMI_IS_VIDEO_DXY_PAGE(hmi_page_id))
					{
						is_video_page=TRUE;
						phmi_page_info=NULL;
						if(cur_step==HMI_DRAW_PAGE)
						{
							hmi_engine_draw_video();
						}
					}
					else
					{
						phmi_page_rect=&hmi_dxy_page_rect[hmi_page_id];
						phmi_page_info = &hmi_dxy_page_table[hmi_page_id];
					}
					#ifdef HMI_GRAPHIC_TWLIB
					element_type=HMI_ELEM_TYPE_DPAGE;
					#endif
				}
				else
				#endif
				#if HMI_SXY_PAGES_NUMBER>0U
				if(HMI_IS_SXY_PAGE(hmi_page_id))					
				{
					if(HMI_IS_VIDEO_SXY_PAGE(hmi_page_id))
					{
						is_video_page=TRUE;
						phmi_page_info=NULL;
						if(cur_step==HMI_DRAW_PAGE)
						{
							hmi_engine_draw_video();
						}
					}
					else
					{
						hmi_object_index=HMI_GET_PAGE_SXY_ID_INDEX(hmi_page_id);
						phmi_page_rect=&hmi_sxy_page_rect[hmi_object_index];
						phmi_page_info = &hmi_sxy_page_table[hmi_object_index];
					}
					#ifdef HMI_GRAPHIC_TWLIB
					element_type=HMI_ELEM_TYPE_SPAGE;
					#endif
				}
				else
				#endif
				{
				}
				#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||
						defined(HMI_GRAPHIC_ST))
				if(cur_step==HMI_GET_DIRTY_ZONE)
				{
					/*old page rect is dirty zone*/
					if(hmi_old_page!=HMI_PAGES_NUMBER)
					{
						pold_page_rect=NULL;
						
						#if HMI_DXY_PAGES_NUMBER>0U
						if(hmi_old_page<HMI_DXY_PAGES_NUMBER)
						{
							hmi_old_page=HMI_GET_PAGE_DXY_ID_INDEX(hmi_old_page);
							pold_page_rect=&hmi_dxy_page_rect[hmi_old_page];
						}
						else
						#endif
						#if HMI_SXY_PAGES_NUMBER>0U
						if(hmi_old_page<HMI_PAGE_SXY_MAX_ID)
						{
							hmi_old_page=HMI_GET_PAGE_SXY_ID_INDEX(hmi_old_page);
							pold_page_rect=&hmi_sxy_page_rect[hmi_old_page];
						}
						else
						#endif
						{
						}
						hmi_get_intersec_rect((HMI_RECT_STR CONST *)pold_page_rect,
											(HMI_RECT_STR CONST *)&hmi_screen_rect,
											&hmi_insec_rect);
						if((hmi_insec_rect.x!=HMI_INVALID_COOR)&&(hmi_insec_rect.y!=HMI_INVALID_COOR)
							&&(hmi_insec_rect.w!=0)&&(hmi_insec_rect.h!=0))						
						{
							if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_old_page)!=0)
							{
								//hmi_set_dirty_zone(&hmi_insec_rect,pdirty_zone); 
								hmi_set_dirty_zone_layer(&hmi_insec_rect,pdirty_zone,
														HMI_PAGE_ALL_LAYER);/*all layer is dirty*/
								HMI_DYNAMIC_OBJECT_CANCEL_CHANGED_FLAG(hmi_old_page); 
							}
						}
					}
				}
				#endif
				if(phmi_page_info!=NULL)/*current page to be display*/
				{
					if(cur_step==HMI_DRAW_PAGE)
					{
						if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_page_id)!=0)
						{
							HMI_DYNAMIC_OBJECT_CANCEL_CHANGED_FLAG(hmi_page_id);
							#ifdef HMI_GRAPHIC_TWLIB
							bDrawBck=TRUE;
							#endif
						}
						#if defined(HMI_GRAPHIC_TWLIB)
						else
						{
							bDrawBck=FALSE;
						}
						#endif
						#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||
								defined(HMI_GRAPHIC_ST))
						hmi_get_intersec_rect((HMI_RECT_STR CONST *)phmi_page_rect,(HMI_RECT_STR CONST *)&hmi_screen_rect,&hmi_union_rect);
						hmi_engine_draw_container(&phmi_page_info->container,(HMI_RECT_STR CONST *)phmi_page_rect,&hmi_union_rect,pdirty_zone);
						#else	/*tw*/						
						hmi_engine_draw_container(&phmi_page_info->container,
													(HMI_RECT_STR CONST *)phmi_page_rect,
													element_type,hmi_page_id,bDrawBck);
						#endif
					}
					#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||
							defined(HMI_GRAPHIC_ST))
					else if(cur_step==HMI_GET_DIRTY_ZONE)
					{
						/*new page dirty zone*/ 
						hmi_get_intersec_rect((HMI_RECT_STR CONST *)phmi_page_rect,
												(HMI_RECT_STR CONST *)&hmi_screen_rect,
												&hmi_insec_rect);
						if((hmi_insec_rect.x!=HMI_INVALID_COOR)&&(hmi_insec_rect.y!=HMI_INVALID_COOR)&&
									(hmi_insec_rect.w!=0)&&(hmi_insec_rect.h!=0))						
						{
							if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_page_id)!=0)
							{ 
								#if HMI_DXY_PAGES_NUMBER>0U
								if(hmi_page_id<HMI_DXY_PAGES_NUMBER)
								{
									/*dyn page dirty zone is buffer rect*/
									hmi_set_dirty_zone(&hmi_screen_rect,pdirty_zone);
								}
								else
								#endif
								#if HMI_SXY_PAGES_NUMBER>0U
								if(hmi_page_id<HMI_PAGE_SXY_MAX_ID)
								{
									/*static page dirty zone is page rect*/
									hmi_set_dirty_zone(&hmi_insec_rect,pdirty_zone);
								}
								else
								#endif
								{
								}
							}
							#if HMI_ALL_DYN_OBJECTS_NUMBER > HMI_PAGES_NUMBER
							#if (HMI_ENGINE_LOW_PRIOR_PAGE_UPDATA == HMI_UPDATE_BUSY) && (HMI_LAYER_0_HIGHEST_PRIORITY > 1)
							else if(hmi_priority_cnt >= hmi_highest_active_page_priority)
							#else
							else
							#endif
							{
								hmi_engine_check_dynamic_object_changed(&phmi_page_info->container,(HMI_RECT_STR CONST *)phmi_page_rect,&hmi_insec_rect,pdirty_zone);
							}
							#if (HMI_ENGINE_LOW_PRIOR_PAGE_UPDATA == HMI_UPDATE_BUSY) && (HMI_LAYER_0_HIGHEST_PRIORITY > 1)
							else
							{
							}
							#endif
							#endif
						}
					}
					else
					{}
					#endif
				}
			}
		}
	}
	if((cur_step==HMI_DRAW_PAGE)&&(!is_video_page))
	{
		#if defined(HMI_GRAPHIC_TWLIB)	
		call_C_hmi_driver_refresh_LCD(bDrawBck);
		#else
		call_C_hmi_driver_refresh_LCD();
		#endif
	}
	return(hmi_driver_woking_status_flag);
}
#endif

#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)
void hmi_engine_draw_page_prop(void)
{
	HMI_PAGE_ID_STR				hmi_page_id		= 0;
	UINT8						hmi_page_alpha	= 0;
	#if (HMI_LAYER_0_HIGHEST_PRIORITY > 1) ||(HMI_LAYER_1_HIGHEST_PRIORITY > 1)
	UINT8                 		hmi_priority_cnt = 0U;
	UINT8                 		hmi_layer_highest_active_priority = 0U;
	UINT8						hmi_layer_highest_prior	= 0U;
	#endif
	UINT8						hmi_screen_id	= 0U;
	HMI_PRIOR_PAGE_STR			*phmi_active_page_id = NULL;
		
	hmi_screen_id =hmi_driver_get_render_screen();
	if(hmi_screen_id == HMI_LAYER_SCREEN0)
	{
		phmi_active_page_id = hmi_layer_0_active_page_id;
	}
	
#if HMI_ALL_LAYERS_NUMBER > 1
	else if(hmi_screen_id == HMI_LAYER_SCREEN1)
	{
		
		phmi_active_page_id = hmi_layer_1_active_page_id;
	}
	else
	{
		phmi_active_page_id = hmi_layer_0_active_page_id;
	}
#endif
	if(phmi_active_page_id != NULL)
	{
		
		#if (HMI_LAYER_0_HIGHEST_PRIORITY > 1) ||(HMI_LAYER_1_HIGHEST_PRIORITY > 1)
		if(hmi_screen_id == HMI_LAYER_SCREEN1)
		{
			hmi_layer_highest_prior  =HMI_LAYER_1_HIGHEST_PRIORITY;
		}
		else
		{
			hmi_layer_highest_prior  =HMI_LAYER_0_HIGHEST_PRIORITY;
		}
		{
			hmi_layer_highest_active_priority = hmi_highest_active_page_priority[hmi_screen_id];
		}
		if(hmi_layer_highest_active_priority < hmi_layer_highest_prior)
		#endif
		{				
			#if (HMI_LAYER_0_HIGHEST_PRIORITY > 1) ||(HMI_LAYER_1_HIGHEST_PRIORITY > 1)
			for(hmi_priority_cnt=0U; hmi_priority_cnt <= hmi_layer_highest_active_priority; hmi_priority_cnt++)
			#endif
			{	
				if(phmi_active_page_id != NULL)
				{
				#if (HMI_LAYER_0_HIGHEST_PRIORITY > 1) ||(HMI_LAYER_1_HIGHEST_PRIORITY > 1)
				hmi_page_id = phmi_active_page_id[hmi_priority_cnt].new_page;						
				#else
				hmi_page_id = phmi_active_page_id[0].new_page;
				#endif
				#if HMI_DXY_PAGES_NUMBER > 0U
				if(HMI_IS_DXY_PAGE(hmi_page_id))
				{
					hmi_page_alpha	= hmi_dxy_page_rect[hmi_page_id].alpha;
				}
				else
				#endif
				#if HMI_SXY_PAGES_NUMBER > 0U
				if(HMI_IS_SXY_PAGE(hmi_page_id))					
				{
					hmi_page_alpha	= hmi_sxy_page_rect[hmi_page_id].alpha;
				}
				else
				#endif
				{
				}
								
				if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_page_id) != 0)
				{							
					HMI_DYNAMIC_OBJECT_CANCEL_CHANGED_FLAG(hmi_page_id);
					hmi_driver_set_page_alpha(hmi_page_alpha,TRUE);
					#ifdef HMI_R_ASYNCHRONOUS_HW_UPDATE
					#if HMI_RENDER_ENABLE_FIFO
					//hmi_set_sync_status(HwUpdate_Popup_FIFO);
					#else
					#ifdef HMI_GRAPHIC_RGL
					hmi_set_sync_status(HwUpdate_WaitScanline);
					#endif
					#endif
					#else
					hmi_driver_send_cmdlist(
											#ifdef HMI_GRAPHIC_OPENGLES
											0
											#endif	
											);
					#endif
				}
				}

			}			
		}	
	}
}
#endif



/*
if layer is void,then disable
else enable
*/
#if 0
BOOLEAN hmi_is_empty_layer(HMI_CONTAINER_STR CONST * phmi_container_info)
{
	BOOLEAN	empty		= FALSE;
	UINT	layer_cnt	= 0;
	UINT	i			= 0;
	HMI_OBJECT_ID_STR hmi_object_id	= 0;	
	HMI_OBJECT_PROP_STR CONST * phmi_container_object_table	= NULL;
	
	if(phmi_container_info != NULL)
	{
		layer_cnt	= phmi_container_info->container_object_table.object_number;
		phmi_container_object_table= phmi_container_info->container_object_table.p_object_table;
		for(i = 0; i < layer_cnt;i++)/*search every layer--container */
		{
			hmi_object_id	= (*phmi_container_object_table++);
			#if HMI_DXY_CONTAINERS_NUMBER > 0
			if(HMI_IS_DYN_XY_CONTAINER(hmi_object_id))
			{				
				hmi_object_id = HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_object_id );
				if(hmi_object_id< HMI_DXY_CONTAINERS_NUMBER) 
				{
					if(!empty)
					{
						empty	= hmi_is_empty_layer(&hmi_dyn_xy_container_table[hmi_object_id]);					
					}
				}				
			}
			else
			#endif

			#if HMI_SXY_CONTAINERS_NUMBER>0
			if(HMI_IS_CONTAINERS_SXY(hmi_object_id))
			{
				hmi_object_id	= HMI_GET_CONTAINERS_SXY_ID_INDEX(hmi_object_id);
				if(hmi_object_id < HMI_SXY_CONTAINERS_NUMBER)
				{
					if(!empty)
					{
						empty	= hmi_is_empty_layer(&hmi_sxy_container_table[hmi_object_id]);					
					}
				}
			}
			else
			#endif
			#if HMI_DYN_CONTAINERS_NUMBER>0
			if(HMI_IS_CONTAINERS_SXY(hmi_object_id))//??
			{
				hmi_object_id	= HMI_GET_CONTAINERS_SXY_ID_INDEX(hmi_object_id);
				if(hmi_object_id < HMI_SXY_CONTAINERS_NUMBER)
				{
					if(!empty)
					{
						empty	= hmi_is_empty_layer(&hmi_sxy_container_table[hmi_object_id]);
					}
				}
			}
			else
			#endif			
			{
				empty	= TRUE;
			}
		}
		}
	}
}
}
#endif
#if (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
void hmi_clear_page_layer()
{
	HMI_PAGE_ID_STR  hmi_new_object_id =0;
	HMI_PAGE_ID_STR  hmi_old_object_id =0;
			
	hmi_new_object_id = hmi_layer_0_active_page_id[0].new_page;
	hmi_old_object_id = hmi_layer_0_active_page_id[0].old_page;
	
	if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_new_object_id) != 0)
	{
		if(hmi_new_object_id !=hmi_old_object_id)
		{
			hmi_driver_free_all_layer(); 
		}
	}		
}
#endif

#if HMI_MAX_WIDTH <=127	
#define	HMI_CLIP_MIN_X		(-0x7F)	
#else		
#define	HMI_CLIP_MIN_X		(-0x7FFF)	
#endif

#if HMI_MAX_HEIGHT <= 127
#define	HMI_CLIP_MIN_Y		(-0x7F)	
#else
#define	HMI_CLIP_MIN_Y		(-0x7FFF)	
#endif
#define HMI_CLIP_MAX_W		0xFFFF
#define HMI_CLIP_MAX_H		0xFFFF


UINT8 hmi_engine_draw_page(
									#if	(defined(HMI_GRAPHIC_STGLIB)||defined(HMI_GRAPHIC_AGG)||	\
										defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513)||		\
										defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||		\
										defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG))	
									
										HMI_RECT_STR *pdirty_zone
									#endif
									)
{	
	BOOLEAN						is_video_page	= FALSE;
	HMI_RECT_STR 				hmi_page_rect	= {0};
	#if (defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_RGL))
	UINT8						hmi_page_alpha	= 0;	
	BOOLEAN						hmi_page_flag	= FALSE;
	HMI_ALPHA_SCALE_PT_STR		hmi_page_scale_alpha ={255U,1.0f,{0,0}};
	#endif
	HMI_PAGE_ID_STR				hmi_page_id		= 0;
	HMI_PAGE_TABLE_STR CONST * phmi_page_info	= NULL;
	#if HMI_LAYER_0_HIGHEST_PRIORITY> 1U
	UINT8                 		hmi_priority_cnt = 0U;
	UINT8                 		hmi_layer_highest_active_priority = 0U;
	#endif
	UINT8                 		hmi_object_index = 0U;
	#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||\
		defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)|| \
		defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)|| \
		defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))	
	HMI_RECT_STR				hmi_screen_rect	= {HMI_CLIP_MIN_X,HMI_CLIP_MIN_Y,HMI_CLIP_MAX_W,HMI_CLIP_MAX_H};
	#elif (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
	HMI_RECT_STR				hmi_screen_rect	= {0,0,HMI_MAX_WIDTH,HMI_MAX_HEIGHT};
	BOOLEAN						hmi_draw_all_element	= FALSE;						
	#else /*tw*/	
	HMI_ELEMENT_TYPE			element_type	= HMI_ELEM_TYPE_SPAGE;
	BOOLEAN						bDrawBck		= FALSE;
	BOOLEAN						exist_group_a	= FALSE;
	#endif	
	UINT8						hmi_screen_id	= 0U;
	HMI_PRIOR_PAGE_STR			*phmi_active_page_id = NULL;
	
	hmi_screen_id =hmi_driver_get_render_screen();
	if(hmi_screen_id == HMI_LAYER_SCREEN0)
	{
		phmi_active_page_id = hmi_layer_0_active_page_id;
	}
	#if HMI_ALL_LAYERS_NUMBER > 1
	else if(hmi_screen_id == HMI_LAYER_SCREEN1)
	{
		
		phmi_active_page_id = hmi_layer_1_active_page_id;
	}
	else
	{
		phmi_active_page_id = hmi_layer_0_active_page_id;
	}
	#endif
	if(phmi_active_page_id != NULL)
	{
		
		#if (HMI_LAYER_0_HIGHEST_PRIORITY > 1) ||(HMI_LAYER_1_HIGHEST_PRIORITY > 1)
		if(hmi_screen_id == HMI_LAYER_SCREEN1)
		{
			hmi_layer_highest_prior = HMI_LAYER_1_HIGHEST_PRIORITY;
		}
		else
		{
			hmi_layer_highest_prior = HMI_LAYER_0_HIGHEST_PRIORITY;
		}
		{
			hmi_layer_highest_active_priority = hmi_highest_active_page_priority[hmi_screen_id];
		}
		if(hmi_layer_highest_active_priority < hmi_layer_highest_prior)
		#endif
		{		
						
			#if (HMI_LAYER_0_HIGHEST_PRIORITY > 1) ||(HMI_LAYER_1_HIGHEST_PRIORITY > 1)
			for(hmi_priority_cnt=0U; hmi_priority_cnt <= hmi_layer_highest_active_priority; hmi_priority_cnt++)
			#endif
			{	
				#if (HMI_LAYER_0_HIGHEST_PRIORITY > 1) ||(HMI_LAYER_1_HIGHEST_PRIORITY > 1)
				hmi_page_id		= phmi_active_page_id[hmi_priority_cnt].new_page;			
				#else
				hmi_page_id		= phmi_active_page_id[0].new_page;			
				#endif
			#if HMI_DXY_PAGES_NUMBER>0U
			if(HMI_IS_DXY_PAGE(hmi_page_id))
			{
				if(HMI_IS_VIDEO_DXY_PAGE(hmi_page_id))
				{
					is_video_page=TRUE;
					phmi_page_info=NULL;	
					#ifndef HMI_GRAPHIC_RGL
					#if HMI_VDXY_PAGES_NUMBER+HMI_VSXY_PAGES_NUMBER > 0
					//hmi_engine_draw_video(hmi_page_id,TRUE);
					#endif
					#endif	
				}
				else
				{
					phmi_page_info = &hmi_dxy_page_table[hmi_page_id];
					hmi_page_rect.x	= hmi_dxy_page_rect[hmi_page_id].x;
					hmi_page_rect.y	= hmi_dxy_page_rect[hmi_page_id].y;
					hmi_page_rect.w	= hmi_dxy_page_rect[hmi_page_id].w;
					hmi_page_rect.h	= hmi_dxy_page_rect[hmi_page_id].h;
					#if defined(HMI_GRAPHIC_RGL	)|| defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)|| defined(S6J3200_GRAPHIC)
					hmi_page_alpha	= hmi_dxy_page_rect[hmi_page_id].alpha;
					
					hmi_dxy_page_rect_bck[hmi_page_id].x	= hmi_page_rect.x;
					hmi_dxy_page_rect_bck[hmi_page_id].y	= hmi_page_rect.y;
					hmi_dxy_page_rect_bck[hmi_page_id].w	= hmi_page_rect.w;
					hmi_dxy_page_rect_bck[hmi_page_id].h	= hmi_page_rect.h;
					#endif
				}
				#ifdef HMI_GRAPHIC_TWLIB
				element_type=HMI_ELEM_TYPE_DPAGE;
				#endif
			}
			else
			#endif
			#if HMI_SXY_PAGES_NUMBER>0U
			if(HMI_IS_SXY_PAGE(hmi_page_id))					
			{
				if(HMI_IS_VIDEO_SXY_PAGE(hmi_page_id))
				{
					is_video_page	= TRUE;
					phmi_page_info	= NULL;	
					#ifndef HMI_GRAPHIC_RGL
					#if HMI_VDXY_PAGES_NUMBER+HMI_VSXY_PAGES_NUMBER > 0
					//hmi_engine_draw_video(hmi_page_id,FALSE);	
					#endif
					#endif
				}
				else
				{
					hmi_object_index=HMI_GET_PAGE_SXY_ID_INDEX(hmi_page_id);
					phmi_page_info	= &hmi_sxy_page_table[hmi_object_index];
					hmi_page_rect.x	= hmi_sxy_page_rect[hmi_object_index].x;
					hmi_page_rect.y	= hmi_sxy_page_rect[hmi_object_index].y;
					hmi_page_rect.w	= hmi_sxy_page_rect[hmi_object_index].w;
					hmi_page_rect.h	= hmi_sxy_page_rect[hmi_object_index].h;
					#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)|| defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(S6J3200_GRAPHIC)
					hmi_page_alpha	= hmi_sxy_page_rect[hmi_object_index].alpha;
					#endif
				}
				#ifdef HMI_GRAPHIC_TWLIB
				element_type=HMI_ELEM_TYPE_SPAGE;
				#endif
			}
			else
			#endif
			{
			}
			
			if(phmi_page_info != NULL)/*current page to be display*/
			{
				#if defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642)
				#if HMI_DYN_LANGUAGE_NUMBER > 0	/*language number >1*/
				if((HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(HMI_LANGUAGE) != 0U))/*if change language setting,all active page need refresh*/
				{				
					hmi_draw_all_element	= TRUE;
					HMI_DYNAMIC_OBJECT_CANCEL_CHANGED_FLAG(HMI_LANGUAGE);
				}
				#endif
				#endif
				if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_page_id) != 0)
				{
					#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)
					hmi_page_flag	= TRUE;
					#endif
					#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||\
						defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||\
						defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||	\
						defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))					
					HMI_DYNAMIC_OBJECT_CANCEL_CHANGED_FLAG(hmi_page_id);
					#elif defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642)
					hmi_draw_all_element	= TRUE;
					HMI_DYNAMIC_OBJECT_CANCEL_CHANGED_FLAG(hmi_page_id);
					#else /*tw8836*/
					bDrawBck=TRUE;
					#if HMI_RENDER_ALL_EXCEPT_BCK==NO
					if(hmi_is_render_mode())
					{
						HMI_DYNAMIC_OBJECT_CANCEL_CHANGED_FLAG(hmi_page_id);
					}
					#endif
					#endif
				}
				#if defined(HMI_GRAPHIC_TWLIB)
				else
				{
					bDrawBck=FALSE;
				}
				#endif
				#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||	\
						defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||	\
						defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||	\
						defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))	
				#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)
				if((HMI_GFX_GET_STATUS(HMI_RGL_PROP_EVENT))||
					(hmi_page_flag	== TRUE))
				{
					hmi_driver_set_page_alpha(hmi_page_alpha,FALSE);
				}
				#endif
				hmi_page_scale_alpha.alpha 		= 255U;
				hmi_page_scale_alpha.scale 		= 1.0f;
				hmi_page_scale_alpha.point.x	= HMI_INVALID_COOR;
				hmi_page_scale_alpha.point.y	= HMI_INVALID_COOR;
				
				hmi_engine_draw_container(&(phmi_page_info->container)/*node list*/,
						(HMI_RECT_STR CONST *)(&hmi_page_rect)/*father zone*/,
						/*(HMI_RECT_STR CONST *)phmi_page_rect,*/
						pdirty_zone,HMI_PAGE_BEGIN_DEPTH,
						&hmi_screen_rect
						#if (defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_RGL))
						,&hmi_page_scale_alpha
						#endif
						);
				#elif (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
				hmi_engine_draw_container(&(phmi_page_info->container)/*node list*/,
					(HMI_RECT_STR CONST *)(&hmi_page_rect),
					hmi_draw_all_element
					);
				#else	/*tw8836*/	
				
				#if HMI_RENDER_ALL_EXCEPT_BCK==YES
				if(bDrawBck)/*draw a page*/
				{
					clear_osdwin_fontwin_status(FALSE/*no off win*/);	/*draw new page before,clear window status*/		
				}
				else
				{
					clear_osdwin_not_include_bck_status(FALSE/*no operation win*/);/*refresh page*/
				}
				HMI_wait_v_blank(1);
				
				hmi_engine_draw_container(&phmi_page_info->container,
											(HMI_RECT_STR CONST *)(&hmi_page_rect),
											element_type,
											hmi_page_id,
											,bDrawBck
											);
				hmi_off_all_free_win();/*off free window*/
				#else	/*only draw set flag element*/
				if(hmi_is_render_mode())
				{
					HMI_wait_v_blank(1);
					
					#ifdef HMI_MUL_PALLETE_ENABLE
					#if HMI_SXY_PAGES_NUMBER>0U
					if(HMI_IS_SXY_PAGE(hmi_page_id))						
					{
						hmi_object_index	= HMI_GET_PAGE_SXY_ID_INDEX(hmi_page_id);
						if((hmi_object_index < HMI_SXY_PAGES_NUMBER))
						{							
							hmi_load_one_page_pallete(&hmi_sxy_page_pallete_table[hmi_object_index]);
							if(hmi_sxy_page_a_table[hmi_object_index] == HMI_NB_ELEMENTS)
							{
								exist_group_a	= FALSE;
							}
							else
							{
								exist_group_a	= TRUE;
							}
							hmi_set_groupA_mode(exist_group_a);
						}
					}
					#endif
					#if HMI_DXY_PAGES_NUMBER>0U
					if(HMI_IS_DXY_PAGE(hmi_page_id))						
					{
						hmi_object_index	= HMI_GET_PAGE_DXY_ID_INDEX(hmi_page_id);
						if(hmi_object_index < HMI_DXY_PAGES_NUMBER)
						{
							hmi_load_one_page_pallete(&hmi_dxy_page_pallete_table[hmi_object_index]);
							if(hmi_dxy_page_a_table[hmi_object_index] == HMI_NB_ELEMENTS)
							{
								exist_group_a	= FALSE;
							}
							else
							{
								exist_group_a	= TRUE;
							}
							hmi_set_groupA_mode(exist_group_a);
						}
					}
					#endif	
					#endif
					hmi_engine_draw_container(&phmi_page_info->container,
											(HMI_RECT_STR CONST *)(&hmi_page_rect),
											element_type,
											hmi_page_id,
											TRUE/*bDrawBck*/
											#if HMI_RENDER_ALL_EXCEPT_BCK==NO
											,HMI_NB_ELEMENTS/*no parent*/
											,!bDrawBck/*draw all element*/
											#endif
											);
				}
				else/*simulate*/
				{
					if(bDrawBck)/*draw a new page*/
					{
						hmi_clear_no_all_win();
					}
					else/*refresh a page.find not used win*/
					{
						hmi_engine_draw_container(&phmi_page_info->container,
											(HMI_RECT_STR CONST *)(&hmi_page_rect),
											element_type,
											hmi_page_id,
											TRUE/*bDrawBck*/
											#if HMI_RENDER_ALL_EXCEPT_BCK==NO
											,HMI_NB_ELEMENTS/*no parent*/
											,FALSE/*search all element*/
											#endif
											);
					}
				}								
				#endif
				#endif						
			}
			else /*no page to be display*/
			{
				#ifdef HMI_GRAPHIC_TWLIB /*close all win*/
				#if HMI_RENDER_ALL_EXCEPT_BCK==YES
				clear_osdwin_fontwin_status(FALSE/*no off win*/);	/*draw new page before,clear window status*/
				hmi_off_all_free_win();/*off free window*/
				#else	/**/
				
				#endif
				#endif
			}
		}	
	}
	}
	if(!is_video_page)
	{
	#if defined(HMI_GRAPHIC_TWLIB)	
		call_C_hmi_driver_refresh_LCD(bDrawBck);
	#elif defined(HMI_GRAPHIC_RGL)
		call_C_hmi_driver_refresh_LCD(pdirty_zone);
	#elif defined(S6J3200_GRAPHIC)
		call_C_hmi_driver_refresh_LCD(pdirty_zone);
	#elif defined(HMI_GRAPHIC_OPENGLES)
		call_C_hmi_driver_refresh_LCD(pdirty_zone,0);
	#elif (defined(HMI_GRAPHIC_YGV642)||defined(HMI_GRAPHIC_YGV641))
		call_C_hmi_driver_refresh_LCD();
	#elif defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)
		call_C_hmi_driver_refresh_LCD(pdirty_zone);
	#else
		call_C_hmi_driver_refresh_LCD(0);
	#endif
	}
	return(hmi_driver_woking_status_flag);
}
#endif


#if (HMI_DYN_CONTAINERS_NUMBER > 0)  && (HMI_ALL_STATIC_CONTAINERS_NUMBER > 0)
static BOOLEAN hmi_engine_get_static_container_id(HMI_OBJECT_ID_STR * phmi_dyn_contain_id/*dyn container index*/)
{
	BOOLEAN hmi_changed_flag	= FALSE;
	UINT8	loop				= 0;

	if((*phmi_dyn_contain_id) < HMI_DYN_CONTAINERS_NUMBER)
   	{
		*phmi_dyn_contain_id = hmi_dyn_container_table[*phmi_dyn_contain_id];
		while((HMI_IS_DYN_CONTAINER(*phmi_dyn_contain_id))&&
			(loop < HMI_DYN_CONTAINER_NESTED_DEPTH))
		{
			*phmi_dyn_contain_id = HMI_GET_DYN_CONTAINERS_ID_INDEX(*phmi_dyn_contain_id);
			if((*phmi_dyn_contain_id) < HMI_DYN_CONTAINERS_NUMBER)
		   	{
				*phmi_dyn_contain_id = hmi_dyn_container_table[*phmi_dyn_contain_id];
		   	}/*lq 2017.6.12*/
			else
			{
				loop	= HMI_DYN_CONTAINER_NESTED_DEPTH;/*end loop*/
			}
			loop++;
		}
		if(loop < HMI_DYN_CONTAINER_NESTED_DEPTH)
		{
			hmi_changed_flag	= TRUE;
		}
		else
		{
			*phmi_dyn_contain_id = HMI_NB_ELEMENTS;
			HMI_GFX_SET_STATUS(HMI_DYN_CONTAINER_NESTED);
		}
   	}/*lq 2017.6.12*/
	
	return(hmi_changed_flag);
}
#endif

#if HMI_ALL_DYN_OBJECTS_NUMBER > HMI_PAGES_NUMBER
#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||\
	defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||\
	defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||\
	defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))
static UINT8 hmi_engine_check_dynamic_object_changed(
								HMI_CONTAINER_STR CONST * phmi_container_info,
								HMI_RECT_STR CONST *pfarther_rect,								
								HMI_RECT_STR	*pdirty_rect,
								U08				depth,
								HMI_RECT_STR	*pcliped_farther_rect
								)
{
	UINT8 hmi_number_object			= phmi_container_info->container_object_table.object_number;
	UINT8 hmi_number_object_const	= hmi_number_object;
	#ifndef HMI_GRAPHIC_RGL
	#ifndef HMI_GRAPHIC_VGLITE
	#ifndef	HMI_GRAPHIC_OPENVG
	#ifndef HMI_GRAPHIC_OPENGLES
	#ifndef S6J3200_GRAPHIC
	UINT8 hmi_page_redrawed_flag	= FALSE;
	#endif
	#endif
	#endif
	#endif
	#endif
	
	UINT8 hmi_object_dirty_flag		= FALSE;
	U08	  hmi_depth					= depth;
	//HMI_RECT_STR	*pold_dirty		= NULL;	
	HMI_OBJECT_PROP_STR CONST * phmi_container_object_table = phmi_container_info->container_object_table.p_object_table;
	HMI_OBJECT_ID_STR	hmi_object_id		= 0U;
	HMI_OBJECT_ID_STR	hmi_object_index	= 0U;
	HMI_OBJECT_ID_STR	hmi_check_index		= 0U;
	HMI_RECT_STR hmi_node_screen_rect		= {HMI_INVALID_COOR,HMI_INVALID_COOR,0,0};
	
	HMI_RECT_STR hmi_insect_rect			= {HMI_INVALID_COOR,HMI_INVALID_COOR,0,0};
	#if HMI_DXY_ROTATION_BITMAPS_NUMBER+HMI_DXY_CUSTOM_CNT +HMI_SXY_CUSTOM_CNT > 0 
	HMI_OBJECT_PROP_STR	hmi_trail_object[1]	= {0};
	HMI_CONTAINER_STR	hmi_trail_container	= {{0,NULL}};
	#endif
	#if HMI_DXY_ROTATION_BITMAPS_NUMBER > 0 
	HMI_OBJECT_ID_STR	hmi_object_index2	= 0U;
	#endif
	HMI_RECT_STR		hmi_rect_temp		= {HMI_INVALID_COOR,HMI_INVALID_COOR,0,0};
	#if HMI_DXY_BITMAPS_NUMBER > 0
	HMI_RECT_STR			rotation_rect	= {0};
	HMI_ROTATION_STR	rotation			= {0,0};
	HMI_ROTATION_STR	rotation_old		= {0,0};
	HMI_RECT_STR hmi_node_screen_rect_old	= {HMI_INVALID_COOR,HMI_INVALID_COOR,0,0};
	float_32			angle_dt			= 0;
	#endif
	#if HMI_DXY_SPLINE_NUMBER + HMI_SXY_SPLINE_NUMBER > 0U
	HMI_OBJECT_ID_STR		spline_prop_id	= 0U;
	#endif
	#if HMI_DXY_CUSTOM_CNT +HMI_SXY_CUSTOM_CNT > 0U
	HMI_CUSTOM_PROP_STR	custom_object_prop  = {0U};
	BOOLEAN				get_success			= FALSE;
	HMI_RECT_STR		hmi_custom_rect		= {0U};
	INT32				custom_type			= -1;
	HMI_RECT_STR		sub_dirty_zone			= {0U};/*2024 04 011*/
	HMI_OBJECT_ID_STR	hmi_custom_type_index	= 0U;/*2024 04 011*/
	PCUSTOM_MANAGER_FUN	phmi_custom_function	= NULL;/*2024 04 011*/
	BOOLEAN				hmi_get_success			= FALSE;/*2024 04 011*/
	HMI_ALPHA_SCALE_PT_STR		hmi_page_scale_alpha = {255U,1.0f,{0,0}};
	#endif

	#if (defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_RGL))
	while(hmi_number_object > 0U)
	#else
	while((hmi_number_object > 0U) && (hmi_page_redrawed_flag == FALSE))
	#endif
	{
		if(hmi_depth == HMI_PAGE_BEGIN_DEPTH)/*search tree from page node*/
		{
			depth	= (hmi_number_object_const-hmi_number_object);/*node of page NO*/
		}
		hmi_object_id = phmi_container_object_table[hmi_check_index].object_id;

	  #if HMI_DXY_IMAGELIST_NUMBER > 0
		if(HMI_IS_DYN_IMAGELIST_DXY(hmi_object_id))
	    {
			hmi_object_index = HMI_GET_DYN_IMAGELIST_DXY_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DXY_IMAGELIST_NUMBER)
			{
				hmi_rect_temp.x	=	hmi_dxy_imagelist_rect[hmi_object_index].x;
				hmi_rect_temp.y	=	hmi_dxy_imagelist_rect[hmi_object_index].y;
				hmi_rect_temp.w	=	hmi_dxy_imagelist_rect[hmi_object_index].w;
				hmi_rect_temp.h	=	hmi_dxy_imagelist_rect[hmi_object_index].h;
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
						(&hmi_rect_temp),
						(&hmi_node_screen_rect));	
			}
			hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
								(HMI_RECT_STR CONST *)pcliped_farther_rect,
								&hmi_insect_rect);			
			if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
		 	{		
				hmi_set_dirty_zone_layer(&hmi_insect_rect,
										pdirty_rect,depth);
				/*add old dirty */
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
						(&hmi_dxy_imagelist_rect_bck[hmi_object_index]),
						(&hmi_node_screen_rect));
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
							(HMI_RECT_STR CONST *)pcliped_farther_rect,
							&hmi_insect_rect);
				hmi_set_dirty_zone_layer(&hmi_insect_rect,
									pdirty_rect,depth);
				
				//hmi_page_redrawed_flag	= TRUE;
				hmi_object_dirty_flag	= TRUE;
			}
			else
			{
				#if HMI_DXY_IMAGE_LIST_MAX_SON_CNT > 0
				if(hmi_object_index < HMI_DXY_IMAGELIST_NUMBER)
				{
					hmi_object_dirty_flag	= hmi_engine_check_dynamic_object_changed(
												&hmi_dxy_imagelist_container_table[hmi_object_index],
												(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),											
												pdirty_rect,
												depth,
												&hmi_insect_rect);												
				}
				#else
				hmi_object_dirty_flag = FALSE;
				#endif
			}
		}
		else
		#endif
		#if HMI_SXY_IMAGELIST_NUMBER>0
		if(HMI_IS_DYN_IMAGELIST_SXY(hmi_object_id))
	    {
			hmi_object_index = HMI_GET_DYN_IMAGELIST_SXY_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_SXY_IMAGELIST_NUMBER)
			{
				hmi_rect_temp.x	=	hmi_sxy_imagelist_rect[hmi_object_index].x;
				hmi_rect_temp.y	=	hmi_sxy_imagelist_rect[hmi_object_index].y;
				hmi_rect_temp.w	=	hmi_sxy_imagelist_rect[hmi_object_index].w;
				hmi_rect_temp.h	=	hmi_sxy_imagelist_rect[hmi_object_index].h;
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
									(&hmi_rect_temp),
									(&hmi_node_screen_rect));
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
									(HMI_RECT_STR CONST *)pcliped_farther_rect,
									&hmi_insect_rect);
			}
			if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
		 	{								
				hmi_set_dirty_zone_layer((HMI_RECT_STR *)(&hmi_insect_rect),
					pdirty_rect,depth);
				/*hmi_page_redrawed_flag = TRUE;*/
				hmi_object_dirty_flag	= TRUE;
			}
			else
			{	
				#if HMI_SXY_IMAGE_LIST_MAX_SON_CNT > 0
				if(hmi_object_index < HMI_SXY_IMAGELIST_NUMBER)
				{
					hmi_object_dirty_flag = hmi_engine_check_dynamic_object_changed(
										&hmi_sxy_imagelist_container_table[hmi_object_index],
										(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),									
										pdirty_rect,
										depth,
										&hmi_insect_rect);
				}
				#else
				hmi_object_dirty_flag = FALSE;
				#endif
			}
		}
		else
		#endif
		#if HMI_DXY_SCROLLBAR_NUMBER>0
		if(HMI_IS_DYN_SCROLLBAR_DXY(hmi_object_id))
	    {
			hmi_object_index = HMI_GET_DYN_SCROLLBAR_DXY_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DXY_SCROLLBAR_NUMBER)
			{
				hmi_rect_temp.x	=	hmi_dxy_scrollbar_rect[hmi_object_index].x;
				hmi_rect_temp.y	=	hmi_dxy_scrollbar_rect[hmi_object_index].y;
				hmi_rect_temp.w	=	hmi_dxy_scrollbar_rect[hmi_object_index].w;
				hmi_rect_temp.h	=	hmi_dxy_scrollbar_rect[hmi_object_index].h;
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
							(&hmi_rect_temp),
							(&hmi_node_screen_rect));											
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
					(HMI_RECT_STR CONST *)pcliped_farther_rect,&hmi_insect_rect);
			}
			if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
		 	{
				hmi_set_dirty_zone_layer(&hmi_node_screen_rect,
										pdirty_rect,depth);									
				/*add old dirty*/
				if(hmi_object_index < HMI_DXY_SCROLLBAR_NUMBER)
				{
					HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
								(&hmi_dxy_scrollbar_rect_bck[hmi_object_index]),
								(&hmi_node_screen_rect));
					hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
								(HMI_RECT_STR CONST *)pcliped_farther_rect,
								&hmi_insect_rect);
					hmi_set_dirty_zone_layer(&hmi_insect_rect,
										pdirty_rect,depth);
				}
				
				hmi_object_dirty_flag	= TRUE;				
				//hmi_page_redrawed_flag	= TRUE;
			}
			else
			{				
				#if HMI_DXY_SCROLLBAR_MAX_SON_CNT > 0
				if(hmi_object_index < HMI_DXY_SCROLLBAR_NUMBER)
				{
					hmi_object_dirty_flag=hmi_engine_check_dynamic_object_changed(
										&hmi_dxy_scrollbar_container_table[hmi_object_index],
										(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),										
										pdirty_rect,
										depth,
										&hmi_insect_rect);
				}
				#else
				hmi_object_dirty_flag = FALSE;
				#endif
			}
		}
		else
		#endif
		#if HMI_SXY_SCROLLBAR_NUMBER>0
		if(HMI_IS_DYN_SCROLLBAR_SXY(hmi_object_id))
	    {
			hmi_object_index = HMI_GET_DYN_SCROLLBAR_SXY_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_SXY_SCROLLBAR_NUMBER)
			{
				hmi_rect_temp.x	=	hmi_sxy_scrollbar_rect[hmi_object_index].x;
				hmi_rect_temp.y	=	hmi_sxy_scrollbar_rect[hmi_object_index].y;
				hmi_rect_temp.w	=	hmi_sxy_scrollbar_rect[hmi_object_index].w;
				hmi_rect_temp.h	=	hmi_sxy_scrollbar_rect[hmi_object_index].h;
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
					(&hmi_rect_temp),
					(&hmi_node_screen_rect));
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
					(HMI_RECT_STR CONST *)pcliped_farther_rect,&hmi_insect_rect);
			}
			if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
		 	{
				hmi_set_dirty_zone_layer(&hmi_insect_rect,pdirty_rect,depth);	
				/*hmi_page_redrawed_flag = TRUE;*/
				hmi_object_dirty_flag=TRUE;
			}
			else
			{
				#if HMI_SXY_SCROLLBAR_MAX_SON_CNT > 0
				if(hmi_object_index < HMI_SXY_SCROLLBAR_NUMBER)
				{
					hmi_object_dirty_flag=hmi_engine_check_dynamic_object_changed(
						&hmi_sxy_scrollbar_container_table[hmi_object_index],
						(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),						
						pdirty_rect,
						depth,
						&hmi_insect_rect);
				}
				#else
				hmi_object_dirty_flag = FALSE;
				#endif
			}
		}
		else
		#endif
		#if HMI_DXY_BUTTON_NUMBER>0
		if(HMI_IS_DYN_BUTTON_DXY(hmi_object_id))
	    {
			hmi_object_index = HMI_GET_DYN_BUTTON_DXY_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DXY_BUTTON_NUMBER)
			{	
				hmi_rect_temp.x	=	hmi_dxy_button_rect[hmi_object_index].x;
				hmi_rect_temp.y	=	hmi_dxy_button_rect[hmi_object_index].y;
				hmi_rect_temp.w	=	hmi_dxy_button_rect[hmi_object_index].w;
				hmi_rect_temp.h	=	hmi_dxy_button_rect[hmi_object_index].h;
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
					(&hmi_rect_temp),
					(&hmi_node_screen_rect));													
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
					(HMI_RECT_STR CONST *)pcliped_farther_rect,
					&hmi_insect_rect);
			}
			if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
		 	{
				hmi_set_dirty_zone_layer((HMI_RECT_STR *)(&hmi_insect_rect),
										pdirty_rect,depth);
				/*add old dirty*/
				if(hmi_object_index < HMI_DXY_BUTTON_NUMBER)
				{
					HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
						(&hmi_dxy_button_rect_bck[hmi_object_index]),
						(&hmi_node_screen_rect));
					hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
								(HMI_RECT_STR CONST *)pcliped_farther_rect,
								&hmi_insect_rect);
					hmi_set_dirty_zone_layer(&hmi_insect_rect,
										pdirty_rect,depth);
					//hmi_page_redrawed_flag = TRUE;
				}
				hmi_object_dirty_flag = TRUE;
			}
			else
			{		
				#if HMI_DXY_BUTTON_MAX_SON_CNT > 0
				if(hmi_object_index < HMI_DXY_BUTTON_NUMBER)
				{
					hmi_object_dirty_flag=hmi_engine_check_dynamic_object_changed(
								&hmi_dxy_button_container_table[hmi_object_index],
								(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
								pdirty_rect,
								depth,
								&hmi_insect_rect);								
				}
				#else
				hmi_object_dirty_flag = FALSE;
				#endif
			}
		}
		else
		#endif
		#if HMI_SXY_BUTTON_NUMBER>0
		if(HMI_IS_DYN_BUTTON_SXY(hmi_object_id))
	    {
			hmi_object_index = HMI_GET_DYN_BUTTON_SXY_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_SXY_BUTTON_NUMBER)
			{
				hmi_rect_temp.x	=	hmi_sxy_button_rect[hmi_object_index].x;
				hmi_rect_temp.y	=	hmi_sxy_button_rect[hmi_object_index].y;
				hmi_rect_temp.w	=	hmi_sxy_button_rect[hmi_object_index].w;
				hmi_rect_temp.h	=	hmi_sxy_button_rect[hmi_object_index].h;
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
						(&hmi_rect_temp),
						(&hmi_node_screen_rect));
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
					(HMI_RECT_STR CONST *)pcliped_farther_rect,&hmi_insect_rect);
			}
			if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
		 	{
				hmi_set_dirty_zone_layer(&hmi_insect_rect,pdirty_rect,depth);	
				/*hmi_page_redrawed_flag = TRUE;*/
				hmi_object_dirty_flag=TRUE;
			}
			else
			{
				#if HMI_SXY_BUTTON_MAX_SON_CNT > 0
				if(hmi_object_index < HMI_SXY_BUTTON_NUMBER)
				{
					hmi_object_dirty_flag=hmi_engine_check_dynamic_object_changed(
						&hmi_sxy_button_container_table[hmi_object_index],
						(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
						pdirty_rect,
						depth,
						&hmi_insect_rect);
				}
				#else
				hmi_object_dirty_flag = FALSE;
				#endif
			}
		}
		else
		#endif
	   #if HMI_DYN_EDIT_TEXTS_NUMBER/*editable text*/ > 0 
	    if(HMI_IS_DYN_TEXTS(hmi_object_id))
		{		
			#if HMI_DYN_XY_EDIT_TEXTS_NUMBER > 0
			if(HMI_IS_DYN_TEXT_DXY(hmi_object_id))
			{
				if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
				{
					hmi_object_index = HMI_GET_DYN_TEXTS_DXY_POS_INDEX(hmi_object_id);
					if(hmi_object_index < HMI_DYN_XY_EDIT_TEXTS_NUMBER)
					{
						
						HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
							(&(hmi_dyn_xy_edit_text_prop_table[hmi_object_index].text_rect)),
							(&hmi_node_screen_rect));
						hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
							(HMI_RECT_STR CONST *)pcliped_farther_rect,
							&hmi_insect_rect);																				
						hmi_set_dirty_zone_layer((&hmi_insect_rect),
										pdirty_rect,depth);	
						/*add old dirty*/
						HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
							(&hmi_dyn_xy_edit_text_prop_table_bck[hmi_object_index]),
							(&hmi_node_screen_rect));
						hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
							(HMI_RECT_STR CONST *)pcliped_farther_rect,
							&hmi_insect_rect);
						hmi_set_dirty_zone_layer((&hmi_insect_rect),
									pdirty_rect,depth);
							//hmi_page_redrawed_flag = TRUE;
						
						hmi_object_dirty_flag = TRUE;
						}					
				}
				else
				{
					#if HMI_EDIT_TEXT_MAX_SON_CNT > 0
					hmi_object_index = HMI_GET_DYN_TEXTS_DXY_POS_INDEX(hmi_object_id);
					if(hmi_object_index < HMI_DYN_XY_EDIT_TEXTS_NUMBER)
					{
						HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
								(&(hmi_dyn_xy_edit_text_prop_table[hmi_object_index].text_rect)),
								(&hmi_node_screen_rect));
						
						hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
							(HMI_RECT_STR CONST *)pcliped_farther_rect,&hmi_insect_rect);
						hmi_object_index = HMI_GET_DYN_TEXTS_INDEX(hmi_object_id);/*all editable text container info at one array*/
						if(hmi_object_index < HMI_DYN_EDIT_TEXTS_NUMBER)
						{
							hmi_object_dirty_flag=hmi_engine_check_dynamic_object_changed(
											&hmi_edit_text_container_table[hmi_object_index],
											(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
											pdirty_rect,
											depth,&hmi_insect_rect);
						}

					}
					#else
					hmi_object_dirty_flag = FALSE;
					#endif
				}
				
			}
			else
			#endif
			#if HMI_S_XY_EDIT_TEXTS_NUMBER > 0
			if(HMI_IS_DYN_TEXT_SXY(hmi_object_id))
			{
				#if HMI_S_XY_EDIT_TEXTS_NUMBER > 0	
				hmi_object_index = HMI_GET_DYN_TEXTS_SXY_POS_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_S_XY_EDIT_TEXTS_NUMBER)
				{
					HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
						(&(hmi_static_xy_edit_text_prop_table[hmi_object_index].text_rect)),
						(&hmi_node_screen_rect));
					hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
						(HMI_RECT_STR CONST *)pcliped_farther_rect,
						&hmi_insect_rect);
				}	
				#endif
				if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
				{
					hmi_set_dirty_zone_layer(&hmi_insect_rect,
									pdirty_rect,depth);	
					/*hmi_page_redrawed_flag = TRUE;*/
					hmi_object_dirty_flag = TRUE;
				}
				else
				{
					#if HMI_DYN_EDIT_TEXTS_NUMBER > 0	
					#if HMI_EDIT_TEXT_MAX_SON_CNT > 0
					hmi_object_index = HMI_GET_DYN_TEXTS_INDEX(hmi_object_id);	
					if(hmi_object_index < HMI_DYN_EDIT_TEXTS_NUMBER)
					{
						hmi_object_dirty_flag=hmi_engine_check_dynamic_object_changed(
							&hmi_edit_text_container_table[hmi_object_index],
							(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
							pdirty_rect,depth,
							&hmi_insect_rect);
					}
					#else
					hmi_object_dirty_flag = FALSE;
					#endif
					#endif
				}
			}
			else
			#endif
			{
			}			 			
		}
	    else
	   #endif
	   #if HMI_DYN_CONTAINERS_NUMBER/*dyn container*/ > 0 
		if(HMI_IS_DYN_CONTAINER(hmi_object_id))
		{
			#if HMI_ALL_STATIC_CONTAINERS_NUMBER > 0  			
			if(hmi_engine_check_container_changed(hmi_object_id,
				(HMI_RECT_STR CONST *)pfarther_rect,
				pdirty_rect,depth,
				pcliped_farther_rect) != FALSE)
			{
				//hmi_page_redrawed_flag	= TRUE;
				hmi_object_dirty_flag	= TRUE;
			}			
		    #endif
		  }
		else
	   #endif
	   #if HMI_DYN_FILL_PAGES_NUMBER > 0
		if(HMI_IS_DYN_NFILL(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_NFILL_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DYN_FILL_PAGES_NUMBER)
			{
				hmi_rect_temp.x	=	hmi_fills_dyn_xy_rect[hmi_object_index].x;
				hmi_rect_temp.y	=	hmi_fills_dyn_xy_rect[hmi_object_index].y;
				hmi_rect_temp.w	=	hmi_fills_dyn_xy_rect[hmi_object_index].w;
				hmi_rect_temp.h	=	hmi_fills_dyn_xy_rect[hmi_object_index].h;
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
						(&hmi_rect_temp),
						(&hmi_node_screen_rect));
				
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
					(HMI_RECT_STR CONST *)pcliped_farther_rect,
					&hmi_insect_rect);
			}
			if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
		 	{
				hmi_set_dirty_zone_layer(&hmi_insect_rect,
										pdirty_rect,depth);	
				/*add old dirty*/
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
					(&hmi_fills_dyn_xy_rect_bck[hmi_object_index]),
					(&hmi_node_screen_rect));
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
							(HMI_RECT_STR CONST *)pcliped_farther_rect,
							&hmi_insect_rect);
				hmi_set_dirty_zone_layer(&hmi_insect_rect,
									pdirty_rect,depth);
					
				//hmi_page_redrawed_flag = TRUE;
				
				hmi_object_dirty_flag = TRUE;
			}
			else
			{
				#if HMI_DXY_FILL_MAX_SON_CNT > 0
				if(hmi_object_index < HMI_DYN_FILL_PAGES_NUMBER)
				{
					hmi_object_dirty_flag=hmi_engine_check_dynamic_object_changed(
						&hmi_dyn_fill_container_table[hmi_object_index],
						(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
						pdirty_rect,
						depth,&hmi_insect_rect);						
				}
				#else
				hmi_object_dirty_flag = FALSE;
				#endif
			}
		}
		else
	   #endif
	   #if HMI_DYN_GFILL_NUMBER > 0
		if(HMI_IS_DYN_GFILL(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_GFILL_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DYN_GFILL_NUMBER)
			{
				hmi_rect_temp.x	=	hmi_gradient_dxy_fill_rect[hmi_object_index].x;
				hmi_rect_temp.y	=	hmi_gradient_dxy_fill_rect[hmi_object_index].y;
				hmi_rect_temp.w	=	hmi_gradient_dxy_fill_rect[hmi_object_index].w;
				hmi_rect_temp.h	=	hmi_gradient_dxy_fill_rect[hmi_object_index].h;
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
						(&hmi_rect_temp),
						(&hmi_node_screen_rect));
				
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
					(HMI_RECT_STR CONST *)pcliped_farther_rect,&hmi_insect_rect);
			}
			if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
		 	{
				hmi_set_dirty_zone_layer(&hmi_insect_rect,
										pdirty_rect,depth);
				
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
					(&hmi_gradient_dxy_fill_rect_bck[hmi_object_index]),
					(&hmi_node_screen_rect));
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
							(HMI_RECT_STR CONST *)pcliped_farther_rect,
							&hmi_insect_rect);
				hmi_set_dirty_zone_layer(&hmi_insect_rect,
									pdirty_rect,depth);
				//hmi_page_redrawed_flag	= TRUE;
				
				hmi_object_dirty_flag	= TRUE;
			}
			else
			{
				#if HMI_DXY_GFILL_MAX_SON_CNT > 0
				if(hmi_object_index < HMI_DYN_GFILL_NUMBER)
				{
					hmi_object_dirty_flag=hmi_engine_check_dynamic_object_changed(
						&hmi_dyn_gfill_container_table[hmi_object_index],
						(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
						pdirty_rect,
						depth,&hmi_insect_rect);
				}
				#else
				hmi_object_dirty_flag = FALSE;
				#endif
			}
		}
		else
	   #endif
		#if HMI_DXY_CUBE_NUMBER > 0
		if(HMI_IS_DYN_CUBE(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_CUBE_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DXY_CUBE_NUMBER)
			{				
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
					(&hmi_cubes_dyn_xy_rect[hmi_object_index].cube_rect),
					(&hmi_node_screen_rect));
				
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
					(HMI_RECT_STR CONST *)pcliped_farther_rect,&hmi_insect_rect);
			}
			if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)/*if cube rect changed ,should be refresh cube father rect*/
		 	{
				hmi_set_dirty_zone_layer(pcliped_farther_rect,
										pdirty_rect,depth);
				#if 0
				/*add old dirty*/
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
				(&hmi_cubes_dyn_xy_rect_bck[hmi_object_index]),
				(&hmi_node_screen_rect));
				
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
					(HMI_RECT_STR CONST *)pcliped_farther_rect,&hmi_insect_rect);
				hmi_set_dirty_zone_layer(&hmi_insect_rect,
										pdirty_rect,depth);
				#endif
				//hmi_page_redrawed_flag	= TRUE;
				
				hmi_object_dirty_flag	= TRUE;
			}
			else
			{
				if(hmi_object_index < HMI_DXY_CUBE_NUMBER)
				{
					hmi_object_dirty_flag=hmi_engine_check_dynamic_object_changed(
						&hmi_cubes_dyn_xy_face[hmi_object_index],
						(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
						pdirty_rect,
						depth,&hmi_insect_rect);						
					if(hmi_object_dirty_flag == TRUE)/*if cube face changed ,cube should be change*/
					{
						hmi_set_dirty_zone_layer((HMI_RECT_STR *)pcliped_farther_rect,
										pdirty_rect,depth);
					}
				}
			}
			
		}
		else 
		#endif
		
		#if HMI_DXY_3DCUBE_NUMBER > 0
		if(HMI_IS_DYN_3DCUBE(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_3DCUBE_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DXY_3DCUBE_NUMBER)
			{				
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
					(&hmi_3dcubes_dyn_xy_rect[hmi_object_index].cube_rect),
					(&hmi_node_screen_rect));
				
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
					(HMI_RECT_STR CONST *)pcliped_farther_rect,&hmi_insect_rect);
			}
			if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)/*if cube rect changed ,should be refresh cube father rect*/
		 	{
				hmi_set_dirty_zone_layer(pcliped_farther_rect,
										pdirty_rect,depth);
		#if 0
				/*add old dirty*/
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
				(&hmi_cubes_dyn_xy_rect_bck[hmi_object_index]),
				(&hmi_node_screen_rect));
				
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
					(HMI_RECT_STR CONST *)pcliped_farther_rect,&hmi_insect_rect);
				hmi_set_dirty_zone_layer(&hmi_insect_rect,
										pdirty_rect,depth);
		#endif
				//hmi_page_redrawed_flag	= TRUE;
				
				hmi_object_dirty_flag	= TRUE;
			}
			else
			{
			}
			
		}
		else 
#endif
	   #if HMI_DXY_CONTAINERS_NUMBER > 0U 
		if(HMI_IS_DYN_XY_CONTAINER(hmi_object_id))
		{
			if(hmi_engine_check_container_changed(hmi_object_id,
					(HMI_RECT_STR CONST *)pfarther_rect,
					pdirty_rect,depth,
					pcliped_farther_rect) != FALSE)
			{
				hmi_object_dirty_flag  = TRUE;
			}
		}
	    else
	   #endif
	   #if HMI_DXY_BITMAPS_NUMBER > 0
	   	if(HMI_IS_DYN_XY_BITMAP(hmi_object_id))
	   	{
			hmi_object_index = HMI_GET_DYN_XY_BITMAP_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DXY_BITMAPS_NUMBER)
			{
				
				hmi_rect_temp.x	=	hmi_bmp_dyn_xy_rect[hmi_object_index].x;
				hmi_rect_temp.y	=	hmi_bmp_dyn_xy_rect[hmi_object_index].y;
				hmi_rect_temp.w	=	hmi_bmp_dyn_xy_rect[hmi_object_index].w;
				hmi_rect_temp.h	=	hmi_bmp_dyn_xy_rect[hmi_object_index].h;
				
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
						(&hmi_rect_temp),
						(&hmi_node_screen_rect));
				angle_dt	= (float_32)fabs(hmi_bmp_dyn_xy_rect[hmi_object_index].angel-
								hmi_bmp_dyn_angel_bck[hmi_object_index]);
				if(angle_dt > HMI_FLOAT_TOLERANCE)
				{
					hmi_rect_temp.x	=	hmi_bmp_dyn_xy_rect_bck[hmi_object_index].x;
					hmi_rect_temp.y	=	hmi_bmp_dyn_xy_rect_bck[hmi_object_index].y;
					hmi_rect_temp.w	=	hmi_bmp_dyn_xy_rect_bck[hmi_object_index].w;
					hmi_rect_temp.h	=	hmi_bmp_dyn_xy_rect_bck[hmi_object_index].h;
						
					HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
								(&hmi_rect_temp),
								(&hmi_node_screen_rect_old));
					#if HMI_DXY_ROTATION_BITMAPS_NUMBER > 0 
					if(hmi_object_index >= HMI_DXY_CENTER_BITMAPS_NUMBER)/*rotation user define */
					{	
						hmi_object_index2	= hmi_object_index-HMI_DXY_CENTER_BITMAPS_NUMBER;
						if(hmi_object_index2 < HMI_DXY_ROTATION_BITMAPS_NUMBER)
						{
							HMI_GET_OBJECT_SCREEN_COOR2(pfarther_rect,
														(&hmi_dxy_bitmap_rotation[hmi_object_index2]),
														(&rotation));
							hmi_get_rotation_pointer_dirty(&hmi_node_screen_rect,
									&hmi_node_screen_rect_old,
									&rotation,&rotation,
									hmi_bmp_dyn_xy_rect[hmi_object_index].angel,
									hmi_bmp_dyn_angel_bck[hmi_object_index],
									&rotation_rect,
									hmi_dxy_bitmap_rotation_trail[hmi_object_index2].trail_en,
									pcliped_farther_rect);
							hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&rotation_rect),
							(HMI_RECT_STR CONST *)(pcliped_farther_rect),&hmi_insect_rect);
						}
					}
					else/*rotation center*/
					#endif
					{						
						rotation.x	= hmi_node_screen_rect.x + (hmi_node_screen_rect.w>>1)/*/2*/;	
						rotation.y	= hmi_node_screen_rect.y + (hmi_node_screen_rect.h>>1)/*/2*/;				
						rotation_old.x	= hmi_node_screen_rect_old.x + (hmi_node_screen_rect_old.w>>1);
						rotation_old.y	= hmi_node_screen_rect_old.y + (hmi_node_screen_rect_old.h>>1);
						
						hmi_get_rotation_pointer_dirty(&hmi_node_screen_rect,
								&hmi_node_screen_rect_old,
								&rotation,&rotation_old,
								hmi_bmp_dyn_xy_rect[hmi_object_index].angel,
								hmi_bmp_dyn_angel_bck[hmi_object_index],&rotation_rect,
								FALSE,
								pcliped_farther_rect);
						hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&rotation_rect),
						(HMI_RECT_STR CONST *)(pcliped_farther_rect),&hmi_insect_rect);						
					}
				}
				else /*old angle == new angle*/
				{
					hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
						(HMI_RECT_STR CONST *)(pcliped_farther_rect),&hmi_insect_rect);
				}
			}
			if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
			{
				if(angle_dt > HMI_FLOAT_TOLERANCE)
				{
					#if HMI_DXY_ROTATION_BITMAPS_NUMBER > 0 
					if(hmi_object_index >= HMI_DXY_CENTER_BITMAPS_NUMBER)/*user define pointer*/
			    	{
						hmi_object_index2	= hmi_object_index-HMI_DXY_CENTER_BITMAPS_NUMBER;
						if(hmi_object_index2 < HMI_DXY_ROTATION_BITMAPS_NUMBER)
						{
							if(hmi_dxy_bitmap_rotation_trail[hmi_object_index2].trail_en ==TRUE)
							{
							
								#if (defined(S6J3200_GRAPHIC))
								hmi_object_dirty_flag  = TRUE;/*S6J200 trail dirty is father zone*/
								#else
								hmi_trail_object[0].object_id	= hmi_dxy_bitmap_rotation_trail[hmi_object_index2].texture;
								hmi_trail_container.container_object_table.object_number	= 1;
								hmi_trail_container.container_object_table.p_object_table	= hmi_trail_object;
										
								hmi_object_dirty_flag = hmi_engine_check_dynamic_object_changed(
																&hmi_trail_container,
																(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
																pdirty_rect,
																depth,
																&hmi_insect_rect);
								#endif
							}
							if(hmi_object_dirty_flag == TRUE)
							{
								hmi_set_dirty_zone_layer((HMI_RECT_STR *)pcliped_farther_rect/*trial image dirty zone is father zone*/,
															pdirty_rect,depth); 
								
								hmi_object_dirty_flag	= TRUE;
						
							}
							else
							{
								hmi_set_dirty_zone_layer((HMI_RECT_STR *)&hmi_insect_rect,
										pdirty_rect,depth);	
								hmi_object_dirty_flag	= TRUE;
							}
						}
					}
					else
					#endif
					{
						hmi_set_dirty_zone_layer((HMI_RECT_STR *)&hmi_insect_rect,
									pdirty_rect,depth);
						hmi_object_dirty_flag	= TRUE;
					}
						
				}
				else /*old angle == new angle*/
				{
					angle_dt	= (float_32)(fabs(hmi_bmp_dyn_xy_rect[hmi_object_index].angel));
					if(angle_dt > HMI_FLOAT_TOLERANCE)
					{
						hmi_rect_temp.x	=	hmi_bmp_dyn_xy_rect_bck[hmi_object_index].x;
						hmi_rect_temp.y	=	hmi_bmp_dyn_xy_rect_bck[hmi_object_index].y;
						hmi_rect_temp.w	=	hmi_bmp_dyn_xy_rect_bck[hmi_object_index].w;
						hmi_rect_temp.h	=	hmi_bmp_dyn_xy_rect_bck[hmi_object_index].h;
							
						HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
									(&hmi_rect_temp),
									(&hmi_node_screen_rect_old));
						#if HMI_DXY_ROTATION_BITMAPS_NUMBER > 0 
						if(hmi_object_index > HMI_DXY_CENTER_BITMAPS_NUMBER)
						{
							hmi_object_index2	= hmi_object_index-HMI_DXY_CENTER_BITMAPS_NUMBER;
							HMI_GET_OBJECT_SCREEN_COOR2(pfarther_rect,
														(&hmi_dxy_bitmap_rotation[hmi_object_index2]),
														(&rotation));
							
							hmi_get_rotation_pointer_dirty(&hmi_node_screen_rect,
									&hmi_node_screen_rect_old,
									&rotation,&rotation,
									hmi_bmp_dyn_xy_rect[hmi_object_index].angel,
									hmi_bmp_dyn_angel_bck[hmi_object_index],
									&rotation_rect,
									hmi_dxy_bitmap_rotation_trail[hmi_object_index2].trail_en,
									pcliped_farther_rect);
							hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&rotation_rect),
							(HMI_RECT_STR CONST *)(pcliped_farther_rect),&hmi_insect_rect);
						}
						else
						#endif
						{						
							rotation.x	= hmi_node_screen_rect.x + (hmi_node_screen_rect.w>>1)/*/2*/;	
							rotation.y	= hmi_node_screen_rect.y + (hmi_node_screen_rect.h>>1)/*/2*/;				
							rotation_old.x	= hmi_node_screen_rect_old.x + (hmi_node_screen_rect_old.w>>1);
							rotation_old.y	= hmi_node_screen_rect_old.y + (hmi_node_screen_rect_old.h>>1);
							
							hmi_get_rotation_pointer_dirty(&hmi_node_screen_rect,
									&hmi_node_screen_rect_old,
									&rotation,&rotation_old,
									hmi_bmp_dyn_xy_rect[hmi_object_index].angel,
									hmi_bmp_dyn_angel_bck[hmi_object_index],&rotation_rect,
									FALSE,
									pcliped_farther_rect);
							hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&rotation_rect),
							(HMI_RECT_STR CONST *)(pcliped_farther_rect),&hmi_insect_rect);						
						}
						hmi_set_dirty_zone_layer(&hmi_insect_rect,
								pdirty_rect,depth);
					}
					else
					{
						hmi_set_dirty_zone_layer(&hmi_insect_rect,
									pdirty_rect,depth);
						/*add old dirty zone*/
						hmi_rect_temp.x	=	hmi_bmp_dyn_xy_rect_bck[hmi_object_index].x;
						hmi_rect_temp.y	=	hmi_bmp_dyn_xy_rect_bck[hmi_object_index].y;
						hmi_rect_temp.w	=	hmi_bmp_dyn_xy_rect_bck[hmi_object_index].w;
						hmi_rect_temp.h	=	hmi_bmp_dyn_xy_rect_bck[hmi_object_index].h;
						
						HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
								(&hmi_rect_temp),
								(&hmi_node_screen_rect));
											
						hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
							(HMI_RECT_STR CONST *)(pcliped_farther_rect),&hmi_insect_rect);
						hmi_set_dirty_zone_layer(&hmi_insect_rect,
									pdirty_rect,depth);
					}
					//hmi_page_redrawed_flag	= TRUE;
					hmi_object_dirty_flag	= TRUE;
				}
				//hmi_object_dirty_flag	= TRUE;
			}
			else/*no set flag*/
			{				
			    if(hmi_object_index < HMI_DXY_BITMAPS_NUMBER)
			    {					
					#if HMI_DXY_IMAGE_MAX_SON_CNT > 0
					hmi_object_dirty_flag = hmi_engine_check_dynamic_object_changed(
												&hmi_dxy_bitmap_container_table[hmi_object_index],
												(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
												pdirty_rect,
												depth,&hmi_insect_rect);
					#else
					hmi_object_dirty_flag = 0;
					#endif
					/*trail*/					
					if(hmi_object_index >= HMI_DXY_CENTER_BITMAPS_NUMBER)/*rotation center*/
					{						
						#if HMI_DXY_ROTATION_BITMAPS_NUMBER > 0 
						if(hmi_object_index > HMI_DXY_CENTER_BITMAPS_NUMBER)
						{
							hmi_object_index2	= hmi_object_index-HMI_DXY_CENTER_BITMAPS_NUMBER;
						}
						else
						{
							hmi_object_index2	= 0U;
						}
						
						if(hmi_object_index2 < HMI_DXY_ROTATION_BITMAPS_NUMBER)
						{
							if(hmi_dxy_bitmap_rotation_trail[hmi_object_index2].trail_en ==TRUE)
							{
								hmi_trail_object[0].object_id	= hmi_dxy_bitmap_rotation_trail[hmi_object_index2].texture;
								hmi_trail_container.container_object_table.object_number	= 1;
								hmi_trail_container.container_object_table.p_object_table	= hmi_trail_object;
										
								hmi_object_dirty_flag = hmi_engine_check_dynamic_object_changed(
																&hmi_trail_container,
																(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
																pdirty_rect,
																depth,
																&hmi_insect_rect);
								if(hmi_object_dirty_flag == TRUE)
								{
									hmi_set_dirty_zone_layer((HMI_RECT_STR *)pcliped_farther_rect/*trial image dirty zone is father zone*/,
																pdirty_rect,depth); 
									
									//hmi_page_redrawed_flag	= TRUE;
									
									hmi_object_dirty_flag	= TRUE;

								}
							}
				    	}
						#endif
					}					
				}
			}
	   	}
		else
	   #endif
		#if HMI_DXY_SPLINE_NUMBER > 0
	   if(HMI_IS_DXY_SPLINE(hmi_object_id))
	   {
		   hmi_object_index = HMI_GET_DXY_SPLINE_INDEX(hmi_object_id);
		   if(hmi_object_index < HMI_DXY_SPLINE_NUMBER)
		   {			   
			   HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
				   (&hmi_dxy_spline_rect[hmi_object_index]),
				   (&hmi_node_screen_rect));
			   
			   hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
				   (HMI_RECT_STR CONST *)pcliped_farther_rect,&hmi_insect_rect);
		   }
		   if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
		   {
			   hmi_set_dirty_zone_layer(&hmi_insect_rect,
									   pdirty_rect,depth);
		#if 1
			   /*add old dirty*/
			   HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
										   (&hmi_spline_dyn_xy_rect_bck[hmi_object_index]),
										   (&hmi_node_screen_rect));
			   
			   hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
									   (HMI_RECT_STR CONST *)pcliped_farther_rect,&hmi_insect_rect);
			   hmi_set_dirty_zone_layer(&hmi_insect_rect,
									   pdirty_rect,depth);
		#endif
			   
			   hmi_object_dirty_flag   = TRUE;
		   }
		   else
		   {
			#if HMI_DXY_SPLINE_MAX_SON_CNT > 0
			   if(hmi_object_index < HMI_DXY_SPLINE_NUMBER)
			   {
				   hmi_object_dirty_flag=hmi_engine_check_dynamic_object_changed(
															   &hmi_dyn_xy_spline_table[hmi_object_index],
															   (HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
															   pdirty_rect,
															   depth,&hmi_insect_rect); 				   
			   }
			#endif
			
			   spline_prop_id  = hmi_dxy_spline_ctrl_point[hmi_object_index];
			   if(spline_prop_id !=HMI_NB_ELEMENTS)
			   {
				   if(HMI_IS_DXY_MUL_SPLINE(hmi_object_id))
				   {
					   hmi_object_dirty_flag = hmi_check_container_child_changed(spline_prop_id, TRUE);
				   }
				   else
				   {
					   hmi_object_dirty_flag = hmi_check_object_child_changed(spline_prop_id);
				   }
				   
				   if(hmi_object_dirty_flag == TRUE)/*if spline child changed ,spline should be change*/
				   {
					   hmi_set_dirty_zone_layer(&hmi_insect_rect,
												   pdirty_rect,depth);
				   }
				   else
				   {
					   spline_prop_id  = hmi_dxy_spline_color[hmi_object_index];
					   if(spline_prop_id !=HMI_NB_ELEMENTS)
					   {
						   hmi_object_dirty_flag = hmi_check_container_child_changed(spline_prop_id,FALSE);
						   if(hmi_object_dirty_flag == TRUE)/*if spline child changed ,spline should be change*/
						   {
							   hmi_set_dirty_zone_layer(&hmi_insect_rect,
														   pdirty_rect,depth);
						   }
						   else
						   {
							   spline_prop_id  = hmi_dxy_spline_zone_color[hmi_object_index];
							   if(spline_prop_id !=HMI_NB_ELEMENTS)
							   {
								   hmi_object_dirty_flag = hmi_check_container_child_changed(spline_prop_id,FALSE);
								   if(hmi_object_dirty_flag == TRUE)/*if spline child changed ,spline should be change*/
								   {
									   hmi_set_dirty_zone_layer(&hmi_insect_rect,
																   pdirty_rect,depth);
								   }
							   }
	   
						   }
					   }
				   }
			   }
		   }
	   }
	   else 
		#endif
		#if HMI_SXY_SPLINE_NUMBER > 0		
		if(HMI_IS_SXY_SPLINE(hmi_object_id))
		{
			hmi_object_index = HMI_GET_SXY_SPLINE_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_SXY_SPLINE_NUMBER)
			{				
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
					(&hmi_sxy_spline_rect[hmi_object_index]),
					(&hmi_node_screen_rect));
				
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
					(HMI_RECT_STR CONST *)pcliped_farther_rect,&hmi_insect_rect);
			}
			if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
		 	{
				hmi_set_dirty_zone_layer(&hmi_insect_rect,
										pdirty_rect,depth);				
				hmi_object_dirty_flag	= TRUE;
			}
			else
			{
		#if HMI_SXY_SPLINE_MAX_SON_CNT > 0
				if(hmi_object_index < HMI_SXY_SPLINE_NUMBER)
				{
					hmi_object_dirty_flag=hmi_engine_check_dynamic_object_changed(
																&hmi_sxy_spline_table[hmi_object_index],
																(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
																pdirty_rect,
																depth,&hmi_insect_rect);					
				}
		#endif
				spline_prop_id	= hmi_sxy_spline_ctrl_point[hmi_object_index];
				if(spline_prop_id !=HMI_NB_ELEMENTS)
				{
					if(HMI_IS_SXY_MUL_SPLINE(hmi_object_id))
					{
						hmi_object_dirty_flag = hmi_check_container_child_changed(spline_prop_id,TRUE);
					}
					else
					{
						hmi_object_dirty_flag = hmi_check_object_child_changed(spline_prop_id);
					}
					
					if(hmi_object_dirty_flag == TRUE)/*if spline child changed ,spline should be change*/
					{
						hmi_set_dirty_zone_layer(&hmi_insect_rect,
													pdirty_rect,depth);
					}
					else
					{
						spline_prop_id	= hmi_sxy_spline_color[hmi_object_index];
						if(spline_prop_id !=HMI_NB_ELEMENTS)
						{
							hmi_object_dirty_flag = hmi_check_container_child_changed(spline_prop_id,FALSE);
							if(hmi_object_dirty_flag == TRUE)/*if spline child changed ,spline should be change*/
							{
								hmi_set_dirty_zone_layer(&hmi_insect_rect,
															pdirty_rect,depth);
							}
							else
							{
								spline_prop_id	= hmi_sxy_spline_zone_color[hmi_object_index];
								if(spline_prop_id !=HMI_NB_ELEMENTS)
								{
									hmi_object_dirty_flag = hmi_check_container_child_changed(spline_prop_id,FALSE);
									if(hmi_object_dirty_flag == TRUE)/*if spline child changed ,spline should be change*/
									{
										hmi_set_dirty_zone_layer(&hmi_insect_rect,
																	pdirty_rect,depth);
									}
								}

							}
						}
					}
				}
			}
		}
		else 
		#endif
		#if HMI_DXY_CUSTOM_CNT > 0
		if(HMI_IS_DYN_XY_CUSTOM(hmi_object_id))
		{
			if(hmi_object_id > HMI_SXY_SPLINE_MAX_ID)
			{
				hmi_object_index = HMI_GET_DXY_CUSTOM_INDEX(hmi_object_id);
			}
			else
			{
				hmi_object_index = 0U;
			}
			get_success = hmi_engine_get_custom_prop(hmi_object_id,&custom_object_prop);
			if(get_success == TRUE)
			{
				hmi_custom_rect.x 	= custom_object_prop.x;
				hmi_custom_rect.y 	= custom_object_prop.y;
				hmi_custom_rect.w 	= custom_object_prop.w;
				hmi_custom_rect.h 	= custom_object_prop.h;
				if(hmi_object_index < HMI_DXY_CUSTOM_CNT)
				{				
					HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
						(&hmi_custom_rect),
						(&hmi_node_screen_rect));
					
					hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
						(HMI_RECT_STR CONST *)pcliped_farther_rect,&hmi_insect_rect);
				}
				if((HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)&&
					((hmi_custom_dyn_xy_rect_bck[hmi_object_index].x != hmi_custom_rect.x)||
					(hmi_custom_dyn_xy_rect_bck[hmi_object_index].y != hmi_custom_rect.y)||
					(hmi_custom_dyn_xy_rect_bck[hmi_object_index].w != hmi_custom_rect.w)||
					(hmi_custom_dyn_xy_rect_bck[hmi_object_index].h != hmi_custom_rect.h)))
			 	{
					hmi_set_dirty_zone_layer(&hmi_insect_rect,
											pdirty_rect,depth);	
					
					#if 1
				   /*add old dirty*/
				   HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
											   (&hmi_custom_dyn_xy_rect_bck[hmi_object_index]),
											   (&hmi_node_screen_rect));
				   
				   hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
										   (HMI_RECT_STR CONST *)pcliped_farther_rect,&hmi_insect_rect);
				   hmi_set_dirty_zone_layer(&hmi_insect_rect,
										   pdirty_rect,depth);
					#endif
					hmi_object_dirty_flag	= TRUE;
				}
				else
				{
					if((custom_object_prop.attr & HMI_ID1) !=0)
					{
						hmi_trail_object[0].object_id	= custom_object_prop.id1;
						hmi_trail_container.container_object_table.object_number	= 1;
						hmi_trail_container.container_object_table.p_object_table	= hmi_trail_object;
										
						hmi_object_dirty_flag = hmi_engine_check_dynamic_object_changed(
														&hmi_trail_container,
														(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
														pdirty_rect,
														depth,
														&hmi_insect_rect);
						if(hmi_object_dirty_flag == TRUE)
						{
							hmi_set_dirty_zone_layer(&hmi_insect_rect,
														pdirty_rect,depth);				
							hmi_object_dirty_flag	= TRUE;

						}
						
					}
					if(((custom_object_prop.attr & HMI_ID2) !=0)&&(hmi_object_dirty_flag == FALSE))
					{
						hmi_trail_object[0].object_id	= custom_object_prop.id2;
						hmi_trail_container.container_object_table.object_number	= 1;
						hmi_trail_container.container_object_table.p_object_table	= hmi_trail_object;
										
						hmi_object_dirty_flag = hmi_engine_check_dynamic_object_changed(
														&hmi_trail_container,
														(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
														pdirty_rect,
														depth,
														&hmi_insect_rect);
						if(hmi_object_dirty_flag == TRUE)
						{
							hmi_set_dirty_zone_layer(&hmi_insect_rect,
														pdirty_rect,depth);				
							hmi_object_dirty_flag	= TRUE;

						}
						
					}
					if(((custom_object_prop.attr & HMI_ID3) !=0)&&(hmi_object_dirty_flag == FALSE))
					{
						hmi_trail_object[0].object_id	= custom_object_prop.id3;
						hmi_trail_container.container_object_table.object_number	= 1;
						hmi_trail_container.container_object_table.p_object_table	= hmi_trail_object;
										
						hmi_object_dirty_flag = hmi_engine_check_dynamic_object_changed(
														&hmi_trail_container,
														(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
														pdirty_rect,
														depth,
														&hmi_insect_rect);
						if(hmi_object_dirty_flag == TRUE)
						{
							hmi_set_dirty_zone_layer(&hmi_insect_rect,
														pdirty_rect,depth);				
							hmi_object_dirty_flag	= TRUE;

						}
						
					}
					if(hmi_object_dirty_flag == FALSE)
					{
						#if 0
						custom_type = is3dcameralightcustom(hmi_object_id);
						if((custom_type == 0/*3d mode */)||(custom_type == 1/*3d mode */)||(custom_type == 6/*3d mode */))
						{
							hmi_object_dirty_flag |= hmi_engine_check_dynamic_object_changed(
															(HMI_CONTAINER_STR CONST * )(&(((HMI_3D_SUB_CUSTOM_USR_DATA_STR*)(hmi_dxy_custom_lib_data[hmi_object_index].pusr_data))->child)),
															(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
															pdirty_rect,
															depth,&hmi_insect_rect);
						}
						#endif
						/*p1 or  p2 or p3 changed*/
						if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
						{
							#ifdef HMI_RENDER_FULL_SCREEN//changed by pxguo 20240411
							hmi_set_dirty_zone_layer(&hmi_insect_rect,
												pdirty_rect,depth);	
							#else
							hmi_get_success = hmi_engine_get_dyn_custom_index(hmi_object_index,&hmi_custom_type_index);
							if(hmi_get_success == TRUE)
							{
								phmi_custom_function = hmi_dxy_custom_widget_info[hmi_custom_type_index].attr_fun.pmanager_fun;
								if(phmi_custom_function !=NULL)
								{
									sub_dirty_zone.w = 0u;/*lq 2024 11 13*/
									sub_dirty_zone.h = 0u;/*lq 2024 11 13*/
									phmi_custom_function(pfarther_rect,
																pdirty_rect,
																HMI_SUB_DIRTY_ZONE_FLAG,
																pcliped_farther_rect,
																hmi_object_id,
																&hmi_page_scale_alpha,/*pfather_alpha_scale,*/
																(void*)(&sub_dirty_zone));
									if((sub_dirty_zone.w != 0u) && (sub_dirty_zone.h != 0u))/*lq 2024 11 13*/
									{
										hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&sub_dirty_zone),
													   (HMI_RECT_STR CONST *)pcliped_farther_rect,&hmi_insect_rect);
							   			hmi_set_dirty_zone_layer(&hmi_insect_rect,
													   pdirty_rect,depth);
									}
									else	/*lq 2024 11 13*/
									{
										hmi_set_dirty_zone_layer(&hmi_insect_rect,
														pdirty_rect,depth);														
									}
								}
							}
							
							#endif
						}
					}
				}
			}
		}
		else
		#endif
		#if HMI_STATIC_TEXTS_NUMBER  > 0 
	    if(HMI_IS_STATIC_TEXTS(hmi_object_id)/*unedit text*/)
		{
			#if HMI_UNEDIT_TEXTS_DYN_XY_NUMBER>0
			if(HMI_IS_UNEDIT_TEXT_DYN_XY(hmi_object_id))
			{
				hmi_object_index  = HMI_GET_UNEDIT_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_UNEDIT_TEXTS_DYN_XY_NUMBER)
				{										
					HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
							(&(hmi_dyn_xy_unedit_text_prop_table[hmi_object_index].text_rect)),
							(&hmi_node_screen_rect));					
					hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
						(HMI_RECT_STR CONST *)pcliped_farther_rect,
						&hmi_insect_rect);
				}
				if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
				{
					hmi_set_dirty_zone_layer(&hmi_insect_rect,
										pdirty_rect,depth);						
					if(hmi_object_index < HMI_UNEDIT_TEXTS_DYN_XY_NUMBER)
					{
						HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
								(&(hmi_dyn_xy_unedit_text_prop_table_bck[hmi_object_index])),
								(&hmi_node_screen_rect));
						hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
									(HMI_RECT_STR CONST *)pcliped_farther_rect,
									&hmi_insect_rect);
						hmi_set_dirty_zone_layer(&hmi_insect_rect,
										pdirty_rect,depth);
						//hmi_page_redrawed_flag	= TRUE;
						
						hmi_object_dirty_flag	= TRUE;
					}
				}
				else
				{
					hmi_object_index = HMI_GET_UNEDIT_INDEX(hmi_object_id);/*all uneditable text contain at one array*/
					if(hmi_object_index < HMI_UNEDIT_TEXTS_DYN_XY_NUMBER)
					{
						#if HMI_UNEDIT_TEXT_MAX_SON_CNT > 0
						hmi_object_dirty_flag = hmi_engine_check_dynamic_object_changed(
							&hmi_static_text_container_table[hmi_object_index],
							(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
							pdirty_rect,
							depth,&hmi_insect_rect);							
						#else
						hmi_object_dirty_flag = FALSE;
						#endif
					}
				}
			}
			else
			#endif
			#if HMI_UNEDIT_TEXTS_STATIC_XY_NUMBER > 0
			if(HMI_IS_UNEDIT_TEXT_STATIC_XY(hmi_object_id))
			{
				hmi_object_index = HMI_GET_S_XY_UNEDIT_POS_COLOR_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_UNEDIT_TEXTS_STATIC_XY_NUMBER)
				{
					HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
									(&hmi_static_xy_unedit_text_prop_table[hmi_object_index].text_rect),
									(&hmi_node_screen_rect));
					hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
						(HMI_RECT_STR CONST *)pcliped_farther_rect,
						&hmi_insect_rect);
				}
				if(HMI_IS_UNEDIT_TEXT_STATIC_XY_DYN_FONT(hmi_object_id))
				{
					if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
					{
						hmi_set_dirty_zone_layer(&hmi_insect_rect/*dfont image dirty zone is self zone*/,
									pdirty_rect,depth);	
						/*hmi_page_redrawed_flag	= TRUE;*/
						hmi_object_dirty_flag	= TRUE;
					}
					else
					{
						hmi_object_index = HMI_GET_UNEDIT_INDEX(hmi_object_id);/*all uneditable text contain at one array*/
						if(hmi_object_index < HMI_STATIC_TEXTS_NUMBER)
						{
							#if HMI_UNEDIT_TEXT_MAX_SON_CNT > 0
							hmi_object_dirty_flag=hmi_engine_check_dynamic_object_changed(
								&hmi_static_text_container_table[hmi_object_index],
								(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
								pdirty_rect,
								depth,&hmi_insect_rect);								
							#else
							hmi_object_dirty_flag = FALSE;
							#endif
						}
					}
				}
				else
				{
					hmi_object_index = HMI_GET_UNEDIT_INDEX(hmi_object_id);/*all uneditable text contain at one array*/
					if(hmi_object_index < HMI_STATIC_TEXTS_NUMBER)
					{
						#if HMI_UNEDIT_TEXT_MAX_SON_CNT > 0
						hmi_object_dirty_flag=hmi_engine_check_dynamic_object_changed(
							&hmi_static_text_container_table[hmi_object_index],
							(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
							pdirty_rect,
							depth,&hmi_insect_rect);							
						#else
						hmi_object_dirty_flag = FALSE;
						#endif
					}
				}
			}
			else
			#endif
			{
			}						
		}
	    else	
		#endif
	   #if HMI_STATIC_FILL_PAGES_NUMBER > 0
		if(HMI_IS_SXY_FILL_PAGES(hmi_object_id))
		{
			#if HMI_SXY_FILL_MAX_SON_CNT > 0
			hmi_object_index  = HMI_GET_SXY_FILL_PAGES_ID_INDEX(hmi_object_id );
			if(hmi_object_index < HMI_STATIC_FILL_PAGES_NUMBER)
			{
				hmi_rect_temp.x	=	hmi_fills_static_xy_rect[hmi_object_index].x;
				hmi_rect_temp.y	=	hmi_fills_static_xy_rect[hmi_object_index].y;
				hmi_rect_temp.w	=	hmi_fills_static_xy_rect[hmi_object_index].w;
				hmi_rect_temp.h	=	hmi_fills_static_xy_rect[hmi_object_index].h;
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
					(&hmi_rect_temp),
					(&hmi_node_screen_rect));
			}
			hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
				(HMI_RECT_STR CONST *)pcliped_farther_rect,&hmi_insect_rect);
			if(hmi_object_index < HMI_STATIC_FILL_PAGES_NUMBER)
			{
				hmi_object_dirty_flag = hmi_engine_check_dynamic_object_changed(
					&hmi_static_fill_container_table[hmi_object_index],
					(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
					pdirty_rect,
					depth,&hmi_insect_rect);					
			}
			#else
			hmi_object_dirty_flag = 0;
			#endif
		}
		else
	   #endif
	   	#if HMI_STATIC_GFILL_NUMBER> 0
		if(HMI_IS_SXY_GFILL_PAGES(hmi_object_id))
		{
			#if HMI_GFILL_MAX_SON_CNT > 0
			hmi_object_index = HMI_GET_SXY_GFILL_PAGES_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_STATIC_GFILL_NUMBER)
			{
				hmi_rect_temp.x	=	hmi_gradient_sxy_fill_rect[hmi_object_index].x;
				hmi_rect_temp.y	=	hmi_gradient_sxy_fill_rect[hmi_object_index].y;
				hmi_rect_temp.w	=	hmi_gradient_sxy_fill_rect[hmi_object_index].w;
				hmi_rect_temp.h	=	hmi_gradient_sxy_fill_rect[hmi_object_index].h;
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
							(&hmi_rect_temp),
							(&hmi_node_screen_rect));
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
					(HMI_RECT_STR CONST *)pcliped_farther_rect,
					&hmi_insect_rect);
				hmi_object_dirty_flag = hmi_engine_check_dynamic_object_changed(
					&hmi_static_gfill_container_table[hmi_object_index],
					(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
					pdirty_rect,
					depth,&hmi_insect_rect);					
			}
			#else
			hmi_object_dirty_flag = 0;
			#endif
		}
		else
	  	#endif
		#if HMI_SXY_CUBE_NUMBER> 0
		if(HMI_IS_SXY_CUBE(hmi_object_id))
		{
			hmi_object_index = HMI_GET_SXY_CUBE_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_SXY_CUBE_NUMBER)
			{
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
							(&hmi_cubes_static_xy_rect[hmi_object_index].cube_rect),
							(&hmi_node_screen_rect));
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
					(HMI_RECT_STR CONST *)pcliped_farther_rect,
					&hmi_insect_rect);
				hmi_object_dirty_flag = hmi_engine_check_dynamic_object_changed(
					&hmi_cubes_static_xy_face[hmi_object_index],
					(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
					pdirty_rect,
					depth,&hmi_insect_rect);					
				if(hmi_object_dirty_flag == TRUE)/*if cube  face texture changed ,cube should be change*/
				{
					hmi_set_dirty_zone_layer(pcliped_farther_rect,
									pdirty_rect,depth);
				}
			}
		}
		else
		#endif
		#if HMI_SXY_3DCUBE_NUMBER> 0
		if(HMI_IS_SXY_3DCUBE(hmi_object_id))
		{
			hmi_object_index = HMI_GET_SXY_3DCUBE_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_SXY_3DCUBE_NUMBER)
			{
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
							(&hmi_3dcubes_static_xy_rect[hmi_object_index].cube_rect),
							(&hmi_node_screen_rect));
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
					(HMI_RECT_STR CONST *)pcliped_farther_rect,
					&hmi_insect_rect);
				#if 0
				hmi_object_dirty_flag = hmi_engine_check_dynamic_object_changed(
					&hmi_cubes_static_xy_face[hmi_object_index],
					(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
					pdirty_rect,
					depth,&hmi_insect_rect);					
				if(hmi_object_dirty_flag == TRUE)/*if cube  face texture changed ,cube should be change*/
				{
					hmi_set_dirty_zone_layer(pcliped_farther_rect,
									pdirty_rect,depth);
				}
				#endif
			}
		}
		else
#endif
	   	#if HMI_SXY_BITMAPS_NUMBER> 0
		if(HMI_IS_S_XY_BITMAP(hmi_object_id))
		{
			#if HMI_SXY_IMAGE_MAX_SON_CNT > 0
			hmi_object_index  = HMI_GET_SXY_BITMAPS_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_SXY_BITMAPS_NUMBER)
			{
				hmi_rect_temp.x	=	hmi_bmp_static_xy_rect[hmi_object_index].x;
				hmi_rect_temp.y	=	hmi_bmp_static_xy_rect[hmi_object_index].y;
				hmi_rect_temp.w	=	hmi_bmp_static_xy_rect[hmi_object_index].w;
				hmi_rect_temp.h	=	hmi_bmp_static_xy_rect[hmi_object_index].h;
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
					(&hmi_rect_temp),
					(&hmi_node_screen_rect));
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
					(HMI_RECT_STR CONST *)pcliped_farther_rect,
					&hmi_insect_rect);
				hmi_object_dirty_flag = hmi_engine_check_dynamic_object_changed(
							&hmi_sxy_bitmap_container_table[hmi_object_index],
							(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
							pdirty_rect,
							depth,&hmi_insect_rect);							
			}
			#else
			hmi_object_dirty_flag = 0;
			#endif
		}
		else
	   #endif
		#if HMI_SXY_CUSTOM_CNT > 0
		if(HMI_IS_CUSTOM_SXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_CUSTOM_SXY_ID_INDEX(hmi_object_id);
			get_success = hmi_engine_get_custom_prop(hmi_object_id,&custom_object_prop);
			if(get_success == TRUE)
			{
				hmi_custom_rect.x 	= custom_object_prop.x;
				hmi_custom_rect.y 	= custom_object_prop.y;
				hmi_custom_rect.w 	= custom_object_prop.w;
				hmi_custom_rect.h 	= custom_object_prop.h;
				if(hmi_object_index < HMI_SXY_CUSTOM_CNT)
				{				
					HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
						(&hmi_custom_rect),
						(&hmi_node_screen_rect));
					
					hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
						(HMI_RECT_STR CONST *)pcliped_farther_rect,&hmi_insect_rect);
				}
				
				if((custom_object_prop.attr & HMI_ID1) !=0)
				{
					hmi_trail_object[0].object_id	= custom_object_prop.id1;
					hmi_trail_container.container_object_table.object_number	= 1;
					hmi_trail_container.container_object_table.p_object_table	= hmi_trail_object;
									
					hmi_object_dirty_flag = hmi_engine_check_dynamic_object_changed(
													&hmi_trail_container,
													(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
													pdirty_rect,
													depth,
													&hmi_insect_rect);
					if(hmi_object_dirty_flag == TRUE)
					{
						hmi_set_dirty_zone_layer(&hmi_insect_rect,
													pdirty_rect,depth);				
						hmi_object_dirty_flag	= TRUE;

					}
					
				}
				if(((custom_object_prop.attr & HMI_ID2) !=0)&&(hmi_object_dirty_flag == FALSE))
				{
					hmi_trail_object[0].object_id	= custom_object_prop.id2;
					hmi_trail_container.container_object_table.object_number	= 1;
					hmi_trail_container.container_object_table.p_object_table	= hmi_trail_object;
									
					hmi_object_dirty_flag = hmi_engine_check_dynamic_object_changed(
													&hmi_trail_container,
													(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
													pdirty_rect,
													depth,
													&hmi_insect_rect);
					if(hmi_object_dirty_flag == TRUE)
					{
						hmi_set_dirty_zone_layer(&hmi_insect_rect,
													pdirty_rect,depth);				
						hmi_object_dirty_flag	= TRUE;

					}
					
				}
				if(((custom_object_prop.attr & HMI_ID3) !=0)&&(hmi_object_dirty_flag == FALSE))
				{
					hmi_trail_object[0].object_id	= custom_object_prop.id3;
					hmi_trail_container.container_object_table.object_number	= 1;
					hmi_trail_container.container_object_table.p_object_table	= hmi_trail_object;
									
					hmi_object_dirty_flag = hmi_engine_check_dynamic_object_changed(
													&hmi_trail_container,
													(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
													pdirty_rect,
													depth,
													&hmi_insect_rect);
					if(hmi_object_dirty_flag == TRUE)
					{
						hmi_set_dirty_zone_layer(&hmi_insect_rect,
													pdirty_rect,depth);				
						hmi_object_dirty_flag	= TRUE;

					}
					
				}
				if(hmi_object_dirty_flag ==FALSE)
				{
					custom_type = is3dcameralightcustom(hmi_object_id);
					if((custom_type == 0/*3d mode */)||(custom_type == 1/*3d mode */)||(custom_type == 6/*3d mode */))
					{
						hmi_object_dirty_flag |= hmi_engine_check_dynamic_object_changed(
														(HMI_CONTAINER_STR CONST * )(&(((HMI_3D_SUB_CUSTOM_USR_DATA_STR *)(hmi_sxy_custom_lib_data[hmi_object_index].pusr_data))->child)),
														(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
														pdirty_rect,
														depth,&hmi_insect_rect);
					}
				}
				
			}
		}
		else
		#endif		
	   #if HMI_SXY_CONTAINERS_NUMBER > 0
		if(HMI_IS_CONTAINERS_SXY(hmi_object_id))
		{
			if(hmi_engine_check_container_changed(hmi_object_id,
				(HMI_RECT_STR CONST *)pfarther_rect,
				pdirty_rect,depth,pcliped_farther_rect) != FALSE)
			{
				hmi_object_dirty_flag = TRUE;
			}
		}
		else
	   #endif	   
		{
		}
		hmi_number_object--;	
		hmi_check_index++;
	}
		
   return	hmi_object_dirty_flag;
}

#endif
#endif

#if HMI_ALL_STATIC_CONTAINERS_NUMBER> 0
#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||\
	defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||\
	defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||\
	defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))
#if (HMI_DYN_CONTAINERS_NUMBER > 0)
static void	hmi_get_dyn_dirty(HMI_OBJECT_ID_STR hmi_object_id,HMI_RECT_STR CONST *pfarther_rect,
					HMI_RECT_STR *pdirty_rect,UINT8 depth,HMI_RECT_STR * pcliped_farther_rect,BOOLEAN old_tree)
{		
	HMI_RECT_STR hmi_insect_rect			= {HMI_INVALID_COOR,HMI_INVALID_COOR,0,0};
	HMI_RECT_STR hmi_node_screen_rect	= {HMI_INVALID_COOR,HMI_INVALID_COOR,0,0};
	HMI_RECT_STR CONST *phmi_container_rect = NULL;	
	
	//hmi_object_id = HMI_GET_DYN_CONTAINERS_ID_INDEX(hmi_object_id);	
	if(hmi_object_id == HMI_DYN_CONTAINER_IS_NULL)
	{

	}
	#if HMI_SXY_CONTAINERS_NUMBER >0
	else if(HMI_IS_CONTAINERS_SXY(hmi_object_id))
	{
		hmi_object_id = HMI_GET_CONTAINERS_SXY_ID_INDEX(hmi_object_id);
		if(hmi_object_id < HMI_SXY_CONTAINERS_NUMBER)
		{
			phmi_container_rect=&hmi_static_container_rect[hmi_object_id];
			hmi_get_object_screen_coor((HMI_RECT_STR CONST *)pfarther_rect,
				phmi_container_rect,&hmi_node_screen_rect);
			hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
						(HMI_RECT_STR CONST *)pcliped_farther_rect ,
						&hmi_insect_rect );
			hmi_set_dirty_zone_layer((HMI_RECT_STR *)(&hmi_insect_rect),
					pdirty_rect,depth);
		}	
	}
	#endif
	#if HMI_DXY_CONTAINERS_NUMBER >0
	else if(HMI_IS_DYN_XY_CONTAINER(hmi_object_id))
	{		
		hmi_object_id = HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_object_id);
		if(hmi_object_id < HMI_DXY_CONTAINERS_NUMBER)
		{
			if(old_tree == TRUE)
			{
				phmi_container_rect = &hmi_dyn_xy_container_rect_bck[hmi_object_id];
			}
			else
			{
				phmi_container_rect = &hmi_dyn_xy_container_rect[hmi_object_id];
			}
			HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,phmi_container_rect,
										(&hmi_node_screen_rect));
			hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
							(HMI_RECT_STR CONST *)pcliped_farther_rect,
							&hmi_insect_rect);
			hmi_set_dirty_zone_layer(&hmi_insect_rect,
									pdirty_rect,depth);
		}		
	}
	#endif
	else
	{
	}				
}
#endif
static BOOLEAN hmi_engine_check_container_changed(HMI_OBJECT_ID_STR hmi_object_id,HMI_RECT_STR CONST *pfarther_rect,
					HMI_RECT_STR *pdirty_rect,UINT8 depth,HMI_RECT_STR * pcliped_farther_rect)
{	
	BOOLEAN 	hmi_container_changed_flag	= FALSE; 
	HMI_RECT_STR CONST *phmi_container_rect = NULL; 	
	HMI_RECT_STR		hmi_node_screen_rect= {HMI_INVALID_COOR,HMI_INVALID_COOR,0,0};
	HMI_RECT_STR			hmi_insect_rect = {HMI_INVALID_COOR,HMI_INVALID_COOR,0,0};
	#if (HMI_DYN_CONTAINERS_NUMBER > 0)
	HMI_OBJECT_ID_STR hmi_object_id_bck 	= 0;
	#endif
	
	#if HMI_DXY_CONTAINERS_NUMBER >0
	if(HMI_IS_DYN_XY_CONTAINER(hmi_object_id))
	{
		if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
		{				
			hmi_object_id = HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_CONTAINERS_NUMBER)
			{
				phmi_container_rect = &hmi_dyn_xy_container_rect[hmi_object_id];
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,phmi_container_rect,
											(&hmi_node_screen_rect));
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
								(HMI_RECT_STR CONST *)pcliped_farther_rect,
								&hmi_insect_rect);
				hmi_set_dirty_zone_layer(&hmi_insect_rect,
										pdirty_rect,depth);
				/*add old dirty*/
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
					(&hmi_dyn_xy_container_rect_bck[hmi_object_id]),
					(&hmi_node_screen_rect));
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
							(HMI_RECT_STR CONST *)pcliped_farther_rect,
							&hmi_insect_rect);
				hmi_set_dirty_zone_layer(&hmi_insect_rect,
									pdirty_rect,depth);
			}
			
			hmi_container_changed_flag = TRUE;
		}
		else
		{
			hmi_object_id = HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_object_id);
			if(hmi_object_id < HMI_DXY_CONTAINERS_NUMBER)
			{				
				phmi_container_rect = &hmi_dyn_xy_container_rect[hmi_object_id];
				
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,phmi_container_rect,
											(&hmi_node_screen_rect));
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
									(HMI_RECT_STR CONST *)pcliped_farther_rect,
									&hmi_insect_rect);
				hmi_container_changed_flag |= hmi_engine_check_dynamic_object_changed(
							&hmi_dyn_xy_container_table[hmi_object_id],
							(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
							pdirty_rect,
							depth,&hmi_insect_rect);
				if(hmi_dxy_container_video_fmt[hmi_object_id].video_fmt_channel & HMI_SCENE_WINDOW)
				{
					if(hmi_container_changed_flag == TRUE)
					{
						hmi_set_dirty_zone_layer(&hmi_insect_rect,
									pdirty_rect,depth);
					}
				}
			}
		}
	}
	else 
	#endif
	#if HMI_SXY_CONTAINERS_NUMBER >0
	if(HMI_IS_CONTAINERS_SXY(hmi_object_id))
	{
		hmi_object_id = HMI_GET_CONTAINERS_SXY_ID_INDEX(hmi_object_id);
		if(hmi_object_id < HMI_SXY_CONTAINERS_NUMBER)
		{
			phmi_container_rect=&hmi_static_container_rect[hmi_object_id];
			hmi_get_object_screen_coor((HMI_RECT_STR CONST *)pfarther_rect,
				phmi_container_rect,&hmi_node_screen_rect);
			hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
						(HMI_RECT_STR CONST *)pcliped_farther_rect ,
						&hmi_insect_rect );
			hmi_container_changed_flag |= hmi_engine_check_dynamic_object_changed(
						&hmi_sxy_container_table[hmi_object_id],
						(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
						pdirty_rect,
						depth,&hmi_insect_rect);
		}				
	}
	else
	#endif
	#if (HMI_DYN_CONTAINERS_NUMBER > 0)
	if(HMI_IS_DYN_CONTAINER(hmi_object_id))
	{					
		if((HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0))
		{	
			/*search old and new tree for dirty zone*/
			hmi_object_id  = HMI_GET_DYN_CONTAINERS_ID_INDEX(hmi_object_id);
			hmi_object_id_bck	= hmi_dyn_container_table_bck[hmi_object_id];
			if(HMI_IS_DYN_CONTAINER(hmi_object_id_bck))
			{
				hmi_object_id_bck	= HMI_GET_DYN_CONTAINERS_ID_INDEX(hmi_object_id_bck);
				hmi_engine_get_bck_static_container_id(&hmi_object_id_bck);
			}
			hmi_engine_get_static_container_id(&hmi_object_id);
			
			if(hmi_object_id != hmi_object_id_bck)
			{
				/*set old tree dirty*/
				hmi_get_dyn_dirty(hmi_object_id_bck,pfarther_rect,
					pdirty_rect,depth,pcliped_farther_rect,TRUE);
				/*set new tree dirty*/
				hmi_get_dyn_dirty(hmi_object_id,pfarther_rect,
					pdirty_rect,depth,pcliped_farther_rect,FALSE);
				hmi_container_changed_flag	= TRUE; 	
			}
			else
			{
				hmi_container_changed_flag |= hmi_engine_check_container_changed(hmi_object_id,
																pfarther_rect,pdirty_rect,depth,
																pcliped_farther_rect);	

			}
		}
		else
		{	
			hmi_object_id  = HMI_GET_DYN_CONTAINERS_ID_INDEX(hmi_object_id);
			//hmi_engine_get_static_container_id(&hmi_object_id);//changed by pxguo  dyn container ->dyn container
			hmi_object_id = hmi_dyn_container_table[hmi_object_id];
			hmi_container_changed_flag |= hmi_engine_check_container_changed(hmi_object_id,
												pfarther_rect,pdirty_rect,depth,
												pcliped_farther_rect);			
		}		
	}
	else
	#endif
	{
	}       
   return(hmi_container_changed_flag);
}

#endif
#endif  


#if defined(HMI_GRAPHIC_TWLIB)
BOOLEAN hmi_get_is_merge(HMI_ELEMENT_TYPE parent_type,HMI_ELEMENT_TYPE child_type)
{
	BOOLEAN bmerge=FALSE;

	switch(parent_type)
	{
		case HMI_ELEM_TYPE_SFILL :
			if(child_type==HMI_ELEM_TYPE_SFILL )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DFILL )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SIMAGE )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DIMAGE )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SIMGLIST )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DIMGLIST )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SBTN )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DBTN )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SPAGE )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DPAGE )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_SSCROLLBAR )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DSCROLLBAR )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DYN_CONTAINER  )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_DXY_CONTINER  )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SXY_CONTAINER  )
			{
				bmerge=TRUE;
			}
			else
			{
				bmerge=TRUE;
			}
			break;
		case HMI_ELEM_TYPE_DFILL :
			bmerge=FALSE;
			break;
		case HMI_ELEM_TYPE_SIMAGE :
			if(child_type==HMI_ELEM_TYPE_SFILL )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DFILL )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SIMAGE )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DIMAGE )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SIMGLIST )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DIMGLIST )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SBTN )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DBTN )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SPAGE )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DPAGE )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_SSCROLLBAR )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DSCROLLBAR )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DYN_CONTAINER  )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_DXY_CONTINER  )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SXY_CONTAINER  )
			{
				bmerge=TRUE;
			}
			else
			{
				bmerge=TRUE;
			}
			break;
		case HMI_ELEM_TYPE_DIMAGE :
			if(child_type==HMI_ELEM_TYPE_SFILL )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DFILL )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SIMAGE )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DIMAGE )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SIMGLIST )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DIMGLIST )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SBTN )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DBTN )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SPAGE )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DPAGE )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_SSCROLLBAR )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DSCROLLBAR )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DYN_CONTAINER  )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_DXY_CONTINER  )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SXY_CONTAINER  )
			{
				bmerge=TRUE;
			}
			else
			{
				bmerge=TRUE;
			}
			break;
		case HMI_ELEM_TYPE_SIMGLIST :
			if(child_type==HMI_ELEM_TYPE_SFILL )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DFILL )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SIMAGE )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DIMAGE )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SIMGLIST )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DIMGLIST )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SBTN )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DBTN )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SPAGE )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DPAGE )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_SSCROLLBAR )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DSCROLLBAR )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DYN_CONTAINER  )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_DXY_CONTINER  )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SXY_CONTAINER  )
			{
				bmerge=TRUE;
			}
			else
			{
				bmerge=TRUE;
			}
			break;
		case HMI_ELEM_TYPE_DIMGLIST :
			if(child_type==HMI_ELEM_TYPE_SFILL )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DFILL )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SIMAGE )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DIMAGE )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SIMGLIST )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DIMGLIST )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SBTN )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DBTN )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SPAGE )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DPAGE )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_SSCROLLBAR )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DSCROLLBAR )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DYN_CONTAINER  )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_DXY_CONTINER  )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SXY_CONTAINER  )
			{
				bmerge=TRUE;
			}
			else
			{
				bmerge=TRUE;
			}
			break;
		case HMI_ELEM_TYPE_SBTN :
			if(child_type==HMI_ELEM_TYPE_SFILL )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DFILL )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SIMAGE )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DIMAGE )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SIMGLIST )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DIMGLIST )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SBTN )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DBTN )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SPAGE )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DPAGE )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_SSCROLLBAR )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DSCROLLBAR )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DYN_CONTAINER  )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_DXY_CONTINER  )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SXY_CONTAINER  )
			{
				bmerge=TRUE;
			}
			else
			{
				bmerge=TRUE;
			}
			break;
		case HMI_ELEM_TYPE_DBTN :
			if(child_type==HMI_ELEM_TYPE_SFILL )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DFILL )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SIMAGE )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DIMAGE )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SIMGLIST )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DIMGLIST )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SBTN )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DBTN )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SPAGE )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DPAGE )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_SSCROLLBAR )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DSCROLLBAR )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DYN_CONTAINER  )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_DXY_CONTINER  )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SXY_CONTAINER  )
			{
				bmerge=TRUE;
			}
			else
			{
				bmerge=TRUE;
			}
			break;
		case HMI_ELEM_TYPE_SPAGE :
			if(child_type==HMI_ELEM_TYPE_SFILL )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DFILL )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SIMAGE )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DIMAGE )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SIMGLIST )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DIMGLIST )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SBTN )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DBTN )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SPAGE )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DPAGE )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_SSCROLLBAR )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DSCROLLBAR )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DYN_CONTAINER  )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_DXY_CONTINER  )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SXY_CONTAINER  )
			{
				bmerge=TRUE;
			}
			else
			{
				bmerge=TRUE;
			}
			break;
		case HMI_ELEM_TYPE_DPAGE :
			if(child_type==HMI_ELEM_TYPE_SFILL )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DFILL )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SIMAGE )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DIMAGE )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SIMGLIST )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DIMGLIST )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SBTN )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DBTN )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SPAGE )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DPAGE )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_SSCROLLBAR )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DSCROLLBAR )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DYN_CONTAINER  )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_DXY_CONTINER  )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SXY_CONTAINER  )
			{
				bmerge=TRUE;
			}
			else
			{
				bmerge=TRUE;
			}
			break;
		case HMI_ELEM_TYPE_SXY_CONTAINER:
			if(child_type==HMI_ELEM_TYPE_SFILL )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DFILL )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SIMAGE )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DIMAGE )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SIMGLIST )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DIMGLIST )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SBTN )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DBTN )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SPAGE )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_DPAGE )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SSCROLLBAR )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_DSCROLLBAR )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_DYN_CONTAINER  )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_DXY_CONTINER  )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SXY_CONTAINER  )
			{
				bmerge=TRUE;
			}
			else
			{
				bmerge=TRUE;
			}
			break;
		case HMI_ELEM_TYPE_DXY_CONTINER:
			if(child_type==HMI_ELEM_TYPE_SFILL )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DFILL )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SIMAGE )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DIMAGE )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SIMGLIST )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DIMGLIST )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SBTN )
			{
				bmerge=TRUE;
			}
			else if(child_type==HMI_ELEM_TYPE_DBTN )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SPAGE )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_DPAGE )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SSCROLLBAR )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_DSCROLLBAR )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_DYN_CONTAINER  )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_DXY_CONTINER  )
			{
				bmerge=FALSE;
			}
			else if(child_type==HMI_ELEM_TYPE_SXY_CONTAINER  )
			{
				bmerge=TRUE;
			}
			else
			{
				bmerge=TRUE;
			}
			break;			
		case HMI_ELEM_TYPE_SSCROLLBAR :
			bmerge=TRUE;/*tw not support scrollbar*/
			break;
		case HMI_ELEM_TYPE_DSCROLLBAR :
			bmerge=TRUE;/*tw not support scrollbar*/
			break;
		default:
			;
	}
	return bmerge;
}
#endif

#if 0
#if HMI_SXY_BITMAPS_NUMBER > 0 need cut off
static void hmi_engine_draw_static_xy_image(HMI_OBJECT_PROP_STR CONST * phmi_image_prop_info,HMI_RECT_STR CONST *pfarther_rect,
												#ifdef HMI_GRAPHIC_AGG||HMI_GRAPHIC_RGL||HMI_GRAPHIC_ST
												HMI_RECT_STR * pvalue_rect,
												HMI_RECT_STR *pdirty_rect
												#else
												HMI_ELEMENT_TYPE	parent_type
												#endif
												)
{
    HMI_OBJECT_ID_STR	hmi_object_id = HMI_GET_SXY_BITMAPS_ID_INDEX(phmi_image_prop_info->object_id);
	HMI_RECT_STR		hmi_temp_rect={HMI_INVALID_COOR,HMI_INVALID_COOR,0,0};
	#ifdef HMI_GRAPHIC_AGG||HMI_GRAPHIC_RGL||HMI_GRAPHIC_ST
	HMI_RECT_STR		hmi_union_rect={HMI_INVALID_COOR,HMI_INVALID_COOR,0,0};
	HMI_RECT_STR		hmi_draw_rect={HMI_INVALID_COOR,HMI_INVALID_COOR,0,0};
	#else /*tw*/
	BOOLEAN				bDrawBck=FALSE;
	#endif
    if(hmi_object_id < HMI_SXY_BITMAPS_NUMBER)
    {
       	HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,(&hmi_bmp_static_xy_rect[hmi_object_id]),(&hmi_temp_rect));
		#ifdef HMI_GRAPHIC_AGG||HMI_GRAPHIC_RGL||HMI_GRAPHIC_ST
		hmi_get_intersec_rect((HMI_RECT_STR CONST *)&hmi_temp_rect,(HMI_RECT_STR CONST *)pvalue_rect,&hmi_union_rect);
		hmi_get_intersec_rect((HMI_RECT_STR CONST *)&hmi_union_rect,(HMI_RECT_STR CONST *)pdirty_rect,&hmi_draw_rect);
		#endif
		#ifdef HMI_GRAPHIC_AGG
       	call_C_hmi_driver_draw_image(&hmi_temp_rect, hmi_bigbitmap_table[0],hmi_bigbitmap_table[1],&hmi_bmp_static_xy_prop_table[hmi_object_id],&hmi_draw_rect);
		#endif
		#ifdef HMI_GRAPHIC_RGL
       	call_C_hmi_driver_draw_image(&hmi_temp_rect, hmi_bigbitmap_table[0],hmi_bigbitmap_table[1],&hmi_bmp_static_xy_prop_table[hmi_object_id],&hmi_draw_rect);
		#endif
		#ifdef HMI_GRAPHIC_TWLIB
       	call_C_hmi_driver_draw_image();
		#endif

		#ifdef HMI_GRAPHIC_AGG||HMI_GRAPHIC_RGL||HMI_GRAPHIC_ST
		hmi_engine_draw_container(&hmi_sxy_bitmap_container_table[hmi_object_id],
								(HMI_RECT_STR CONST *)&hmi_temp_rect,
								&hmi_union_rect,pdirty_rect);
		#else /*tw*/
		bDrawBck=hmi_get_is_merge(parent_type,HMI_SIMAGE);
		hmi_engine_draw_container(&hmi_sxy_bitmap_container_table[hmi_object_id],
								(HMI_RECT_STR CONST *)&hmi_temp_rect,
								HMI_DIMAGE ,bDrawBck);
		#endif

	}
}
#endif
#endif

#if 0
#if HMI_DXY_BITMAPS_NUMBER > 0 
static void hmi_engine_draw_dyn_xy_image(HMI_OBJECT_PROP_STR CONST * phmi_image_prop_info,HMI_RECT_STR CONST *pfarther_rect,HMI_RECT_STR * pvalue_rect,HMI_RECT_STR *pdirty_rect)
{
    HMI_OBJECT_ID_STR	hmi_object_id = HMI_GET_DYN_XY_BITMAP_INDEX(phmi_image_prop_info->object_id);
	HMI_RECT_STR		hmi_screen_target={HMI_INVALID_COOR,HMI_INVALID_COOR,0,0};
	HMI_RECT_STR		hmi_union_rect={HMI_INVALID_COOR,HMI_INVALID_COOR,0,0};
	HMI_RECT_STR		hmi_draw_rect={HMI_INVALID_COOR,HMI_INVALID_COOR,0,0};

    if(hmi_object_id < HMI_DXY_BITMAPS_NUMBER)
    {
		HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,(&hmi_bmp_dyn_xy_rect[hmi_object_id]),(&hmi_screen_target));
		#ifdef HMI_GRAPHIC_AGG||HMI_GRAPHIC_RGL||HMI_GRAPHIC_ST
		hmi_get_intersec_rect((HMI_RECT_STR CONST *)&hmi_screen_target,(HMI_RECT_STR CONST *)pvalue_rect,&hmi_union_rect);
		hmi_get_intersec_rect((HMI_RECT_STR CONST *)&hmi_union_rect,(HMI_RECT_STR CONST *)pdirty_rect,&hmi_draw_rect);
		#endif
		
		#ifdef HMI_GRAPHIC_AGG
       	call_C_hmi_driver_draw_image(&hmi_screen_target, hmi_bigbitmap_table[0],hmi_bigbitmap_table[1], &hmi_bmp_dyn_xy_prop_table[hmi_object_id],&hmi_draw_rect);
		#endif
		#ifdef HMI_GRAPHIC_RGL
       	call_C_hmi_driver_draw_image(&hmi_screen_target, &hmi_bmp_dyn_xy_rect[hmi_object_id], &hmi_bmp_dyn_xy_prop_table[hmi_object_id],&hmi_draw_rect);
		#endif
		#ifdef HMI_GRAPHIC_TWLIB
		call_C_hmi_driver_draw_image(&hmi_screen_target, &hmi_dxy_bitmap_attr_table[hmi_object_id], &hmi_bmp_dyn_xy_prop_table[hmi_object_id],&hmi_draw_rect);
		#endif
		hmi_engine_draw_container(&hmi_dxy_bitmap_container_table[hmi_object_id],(HMI_RECT_STR CONST *)&hmi_screen_target,&hmi_union_rect,pdirty_rect);
	}
}
#endif
#endif


#if 0
void hmi_engine_draw_container_bck(HMI_OBJECT_ID_STR elemID)
{
}
#endif




#if (defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC))
//#if((HMI_DXY_CONTAINERS_NUMBER+HMI_SXY_CONTAINERS_NUMBER) > 0)	//2018 10 
static void hmi_cube_get_id_prop(HMI_OBJECT_ID_STR		hmi_object_id,
								HMI_CUBE_TEXTURE_PROP	*pcube_texture,
								SPOINT_TP				*pfather_point)			
{
	HMI_OBJECT_ID_STR			hmi_object_index	= 0;
	if(pcube_texture != NULL)
	{
		pcube_texture->tex_rect.x		= 0;
		pcube_texture->tex_rect.y		= 0;
		pcube_texture->tex_rect.w		= 0U;
		pcube_texture->tex_rect.h		= 0U;
		pcube_texture->tex_rect.angel	= 0;
		pcube_texture->tex_rect.alpha	= 0;
#if		defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)	||defined(HMI_GRAPHIC_OPENVG)	
		pcube_texture->tex_prop.w		= 0U;
		pcube_texture->tex_prop.h		= 0U;
		pcube_texture->tex_prop.data_len= 0U;
		pcube_texture->tex_prop.pbitmap_data	= NULL;
#elif	(defined(HMI_GRAPHIC_OPENGLES))		
		pcube_texture->tex_prop.pbitmap_data.seg_no		= 0U;
		pcube_texture->tex_prop.pbitmap_data.index		= 0U;
		pcube_texture->tex_prop.pbitmap_data.fmt_index	= HMI_MAX_CNT_BIG_IMAGE;
		pcube_texture->tex_prop.pbitmap_data.x			= 0U;
		pcube_texture->tex_prop.pbitmap_data.y			= 0U;
		pcube_texture->tex_prop.pbitmap_data.w			= 0U;
		pcube_texture->tex_prop.pbitmap_data.h			= 0U;		
#endif
		pcube_texture->tex_attr.image_attr	= 0;
		#if defined( HMI_MCU_S6J3200)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)
		pcube_texture->tex_attr.pixel_fmt	= 0;
		#endif

		pcube_texture->tex_id=HMI_ALL_OBJECT;

		if(hmi_object_id  >= HMI_PAGE_SXY_MAX_ID)/*not a page*/
		{	 	
			#if HMI_DXY_IMAGELIST_NUMBER>0
			if(HMI_IS_DYN_IMAGELIST_DXY(hmi_object_id))
		    {
				hmi_object_index = HMI_GET_DYN_IMAGELIST_DXY_ID_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_DXY_IMAGELIST_NUMBER)
				{
					if(pfather_point !=NULL)
					{
						pfather_point->x		+= hmi_dxy_imagelist_rect[hmi_object_index].x;
						pfather_point->y		+= hmi_dxy_imagelist_rect[hmi_object_index].y;
					}
					pcube_texture->tex_rect.x	= hmi_dxy_imagelist_rect[hmi_object_index].x;
					pcube_texture->tex_rect.y	= hmi_dxy_imagelist_rect[hmi_object_index].y;
					pcube_texture->tex_rect.w	= hmi_dxy_imagelist_rect[hmi_object_index].w;
					pcube_texture->tex_rect.h	= hmi_dxy_imagelist_rect[hmi_object_index].h;
					pcube_texture->tex_rect.angel= 0;
					pcube_texture->tex_rect.alpha= hmi_dxy_imagelist_rect[hmi_object_index].alpha;
				#if		defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)	||defined(HMI_GRAPHIC_OPENVG)					
					pcube_texture->tex_prop.w		= hmi_dxy_imagelist_table[hmi_object_index].file.w;
					pcube_texture->tex_prop.h		= hmi_dxy_imagelist_table[hmi_object_index].file.h;
					pcube_texture->tex_prop.data_len= hmi_dxy_imagelist_table[hmi_object_index].file.pimagelist_attr->data_len;
					pcube_texture->tex_prop.pbitmap_data=hmi_dxy_imagelist_table[hmi_object_index].file.pimagelist_attr->pbitmap_data;
				#elif	(defined(HMI_GRAPHIC_OPENGLES))
					#if 0
					pcube_texture->tex_prop.pbitmap_data.seg_no		= hmi_dxy_imagelist_table[hmi_object_index].file.pimagelist_attr;
					pcube_texture->tex_prop.pbitmap_data.index		= hmi_dxy_imagelist_table[hmi_object_index];
					pcube_texture->tex_prop.pbitmap_data.fmt_index	= hmi_dxy_imagelist_table[hmi_object_index];
					pcube_texture->tex_prop.pbitmap_data.x			= hmi_dxy_imagelist_table[hmi_object_index];
					pcube_texture->tex_prop.pbitmap_data.y			= hmi_dxy_imagelist_table[hmi_object_index];
					pcube_texture->tex_prop.pbitmap_data.w			= hmi_dxy_imagelist_table[hmi_object_index];
					pcube_texture->tex_prop.pbitmap_data.h			= hmi_dxy_imagelist_table[hmi_object_index];
					#endif
				#endif	
					pcube_texture->tex_attr.image_attr	= hmi_dxy_imglist_attr_table[hmi_object_index].image_attr;
					#if defined( HMI_MCU_S6J3200 )||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)
					pcube_texture->tex_attr.pixel_fmt	= hmi_dxy_imglist_attr_table[hmi_object_index].pixel_fmt;
					#endif
					pcube_texture->tex_id=hmi_object_id;
				}
			}		
			else
			#endif
			#if HMI_SXY_IMAGELIST_NUMBER>0
			if(HMI_IS_DYN_IMAGELIST_SXY(hmi_object_id))
		    {
				hmi_object_index	= HMI_GET_DYN_IMAGELIST_SXY_ID_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_SXY_IMAGELIST_NUMBER)
				{	
					if(pfather_point != NULL)
					{
						pfather_point->x	+= hmi_sxy_imagelist_rect[hmi_object_index].x;
						pfather_point->y	+= hmi_sxy_imagelist_rect[hmi_object_index].y;
					}
					pcube_texture->tex_rect.x	= hmi_sxy_imagelist_rect[hmi_object_index].x;
					pcube_texture->tex_rect.y	= hmi_sxy_imagelist_rect[hmi_object_index].y;
					pcube_texture->tex_rect.w	= hmi_sxy_imagelist_rect[hmi_object_index].w;
					pcube_texture->tex_rect.h	= hmi_sxy_imagelist_rect[hmi_object_index].h;
					pcube_texture->tex_rect.angel= 0;
					pcube_texture->tex_rect.alpha= hmi_sxy_imagelist_rect[hmi_object_index].alpha;
				#if		defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)
					pcube_texture->tex_prop.w	= hmi_sxy_imagelist_table[hmi_object_index].file.w;
					pcube_texture->tex_prop.h	= hmi_sxy_imagelist_table[hmi_object_index].file.h;
					pcube_texture->tex_prop.data_len	= hmi_sxy_imagelist_table[hmi_object_index].file.pimagelist_attr->data_len;
					pcube_texture->tex_prop.pbitmap_data= hmi_sxy_imagelist_table[hmi_object_index].file.pimagelist_attr->pbitmap_data;
				#elif	(defined(HMI_GRAPHIC_OPENGLES))
					#if 0
					pcube_texture->tex_prop.pbitmap_data.seg_no		= 0U;
					pcube_texture->tex_prop.pbitmap_data.index		= 0U;
					pcube_texture->tex_prop.pbitmap_data.fmt_index	= HMI_MAX_CNT_BIG_IMAGE;
					pcube_texture->tex_prop.pbitmap_data.x			= 0U;
					pcube_texture->tex_prop.pbitmap_data.y			= 0U;
					pcube_texture->tex_prop.pbitmap_data.w			= 0U;
					pcube_texture->tex_prop.pbitmap_data.h			= 0U;//????
					#endif
				#endif		
					pcube_texture->tex_attr.image_attr	= hmi_sxy_imglist_attr_table[hmi_object_index].image_attr;
					#if defined( HMI_MCU_S6J3200 )||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)
					pcube_texture->tex_attr.pixel_fmt	= hmi_sxy_imglist_attr_table[hmi_object_index].pixel_fmt;
					#endif

					pcube_texture->tex_id				= hmi_object_id;
				}
			}
			else
			#endif
			#if HMI_DYN_CONTAINERS_NUMBER > 0
			if(HMI_IS_DYN_CONTAINER(hmi_object_id))
			{		  	
				#if HMI_ALL_STATIC_CONTAINERS_NUMBER > 0  
				hmi_object_id  = HMI_GET_DYN_CONTAINERS_ID_INDEX(hmi_object_id);
			  	hmi_engine_get_static_container_id(&hmi_object_id);/*get static container ID*/
							
				#if HMI_DXY_CONTAINERS_NUMBER>0U
				if(HMI_IS_DYN_XY_CONTAINER(hmi_object_id))
				{
					 hmi_object_index = HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_object_id );
					if(hmi_object_index < HMI_DXY_CONTAINERS_NUMBER)
					{
						if(pfather_point !=NULL)
						{
							pfather_point->x	+= hmi_dyn_xy_container_rect[hmi_object_index].x;
							pfather_point->y	+= hmi_dyn_xy_container_rect[hmi_object_index].y;
						}
						hmi_cube_get_id_prop(hmi_object_id,pcube_texture,pfather_point);
					}
				}
				else
				#endif
				{
				}
				#if HMI_SXY_CONTAINERS_NUMBER>0
				if(HMI_IS_CONTAINERS_SXY(hmi_object_id))
				{
					hmi_object_index = HMI_GET_CONTAINERS_SXY_ID_INDEX(hmi_object_id );
					if(hmi_object_index < HMI_SXY_CONTAINERS_NUMBER)
					{
						if(pfather_point !=NULL)
						{
							pfather_point->x	+= hmi_static_container_rect[hmi_object_index].x;
							pfather_point->y	+= hmi_static_container_rect[hmi_object_index].y;
						}
						hmi_cube_get_id_prop(hmi_object_id,pcube_texture,pfather_point);				
					}
				}
				else
				#endif
				{
				}	
				#endif
			}
			else
			#endif
			#if HMI_DXY_CONTAINERS_NUMBER > 0U  
			if(HMI_IS_DYN_XY_CONTAINER(hmi_object_id))
			{
				hmi_object_index = HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_object_id );
				if(hmi_object_index < HMI_DXY_CONTAINERS_NUMBER)
				{
					if(pfather_point !=NULL)
					{
						pfather_point->x	+= hmi_dyn_xy_container_rect[hmi_object_index].x;
						pfather_point->y	+= hmi_dyn_xy_container_rect[hmi_object_index].y;
					}
					hmi_object_index = hmi_dyn_xy_container_table[hmi_object_index].container_object_table.p_object_table->object_id;/*get container first child to disp*/
					hmi_cube_get_id_prop(hmi_object_index,pcube_texture,pfather_point);
				}
			}
			else
			#endif 
			#if HMI_DXY_BITMAPS_NUMBER> 0 
			if(HMI_IS_DYN_XY_BITMAP(hmi_object_id))
			{	  
				hmi_object_index = HMI_GET_DYN_XY_BITMAP_INDEX(hmi_object_id );
				if(hmi_object_index < HMI_DXY_BITMAPS_NUMBER)
				{	
					if(pfather_point !=NULL)
					{
						pfather_point->x	+= hmi_bmp_dyn_xy_rect[hmi_object_index].x;
						pfather_point->y	+= hmi_bmp_dyn_xy_rect[hmi_object_index].y;
					}
					pcube_texture->tex_rect.x	= hmi_bmp_dyn_xy_rect[hmi_object_index].x;
					pcube_texture->tex_rect.y	= hmi_bmp_dyn_xy_rect[hmi_object_index].y;
					pcube_texture->tex_rect.w	= hmi_bmp_dyn_xy_rect[hmi_object_index].w;
					pcube_texture->tex_rect.h	= hmi_bmp_dyn_xy_rect[hmi_object_index].h;
					pcube_texture->tex_rect.angel= hmi_bmp_dyn_xy_rect[hmi_object_index].angel;
					pcube_texture->tex_rect.alpha= hmi_bmp_dyn_xy_rect[hmi_object_index].alpha;
				#if		defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)
					pcube_texture->tex_prop.w	= hmi_bmp_dyn_xy_prop_table[hmi_object_index].w;
					pcube_texture->tex_prop.h	= hmi_bmp_dyn_xy_prop_table[hmi_object_index].h;
					pcube_texture->tex_prop.data_len=hmi_bmp_dyn_xy_prop_table[hmi_object_index].data_len;
					pcube_texture->tex_prop.pbitmap_data=hmi_bmp_dyn_xy_prop_table[hmi_object_index].pbitmap_data;
				#elif	(defined(HMI_GRAPHIC_OPENGLES))
					pcube_texture->tex_prop.pbitmap_data.seg_no		= 
						hmi_bmp_dyn_xy_prop_table[hmi_object_index].pbitmap_data.seg_no;
				
					pcube_texture->tex_prop.pbitmap_data.index		= 
						hmi_bmp_dyn_xy_prop_table[hmi_object_index].pbitmap_data.index;
					
					pcube_texture->tex_prop.pbitmap_data.fmt_index	= 
						hmi_bmp_dyn_xy_prop_table[hmi_object_index].pbitmap_data.fmt_index;
					
					pcube_texture->tex_prop.pbitmap_data.x			= 
						hmi_bmp_dyn_xy_prop_table[hmi_object_index].pbitmap_data.x;
					
					pcube_texture->tex_prop.pbitmap_data.y			= 
						hmi_bmp_dyn_xy_prop_table[hmi_object_index].pbitmap_data.y;
					
					pcube_texture->tex_prop.pbitmap_data.w			= 
						hmi_bmp_dyn_xy_prop_table[hmi_object_index].pbitmap_data.w;
					
					pcube_texture->tex_prop.pbitmap_data.h			= 
						hmi_bmp_dyn_xy_prop_table[hmi_object_index].pbitmap_data.h;
				#endif
					pcube_texture->tex_attr.image_attr	= hmi_dxy_bitmap_attr_table[hmi_object_index].image_attr;
					#if defined( HMI_MCU_S6J3200 )||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)
					pcube_texture->tex_attr.pixel_fmt	= hmi_dxy_bitmap_attr_table[hmi_object_index].pixel_fmt;
					#endif
					pcube_texture->tex_id				= hmi_object_id;
				}
			}
			else
			#endif
			#if HMI_SXY_BITMAPS_NUMBER> 0
			if(HMI_IS_S_XY_BITMAP(hmi_object_id))
			{	         
		        hmi_object_index = HMI_GET_SXY_BITMAPS_ID_INDEX(hmi_object_id);			
			    if(hmi_object_index < HMI_SXY_BITMAPS_NUMBER)
			    {
					if(pfather_point !=NULL)
					{
						pfather_point->x+=hmi_bmp_static_xy_rect[hmi_object_index].x;
						pfather_point->y+=hmi_bmp_static_xy_rect[hmi_object_index].y;
					}
					pcube_texture->tex_rect.x	= hmi_bmp_static_xy_rect[hmi_object_index].x;
					pcube_texture->tex_rect.y	= hmi_bmp_static_xy_rect[hmi_object_index].y;
					pcube_texture->tex_rect.w	= hmi_bmp_static_xy_rect[hmi_object_index].w;
					pcube_texture->tex_rect.h	= hmi_bmp_static_xy_rect[hmi_object_index].h;
					pcube_texture->tex_rect.angel= 0;
					pcube_texture->tex_rect.alpha= hmi_bmp_static_xy_rect[hmi_object_index].alpha;
				#if		defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)
					pcube_texture->tex_prop.w	= hmi_bmp_static_xy_prop_table[hmi_object_index].w;
					pcube_texture->tex_prop.h	= hmi_bmp_static_xy_prop_table[hmi_object_index].h;
					pcube_texture->tex_prop.data_len= hmi_bmp_static_xy_prop_table[hmi_object_index].data_len;
					pcube_texture->tex_prop.pbitmap_data= hmi_bmp_static_xy_prop_table[hmi_object_index].pbitmap_data;
				#elif	(defined(HMI_GRAPHIC_OPENGLES))
					pcube_texture->tex_prop.pbitmap_data	= hmi_bmp_static_xy_prop_table[hmi_object_index].pbitmap_data;
					#if 1
					pcube_texture->tex_prop.pbitmap_data.seg_no		= 
						hmi_bmp_static_xy_prop_table[hmi_object_index].pbitmap_data.seg_no;
				
					pcube_texture->tex_prop.pbitmap_data.index		= 
						hmi_bmp_static_xy_prop_table[hmi_object_index].pbitmap_data.index;
					
					pcube_texture->tex_prop.pbitmap_data.fmt_index	= 
						hmi_bmp_static_xy_prop_table[hmi_object_index].pbitmap_data.fmt_index;
					
					pcube_texture->tex_prop.pbitmap_data.x			= 
						hmi_bmp_static_xy_prop_table[hmi_object_index].pbitmap_data.x;
					
					pcube_texture->tex_prop.pbitmap_data.y			= 
						hmi_bmp_static_xy_prop_table[hmi_object_index].pbitmap_data.y;
					
					pcube_texture->tex_prop.pbitmap_data.w			= 
						hmi_bmp_static_xy_prop_table[hmi_object_index].pbitmap_data.w;
					
					pcube_texture->tex_prop.pbitmap_data.h			= 
						hmi_bmp_static_xy_prop_table[hmi_object_index].pbitmap_data.h;
					#endif
				#endif
					pcube_texture->tex_attr.image_attr = hmi_sxy_bitmap_attr_table[hmi_object_index].image_attr;
					#if defined( HMI_MCU_S6J3200 )||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)
					pcube_texture->tex_attr.pixel_fmt	= hmi_sxy_bitmap_attr_table[hmi_object_index].pixel_fmt;
					#endif
					pcube_texture->tex_id		= hmi_object_id;
			    }
			}
			else
			#endif 
			#if HMI_SXY_CUSTOM_CNT > 0
			if(HMI_IS_CUSTOM_SXY(hmi_object_id))
			{
			}
			else
			#endif	
			#if HMI_SXY_CONTAINERS_NUMBER > 0 
			if(HMI_IS_CONTAINERS_SXY(hmi_object_id))
			{
				hmi_object_index = HMI_GET_CONTAINERS_SXY_ID_INDEX(hmi_object_id );
				if(hmi_object_index < HMI_SXY_CONTAINERS_NUMBER)
				{
					if(pfather_point !=NULL)
					{
						pfather_point->x	+= hmi_static_container_rect[hmi_object_index].x;
						pfather_point->y	+= hmi_static_container_rect[hmi_object_index].y;
					}
					hmi_object_index	= hmi_sxy_container_table[hmi_object_index].container_object_table.p_object_table->object_id;/*get container first child to disp*/
					hmi_cube_get_id_prop(hmi_object_index,pcube_texture,pfather_point);
				}
			}
			else
			#endif	  
			{
				;
			}	   	   
		}
	}
}

/*
pfather_point x,y always 0,0
*/
static void hmi_cube_get_id_prop2(HMI_OBJECT_ID_STR		hmi_object_id,
								HMI_CUBE_TEXTURE_PROP	*pcube_texture,
								SPOINT_TP				*pfather_point)			
{
	hmi_cube_get_id_prop(hmi_object_id,
							pcube_texture,
							pfather_point);
	
	if(pfather_point != NULL)
	{
		pfather_point->x	= 0;
		pfather_point->y	= 0;
	}
}


//#endif
/*
Get  element x,y(not include container)
*/
static void hmi_cube_get_id_point(HMI_OBJECT_ID_STR		hmi_object_id,
											S3POINT_TP			*ppoint)			
{
	HMI_OBJECT_ID_STR			hmi_object_index	= 0;
	
	if(ppoint != NULL)
	{	 		
		#if HMI_DXY_PAGES_NUMBER > 0 
		if(HMI_IS_DXY_PAGE(hmi_object_id))
		{
			hmi_object_index	= HMI_GET_PAGE_DXY_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DXY_PAGES_NUMBER)
			{
				ppoint->x	= hmi_dxy_page_rect[hmi_object_index].x;
				ppoint->y	= hmi_dxy_page_rect[hmi_object_index].y;
				ppoint->z	= 0;		 		
			}
		}
		else
		#endif

		#if HMI_SXY_PAGES_NUMBER > 0 
		if(HMI_IS_SXY_PAGE(hmi_object_id))
		{
			hmi_object_index	= HMI_GET_PAGE_SXY_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_SXY_PAGES_NUMBER)
			{
				ppoint->x	= hmi_sxy_page_rect[hmi_object_index].x;
				ppoint->y	= hmi_sxy_page_rect[hmi_object_index].y;
				ppoint->z	= 0;		 		
			}
		}
		else
		#endif

		#if HMI_DYN_EDIT_TEXTS_NUMBER/*editable text*/ > 0	   
		if(HMI_IS_DYN_TEXTS(hmi_object_id))
		{	         
			#if HMI_DYN_XY_EDIT_TEXTS_NUMBER > 0
			if(HMI_IS_DYN_TEXT_DXY(hmi_object_id))
		   	{
				hmi_object_index = HMI_GET_DYN_TEXTS_DXY_POS_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_DYN_XY_EDIT_TEXTS_NUMBER)
				{
					ppoint->x	= hmi_dyn_xy_edit_text_prop_table[hmi_object_index].text_rect.x;
					ppoint->y	= hmi_dyn_xy_edit_text_prop_table[hmi_object_index].text_rect.y;
					ppoint->z	= 0;										
				}				
			}
			else
			#endif		 	
			#if HMI_S_XY_EDIT_TEXTS_NUMBER > 0
			if(HMI_IS_DYN_TEXT_SXY(hmi_object_id))
			{
				hmi_object_index = HMI_GET_DYN_TEXTS_SXY_POS_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_S_XY_EDIT_TEXTS_NUMBER)
				{
					ppoint->x	= hmi_static_xy_edit_text_prop_table[hmi_object_index].text_rect.x;
					ppoint->y	= hmi_static_xy_edit_text_prop_table[hmi_object_index].text_rect.y;
					ppoint->z	= 0;										
				}				
			}
			else
			#endif
			{
			}
		}
		else
		#endif	
		
		#if HMI_STATIC_TEXTS_NUMBER/*uneditable text*/ > 0 
		if(HMI_IS_STATIC_TEXTS(hmi_object_id)) 
		{	         
			 #if HMI_UNEDIT_TEXTS_DYN_XY_NUMBER > 0
		     if(HMI_IS_UNEDIT_TEXT_DYN_XY(hmi_object_id))
		     {	
			 	hmi_object_index  = HMI_GET_DYN_XY_UNEDIT_POS_COLOR_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_UNEDIT_TEXTS_DYN_XY_NUMBER)
				{
					ppoint->x	= hmi_dyn_xy_unedit_text_prop_table[hmi_object_index].text_rect.x;
					ppoint->y	= hmi_dyn_xy_unedit_text_prop_table[hmi_object_index].text_rect.y;
					ppoint->z	= 0;				 		
				}											
		     }
			 else 
			#endif
			{
				#if HMI_UNEDIT_TEXTS_STATIC_XY_NUMBER > 0					
				if(HMI_IS_UNEDIT_TEXT_STATIC_XY(hmi_object_id))
				{
					hmi_object_index = HMI_GET_S_XY_UNEDIT_POS_COLOR_INDEX(hmi_object_id);
					if(hmi_object_index < HMI_UNEDIT_TEXTS_STATIC_XY_NUMBER)
					{
						ppoint->x	= hmi_static_xy_unedit_text_prop_table[hmi_object_index].text_rect.x;
						ppoint->y	= hmi_static_xy_unedit_text_prop_table[hmi_object_index].text_rect.y;
						ppoint->z	= 0;						
					}															
				}
				#endif
			}
		} 
		else
		#endif	

		#if HMI_DXY_SCROLLBAR_NUMBER>0
		if(HMI_IS_DYN_SCROLLBAR_DXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_SCROLLBAR_DXY_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DXY_SCROLLBAR_NUMBER)
			{
				ppoint->x	= hmi_dxy_scrollbar_rect[hmi_object_index].x;
				ppoint->y	= hmi_dxy_scrollbar_rect[hmi_object_index].y;
				ppoint->z	= 0;				
			}						
		}
		else		
		#endif
		
		#if HMI_SXY_SCROLLBAR_NUMBER>0
		if(HMI_IS_DYN_SCROLLBAR_SXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_SCROLLBAR_SXY_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_SXY_SCROLLBAR_NUMBER)
			{
				ppoint->x	= hmi_sxy_scrollbar_rect[hmi_object_index].x;
				ppoint->y	= hmi_sxy_scrollbar_rect[hmi_object_index].y;
				ppoint->z	= 0;								
			}			
		}
		else
		#endif


		#if HMI_SXY_BUTTON_NUMBER>0 		
		if(HMI_IS_DYN_BUTTON_SXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_BUTTON_SXY_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_SXY_BUTTON_NUMBER)
			{
				ppoint->x	= hmi_sxy_button_rect[hmi_object_index].x;
				ppoint->y	= hmi_sxy_button_rect[hmi_object_index].y;
				ppoint->z	= 0;							
			}			
		}		
		else 		
		#endif

		#if HMI_DXY_BUTTON_NUMBER > 0  
		if(HMI_IS_DYN_BUTTON_DXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_BUTTON_DXY_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DXY_BUTTON_NUMBER)
			{
				ppoint->x	= hmi_dxy_button_rect[hmi_object_index].x;
				ppoint->y	= hmi_dxy_button_rect[hmi_object_index].y;
				ppoint->z	= 0;												
			}					
		}		
		else
		#endif

		#if HMI_SXY_IMAGELIST_NUMBER>0
		if(HMI_IS_DYN_IMAGELIST_SXY(hmi_object_id))
	    {
			hmi_object_index = HMI_GET_DYN_IMAGELIST_SXY_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_SXY_IMAGELIST_NUMBER)
			{	
				ppoint->x	= hmi_sxy_imagelist_rect[hmi_object_index].x;
				ppoint->y	= hmi_sxy_imagelist_rect[hmi_object_index].y;	
				ppoint->z	= 0;
			}
		}
		else
		#endif	
		
		#if HMI_DXY_IMAGELIST_NUMBER > 0
		if(HMI_IS_DYN_IMAGELIST_DXY(hmi_object_id))
	    {
			hmi_object_index = HMI_GET_DYN_IMAGELIST_DXY_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DXY_IMAGELIST_NUMBER)
			{
				ppoint->x	= hmi_dxy_imagelist_rect[hmi_object_index].x;
				ppoint->y	= hmi_dxy_imagelist_rect[hmi_object_index].y;
				ppoint->z	= 0;
			}
		}		
		else
		#endif
		
		#if HMI_STATIC_FILL_PAGES_NUMBER > 0 
		if(HMI_IS_SXY_FILL_PAGES(hmi_object_id))
		{
			hmi_object_index  = HMI_GET_SXY_FILL_PAGES_ID_INDEX(hmi_object_id );
			if(hmi_object_index < HMI_STATIC_FILL_PAGES_NUMBER)
			{
				ppoint->x	= hmi_fills_static_xy_rect[hmi_object_index].x;
				ppoint->y	= hmi_fills_static_xy_rect[hmi_object_index].y;
				ppoint->z	= hmi_fills_static_xy_rect[hmi_object_index].z;
				
			}				 
		}
		else
		#endif

		#if HMI_DYN_FILL_PAGES_NUMBER > 0
		if(HMI_IS_DYN_NFILL(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_NFILL_INDEX(hmi_object_id );
			if(hmi_object_index < HMI_DYN_FILL_PAGES_NUMBER)
			{
				ppoint->x	= hmi_fills_dyn_xy_rect[hmi_object_index].x;
				ppoint->y	= hmi_fills_dyn_xy_rect[hmi_object_index].y;	
				ppoint->z	= hmi_fills_dyn_xy_rect[hmi_object_index].z;						
			}			
		}
		else
		#endif
									
		#if HMI_DXY_BITMAPS_NUMBER> 0 
		if(HMI_IS_DYN_XY_BITMAP(hmi_object_id))
		{	  
			hmi_object_index = HMI_GET_DYN_XY_BITMAP_INDEX(hmi_object_id );
			if(hmi_object_index < HMI_DXY_BITMAPS_NUMBER)
			{		
				ppoint->x	= hmi_bmp_dyn_xy_rect[hmi_object_index].x;
				ppoint->y	= hmi_bmp_dyn_xy_rect[hmi_object_index].y;				
			}
		}
		else
		#endif
		#if HMI_SXY_BITMAPS_NUMBER> 0
		if(HMI_IS_S_XY_BITMAP(hmi_object_id))
		{	         
	        hmi_object_index = HMI_GET_SXY_BITMAPS_ID_INDEX(hmi_object_id);			
		    if(hmi_object_index < HMI_SXY_BITMAPS_NUMBER)
		    {
				ppoint->x	= hmi_bmp_static_xy_rect[hmi_object_index].x;
				ppoint->y	= hmi_bmp_static_xy_rect[hmi_object_index].y;				
		    }
		}
		else
		#endif
		#if HMI_DXY_CONTAINERS_NUMBER > 0U	
		if(HMI_IS_DYN_XY_CONTAINER(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_object_id );
			if(hmi_object_index < HMI_DXY_CONTAINERS_NUMBER)
			{
				ppoint->x	= hmi_dyn_xy_container_rect[hmi_object_index].x;
				ppoint->y	= hmi_dyn_xy_container_rect[hmi_object_index].y;
			}
		}
		else
		#endif 
		#if HMI_SXY_CONTAINERS_NUMBER > 0 
		if(HMI_IS_CONTAINERS_SXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_CONTAINERS_SXY_ID_INDEX(hmi_object_id );
			if(hmi_object_index < HMI_SXY_CONTAINERS_NUMBER)
			{
				ppoint->x	= hmi_static_container_rect[hmi_object_index].x;
				ppoint->y	= hmi_static_container_rect[hmi_object_index].y;
			}
		}
		else
		#endif	  
		{
			;
		}	   	   
	}
}

#define HMI_SPLINE_DOWN_ZONE_OFFSET_DELTA		5
#define HMI_SPLINE_MALLOC_EXPAND				4

BOOLEAN hmi_write_one_spline_point(HMI_RECT_STR CONST 	* 	prect,
								SPOINT_TP *				ppoint_array,	
								POINT_RANGE_TP	 CONST *ppoint_range,
								POINT_FIFO_TP *			pphead_tail_range,	
								UINT8					direct)
{			
	SPOINT32_TP *phmi_input_point_polygon	= NULL;	
	/*SPOINT_TP		side		= {0,0};*/
	SINT16			max_min		= 0;
	SINT16			max_min_xy	= 0;
	SINT16			clip		= 0;
	SINT16			abs_x		= 0;
	SINT32			offset		= 0;
	UINT16			head		= 0U;
	UINT16			tail		= 0U;
	UINT16			pre			= 0U;
	UINT16			fifo_cnt	= 0U;
	UINT16			index		= 0U;
	UINT16			begin		= 0U;
	UINT16			end			= 0U;
	BOOLEAN			finished	= FALSE;
	UINT32			hmi_spline_point_nb	=0U;
	BOOLEAN			hmi_write_success	= 0U;
	
	if((ppoint_array != NULL) && (ppoint_range != NULL) &&
		(pphead_tail_range != NULL) &&(prect != NULL)&&
		(pphead_tail_range->head != pphead_tail_range->tail))
	{
		head	= pphead_tail_range->head;
		tail	= pphead_tail_range->tail;
		offset	= pphead_tail_range->offset;
		begin	= ppoint_range->begin;
		end		= ppoint_range->end;
		
		if((direct == HMI_DOWN)||
			(direct == HMI_DOWN_BEZIER))
		{				
			if(offset <= 0)
			{
				clip	= 0 ;
				pre		= head;					
				/*clip the point at 'prect->x' left side*/
				while((head != tail) && (finished == FALSE))/*not empty*/
				{		
					abs_x	= (SINT16)(ppoint_array[head].x + offset);
					if(abs_x	< clip)
					{
						pre	= head;
						head++;
						if(head >= end)
						{
							head	= begin;
						}
					}
					else
					{
						finished	= TRUE;
					}
				}
				if(finished == TRUE)
				{
					pphead_tail_range->head	= pre;
				}
				/*alloc*/
				head	= pphead_tail_range->head;
				if(tail > head)
				{
					hmi_spline_point_nb	= tail - head;
				}
				else
				{
					hmi_spline_point_nb	= (ppoint_range->end - head) + 
											(tail - ppoint_range->begin);
				}
				
				hmi_spline_point_nb	= hmi_spline_point_nb + HMI_SPLINE_ZONE_CLOSE_POINT_NB; 
				
				hmi_spline_input_point.length			= (U16)hmi_spline_point_nb;
				hmi_spline_input_point.remain_length	= 0U;
				if(hmi_spline_input_point.phmi_input_point_polygon == NULL)
				{
				#if defined(HMI_GRAPHIC_OPENGLES) /*expand line to zone,need double size.2020,03,07*/
					if(hmi_spline_point_nb + hmi_spline_point_nb < HMI_MAX_SPLINE_POINT)
				#else
					if(hmi_spline_point_nb  < HMI_MAX_SPLINE_POINT)
				#endif
					{					
						hmi_spline_input_point.act_array_len 	= HMI_MAX_SPLINE_POINT ;					
					}
					else
					{					
						hmi_spline_input_point.act_array_len 	= (U16)(hmi_spline_point_nb * HMI_SPLINE_MALLOC_EXPAND);					
					}				
					
					hmi_spline_point_nb =(sizeof(SPOINT32_TP)) * (hmi_spline_input_point.act_array_len);
					hmi_spline_input_point.phmi_input_point_polygon = (SPOINT32_TP*)(hmi_malloc_vram(hmi_spline_point_nb));
					hmi_spline_point_nb =(sizeof(HMI_NEXT_PRE_LIST_STR))*(hmi_spline_input_point.act_array_len);
					hmi_spline_input_point.pnext_pre_list	= (HMI_NEXT_PRE_LIST_STR*)(hmi_malloc_vram(hmi_spline_point_nb));
				}
				else
				{
				#if defined(HMI_GRAPHIC_OPENGLES) /*expand line to zone,need double size.2020,03,07*/
					if(hmi_spline_point_nb + hmi_spline_point_nb  > hmi_spline_input_point.act_array_len)
				#else
					if(hmi_spline_point_nb  > hmi_spline_input_point.act_array_len)
				#endif
					{																					
						hmi_spline_input_point.act_array_len 			= (U16)(hmi_spline_point_nb * HMI_SPLINE_MALLOC_EXPAND);					
								
						hmi_dealloc_vram((void *)(hmi_spline_input_point.phmi_input_point_polygon));
						hmi_dealloc_vram((void *)(hmi_spline_input_point.pnext_pre_list));
						hmi_spline_input_point.phmi_input_point_polygon	= NULL;
						hmi_spline_point_nb = (sizeof(SPOINT32_TP))*
												(hmi_spline_input_point.act_array_len);
						hmi_spline_input_point.phmi_input_point_polygon	=(SPOINT32_TP*)(hmi_malloc_vram(hmi_spline_point_nb));
						hmi_spline_point_nb = (sizeof(HMI_NEXT_PRE_LIST_STR))*
												(hmi_spline_input_point.act_array_len);
						hmi_spline_input_point.pnext_pre_list	= (HMI_NEXT_PRE_LIST_STR*)(hmi_malloc_vram(hmi_spline_point_nb));
					}
				}
				
				/*copy point to array*/
				head			= pphead_tail_range->head;
				max_min			= 0;
				index			= 1U;
				phmi_input_point_polygon	= hmi_spline_input_point.phmi_input_point_polygon;
				if(phmi_input_point_polygon != NULL)
				{
					while((head != tail))/*not empty*/ 
					{						
						phmi_input_point_polygon[index].x	= (SINT32)(ppoint_array[head].x + offset);
						phmi_input_point_polygon[index].y	= (SINT32)(ppoint_array[head].y);						
						if(ppoint_array[head].y > max_min)
						{
							max_min	= ppoint_array[head].y;
						}
						fifo_cnt++;
						index++;
						head++;
						if(head >= end)
						{
							head	= begin; 
						}
					}
					hmi_write_success	= TRUE;
					max_min_xy	=  prect->h - 1;
					if(max_min_xy > max_min)
					{
						max_min	= max_min_xy;
					}
					else
					{
						max_min += HMI_SPLINE_DOWN_ZONE_OFFSET_DELTA;
					}
					if(fifo_cnt >= HMI_SPLINE_ZONE_MIN_CNT)
					{
						/*write last point*/
						phmi_input_point_polygon[index].x	= phmi_input_point_polygon[index - 1].x;
						phmi_input_point_polygon[index].y	= max_min;
						/*write 0 point*/
						phmi_input_point_polygon[0].x		= phmi_input_point_polygon[1].x;
						phmi_input_point_polygon[0].y		= max_min;
					}
				}
			}
			else if(offset > 0)
			{
				clip	= prect->x + prect->w;				
				pre		= head;					
				/*clip the point at 'prect->x+ prect->w' right side*/
				while((head != tail) && (finished == FALSE))/*not empty*/
				{		
					abs_x	= (SINT16)(ppoint_array[head].x + offset);
					if(abs_x	> clip)
					{
						pre	= head;
						head++;
						if(head >= end)
						{
							head	= begin;
						}
					}
					else
					{
						finished	= TRUE;
					}
				}
				if(finished == TRUE)
				{
					pphead_tail_range->head	= pre;
				}
				/*alloc*/
				head	= pphead_tail_range->head;
				if(tail > head)
				{
					hmi_spline_point_nb	= tail - head;
				}
				else
				{
					hmi_spline_point_nb	= (ppoint_range->end - head) + 
											(tail - ppoint_range->begin);
				}
				
				if((direct != HMI_CLOSE)&&
					(direct != HMI_CLOSE_BEZIER))
				{
					hmi_spline_point_nb	= hmi_spline_point_nb + HMI_SPLINE_ZONE_CLOSE_POINT_NB; 
				}
				
				hmi_spline_input_point.length			= (U16)hmi_spline_point_nb;
				hmi_spline_input_point.remain_length	= 0U;
				if(hmi_spline_input_point.phmi_input_point_polygon == NULL)
				{
				#if defined(HMI_GRAPHIC_OPENGLES)
					if(hmi_spline_point_nb + hmi_spline_point_nb < HMI_MAX_SPLINE_POINT) /*expand line to zone,need double size.2020,03,07*/
				#else
					if(hmi_spline_point_nb  < HMI_MAX_SPLINE_POINT)
				#endif
					{					
						hmi_spline_input_point.act_array_len 	= HMI_MAX_SPLINE_POINT;					
					}										
					else
					{					
						hmi_spline_input_point.act_array_len 	= (U16)(hmi_spline_point_nb * HMI_SPLINE_MALLOC_EXPAND);					
					}
					hmi_spline_point_nb =(sizeof(SPOINT32_TP))*(hmi_spline_input_point.act_array_len);
					hmi_spline_input_point.phmi_input_point_polygon = (SPOINT32_TP*)(hmi_malloc_vram(hmi_spline_point_nb));
					hmi_spline_point_nb =(sizeof(HMI_NEXT_PRE_LIST_STR))*(hmi_spline_input_point.act_array_len);
					hmi_spline_input_point.pnext_pre_list	= (HMI_NEXT_PRE_LIST_STR*)(hmi_malloc_vram(hmi_spline_point_nb));
				}
				else
				{
				#if defined(HMI_GRAPHIC_OPENGLES) /*expand line to zone,need double size.2020,03,07*/
					if(hmi_spline_point_nb + hmi_spline_point_nb > hmi_spline_input_point.act_array_len)
				#else
					if(hmi_spline_point_nb  > hmi_spline_input_point.act_array_len)
				#endif
					{					
						hmi_spline_input_point.act_array_len 			= (U16)(hmi_spline_point_nb * HMI_SPLINE_MALLOC_EXPAND);					
						hmi_dealloc_vram((void *)(hmi_spline_input_point.phmi_input_point_polygon));
						hmi_dealloc_vram((void *)(hmi_spline_input_point.pnext_pre_list));
						hmi_spline_input_point.phmi_input_point_polygon	= NULL;
						hmi_spline_point_nb = (sizeof(SPOINT32_TP))*
												(hmi_spline_input_point.act_array_len);
						hmi_spline_input_point.phmi_input_point_polygon	=(SPOINT32_TP*)(hmi_malloc_vram(hmi_spline_point_nb));
						hmi_spline_point_nb = (sizeof(HMI_NEXT_PRE_LIST_STR))*
												(hmi_spline_input_point.act_array_len);
						hmi_spline_input_point.pnext_pre_list	= (HMI_NEXT_PRE_LIST_STR*)(hmi_malloc_vram(hmi_spline_point_nb));
					}
				}
				
				/*copy point to array*/
				head			= pphead_tail_range->head;
				max_min			= 0;
				index			= 1U;
				phmi_input_point_polygon	= hmi_spline_input_point.phmi_input_point_polygon;
				if(phmi_input_point_polygon != NULL)
				{
					while((head != tail))/*not empty*/ 
					{						
						phmi_input_point_polygon[index].x	= (SINT32)(ppoint_array[head].x + offset);
						phmi_input_point_polygon[index].y	= (SINT32)(ppoint_array[head].y);						
						if(ppoint_array[head].y > max_min)
						{
							max_min	= ppoint_array[head].y;
						}
						fifo_cnt++;
						index++;
						head++;
						if(head >= end)
						{
							head	= begin; 
						}
					}
					hmi_write_success	= TRUE;
					max_min_xy	=  prect->h-1;
					if(max_min_xy > max_min)
					{
						max_min	= max_min_xy;
					}
					else
					{
						max_min	+= HMI_SPLINE_DOWN_ZONE_OFFSET_DELTA;
					}
					if(fifo_cnt >= HMI_SPLINE_ZONE_MIN_CNT)
					{
						/*write last point*/
						phmi_input_point_polygon[index].x	= phmi_input_point_polygon[index - 1].x;
						phmi_input_point_polygon[index].y	= max_min;
						/*write 0 point*/
						phmi_input_point_polygon[0].x		= phmi_input_point_polygon[1].x;
						phmi_input_point_polygon[0].y		= max_min;
					}
				}
			}
			else
			{
			}
		}
		else if((direct == HMI_UP)||
			(direct == HMI_UP_BEZIER))
		{
		#if 0
			if(pphead_tail_range->offset < 0)
			{
				clip	= prect->x;
			}
			else if(pphead_tail_range->offset > 0)
			{
				clip	= prect->x + prect->w;
			}
			else
			{
			}
		#endif
		}
		else if((direct == HMI_LEFT)||
			(direct == HMI_LEFT_BEZIER))
			
		{
		#if 0
			if(pphead_tail_range->offset < 0)
			{
				clip	= prect->x;
			}
			else if(pphead_tail_range->offset > 0)
			{
				clip	= prect->x + prect->w;
			}
			else
			{
			}
		#endif
		}
		else if((direct == HMI_RIGHT)||
			(direct == HMI_RIGHT_BEZIER))
		{
		#if 0
			if(pphead_tail_range->offset < 0)
			{
				clip	= prect->x;
			}
			else if(pphead_tail_range->offset > 0)
			{
				clip	= prect->x + prect->w;
			}
			else
			{
			}
		#endif
		}
		#if 0
		else if(direct == HMI_CLOSE)
		{
			if(pphead_tail_range->offset < 0)
			{
				clip	= prect->x;
			}
			else if(pphead_tail_range->offset > 0)
			{
				clip	= prect->x + prect->w;
			}
			else
			{
			}
		}
		#endif
		else
		{
		}
	}
	
	return hmi_write_success;
}

BOOLEAN hmi_write_mul_spline_point(HMI_RECT_STR CONST 	*	pfather_rect, 
									HMI_OBJECT_ID_STR 		hmi_object_id,
									UINT8					direct)
{
	HMI_OBJECT_ID_STR	hmi_object_index	= 0;
	UINT8				hmi_number_object	= 0;
	HMI_OBJECT_PROP_STR CONST *	phmi_container_object_table	= NULL;
	UINT32 				hmi_spline_point_nb	= 0;	
	S3POINT_TP			hmi_point_xy		= {0};			
	UINT8				object_index		= 0;
	SINT16				hmi_min_x			= 0;
	SINT16				hmi_min_y			= 0;
	SINT16				hmi_max_x			= 0;
	SINT16				hmi_max_y			= 0;
	BOOLEAN				hmi_get_point_flag	= 0;
	SPOINT32_TP			*phmi_input_point	= NULL;
	
	hmi_spline_input_point.length			= 0;
	hmi_spline_input_point.remain_length	= 0;
	if(hmi_object_id  >= HMI_PAGE_SXY_MAX_ID)/*not a page*/
	{	
		#if HMI_DYN_CONTAINERS_NUMBER > 0
		if(HMI_IS_DYN_CONTAINER(hmi_object_id))
		{			
			#if HMI_ALL_STATIC_CONTAINERS_NUMBER > 0  
			hmi_object_id  = HMI_GET_DYN_CONTAINERS_ID_INDEX(hmi_object_id);
			hmi_engine_get_static_container_id(&hmi_object_id);/*get static container ID*/			
			#endif
		}
		#endif
		#if HMI_DXY_CONTAINERS_NUMBER > 0  
		if(HMI_IS_DYN_XY_CONTAINER(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_object_id );
			if(hmi_object_index < HMI_DXY_CONTAINERS_NUMBER)
			{
				hmi_number_object			= hmi_dyn_xy_container_table[hmi_object_index].container_object_table.object_number;
				phmi_container_object_table	= hmi_dyn_xy_container_table[hmi_object_index].container_object_table.p_object_table;
			}
		}
		else
		#endif 
		#if HMI_SXY_CONTAINERS_NUMBER > 0 
		if(HMI_IS_CONTAINERS_SXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_CONTAINERS_SXY_ID_INDEX(hmi_object_id );
			if(hmi_object_index < HMI_SXY_CONTAINERS_NUMBER)
			{
				hmi_number_object			= hmi_sxy_container_table[hmi_object_index].container_object_table.object_number;
				phmi_container_object_table	= hmi_sxy_container_table[hmi_object_index].container_object_table.p_object_table;
			}
		}
		else
		#endif	  
		{
			;
		}
		
		/*ALLOC POINT RANGE*/
		if((phmi_container_object_table  != NULL)&&(hmi_number_object >1))
		{
			hmi_spline_point_nb	=hmi_number_object;
			if((direct != HMI_CLOSE)&&(direct != HMI_CLOSE_BEZIER))
			{
				hmi_spline_point_nb	+= HMI_SPLINE_ZONE_CLOSE_POINT_NB; 
			}
			hmi_spline_input_point.length			= (U16)hmi_spline_point_nb;
			hmi_spline_input_point.remain_length	= (U16)hmi_spline_point_nb;
			if(hmi_spline_input_point.phmi_input_point_polygon == NULL)
			{
				if(hmi_spline_point_nb < HMI_MAX_SPLINE_POINT)
				{
					hmi_spline_input_point.act_array_len 	= HMI_MAX_SPLINE_POINT;
				}
				else
				{
					hmi_spline_input_point.act_array_len 	= (U16)hmi_spline_point_nb;
				}
				hmi_spline_point_nb =(sizeof(SPOINT32_TP))*(hmi_spline_input_point.act_array_len);
				hmi_spline_input_point.phmi_input_point_polygon = (SPOINT32_TP*)(hmi_malloc_vram(hmi_spline_point_nb));
				hmi_spline_point_nb =(sizeof(HMI_NEXT_PRE_LIST_STR))*(hmi_spline_input_point.act_array_len);
				hmi_spline_input_point.pnext_pre_list	= (HMI_NEXT_PRE_LIST_STR*)(hmi_malloc_vram(hmi_spline_point_nb));
			}
			else
			{
				if(hmi_spline_point_nb > hmi_spline_input_point.act_array_len)
				{
					hmi_spline_input_point.act_array_len 			= (U16)hmi_spline_point_nb;
					hmi_dealloc_vram((void *)(hmi_spline_input_point.phmi_input_point_polygon));
					hmi_dealloc_vram((void *)(hmi_spline_input_point.pnext_pre_list));
					hmi_spline_input_point.phmi_input_point_polygon	= NULL;
					hmi_spline_point_nb = (sizeof(SPOINT32_TP))*(hmi_spline_input_point.act_array_len);
					hmi_spline_input_point.phmi_input_point_polygon	=(SPOINT32_TP*)(hmi_malloc_vram(hmi_spline_point_nb));
					hmi_spline_point_nb = (sizeof(HMI_NEXT_PRE_LIST_STR))*(hmi_spline_input_point.act_array_len);
					hmi_spline_input_point.pnext_pre_list	= (HMI_NEXT_PRE_LIST_STR*)(hmi_malloc_vram(hmi_spline_point_nb));
				}
			}
			hmi_get_point_flag = TRUE;
		}
		else
		{
			hmi_get_point_flag = FALSE;
		}
		
		if(hmi_get_point_flag == TRUE)//&&(phmi_container_object_table != NULL/*he2021-03-03*/)
		{	
			if(phmi_container_object_table != NULL/*he2021-03-03*/)
			{
			object_index = 0;
			phmi_input_point	= hmi_spline_input_point.phmi_input_point_polygon;
			while(object_index < hmi_number_object)
			{
				hmi_cube_get_id_point(phmi_container_object_table[object_index].object_id,&hmi_point_xy);
				if((direct == HMI_CLOSE)||(direct == HMI_CLOSE_BEZIER))
				{
					phmi_input_point[object_index].x	= hmi_point_xy.x;
					phmi_input_point[object_index].y	= hmi_point_xy.y;
				}
				else
				{
					phmi_input_point[object_index+1].x	= hmi_point_xy.x;
					phmi_input_point[object_index+1].y	= hmi_point_xy.y;
					
					if(object_index ==0)
					{
						hmi_min_x	= hmi_point_xy.x;
						hmi_min_y	= hmi_point_xy.y;
						hmi_max_x	= hmi_point_xy.x;
						hmi_max_y	= hmi_point_xy.y;
					}
					else
					{
						if(hmi_point_xy.x < hmi_min_x)
						{
							hmi_min_x = hmi_point_xy.x;
						}

						if(hmi_point_xy.x > hmi_max_x)
						{
							hmi_max_x = hmi_point_xy.x;
						}

						if(hmi_point_xy.y < hmi_min_y)
						{
							hmi_min_y = hmi_point_xy.y;
						}

						if(hmi_point_xy.y > hmi_max_y)
						{
							hmi_max_y = hmi_point_xy.y;
						}
					}	
				}
				object_index++;
			}
			
			if((direct != HMI_CLOSE)&&
				(direct != HMI_CLOSE_BEZIER))
			{
				if((direct == HMI_DOWN)||
					(direct == HMI_DOWN_BEZIER))
				{
					if(hmi_max_y < pfather_rect->h)
					{
						hmi_max_y = pfather_rect->h-1;
					}
					else
					{
						hmi_max_y += HMI_SPLINE_DOWN_ZONE_OFFSET_DELTA;
					}
					phmi_input_point[object_index+1].x	= phmi_input_point[object_index].x;
					phmi_input_point[object_index+1].y	= hmi_max_y ;
					phmi_input_point[0].x				= phmi_input_point[1].x;
					phmi_input_point[0].y				= hmi_max_y ;
				}
				else if((direct == HMI_UP)||
						(direct == HMI_UP_BEZIER))
				{
					if(hmi_min_y > 0)
					{
						hmi_min_y = 0;
					}
					else
					{
						hmi_max_y -= HMI_SPLINE_DOWN_ZONE_OFFSET_DELTA;
					}
					phmi_input_point[object_index+1].x	= phmi_input_point[object_index].x;
					phmi_input_point[object_index+1].y	= hmi_min_y ;
					phmi_input_point[0].x				= phmi_input_point[1].x;
					phmi_input_point[0].y				= hmi_min_y ;
				}
				else if((direct == HMI_RIGHT)||
						(direct == HMI_RIGHT_BEZIER))
				{
					if(hmi_max_x < pfather_rect->w )
					{
						hmi_max_x = pfather_rect->w-1;
					}
					phmi_input_point[object_index+1].x	= hmi_max_x;
					phmi_input_point[object_index+1].y	= phmi_input_point[object_index].y ;
					phmi_input_point[0].x				= hmi_max_x;
					phmi_input_point[0].y				= phmi_input_point[1].y ;
				}
				else if((direct == HMI_LEFT)||
						(direct == HMI_LEFT_BEZIER))
				{
					if(hmi_min_x > 0 )
					{
						hmi_min_x = 0;
					}
					phmi_input_point[object_index+1].x	= hmi_min_x;
					phmi_input_point[object_index+1].y	= phmi_input_point[object_index].y ;
					phmi_input_point[0].x				= hmi_min_x;
					phmi_input_point[0].y				= phmi_input_point[1].y ;
				}
				else
				{
				}
	
			}

			}
		}
	}
	return hmi_get_point_flag;
}


void hmi_get_axis(S3POINT_TP	 *paxis,HMI_CUBE_AXIS_PROP	CONST *pelem_axis,HMI_RECT_STR CONST *pfarther_rect)
{
	if((paxis != NULL)&&(pelem_axis != NULL)&&(pfarther_rect != NULL))
	{
		hmi_cube_get_container_point(pelem_axis->private_axis,&paxis[HMI_ROTATION_PRIVATE_AXIS]);
		hmi_cube_get_container_point(pelem_axis->public_axis1,&paxis[HMI_ROTATION_PUBLIC_AXIS1]);
		hmi_cube_get_container_point(pelem_axis->public_axis2,&paxis[HMI_ROTATION_PUBLIC_AXIS2]);		
		
		HMI_GET_OBJECT_SCREEN_COOR2(pfarther_rect,
					(&paxis[HMI_ROTATION_PUBLIC_AXIS1]),
					(&paxis[HMI_ROTATION_PUBLIC_AXIS1]));

		HMI_GET_OBJECT_SCREEN_COOR2(pfarther_rect,
					(&paxis[HMI_ROTATION_PUBLIC_AXIS2]),
					(&paxis[HMI_ROTATION_PUBLIC_AXIS2]));
	}
}
#endif

#if HMI_DXY_ROTATION_BITMAPS_NUMBER > 0
static BOOLEAN hmi_get_container_child_id(HMI_OBJECT_ID_STR *phmi_object_id)
{
	HMI_OBJECT_ID_STR			hmi_object_index	= 0;
	BOOLEAN 					hmi_changed_flag 	= FALSE;
	if((*phmi_object_id)  >= HMI_PAGE_SXY_MAX_ID)/*not a page*/
	{	
	#if HMI_DYN_CONTAINERS_NUMBER > 0
		if(HMI_IS_DYN_CONTAINER((*phmi_object_id)))
		{			
		#if HMI_ALL_STATIC_CONTAINERS_NUMBER > 0  
			(*phmi_object_id) = HMI_GET_DYN_CONTAINERS_ID_INDEX((*phmi_object_id));
			hmi_engine_get_static_container_id(phmi_object_id);/*get static container ID*/
			hmi_changed_flag = hmi_get_container_child_id(phmi_object_id);			
		#endif
		}
		else
	#endif
	#if HMI_DXY_CONTAINERS_NUMBER > 0  
		if(HMI_IS_DYN_XY_CONTAINER((*phmi_object_id)))
		{
			hmi_object_index = HMI_GET_DYN_XY_CONTAINER_INDEX((*phmi_object_id));
			if(hmi_object_index < HMI_DXY_CONTAINERS_NUMBER)
			{
				(*phmi_object_id)	= hmi_dyn_xy_container_table[hmi_object_index].container_object_table.p_object_table[0].object_id;/*get container first child to disp*/
				hmi_changed_flag	= hmi_get_container_child_id(phmi_object_id);
			}
		}
		else
	#endif 
	#if HMI_DXY_BITMAPS_NUMBER > 0 
		if(HMI_IS_DYN_XY_BITMAP((*phmi_object_id)))
		{	  
			hmi_changed_flag = TRUE;
		}
		else
	#endif
	#if HMI_SXY_BITMAPS_NUMBER > 0
		if(HMI_IS_S_XY_BITMAP((*phmi_object_id)))
		{			 
			hmi_changed_flag = TRUE;
		}
		else
	#endif 
	#if HMI_SXY_CONTAINERS_NUMBER > 0 
		if(HMI_IS_CONTAINERS_SXY((*phmi_object_id)))
		{
			hmi_object_index = HMI_GET_CONTAINERS_SXY_ID_INDEX((*phmi_object_id) );
			if(hmi_object_index < HMI_SXY_CONTAINERS_NUMBER)
			{
				(*phmi_object_id) = hmi_sxy_container_table[hmi_object_index].container_object_table.p_object_table[0].object_id;/*get container first child to disp*/
				hmi_changed_flag = hmi_get_container_child_id(phmi_object_id);
			}
		}
		else
	#endif	  
		{
			hmi_changed_flag=FALSE;
		}		   
	}
	return hmi_changed_flag;
}
#endif

/*SPLINE*/
#if HMI_DXY_SPLINE_NUMBER +HMI_SXY_SPLINE_NUMBER > 0
static BOOLEAN hmi_check_container_child_changed(HMI_OBJECT_ID_STR hmi_object_id, BOOLEAN check_all_child)/*true is check all ,false is check one */
{
	BOOLEAN 					hmi_obejct_changed			= FALSE;
	HMI_OBJECT_ID_STR			hmi_object_index			= 0;
	BOOLEAN 					hmi_container_loop_stop 	= FALSE;
	UINT8						hmi_loop					= 0;
	UINT8						hmi_number_object			= 0;
	HMI_OBJECT_PROP_STR CONST * phmi_container_object_table = NULL;
	
	if(hmi_object_id  >= HMI_PAGE_SXY_MAX_ID)/*not a page*/
	{
		while((hmi_container_loop_stop == FALSE)&&(hmi_loop < 255) &&(hmi_obejct_changed == FALSE))
		{
			hmi_loop++;
		#if HMI_DYN_CONTAINERS_NUMBER > 0
			if(HMI_IS_DYN_CONTAINER(hmi_object_id))
			{
				if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
				{
					hmi_obejct_changed = TRUE;
				}
				else
				{
				#if HMI_ALL_STATIC_CONTAINERS_NUMBER > 0  
					hmi_object_id  = HMI_GET_DYN_CONTAINERS_ID_INDEX(hmi_object_id);
					hmi_engine_get_static_container_id(&hmi_object_id);/*get static container ID*/
				#endif
				}
			}
		#endif					
		#if HMI_DXY_CONTAINERS_NUMBER > 0
			if(HMI_IS_DYN_XY_CONTAINER(hmi_object_id))
			{
				hmi_object_index = HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_object_id );
				if(hmi_object_index < HMI_DXY_CONTAINERS_NUMBER)
				{
					if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
					{
						hmi_obejct_changed = TRUE;
						hmi_container_loop_stop = TRUE;
					}
					else
					{
						hmi_number_object			= hmi_dyn_xy_container_table[hmi_object_index].container_object_table.object_number;
						phmi_container_object_table = hmi_dyn_xy_container_table[hmi_object_index].container_object_table.p_object_table;
						if(hmi_number_object > 0)
						{
							hmi_object_id				= phmi_container_object_table->object_id;
							if(check_all_child)
							{
								hmi_container_loop_stop = TRUE;
							}
						}
						else
						{
							hmi_container_loop_stop = TRUE;
							hmi_object_id			= HMI_NB_ELEMENTS;
						} 
					}
				}
			}
			else
		#endif
		#if HMI_SXY_CONTAINERS_NUMBER > 0
			if(HMI_IS_CONTAINERS_SXY(hmi_object_id))
			{
				hmi_object_index = HMI_GET_CONTAINERS_SXY_ID_INDEX(hmi_object_id );
				if(hmi_object_index < HMI_SXY_CONTAINERS_NUMBER)
				{
					hmi_number_object			= hmi_sxy_container_table[hmi_object_index].container_object_table.object_number;
					phmi_container_object_table = hmi_sxy_container_table[hmi_object_index].container_object_table.p_object_table;
					if(hmi_number_object > 0)
					{
						hmi_object_id				= phmi_container_object_table->object_id;
						if(check_all_child)
						{
							hmi_container_loop_stop = TRUE;
						}
					}
					else
					{
						hmi_container_loop_stop = TRUE;
						hmi_object_id			=HMI_NB_ELEMENTS;
					}
				}
			}
			else
		#endif
			{
				hmi_container_loop_stop = TRUE;
			}
		}
		
		if((hmi_obejct_changed == FALSE)&&(hmi_object_id !=HMI_NB_ELEMENTS))
		{
			if(check_all_child)
			{
				hmi_loop =0;
				do
				{
					hmi_obejct_changed = hmi_check_object_child_changed(hmi_object_id);
					hmi_loop ++;
					if(phmi_container_object_table != NULL)
					{
						if(hmi_loop < hmi_number_object)
						{
							hmi_object_id = phmi_container_object_table[hmi_loop].object_id;
						}
					}
				}
				while((hmi_loop < hmi_number_object)&&(hmi_obejct_changed == FALSE));
			}
			else
			{
				hmi_obejct_changed = hmi_check_object_child_changed(hmi_object_id);
			}
		}
	}
	return hmi_obejct_changed;
}

static BOOLEAN hmi_check_object_child_changed(HMI_OBJECT_ID_STR hmi_object_id)
{
	BOOLEAN						hmi_obejct_changed			= FALSE;
	HMI_OBJECT_ID_STR			hmi_object_index			= 0U;
	
	if(hmi_object_id  >= HMI_PAGE_SXY_MAX_ID)/*not a page*/
	{
		#if HMI_DYN_CONTAINERS_NUMBER > 0
		if(HMI_IS_DYN_CONTAINER(hmi_object_id))
		{
			if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
		 	{
				hmi_obejct_changed = TRUE;
			}
			else
			{
				#if HMI_ALL_STATIC_CONTAINERS_NUMBER > 0  
				hmi_object_id  = HMI_GET_DYN_CONTAINERS_ID_INDEX(hmi_object_id);
				hmi_engine_get_static_container_id(&hmi_object_id);/*get static container ID*/
				#endif
			}
		}
		#endif					
		#if HMI_DXY_CONTAINERS_NUMBER > 0
		if(HMI_IS_DYN_XY_CONTAINER(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_object_id );
			if(hmi_object_index < HMI_DXY_CONTAINERS_NUMBER)
			{
				if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
			 	{
					hmi_obejct_changed = TRUE;
				}
				else
				{
					hmi_obejct_changed = FALSE;
				}
			}
		}
		else
		#endif
		#if HMI_SXY_CONTAINERS_NUMBER > 0
		if(HMI_IS_CONTAINERS_SXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_CONTAINERS_SXY_ID_INDEX(hmi_object_id );
			if(hmi_object_index < HMI_SXY_CONTAINERS_NUMBER)
			{
				hmi_obejct_changed = FALSE;
			}
		}
		else
		#endif
		#if HMI_DYN_FILL_PAGES_NUMBER > 0
		if(HMI_IS_DYN_NFILL(hmi_object_id))
		{
			if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
		 	{
				hmi_obejct_changed = TRUE;
			}		
		}
		else
		#endif
		#if HMI_DXY_BITMAPS_NUMBER> 0 
		if(HMI_IS_DYN_XY_BITMAP(hmi_object_id))
		{	  
			if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
		 	{
				hmi_obejct_changed = TRUE;
			}
		}
		else
		#endif
		#if HMI_STATIC_FILL_PAGES_NUMBER > 0 
		if(HMI_IS_SXY_FILL_PAGES(hmi_object_id))
		{
			hmi_obejct_changed = FALSE;				 
		}
		else
		#endif
		#if HMI_SXY_BITMAPS_NUMBER> 0
		if(HMI_IS_S_XY_BITMAP(hmi_object_id))
		{	         
	        hmi_obejct_changed = FALSE;
		}
		else
		#endif 		  
		{
			hmi_obejct_changed = FALSE;	 
		}
	}		
	return hmi_obejct_changed;
}
#endif
#if HMI_DXY_CUSTOM_CNT + HMI_SXY_CUSTOM_CNT >0U
BOOLEAN hmi_engine_get_custom_prop(HMI_OBJECT_ID_STR custom_id,
										HMI_CUSTOM_PROP_STR *pobject_prop)
{
	HMI_OBJECT_ID_STR				hmi_object_index		= 0U;
	HMI_OBJECT_ID_STR				custom_lib_index		= 0U;
	BOOLEAN							get_success				= FALSE;
	HMI_CUSTOM_INFO_STR CONST*		phmi_custom_widget_info	= NULL;
	CUSTOM_ATTR_FUN_STR	CONST*		pcustom_attr			= NULL;
	
	pobject_prop->begin	= 0U;
	pobject_prop->end	= 0U;
	pobject_prop->x		= 0U;
	pobject_prop->y		= 0U;
	pobject_prop->z		= 0U;
	pobject_prop->w		= 0U;
	pobject_prop->h		= 0U;
	pobject_prop->p1	= 0U;
	pobject_prop->p2	= 0U;
	pobject_prop->p3	= 0U;
	pobject_prop->const_p1	= 0U;
	pobject_prop->const_p2	= 0U;
	pobject_prop->const_p3	= 0U;
	pobject_prop->const_p4	= 0U;
	pobject_prop->const_p5	= 0U;
	pobject_prop->const_p6	= 0U;
	pobject_prop->id1		= 0U;
	pobject_prop->id2		= 0U;
	pobject_prop->id3		= 0U;
	pobject_prop->attr		= 0U;
	
	pobject_prop->pcustom_data	= NULL;
	pobject_prop->pbeg_end		= NULL;
	pobject_prop->phead_tail	= NULL;
	#if HMI_DXY_CUSTOM_CNT>0
	if(HMI_IS_DYN_XY_CUSTOM(custom_id))
	{
		//hmi_object_index = HMI_GET_DXY_CUSTOM_INDEX(custom_id);
		if(custom_id > HMI_SXY_SPLINE_MAX_ID)
		{
			hmi_object_index = HMI_GET_DXY_CUSTOM_INDEX(custom_id);
		}
		else
		{
			hmi_object_index = 0U;
		}
		get_success 	 = hmi_engine_get_dyn_custom_index(hmi_object_index,&custom_lib_index);
		if(get_success == TRUE)
		{
			phmi_custom_widget_info =&hmi_dxy_custom_widget_info[custom_lib_index];
		}
	}
	else
	#endif
	#if HMI_SXY_CUSTOM_CNT>0
	if(HMI_IS_CUSTOM_SXY(custom_id))
	{
		hmi_object_index = HMI_GET_CUSTOM_SXY_ID_INDEX(custom_id);
		get_success 	 = hmi_engine_get_static_custom_index(hmi_object_index,&custom_lib_index);
		if(get_success ==TRUE)
		{
			phmi_custom_widget_info =&hmi_sxy_custom_widget_info[custom_lib_index];
		}
	}
	else
	#endif
	{
	}
	if(phmi_custom_widget_info !=NULL)
	{
		hmi_object_index -=phmi_custom_widget_info->begin;
		pcustom_attr = &(phmi_custom_widget_info->attr_fun);

		pobject_prop->begin	= phmi_custom_widget_info->begin;
		pobject_prop->end	= phmi_custom_widget_info->end;
		
		if((pcustom_attr->attr & HMI_ID1) !=0)
		{
			pobject_prop->id1 = phmi_custom_widget_info->pelement_ID1[hmi_object_index];
		}
		if((pcustom_attr->attr & HMI_ID2) !=0)
		{
			pobject_prop->id2 = phmi_custom_widget_info->pelement_ID2[hmi_object_index];
		}
		if((pcustom_attr->attr & HMI_ID3) !=0)
		{
			pobject_prop->id3 = phmi_custom_widget_info->pelement_ID3[hmi_object_index];
		}
		if((pcustom_attr->attr & HMI_XYZWH_F) == 0u)
		{
			pobject_prop->x = ((HMI_CUSTOM_XYZWH_STR *)(phmi_custom_widget_info->pxyzwh))[hmi_object_index].x;
			pobject_prop->y	= ((HMI_CUSTOM_XYZWH_STR *)(phmi_custom_widget_info->pxyzwh))[hmi_object_index].y;
			pobject_prop->z	= ((HMI_CUSTOM_XYZWH_STR *)(phmi_custom_widget_info->pxyzwh))[hmi_object_index].z;
			pobject_prop->w = ((HMI_CUSTOM_XYZWH_STR *)(phmi_custom_widget_info->pxyzwh))[hmi_object_index].w;
			pobject_prop->h = ((HMI_CUSTOM_XYZWH_STR *)(phmi_custom_widget_info->pxyzwh))[hmi_object_index].h;
		}
		else	/*float*/
		{			
			pobject_prop->x = (HMI_CUSTOM_P_INT )(HMI_F32_TO_U32(((HMI_CUSTOM_XYZWH_FLOAT_STR *)(phmi_custom_widget_info->pxyzwh))[hmi_object_index].x));			
			pobject_prop->y	= (HMI_CUSTOM_P_INT )(HMI_F32_TO_U32(((HMI_CUSTOM_XYZWH_FLOAT_STR *)(phmi_custom_widget_info->pxyzwh))[hmi_object_index].y));			
			pobject_prop->z	= (HMI_CUSTOM_P_INT )(HMI_F32_TO_U32(((HMI_CUSTOM_XYZWH_FLOAT_STR *)(phmi_custom_widget_info->pxyzwh))[hmi_object_index].z));			
			pobject_prop->w = (HMI_CUSTOM_P_INT )(HMI_F32_TO_U32(((HMI_CUSTOM_XYZWH_FLOAT_STR *)(phmi_custom_widget_info->pxyzwh))[hmi_object_index].w));			
			pobject_prop->h = (HMI_CUSTOM_P_INT )(HMI_F32_TO_U32(((HMI_CUSTOM_XYZWH_FLOAT_STR *)(phmi_custom_widget_info->pxyzwh))[hmi_object_index].h));
		}

		if((pcustom_attr->attr & HMI_P1) !=0)
		{	
			if((pcustom_attr->attr & HMI_P1_F) !=0)
			{
				pobject_prop->p1	= (HMI_CUSTOM_P_INT )(HMI_F32_TO_U32(((HMI_CUSTOM_P_FLOAT *)(phmi_custom_widget_info->pp1))[hmi_object_index]));
			}
			else
			{
				pobject_prop->p1 	= (((HMI_CUSTOM_P_INT *)(phmi_custom_widget_info->pp1))[hmi_object_index]);
			}
			
		}
		if((pcustom_attr->attr & HMI_P2) !=0)
		{
			if((pcustom_attr->attr & HMI_P2_F) !=0)
			{
				pobject_prop->p2 		= (HMI_CUSTOM_P_INT )(HMI_F32_TO_U32(((HMI_CUSTOM_P_FLOAT *)(phmi_custom_widget_info->pp2))[hmi_object_index]));
			}
			else
			{
				pobject_prop->p2 		= (((HMI_CUSTOM_P_INT *)(phmi_custom_widget_info->pp2))[hmi_object_index]);
			}
		}
		if((pcustom_attr->attr & HMI_P3) !=0)
		{
			if((pcustom_attr->attr & HMI_P3_F) !=0)
			{
				pobject_prop->p3 		= (HMI_CUSTOM_P_INT )(HMI_F32_TO_U32(((HMI_CUSTOM_P_FLOAT *)(phmi_custom_widget_info->pp3))[hmi_object_index]));
			}
			else
			{
				pobject_prop->p3 		= ((HMI_CUSTOM_P_INT *)(phmi_custom_widget_info->pp3))[hmi_object_index];
			}
		}
		if((pcustom_attr->attr & HMI_CON_P1) !=0)
		{
			if((pcustom_attr->attr & HMI_CON_F1) !=0)
			{
				pobject_prop->const_p1	= (HMI_CUSTOM_P_INT )(HMI_F32_TO_U32(((HMI_CUSTOM_P_FLOAT *)(phmi_custom_widget_info->pconst_p1))[hmi_object_index]));
			}
			else
			{
				pobject_prop->const_p1	= ((HMI_CUSTOM_P_INT *)(phmi_custom_widget_info->pconst_p1))[hmi_object_index];
			}
		}
		
		if((pcustom_attr->attr & HMI_CON_P2) !=0)
		{
			if((pcustom_attr->attr & HMI_CON_F2) !=0)
			{
				pobject_prop->const_p2	= (HMI_CUSTOM_P_INT )(HMI_F32_TO_U32(((HMI_CUSTOM_P_FLOAT *)(phmi_custom_widget_info->pconst_p2))[hmi_object_index]));
			}
			else
			{
				pobject_prop->const_p2	= ((HMI_CUSTOM_P_INT *)(phmi_custom_widget_info->pconst_p2))[hmi_object_index];
			}
		}
		
		if((pcustom_attr->attr & HMI_CON_P3) !=0)
		{
			if((pcustom_attr->attr & HMI_CON_F3) !=0)
			{
				pobject_prop->const_p3	= (HMI_CUSTOM_P_INT )(HMI_F32_TO_U32(((HMI_CUSTOM_P_FLOAT *)(phmi_custom_widget_info->pconst_p3))[hmi_object_index]));
			}
			else
			{
				pobject_prop->const_p3	= ((HMI_CUSTOM_P_INT *)(phmi_custom_widget_info->pconst_p3))[hmi_object_index];
			}
		}

		if((pcustom_attr->attr & HMI_CON_P4) !=0)
		{
			if((pcustom_attr->attr & HMI_CON_F4) !=0)
			{
				pobject_prop->const_p4	= (HMI_CUSTOM_P_INT )(HMI_F32_TO_U32(((HMI_CUSTOM_P_FLOAT *)(phmi_custom_widget_info->pconst_p4))[hmi_object_index]));
			}
			else
			{
				pobject_prop->const_p4	= ((HMI_CUSTOM_P_INT *)(phmi_custom_widget_info->pconst_p4))[hmi_object_index];
			}
		}

		if((pcustom_attr->attr & HMI_CON_P5) !=0)
		{
			if((pcustom_attr->attr & HMI_CON_F5) !=0)
			{
				pobject_prop->const_p5	= (HMI_CUSTOM_P_INT )(HMI_F32_TO_U32(((HMI_CUSTOM_P_FLOAT *)(phmi_custom_widget_info->pconst_p5))[hmi_object_index]));
			}
			else
			{
				pobject_prop->const_p5	= ((HMI_CUSTOM_P_INT *)(phmi_custom_widget_info->pconst_p5))[hmi_object_index];
			}
		}

		if((pcustom_attr->attr & HMI_CON_P6) !=0)
		{
			if((pcustom_attr->attr & HMI_CON_F6) !=0)
			{
				pobject_prop->const_p6	= (HMI_CUSTOM_P_INT )(HMI_F32_TO_U32(((HMI_CUSTOM_P_FLOAT *)(phmi_custom_widget_info->pconst_p6))[hmi_object_index]));
			}
			else
			{
				pobject_prop->const_p6	= ((HMI_CUSTOM_P_INT *)(phmi_custom_widget_info->pconst_p6))[hmi_object_index];
			}
		}

		pobject_prop->attr =pcustom_attr->attr;
		
#if 0	// 2024 08 19 lq
		pobject_prop->pcustom_data	= phmi_custom_widget_info->pcustom_data;
		pobject_prop->pbeg_end		= phmi_custom_widget_info->pbeg_end;
		pobject_prop->phead_tail	= phmi_custom_widget_info->phead_tail;
#else	
		if ((pobject_prop->attr & HMI_FIFO_EN) != 0u)
		{
			pobject_prop->pcustom_data	= phmi_custom_widget_info->pcustom_data;
			pobject_prop->pbeg_end		= &(phmi_custom_widget_info->pbeg_end[hmi_object_index]);
			pobject_prop->phead_tail	= &(phmi_custom_widget_info->phead_tail[hmi_object_index]);

			if(pobject_prop->pbeg_end != NULL)			
			{
				if ((pobject_prop->attr & HMI_FIFO_F) != 0u)
				{
					pobject_prop->pcustom_data	= (void *)(((float_32 *)phmi_custom_widget_info->pcustom_data) +
												pobject_prop->pbeg_end[0].head_index);
				}
				else
				{
					pobject_prop->pcustom_data	= (void *)(((INT32 *)phmi_custom_widget_info->pcustom_data) +
												pobject_prop->pbeg_end[0].head_index);
				}
			}
		}
#endif
	}

	return get_success;
}
#endif
HMI_CONTAINER_STR CONST* hmi_engine_get_container_child_addr(HMI_OBJECT_ID_STR hmi_container_id)
{
	HMI_OBJECT_ID_STR	hmi_container_index	= 0U;
	HMI_CONTAINER_STR CONST *pcontainer_table	= NULL;

	
	#if HMI_DYN_CONTAINERS_NUMBER > 0
	if(HMI_IS_DYN_CONTAINER(hmi_container_id))
	{			
		#if HMI_ALL_STATIC_CONTAINERS_NUMBER > 0  
		hmi_container_id  = HMI_GET_DYN_CONTAINERS_ID_INDEX(hmi_container_id);
		hmi_engine_get_static_container_id(&hmi_container_id);/*get static container ID*/					
		#endif
	}
	#endif
	#if HMI_DXY_CONTAINERS_NUMBER>0U
	if(HMI_IS_DYN_XY_CONTAINER(hmi_container_id))
	{
		hmi_container_index	= HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_container_id);
		pcontainer_table	= &hmi_dyn_xy_container_table[hmi_container_index];
	}
	else
	#endif
	#if HMI_SXY_CONTAINERS_NUMBER>0
	if(HMI_IS_CONTAINERS_SXY(hmi_container_id))
	{
		hmi_container_index = HMI_GET_CONTAINERS_SXY_ID_INDEX(hmi_container_id);
		pcontainer_table	= &hmi_sxy_container_table[hmi_container_index];
	}
	else
	#endif
	{
	}
	return pcontainer_table;
}

void hmi_engine_get_object_prop2(HMI_OBJECT_ID_STR hmi_object_id,
										HMI_ELEMENT_PROP2_STR *pobject_prop)
{
	HMI_OBJECT_ID_STR hmi_object_index	= 0;
	#if HMI_DXY_ROTATION_BITMAPS_NUMBER > 0U 
	HMI_OBJECT_ID_STR hmi_object_index2	= 0U;
	#endif
	float_32		  image_angel		= 0;
	
	if(pobject_prop !=NULL)
	{
		pobject_prop->trail_enable= FALSE;
		pobject_prop->rotation.x= 0U;
		pobject_prop->rotation.y= 0U;
		pobject_prop->rotation_center = TRUE;
		pobject_prop->img_compress.image_attr = 0U;
		#if defined( HMI_MCU_S6J3200 )||defined( HMI_MCU_RT1170 )|| defined(HMI_MCU_RT1172 )
		pobject_prop->img_compress.pixel_fmt = 0U;
		#endif
		pobject_prop->trail_id		=	HMI_NB_ELEMENTS;
		pobject_prop->trail_CCW 	=	0;

		/*he2021-03-03*/
		#if HMI_DXY_BITMAPS_NUMBER > 0
		if(HMI_IS_DYN_XY_BITMAP(hmi_object_id))
		{	  
			hmi_object_index = HMI_GET_DYN_XY_BITMAP_INDEX(hmi_object_id );
			#if HMI_DXY_CENTER_BITMAPS_NUMBER > 0
			if(HMI_IS_DXY_CENTER_SINGEL_TEXTURE_BITAMP(hmi_object_id))
			{	
				image_angel		= hmi_bmp_dyn_xy_rect[hmi_object_index].angel;
				pobject_prop->trail_enable= FALSE;
				if(fabs(image_angel) > HMI_FLOAT_TOLERANCE)
				{
					pobject_prop->rotation_center = TRUE;
				}
				else
				{
					pobject_prop->rotation_center = FALSE;	
				}
				
				pobject_prop->img_compress.image_attr = hmi_dxy_bitmap_attr_table[hmi_object_index].image_attr;
				#if defined( HMI_MCU_S6J3200 )||defined( HMI_MCU_RT1170 )||defined( HMI_MCU_AMT630H )|| defined(HMI_MCU_RT1172 )
				pobject_prop->img_compress.pixel_fmt =  hmi_dxy_bitmap_attr_table[hmi_object_index].pixel_fmt;
				#endif
				pobject_prop->pimage_prop_info = &hmi_bmp_dyn_xy_prop_table[hmi_object_index];
				pobject_prop->rotation.x= 0U;
				pobject_prop->rotation.y= 0U;
			}
			else 
			#endif
			{
			#if HMI_DXY_ROTATION_BITMAPS_NUMBER > 0 
				if(hmi_object_index > HMI_DXY_CENTER_BITMAPS_NUMBER)
				{
					hmi_object_index2	= hmi_object_index-HMI_DXY_CENTER_BITMAPS_NUMBER;
				}
				else
				{
					hmi_object_index2	= 0U;
				}
				if(hmi_object_index2 < HMI_DXY_ROTATION_BITMAPS_NUMBER)
				{
				pobject_prop->trail_enable= hmi_dxy_bitmap_rotation_trail[hmi_object_index2].trail_en;
				pobject_prop->rotation_center = FALSE;
				pobject_prop->img_compress.image_attr = hmi_dxy_bitmap_attr_table[hmi_object_index].image_attr;
				#if defined( HMI_MCU_S6J3200 )||defined( HMI_MCU_RT1170 )||defined( HMI_MCU_AMT630H )|| defined(HMI_MCU_RT1172 )
				pobject_prop->img_compress.pixel_fmt =  hmi_dxy_bitmap_attr_table[hmi_object_index].pixel_fmt;
				#endif
				pobject_prop->pimage_prop_info = &hmi_bmp_dyn_xy_prop_table[hmi_object_index];
				pobject_prop->rotation.x= hmi_dxy_bitmap_rotation[hmi_object_index2].x;
				pobject_prop->rotation.y= hmi_dxy_bitmap_rotation[hmi_object_index2].y;
				pobject_prop->trail_id		=	hmi_dxy_bitmap_rotation_trail[hmi_object_index2].texture;
				pobject_prop->trail_CCW 	=	hmi_dxy_bitmap_rotation_trail_attr[hmi_object_index2];
				}
			#endif
			}
			
		}
		else
		#endif
		#if HMI_SXY_BITMAPS_NUMBER> 0
		if(HMI_IS_S_XY_BITMAP(hmi_object_id))
		{
			hmi_object_index  = HMI_GET_SXY_BITMAPS_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_SXY_BITMAPS_NUMBER)
			{
				image_angel		= hmi_bmp_static_xy_rect[hmi_object_index].angel;
				pobject_prop->trail_enable= FALSE;
				if(fabs(image_angel) > HMI_FLOAT_TOLERANCE)
				{
					pobject_prop->rotation_center = TRUE;
				}
				else
				{
					pobject_prop->rotation_center = FALSE;	
				}
				pobject_prop->img_compress.image_attr = hmi_sxy_bitmap_attr_table[hmi_object_index].image_attr;
				#if defined( HMI_MCU_S6J3200 )||defined( HMI_MCU_RT1170 )||defined( HMI_MCU_AMT630H )|| defined(HMI_MCU_RT1172 )
				pobject_prop->img_compress.pixel_fmt =  hmi_sxy_bitmap_attr_table[hmi_object_index].pixel_fmt;
				#endif
				pobject_prop->pimage_prop_info = &hmi_bmp_static_xy_prop_table[hmi_object_index];
				pobject_prop->rotation.x= 0U;
				pobject_prop->rotation.y= 0U;
			}
		}
		else
		#endif
		{
		}
		}
		
}


void hmi_engine_get_object_prop(HMI_OBJECT_ID_STR hmi_object_id,
										HMI_ELEMENT_PROP_STR *pobject_prop)
{
	HMI_OBJECT_ID_STR hmi_object_index= 0;
	if(pobject_prop !=NULL)
	{
		pobject_prop->x = 0U;
		pobject_prop->y = 0U;
		pobject_prop->z	= 0U;
		pobject_prop->w = 0U;
		pobject_prop->h = 0U;
		pobject_prop->c1	= 0U;
		pobject_prop->c2	= 0U;
		pobject_prop->alpha	= 255U;
		pobject_prop->angle	= 0.0f;
		pobject_prop->scale	= 1.0f;
		
		#if HMI_DYN_CONTAINERS_NUMBER > 0
		if(HMI_IS_DYN_CONTAINER(hmi_object_id))
		{			
			#if HMI_ALL_STATIC_CONTAINERS_NUMBER > 0  
			hmi_object_id  = HMI_GET_DYN_CONTAINERS_ID_INDEX(hmi_object_id);
			hmi_engine_get_static_container_id(&hmi_object_id);/*get static container ID*/					
			#endif
		}
		#endif

		#if HMI_DXY_IMAGELIST_NUMBER > 0
		if(HMI_IS_DYN_IMAGELIST_DXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_IMAGELIST_DXY_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DXY_IMAGELIST_NUMBER)
			{
				pobject_prop->x	= hmi_dxy_imagelist_rect[hmi_object_index].x;
				pobject_prop->y	= hmi_dxy_imagelist_rect[hmi_object_index].y; 
				pobject_prop->w	= hmi_dxy_imagelist_rect[hmi_object_index].w;
				pobject_prop->h	= hmi_dxy_imagelist_rect[hmi_object_index].h;
				pobject_prop->alpha	= hmi_dxy_imagelist_rect[hmi_object_index].alpha;
			}
		}
		else
		#endif
#if HMI_SXY_IMAGELIST_NUMBER>0
		if(HMI_IS_DYN_IMAGELIST_SXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_IMAGELIST_SXY_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_SXY_IMAGELIST_NUMBER)
			{
				pobject_prop->x	= hmi_sxy_imagelist_rect[hmi_object_index].x;
				pobject_prop->y	= hmi_sxy_imagelist_rect[hmi_object_index].y; 
				pobject_prop->w	= hmi_sxy_imagelist_rect[hmi_object_index].w;
				pobject_prop->h	= hmi_sxy_imagelist_rect[hmi_object_index].h;
				pobject_prop->alpha	= hmi_sxy_imagelist_rect[hmi_object_index].alpha;
			}
		}
		else
#endif
#if HMI_DXY_SCROLLBAR_NUMBER>0
		if(HMI_IS_DYN_SCROLLBAR_DXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_SCROLLBAR_DXY_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DXY_SCROLLBAR_NUMBER)
			{
				pobject_prop->x	= hmi_dxy_scrollbar_rect[hmi_object_index].x;
				pobject_prop->y	= hmi_dxy_scrollbar_rect[hmi_object_index].y; 
				pobject_prop->w	= hmi_dxy_scrollbar_rect[hmi_object_index].w;
				pobject_prop->h	= hmi_dxy_scrollbar_rect[hmi_object_index].h;
				pobject_prop->alpha	= hmi_dxy_scrollbar_rect[hmi_object_index].alpha;
			}
		}
		else
#endif
#if HMI_SXY_SCROLLBAR_NUMBER>0
		if(HMI_IS_DYN_SCROLLBAR_SXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_SCROLLBAR_SXY_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_SXY_SCROLLBAR_NUMBER)
			{
				pobject_prop->x	= hmi_sxy_scrollbar_rect[hmi_object_index].x;
				pobject_prop->y	= hmi_sxy_scrollbar_rect[hmi_object_index].y; 
				pobject_prop->w	= hmi_sxy_scrollbar_rect[hmi_object_index].w;
				pobject_prop->h	= hmi_sxy_scrollbar_rect[hmi_object_index].h;
				pobject_prop->alpha	= hmi_sxy_scrollbar_rect[hmi_object_index].alpha;
			}
		}
		else
#endif
#if HMI_DXY_BUTTON_NUMBER>0
		if(HMI_IS_DYN_BUTTON_DXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_BUTTON_DXY_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DXY_BUTTON_NUMBER)
			{	
				pobject_prop->x	= hmi_dxy_button_rect[hmi_object_index].x;
				pobject_prop->y	= hmi_dxy_button_rect[hmi_object_index].y; 
				pobject_prop->w	= hmi_dxy_button_rect[hmi_object_index].w;
				pobject_prop->h	= hmi_dxy_button_rect[hmi_object_index].h;
				pobject_prop->alpha	= hmi_dxy_button_rect[hmi_object_index].alpha;
			}
		}
		else
#endif
#if HMI_SXY_BUTTON_NUMBER>0
		if(HMI_IS_DYN_BUTTON_SXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_BUTTON_SXY_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_SXY_BUTTON_NUMBER)
			{
				pobject_prop->x	= hmi_sxy_button_rect[hmi_object_index].x;
				pobject_prop->y	= hmi_sxy_button_rect[hmi_object_index].y; 
				pobject_prop->w	= hmi_sxy_button_rect[hmi_object_index].w;
				pobject_prop->h	= hmi_sxy_button_rect[hmi_object_index].h;
				pobject_prop->alpha	= hmi_sxy_button_rect[hmi_object_index].alpha;
			}
		}
		else
#endif
#if HMI_DYN_EDIT_TEXTS_NUMBER/*editable text*/ > 0 
		if(HMI_IS_DYN_TEXTS(hmi_object_id))
		{		
			#if HMI_DYN_XY_EDIT_TEXTS_NUMBER > 0
			if(HMI_IS_DYN_TEXT_DXY(hmi_object_id))
			{
				hmi_object_index = HMI_GET_DYN_TEXTS_DXY_POS_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_DYN_XY_EDIT_TEXTS_NUMBER)
				{
					pobject_prop->x	= (hmi_dyn_xy_edit_text_prop_table[hmi_object_index].text_rect.x);
					pobject_prop->y	= (hmi_dyn_xy_edit_text_prop_table[hmi_object_index].text_rect.y); 
					pobject_prop->w	= (hmi_dyn_xy_edit_text_prop_table[hmi_object_index].text_rect.w);
					pobject_prop->h	= (hmi_dyn_xy_edit_text_prop_table[hmi_object_index].text_rect.h);
					pobject_prop->c1= (hmi_dyn_xy_edit_text_prop_table[hmi_object_index].color);

				}
			}
			else
			#endif
			#if HMI_S_XY_EDIT_TEXTS_NUMBER > 0
			if(HMI_IS_DYN_TEXT_SXY(hmi_object_id))
			{
				pobject_prop->x	= (hmi_static_xy_edit_text_prop_table[hmi_object_index].text_rect.x);
				pobject_prop->y	= (hmi_static_xy_edit_text_prop_table[hmi_object_index].text_rect.y); 
				pobject_prop->w	= (hmi_static_xy_edit_text_prop_table[hmi_object_index].text_rect.w);
				pobject_prop->h	= (hmi_static_xy_edit_text_prop_table[hmi_object_index].text_rect.h);
				pobject_prop->c1= (hmi_static_xy_edit_text_prop_table[hmi_object_index].color);
			}
			else
			#endif
			{
			}						
		}
		else
		#endif
		#if HMI_DYN_CONTAINERS_NUMBER/*dyn container*/ > 0 
		if(HMI_IS_DYN_CONTAINER(hmi_object_id))
		{
		}
		else
		#endif
		#if HMI_DYN_FILL_PAGES_NUMBER > 0
		if(HMI_IS_DYN_NFILL(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_NFILL_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DYN_FILL_PAGES_NUMBER)
			{
				pobject_prop->x	= hmi_fills_dyn_xy_rect[hmi_object_index].x;
				pobject_prop->y	= hmi_fills_dyn_xy_rect[hmi_object_index].y; 
				pobject_prop->w	= hmi_fills_dyn_xy_rect[hmi_object_index].w;
				pobject_prop->h	= hmi_fills_dyn_xy_rect[hmi_object_index].h;
				pobject_prop->z	= hmi_fills_dyn_xy_rect[hmi_object_index].z;
				pobject_prop->c1= hmi_fills_dyn_prop_table[hmi_object_index].color;
			}
		}
		else
		#endif
		#if HMI_DYN_GFILL_NUMBER > 0
		if(HMI_IS_DYN_GFILL(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_GFILL_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DYN_GFILL_NUMBER)
			{
				pobject_prop->x	= hmi_gradient_dxy_fill_rect[hmi_object_index].x;
				pobject_prop->y	= hmi_gradient_dxy_fill_rect[hmi_object_index].y; 
				pobject_prop->w	= hmi_gradient_dxy_fill_rect[hmi_object_index].w;
				pobject_prop->h	= hmi_gradient_dxy_fill_rect[hmi_object_index].h;
				pobject_prop->z	= hmi_gradient_dxy_fill_rect[hmi_object_index].z;
				pobject_prop->c1= hmi_gradient_dxy_fill_table[hmi_object_index].color1;
				pobject_prop->c2= hmi_gradient_dxy_fill_table[hmi_object_index].color2;
			}
		}
		else
#endif
#if HMI_DXY_CUBE_NUMBER > 0
		if(HMI_IS_DYN_CUBE(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_CUBE_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DXY_CUBE_NUMBER)
			{	
				pobject_prop->x	= hmi_cubes_dyn_xy_rect[hmi_object_index].cube_rect.x;
				pobject_prop->y	= hmi_cubes_dyn_xy_rect[hmi_object_index].cube_rect.y; 
				pobject_prop->w	= hmi_cubes_dyn_xy_rect[hmi_object_index].cube_rect.w;
				pobject_prop->h	= hmi_cubes_dyn_xy_rect[hmi_object_index].cube_rect.h;
				pobject_prop->z	= hmi_cubes_dyn_xy_rect[hmi_object_index].z;
				pobject_prop->scale= hmi_cubes_dyn_xy_rect[hmi_object_index].scale;
				pobject_prop->angle= hmi_cubes_dyn_xy_rect[hmi_object_index].private_angel;
			}
		}
		else 
#endif

#if HMI_DXY_3DCUBE_NUMBER > 0
		if(HMI_IS_DYN_3DCUBE(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_3DCUBE_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DXY_3DCUBE_NUMBER)
			{
				pobject_prop->x	= hmi_3dcubes_dyn_xy_rect[hmi_object_index].cube_rect.x;
				pobject_prop->y	= hmi_3dcubes_dyn_xy_rect[hmi_object_index].cube_rect.y; 
				pobject_prop->w	= hmi_3dcubes_dyn_xy_rect[hmi_object_index].cube_rect.w;
				pobject_prop->h	= hmi_3dcubes_dyn_xy_rect[hmi_object_index].cube_rect.h;
				pobject_prop->z	= hmi_3dcubes_dyn_xy_rect[hmi_object_index].z;
				pobject_prop->scale= hmi_3dcubes_dyn_xy_rect[hmi_object_index].scale;
				pobject_prop->angle= hmi_3dcubes_dyn_xy_rect[hmi_object_index].private_angel;
			}
		}
		else 
#endif
#if HMI_DXY_CONTAINERS_NUMBER > 0U 
		if(HMI_IS_DYN_XY_CONTAINER(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DXY_CONTAINERS_NUMBER)
			{
				pobject_prop->x	= hmi_dyn_xy_container_rect[hmi_object_index].x;
				pobject_prop->y	= hmi_dyn_xy_container_rect[hmi_object_index].y; 
				pobject_prop->w	= hmi_dyn_xy_container_rect[hmi_object_index].w;
				pobject_prop->h	= hmi_dyn_xy_container_rect[hmi_object_index].h;
				#if HMI_DXY_CONTAINERS_SCALE_ALPHA_NUMBER >0U
				if(hmi_object_index < HMI_DXY_CONTAINERS_SCALE_ALPHA_NUMBER)
				{
					pobject_prop->scale= hmi_dyn_xy_container_alpha_scale[hmi_object_index].scale;
					pobject_prop->alpha= hmi_dyn_xy_container_alpha_scale[hmi_object_index].alpha;
				}
				#endif
			}
		}
		else
#endif
#if HMI_DXY_BITMAPS_NUMBER > 0
		if(HMI_IS_DYN_XY_BITMAP(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_XY_BITMAP_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DXY_BITMAPS_NUMBER)
			{
				pobject_prop->x	= hmi_bmp_dyn_xy_rect[hmi_object_index].x;
				pobject_prop->y	= hmi_bmp_dyn_xy_rect[hmi_object_index].y; 
				pobject_prop->w	= hmi_bmp_dyn_xy_rect[hmi_object_index].w;
				pobject_prop->h	= hmi_bmp_dyn_xy_rect[hmi_object_index].h;
				pobject_prop->alpha	= hmi_bmp_dyn_xy_rect[hmi_object_index].alpha;
				pobject_prop->angle	= hmi_bmp_dyn_xy_rect[hmi_object_index].angel;
			}
		}
		else
#endif
#if HMI_DXY_SPLINE_NUMBER > 0
		if(HMI_IS_DXY_SPLINE(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DXY_SPLINE_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DXY_SPLINE_NUMBER)
			{	
				pobject_prop->x	= hmi_dxy_spline_rect[hmi_object_index].x;
				pobject_prop->y	= hmi_dxy_spline_rect[hmi_object_index].y; 
				pobject_prop->w	= hmi_dxy_spline_rect[hmi_object_index].w;
				pobject_prop->h	= hmi_dxy_spline_rect[hmi_object_index].h;
				pobject_prop->scale= hmi_dxy_spline_scale[hmi_object_index];
			}
		}
		else 
#endif
#if HMI_SXY_SPLINE_NUMBER > 0		
		if(HMI_IS_SXY_SPLINE(hmi_object_id))
		{
			hmi_object_index = HMI_GET_SXY_SPLINE_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_SXY_SPLINE_NUMBER)
			{
				pobject_prop->x	= hmi_sxy_spline_rect[hmi_object_index].x;
				pobject_prop->y	= hmi_sxy_spline_rect[hmi_object_index].y; 
				pobject_prop->w	= hmi_sxy_spline_rect[hmi_object_index].w;
				pobject_prop->h	= hmi_sxy_spline_rect[hmi_object_index].h;
			}
		}
		else 
#endif
#if HMI_DXY_CUSTOM_CNT > 0
		if(HMI_IS_DYN_XY_CUSTOM(hmi_object_id))
		{
			//hmi_object_index = HMI_GET_DXY_CUSTOM_INDEX(hmi_object_id);
			if(hmi_object_id > HMI_SXY_SPLINE_MAX_ID)
			{
				hmi_object_index = HMI_GET_DXY_CUSTOM_INDEX(hmi_object_id);
			}
			else
			{
				hmi_object_index = 0U;
			}
		}
		else
#endif
#if HMI_STATIC_TEXTS_NUMBER  > 0 
		if(HMI_IS_STATIC_TEXTS(hmi_object_id)/*unedit text*/)
		{
			#if HMI_UNEDIT_TEXTS_DYN_XY_NUMBER>0
			if(HMI_IS_UNEDIT_TEXT_DYN_XY(hmi_object_id))
			{
				hmi_object_index  = HMI_GET_UNEDIT_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_UNEDIT_TEXTS_DYN_XY_NUMBER)
				{
					pobject_prop->x	= (hmi_dyn_xy_unedit_text_prop_table[hmi_object_index].text_rect.x);
					pobject_prop->y	= (hmi_dyn_xy_unedit_text_prop_table[hmi_object_index].text_rect.y); 
					pobject_prop->w	= (hmi_dyn_xy_unedit_text_prop_table[hmi_object_index].text_rect.w);
					pobject_prop->h	= (hmi_dyn_xy_unedit_text_prop_table[hmi_object_index].text_rect.h);
					pobject_prop->c1= (hmi_dyn_xy_unedit_text_prop_table[hmi_object_index].color);
				}
			}
			else
			#endif
			#if HMI_UNEDIT_TEXTS_STATIC_XY_NUMBER > 0
			if(HMI_IS_UNEDIT_TEXT_STATIC_XY(hmi_object_id))
			{
				hmi_object_index = HMI_GET_S_XY_UNEDIT_POS_COLOR_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_UNEDIT_TEXTS_STATIC_XY_NUMBER)
				{
					pobject_prop->x	= (hmi_static_xy_unedit_text_prop_table[hmi_object_index].text_rect.x);
					pobject_prop->y	= (hmi_static_xy_unedit_text_prop_table[hmi_object_index].text_rect.y); 
					pobject_prop->w	= (hmi_static_xy_unedit_text_prop_table[hmi_object_index].text_rect.w);
					pobject_prop->h	= (hmi_static_xy_unedit_text_prop_table[hmi_object_index].text_rect.h);
					pobject_prop->c1= (hmi_static_xy_unedit_text_prop_table[hmi_object_index].color);
				}
			}
			else
			#endif
			{
			}						
		}
		else	
#endif
#if HMI_STATIC_FILL_PAGES_NUMBER > 0
		if(HMI_IS_SXY_FILL_PAGES(hmi_object_id))
		{
			hmi_object_index  = HMI_GET_SXY_FILL_PAGES_ID_INDEX(hmi_object_id );
			if(hmi_object_index < HMI_STATIC_FILL_PAGES_NUMBER)
			{
				pobject_prop->x	= hmi_fills_static_xy_rect[hmi_object_index].x;
				pobject_prop->y	= hmi_fills_static_xy_rect[hmi_object_index].y;
				pobject_prop->z = hmi_fills_static_xy_rect[hmi_object_index].z;
				pobject_prop->w	= hmi_fills_static_xy_rect[hmi_object_index].w;
				pobject_prop->h	= hmi_fills_static_xy_rect[hmi_object_index].h;
				pobject_prop->c1= hmi_fills_static_prop_table[hmi_object_index].color;
			}
		}
		else
#endif
#if HMI_STATIC_GFILL_NUMBER> 0
		if(HMI_IS_SXY_GFILL_PAGES(hmi_object_id))
		{
			hmi_object_index = HMI_GET_SXY_GFILL_PAGES_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_STATIC_GFILL_NUMBER)
			{
				pobject_prop->x	= hmi_gradient_sxy_fill_rect[hmi_object_index].x;
				pobject_prop->y	= hmi_gradient_sxy_fill_rect[hmi_object_index].y;
				pobject_prop->z = hmi_gradient_sxy_fill_rect[hmi_object_index].z;
				pobject_prop->w	= hmi_gradient_sxy_fill_rect[hmi_object_index].w;
				pobject_prop->h	= hmi_gradient_sxy_fill_rect[hmi_object_index].h;
				pobject_prop->c1= hmi_gradient_sxy_fill_table[hmi_object_index].color1;	
				pobject_prop->c2= hmi_gradient_sxy_fill_table[hmi_object_index].color2;
			}
		}
		else
#endif
#if HMI_SXY_CUBE_NUMBER> 0
		if(HMI_IS_SXY_CUBE(hmi_object_id))
		{
			hmi_object_index = HMI_GET_SXY_CUBE_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_SXY_CUBE_NUMBER)
			{
				pobject_prop->x	= hmi_cubes_static_xy_rect[hmi_object_index].cube_rect.x;
				pobject_prop->y	= hmi_cubes_static_xy_rect[hmi_object_index].cube_rect.y;
				pobject_prop->z = hmi_cubes_static_xy_rect[hmi_object_index].z;
				pobject_prop->w	= hmi_cubes_static_xy_rect[hmi_object_index].cube_rect.w;
				pobject_prop->h	= hmi_cubes_static_xy_rect[hmi_object_index].cube_rect.h;
				pobject_prop->angle	= hmi_cubes_static_xy_rect[hmi_object_index].angel;	
				pobject_prop->scale	= hmi_cubes_static_xy_rect[hmi_object_index].scale;
			}
		}
		else
#endif
#if HMI_SXY_3DCUBE_NUMBER> 0
		if(HMI_IS_SXY_3DCUBE(hmi_object_id))
		{
			hmi_object_index = HMI_GET_SXY_3DCUBE_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_SXY_3DCUBE_NUMBER)
			{
				pobject_prop->x	= hmi_3dcubes_static_xy_rect[hmi_object_index].cube_rect.x;
				pobject_prop->y	= hmi_3dcubes_static_xy_rect[hmi_object_index].cube_rect.y;
				pobject_prop->z = hmi_3dcubes_static_xy_rect[hmi_object_index].z;
				pobject_prop->w	= hmi_3dcubes_static_xy_rect[hmi_object_index].cube_rect.w;
				pobject_prop->h	= hmi_3dcubes_static_xy_rect[hmi_object_index].cube_rect.h;
				pobject_prop->angle	= hmi_3dcubes_static_xy_rect[hmi_object_index].angel;	
				pobject_prop->scale	= hmi_3dcubes_static_xy_rect[hmi_object_index].scale;
			}
		}
		else
		#endif
		#if HMI_SXY_BITMAPS_NUMBER> 0
		if(HMI_IS_S_XY_BITMAP(hmi_object_id))
		{
			hmi_object_index  = HMI_GET_SXY_BITMAPS_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_SXY_BITMAPS_NUMBER)
			{
				pobject_prop->x	= hmi_bmp_static_xy_rect[hmi_object_index].x;
				pobject_prop->y	= hmi_bmp_static_xy_rect[hmi_object_index].y; 
				pobject_prop->w	= hmi_bmp_static_xy_rect[hmi_object_index].w;
				pobject_prop->h	= hmi_bmp_static_xy_rect[hmi_object_index].h;
				pobject_prop->alpha	= hmi_bmp_static_xy_rect[hmi_object_index].alpha;
				pobject_prop->angle	= hmi_bmp_static_xy_rect[hmi_object_index].angel;
			}
		}
		else
		#endif
		#if HMI_SXY_CONTAINERS_NUMBER > 0
		if(HMI_IS_CONTAINERS_SXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_CONTAINERS_SXY_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_SXY_CONTAINERS_NUMBER)
			{
				pobject_prop->x	= hmi_static_container_rect[hmi_object_index].x;
				pobject_prop->y	= hmi_static_container_rect[hmi_object_index].y; 
				pobject_prop->w	= hmi_static_container_rect[hmi_object_index].w;
				pobject_prop->h	= hmi_static_container_rect[hmi_object_index].h;
			}
		}
		else
		#endif
		{
		}
		}
}

#if HMI_DXY_SPLINE_NUMBER +\
	HMI_SXY_SPLINE_NUMBER+\
	HMI_DXY_ROTATION_BITMAPS_NUMBER+\
	HMI_DXY_CUBE_NUMBER +\
	HMI_SXY_CUBE_NUMBER+\
	HMI_DXY_3DCUBE_NUMBER +\
	HMI_SXY_3DCUBE_NUMBER > 0U

static void hmi_get_object_wh(HMI_OBJECT_ID_STR hmi_object_id,
										HMI_WIDTH_STR *pwidth,
										HMI_HEIGHT_STR *pheight)
{
	HMI_OBJECT_ID_STR hmi_object_index= 0;
	
#if HMI_DYN_CONTAINERS_NUMBER > 0
	if(HMI_IS_DYN_CONTAINER(hmi_object_id))
	{			
		#if HMI_ALL_STATIC_CONTAINERS_NUMBER > 0  
		hmi_object_id  = HMI_GET_DYN_CONTAINERS_ID_INDEX(hmi_object_id);
		hmi_engine_get_static_container_id(&hmi_object_id);/*get static container ID*/					
		#endif
	}
#endif

#if HMI_DXY_IMAGELIST_NUMBER > 0
	if(HMI_IS_DYN_IMAGELIST_DXY(hmi_object_id))
	{
		hmi_object_index = HMI_GET_DYN_IMAGELIST_DXY_ID_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_DXY_IMAGELIST_NUMBER)
		{
			*pwidth =	hmi_dxy_imagelist_rect[hmi_object_index].w;
			*pheight =	hmi_dxy_imagelist_rect[hmi_object_index].h;	
		}
	}
	else
#endif
#if HMI_SXY_IMAGELIST_NUMBER>0
	if(HMI_IS_DYN_IMAGELIST_SXY(hmi_object_id))
	{
		hmi_object_index = HMI_GET_DYN_IMAGELIST_SXY_ID_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_SXY_IMAGELIST_NUMBER)
		{
			*pwidth	=	hmi_sxy_imagelist_rect[hmi_object_index].w;
			*pheight	=	hmi_sxy_imagelist_rect[hmi_object_index].h;
		}
	}
	else
#endif
#if HMI_DXY_SCROLLBAR_NUMBER>0
	if(HMI_IS_DYN_SCROLLBAR_DXY(hmi_object_id))
	{
		hmi_object_index = HMI_GET_DYN_SCROLLBAR_DXY_ID_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_DXY_SCROLLBAR_NUMBER)
		{
			*pwidth =	hmi_dxy_scrollbar_rect[hmi_object_index].w;
			*pheight =	hmi_dxy_scrollbar_rect[hmi_object_index].h;
		}
	}
	else
#endif
#if HMI_SXY_SCROLLBAR_NUMBER>0
	if(HMI_IS_DYN_SCROLLBAR_SXY(hmi_object_id))
	{
		hmi_object_index = HMI_GET_DYN_SCROLLBAR_SXY_ID_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_SXY_SCROLLBAR_NUMBER)
		{
			*pwidth =	hmi_sxy_scrollbar_rect[hmi_object_index].w;
			*pheight =	hmi_sxy_scrollbar_rect[hmi_object_index].h;
		}
	}
	else
#endif
#if HMI_DXY_BUTTON_NUMBER>0
	if(HMI_IS_DYN_BUTTON_DXY(hmi_object_id))
	{
		hmi_object_index = HMI_GET_DYN_BUTTON_DXY_ID_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_DXY_BUTTON_NUMBER)
		{	
			*pwidth =	hmi_dxy_button_rect[hmi_object_index].w;
			*pheight =	hmi_dxy_button_rect[hmi_object_index].h;
		}
	}
	else
#endif
#if HMI_SXY_BUTTON_NUMBER>0
	if(HMI_IS_DYN_BUTTON_SXY(hmi_object_id))
	{
		hmi_object_index = HMI_GET_DYN_BUTTON_SXY_ID_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_SXY_BUTTON_NUMBER)
		{
			*pwidth =	hmi_sxy_button_rect[hmi_object_index].w;
			*pheight =	hmi_sxy_button_rect[hmi_object_index].h;
		}
	}
	else
#endif
#if HMI_DYN_EDIT_TEXTS_NUMBER/*editable text*/ > 0 
	if(HMI_IS_DYN_TEXTS(hmi_object_id))
	{		
		#if HMI_DYN_XY_EDIT_TEXTS_NUMBER > 0
		if(HMI_IS_DYN_TEXT_DXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_TEXTS_DXY_POS_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DYN_XY_EDIT_TEXTS_NUMBER)
			{
				*pwidth =(hmi_dyn_xy_edit_text_prop_table[hmi_object_index].text_rect.w);
				*pheight =(hmi_dyn_xy_edit_text_prop_table[hmi_object_index].text_rect.h);

			}
		}
		else
		#endif
		#if HMI_S_XY_EDIT_TEXTS_NUMBER > 0
		if(HMI_IS_DYN_TEXT_SXY(hmi_object_id))
		{
			*pwidth =(hmi_static_xy_edit_text_prop_table[hmi_object_index].text_rect.w);
			*pheight =(hmi_static_xy_edit_text_prop_table[hmi_object_index].text_rect.h);
		}
		else
		#endif
		{
		}						
	}
	else
#endif
#if HMI_DYN_CONTAINERS_NUMBER/*dyn container*/ > 0 
	if(HMI_IS_DYN_CONTAINER(hmi_object_id))
	{
	}
	else
#endif
#if HMI_DYN_FILL_PAGES_NUMBER > 0
	if(HMI_IS_DYN_NFILL(hmi_object_id))
	{
		hmi_object_index = HMI_GET_DYN_NFILL_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_DYN_FILL_PAGES_NUMBER)
		{
			*pwidth =	hmi_fills_dyn_xy_rect[hmi_object_index].w;
			*pheight =	hmi_fills_dyn_xy_rect[hmi_object_index].h;
		}
	}
	else
#endif
#if HMI_DYN_GFILL_NUMBER > 0
	if(HMI_IS_DYN_GFILL(hmi_object_id))
	{
		hmi_object_index = HMI_GET_DYN_GFILL_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_DYN_GFILL_NUMBER)
		{
			*pwidth =	hmi_gradient_dxy_fill_rect[hmi_object_index].w;
			*pheight =	hmi_gradient_dxy_fill_rect[hmi_object_index].h;
		}
	}
	else
#endif
#if HMI_DXY_CUBE_NUMBER > 0
	if(HMI_IS_DYN_CUBE(hmi_object_id))
	{
		hmi_object_index = HMI_GET_DYN_CUBE_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_DXY_CUBE_NUMBER)
		{	
			*pwidth =	hmi_cubes_dyn_xy_rect[hmi_object_index].cube_rect.w;
			*pheight =	hmi_cubes_dyn_xy_rect[hmi_object_index].cube_rect.h;
		}
	}
	else 
#endif

#if HMI_DXY_3DCUBE_NUMBER > 0
	if(HMI_IS_DYN_3DCUBE(hmi_object_id))
	{
		hmi_object_index = HMI_GET_DYN_3DCUBE_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_DXY_3DCUBE_NUMBER)
		{
			*pwidth =	hmi_3dcubes_dyn_xy_rect[hmi_object_index].cube_rect.w;
			*pheight =	hmi_3dcubes_dyn_xy_rect[hmi_object_index].cube_rect.h;
		}
	}
	else 
#endif
#if HMI_DXY_CONTAINERS_NUMBER > 0U 
	if(HMI_IS_DYN_XY_CONTAINER(hmi_object_id))
	{
		hmi_object_index = HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_DXY_CONTAINERS_NUMBER)
		{
			*pwidth =	hmi_dyn_xy_container_rect[hmi_object_index].w;
			*pheight =	hmi_dyn_xy_container_rect[hmi_object_index].h;
		}
	}
	else
#endif
#if HMI_DXY_BITMAPS_NUMBER > 0
	if(HMI_IS_DYN_XY_BITMAP(hmi_object_id))
	{
		hmi_object_index = HMI_GET_DYN_XY_BITMAP_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_DXY_BITMAPS_NUMBER)
		{
			*pwidth =	hmi_bmp_dyn_xy_rect[hmi_object_index].w;
			*pheight =	hmi_bmp_dyn_xy_rect[hmi_object_index].h;
		}
	}
	else
#endif
#if HMI_DXY_SPLINE_NUMBER > 0
	if(HMI_IS_DXY_SPLINE(hmi_object_id))
	{
		hmi_object_index = HMI_GET_DXY_SPLINE_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_DXY_SPLINE_NUMBER)
		{	
			*pwidth =	hmi_dxy_spline_rect[hmi_object_index].w;
			*pheight =	hmi_dxy_spline_rect[hmi_object_index].h; 
		}
	}
	else 
#endif
#if HMI_SXY_SPLINE_NUMBER > 0		
	if(HMI_IS_SXY_SPLINE(hmi_object_id))
	{
		hmi_object_index = HMI_GET_SXY_SPLINE_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_SXY_SPLINE_NUMBER)
		{
			*pwidth =	hmi_sxy_spline_rect[hmi_object_index].w;
			*pheight =	hmi_sxy_spline_rect[hmi_object_index].h;
		}
	}
	else 
#endif
#if HMI_DXY_CUSTOM_CNT > 0
	if(HMI_IS_DYN_XY_CUSTOM(hmi_object_id))
	{
		if(hmi_object_id > HMI_SXY_SPLINE_MAX_ID)
		{
			hmi_object_index = HMI_GET_DXY_CUSTOM_INDEX(hmi_object_id);
		}
		else
		{
			hmi_object_index = 0U;
		}
	}
	else
#endif
#if HMI_STATIC_TEXTS_NUMBER  > 0 
	if(HMI_IS_STATIC_TEXTS(hmi_object_id)/*unedit text*/)
	{
		#if HMI_UNEDIT_TEXTS_DYN_XY_NUMBER>0
		if(HMI_IS_UNEDIT_TEXT_DYN_XY(hmi_object_id))
		{
			hmi_object_index  = HMI_GET_UNEDIT_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_UNEDIT_TEXTS_DYN_XY_NUMBER)
			{
				*pwidth =	(hmi_dyn_xy_unedit_text_prop_table[hmi_object_index].text_rect).w;
				*pheight =	(hmi_dyn_xy_unedit_text_prop_table[hmi_object_index].text_rect).h;
			}
		}
		else
		#endif
		#if HMI_UNEDIT_TEXTS_STATIC_XY_NUMBER > 0
		if(HMI_IS_UNEDIT_TEXT_STATIC_XY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_S_XY_UNEDIT_POS_COLOR_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_UNEDIT_TEXTS_STATIC_XY_NUMBER)
			{
				*pwidth =	(hmi_static_xy_unedit_text_prop_table[hmi_object_index].text_rect).w;
				*pheight =	(hmi_static_xy_unedit_text_prop_table[hmi_object_index].text_rect).h;
			}
		}
		else
		#endif
		{
		}						
	}
	else	
#endif
#if HMI_STATIC_FILL_PAGES_NUMBER > 0
	if(HMI_IS_SXY_FILL_PAGES(hmi_object_id))
	{
		hmi_object_index  = HMI_GET_SXY_FILL_PAGES_ID_INDEX(hmi_object_id );
		if(hmi_object_index < HMI_STATIC_FILL_PAGES_NUMBER)
		{
			*pwidth =	hmi_fills_static_xy_rect[hmi_object_index].w;
			*pheight =	hmi_fills_static_xy_rect[hmi_object_index].h;
		}
	}
	else
#endif
#if HMI_STATIC_GFILL_NUMBER> 0
	if(HMI_IS_SXY_GFILL_PAGES(hmi_object_id))
	{
		hmi_object_index = HMI_GET_SXY_GFILL_PAGES_ID_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_STATIC_GFILL_NUMBER)
		{
			*pwidth =	hmi_gradient_sxy_fill_rect[hmi_object_index].w;
			*pheight =	hmi_gradient_sxy_fill_rect[hmi_object_index].h;
		}
	}
	else
#endif
#if HMI_SXY_CUBE_NUMBER> 0
	if(HMI_IS_SXY_CUBE(hmi_object_id))
	{
		hmi_object_index = HMI_GET_SXY_CUBE_ID_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_SXY_CUBE_NUMBER)
		{
			*pwidth =	hmi_cubes_static_xy_rect[hmi_object_index].cube_rect.w;
			*pheight =	hmi_cubes_static_xy_rect[hmi_object_index].cube_rect.h;
		}
	}
	else
#endif
#if HMI_SXY_3DCUBE_NUMBER> 0
	if(HMI_IS_SXY_3DCUBE(hmi_object_id))
	{
		hmi_object_index = HMI_GET_SXY_3DCUBE_ID_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_SXY_3DCUBE_NUMBER)
		{
			*pwidth =	hmi_3dcubes_static_xy_rect[hmi_object_index].cube_rect.w;
			*pheight =	hmi_3dcubes_static_xy_rect[hmi_object_index].cube_rect.h;
		}
	}
	else
#endif
#if HMI_SXY_BITMAPS_NUMBER> 0
	if(HMI_IS_S_XY_BITMAP(hmi_object_id))
	{
		hmi_object_index  = HMI_GET_SXY_BITMAPS_ID_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_SXY_BITMAPS_NUMBER)
		{
			*pwidth =	hmi_bmp_static_xy_rect[hmi_object_index].w;
			*pheight =	hmi_bmp_static_xy_rect[hmi_object_index].h;							
		}
	}
	else
#endif
#if HMI_SXY_CONTAINERS_NUMBER > 0
	if(HMI_IS_CONTAINERS_SXY(hmi_object_id))
	{
		hmi_object_index = HMI_GET_CONTAINERS_SXY_ID_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_SXY_CONTAINERS_NUMBER)
		{
			*pwidth =	hmi_static_container_rect[hmi_object_index].w;
			*pheight =	hmi_static_container_rect[hmi_object_index].h;
		}
	}
	else
#endif
	{
	}
}
#endif

#if HMI_DXY_SPLINE_NUMBER + HMI_SXY_SPLINE_NUMBER	+ HMI_DXY_CUSTOM_CNT + HMI_SXY_CUSTOM_CNT> 0
BOOLEAN hmi_spline_get_id_prop(HMI_OBJECT_ID_STR		hmi_object_id,
								HMI_SPLINE_COLOR_STR*   pspline_color
								)
{
	HMI_OBJECT_ID_STR			hmi_object_index		= 0;
	BOOLEAN						hmi_container_loop_stop = FALSE;
	BOOLEAN						hmi_get_success			= FALSE;
	UINT8						hmi_loop				= 0;

	hmi_get_success	 = FALSE;
	if((hmi_object_id  >= HMI_PAGE_SXY_MAX_ID)&&(pspline_color != NULL))/*not a page*/
	{
		pspline_color->fill_flag		= HMI_FILL_NONE;
		while((hmi_container_loop_stop == FALSE)&&(hmi_loop < 255))
		{
			hmi_loop++;
			#if HMI_DYN_CONTAINERS_NUMBER > 0
			if(HMI_IS_DYN_CONTAINER(hmi_object_id))
			{			
			#if HMI_ALL_STATIC_CONTAINERS_NUMBER > 0  
				hmi_object_id  = HMI_GET_DYN_CONTAINERS_ID_INDEX(hmi_object_id);
				hmi_engine_get_static_container_id(&hmi_object_id);/*get static container ID*/
			#endif
			}
			#endif		
			#if HMI_DXY_CONTAINERS_NUMBER > 0U  		
			if(HMI_IS_DYN_XY_CONTAINER(hmi_object_id))
			{
				hmi_object_index = HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_object_id );
				if(hmi_object_index < HMI_DXY_CONTAINERS_NUMBER)
				{
					if(hmi_dyn_xy_container_table[hmi_object_index].container_object_table.object_number > 0)
					{
						hmi_object_id	= hmi_dyn_xy_container_table[hmi_object_index].container_object_table.p_object_table[0].object_id;/*get container first child to disp*/
					}
					else
					{
						hmi_object_id			= HMI_NB_ELEMENTS;
						hmi_container_loop_stop	= TRUE;
					}
				}
			}
			else
			#endif 
			#if HMI_SXY_CONTAINERS_NUMBER > 0 
			if(HMI_IS_CONTAINERS_SXY(hmi_object_id))
			{
				hmi_object_index = HMI_GET_CONTAINERS_SXY_ID_INDEX(hmi_object_id );
				if(hmi_object_index<HMI_SXY_CONTAINERS_NUMBER)
				{
					if(hmi_sxy_container_table[hmi_object_index].container_object_table.object_number > 0)
					{
						hmi_object_id	= hmi_sxy_container_table[hmi_object_index].container_object_table.p_object_table[0].object_id;/*get container first child to disp*/
					}
					else
					{
						hmi_object_id			= HMI_NB_ELEMENTS;
						hmi_container_loop_stop	= TRUE;
					}
				}
			}
			else
			#endif
			{
				hmi_container_loop_stop	= TRUE;
			}
		}
		#if HMI_DYN_FILL_PAGES_NUMBER > 0 
		if(HMI_IS_DYN_NFILL(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_NFILL_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DYN_FILL_PAGES_NUMBER)
			{
				pspline_color->fill_color = hmi_fills_dyn_prop_table[hmi_object_index].color;
				pspline_color->fill_flag	= HMI_COLOR;
				hmi_get_success	= TRUE;
			}
		}
		else
		#endif
		#if HMI_DXY_BITMAPS_NUMBER > 0 
		if(HMI_IS_DYN_XY_BITMAP(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_XY_BITMAP_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DXY_BITMAPS_NUMBER)
			{
				pspline_color->spline_texture.ptex_rect	= (HMI_RECT_ALPHA_ANGEL_STR CONST	*)(&hmi_bmp_dyn_xy_rect[hmi_object_index]);
				pspline_color->spline_texture.ptex_prop	= &hmi_bmp_dyn_xy_prop_table[hmi_object_index];
				pspline_color->spline_texture.ptex_attr	= &hmi_dxy_bitmap_attr_table[hmi_object_index];
				pspline_color->spline_texture.tex_id	= hmi_object_id;
				pspline_color->fill_flag				= HMI_TEXTURE;
				hmi_get_success	= TRUE;
			}
		}
		else
		#endif
		#if HMI_STATIC_FILL_PAGES_NUMBER > 0 
		if(HMI_IS_SXY_FILL_PAGES(hmi_object_id))
		{
			hmi_object_index = HMI_GET_SXY_FILL_PAGES_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_STATIC_FILL_PAGES_NUMBER)
			{
				pspline_color->fill_color 		= hmi_fills_static_prop_table[hmi_object_index].color;
				pspline_color->fill_flag		= HMI_COLOR;
				hmi_get_success	= TRUE;
			}
		}
		else
		#endif
		#if HMI_SXY_BITMAPS_NUMBER > 0
		if(HMI_IS_S_XY_BITMAP(hmi_object_id))
		{			 
			hmi_object_index = HMI_GET_SXY_BITMAPS_ID_INDEX(hmi_object_id );
			if(hmi_object_index < HMI_SXY_BITMAPS_NUMBER)
			{	
				pspline_color->spline_texture.ptex_rect	= &hmi_bmp_static_xy_rect[hmi_object_index];
				pspline_color->spline_texture.ptex_prop	= &hmi_bmp_static_xy_prop_table[hmi_object_index];
				pspline_color->spline_texture.ptex_attr	= &hmi_sxy_bitmap_attr_table[hmi_object_index];
				pspline_color->spline_texture.tex_id	= hmi_object_id;
				pspline_color->fill_flag				= HMI_TEXTURE;
				hmi_get_success	= TRUE;
			}
		}
		else
		#endif
		{
			pspline_color->fill_flag		= HMI_FILL_NONE;
			hmi_get_success = FALSE;
		}
	}
	return hmi_get_success;
}
#endif

#if((HMI_SXY_CUBE_NUMBER+				\
	HMI_DXY_CUBE_NUMBER+				\
	HMI_DXY_ROTATION_BITMAPS_NUMBER+	\
	HMI_DXY_CUSTOM_CNT+					\
	HMI_SXY_CUSTOM_CNT)> 0)
	#if (defined(HMI_GRAPHIC_OPENGLES))

GLuint		hmi_draw_container_render_buffer(HMI_OBJECT_ID_STR	hmi_object_id,UINT32 render_w,UINT32 render_h)
{	
	GLuint					mask_texture			= 0;
	HMI_CONTAINER_STR CONST *phmi_container_table	= NULL;
	HMI_RECT_STR			container_zone			= {0,0,0,0};	
	HMI_ELEMENT_PROP_STR	object_prop 			= {0,0,0,0,0,0,0,0,0,0};
	HMI_ALPHA_SCALE_PT_STR	hmi_container_scale_alpha= {255u,1.0f,{0,0}};
	UINT8					pre_cur_disp			= 0;
#if defined(HMI_SIMULATOR_QD)
	HMI_RECT_STR			father_container_zone	= {0,0,0,0};
#endif
	
	phmi_container_table	= hmi_engine_get_container_child_addr(hmi_object_id);
	if(phmi_container_table != NULL)
	{
		hmi_engine_get_object_prop(hmi_object_id,&object_prop);
		pre_cur_disp	= hmi_driver_get_render_screen();
		hmi_switch_render_buffer(HMI_CUBE_RENDER_BUFFER,
								render_w,
								render_h,
								GL_RGBA,
								HMI_PHYSICAL_SCREEN_NO + HMI_CUBE_RENDER_BUFFER);		
		
		/*convert screen coordinate*/
		container_zone.x	= 0;
		container_zone.y	= 0;
		container_zone.w	= (HMI_WIDTH_STR)render_w;
		container_zone.h	= (HMI_HEIGHT_STR)render_h;
	#if defined(HMI_SIMULATOR_QD)		
		father_container_zone.w	= render_w;
		father_container_zone.h = render_h;
		
		hmi_engine_draw_container(phmi_container_table,
					(HMI_RECT_STR CONST *)(&father_container_zone),
					&container_zone,
					0/*no used*/,		
					hmi_object_id,		
					&container_zone,/*all father insection zone*/
					&hmi_container_scale_alpha);
	#else
		hmi_engine_draw_container(phmi_container_table,
					(HMI_RECT_STR CONST *)(&container_zone),
					&container_zone,
					0/*no used*/,					
					&container_zone,/*all father insection zone*/
					&hmi_container_scale_alpha);
	#endif
		
#ifdef _DEBUG
		
#endif	
		/*restory current disp*/
		hmi_driver_set_render_screen(pre_cur_disp);
		set_ortho_matrix2(0/*used old value*/,0/*used old value*/); 	
				
		/*switch to previous Render buffer*/				
		restory_previous_frambuffer(HMI_CUBE_RENDER_BUFFER);
		mask_texture	= hmi_render_buffer_qd[HMI_CUBE_RENDER_BUFFER].texture; 			
	}

	return	mask_texture;
}
	
GLuint		hmi_draw_container_render_buffer3(HMI_OBJECT_ID_STR	hmi_object_id,UINT32 render_w,UINT32 render_h,UINT32 clear_color)
{	
	GLuint					mask_texture			= 0;
	HMI_CONTAINER_STR CONST *phmi_container_table	= NULL;
	HMI_RECT_STR			container_zone			= {0,0,0,0};	
	HMI_ELEMENT_PROP_STR	object_prop 			= {0,0,0,0,0,0,0,0,0,0};
	HMI_ALPHA_SCALE_PT_STR	hmi_container_scale_alpha= {255u,1.0f,{0,0}};
	UINT8					pre_cur_disp			= 0;
#if defined(HMI_SIMULATOR_QD)
	HMI_RECT_STR			father_container_zone	= {0,0,0,0};
#endif
	
	phmi_container_table	= hmi_engine_get_container_child_addr(hmi_object_id);
	if(phmi_container_table != NULL)
	{
		hmi_engine_get_object_prop(hmi_object_id,&object_prop);
		pre_cur_disp	= hmi_driver_get_render_screen();
		hmi_switch_render_buffer3(HMI_CUBE_RENDER_BUFFER,
								render_w,
								render_h,
								GL_RGBA,
								HMI_PHYSICAL_SCREEN_NO + HMI_CUBE_RENDER_BUFFER,
								clear_color);		
		
		/*convert screen coordinate*/
		container_zone.x	= 0;
		container_zone.y	= 0;
		container_zone.w	= (HMI_WIDTH_STR)render_w;
		container_zone.h	= (HMI_HEIGHT_STR)render_h;
#if defined(HMI_SIMULATOR_QD)		
		father_container_zone.w = render_w;
		father_container_zone.h = render_h;
		
		hmi_engine_draw_container(phmi_container_table,
					(HMI_RECT_STR CONST *)(&father_container_zone),
					&container_zone,
					0/*no used*/,		
					hmi_object_id,		
					&container_zone,/*all father insection zone*/
					&hmi_container_scale_alpha);
#else
		hmi_engine_draw_container(phmi_container_table,
					(HMI_RECT_STR CONST *)(&container_zone),
					&container_zone,
					0/*no used*/,					
					&container_zone,/*all father insection zone*/
					&hmi_container_scale_alpha);
#endif
		
#ifdef _DEBUG
		
#endif	
		/*restory current disp*/
		hmi_driver_set_render_screen(pre_cur_disp);
		set_ortho_matrix2(0/*used old value*/,0/*used old value*/); 	
				
		/*switch to previous Render buffer*/				
		restory_previous_frambuffer(HMI_CUBE_RENDER_BUFFER);
		mask_texture	= hmi_render_buffer_qd[HMI_CUBE_RENDER_BUFFER].texture; 			
	}

	return	mask_texture;
}

#endif	

static void hmi_cube_get_container_prop(HMI_OBJECT_ID_STR		hmi_object_id,
								HMI_CUBE_TEXTURE_PROP	*pcube_texture,
								HMI_CUBE_TEXTURE_PROP	*pbump_texture,
								SPOINT_TP				*pfather_point)	
{
	#if ((HMI_DYN_CONTAINERS_NUMBER > 0)||	\
		(HMI_DXY_CONTAINERS_NUMBER > 0)||	\
		(HMI_SXY_CONTAINERS_NUMBER > 0))
	HMI_OBJECT_ID_STR			hmi_object_index	= 0;
	HMI_RECT_STR				father_zone			= {0,0,0u,0u};
	HMI_ALPHA_SCALE_PT_STR		father_alpha_scale	= {255u,1.0f,{0,0}};
	#endif

	#if (defined(HMI_GRAPHIC_OPENGLES))
	GLuint					mask_texture			= 0;
	#endif
	
	if(hmi_object_id  >= HMI_PAGE_SXY_MAX_ID)/*not a page*/
	{	 	
		#if HMI_DYN_CONTAINERS_NUMBER > 0
		if(HMI_IS_DYN_CONTAINER(hmi_object_id))
		{		  	
			#if HMI_ALL_STATIC_CONTAINERS_NUMBER > 0  
			hmi_object_id  = HMI_GET_DYN_CONTAINERS_ID_INDEX(hmi_object_id);
		  	hmi_engine_get_static_container_id(&hmi_object_id);/*get static container ID*/
						
			#if HMI_DXY_CONTAINERS_NUMBER>0U
			if(HMI_IS_DYN_XY_CONTAINER(hmi_object_id))
			{
				 hmi_object_index= HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_object_id );
				if(hmi_object_index< HMI_DXY_CONTAINERS_NUMBER)
				{
					if(pfather_point != NULL)
					{
						pfather_point->x += hmi_dyn_xy_container_rect[hmi_object_index].x;
						pfather_point->y += hmi_dyn_xy_container_rect[hmi_object_index].y;
					}
				
				
					hmi_cube_get_container_prop(hmi_object_id,
											pcube_texture,
											pbump_texture,
											pfather_point);
				
				}
			}
			else
			#endif
			{
			}
			#if HMI_SXY_CONTAINERS_NUMBER>0
			if(HMI_IS_CONTAINERS_SXY(hmi_object_id))
			{
				hmi_object_index= HMI_GET_CONTAINERS_SXY_ID_INDEX(hmi_object_id );
				if(hmi_object_index< HMI_SXY_CONTAINERS_NUMBER)
				{
					if(pfather_point != NULL)
					{
						pfather_point->x += hmi_static_container_rect[hmi_object_index].x;
						pfather_point->y += hmi_static_container_rect[hmi_object_index].y;
					}
				
					hmi_cube_get_container_prop(hmi_object_id,
											pcube_texture,
											pbump_texture,
											pfather_point);					
				}
			}
			else
			#endif
			{
			}	
			#endif
		}
		else
		#endif
		#if HMI_DXY_CONTAINERS_NUMBER > 0U  		
		if(HMI_IS_DYN_XY_CONTAINER(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_object_id );
			if(hmi_object_index < HMI_DXY_CONTAINERS_NUMBER)
			{
				if(pfather_point != NULL)
				{
					pfather_point->x += hmi_dyn_xy_container_rect[hmi_object_index].x;
					pfather_point->y += hmi_dyn_xy_container_rect[hmi_object_index].y;
				}
			#if (defined(HMI_GRAPHIC_OPENGLES))
				mask_texture	= hmi_draw_container_render_buffer(hmi_object_id,
												hmi_dyn_xy_container_rect[hmi_object_index].w,
												hmi_dyn_xy_container_rect[hmi_object_index].h);
											
				pcube_texture->tex_id							= 0;
				pcube_texture->tex_prop.pbitmap_data.w			= 0;/*texure  valide*/
				pcube_texture->tex_prop.pbitmap_data.h			= 0;/*texure  valide*/
				pcube_texture->tex_prop.pbitmap_data.fmt_index	= (HMI_BIG_IMAGE_INDEX_STR)mask_texture;/*may be error.texture is int32*/
				
			#else
				if(hmi_dyn_xy_container_table[hmi_object_index].container_object_table.object_number > 0)
				{
					hmi_object_id	= hmi_dyn_xy_container_table[hmi_object_index].container_object_table.p_object_table[0].object_id;/*get container first child to disp*/
					hmi_cube_get_id_prop(hmi_object_id,pcube_texture,pfather_point);
				}
				if(hmi_dyn_xy_container_table[hmi_object_index].container_object_table.object_number > 1)
				{
					hmi_object_id	= hmi_dyn_xy_container_table[hmi_object_index].container_object_table.p_object_table[1].object_id;/*get container first child to disp*/
					hmi_cube_get_id_prop(hmi_object_id,pbump_texture,pfather_point);
				}
			#endif
			}
		}
		else
		#endif 
		#if HMI_SXY_CONTAINERS_NUMBER > 0 
		if(HMI_IS_CONTAINERS_SXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_CONTAINERS_SXY_ID_INDEX(hmi_object_id );
			if(hmi_object_index < HMI_SXY_CONTAINERS_NUMBER)
			{
				if(pfather_point != NULL)
				{
					pfather_point->x += hmi_static_container_rect[hmi_object_index].x;
					pfather_point->y += hmi_static_container_rect[hmi_object_index].y;
				}
			#if (defined(HMI_GRAPHIC_OPENGLES))				
				/*For container,draw container at  render buffer*/								
				mask_texture	= hmi_draw_container_render_buffer(hmi_object_id,
												hmi_static_container_rect[hmi_object_index].w,
												hmi_static_container_rect[hmi_object_index].h);
											
				pcube_texture->tex_id							= 0;
				pcube_texture->tex_prop.pbitmap_data.w			= 0;/*texure  valide*/
				pcube_texture->tex_prop.pbitmap_data.h			= 0;/*texure  valide*/
				pcube_texture->tex_prop.pbitmap_data.fmt_index	= (HMI_BIG_IMAGE_INDEX_STR)mask_texture;/*may be error.texture is int32*/
			#else			
				if(hmi_sxy_container_table[hmi_object_index].container_object_table.object_number > 0)
				{
					hmi_object_id		= hmi_sxy_container_table[hmi_object_index].container_object_table.p_object_table[0].object_id;/*get container first child to disp*/
					hmi_cube_get_id_prop(hmi_object_id,pcube_texture,pfather_point);
				}
				if(hmi_sxy_container_table[hmi_object_index].container_object_table.object_number > 1)
				{
					hmi_object_id=hmi_sxy_container_table[hmi_object_index].container_object_table.p_object_table[1].object_id;/*get container first child to disp*/
					hmi_cube_get_id_prop(hmi_object_id,pbump_texture,pfather_point);
				}
			#endif	
			}
		}
		else
		#endif	   
		{
			hmi_cube_get_id_prop(hmi_object_id,pcube_texture,pfather_point);
		}	   	   
	}
}

/*
pfather_point is relevation coordinate.
*/
static void hmi_cube_get_container_prop2(HMI_OBJECT_ID_STR		hmi_object_id,
								HMI_CUBE_TEXTURE_PROP	*pcube_texture,
								HMI_CUBE_TEXTURE_PROP	*pbump_texture,
								SPOINT_TP				*pfather_point)	
{
	#if ((HMI_DYN_CONTAINERS_NUMBER > 0)||	\
		(HMI_DXY_CONTAINERS_NUMBER > 0)||	\
		(HMI_SXY_CONTAINERS_NUMBER > 0))
	HMI_OBJECT_ID_STR			hmi_object_index	= 0;	
	#endif
	if(hmi_object_id  >= HMI_PAGE_SXY_MAX_ID)/*not a page*/
	{	 	
		#if HMI_DYN_CONTAINERS_NUMBER > 0
		if(HMI_IS_DYN_CONTAINER(hmi_object_id))
		{		  	
			#if HMI_ALL_STATIC_CONTAINERS_NUMBER > 0  
			hmi_object_id  = HMI_GET_DYN_CONTAINERS_ID_INDEX(hmi_object_id);
		  	hmi_engine_get_static_container_id(&hmi_object_id);/*get static container ID*/
						
			#if HMI_DXY_CONTAINERS_NUMBER>0U
			if(HMI_IS_DYN_XY_CONTAINER(hmi_object_id))
			{
				 hmi_object_index= HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_object_id );
				if(hmi_object_index< HMI_DXY_CONTAINERS_NUMBER)
				{
					if(pfather_point != NULL)
					{
						/*not include container x,y*/
						pfather_point->x = -hmi_dyn_xy_container_rect[hmi_object_index].x;
						pfather_point->y = -hmi_dyn_xy_container_rect[hmi_object_index].y;
					}
					hmi_cube_get_container_prop(hmi_object_id,
											pcube_texture,
											pbump_texture,
											pfather_point);
				}
			}
			else
			#endif
			{
			}
			#if HMI_SXY_CONTAINERS_NUMBER>0
			if(HMI_IS_CONTAINERS_SXY(hmi_object_id))
			{
				hmi_object_index= HMI_GET_CONTAINERS_SXY_ID_INDEX(hmi_object_id );
				if(hmi_object_index< HMI_SXY_CONTAINERS_NUMBER)
				{
					if(pfather_point != NULL)
					{
						/*not include container x,y*/
						pfather_point->x = -hmi_static_container_rect[hmi_object_index].x;
						pfather_point->y = -hmi_static_container_rect[hmi_object_index].y;
					}
					hmi_cube_get_container_prop(hmi_object_id,
											pcube_texture,
											pbump_texture,
											pfather_point);				
				}
			}
			else
			#endif
			{
			}	
			#endif
		}
		else
		#endif
		#if HMI_DXY_CONTAINERS_NUMBER > 0U  		
		if(HMI_IS_DYN_XY_CONTAINER(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_object_id );
			if(hmi_object_index < HMI_DXY_CONTAINERS_NUMBER)
			{				
				if(hmi_dyn_xy_container_table[hmi_object_index].container_object_table.object_number > 0)
				{
					hmi_object_id	= hmi_dyn_xy_container_table[hmi_object_index].container_object_table.p_object_table[0].object_id;/*get container first child to disp*/
					hmi_cube_get_id_prop(hmi_object_id,pcube_texture,pfather_point);
				}
				if(hmi_dyn_xy_container_table[hmi_object_index].container_object_table.object_number > 1)
				{
					hmi_object_id	= hmi_dyn_xy_container_table[hmi_object_index].container_object_table.p_object_table[1].object_id;/*get container first child to disp*/
					hmi_cube_get_id_prop(hmi_object_id,pbump_texture,pfather_point);
				}
			}
		}
		else
		#endif 
		#if HMI_SXY_CONTAINERS_NUMBER > 0 
		if(HMI_IS_CONTAINERS_SXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_CONTAINERS_SXY_ID_INDEX(hmi_object_id );
			if(hmi_object_index<HMI_SXY_CONTAINERS_NUMBER)
			{				
				if(hmi_sxy_container_table[hmi_object_index].container_object_table.object_number > 0)
				{
					hmi_object_id	= hmi_sxy_container_table[hmi_object_index].container_object_table.p_object_table[0].object_id;/*get container first child to disp*/
					hmi_cube_get_id_prop(hmi_object_id,pcube_texture,pfather_point);
				}
				if(hmi_sxy_container_table[hmi_object_index].container_object_table.object_number > 1)
				{
					hmi_object_id	= hmi_sxy_container_table[hmi_object_index].container_object_table.p_object_table[1].object_id;/*get container first child to disp*/
					hmi_cube_get_id_prop(hmi_object_id,pbump_texture,pfather_point);
				}
				
			}
		}
		else
		#endif	   
		{
			hmi_cube_get_id_prop2(hmi_object_id,pcube_texture,pfather_point);
		}	   	   
	}
}

static void hmi_cube_get_container_prop3(HMI_OBJECT_ID_STR		hmi_object_id,
								HMI_CUBE_TEXTURE_PROP	*pcube_texture,
								HMI_CUBE_TEXTURE_PROP	*pbump_texture,
								SPOINT_TP				*pfather_point)	
{
	#if ((HMI_DYN_CONTAINERS_NUMBER > 0)||	\
		(HMI_DXY_CONTAINERS_NUMBER > 0)||	\
		(HMI_SXY_CONTAINERS_NUMBER > 0))
	HMI_OBJECT_ID_STR			hmi_object_index	= 0;
	HMI_RECT_STR				father_zone			= {0,0,0u,0u};
	HMI_ALPHA_SCALE_PT_STR		father_alpha_scale	= {255u,1.0f,{0,0}};
	#endif

	#if (defined(HMI_GRAPHIC_OPENGLES))
	GLuint					mask_texture			= 0;
	#endif
	
	if(hmi_object_id  >= HMI_PAGE_SXY_MAX_ID)/*not a page*/
	{	 	
		#if HMI_DYN_CONTAINERS_NUMBER > 0
		if(HMI_IS_DYN_CONTAINER(hmi_object_id))
		{		  	
			#if HMI_ALL_STATIC_CONTAINERS_NUMBER > 0  
			hmi_object_id  = HMI_GET_DYN_CONTAINERS_ID_INDEX(hmi_object_id);
		  	hmi_engine_get_static_container_id(&hmi_object_id);/*get static container ID*/
						
			#if HMI_DXY_CONTAINERS_NUMBER>0U
			if(HMI_IS_DYN_XY_CONTAINER(hmi_object_id))
			{
				 hmi_object_index= HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_object_id );
				if(hmi_object_index< HMI_DXY_CONTAINERS_NUMBER)
				{
					if(pfather_point != NULL)
					{
						pfather_point->x += hmi_dyn_xy_container_rect[hmi_object_index].x;
						pfather_point->y += hmi_dyn_xy_container_rect[hmi_object_index].y;
					}
				
				
					hmi_cube_get_container_prop(hmi_object_id,
											pcube_texture,
											pbump_texture,
											pfather_point);
				
				}
			}
			else
			#endif
			{
			}
			#if HMI_SXY_CONTAINERS_NUMBER>0
			if(HMI_IS_CONTAINERS_SXY(hmi_object_id))
			{
				hmi_object_index= HMI_GET_CONTAINERS_SXY_ID_INDEX(hmi_object_id );
				if(hmi_object_index< HMI_SXY_CONTAINERS_NUMBER)
				{
					if(pfather_point != NULL)
					{
						pfather_point->x += hmi_static_container_rect[hmi_object_index].x;
						pfather_point->y += hmi_static_container_rect[hmi_object_index].y;
					}
				
					hmi_cube_get_container_prop(hmi_object_id,
											pcube_texture,
											pbump_texture,
											pfather_point);					
				}
			}
			else
			#endif
			{
			}	
			#endif
		}
		else
		#endif
		#if HMI_DXY_CONTAINERS_NUMBER > 0U  		
		if(HMI_IS_DYN_XY_CONTAINER(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_object_id );
			if(hmi_object_index < HMI_DXY_CONTAINERS_NUMBER)
			{
				if(pfather_point != NULL)
				{
					pfather_point->x += hmi_dyn_xy_container_rect[hmi_object_index].x;
					pfather_point->y += hmi_dyn_xy_container_rect[hmi_object_index].y;
				}
			#if (0/*defined(HMI_GRAPHIC_OPENGLES)*/)
				mask_texture	= hmi_draw_container_render_buffer(hmi_object_id,
												hmi_dyn_xy_container_rect[hmi_object_index].w,
												hmi_dyn_xy_container_rect[hmi_object_index].h);
											
				pcube_texture->tex_id							= 0;
				pcube_texture->tex_prop.pbitmap_data.w			= 0;/*texure  valide*/
				pcube_texture->tex_prop.pbitmap_data.h			= 0;/*texure  valide*/
				pcube_texture->tex_prop.pbitmap_data.fmt_index	= (HMI_BIG_IMAGE_INDEX_STR)mask_texture;/*may be error.texture is int32*/
				
			#else
				if(hmi_dyn_xy_container_table[hmi_object_index].container_object_table.object_number > 0)
				{
					hmi_object_id	= hmi_dyn_xy_container_table[hmi_object_index].container_object_table.p_object_table[0].object_id;/*get container first child to disp*/
					hmi_cube_get_id_prop(hmi_object_id,pcube_texture,pfather_point);
				}
				if(hmi_dyn_xy_container_table[hmi_object_index].container_object_table.object_number > 1)
				{
					hmi_object_id	= hmi_dyn_xy_container_table[hmi_object_index].container_object_table.p_object_table[1].object_id;/*get container first child to disp*/
					hmi_cube_get_id_prop(hmi_object_id,pbump_texture,pfather_point);
				}
			#endif
			}
		}
		else
		#endif 
		#if HMI_SXY_CONTAINERS_NUMBER > 0 
		if(HMI_IS_CONTAINERS_SXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_CONTAINERS_SXY_ID_INDEX(hmi_object_id );
			if(hmi_object_index<HMI_SXY_CONTAINERS_NUMBER)
			{
				if(pfather_point != NULL)
				{
					pfather_point->x += hmi_static_container_rect[hmi_object_index].x;
					pfather_point->y += hmi_static_container_rect[hmi_object_index].y;
				}
			#if (0/*defined(HMI_GRAPHIC_OPENGLES)*/)				
				/*For container,draw container at  render buffer*/								
				mask_texture	= hmi_draw_container_render_buffer(hmi_object_id,
												hmi_static_container_rect[hmi_object_index].w,
												hmi_static_container_rect[hmi_object_index].h);
											
				pcube_texture->tex_id							= 0;
				pcube_texture->tex_prop.pbitmap_data.w			= 0;/*texure  valide*/
				pcube_texture->tex_prop.pbitmap_data.h			= 0;/*texure  valide*/
				pcube_texture->tex_prop.pbitmap_data.fmt_index	= (HMI_BIG_IMAGE_INDEX_STR)mask_texture;/*may be error.texture is int32*/
			#else			
				if(hmi_sxy_container_table[hmi_object_index].container_object_table.object_number > 0)
				{
					hmi_object_id		= hmi_sxy_container_table[hmi_object_index].container_object_table.p_object_table[0].object_id;/*get container first child to disp*/
					hmi_cube_get_id_prop(hmi_object_id,pcube_texture,pfather_point);
				}
				if(hmi_sxy_container_table[hmi_object_index].container_object_table.object_number > 1)
				{
					hmi_object_id=hmi_sxy_container_table[hmi_object_index].container_object_table.p_object_table[1].object_id;/*get container first child to disp*/
					hmi_cube_get_id_prop(hmi_object_id,pbump_texture,pfather_point);
				}
			#endif	
			}
		}
		else
		#endif	   
		{
			hmi_cube_get_id_prop(hmi_object_id,pcube_texture,pfather_point);
		}	   	   
	}
}
#endif

static void hmi_cube_get_container_point(HMI_OBJECT_ID_STR		hmi_object_id,								
											S3POINT_TP			*ppoint)	
{
	#if ((HMI_DYN_CONTAINERS_NUMBER > 0)||	\
		(HMI_DXY_CONTAINERS_NUMBER > 0)||	\
		(HMI_SXY_CONTAINERS_NUMBER > 0))
	HMI_OBJECT_ID_STR		hmi_object_index	= 0;	
	#endif
	if((ppoint != NULL)&&(hmi_object_id != HMI_NB_ELEMENTS))
	{	 	
		#if HMI_DYN_CONTAINERS_NUMBER > 0
		if(HMI_IS_DYN_CONTAINER(hmi_object_id))
		{		  	
			#if HMI_ALL_STATIC_CONTAINERS_NUMBER > 0  
			hmi_object_id  = HMI_GET_DYN_CONTAINERS_ID_INDEX(hmi_object_id);
		  	hmi_engine_get_static_container_id(&hmi_object_id);/*get static container ID*/
						
			#if HMI_DXY_CONTAINERS_NUMBER>0U
			if(HMI_IS_DYN_XY_CONTAINER(hmi_object_id))
			{
				 hmi_object_index = HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_object_id );
				if(hmi_object_index < HMI_DXY_CONTAINERS_NUMBER)
				{
					hmi_cube_get_container_point(hmi_object_id,ppoint);
				}
			}
			else
			#endif
			{
			}
			#if HMI_SXY_CONTAINERS_NUMBER>0
			if(HMI_IS_CONTAINERS_SXY(hmi_object_id))
			{
				hmi_object_index = HMI_GET_CONTAINERS_SXY_ID_INDEX(hmi_object_id );
				if(hmi_object_index < HMI_SXY_CONTAINERS_NUMBER)
				{
					hmi_cube_get_container_point(hmi_object_id,ppoint);				
				}
			}
			else
			#endif
			{
			}	
			#endif
		}
		else
		#endif
		#if HMI_DXY_CONTAINERS_NUMBER > 0U  		
		if(HMI_IS_DYN_XY_CONTAINER(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_object_id );
			if(hmi_object_index < HMI_DXY_CONTAINERS_NUMBER)
			{
				if(hmi_dyn_xy_container_table[hmi_object_index].container_object_table.object_number >= 1)
				{
					hmi_object_id	= hmi_dyn_xy_container_table[hmi_object_index].container_object_table.p_object_table[0].object_id;/*get container first child to disp*/
					hmi_cube_get_container_point(hmi_object_id,ppoint);				
				}
			}
		}
		else
		#endif 
		#if HMI_SXY_CONTAINERS_NUMBER > 0 
		if(HMI_IS_CONTAINERS_SXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_CONTAINERS_SXY_ID_INDEX(hmi_object_id );
			if(hmi_object_index < HMI_SXY_CONTAINERS_NUMBER)
			{
				if(hmi_sxy_container_table[hmi_object_index].container_object_table.object_number >= 1)
				{
					hmi_object_id	= hmi_sxy_container_table[hmi_object_index].container_object_table.p_object_table[0].object_id;/*get container first child to disp*/
					hmi_cube_get_container_point(hmi_object_id,ppoint);				
				}
			}
		}
		else /*image,imagelist,page,fill,scroll bar,text,button*/
		#endif	  
		{
			hmi_cube_get_id_point(hmi_object_id,ppoint);
		}	   	   
	}
}


/*static 2023 04 26*/ void  hmi_engine_draw_container(HMI_CONTAINER_STR CONST * phmi_container_info,HMI_RECT_STR CONST *pfarther_rect
										#if  (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||\
											defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||\
											defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||\
											defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))
										,HMI_RECT_STR *pdirty_rect,
										U08		depth,/*node of page NO*/
										HMI_RECT_STR *pcliped_farther_rect
										#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))										
										,HMI_ALPHA_SCALE_PT_STR *pfather_alpha_scale 
										#endif
										#elif (defined(HMI_GRAPHIC_TWLIB))	
										,HMI_ELEMENT_TYPE parent_object_id_type,
										HMI_OBJECT_ID_STR hmi_object_id,
										BOOLEAN				bDrawBck
										#if HMI_RENDER_ALL_EXCEPT_BCK==NO
										,HMI_OBJECT_ID_STR parent_object_id 
										,BOOLEAN				only_draw_flag
										#endif	
										#elif(defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
										,BOOLEAN				draw_flag 
										#endif
										) REENTRANT
										
{
	UINT8 hmi_number_object  = phmi_container_info->container_object_table.object_number;
	HMI_OBJECT_PROP_STR CONST * phmi_container_object_table= phmi_container_info->container_object_table.p_object_table;
#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_ST)\
			||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_OPENGLES)\
			||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)\
		||defined(HMI_GRAPHIC_ST7513))
	UINT8 hmi_number_object_const	= hmi_number_object;
	U08	hmi_depth=depth;
#endif
#if defined(S6J3200_GRAPHIC)
	#if HMI_DXY_CONTAINERS_NUMBER > 0U
	HMI_OBJECT_ID_STR	hmi_object_id		= 0;
	HMI_ELEMENT_PROP_STR hmi_object_prop	= {0};
	#endif
#endif
	#if defined(HMI_GRAPHIC_TWLIB)
	BYTE				value_index		= 0;
	BYTE				value_beg_bits	= 0;
	BYTE				value_imglist	= 0;
	BOOLEAN				bRlc			= FALSE;
	HMI_OBJECT_ID_STR hmi_object_id_const=hmi_object_id;
	UINT16				imagelist_date	= 0;
	#endif

		
	#if defined(HMI_GRAPHIC_TWLIB)
	/*Draw element background*/
	if((!only_draw_flag)
		#if HMI_RENDER_ALL_EXCEPT_BCK==NO
		||(!hmi_is_render_mode())
		#endif
		)
	{
		#if HMI_DXY_PAGES_NUMBER>0
		if(HMI_IS_DXY_PAGE(hmi_object_id))
		{
			hmi_object_id=HMI_GET_PAGE_DXY_ID_INDEX(hmi_object_id);
			if(hmi_object_id<HMI_DXY_PAGES_NUMBER)
			{	
				if(bDrawBck)
				{
					call_C_hmi_driver_draw_image(pfarther_rect,hmi_dxy_page_table_rlc[hmi_object_id],
												NULL,&hmi_dxy_page_table_bck[hmi_object_id]/*,
												TRUE dxy*/,FALSE/*merge*/
												#if HMI_RENDER_ALL_EXCEPT_BCK==NO
												,parent_object_id
												,hmi_object_id_const
												#endif
												);
				}
			}
		}
		#endif
		#if HMI_SXY_PAGES_NUMBER>0
		if(HMI_IS_SXY_PAGE(hmi_object_id))
		{
			hmi_object_id=HMI_GET_PAGE_SXY_ID_INDEX(hmi_object_id);
			if(hmi_object_id<HMI_SXY_PAGES_NUMBER)
			{	
				if(bDrawBck)
				{
					call_C_hmi_driver_draw_image(pfarther_rect,hmi_sxy_page_table_rlc[hmi_object_id],
												NULL,&hmi_sxy_page_table_bck[hmi_object_id]/*,
												FALSE dxy*/,FALSE/*merge*/
												#if HMI_RENDER_ALL_EXCEPT_BCK==NO
												,parent_object_id
												,hmi_object_id_const
												#endif
												);
				}
			}
		}
		#endif

		#if HMI_DXY_CONTAINERS_NUMBER>0
		if(HMI_IS_DYN_XY_CONTAINER(hmi_object_id))
		{
			hmi_object_id=HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_object_id);
			if(hmi_object_id<HMI_DXY_CONTAINERS_NUMBER)
			{	
				if(bDrawBck)
				{
					imagelist_date=(hmi_object_id*HMI_TW_CONTAINER_COMPRESS_MAX_STATUS_BIT_LEN);
					value_index=imagelist_date>>3;
					value_beg_bits=imagelist_date%8;
					value_imglist=hmi_dyn_xy_container_compress[value_index];
					value_imglist=value_imglist<<(8-(value_beg_bits+HMI_TW_CONTAINER_COMPRESS_MAX_STATUS_BIT_LEN));
					value_imglist=value_imglist>>(8-HMI_TW_CONTAINER_COMPRESS_MAX_STATUS_BIT_LEN);
					bRlc=value_imglist;
					call_C_hmi_driver_draw_image(pfarther_rect,bRlc/*rlc*/,NULL,
												(HMI_BITMAP_STR*)(&hmi_dyn_xy_container_table_bck[hmi_object_id])/*,
												TRUE*/,FALSE
												#if HMI_RENDER_ALL_EXCEPT_BCK==NO
												,parent_object_id
												,hmi_object_id_const
												#endif
												);
				}
			}
		}
		#endif

		#if HMI_SXY_CONTAINERS_NUMBER>0
		if(HMI_IS_CONTAINERS_SXY(hmi_object_id))
		{
			hmi_object_id=HMI_GET_CONTAINERS_SXY_ID_INDEX(hmi_object_id);
			if(hmi_object_id<HMI_SXY_CONTAINERS_NUMBER)
			{
				if(bDrawBck)
				{
					imagelist_date =(hmi_object_id*HMI_TW_CONTAINER_COMPRESS_MAX_STATUS_BIT_LEN);
					value_index=imagelist_date>>3;
					value_beg_bits=imagelist_date&0x07;
					value_imglist=hmi_static_container_compress[value_index];
					value_imglist=value_imglist<<(8-(value_beg_bits+HMI_TW_CONTAINER_COMPRESS_MAX_STATUS_BIT_LEN));
					value_imglist=value_imglist>>(8-HMI_TW_CONTAINER_COMPRESS_MAX_STATUS_BIT_LEN);
					bRlc=value_imglist;
					call_C_hmi_driver_draw_image(pfarther_rect,bRlc/*rlc*/,NULL,
												&hmi_sxy_container_table_bck[hmi_object_id]/*,
												FALSE*/,FALSE
												#if HMI_RENDER_ALL_EXCEPT_BCK==NO
												,parent_object_id
												,hmi_object_id_const
												#endif
												);
				}
			}
		}
		#endif				
	}	
	else /*refresh page*/
	{
		#if HMI_DXY_PAGES_NUMBER>0
		if(HMI_IS_DXY_PAGE(hmi_object_id))
		{
			
		}
		#endif
		#if HMI_SXY_PAGES_NUMBER>0
		if(HMI_IS_SXY_PAGE(hmi_object_id))
		{
			
		}
		#endif		
	}
	#endif /*HMI_GRAPHIC_TWLIB*/
	/*draw element*/

	while(hmi_number_object > 0)	
	{		
		#if defined(S6J3200_GRAPHIC)
		if(hmi_depth == HMI_PAGE_BEGIN_DEPTH)/*search tree from page node*/
		{
			depth = (hmi_number_object_const - hmi_number_object);/*node of page no*/		 
			hmi_engine_draw_object(phmi_container_object_table,
				(HMI_RECT_STR CONST *)pfarther_rect,
				pdirty_rect,
				depth,
				pcliped_farther_rect
				#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))				
				,pfather_alpha_scale
				#endif
				);
				
			#if HMI_DXY_CONTAINERS_NUMBER > 0U
			hmi_object_id = phmi_container_object_table->object_id;
			if(HMI_IS_DYN_XY_CONTAINER(hmi_object_id))
			{
				hmi_engine_get_object_prop(hmi_object_id,&hmi_object_prop);
				if(hmi_object_prop.alpha != 255)
				{
					hmi_driver_set_win_alpha(hmi_object_prop.alpha,depth);
				}
			}
			#endif
			//hmi_current_screen_swap_one_buffer(depth,pdirty_rect);
			phmi_container_object_table	+=1;
		}
		else
		{				 			 
			hmi_engine_draw_object(phmi_container_object_table++,
				(HMI_RECT_STR CONST *)pfarther_rect,
				pdirty_rect,
				depth,
				pcliped_farther_rect
				#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))				
				,pfather_alpha_scale
				#endif
				);			 			
		}
		
		#elif (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||	\
				defined(HMI_GRAPHIC_OPENGLES)||	 \
				defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||	 \
	 			defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))
		if(hmi_depth == HMI_PAGE_BEGIN_DEPTH)/*search tree from page node*/
		{
			depth = (hmi_number_object_const-hmi_number_object);/*node of page no*/
		}
		hmi_engine_draw_object(phmi_container_object_table++,
			 (HMI_RECT_STR CONST *)pfarther_rect,
			 pdirty_rect,
			 depth,
			 pcliped_farther_rect
			#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL)) 			
			 ,pfather_alpha_scale
			#endif
			 );
		
		#elif(defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
	 	hmi_engine_draw_object(phmi_container_object_table++,
		 		(HMI_RECT_STR CONST *)pfarther_rect,draw_flag);
		#else /*tw*/
 		hmi_engine_draw_object(phmi_container_object_table++,
			 (HMI_RECT_STR CONST *)pfarther_rect
			 ,parent_object_id_type
			#if HMI_RENDER_ALL_EXCEPT_BCK==NO				
			 ,hmi_object_id_const
			 ,only_draw_flag
			#endif
			 );
		#endif
	hmi_number_object--;

	}
}
										
#if HMI_DXY_SPLINE_NUMBER + HMI_SXY_SPLINE_NUMBER > 0U
void hmi_clear_spline_point_polyline(HMI_INPUT_POLYGON_STR *pspline_input_point)
{
	if(pspline_input_point != NULL)
	{
		pspline_input_point->length			= 0U;
		pspline_input_point->remain_length	= 0U;
	}
}
#endif

void hmi_engine_get_id_pos(HMI_OBJECT_ID_STR hmi_object_id,S3POINT_TP *ppos)
{
	HMI_OBJECT_ID_STR hmi_object_index= 0;
#if HMI_DYN_CONTAINERS_NUMBER > 0
		if(HMI_IS_DYN_CONTAINER(hmi_object_id))
		{			
			#if HMI_ALL_STATIC_CONTAINERS_NUMBER > 0  
				hmi_object_id  = HMI_GET_DYN_CONTAINERS_ID_INDEX(hmi_object_id);
				hmi_engine_get_static_container_id(&hmi_object_id);/*get static container ID*/					
			#endif
		}
#endif

#if HMI_DXY_IMAGELIST_NUMBER > 0
	if(HMI_IS_DYN_IMAGELIST_DXY(hmi_object_id))
	{
		hmi_object_index = HMI_GET_DYN_IMAGELIST_DXY_ID_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_DXY_IMAGELIST_NUMBER)
		{
			ppos->x =	hmi_dxy_imagelist_rect[hmi_object_index].x;
			ppos->y =	hmi_dxy_imagelist_rect[hmi_object_index].y;	
		}
	}
	else
#endif
#if HMI_SXY_IMAGELIST_NUMBER>0
	if(HMI_IS_DYN_IMAGELIST_SXY(hmi_object_id))
	{
		hmi_object_index = HMI_GET_DYN_IMAGELIST_SXY_ID_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_SXY_IMAGELIST_NUMBER)
		{
			ppos->x =	hmi_sxy_imagelist_rect[hmi_object_index].x;
			ppos->y =	hmi_sxy_imagelist_rect[hmi_object_index].y;
		}
	}
	else
#endif
#if HMI_DXY_SCROLLBAR_NUMBER>0
	if(HMI_IS_DYN_SCROLLBAR_DXY(hmi_object_id))
	{
		hmi_object_index = HMI_GET_DYN_SCROLLBAR_DXY_ID_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_DXY_SCROLLBAR_NUMBER)
		{
			ppos->x =	hmi_dxy_scrollbar_rect[hmi_object_index].x;
			ppos->y =	hmi_dxy_scrollbar_rect[hmi_object_index].y;
		}
		
	}
	else
#endif
#if HMI_SXY_SCROLLBAR_NUMBER>0
	if(HMI_IS_DYN_SCROLLBAR_SXY(hmi_object_id))
	{
		hmi_object_index = HMI_GET_DYN_SCROLLBAR_SXY_ID_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_SXY_SCROLLBAR_NUMBER)
		{
			ppos->x =	hmi_sxy_scrollbar_rect[hmi_object_index].x;
			ppos->y =	hmi_sxy_scrollbar_rect[hmi_object_index].y;
		}
	}
	else
#endif
#if HMI_DXY_BUTTON_NUMBER>0
	if(HMI_IS_DYN_BUTTON_DXY(hmi_object_id))
	{
		hmi_object_index = HMI_GET_DYN_BUTTON_DXY_ID_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_DXY_BUTTON_NUMBER)
		{	
			ppos->x =	hmi_dxy_button_rect[hmi_object_index].x;
			ppos->y =	hmi_dxy_button_rect[hmi_object_index].y;
		}
	}
	else
#endif
#if HMI_SXY_BUTTON_NUMBER>0
	if(HMI_IS_DYN_BUTTON_SXY(hmi_object_id))
	{
		hmi_object_index = HMI_GET_DYN_BUTTON_SXY_ID_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_SXY_BUTTON_NUMBER)
		{
			ppos->x =	hmi_sxy_button_rect[hmi_object_index].x;
			ppos->y =	hmi_sxy_button_rect[hmi_object_index].y;
		}
	}
	else
#endif
#if HMI_DYN_EDIT_TEXTS_NUMBER/*editable text*/ > 0 
	if(HMI_IS_DYN_TEXTS(hmi_object_id))
	{		
	#if HMI_DYN_XY_EDIT_TEXTS_NUMBER > 0
		if(HMI_IS_DYN_TEXT_DXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_TEXTS_DXY_POS_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DYN_XY_EDIT_TEXTS_NUMBER)
			{
				ppos->x =(hmi_dyn_xy_edit_text_prop_table[hmi_object_index].text_rect.x);
				ppos->y =(hmi_dyn_xy_edit_text_prop_table[hmi_object_index].text_rect.y);
								
			}
		}
		else
	#endif
	#if HMI_S_XY_EDIT_TEXTS_NUMBER > 0
		if(HMI_IS_DYN_TEXT_SXY(hmi_object_id))
		{
			ppos->x =(hmi_static_xy_edit_text_prop_table[hmi_object_index].text_rect.x);
			ppos->y =(hmi_static_xy_edit_text_prop_table[hmi_object_index].text_rect.y);
		}
		else
	#endif
		{
		}						
	}
	else
#endif
#if HMI_DYN_CONTAINERS_NUMBER/*dyn container*/ > 0 
	if(HMI_IS_DYN_CONTAINER(hmi_object_id))
	{
	}
	else
#endif
#if HMI_DYN_FILL_PAGES_NUMBER > 0
	if(HMI_IS_DYN_NFILL(hmi_object_id))
	{
		hmi_object_index = HMI_GET_DYN_NFILL_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_DYN_FILL_PAGES_NUMBER)
		{
			ppos->x =	hmi_fills_dyn_xy_rect[hmi_object_index].x;
			ppos->y =	hmi_fills_dyn_xy_rect[hmi_object_index].y;
			ppos->z=	hmi_fills_dyn_xy_rect[hmi_object_index].z;
		}
	}
	else
#endif
#if HMI_DYN_GFILL_NUMBER > 0
	if(HMI_IS_DYN_GFILL(hmi_object_id))
	{
		hmi_object_index = HMI_GET_DYN_GFILL_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_DYN_GFILL_NUMBER)
		{
			ppos->x =	hmi_gradient_dxy_fill_rect[hmi_object_index].x;
			ppos->y =	hmi_gradient_dxy_fill_rect[hmi_object_index].y;
			ppos->z=	hmi_gradient_dxy_fill_rect[hmi_object_index].z;
		}
	}
	else
#endif
#if HMI_DXY_CUBE_NUMBER > 0
	if(HMI_IS_DYN_CUBE(hmi_object_id))
	{
		hmi_object_index = HMI_GET_DYN_CUBE_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_DXY_CUBE_NUMBER)
		{	
			ppos->x =	hmi_cubes_dyn_xy_rect[hmi_object_index].cube_rect.x;
			ppos->y =	hmi_cubes_dyn_xy_rect[hmi_object_index].cube_rect.y;
			ppos->z =	hmi_cubes_dyn_xy_rect[hmi_object_index].z;
		}
	}
	else 
#endif
	
#if HMI_DXY_3DCUBE_NUMBER > 0
	if(HMI_IS_DYN_3DCUBE(hmi_object_id))
	{
		hmi_object_index = HMI_GET_DYN_3DCUBE_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_DXY_3DCUBE_NUMBER)
		{
			ppos->x =	hmi_3dcubes_dyn_xy_rect[hmi_object_index].cube_rect.x;
			ppos->y =	hmi_3dcubes_dyn_xy_rect[hmi_object_index].cube_rect.y;
			ppos->z =	hmi_3dcubes_dyn_xy_rect[hmi_object_index].z;
		
		}
	}
	else 
#endif
#if HMI_DXY_CONTAINERS_NUMBER > 0U 
	if(HMI_IS_DYN_XY_CONTAINER(hmi_object_id))
	{
		hmi_object_index = HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_DXY_CONTAINERS_NUMBER)
		{
			ppos->x =	hmi_dyn_xy_container_rect[hmi_object_index].x;
			ppos->y =	hmi_dyn_xy_container_rect[hmi_object_index].y;
		}
	}
	else
#endif
#if HMI_DXY_BITMAPS_NUMBER > 0
	if(HMI_IS_DYN_XY_BITMAP(hmi_object_id))
	{
		hmi_object_index = HMI_GET_DYN_XY_BITMAP_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_DXY_BITMAPS_NUMBER)
		{
			ppos->x =	hmi_bmp_dyn_xy_rect[hmi_object_index].x;
			ppos->y =	hmi_bmp_dyn_xy_rect[hmi_object_index].y;
		}
	}
	else
#endif
#if HMI_DXY_SPLINE_NUMBER > 0
   if(HMI_IS_DXY_SPLINE(hmi_object_id))
   {
	   hmi_object_index = HMI_GET_DXY_SPLINE_INDEX(hmi_object_id);
	   if(hmi_object_index < HMI_DXY_SPLINE_NUMBER)
	   {	
			ppos->x =	hmi_dxy_spline_rect[hmi_object_index].x;
			ppos->y =	hmi_dxy_spline_rect[hmi_object_index].y; 
	   }
   }
   else 
#endif
#if HMI_SXY_SPLINE_NUMBER > 0		
	if(HMI_IS_SXY_SPLINE(hmi_object_id))
	{
		hmi_object_index = HMI_GET_SXY_SPLINE_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_SXY_SPLINE_NUMBER)
		{
			ppos->x =	hmi_sxy_spline_rect[hmi_object_index].x;
			ppos->y =	hmi_sxy_spline_rect[hmi_object_index].y;
		}
	}
	else 
#endif
#if HMI_DXY_CUSTOM_CNT > 0
	if(HMI_IS_DYN_XY_CUSTOM(hmi_object_id))
	{
		//hmi_object_index = HMI_GET_DXY_CUSTOM_INDEX(hmi_object_id);
		if(hmi_object_id > HMI_SXY_SPLINE_MAX_ID)
		{
			hmi_object_index = HMI_GET_DXY_CUSTOM_INDEX(hmi_object_id);
		}
		else
		{
			hmi_object_index = 0U;
		}
	}
	else
#endif

#if HMI_STATIC_TEXTS_NUMBER  > 0 
	if(HMI_IS_STATIC_TEXTS(hmi_object_id)/*unedit text*/)
	{
	#if HMI_UNEDIT_TEXTS_DYN_XY_NUMBER>0
		if(HMI_IS_UNEDIT_TEXT_DYN_XY(hmi_object_id))
		{
			hmi_object_index  = HMI_GET_UNEDIT_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_UNEDIT_TEXTS_DYN_XY_NUMBER)
			{
			ppos->x =	(hmi_dyn_xy_unedit_text_prop_table[hmi_object_index].text_rect).x;
			ppos->y =	(hmi_dyn_xy_unedit_text_prop_table[hmi_object_index].text_rect).y;
			}
		}
		else
	#endif
	#if HMI_UNEDIT_TEXTS_STATIC_XY_NUMBER > 0
		if(HMI_IS_UNEDIT_TEXT_STATIC_XY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_S_XY_UNEDIT_POS_COLOR_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_UNEDIT_TEXTS_STATIC_XY_NUMBER)
			{
			ppos->x =	(hmi_static_xy_unedit_text_prop_table[hmi_object_index].text_rect).x;
			ppos->y =	(hmi_static_xy_unedit_text_prop_table[hmi_object_index].text_rect).y;
			}
		}
		else
	#endif
		{
		}						
	}
	else	
#endif
#if HMI_STATIC_FILL_PAGES_NUMBER > 0
	if(HMI_IS_SXY_FILL_PAGES(hmi_object_id))
	{
		hmi_object_index  = HMI_GET_SXY_FILL_PAGES_ID_INDEX(hmi_object_id );
		if(hmi_object_index < HMI_STATIC_FILL_PAGES_NUMBER)
		{
			ppos->x =	hmi_fills_static_xy_rect[hmi_object_index].x;
			ppos->y =	hmi_fills_static_xy_rect[hmi_object_index].y;
			ppos->z=	hmi_fills_static_xy_rect[hmi_object_index].z;
		}
	}
	else
#endif
	#if HMI_STATIC_GFILL_NUMBER> 0
	if(HMI_IS_SXY_GFILL_PAGES(hmi_object_id))
	{
		hmi_object_index = HMI_GET_SXY_GFILL_PAGES_ID_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_STATIC_GFILL_NUMBER)
		{
			ppos->x =	hmi_gradient_sxy_fill_rect[hmi_object_index].x;
			ppos->y =	hmi_gradient_sxy_fill_rect[hmi_object_index].y;
			ppos->z =	hmi_gradient_sxy_fill_rect[hmi_object_index].z;					
		}
	}
	else
	#endif
#if HMI_SXY_CUBE_NUMBER> 0
	if(HMI_IS_SXY_CUBE(hmi_object_id))
	{
		hmi_object_index = HMI_GET_SXY_CUBE_ID_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_SXY_CUBE_NUMBER)
		{
			ppos->x =	hmi_cubes_static_xy_rect[hmi_object_index].cube_rect.x;
			ppos->y =	hmi_cubes_static_xy_rect[hmi_object_index].cube_rect.y;
			ppos->z =	hmi_cubes_static_xy_rect[hmi_object_index].z;
			
		}
	}
	else
#endif
#if HMI_SXY_3DCUBE_NUMBER> 0
	if(HMI_IS_SXY_3DCUBE(hmi_object_id))
	{
		hmi_object_index = HMI_GET_SXY_3DCUBE_ID_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_SXY_3DCUBE_NUMBER)
		{
			ppos->x =	hmi_3dcubes_static_xy_rect[hmi_object_index].cube_rect.x;
			ppos->y =	hmi_3dcubes_static_xy_rect[hmi_object_index].cube_rect.y;
			ppos->z =	hmi_3dcubes_static_xy_rect[hmi_object_index].z;
		}
	}
	else
#endif
	#if HMI_SXY_BITMAPS_NUMBER> 0
	if(HMI_IS_S_XY_BITMAP(hmi_object_id))
	{
		hmi_object_index  = HMI_GET_SXY_BITMAPS_ID_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_SXY_BITMAPS_NUMBER)
		{
			ppos->x =	hmi_bmp_static_xy_rect[hmi_object_index].x;
			ppos->y =	hmi_bmp_static_xy_rect[hmi_object_index].y;							
		}
	}
	else
#endif
#if HMI_SXY_CONTAINERS_NUMBER > 0
	if(HMI_IS_CONTAINERS_SXY(hmi_object_id))
	{
		hmi_object_index = HMI_GET_CONTAINERS_SXY_ID_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_SXY_CONTAINERS_NUMBER)
		{
			ppos->x =	hmi_static_container_rect[hmi_object_index].x;
			ppos->y =	hmi_static_container_rect[hmi_object_index].y;
		}
	}
	else
#endif
	{
	}
}


void hmi_engine_get_id_color(HMI_OBJECT_ID_STR hmi_object_id,HMI_COLOR_STR *pcolor)
{
#if HMI_DYN_FILL_PAGES_NUMBER +	\
	HMI_DYN_GFILL_NUMBER+		\
	HMI_STATIC_FILL_PAGES_NUMBER+	\
	HMI_STATIC_GFILL_NUMBER		> 0U
	HMI_OBJECT_ID_STR hmi_object_index= 0U;
#endif

#if HMI_DYN_FILL_PAGES_NUMBER > 0
	if(HMI_IS_DYN_NFILL(hmi_object_id))
	{
		hmi_object_index = HMI_GET_DYN_NFILL_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_DYN_FILL_PAGES_NUMBER)
		{
			*pcolor=	hmi_fills_dyn_prop_table[hmi_object_index].color;
		}
	}
	else
#endif
#if HMI_DYN_GFILL_NUMBER > 0
	if(HMI_IS_DYN_GFILL(hmi_object_id))
	{
		hmi_object_index = HMI_GET_DYN_GFILL_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_DYN_GFILL_NUMBER)
		{
			*pcolor = hmi_gradient_dxy_fill_table[hmi_object_index].color1;
		}
	}
	else
#endif		
#if HMI_STATIC_FILL_PAGES_NUMBER > 0
	if(HMI_IS_SXY_FILL_PAGES(hmi_object_id))
	{
		hmi_object_index  = HMI_GET_SXY_FILL_PAGES_ID_INDEX(hmi_object_id );
		if(hmi_object_index < HMI_STATIC_FILL_PAGES_NUMBER)
		{
			*pcolor=	hmi_fills_static_prop_table[hmi_object_index].color;
		}
	}
	else
#endif
#if HMI_STATIC_GFILL_NUMBER> 0
	if(HMI_IS_SXY_GFILL_PAGES(hmi_object_id))
	{
		hmi_object_index = HMI_GET_SXY_GFILL_PAGES_ID_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_STATIC_GFILL_NUMBER)
		{
			*pcolor = hmi_gradient_sxy_fill_table[hmi_object_index].color1;			
		}
	}
	else
#endif
	{
		*pcolor = 0;
	}
}
#if HMI_DXY_CUBE_NUMBER +HMI_SXY_CUBE_NUMBER > 0U
void hmi_engine_get_cube_attr(S3POINT_TP*	pcube_rotation_axis,
								HMI_CAMERA_STR*		pcube_camera,
								HMI_OBJECT_ID_STR	hmi_object_id,
								HMI_RECT_STR CONST *pfarther_rect)
{
	
	//HMI_CAMERA_STR				cube_camera				= {0};
	HMI_OBJECT_ID_STR			hmi_object_index		= 0U;
	U08							cube_attr				= 0U;
	U08							cube_axis_attr			= 0U;
	HMI_OBJECT_ID_STR			hmi_element_id			= 0U;
	//S3POINT_TP					cube_rotation_axis[HMI_ROTATION_AXIS_CNT]	={0};
	HMI_3DCUBE_STR		CONST	*pcubes_rect			= NULL;
	HMI_CUBE_AXIS_ELEM_PROP	CONST *pcubes_axis			= NULL;
	S3POINT_TP					 hmi_pos				={0};
	HMI_WIDTH_STR				hmi_width				= 0U;
	HMI_HEIGHT_STR				hmi_height				= 0U;
	
	if(HMI_IS_DYN_CUBE(hmi_object_id))
	{			 
		hmi_object_index = HMI_GET_DYN_CUBE_INDEX(hmi_object_id);			
		if(hmi_object_index < HMI_DXY_CUBE_NUMBER)
		{
			#if HMI_DXY_CUBE_NUMBER > 0
			pcubes_rect	= &hmi_cubes_dyn_xy_rect[hmi_object_index];
			pcubes_axis	= &hmi_cubes_dyn_xy_axis[hmi_object_index];
			#endif
		}
	}
	else if(HMI_IS_SXY_CUBE(hmi_object_id))
	{			 
		hmi_object_index = HMI_GET_SXY_CUBE_ID_INDEX(hmi_object_id);			
		if(hmi_object_index < HMI_SXY_CUBE_NUMBER)
		{
			#if HMI_SXY_CUBE_NUMBER > 0
			pcubes_rect	= &hmi_cubes_static_xy_rect[hmi_object_index];
			pcubes_axis	= &hmi_cubes_static_xy_axis[hmi_object_index];
			#endif
		}
	}
	else
	{

	}
	
	if((pcubes_rect != NULL)&&(pcubes_axis != NULL))
	{
		cube_attr				= pcubes_rect->attribute;
		
		if((cube_attr & HMI_CAM_POS_ELEM)!= 0U)
		{
			hmi_element_id			=(HMI_OBJECT_ID_STR)((pcubes_rect->position).x);
			hmi_engine_get_id_pos(hmi_element_id,&hmi_pos);
			pcube_camera->position.x	=	hmi_pos.x;
			pcube_camera->position.y	=	hmi_pos.y;
			pcube_camera->position.z	=	hmi_pos.z;
			hmi_get_object_wh(hmi_element_id,
								(&hmi_width),
								(&hmi_height));
			pcube_camera->camera_near	= (float_32)hmi_width;
			pcube_camera->camera_far	= (float_32)hmi_height;
		}
		else
		{
			//cube_camera.position = p3dcubes_rect->position;
			pcube_camera->position.x	=	pcubes_rect->position.x;
			pcube_camera->position.y	=	pcubes_rect->position.y;
			pcube_camera->position.z	=	pcubes_rect->position.z;
		}

		if((cube_attr & HMI_LOOKAT_ELEM)!= 0U)
		{
			hmi_element_id			=(HMI_OBJECT_ID_STR)((pcubes_rect->target).x);
			hmi_engine_get_id_pos(hmi_element_id,&hmi_pos);
			pcube_camera->target.x	=	hmi_pos.x;
			pcube_camera->target.y	=	hmi_pos.y;
			pcube_camera->target.z	=	hmi_pos.z;
			hmi_get_object_wh(hmi_element_id,
								&hmi_width,
								&hmi_height);
				pcube_camera->target.x	+=	(hmi_width >> 1u);
				pcube_camera->target.y	+=	(hmi_height >> 1u);
		}
		else
		{
			pcube_camera->target =pcubes_rect->target;
		}

		if((cube_attr & HMI_UP_ELEM) != 0U)
		{
			hmi_element_id		= (HMI_OBJECT_ID_STR)((pcubes_rect->up).x);
			hmi_engine_get_id_pos(hmi_element_id,&hmi_pos);
			pcube_camera->up.x	= hmi_pos.x;
			pcube_camera->up.y	= hmi_pos.y;
			pcube_camera->up.z	= hmi_pos.z;
		}
		else
		{
				pcube_camera->up.x = pcubes_rect->up.x;
				pcube_camera->up.y = pcubes_rect->up.y;
				pcube_camera->up.z = pcubes_rect->up.z;
		}

		cube_axis_attr	= pcubes_axis->attribute;
		if((cube_axis_attr & HMI_PRI_AXIS)!= 0U)
		{
			hmi_element_id = (HMI_OBJECT_ID_STR)((pcubes_axis->private_pos).x);
			hmi_engine_get_id_pos(hmi_element_id,&hmi_pos);
			pcube_rotation_axis[HMI_ROTATION_PRIVATE_AXIS] = hmi_pos;
		}
		else
		{
			pcube_rotation_axis[HMI_ROTATION_PRIVATE_AXIS] = pcubes_axis->private_pos;
		}

		if((cube_axis_attr & HMI_PUBLIC1_AXIS)!= 0U)
		{
			hmi_element_id = (HMI_OBJECT_ID_STR)((pcubes_axis->public1).x);
			hmi_engine_get_id_pos(hmi_element_id,&hmi_pos);
			pcube_rotation_axis[HMI_ROTATION_PUBLIC_AXIS1] = hmi_pos;
		}
		else
		{
			pcube_rotation_axis[HMI_ROTATION_PUBLIC_AXIS1] = pcubes_axis->public1;
		}

		if((cube_axis_attr & HMI_PUBLIC2_AXIS)!= 0U)
		{
			hmi_element_id = (HMI_OBJECT_ID_STR)((pcubes_axis->public2).x);
			hmi_engine_get_id_pos(hmi_element_id,&hmi_pos);
			pcube_rotation_axis[HMI_ROTATION_PUBLIC_AXIS2] = hmi_pos;
		}
		else
		{
			pcube_rotation_axis[HMI_ROTATION_PUBLIC_AXIS2] = pcubes_axis->public2;
		}

	}
}
#endif

#if HMI_DXY_3DCUBE_NUMBER + HMI_SXY_3DCUBE_NUMBER > 0U
void hmi_engine_draw_3dcube(
								HMI_RECT_STR	*pscreen_target,
								//HMI_CUBE_STR *pcube_str,
								//S3POINT_TP	*protation_axis,
								HMI_RECT_STR		*pclip_rect,
								HMI_RECT_STR	*pdirty_rect,
								HMI_OBJECT_ID_STR hmi_object_id,
								UINT32 node_index)
{
#if 0
	HMI_VERTEX_ATTR_STR			cube_vertex_attr		= {0};
	HMI_MATERIAL_PROPERTIES		cube_vertex_material	= {0};
	BOOLEAN						cube_normal_enable		= FALSE;
	BOOLEAN						cube_color_enable		= FALSE;
	BOOLEAN						cube_texture_enable	= FALSE;
	UINT32_T					cube_side				= 0U;
	DIRECTIONAL_LIGHT_STR		cube_direction_light	= {0};
	HMI_CAMERA_STR				cube_camera				= {0};
	HMI_3D_NODE_STR	CONST		*pcube_node_list		= &hmi_3d_node_list[node_index];
	UINT32						cube_child_begin		= pcube_node_list->node_list_index;/*child begin index */
	UINT32						cube_child_end			= cube_child_begin + pcube_node_list->node_cn;/*child cnt */	
	UINT32						child_index				= 0U;
	UINT32						material_color_index	= 0U;
	UINT8						color_index				= 0U;
	UINT8						pos_index				= 0U;
	HMI_OBJECT_ID_STR			hmi_object_index		= 0U;
	HMI_CUBE_STR				cube_str				= {0};
	U08							cube_attr				= 0U;
	U08							cube_axis_attr			= 0U;
	HMI_OBJECT_ID_STR			hmi_element_id			= 0U;
	S3POINT_TP					cube_rotation_axis[HMI_ROTATION_AXIS_CNT]	={0};
	HMI_3DCUBE_STR CONST		*p3dcubes_rect			= NULL;
	HMI_CUBE_AXIS_ELEM_PROP CONST	*p3dcubes_axis		= NULL;
	HMI_COLOR_STR				 	hmi_color			= 0U;
	S3POINT_TP					 	hmi_pos				= {0};
	HMI_BITMAP_STR CONST*		 	ptexture_attr		= NULL;
	HMI_WIDTH_STR					hmi_width			= 0U;
	HMI_HEIGHT_STR					hmi_height			= 0U;
	UINT16							mesh_index			= 0U;
	HMI_RANGE_MESH_STR CONST*		pindex				= NULL;
	HMI_RANGE_MESH_STR CONST*		pvertex				= NULL;
	HMI_RANGE_MESH_STR CONST*		pnormal				= NULL;
	HMI_RANGE_MESH_STR CONST*		puv					= NULL;
	HMI_RANGE_MESH_STR CONST*		pmaterial			= NULL;
	HMI_RANGE_MESH_STR CONST*		pcolor				= NULL;
	
	if(HMI_IS_DYN_3DCUBE(hmi_object_id))
	{			 
		hmi_object_index = HMI_GET_DYN_3DCUBE_INDEX(hmi_object_id);			
		if(hmi_object_index < HMI_DXY_3DCUBE_NUMBER)
		{
			#if HMI_DXY_3DCUBE_NUMBER > 0
			p3dcubes_rect	= &hmi_3dcubes_dyn_xy_rect[hmi_object_index];
			p3dcubes_axis	= &hmi_3d_cubes_dyn_xy_axis[hmi_object_index];
			ptexture_attr	= &hmi_3D_texture_dyn_xy_prop_table[hmi_object_index];
			#endif
		}
	}
	else if(HMI_IS_SXY_3DCUBE(hmi_object_id))
	{			 
		hmi_object_index = HMI_GET_SXY_3DCUBE_ID_INDEX(hmi_object_id);			
		if(hmi_object_index < HMI_SXY_3DCUBE_NUMBER)
		{
			#if HMI_SXY_3DCUBE_NUMBER > 0
			p3dcubes_rect	= &hmi_3dcubes_static_xy_rect[hmi_object_index];
			p3dcubes_axis	= &hmi_3d_cubes_static_xy_axis[hmi_object_index];
			ptexture_attr	= &hmi_3D_texture_static_xy_prop_table[hmi_object_index];
			#endif
		}
	}
	else
	{

	}
	
	if((p3dcubes_rect != NULL)&&(p3dcubes_axis != NULL))
	{
		cube_str.angel			= p3dcubes_rect->angel;
		cube_str.cube_rect		= p3dcubes_rect->cube_rect;
		cube_str.bump			= p3dcubes_rect->bump;
		cube_str.z				= p3dcubes_rect->z;
		cube_str.private_angel	= p3dcubes_rect->private_angel;
		cube_str.scale			= p3dcubes_rect->scale;
		cube_attr				= p3dcubes_rect->attribute;
		if((cube_attr & HMI_L_POS_ELEM) != 0U)
		{
			hmi_element_id	= (HMI_OBJECT_ID_STR)(p3dcubes_rect->direction[HMI_POS_X]);
			hmi_engine_get_id_pos(hmi_element_id,&hmi_pos);
			cube_direction_light.direction[HMI_POS_X]	=	hmi_pos.x;
			cube_direction_light.direction[HMI_POS_Y]	=	hmi_pos.y;
			cube_direction_light.direction[HMI_POS_Z]	=	hmi_pos.z;
			/*width as specluar exponent*/
			hmi_get_object_wh(hmi_element_id,
								(&hmi_width),
								(&hmi_height));
			cube_direction_light.specular_exponent	= (GLfloat)hmi_width;
		}
		else
		{
			for(pos_index =0;pos_index < HMI_POS_COMPONENT_COUNT;pos_index++)
			{
				cube_direction_light.direction[pos_index]=	(float_32)(p3dcubes_rect->direction[pos_index] / HMI_MAX_FLOAT_ARGB_CHANNEL);
			}
			cube_direction_light.specular_exponent	= 1.0f;
		}

		if((cube_attr & HMI_L_DIFF_ELEM) != 0U)
		{
			hmi_element_id = (HMI_OBJECT_ID_STR)(p3dcubes_rect->diffuse_color[HMI_ABGR_COLOR_R]);
			hmi_engine_get_id_color(hmi_element_id,&hmi_color);
			
			cube_direction_light.diffuse_color[HMI_ABGR_COLOR_A]=	(float_32)(HMI_GET_A_ARGB8888(hmi_color) / HMI_MAX_FLOAT_ARGB_CHANNEL);
			cube_direction_light.diffuse_color[HMI_ABGR_COLOR_R]=	(float_32)(HMI_GET_R_ARGB8888(hmi_color) / HMI_MAX_FLOAT_ARGB_CHANNEL);
			cube_direction_light.diffuse_color[HMI_ABGR_COLOR_G]=	(float_32)(HMI_GET_G_ARGB8888(hmi_color) / HMI_MAX_FLOAT_ARGB_CHANNEL);
			cube_direction_light.diffuse_color[HMI_ABGR_COLOR_B]=	(float_32)(HMI_GET_B_ARGB8888(hmi_color) / HMI_MAX_FLOAT_ARGB_CHANNEL);
			
		}
		else
		{
			for(pos_index = 0U;pos_index < HMI_COLOR_COMPONENT_COUNT;pos_index++)
			{
				cube_direction_light.diffuse_color[pos_index]	=	(float_32)(p3dcubes_rect->diffuse_color[pos_index] / HMI_MAX_FLOAT_ARGB_CHANNEL);
			}
		}

		if((cube_attr & HMI_L_AMB_ELEM) != 0U)
		{
			hmi_element_id = (HMI_OBJECT_ID_STR)(p3dcubes_rect->ambient_color[HMI_ABGR_COLOR_R]);
			hmi_engine_get_id_color(hmi_element_id,&hmi_color);
			cube_direction_light.ambient_color[HMI_ABGR_COLOR_A]	=	(float_32)(HMI_GET_A_ARGB8888(hmi_color) / HMI_MAX_FLOAT_ARGB_CHANNEL);
			cube_direction_light.ambient_color[HMI_ABGR_COLOR_R]	=	(float_32)(HMI_GET_R_ARGB8888(hmi_color) / HMI_MAX_FLOAT_ARGB_CHANNEL);
			cube_direction_light.ambient_color[HMI_ABGR_COLOR_G]	=	(float_32)(HMI_GET_G_ARGB8888(hmi_color) / HMI_MAX_FLOAT_ARGB_CHANNEL);
			cube_direction_light.ambient_color[HMI_ABGR_COLOR_B]	=	(float_32)(HMI_GET_B_ARGB8888(hmi_color) / HMI_MAX_FLOAT_ARGB_CHANNEL);
			
		}
		else
		{
			for(pos_index = 0U;pos_index < HMI_COLOR_COMPONENT_COUNT;pos_index++)
			{
				cube_direction_light.ambient_color[pos_index]	=	(float_32)(p3dcubes_rect->ambient_color[pos_index] / HMI_MAX_FLOAT_ARGB_CHANNEL);
			}
		}

		if((cube_attr & HMI_L_SPE_ELEM) != 0U)
		{
			hmi_element_id	=(HMI_OBJECT_ID_STR)(p3dcubes_rect->specular_color[HMI_ABGR_COLOR_R]);
			hmi_engine_get_id_color(hmi_element_id,&hmi_color);
			cube_direction_light.specular_color[HMI_ABGR_COLOR_A]	=	(float_32)(HMI_GET_A_ARGB8888(hmi_color) / HMI_MAX_FLOAT_ARGB_CHANNEL);
			cube_direction_light.specular_color[HMI_ABGR_COLOR_R]	=	(float_32)(HMI_GET_R_ARGB8888(hmi_color) / HMI_MAX_FLOAT_ARGB_CHANNEL);
			cube_direction_light.specular_color[HMI_ABGR_COLOR_G]	=	(float_32)(HMI_GET_G_ARGB8888(hmi_color) / HMI_MAX_FLOAT_ARGB_CHANNEL);
			cube_direction_light.specular_color[HMI_ABGR_COLOR_B]	=	(float_32)(HMI_GET_B_ARGB8888(hmi_color) / HMI_MAX_FLOAT_ARGB_CHANNEL);
			
		}
		else
		{
			for(pos_index = 0U;pos_index < HMI_COLOR_COMPONENT_COUNT;pos_index++)
			{
				cube_direction_light.specular_color[pos_index]	=	(float_32)(p3dcubes_rect->specular_color[pos_index] / HMI_MAX_FLOAT_ARGB_CHANNEL);
			}
		}

		if((cube_attr & HMI_CAM_POS_ELEM) != 0U)
		{
			hmi_element_id			=	(HMI_OBJECT_ID_STR)((p3dcubes_rect->position).x);
			hmi_engine_get_id_pos(hmi_element_id,&hmi_pos);
			cube_camera.position.x	=	hmi_pos.x;
			cube_camera.position.y	=	hmi_pos.y;
			cube_camera.position.z	=	hmi_pos.z;
			hmi_get_object_wh(hmi_element_id,
								(&hmi_width),
								(&hmi_height));
			cube_camera.camera_near = (float_32)hmi_width;
			cube_camera.camera_far	= (float_32)hmi_height;
			cube_direction_light.camera_pos[HMI_POS_X] = (float_32)(hmi_pos.x);
			cube_direction_light.camera_pos[HMI_POS_Y] = (float_32)(hmi_pos.y);
			cube_direction_light.camera_pos[HMI_POS_Z] = (float_32)(hmi_pos.z);
		}
		else
		{
			//cube_camera.position = p3dcubes_rect->position;
			cube_camera.position.x	=	p3dcubes_rect->position.x;
			cube_camera.position.y	=	p3dcubes_rect->position.y;
			cube_camera.position.z	=	p3dcubes_rect->position.z;
			cube_direction_light.camera_pos[HMI_POS_X] = p3dcubes_rect->position.x;
			cube_direction_light.camera_pos[HMI_POS_Y] = p3dcubes_rect->position.y;
			cube_direction_light.camera_pos[HMI_POS_Z] = p3dcubes_rect->position.z;
		}

		if((cube_attr & HMI_LOOKAT_ELEM) != 0U)
		{
			hmi_element_id			= (HMI_OBJECT_ID_STR)((p3dcubes_rect->target).x);
			hmi_engine_get_id_pos(hmi_element_id,&hmi_pos);
			hmi_get_object_wh(hmi_element_id,
								(&hmi_width),
								(&hmi_height));
			cube_camera.target.x	=	hmi_pos.x+(hmi_width >> 1U);
			cube_camera.target.y	=	hmi_pos.y+(hmi_height >> 1U);
			cube_camera.target.z	=	hmi_pos.z;
		}
		else
		{
			cube_camera.target		= p3dcubes_rect->target;
		}

		if((cube_attr & HMI_UP_ELEM) != 0U)
		{
			hmi_element_id		= (HMI_OBJECT_ID_STR)((p3dcubes_rect->up).x);
			hmi_engine_get_id_pos(hmi_element_id,&hmi_pos);
			cube_camera.up.x	= hmi_pos.x;
			cube_camera.up.y	= hmi_pos.y;
			cube_camera.up.z	= hmi_pos.z;
		}
		else
		{
			cube_camera.up = p3dcubes_rect->up;
		}

		cube_side		= p3dcubes_axis->attribute;
		cube_axis_attr	= p3dcubes_axis->attribute;
		if((cube_axis_attr & HMI_PRI_AXIS) != 0U)
		{
			hmi_element_id = (HMI_OBJECT_ID_STR)((p3dcubes_axis->private_pos).x);
			hmi_engine_get_id_pos(hmi_element_id,&hmi_pos);
			cube_rotation_axis[HMI_ROTATION_PRIVATE_AXIS] = hmi_pos;
		}
		else
		{
			cube_rotation_axis[HMI_ROTATION_PRIVATE_AXIS] = p3dcubes_axis->private_pos;
		}

		if((cube_axis_attr & HMI_PUBLIC1_AXIS)!= 0U)
		{
			hmi_element_id = (HMI_OBJECT_ID_STR)((p3dcubes_axis->public1).x);
			hmi_engine_get_id_pos(hmi_element_id,&hmi_pos);
			cube_rotation_axis[HMI_ROTATION_PUBLIC_AXIS1] = hmi_pos;
		}
		else
		{
			cube_rotation_axis[HMI_ROTATION_PUBLIC_AXIS1] = p3dcubes_axis->public1;
		}

		if((cube_axis_attr & HMI_PUBLIC2_AXIS) != 0U)
		{
			hmi_element_id =(HMI_OBJECT_ID_STR)((p3dcubes_axis->public2).x);
			hmi_engine_get_id_pos(hmi_element_id,&hmi_pos);
			cube_rotation_axis[HMI_ROTATION_PUBLIC_AXIS2] = hmi_pos;
		}
		else
		{
			cube_rotation_axis[HMI_ROTATION_PUBLIC_AXIS2] = p3dcubes_axis->public2;
		}

	}

	
	pindex		= pcube_node_list->pindex_range;
	pnormal		= pcube_node_list->pnormal_range;
	puv			= pcube_node_list->puv_range;
	pmaterial	= pcube_node_list->pmaterial_range;
	pcolor		= pcube_node_list->pcolor_range;
	pvertex		= pcube_node_list->pvertex_range;

	/*Display node --- mesh*/
	for(mesh_index = 0U; mesh_index < pcube_node_list->mesh_cn; mesh_index++)
	{
		if(pindex[mesh_index].end > pindex[mesh_index].begin)
		{
			cube_vertex_attr.numIndices		= pindex[mesh_index].end - pindex[mesh_index].begin;
			cube_vertex_attr.pindices		= &hmi_dxy_sxy_3d_index[pindex[mesh_index].begin];
		}
		else
		{
			cube_vertex_attr.numIndices		= 0;
			cube_vertex_attr.pindices		= NULL;
		}

		if(pnormal[mesh_index].end > pnormal[mesh_index].begin)
		{
			cube_vertex_material.normalEn	= TRUE;
			cube_normal_enable				= TRUE;
			cube_vertex_attr.pnormalArray	= &hmi_dxy_sxy_3d_normal[pnormal[mesh_index].begin];
		}
		else
		{
			cube_vertex_material.normalEn 	= FALSE;
			cube_normal_enable				= FALSE;
			cube_vertex_attr.pnormalArray	= NULL;
		}

		if(pcolor[mesh_index].end > pcolor[mesh_index].begin)
		{
			cube_vertex_material.colorlEn	= TRUE;
			cube_color_enable				= TRUE;
			cube_vertex_material.texturelEn = FALSE;
			cube_texture_enable				= FALSE;
			cube_vertex_attr.pcolorArray	= &hmi_dxy_sxy_3d_color[pcolor[mesh_index].begin];
		}
		else
		{ 
			cube_vertex_material.colorlEn	= FALSE;
			cube_color_enable				= FALSE;
			cube_vertex_material.texturelEn = TRUE;
			cube_texture_enable				= TRUE;
			cube_vertex_attr.pcolorArray	= NULL;
		}
		
		if(puv[mesh_index].end > puv[mesh_index].begin)
		{
			cube_vertex_attr.puvArray		= &hmi_dxy_sxy_3d_uv[puv[mesh_index].begin];
		}
		else
		{
			cube_vertex_attr.puvArray		= NULL;
		}
		
		cube_vertex_attr.textureID	= HMI_NB_ELEMENTS;
		material_color_index		= pmaterial[mesh_index].begin;
		for(color_index = 0U; color_index < HMI_COLOR_COMPONENT_COUNT;color_index++)
		{
			cube_vertex_material.materialProp.ambient_color[color_index]	= hmi_dxy_sxy_3d_material[material_color_index].ambient_color[color_index];
			cube_vertex_material.materialProp.diffuse_color[color_index]	= hmi_dxy_sxy_3d_material[material_color_index].diffuse_color[color_index];
			cube_vertex_material.materialProp.specular_color[color_index]	= hmi_dxy_sxy_3d_material[material_color_index].specular_color[color_index];
		}
		cube_vertex_material.textureID = HMI_NB_ELEMENTS;
		pcube_node_list_buf	= &hmi_dxy_sxy_3d_ver_buf[material_color_index];
		if(pvertex[mesh_index].end > pvertex[mesh_index].begin)
		{
			cube_vertex_attr.vertexNum		= (GLint)(pvertex[mesh_index].end - pvertex[mesh_index].begin) / VERTEX_POS_SIZE;
			cube_vertex_attr.pvertexArray	= &hmi_dxy_sxy_3d_vertex[pvertex[mesh_index].begin];
			call_C_hmi_draw3d_mode_shader(pscreen_target,
								&cube_str,
								cube_rotation_axis,
								&cube_vertex_attr,
								&cube_vertex_material,
								cube_normal_enable,
								cube_color_enable,
								cube_texture_enable,
								cube_str.scale/* x scale*/,
								cube_str.scale/* y scale*/,
								cube_str.scale/* z scale*/,								
								pclip_rect,
								pdirty_rect,
								cube_side,
								&cube_direction_light,
								&cube_camera,
								ptexture_attr,
								pcube_node_list_buf->vboIds);
		}
		else
		{
			cube_vertex_attr.vertexNum		= 0;
			cube_vertex_attr.pvertexArray	= NULL;
		}
	}

	/*Display child of node,mesh*/
	for(child_index = cube_child_begin; child_index < cube_child_end; child_index++)
	{
		hmi_engine_draw_3dcube(pscreen_target,
								pclip_rect,
								pdirty_rect,
								hmi_object_id,
								child_index);
	}
#endif	
}
#endif
void hmi_engine_get_scale_rect(HMI_RECT_STR *phmi_rect
										,HMI_ALPHA_SCALE_PT_STR *pfather_alpha_scale
										,HMI_RECT_STR* phmi_scale_rect
										)
{
	HMI_RECT_STR			hmi_scale_screen	= {0,0,0,0};
	HMI_X_STR				hmi_origin_x		= 0;
	HMI_Y_STR				hmi_origin_y		= 0;
	HMI_X_STR				hmi_screen_origin_x	= 0;
	HMI_Y_STR				hmi_screen_origin_y	= 0;
	
	hmi_scale_screen.x = phmi_rect->x;
	hmi_scale_screen.y = phmi_rect->y;
	hmi_scale_screen.w = phmi_rect->w;
	hmi_scale_screen.h = phmi_rect->h;

	HMI_COORDINATE_ORIGIN_TRANS(hmi_scale_screen.x,
							hmi_scale_screen.y,
							(HMI_X_STR)(pfather_alpha_scale->point.x),
							(HMI_Y_STR)(pfather_alpha_scale->point.y));
			
	hmi_origin_x	= hmi_scale_screen.x;
	hmi_origin_y	= hmi_scale_screen.y;
	hmi_origin_x	= (HMI_X_STR)(hmi_origin_x * pfather_alpha_scale->scale);
	hmi_origin_y	= (HMI_Y_STR)(hmi_origin_y * pfather_alpha_scale->scale);
	hmi_screen_origin_x = 0;
	hmi_screen_origin_y = 0;
	HMI_COORDINATE_ORIGIN_TRANS(hmi_screen_origin_x,
								hmi_screen_origin_y,
								(HMI_X_STR)(pfather_alpha_scale->point.x),
								(HMI_Y_STR)(pfather_alpha_scale->point.y));
	HMI_COORDINATE_ORIGIN_TRANS(hmi_origin_x,hmi_origin_y,
							hmi_screen_origin_x,
							hmi_screen_origin_y);
	hmi_scale_screen.x	= hmi_origin_x;
	hmi_scale_screen.y	= hmi_origin_y;
	hmi_scale_screen.w	= (HMI_WIDTH_STR)(hmi_scale_screen.w * pfather_alpha_scale->scale);
	hmi_scale_screen.h	= (HMI_HEIGHT_STR)(hmi_scale_screen.h * pfather_alpha_scale->scale);

	phmi_scale_rect->x	= hmi_scale_screen.x;
	phmi_scale_rect->y	= hmi_scale_screen.y;
	phmi_scale_rect->w	= hmi_scale_screen.w;
	phmi_scale_rect->h	= hmi_scale_screen.h;
	

}

#ifdef HMI_GRAPHIC_OPENGLES
void hmi_draw_opengl_scene(HMI_RECT_STR	*pfather_dsp_zone,HMI_OBJECT_ID_STR		custom_id);
#endif
static void  hmi_engine_draw_object(HMI_OBJECT_PROP_STR CONST * phmi_object_prop_table,HMI_RECT_STR CONST *pfarther_rect
											#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||	\
												defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||	\
												defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||	\
												defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))
											,HMI_RECT_STR *pdirty_rect,
											U08		depth,
											HMI_RECT_STR *pcliped_farther_rect
											#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))											
											//,UINT8 father_alpha
											,HMI_ALPHA_SCALE_PT_STR *pfather_alpha_scale
											#endif
											#elif (defined(HMI_GRAPHIC_TW8836)) /*tw*/
											,HMI_ELEMENT_TYPE parent_object_id_type
											#if HMI_RENDER_ALL_EXCEPT_BCK==NO												
											,HMI_OBJECT_ID_STR parent_object_id
											,BOOLEAN		only_draw_flag
											#endif
											#elif(defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
											,BOOLEAN				draw_flag
											#endif
											) REENTRANT
{
	HMI_OBJECT_ID_STR	hmi_object_id		= phmi_object_prop_table->object_id;
	HMI_OBJECT_ID_STR	hmi_object_index	= 0;
#if (HMI_DXY_ROTATION_BITMAPS_NUMBER +		\
	HMI_DXY_CENTER_MUL_TEXTURE_BITMAPS_NUMBER +	\
	HMI_DXY_SPLINE_NUMBER+HMI_SXY_SPLINE_NUMBER)> 0	
	HMI_OBJECT_ID_STR	hmi_object_index2	= 0;
#endif
#if HMI_DXY_ROTATION_BITMAPS_NUMBER > 0
	HMI_ROTATION_TRAIL_STR trail_info		= {0};
	BOOLEAN				hmi_trail_get_id_flag	= FALSE;
	HMI_ROTATION_STR	rotation		= {0,0};
#endif
	HMI_RECT_STR		hmi_rect_temp		= {HMI_INVALID_COOR,HMI_INVALID_COOR,0,0};
#if ((HMI_DXY_IMAGELIST_NUMBER > 0U) 	\
		||(HMI_SXY_IMAGELIST_NUMBER > 0U)	\
		||(HMI_DXY_SCROLLBAR_NUMBER > 0U)	\
		||(HMI_SXY_SCROLLBAR_NUMBER > 0U)	\
		||(HMI_DXY_BUTTON_NUMBER > 0U)		\
		||(HMI_SXY_BUTTON_NUMBER > 0U))
	UINT16				value_index			= 0;
	BYTE				value_beg_bits		= 0;
	BYTE				value_imglist		= 0;
	UINT16				imagelist_date		= 0;
#endif
#if ((HMI_DYN_EDIT_TEXTS_NUMBER > 0U) || (HMI_STATIC_TEXTS_NUMBER > 0U))
	HMI_TEXT_RECT_STR 	text_prop		= {{HMI_INVALID_COOR,HMI_INVALID_COOR,0,0},0};
	U08					font_id			= HMI_INVALIATE_FONT_ID;
#endif
	HMI_RECT_STR		hmi_screen_rect	= {HMI_INVALID_COOR,HMI_INVALID_COOR,0,0};
	HMI_RECT_STR		hmi_clip_rect	= {HMI_INVALID_COOR,HMI_INVALID_COOR,0,0};/*clip zone*/	
#if (HMI_STATIC_TEXTS_NUMBER > 0U)
	HMI_TEXT_PROP_STR	text_info		= {0,0,0,NULL};
	HMI_CHAR_STR		**pp_str_list	= NULL;
#endif
#if ((HMI_STATIC_GFILL_NUMBER +	HMI_DYN_GFILL_NUMBER+		\
	HMI_STATIC_FILL_PAGES_NUMBER +	HMI_DYN_FILL_PAGES_NUMBER+	\
	HMI_DYN_EDIT_TEXTS_NUMBER) > 0)

	UINT8				hmi_alpha			= 0;
#endif
	#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||	\
		defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||	\
		defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||	\
		defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))		
	HMI_RECT_STR new_cliped_farther_rect= {HMI_INVALID_COOR,HMI_INVALID_COOR,0,0};	
	HMI_OBJECT_ID_STR 	hmi_object_id_const	= hmi_object_id;/*2023 02 15*/
	#elif(defined(HMI_GRAPHIC_TW8836)) /*tw*/
	BOOLEAN				bMerge		= FALSE;	
	BOOLEAN				flag		= FALSE;	
	HMI_OBJECT_ID_STR 	hmi_object_id_const	= hmi_object_id;
	#elif (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
	BOOLEAN				flag		= FALSE;	
	BOOLEAN				child_draw_flag	= FALSE;
	#endif
#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
	#if ((HMI_DXY_CUBE_NUMBER+HMI_SXY_CUBE_NUMBER) > 0)
	HMI_CUBE_FACE_STR		hmi_cube_textrue	= {0};
	HMI_CUBE_FACE_STR		hmi_bump_textrue	= {0};	
	#endif
	
	#if HMI_DXY_ROTATION_BITMAPS_NUMBER > 0 
	HMI_CUBE_TEXTURE_PROP	image_trail_texture		= {0};
	HMI_CUBE_TEXTURE_PROP	image_trail_bump_texture= {0};
	HMI_OBJECT_ID_STR		hmi_trail_id		=HMI_NB_ELEMENTS;
	SPOINT_TP				trail_point	={0};
	HMI_RECT_STR			trail_rect	={0};	
	#endif
#if ((HMI_DXY_CUBE_NUMBER+HMI_SXY_CUBE_NUMBER) > 0)
	//S3POINT_TP			axis[HMI_ROTATION_AXIS_CNT] = {{0,0,0},{0,0,0},{0,0,0}};	
	S3POINT_TP			cube_rotation_axis[HMI_ROTATION_AXIS_CNT]	={0};
	HMI_CAMERA_STR		cube_camera ={0};
#endif
#endif

#if (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
	UINT16	container_max_layer	= 0;
	UINT16  container_max_text_index	= 0;
	UINT16	next_layer			= 0;
	UINT16 	next_text_index		= 0;
	UINT8	free_index			= 0;
	#if HMI_SCENE_DYN_CONTAINERS_NUMBER > 0
	HMI_DYN_CONTAINER_DATA_STR		hmi_animat_action	= 0;	
	U32								hmi_animat_address	= 0;
	#endif
#endif

#if ((HMI_DXY_SPLINE_NUMBER+HMI_SXY_SPLINE_NUMBER) > 0)
	HMI_WIDTH_STR			spline_width	= 0;
	HMI_HEIGHT_STR			height			=0;
	HMI_OBJECT_ID_STR		hmi_spline_prop_id		= 0;
	HMI_SPLINE_COLOR_STR	spline_color			= {0};
	HMI_SPLINE_COLOR_STR	spline_zone_color		= {0};
	BOOLEAN					hmi_get_color_success		= FALSE;
	BOOLEAN					hmi_get_zone_color_success	= FALSE;
	BOOLEAN					hmi_get_ctrl_point_success	= FALSE;
	HMI_SPLINE_PROP_STR 	hmi_spline_prop = {0};
#endif
#if HMI_DYN_CONTAINERS_NUMBER >0
	UINT8	loop				= 0;
#endif
#if HMI_DXY_CUSTOM_CNT+HMI_SXY_CUSTOM_CNT > 0
	HMI_OBJECT_ID_STR		hmi_custom_type_index	= 0U;
	PCUSTOM_MANAGER_FUN		phmi_custom_function	= NULL;
	BOOLEAN					hmi_get_success			= FALSE;
#endif
#if HMI_DXY_CONTAINERS_SCALE_ALPHA_NUMBER > 0U
	float_32 hmi_dxy_container_scale	= 0.0f;
#endif
#if HMI_DXY_CUSTOM_CNT +HMI_SXY_CUSTOM_CNT > 0U
	HMI_CUSTOM_PROP_STR	custom_object_prop  ={0U};
	BOOLEAN				get_success			=FALSE;
#endif
	HMI_RECT_STR		hmi_scale_rect	= {0};

	HMI_ALPHA_SCALE_PT_STR container_alpha_scale ={255U,1.0f,{HMI_INVALID_COOR,HMI_INVALID_COOR}};

	if(pfather_alpha_scale != NULL)
	{
		container_alpha_scale.alpha		= pfather_alpha_scale->alpha;
		container_alpha_scale.point.x	= pfather_alpha_scale->point.x;
		container_alpha_scale.point.y	= pfather_alpha_scale->point.y;
		container_alpha_scale.scale		= pfather_alpha_scale->scale;
	
		if(hmi_object_id  >= HMI_PAGE_SXY_MAX_ID)/*not a page*/
	 	{	
		   	#if HMI_ALL_DYN_OBJECTS_NUMBER > 0U
			if(hmi_object_id  < HMI_ALL_DYN_OBJECTS_NUMBER)
			{			
				#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)|| \
					defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)|| \
					defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)|| \
					defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))
				HMI_DYNAMIC_OBJECT_CANCEL_CHANGED_FLAG(hmi_object_id );/*clear refresh flag*/
				#elif(defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
				flag	= HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id);
				
				if(flag)
				{	
					HMI_DYNAMIC_OBJECT_CANCEL_CHANGED_FLAG(hmi_object_id);/*clear refresh flag*/
				}
				#else /*tw8836&&yamaha*/
				flag	= HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id);
				#if HMI_RENDER_ALL_EXCEPT_BCK==NO
				if(hmi_is_render_mode())/*only render ,clear the flag*/
				{
					HMI_DYNAMIC_OBJECT_CANCEL_CHANGED_FLAG(hmi_object_id );/*clear refresh flag*/
				}
				#endif
				#endif
			}
			#endif
			#if (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
			if(draw_flag)
			{
				child_draw_flag = TRUE;
			}
			else
			{
				if(flag)
				{
					child_draw_flag = TRUE;
				}
				else
				{
					child_draw_flag = FALSE;
				}
			}
			#endif
			#if HMI_DXY_IMAGELIST_NUMBER>0
			if(HMI_IS_DYN_IMAGELIST_DXY(hmi_object_id))
		    {
				hmi_object_index = HMI_GET_DYN_IMAGELIST_DXY_ID_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_DXY_IMAGELIST_NUMBER)
				{
					hmi_rect_temp.x	=	hmi_dxy_imagelist_rect[hmi_object_index].x;
					hmi_rect_temp.y	=	hmi_dxy_imagelist_rect[hmi_object_index].y;
					hmi_rect_temp.w	=	hmi_dxy_imagelist_rect[hmi_object_index].w;
					hmi_rect_temp.h	=	hmi_dxy_imagelist_rect[hmi_object_index].h;
					
					HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
						(&hmi_rect_temp),
						(&(hmi_screen_rect)));
					#if (defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(S6J3200_GRAPHIC))
					/*element draw rect backups*/
					hmi_dxy_imagelist_rect_bck[hmi_object_index].x	= hmi_rect_temp.x;
					hmi_dxy_imagelist_rect_bck[hmi_object_index].y	= hmi_rect_temp.y;
					hmi_dxy_imagelist_rect_bck[hmi_object_index].w	= hmi_rect_temp.w;
					hmi_dxy_imagelist_rect_bck[hmi_object_index].h	= hmi_rect_temp.h;
					#endif
				}
				#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||\
					defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||\
					defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||\
					defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))
					if((fabs(pfather_alpha_scale->scale - HMI_SCALE_1)) > HMI_FLOAT_TOLERANCE)
					{
						hmi_engine_get_scale_rect(&hmi_screen_rect,pfather_alpha_scale,&hmi_scale_rect);
						hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
											&hmi_scale_rect,
											&(new_cliped_farther_rect ));
					}
					else
					{
						hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
											&hmi_screen_rect,
											&(new_cliped_farther_rect ));
					}
					hmi_get_intersec_rect_depth((HMI_RECT_STR CONST *)(&(new_cliped_farther_rect)),
									depth,(HMI_RECT_STR  *)pdirty_rect,&hmi_clip_rect);
					
				#endif
				
				#ifdef HMI_GRAPHIC_AGG
				call_C_hmi_driver_draw_imagelist(&(hmi_screen_rect),
									hmi_dxy_imglist_attr_table[hmi_object_index],
									&hmi_dxy_imagelist_table[hmi_object_index],
									hmi_dxy_imagelist_index[hmi_object_index],
									&hmi_clip_rect);
				/*Container*/
				#endif
				
				#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
				if(hmi_object_index < HMI_DXY_IMAGELIST_NUMBER)
				{
					if((hmi_clip_rect.w != 0)&&
						(hmi_clip_rect.h != 0))
					{
						imagelist_date =(hmi_object_index*HMI_IMGLIST_MAX_STATUS_BIT_LEN);
						value_index		= (imagelist_date>>3)/*/8*/;
						value_beg_bits	= (imagelist_date&0x07)/*%8*/;
						value_imglist	= hmi_dxy_imagelist_index[value_index];
						value_imglist	= value_imglist<<(8-(value_beg_bits+HMI_IMGLIST_MAX_STATUS_BIT_LEN));
						value_imglist	= value_imglist>>(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
												 
						call_C_hmi_driver_draw_imagelist(&(hmi_screen_rect)/*node screen position*/,											
														hmi_dxy_imglist_attr_table[hmi_object_index]/*compress fmt*/,
														&hmi_dxy_imagelist_rect[hmi_object_index]/*alpha*/,
														&hmi_dxy_imagelist_table[hmi_object_index],
														value_imglist,
														&hmi_clip_rect,
														pdirty_rect,
														depth,
														hmi_object_id,
														pfather_alpha_scale);
						#if HMI_DXY_IMAGE_LIST_MAX_SON_CNT > 0
						if(container_alpha_scale.alpha > hmi_dxy_imagelist_rect[hmi_object_index].alpha)
						{
							container_alpha_scale.alpha	= hmi_dxy_imagelist_rect[hmi_object_index].alpha;
						}
						hmi_engine_draw_container(&hmi_dxy_imagelist_container_table[hmi_object_index],
											(HMI_RECT_STR CONST *)(&(hmi_screen_rect)),
											pdirty_rect,
											depth,
											&new_cliped_farther_rect/*all father insection zone*/
											,&container_alpha_scale
											);
						#endif
					}
				}
				#endif
				#ifdef HMI_GRAPHIC_ST7513
				if(hmi_object_index < HMI_DXY_IMAGELIST_NUMBER)
				{
					value_index		= (hmi_object_index*HMI_IMGLIST_MAX_STATUS_BIT_LEN)>>3/*/8*/;
					value_beg_bits	= (hmi_object_index*HMI_IMGLIST_MAX_STATUS_BIT_LEN)&0x07/*%8*/;
					value_imglist	= hmi_dxy_imagelist_index[value_index];
					value_imglist	= value_imglist<<(8-(value_beg_bits+HMI_IMGLIST_MAX_STATUS_BIT_LEN));
					value_imglist	= value_imglist>>(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
											 
					call_C_hmi_driver_draw_imagelist((HMI_RECT_STR CONST *)(&(hmi_screen_rect))/*node screen position*/,												
													(HMI_IMAGE_LIST_STR CONST 	*)(&hmi_dxy_imagelist_table[hmi_object_index]),
													value_imglist,
													&hmi_clip_rect);
					hmi_engine_draw_container(&hmi_dxy_imagelist_container_table[hmi_object_index],
										(HMI_RECT_STR CONST *)(&((hmi_screen_rect))/*new parent zone*/),
										pdirty_rect,
										depth,
										&new_cliped_farther_rect/*all father insection zone*/);
				}
				#endif
				#ifdef HMI_GRAPHIC_TWLIB	
				bMerge			= hmi_get_is_merge(parent_object_id_type,HMI_ELEM_TYPE_DIMGLIST);			
				value_index		= (hmi_object_index*HMI_IMGLIST_MAX_STATUS_BIT_LEN)/8;
				value_beg_bits	= (hmi_object_index*HMI_IMGLIST_MAX_STATUS_BIT_LEN)%8;
				value_imglist	= hmi_dxy_imagelist_index[value_index];
				value_imglist	= value_imglist<<(8-(value_beg_bits+HMI_IMGLIST_MAX_STATUS_BIT_LEN));
				value_imglist	= value_imglist>>(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);			
				#if HMI_RENDER_ALL_EXCEPT_BCK==NO
				if(((!only_draw_flag)||!hmi_is_render_mode())||/*simulate*/
					(only_draw_flag&&(flag)))
				#endif
				{
					call_C_hmi_driver_draw_imagelist(&(hmi_screen_rect),
										(HMI_RECT_ALPHA_STR *)(&hmi_dxy_imagelist_rect[hmi_object_index]),
										&hmi_dxy_imagelist_table[hmi_object_index],
										value_imglist/*,
										TRUE dxy*/,bMerge,FALSE/*Button*/
										#if HMI_RENDER_ALL_EXCEPT_BCK==NO
										,parent_object_id
										,hmi_object_id_const
										#endif
										);				
				}
				if(hmi_dxy_imagelist_container_table[hmi_object_index].container_object_table.object_number>0)
				{
					hmi_engine_draw_container(&(hmi_dxy_imagelist_container_table[hmi_object_index]),
													(HMI_RECT_STR CONST *)(&(hmi_screen_rect)),
													HMI_ELEM_TYPE_DIMGLIST,
													hmi_object_id_const,
													FALSE
													#if HMI_RENDER_ALL_EXCEPT_BCK==NO
														,phmi_object_prop_table->object_id 
														,only_draw_flag&&(!flag)
													#endif
													);
				}
				#endif
				
				#if (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
				if(hmi_object_index < HMI_DXY_IMAGELIST_NUMBER)
				{
					if(flag ||draw_flag)
					{
						value_index		= (hmi_object_index*HMI_IMGLIST_MAX_STATUS_BIT_LEN)/8;
						value_beg_bits	= (hmi_object_index*HMI_IMGLIST_MAX_STATUS_BIT_LEN)%8;
						value_imglist	= hmi_dxy_imagelist_index[value_index];
						value_imglist	= value_imglist<<(8-(value_beg_bits+HMI_IMGLIST_MAX_STATUS_BIT_LEN));
						value_imglist	= value_imglist>>(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
												 
						call_C_hmi_driver_draw_imagelist(&(hmi_screen_rect)/*node screen position*/,											
														&hmi_dxy_imglist_attr_table[hmi_object_index]/*compress fmt*/,
														&hmi_dxy_imagelist_rect[hmi_object_index]/*alpha*/,
														&hmi_dxy_imagelist_table[hmi_object_index],
														value_imglist
														);
					}
					hmi_set_cur_win_used();	
					#if HMI_DXY_IMAGE_LIST_MAX_SON_CNT > 0
					hmi_engine_draw_container(&hmi_dxy_imagelist_container_table[hmi_object_index],
										(HMI_RECT_STR CONST *)(&(hmi_screen_rect)),
										child_draw_flag
										);
					#endif
					
					
					
				}
				#endif
			}		
			else
			#endif
			#if HMI_SXY_IMAGELIST_NUMBER > 0
			if(HMI_IS_DYN_IMAGELIST_SXY(hmi_object_id))
		    {
				hmi_object_index = HMI_GET_DYN_IMAGELIST_SXY_ID_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_SXY_IMAGELIST_NUMBER)
				{
					hmi_rect_temp.x	=	hmi_sxy_imagelist_rect[hmi_object_index].x;
					hmi_rect_temp.y	=	hmi_sxy_imagelist_rect[hmi_object_index].y;
					hmi_rect_temp.w	=	hmi_sxy_imagelist_rect[hmi_object_index].w;
					hmi_rect_temp.h	=	hmi_sxy_imagelist_rect[hmi_object_index].h;
					HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
										(&hmi_rect_temp),
										(&hmi_screen_rect));
						
				}
				#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||\
					defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||\
					defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||\
					defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))
					if((fabs(pfather_alpha_scale->scale - HMI_SCALE_1)) > HMI_FLOAT_TOLERANCE)
					{
						hmi_engine_get_scale_rect(&hmi_screen_rect,pfather_alpha_scale,&hmi_scale_rect);
						hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
											&hmi_scale_rect,
											&(new_cliped_farther_rect ));
					}
					else
					{
						hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
											&hmi_screen_rect,
											&(new_cliped_farther_rect ));
					}
					hmi_get_intersec_rect_depth((HMI_RECT_STR CONST *)(&(new_cliped_farther_rect)),
									depth,(HMI_RECT_STR  *)pdirty_rect,&hmi_clip_rect);
				#endif
				#ifdef HMI_GRAPHIC_AGG
				call_C_hmi_driver_draw_imagelist(&(hmi_screen_rect),
						hmi_bigbitmap_table[0],hmi_bigbitmap_table[1],
						&hmi_sxy_imagelist_table[hmi_object_index],
						hmi_sxy_imagelist_index[hmi_object_index],
						&hmi_clip_rect);
				hmi_engine_draw_container(&hmi_sxy_imagelist_container_table[hmi_object_index],
					(HMI_RECT_STR CONST *)(&(hmi_screen_rect)),
					pdirty_rect,depth,&new_cliped_farther_rect/*all father insection zone*/);
				#endif

			#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
			if(hmi_object_index < HMI_SXY_IMAGELIST_NUMBER)
			{
				if((hmi_clip_rect.w != 0)&&
					(hmi_clip_rect.h != 0))
				{
					imagelist_date =(hmi_object_index*HMI_IMGLIST_MAX_STATUS_BIT_LEN);
					value_index		= (imagelist_date>>3)/*/8*/;
					value_beg_bits	= (imagelist_date&0x07)/*%8*/;
					value_imglist	= hmi_sxy_imagelist_index[value_index];
					value_imglist	= value_imglist<<(8-(value_beg_bits+HMI_IMGLIST_MAX_STATUS_BIT_LEN));
					value_imglist	= value_imglist>>(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
								
					call_C_hmi_driver_draw_imagelist(&(hmi_screen_rect)/*node screen position*/,
										hmi_sxy_imglist_attr_table[hmi_object_index],
										(HMI_RECT_ALPHA_STR *)(&hmi_sxy_imagelist_rect[hmi_object_index]),
										&hmi_sxy_imagelist_table[hmi_object_index],
										value_imglist,
										&hmi_clip_rect,
										pdirty_rect,
										depth,
										hmi_object_id,
										pfather_alpha_scale);
					
					#if HMI_SXY_IMAGE_LIST_MAX_SON_CNT > 0
					if(container_alpha_scale.alpha > hmi_sxy_imagelist_rect[hmi_object_index].alpha)
					{
						container_alpha_scale.alpha = hmi_sxy_imagelist_rect[hmi_object_index].alpha;
					}
					hmi_engine_draw_container(&hmi_sxy_imagelist_container_table[hmi_object_index],
										(HMI_RECT_STR CONST *)(&(hmi_screen_rect)),
										pdirty_rect,depth,
										&new_cliped_farther_rect/*all father insection zone*/
										,&container_alpha_scale);	
					#endif
				}
			}
			#endif
			#ifdef HMI_GRAPHIC_ST7513
			if(hmi_object_index < HMI_SXY_IMAGELIST_NUMBER)
			{
				value_index		= (hmi_object_index*HMI_IMGLIST_MAX_STATUS_BIT_LEN)>>3/*/8*/;
				value_beg_bits	= (hmi_object_index*HMI_IMGLIST_MAX_STATUS_BIT_LEN)&0x07/*%8*/;
				value_imglist	= hmi_sxy_imagelist_index[value_index];
				value_imglist	= value_imglist<<(8-(value_beg_bits+HMI_IMGLIST_MAX_STATUS_BIT_LEN));
				value_imglist	= value_imglist>>(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
							
				call_C_hmi_driver_draw_imagelist(&(hmi_screen_rect)/*node screen position*/,										
									&hmi_sxy_imagelist_table[hmi_object_index],
									value_imglist,
									&hmi_clip_rect);
				hmi_engine_draw_container(&(hmi_sxy_imagelist_container_table[hmi_object_index]),
											(HMI_RECT_STR CONST *)(&(hmi_screen_rect)),
											pdirty_rect,depth,&new_cliped_farther_rect/*all father insection zone*/);
			}
			#endif
			
			#ifdef HMI_GRAPHIC_TWLIB	
			if(hmi_object_index < HMI_SXY_IMAGELIST_NUMBER)
			{
				bMerge			= hmi_get_is_merge(parent_object_id_type,HMI_ELEM_TYPE_SIMGLIST);
				value_index		= (hmi_object_index*HMI_IMGLIST_MAX_STATUS_BIT_LEN)>>3/*/8*/;
				value_beg_bits	= (hmi_object_index*HMI_IMGLIST_MAX_STATUS_BIT_LEN)&0x07/*%8*/;
				value_imglist	= hmi_sxy_imagelist_index[value_index];
				value_imglist	= value_imglist<<(8-(value_beg_bits+HMI_IMGLIST_MAX_STATUS_BIT_LEN));
				value_imglist	= value_imglist>>(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
				#if HMI_RENDER_ALL_EXCEPT_BCK==NO
				if(((!only_draw_flag)||!hmi_is_render_mode())||/*simulate*/
								(only_draw_flag&&(flag)))
				#endif
				{
					call_C_hmi_driver_draw_imagelist(&(hmi_screen_rect),(HMI_RECT_ALPHA_STR *)(&hmi_sxy_imagelist_rect[hmi_object_index]),
													&hmi_sxy_imagelist_table[hmi_object_index],
													value_imglist/*,
													FALSE dxy*/,bMerge,FALSE/*Button*/
													#if HMI_RENDER_ALL_EXCEPT_BCK==NO
													,parent_object_id
													,hmi_object_id_const
													#endif
													);					
				}
				if(hmi_sxy_imagelist_container_table[hmi_object_index].container_object_table.object_number>0)
				{
					hmi_engine_draw_container(&(hmi_sxy_imagelist_container_table[hmi_object_index]),
													(HMI_RECT_STR CONST *)(&(hmi_screen_rect)),
													HMI_ELEM_TYPE_SIMGLIST,
													hmi_object_id_const,
													FALSE
													#if HMI_RENDER_ALL_EXCEPT_BCK==NO
														,phmi_object_prop_table->object_id 
														,only_draw_flag&&(!flag)
													#endif
													); 
				}
			}
			#endif
			
			#if (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
			if(hmi_object_index < HMI_SXY_IMAGELIST_NUMBER)
			{
				if(flag || draw_flag)
				{
					value_index		= (hmi_object_index*HMI_IMGLIST_MAX_STATUS_BIT_LEN)/8;
					value_beg_bits	= (hmi_object_index*HMI_IMGLIST_MAX_STATUS_BIT_LEN)%8;
					value_imglist	= hmi_sxy_imagelist_index[value_index];
					value_imglist	= value_imglist<<(8-(value_beg_bits+HMI_IMGLIST_MAX_STATUS_BIT_LEN));
					value_imglist	= value_imglist>>(8-HMI_IMGLIST_MAX_STATUS_BIT_LEN);
								
					call_C_hmi_driver_draw_imagelist(&(hmi_screen_rect)/*node screen position*/,
										&hmi_sxy_imglist_attr_table[hmi_object_index],
										(HMI_RECT_ALPHA_ANGEL_STR *)(&hmi_sxy_imagelist_rect[hmi_object_index]),
										&hmi_sxy_imagelist_table[hmi_object_index],
										value_imglist
										);
				}
				hmi_set_cur_win_used();	
				
				#if HMI_SXY_IMAGE_LIST_MAX_SON_CNT > 0
				hmi_engine_draw_container(&hmi_sxy_imagelist_container_table[hmi_object_index],
									(HMI_RECT_STR CONST *)(&(hmi_screen_rect)),
									child_draw_flag
									);	
				#endif
				
			}
			#endif
		}
		else
		#endif
		
		#if HMI_DXY_SCROLLBAR_NUMBER>0
		if(HMI_IS_DYN_SCROLLBAR_DXY(hmi_object_id))
	    {
			hmi_object_index = HMI_GET_DYN_SCROLLBAR_DXY_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DXY_SCROLLBAR_NUMBER)
			{
				hmi_rect_temp.x	=	hmi_dxy_scrollbar_rect[hmi_object_index].x;
				hmi_rect_temp.y	=	hmi_dxy_scrollbar_rect[hmi_object_index].y;
				hmi_rect_temp.w	=	hmi_dxy_scrollbar_rect[hmi_object_index].w;
				hmi_rect_temp.h	=	hmi_dxy_scrollbar_rect[hmi_object_index].h;
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
					(&hmi_rect_temp),
					(&(hmi_screen_rect)));
				#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
				/*element draw rect backups*/
				hmi_dxy_scrollbar_rect_bck[hmi_object_index].x	= hmi_rect_temp.x;
				hmi_dxy_scrollbar_rect_bck[hmi_object_index].y	= hmi_rect_temp.y;
				hmi_dxy_scrollbar_rect_bck[hmi_object_index].w	= hmi_rect_temp.w;
				hmi_dxy_scrollbar_rect_bck[hmi_object_index].h	= hmi_rect_temp.h;
				#endif
			}
			#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||\
				defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||\
				defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||\
				defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))
			if((fabs(pfather_alpha_scale->scale - HMI_SCALE_1)) > HMI_FLOAT_TOLERANCE)
			{
				hmi_engine_get_scale_rect(&hmi_screen_rect,pfather_alpha_scale,&hmi_scale_rect);
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
									&hmi_scale_rect,
									&(new_cliped_farther_rect ));
			}
			else
			{
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
									&hmi_screen_rect,
									&(new_cliped_farther_rect ));
			}
			hmi_get_intersec_rect_depth((HMI_RECT_STR CONST *)(&(new_cliped_farther_rect)),
							depth,(HMI_RECT_STR  *)pdirty_rect,&hmi_clip_rect);
			#endif
			#ifdef HMI_GRAPHIC_AGG
			call_C_hmi_driver_draw_scrollbar(&(hmi_screen_rect),
											hmi_bigbitmap_table[0],hmi_bigbitmap_table[1],
											&hmi_dxy_scrollbar_table[hmi_object_index],
											hmi_dxy_scrollbar_cur_range[hmi_object_index],
											&hmi_clip_rect);
			hmi_engine_draw_container(&hmi_dxy_scrollbar_container_table[hmi_object_index],
				(HMI_RECT_STR CONST *)&(hmi_screen_rect ),
				pdirty_rect,depth,
				&new_cliped_farther_rect/*all father insection zone*/);
			#endif
			
			#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
			#if HMI_SCROLLBAR_MAX_STATUS >= 0
			if((hmi_clip_rect.w != 0)&&
					(hmi_clip_rect.h != 0))
			{
				imagelist_date =(hmi_object_index*HMI_SCROLLBAR_MAX_STATUS_BIT_LEN);
				value_index		= imagelist_date>>3/*/8*/;
				value_beg_bits	= imagelist_date&0x07/*%8*/;
				value_imglist	= hmi_dxy_scrollbar_cur_range[value_index];
				value_imglist	= value_imglist<<(8-(value_beg_bits+HMI_SCROLLBAR_MAX_STATUS_BIT_LEN));
				value_imglist	= value_imglist>>(8-HMI_SCROLLBAR_MAX_STATUS_BIT_LEN);
				if(hmi_object_index < HMI_DXY_SCROLLBAR_NUMBER)
				{
					call_C_hmi_driver_draw_scrollbar(&(hmi_screen_rect)/*node screen position*/,
							hmi_dxy_scrollbar_attr_table[hmi_object_index]/*image format*/,						
							value_imglist/*current value*/,
							&hmi_clip_rect,
							pdirty_rect/*clip zone*/,
							(HMI_SCROLL_BAR_STR	*)&hmi_dxy_scrollbar_table[hmi_object_index]/*scrollbar info*/,
							depth/*gdi layer*/,
							hmi_object_id,
							pfather_alpha_scale);
					#if HMI_DXY_SCROLLBAR_MAX_SON_CNT > 0
					hmi_engine_draw_container(&hmi_dxy_scrollbar_container_table[hmi_object_index],
						(HMI_RECT_STR CONST *)&(hmi_screen_rect ),pdirty_rect,depth,
						&new_cliped_farther_rect/*all father insection zone*/
						,&container_alpha_scale);
					#endif
				}
			}		
			#endif
			#endif
						
			#ifdef HMI_GRAPHIC_ST7513
			#if HMI_SCROLLBAR_MAX_STATUS >= 0
			value_index		= (hmi_object_index*HMI_SCROLLBAR_MAX_STATUS_BIT_LEN)>>3/*/8*/;
			value_beg_bits	= (hmi_object_index*HMI_SCROLLBAR_MAX_STATUS_BIT_LEN)&0x07/*%8*/;
			value_imglist	= hmi_dxy_scrollbar_cur_range[value_index];
			value_imglist	= value_imglist<<(8-(value_beg_bits+HMI_SCROLLBAR_MAX_STATUS_BIT_LEN));
			value_imglist	= value_imglist>>(8-HMI_SCROLLBAR_MAX_STATUS_BIT_LEN);
			if(hmi_object_index < HMI_DXY_SCROLLBAR_NUMBER)
			{
				call_C_hmi_driver_draw_scrollbar(
					(HMI_RECT_STR CONST *)(&(hmi_screen_rect))/*node screen position*/,						
						value_imglist/*current value*/,
						&hmi_clip_rect/*clip zone*/,
						(HMI_SCROLL_BAR_STR	*)(&hmi_dxy_scrollbar_table[hmi_object_index])/*scrollbar info*/)	;
				
				hmi_engine_draw_container(&hmi_sxy_scrollbar_container_table[hmi_object_index],
					(HMI_RECT_STR CONST *)&(hmi_screen_rect),pdirty_rect,depth,
					&new_cliped_farther_rect/*all father insection zone*/);
			}
			#endif
			#endif
			#ifdef HMI_GRAPHIC_TWLIB
			/*tw not support scrollbar*/
			#endif
			#if (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
			/*yamaha not support scrollbar*/
			#endif
			
		}
		else		
		#endif
		#if HMI_SXY_SCROLLBAR_NUMBER>0
		if(HMI_IS_DYN_SCROLLBAR_SXY(hmi_object_id))
	    {
			hmi_object_index = HMI_GET_DYN_SCROLLBAR_SXY_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_SXY_SCROLLBAR_NUMBER)
			{
				hmi_rect_temp.x	=	hmi_sxy_scrollbar_rect[hmi_object_index].x;
				hmi_rect_temp.y	=	hmi_sxy_scrollbar_rect[hmi_object_index].y;
				hmi_rect_temp.w	=	hmi_sxy_scrollbar_rect[hmi_object_index].w;
				hmi_rect_temp.h	=	hmi_sxy_scrollbar_rect[hmi_object_index].h;
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
					(&hmi_rect_temp),
					(&(hmi_screen_rect)));				
			}
			#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||\
				defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||\
				defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||\
				defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))
			if((fabs(pfather_alpha_scale->scale - HMI_SCALE_1)) > HMI_FLOAT_TOLERANCE)
			{
				hmi_engine_get_scale_rect(&hmi_screen_rect,pfather_alpha_scale,&hmi_scale_rect);
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
									&hmi_scale_rect,
									&(new_cliped_farther_rect ));
			}
			else
			{
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
									&hmi_screen_rect,
									&(new_cliped_farther_rect ));
			}
			hmi_get_intersec_rect_depth((HMI_RECT_STR CONST *)(&(new_cliped_farther_rect)),
								depth,(HMI_RECT_STR  *)pdirty_rect,&hmi_clip_rect);			
			#endif
			#ifdef HMI_GRAPHIC_AGG
			call_C_hmi_driver_draw_scrollbar(&(hmi_screen_rect),
								hmi_bigbitmap_table[0],hmi_bigbitmap_table[1],
								&hmi_sxy_scrollbar_table[hmi_object_index],
								hmi_sxy_scrollbar_cur_range[hmi_object_index],
								&hmi_clip_rect);
			hmi_engine_draw_container(&hmi_sxy_scrollbar_container_table[hmi_object_index],
				(HMI_RECT_STR CONST *)&(hmi_screen_rect),
				pdirty_rect);
			#endif

			#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
			#if HMI_SCROLLBAR_MAX_STATUS >= 0
			if((hmi_clip_rect.w != 0)&&
					(hmi_clip_rect.h != 0))
			{
				imagelist_date =(hmi_object_index*HMI_SCROLLBAR_MAX_STATUS_BIT_LEN);
				value_index		= imagelist_date>>3/*/8*/;
				value_beg_bits	= imagelist_date&0x07/*%8*/;
				value_imglist	= hmi_sxy_scrollbar_cur_range[value_index];
				value_imglist	= value_imglist<<(8-(value_beg_bits+HMI_SCROLLBAR_MAX_STATUS_BIT_LEN));
				value_imglist	= value_imglist>>(8-HMI_SCROLLBAR_MAX_STATUS_BIT_LEN);
				
				if(hmi_object_index < HMI_SXY_SCROLLBAR_NUMBER)
				{
					call_C_hmi_driver_draw_scrollbar(&(hmi_screen_rect)/*node screen position*/,
							hmi_sxy_scrollbar_attr_table[hmi_object_index]/*image format*/,						
							value_imglist/*current value*/,
							&hmi_clip_rect,
							pdirty_rect/*clip zone*/,
							(HMI_SCROLL_BAR_STR	*)(&hmi_sxy_scrollbar_table[hmi_object_index])/*scrollbar info*/,
							depth/*gdi layer*/,
							hmi_object_id,
							pfather_alpha_scale);
					#if HMI_SXY_SCROLLBAR_MAX_SON_CNT > 0
					hmi_engine_draw_container(&hmi_sxy_scrollbar_container_table[hmi_object_index],
						(HMI_RECT_STR CONST *)&(hmi_screen_rect ),pdirty_rect,depth,
						&new_cliped_farther_rect/*all father insection zone*/,
						&container_alpha_scale);
					#endif
				}
			}
			#endif
			#endif

			#ifdef HMI_GRAPHIC_ST7513
			#if HMI_SCROLLBAR_MAX_STATUS >= 0
			value_index		= (hmi_object_index*HMI_SCROLLBAR_MAX_STATUS_BIT_LEN)>>3/*/8*/;
			value_beg_bits	= (hmi_object_index*HMI_SCROLLBAR_MAX_STATUS_BIT_LEN)&0x07/*%8*/;
			value_imglist	= hmi_sxy_scrollbar_cur_range[value_index];
			value_imglist	= value_imglist<<(8-(value_beg_bits+HMI_SCROLLBAR_MAX_STATUS_BIT_LEN));
			value_imglist	= value_imglist>>(8-HMI_SCROLLBAR_MAX_STATUS_BIT_LEN);
			if(hmi_object_index < HMI_SXY_SCROLLBAR_NUMBER)
			{
				call_C_hmi_driver_draw_scrollbar((HMI_RECT_STR CONST *)(&(hmi_screen_rect))/*node screen position*/,						
						value_imglist/*current value*/,
						&hmi_clip_rect/*clip zone*/,
						(HMI_SCROLL_BAR_STR	*)(&hmi_sxy_scrollbar_table[hmi_object_index])/*scrollbar info*/);					
				hmi_engine_draw_container((HMI_CONTAINER_STR CONST *)(&hmi_sxy_scrollbar_container_table[hmi_object_index]),
					(HMI_RECT_STR CONST *)(&(hmi_screen_rect )),pdirty_rect,depth,
					&new_cliped_farther_rect/*all father insection zone*/);
			}
			#endif
			#endif
			
			#ifdef HMI_GRAPHIC_TWLIB
			/*
			call_C_hmi_driver_draw_scrollbar(&(text_prop.text_rect),hmi_bigbitmap_table[0],hmi_bigbitmap_table[1],&hmi_sxy_scrollbar_table[hmi_object_id],hmi_sxy_scrollbar_cur_range[hmi_object_id],&hmi_draw_rect);
			hmi_engine_draw_container(&hmi_sxy_scrollbar_container_table[hmi_object_id],(HMI_RECT_STR CONST *)&(text_prop.text_rect),HMI_SSCROLLBAR ,pdirty_rect);
			*/ /*tw not support scrollbar*/
			#endif
			#if (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
			/*tw not support scrollbar*/
			#endif
		}
		else
		#endif
		#if HMI_DXY_BUTTON_NUMBER > 0  
		if(HMI_IS_DYN_BUTTON_DXY(hmi_object_id))
	    {
			hmi_object_index = HMI_GET_DYN_BUTTON_DXY_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DXY_BUTTON_NUMBER)
			{
				hmi_rect_temp.x	=	hmi_dxy_button_rect[hmi_object_index].x;
				hmi_rect_temp.y	=	hmi_dxy_button_rect[hmi_object_index].y;
				hmi_rect_temp.w	=	hmi_dxy_button_rect[hmi_object_index].w;
				hmi_rect_temp.h	=	hmi_dxy_button_rect[hmi_object_index].h;
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
					(&hmi_rect_temp),
					(&(hmi_screen_rect)));
				#if (defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_RGL))
				/*element draw rect backups*/
				hmi_dxy_button_rect_bck[hmi_object_index].x	= hmi_rect_temp.x;
				hmi_dxy_button_rect_bck[hmi_object_index].y	= hmi_rect_temp.y;
				hmi_dxy_button_rect_bck[hmi_object_index].w	= hmi_rect_temp.w;
				hmi_dxy_button_rect_bck[hmi_object_index].h	= hmi_rect_temp.h;
				#endif
			}
			#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||\
				defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||\
				defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||\
				defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))
			if((fabs(pfather_alpha_scale->scale - HMI_SCALE_1)) > HMI_FLOAT_TOLERANCE)
			{
				hmi_engine_get_scale_rect(&hmi_screen_rect,pfather_alpha_scale,&hmi_scale_rect);
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
									&hmi_scale_rect,
									&(new_cliped_farther_rect ));
			}
			else
			{
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
									&hmi_screen_rect,
									&(new_cliped_farther_rect ));
			}
			hmi_get_intersec_rect_depth((HMI_RECT_STR CONST *)(&(new_cliped_farther_rect)),
								depth,(HMI_RECT_STR  *)pdirty_rect,&hmi_clip_rect);			
			#endif
			#ifdef HMI_GRAPHIC_AGG
			call_C_hmi_driver_draw_imagelist(&(hmi_screen_rect),
							hmi_bigbitmap_table[0],hmi_bigbitmap_table[1],
							&hmi_dxy_button_table[hmi_object_index].button_image,
							hmi_dxy_button_press_status[hmi_object_index],
							&hmi_clip_rect);
			
			hmi_engine_draw_container(&hmi_dxy_button_container_table[hmi_object_index],
						(HMI_RECT_STR CONST *)&(hmi_screen_rect ),
						,pdirty_rect,depth,&new_cliped_farther_rect/*all father insection zone*/);
			#endif	

			#if (defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_RGL))	
			if(hmi_object_index < HMI_DXY_BUTTON_NUMBER)
			{
				#if HMI_BTN_MAX_STATUS > 0
				if((hmi_clip_rect.w != 0)&&
					(hmi_clip_rect.h != 0))
				{
					imagelist_date	= (hmi_object_index*HMI_BTN_MAX_STATUS_BIT_LEN);
					value_index		= imagelist_date>>3/*/8*/;
					value_beg_bits	= imagelist_date&0x07/*%8*/;
					value_imglist	= hmi_dxy_button_press_status[value_index];			
					value_imglist	= value_imglist<<(8-(value_beg_bits+HMI_BTN_MAX_STATUS_BIT_LEN));
					value_imglist	= value_imglist>>(8-HMI_BTN_MAX_STATUS_BIT_LEN);
					call_C_hmi_driver_draw_imagelist(&(hmi_screen_rect),
													hmi_dxy_button_attr_table[hmi_object_index],
													(HMI_RECT_ALPHA_STR *)(&hmi_dxy_button_rect[hmi_object_index]),
													&(hmi_dxy_button_table[hmi_object_index].button_image),
													value_imglist,
													&hmi_clip_rect,
													pdirty_rect,
													depth,
													hmi_object_id,
													pfather_alpha_scale);
					#if HMI_DXY_BUTTON_MAX_SON_CNT > 0
					if(container_alpha_scale.alpha> hmi_dxy_button_rect[hmi_object_index].alpha)
					{
						container_alpha_scale.alpha= hmi_dxy_button_rect[hmi_object_index].alpha;
					}
					hmi_engine_draw_container(&(hmi_dxy_button_container_table[hmi_object_index]),
												(HMI_RECT_STR CONST *)(&(hmi_screen_rect )),
												pdirty_rect,
												depth,&new_cliped_farther_rect/*all father insection zone*/
												,&container_alpha_scale);
					#endif
				}
				#endif
			}
			#endif

			#ifdef HMI_GRAPHIC_ST7513									
			if(hmi_object_index < HMI_DXY_BUTTON_NUMBER)
			{
				#if HMI_BTN_MAX_STATUS > 0
				value_index		= (hmi_object_index*HMI_BTN_MAX_STATUS_BIT_LEN)>>3/*/8*/;
				value_beg_bits	= (hmi_object_index*HMI_BTN_MAX_STATUS_BIT_LEN)&0x07/*%8*/;
				value_imglist	= hmi_dxy_button_press_status[value_index];			
				value_imglist	= value_imglist<<(8-(value_beg_bits+HMI_BTN_MAX_STATUS_BIT_LEN));
				value_imglist	= value_imglist>>(8-HMI_BTN_MAX_STATUS_BIT_LEN);
				call_C_hmi_driver_draw_imagelist(&(hmi_screen_rect),																								
												&(hmi_dxy_button_table[hmi_object_index].button_image),
												value_imglist,
												&hmi_clip_rect);
												
				hmi_engine_draw_container(&(hmi_dxy_button_container_table[hmi_object_index]),
											(HMI_RECT_STR CONST *)(&(hmi_screen_rect )),
											pdirty_rect,
											depth,&new_cliped_farther_rect/*all father insection zone*/);
				#endif
			}
			#endif
			
			#ifdef HMI_GRAPHIC_TWLIB	
			if(hmi_object_index < HMI_DXY_BUTTON_NUMBER)
			{
				#if HMI_BTN_MAX_STATUS > 0
				bMerge			= hmi_get_is_merge(parent_object_id_type,HMI_ELEM_TYPE_DBTN);
				value_index		= (hmi_object_index*HMI_BTN_MAX_STATUS_BIT_LEN)/8;
				value_beg_bits	= (hmi_object_index*HMI_BTN_MAX_STATUS_BIT_LEN)%8;
				value_imglist	= hmi_dxy_button_press_status[value_index];
				value_imglist	= value_imglist<<(8-(value_beg_bits+HMI_BTN_MAX_STATUS_BIT_LEN));
				value_imglist	= value_imglist>>(8-HMI_BTN_MAX_STATUS_BIT_LEN);
				#if HMI_RENDER_ALL_EXCEPT_BCK==NO
				if(((!only_draw_flag)||!hmi_is_render_mode())||/*simulate*/
								(only_draw_flag&&(flag)))
				#endif
				{
					call_C_hmi_driver_draw_imagelist(&(hmi_screen_rect),(HMI_RECT_ALPHA_STR *)(&hmi_dxy_button_rect[hmi_object_index]),
													&(hmi_dxy_button_table[hmi_object_index].button_image),
													value_imglist/*,
													TRUE dxy*/,bMerge,
													TRUE/*Button*/
													#if HMI_RENDER_ALL_EXCEPT_BCK==NO
													,parent_object_id
													,hmi_object_id_const
													#endif
													);				
				}
				if(hmi_dxy_button_container_table[hmi_object_index].container_object_table.object_number>0)
				{
					hmi_engine_draw_container(&(hmi_dxy_button_container_table[hmi_object_index]),
												(HMI_RECT_STR CONST *)(&(hmi_screen_rect)),
												HMI_ELEM_TYPE_DBTN,
												hmi_object_id_const,
												FALSE
												#if HMI_RENDER_ALL_EXCEPT_BCK==NO
												,phmi_object_prop_table->object_id 
												,only_draw_flag&&(!flag)
												#endif
												);
				}
				#endif
			}
			#endif
			
			#if (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
			if(hmi_object_index < HMI_DXY_BUTTON_NUMBER)
			{
				#if HMI_BTN_MAX_STATUS > 0
				if(flag || draw_flag)
				{
					value_index		= (hmi_object_index*HMI_BTN_MAX_STATUS_BIT_LEN)/8;
					value_beg_bits	= (hmi_object_index*HMI_BTN_MAX_STATUS_BIT_LEN)%8;
					value_imglist	= hmi_dxy_button_press_status[value_index];			
					value_imglist	= value_imglist<<(8-(value_beg_bits+HMI_BTN_MAX_STATUS_BIT_LEN));
					value_imglist	= value_imglist>>(8-HMI_BTN_MAX_STATUS_BIT_LEN);
					call_C_hmi_driver_draw_imagelist(&(hmi_screen_rect),
													&hmi_dxy_button_attr_table[hmi_object_index],
													(HMI_RECT_ALPHA_ANGEL_STR *)(&hmi_dxy_button_rect[hmi_object_index]),
													&(hmi_dxy_button_table[hmi_object_index].button_image),
													value_imglist
													);
				}
				hmi_set_cur_win_used();
				
				#if HMI_DXY_BUTTON_MAX_SON_CNT > 0
				hmi_engine_draw_container(&(hmi_dxy_button_container_table[hmi_object_index]),
											(HMI_RECT_STR CONST *)(&(hmi_screen_rect )),
											child_draw_flag
											);
				#endif
				
				#endif
			}
			#endif
		}		
		else
		#endif
		
		#if HMI_SXY_BUTTON_NUMBER>0 		
		if(HMI_IS_DYN_BUTTON_SXY(hmi_object_id))
	    {
			hmi_object_index = HMI_GET_DYN_BUTTON_SXY_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_SXY_BUTTON_NUMBER)
			{
				hmi_rect_temp.x	=	hmi_sxy_button_rect[hmi_object_index].x;
				hmi_rect_temp.y	=	hmi_sxy_button_rect[hmi_object_index].y;
				hmi_rect_temp.w	=	hmi_sxy_button_rect[hmi_object_index].w;
				hmi_rect_temp.h	=	hmi_sxy_button_rect[hmi_object_index].h;
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
											(&hmi_rect_temp),
											(&(hmi_screen_rect)));				
			}
			
			#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||\
				defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||\
				defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||\
				defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))
			if((fabs(pfather_alpha_scale->scale - HMI_SCALE_1)) > HMI_FLOAT_TOLERANCE)
			{
				hmi_engine_get_scale_rect(&hmi_screen_rect,pfather_alpha_scale,&hmi_scale_rect);
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
									&hmi_scale_rect,
									&(new_cliped_farther_rect ));
			}
			else
			{
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
									&hmi_screen_rect,
									&(new_cliped_farther_rect ));
			}
			hmi_get_intersec_rect_depth((HMI_RECT_STR CONST *)(&(new_cliped_farther_rect)),
								depth,(HMI_RECT_STR  *)pdirty_rect,&hmi_clip_rect);
			#endif
			#ifdef HMI_GRAPHIC_AGG
			call_C_hmi_driver_draw_imagelist(&(hmi_screen_rect),
											hmi_bigbitmap_table[0],
											hmi_bigbitmap_table[1],
											&hmi_sxy_button_table[hmi_object_index].button_image,
											hmi_sxy_button_press_status[hmi_object_index],
											&hmi_clip_rect);
			
			#endif

			#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
			#if HMI_BTN_MAX_STATUS > 0
			if((hmi_clip_rect.w != 0)&&
					(hmi_clip_rect.h != 0))
			{
				imagelist_date	= (hmi_object_index*HMI_BTN_MAX_STATUS_BIT_LEN);
				value_index		= imagelist_date>>3/*/8*/;
				value_beg_bits	= imagelist_date&0x07/*%8*/;
				value_imglist	= hmi_sxy_button_press_status[value_index];
				value_imglist	= value_imglist<<(8-(value_beg_bits+HMI_BTN_MAX_STATUS_BIT_LEN));
				value_imglist	= value_imglist>>(8-HMI_BTN_MAX_STATUS_BIT_LEN);
				if(hmi_object_index < HMI_SXY_BUTTON_NUMBER)
				{
					call_C_hmi_driver_draw_imagelist(&(hmi_screen_rect),
													hmi_sxy_button_attr_table[hmi_object_index],
													(HMI_RECT_ALPHA_STR *)(&hmi_sxy_button_rect[hmi_object_index]),
													&(hmi_sxy_button_table[hmi_object_index].button_image),
													value_imglist,
													&hmi_clip_rect,
													pdirty_rect,
													depth,
													hmi_object_id,
													pfather_alpha_scale);
					#if HMI_SXY_BUTTON_MAX_SON_CNT > 0
					if(container_alpha_scale.alpha > hmi_sxy_button_rect[hmi_object_index].alpha)
					{
						container_alpha_scale.alpha = hmi_sxy_button_rect[hmi_object_index].alpha;
					}
					hmi_engine_draw_container(&(hmi_sxy_button_container_table[hmi_object_index]),
												(HMI_RECT_STR CONST *)(&(hmi_screen_rect )),
												pdirty_rect,
												depth,&new_cliped_farther_rect/*all father insection zone*/
												,&container_alpha_scale);
					#endif
				}
			}
			#endif
			#endif

			#ifdef HMI_GRAPHIC_ST7513
			#if HMI_BTN_MAX_STATUS > 0
			value_index		= (hmi_object_index*HMI_BTN_MAX_STATUS_BIT_LEN)>>3/*/8*/;
			value_beg_bits	= (hmi_object_index*HMI_BTN_MAX_STATUS_BIT_LEN)&0x07/*%8*/;
			value_imglist	= hmi_sxy_button_press_status[value_index];
			value_imglist	= value_imglist<<(8-(value_beg_bits+HMI_BTN_MAX_STATUS_BIT_LEN));
			value_imglist	= value_imglist>>(8-HMI_BTN_MAX_STATUS_BIT_LEN);
			if(hmi_object_index < HMI_SXY_BUTTON_NUMBER)
			{
				call_C_hmi_driver_draw_imagelist(&(hmi_screen_rect),												
												&(hmi_sxy_button_table[hmi_object_index].button_image),
												value_imglist,
												&hmi_clip_rect);												
				hmi_engine_draw_container(&(hmi_sxy_button_container_table[hmi_object_index]),
											(HMI_RECT_STR CONST *)(&(hmi_screen_rect)),
											pdirty_rect,
											depth,&new_cliped_farther_rect/*all father insection zone*/);
			}
			#endif
			#endif
			
			#ifdef HMI_GRAPHIC_TWLIB
			#if HMI_BTN_MAX_STATUS > 0
			bMerge			= hmi_get_is_merge(parent_object_id_type,HMI_ELEM_TYPE_SBTN);
			value_index		= (hmi_object_index*HMI_BTN_MAX_STATUS_BIT_LEN)>>3/*/8*/;
			value_beg_bits	= (hmi_object_index*HMI_BTN_MAX_STATUS_BIT_LEN)&0x07/*%8*/;
			value_imglist	= hmi_sxy_button_press_status[value_index];
			value_imglist	= value_imglist<<(8-(value_beg_bits+HMI_BTN_MAX_STATUS_BIT_LEN));
			value_imglist	= value_imglist>>(8-HMI_BTN_MAX_STATUS_BIT_LEN);
			#if HMI_RENDER_ALL_EXCEPT_BCK==NO
			if(((!only_draw_flag)||!hmi_is_render_mode())||/*simulate*/
								(only_draw_flag&&(flag)))
			#endif
			{
				if(hmi_object_index < HMI_SXY_BUTTON_NUMBER)
				{
					call_C_hmi_driver_draw_imagelist(&(hmi_screen_rect),(HMI_RECT_ALPHA_STR *)(&hmi_sxy_button_rect[hmi_object_index]),
													&(hmi_sxy_button_table[hmi_object_index].button_image),
													value_imglist/*,
													FALSE sxy*/,bMerge,
													TRUE/*Button*/
													#if HMI_RENDER_ALL_EXCEPT_BCK==NO
													,parent_object_id
													,hmi_object_id_const
													#endif
													);					
				}
			}
			if(hmi_object_index < HMI_SXY_BUTTON_NUMBER)
			{
				if(hmi_sxy_button_container_table[hmi_object_index].container_object_table.object_number>0)
				{
					hmi_engine_draw_container(&(hmi_sxy_button_container_table[hmi_object_index]),
														(HMI_RECT_STR CONST *)(&(hmi_screen_rect)),
														HMI_ELEM_TYPE_SBTN,
														hmi_object_id_const,
														FALSE
														#if HMI_RENDER_ALL_EXCEPT_BCK==NO
														,phmi_object_prop_table->object_id 
														,only_draw_flag&&(!flag)
														#endif
														);
				}
			}
			#endif
			#endif
			
			#if (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
			#if HMI_BTN_MAX_STATUS > 0
			if(flag || draw_flag)
			{
				value_index		= (hmi_object_index*HMI_BTN_MAX_STATUS_BIT_LEN)/8;
				value_beg_bits	= (hmi_object_index*HMI_BTN_MAX_STATUS_BIT_LEN)%8;
				value_imglist	= hmi_sxy_button_press_status[value_index];
				value_imglist	= value_imglist<<(8-(value_beg_bits+HMI_BTN_MAX_STATUS_BIT_LEN));
				value_imglist	= value_imglist>>(8-HMI_BTN_MAX_STATUS_BIT_LEN);
				if(hmi_object_index < HMI_SXY_BUTTON_NUMBER)
				{
					call_C_hmi_driver_draw_imagelist(&(hmi_screen_rect),
													&hmi_sxy_button_attr_table[hmi_object_index],
													(HMI_RECT_ALPHA_ANGEL_STR *)(&hmi_sxy_button_rect[hmi_object_index]),
													&(hmi_sxy_button_table[hmi_object_index].button_image),
													value_imglist
													);
				}
			}
			hmi_set_cur_win_used();
			#if HMI_SXY_BUTTON_MAX_SON_CNT > 0	
			hmi_engine_draw_container(&(hmi_sxy_button_container_table[hmi_object_index]),
										(HMI_RECT_STR CONST *)(&(hmi_screen_rect )),
										child_draw_flag
										);
			#endif
			#endif
			#endif						
		}		
		else 		
		#endif
		#if HMI_DYN_EDIT_TEXTS_NUMBER/*editable text*/ > 0	   
		if(HMI_IS_DYN_TEXTS(hmi_object_id))
		{	         
			#if HMI_DYN_XY_EDIT_TEXTS_NUMBER > 0
			if(HMI_IS_DYN_TEXT_DXY(hmi_object_id))
		   	{
				hmi_object_index = HMI_GET_DYN_TEXTS_DXY_POS_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_DYN_XY_EDIT_TEXTS_NUMBER)
				{
					hmi_get_object_screen_coor((HMI_RECT_STR CONST *)pfarther_rect,
											&hmi_dyn_xy_edit_text_prop_table[hmi_object_index].text_rect,
											&(hmi_screen_rect));
					#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG))
					/*element draw rect backups*/
					hmi_dyn_xy_edit_text_prop_table_bck[hmi_object_index].x	= hmi_dyn_xy_edit_text_prop_table[hmi_object_index].text_rect.x;
					hmi_dyn_xy_edit_text_prop_table_bck[hmi_object_index].y	= hmi_dyn_xy_edit_text_prop_table[hmi_object_index].text_rect.y;
					hmi_dyn_xy_edit_text_prop_table_bck[hmi_object_index].w	= hmi_dyn_xy_edit_text_prop_table[hmi_object_index].text_rect.w;
					hmi_dyn_xy_edit_text_prop_table_bck[hmi_object_index].h	= hmi_dyn_xy_edit_text_prop_table[hmi_object_index].text_rect.h;
					#endif
				}
				
				#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||\
					defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||\
					defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||\
					defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))
				if((fabs(pfather_alpha_scale->scale - HMI_SCALE_1)) > HMI_FLOAT_TOLERANCE)
				{
					hmi_engine_get_scale_rect(&hmi_screen_rect,pfather_alpha_scale,&hmi_scale_rect);
					hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
										&hmi_scale_rect,
										&(new_cliped_farther_rect ));
				}
				else
				{
					hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
										&hmi_screen_rect,
										&(new_cliped_farther_rect ));
				}
				hmi_get_intersec_rect_depth((HMI_RECT_STR CONST *)(&(new_cliped_farther_rect)),
												depth,(HMI_RECT_STR  *)pdirty_rect,&hmi_clip_rect);												
				#endif
				text_prop.text_rect.x	= hmi_screen_rect.x;
				text_prop.text_rect.y	= hmi_screen_rect.y;
				text_prop.text_rect.w	= hmi_screen_rect.w;
				text_prop.text_rect.h	= hmi_screen_rect.h;
				if(hmi_object_index < HMI_DYN_XY_EDIT_TEXTS_NUMBER)
				{
					text_prop.color = hmi_dyn_xy_edit_text_prop_table[hmi_object_index].color;
				}
				#if HMI_EDIT_TEXT_DXY_DYN_FONT_NUM > 0
				if(HMI_IS_DYN_TEXT_DXY_DYN_FONT(hmi_object_id))
				{
					hmi_object_index	= HMI_GET_DYN_TEXTS_DXY_DYN_FONT_POS_INDEX(hmi_object_id);
					if(hmi_object_index < HMI_EDIT_TEXT_DXY_DYN_FONT_NUM)
					{
						font_id	= hmi_edit_text_dxy_dyn_font_table[hmi_object_index];
					}
				}
				#endif
								
				#ifdef HMI_GRAPHIC_AGG
				call_C_hmi_driver_draw_text(
	                     &text_prop,
	                     &hmi_edit_text_table[hmi_object_index],
	                     &hmi_clip_rect
	                     #ifdef HMI_CLIP_TEXT 
						,hmi_clip_text
						#endif
							);
				#if HMI_EDIT_TEXT_MAX_SON_CNT > 0
				hmi_engine_draw_container(&hmi_edit_text_container_table[hmi_object_index],
										(HMI_RECT_STR CONST *)(&(hmi_screen_rect)),										
										pdirty_rect,
										depth,&new_cliped_farther_rect/*all father insection zone*/); 
				#endif
				#endif	

				#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
				if((hmi_clip_rect.w != 0)&&
					(hmi_clip_rect.h != 0))
				{
					hmi_object_index = HMI_GET_DYN_TEXTS_INDEX(hmi_object_id);
					if(hmi_object_index < HMI_DYN_EDIT_TEXTS_NUMBER)
					{
						#if HMI_ALL_FONT_NUMBER > 0
						call_C_hmi_driver_draw_text(
			                     &text_prop,
			                     &hmi_edit_text_table[hmi_object_index],
			                     &hmi_clip_rect,
			                     pdirty_rect,
			                     font_id,
			                     depth
			                     #ifdef HMI_CLIP_TEXT 
								,hmi_clip_text
								#endif
								,pfather_alpha_scale
								);
						#endif
						hmi_alpha	= HMI_RGL_ALLPHA(text_prop.color);
						if(container_alpha_scale.alpha > hmi_alpha)
						{
							container_alpha_scale.alpha = hmi_alpha;
						}
						#if HMI_EDIT_TEXT_MAX_SON_CNT > 0
						hmi_engine_draw_container(&hmi_edit_text_container_table[hmi_object_index],
												(HMI_RECT_STR CONST *)(&(hmi_screen_rect )),
												pdirty_rect,
												depth,&new_cliped_farther_rect/*all father insection zone*/
												,&container_alpha_scale);
						#endif
					}
				}
				#endif	

				#ifdef HMI_GRAPHIC_ST7513
				hmi_object_index = HMI_GET_DYN_TEXTS_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_DYN_EDIT_TEXTS_NUMBER)
				{
					call_C_hmi_driver_draw_text(
		                     &text_prop,
		                     &hmi_edit_text_table[hmi_object_index],
		                     &hmi_clip_rect,
		                     font_id
		                     #ifdef HMI_CLIP_TEXT 
							,hmi_clip_text
							#endif
								);
					#if HMI_EDIT_TEXT_MAX_SON_CNT > 0
					hmi_engine_draw_container(&hmi_edit_text_container_table[hmi_object_index],
											(HMI_RECT_STR CONST *)(&(hmi_screen_rect )),
											pdirty_rect,
											depth,&new_cliped_farther_rect/*all father insection zone*/);
					#endif
				}
				#endif	
				
				#ifdef HMI_GRAPHIC_TWLIB	
				hmi_object_index = HMI_GET_DYN_TEXTS_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_DYN_EDIT_TEXTS_NUMBER)
				{
					#if HMI_RENDER_ALL_EXCEPT_BCK==NO
					if(((!only_draw_flag)||!hmi_is_render_mode())||/*simulate*/
								(only_draw_flag&&(flag)))
					#endif
					{
						call_C_hmi_driver_draw_text(
			                     &text_prop,
			                     &hmi_edit_text_table[hmi_object_index]
			                     #if HMI_RENDER_ALL_EXCEPT_BCK==NO
									,parent_object_id
									,hmi_object_id_const
								#endif
			                     );	
					}
				}
				#endif
				
				#if (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
				#if HMI_ALL_FONT_NUMBER > 0
				hmi_object_index = HMI_GET_DYN_TEXTS_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_DYN_EDIT_TEXTS_NUMBER)
				{
					if(flag || draw_flag)
					{
						if((hmi_edit_text_table[hmi_object_index].properties & HMI_TEXT_PROP_SCROLABLE) != 0)
						{
							hmi_driver_setting_clipping_layer(&(text_prop.text_rect),HMI_CLIPPING_LAYER);
						}
					}
					
					if((hmi_edit_text_table[hmi_object_index].properties & HMI_TEXT_PROP_SCROLABLE) != 0)
					{
						hmi_set_cur_win_used();		
					}
					
					if(flag || draw_flag)
					{
						call_C_hmi_driver_draw_text(
			                     &text_prop,
			                     &hmi_edit_text_table[hmi_object_index],
			                     font_id
								#ifdef HMI_CLIP_TEXT 
								,hmi_clip_text
								#endif
								);
						
					}
				}
				
				hmi_set_cur_win_used();
				hmi_set_text_cur_index_used();
				#endif
				#if HMI_EDIT_TEXT_MAX_SON_CNT > 0
				hmi_engine_draw_container(&hmi_edit_text_container_table[hmi_object_index],
										(HMI_RECT_STR CONST *)(&(hmi_screen_rect)),
										child_draw_flag);
				#endif
				#endif	
			}
			else
			#endif		 	
			#if HMI_S_XY_EDIT_TEXTS_NUMBER > 0
			if(HMI_IS_DYN_TEXT_SXY(hmi_object_id))
			{
				hmi_object_index = HMI_GET_DYN_TEXTS_SXY_POS_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_S_XY_EDIT_TEXTS_NUMBER)
				{
					hmi_get_object_screen_coor((HMI_RECT_STR CONST *)pfarther_rect,
												&hmi_static_xy_edit_text_prop_table[hmi_object_index].text_rect,
												&(hmi_screen_rect));					
				}
				#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||\
					defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||\
					defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||\
					defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))
				if((fabs(pfather_alpha_scale->scale - HMI_SCALE_1)) > HMI_FLOAT_TOLERANCE)
				{
					hmi_engine_get_scale_rect(&hmi_screen_rect,pfather_alpha_scale,&hmi_scale_rect);
					hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
										&hmi_scale_rect,
										&(new_cliped_farther_rect ));
				}
				else
				{
					hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
										&hmi_screen_rect,
										&(new_cliped_farther_rect ));
				}
				hmi_get_intersec_rect_depth((HMI_RECT_STR CONST *)(&(new_cliped_farther_rect)),
									depth,(HMI_RECT_STR  *)pdirty_rect,&hmi_clip_rect);													
				#endif
				text_prop.text_rect.x	= hmi_screen_rect.x;
				text_prop.text_rect.y	= hmi_screen_rect.y;
				text_prop.text_rect.w	= hmi_screen_rect.w;
				text_prop.text_rect.h	= hmi_screen_rect.h;
				if(hmi_object_index < HMI_S_XY_EDIT_TEXTS_NUMBER)
				{
					text_prop.color = hmi_static_xy_edit_text_prop_table[hmi_object_index].color;
				}

				#if HMI_EDIT_TEXT_SXY_DYN_FONT_NUM > 0
				if(HMI_IS_DYN_TEXT_SXY_DYN_FONT(hmi_object_id))
				{
					hmi_object_index	= HMI_GET_DYN_TEXTS_SXY_DYN_FONT_POS_INDEX(hmi_object_id);
					if(hmi_object_index < HMI_EDIT_TEXT_SXY_DYN_FONT_NUM)
					{
						font_id	= hmi_edit_text_sxy_dyn_font_table[hmi_object_index];
					}
				}
				#endif
				
				#ifdef HMI_GRAPHIC_AGG
				hmi_object_index = HMI_GET_DYN_TEXTS_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_DYN_EDIT_TEXTS_NUMBER)
				{
		            call_C_hmi_driver_draw_text(&text_prop,
												&hmi_edit_text_table[hmi_object_index],
												&hmi_clip_rect
											#ifdef HMI_CLIP_TEXT 
												,hmi_clip_text
											#endif
												);
					#if HMI_EDIT_TEXT_MAX_SON_CNT > 0
					hmi_engine_draw_container(&hmi_edit_text_container_table[hmi_object_index],
											(HMI_RECT_STR CONST *)(&(hmi_screen_rect )),
											pdirty_rect,
											depth,&new_cliped_farther_rect/*all father insection zone*/);
					#endif
				}
				#endif	

				#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
				if((hmi_clip_rect.w != 0)&&
					(hmi_clip_rect.h != 0))
				{
					hmi_object_index = HMI_GET_DYN_TEXTS_INDEX(hmi_object_id);
					if(hmi_object_index < HMI_DYN_EDIT_TEXTS_NUMBER)
					{
						#if HMI_ALL_FONT_NUMBER > 0
			            call_C_hmi_driver_draw_text(&text_prop,
													&hmi_edit_text_table[hmi_object_index],
													&hmi_clip_rect,
													pdirty_rect,
													font_id,
													 depth
												#ifdef HMI_CLIP_TEXT 
													,hmi_clip_text
												#endif
												,pfather_alpha_scale
													);
						#endif
						hmi_alpha = HMI_RGL_ALLPHA(text_prop.color);
						if(container_alpha_scale.alpha > hmi_alpha)
						{
							container_alpha_scale.alpha = hmi_alpha;
						}
						#if HMI_EDIT_TEXT_MAX_SON_CNT > 0
						hmi_engine_draw_container(&hmi_edit_text_container_table[hmi_object_index],
												(HMI_RECT_STR CONST *)(&(hmi_screen_rect )),											
												pdirty_rect,
												depth,&new_cliped_farther_rect/*all father insection zone*/,
												&container_alpha_scale);
						#endif
					}
				}
				#endif	

				#ifdef HMI_GRAPHIC_ST7513
					hmi_object_index = HMI_GET_DYN_TEXTS_INDEX(hmi_object_id);
					if(hmi_object_index < HMI_DYN_EDIT_TEXTS_NUMBER)
					{
			            call_C_hmi_driver_draw_text(&text_prop,
													&hmi_edit_text_table[hmi_object_index],
													&hmi_clip_rect,
													font_id
												#ifdef HMI_CLIP_TEXT 
													,hmi_clip_text
												#endif
													);
						#if HMI_EDIT_TEXT_MAX_SON_CNT > 0
						hmi_engine_draw_container(&hmi_edit_text_container_table[hmi_object_index],
												(HMI_RECT_STR CONST *)(&(hmi_screen_rect)),											
												pdirty_rect,
												depth,&new_cliped_farther_rect/*all father insection zone*/);
						#endif
					}
				#endif	
				
				#ifdef HMI_GRAPHIC_TWLIB					
					hmi_object_index = HMI_GET_DYN_TEXTS_INDEX(hmi_object_id);
					if(hmi_object_index < HMI_DYN_EDIT_TEXTS_NUMBER)
					{
						#if HMI_RENDER_ALL_EXCEPT_BCK==NO
						if(((!only_draw_flag)||!hmi_is_render_mode())||/*simulate*/
								(only_draw_flag&&(flag)))
						#endif
						{
				            call_C_hmi_driver_draw_text(&text_prop,
														&hmi_edit_text_table[hmi_object_index]	
														#if HMI_RENDER_ALL_EXCEPT_BCK==NO
														,parent_object_id
														,hmi_object_id_const
														#endif
														);
						}
						#if 0
						#if HMI_EDIT_TEXT_MAX_SON_CNT > 0
						hmi_engine_draw_container(&hmi_edit_text_container_table[hmi_object_id],
												(HMI_RECT_STR CONST *)&(text_prop.text_rect),
												&hmi_union_rect,
												pdirty_rect);
						#endif
						#endif /*tw not support */
					}
				#endif	
				
				#if (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
				hmi_object_index = HMI_GET_DYN_TEXTS_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_DYN_EDIT_TEXTS_NUMBER)
				{
					#if HMI_ALL_FONT_NUMBER > 0
					if(flag || draw_flag)
					{
						if((hmi_edit_text_table[hmi_object_index].properties & HMI_TEXT_PROP_SCROLABLE) != 0)
						{
							hmi_driver_setting_clipping_layer(&(text_prop.text_rect),HMI_CLIPPING_LAYER);		
						}
					}
					if((hmi_edit_text_table[hmi_object_index].properties & HMI_TEXT_PROP_SCROLABLE) != 0)
					{
						hmi_set_cur_win_used();			
					}
					if(flag || draw_flag)
					{
						call_C_hmi_driver_draw_text(&text_prop,
													&hmi_edit_text_table[hmi_object_index],
													font_id
													#ifdef HMI_CLIP_TEXT 
													,hmi_clip_text
													#endif
													);
					}
					
					hmi_set_cur_win_used();
					hmi_set_text_cur_index_used();
					#endif
					#if HMI_EDIT_TEXT_MAX_SON_CNT > 0
					hmi_engine_draw_container(&hmi_edit_text_container_table[hmi_object_index],
												(HMI_RECT_STR CONST *)(&(hmi_screen_rect )),
												child_draw_flag
												);
					#endif
				}
				#endif	
			}
			else
			#endif
			{
			}
		}
		else
		#endif	 
		#if HMI_DYN_CONTAINERS_NUMBER > 0
		if(HMI_IS_DYN_CONTAINER(hmi_object_id))
		{	
			hmi_object_id  = HMI_GET_DYN_CONTAINERS_ID_INDEX(hmi_object_id);
			/*scene animal*/
			#if (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
			#if HMI_SCENE_DYN_CONTAINERS_NUMBER > 0
			if(hmi_object_id	< HMI_SCENE_DYN_CONTAINERS_NUMBER)
			{
				hmi_animat_action 	= hmi_dyn_container_table[hmi_object_id];
				hmi_animat_address 	= hmi_scene_data_table[hmi_object_id];
				hmi_driver_animat_action(hmi_animat_action,hmi_animat_address);
				if(flag)
				{
					HMI_DYNAMIC_OBJECT_CANCEL_CHANGED_FLAG(hmi_object_id );/*clear refresh flag*/
				}
			}
			else
			#endif
			#endif
			{
			#if HMI_ALL_STATIC_CONTAINERS_NUMBER > 0  
			#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
			/*dyn container id backups*/
			hmi_dyn_container_table_bck[hmi_object_id]	= hmi_dyn_container_table[hmi_object_id];
			#endif
			
			#if (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
			hmi_object_index =hmi_object_id;
			#if HMI_SCENE_DYN_CONTAINERS_NUMBER > 0
			if(hmi_object_id	> HMI_SCENE_DYN_CONTAINERS_NUMBER)
			{
				hmi_object_index =hmi_object_id -HMI_SCENE_DYN_CONTAINERS_NUMBER;
			}
			else
			{
				hmi_object_index =hmi_object_id;
			}
			#endif
			container_max_layer=hmi_get_cur_win();
			container_max_layer+=hmi_dyn_container_yamaha_prop[hmi_object_index].max_layer_nb;
			container_max_text_index=hmi_get_text_cur_number();
			container_max_text_index+=hmi_dyn_container_yamaha_prop[hmi_object_index].max_text_nb;
			#endif
			//hmi_engine_get_static_container_id(&hmi_object_id);/*get static container ID*/
			hmi_object_id = hmi_dyn_container_table[hmi_object_id];
			while((HMI_IS_DYN_CONTAINER(hmi_object_id))&&
				(loop < HMI_DYN_CONTAINER_NESTED_DEPTH)&&
				(hmi_object_id !=HMI_DYN_CONTAINER_IS_NULL))
			{
				hmi_object_id = HMI_GET_DYN_CONTAINERS_ID_INDEX(hmi_object_id);
				#if (defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(S6J3200_GRAPHIC))
				/*dyn container id backups*/
				hmi_dyn_container_table_bck[hmi_object_id]	= hmi_dyn_container_table[hmi_object_id];
				#endif
			#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
				HMI_DYNAMIC_OBJECT_CANCEL_CHANGED_FLAG(hmi_object_id );/*clear refresh flag*/
				#endif
				if((hmi_object_id) < HMI_DYN_CONTAINERS_NUMBER)
			   	{
					hmi_object_id= hmi_dyn_container_table[hmi_object_id];
			   	}/*lq 2017.6.12*/
				else
				{
					loop	= HMI_DYN_CONTAINER_NESTED_DEPTH;/*end loop*/
				}
				loop++;
			}
		
			#if defined(HMI_GRAPHIC_TWLIB) /*tw*/
			hmi_object_id_const = hmi_object_id;
			#endif

			if(hmi_object_id == HMI_DYN_CONTAINER_IS_NULL)
			{
			}
			#if HMI_DXY_CONTAINERS_NUMBER>0U
			else if(HMI_IS_DYN_XY_CONTAINER(hmi_object_id))
			{
				HMI_DYNAMIC_OBJECT_CANCEL_CHANGED_FLAG(hmi_object_id );/*clear refresh flag*/
				hmi_object_id = HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_object_id );
				if(hmi_object_id< HMI_DXY_CONTAINERS_NUMBER)
				{
					hmi_get_object_screen_coor((HMI_RECT_STR CONST *)pfarther_rect,
						&hmi_dyn_xy_container_rect[hmi_object_id],
						&(hmi_screen_rect));	
					#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
					/*element draw rect backups*/
					hmi_dyn_xy_container_rect_bck[hmi_object_id].x	= hmi_dyn_xy_container_rect[hmi_object_id].x;
					hmi_dyn_xy_container_rect_bck[hmi_object_id].y	= hmi_dyn_xy_container_rect[hmi_object_id].y;
					hmi_dyn_xy_container_rect_bck[hmi_object_id].w	= hmi_dyn_xy_container_rect[hmi_object_id].w;
					hmi_dyn_xy_container_rect_bck[hmi_object_id].h	= hmi_dyn_xy_container_rect[hmi_object_id].h;
					#endif
				}
				#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||\
					defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||\
					defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||\
					defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))
				if((fabs(pfather_alpha_scale->scale - HMI_SCALE_1)) > HMI_FLOAT_TOLERANCE)
				{
					hmi_engine_get_scale_rect(&hmi_screen_rect,pfather_alpha_scale,&hmi_scale_rect);
					hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
										&hmi_scale_rect,
										&(new_cliped_farther_rect ));
				}
				else
				{
					hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
										&hmi_screen_rect,
										&(new_cliped_farther_rect ));
				}
				hmi_get_intersec_rect_depth((HMI_RECT_STR CONST *)(&(new_cliped_farther_rect)),
								depth,(HMI_RECT_STR  *)pdirty_rect,&hmi_clip_rect);	
				
				#endif

				#ifdef HMI_GRAPHIC_AGG
				hmi_engine_draw_container(&hmi_dyn_xy_container_table[hmi_object_id],
						(HMI_RECT_STR CONST *)&(hmi_screen_rect),
						pdirty_rect,depth,&new_cliped_farther_rect/*all father insection zone*/);
				#endif
				
				#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
				if((hmi_clip_rect.w != 0)&&
					(hmi_clip_rect.h != 0))
				{
					if(hmi_dxy_container_video_fmt[hmi_object_id].video_fmt_channel&HMI_VIDEO_WINDOW)
					{
						hmi_display_video(&hmi_dxy_container_video_fmt[hmi_object_id],
										&hmi_dyn_xy_container_rect[hmi_object_id],
										&hmi_dxy_container_video_status[hmi_object_id],
										depth);
					}/*2016.9.7. for support video capture in*/
					/*dxy container scale */
					#if HMI_DXY_CONTAINERS_SCALE_ALPHA_NUMBER >0U
					if(hmi_object_id < HMI_DXY_CONTAINERS_SCALE_ALPHA_NUMBER)
					{
						hmi_dxy_container_scale	= hmi_dyn_xy_container_alpha_scale[hmi_object_id].scale;
						if((fabs(hmi_dxy_container_scale - HMI_SCALE_1)) > HMI_FLOAT_TOLERANCE)/*scale*/
						{
							if((fabs(pfather_alpha_scale->scale - HMI_SCALE_1)) < HMI_FLOAT_TOLERANCE)/*father no scale*/
							{
								container_alpha_scale.point.x = hmi_screen_rect.x+(hmi_screen_rect.w >> 1);
								container_alpha_scale.point.y = hmi_screen_rect.y+(hmi_screen_rect.h >> 1);
							}
							container_alpha_scale.scale = hmi_dxy_container_scale * pfather_alpha_scale->scale;
						}
						if(pfather_alpha_scale->alpha > hmi_dyn_xy_container_alpha_scale[hmi_object_id].alpha)
						{
							container_alpha_scale.alpha =hmi_dyn_xy_container_alpha_scale[hmi_object_id].alpha;
						}
					}
					#endif
				#if (defined(HMI_GRAPHIC_OPENGLES)) // 2023 02 15 lq
					if(!(hmi_dxy_container_video_fmt[hmi_object_id].
						video_fmt_channel & HMI_SCENE_WINDOW)) // support 3d scene 2023 02 15 lq
					{
						hmi_engine_draw_container(&hmi_dyn_xy_container_table[hmi_object_id],
										(HMI_RECT_STR CONST *)&(hmi_screen_rect ),
										pdirty_rect,depth,&new_cliped_farther_rect/*all father insection zone*/,&container_alpha_scale);
					}
					else	// support 3d scene 2023 02 15 lq
					{
					#if HMI_DXY_CUSTOM_CNT + HMI_SXY_CUSTOM_CNT >0U
						hmi_draw_opengl_scene(&new_cliped_farther_rect,
											hmi_object_id_const);
					#endif
						hmi_engine_draw_container(&hmi_dyn_xy_container_table[hmi_object_id],
										(HMI_RECT_STR CONST *)&(hmi_screen_rect ),
										pdirty_rect,depth,&new_cliped_farther_rect/*all father insection zone*/,&container_alpha_scale);
					}
				#else
					hmi_engine_draw_container(&hmi_dyn_xy_container_table[hmi_object_id],
										(HMI_RECT_STR CONST *)&(hmi_screen_rect ),
										pdirty_rect,depth,&new_cliped_farther_rect/*all father insection zone*/,&container_alpha_scale);
				#endif
				}
				#endif
				
				#ifdef HMI_GRAPHIC_ST7513
				hmi_engine_draw_container(&hmi_dyn_xy_container_table[hmi_object_id],
						(HMI_RECT_STR CONST *)&(hmi_screen_rect),
						pdirty_rect,depth,&new_cliped_farther_rect/*all father insection zone*/);
				#endif
				
				#ifdef HMI_GRAPHIC_TWLIB /*tw*/
				parent_object_id_type = HMI_ELEM_TYPE_DYN_CONTAINER;
				bMerge	= hmi_get_is_merge(HMI_ELEM_TYPE_DYN_CONTAINER/*parent_object_id_type*/,HMI_ELEM_TYPE_DXY_CONTINER);
				#if HMI_RENDER_ALL_EXCEPT_BCK==NO
				//if(((!only_draw_flag)||!hmi_is_render_mode())||
								//(only_draw_flag&&(flag)))
				#endif 
				{
					if(hmi_dyn_xy_container_table[hmi_object_id].container_object_table.object_number>0)
					{
						hmi_engine_draw_container(&(hmi_dyn_xy_container_table[hmi_object_id]),
													(HMI_RECT_STR CONST *)(&(hmi_screen_rect )),
													/*HMI_ELEM_TYPE_DYN_CONTAINER*/HMI_ELEM_TYPE_DXY_CONTINER,
													hmi_object_id_const,
													!bMerge
													#if HMI_RENDER_ALL_EXCEPT_BCK==NO
													,phmi_object_prop_table->object_id
													,only_draw_flag&&(!flag)
													#endif
													);
													
					}
				}
				#endif
				
				#if (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
				if(hmi_object_id< HMI_DXY_CONTAINERS_NUMBER)
				{
					hmi_engine_draw_container(&hmi_dyn_xy_container_table[hmi_object_id],
							(HMI_RECT_STR CONST *)(&(hmi_screen_rect )),
							child_draw_flag
							);
				}
				#endif
			}
			#endif	
			#if HMI_SXY_CONTAINERS_NUMBER>0
			else if(HMI_IS_CONTAINERS_SXY(hmi_object_id))
			{
				hmi_object_id = HMI_GET_CONTAINERS_SXY_ID_INDEX(hmi_object_id );
				if(hmi_object_id< HMI_SXY_CONTAINERS_NUMBER)
				{
					hmi_get_object_screen_coor((HMI_RECT_STR CONST *)pfarther_rect,
						&hmi_static_container_rect[hmi_object_id],&(hmi_screen_rect));					
				}
				#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||\
					defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||\
					defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||\
					defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))
				if((fabs(pfather_alpha_scale->scale - HMI_SCALE_1)) > HMI_FLOAT_TOLERANCE)
				{
					hmi_engine_get_scale_rect(&hmi_screen_rect,pfather_alpha_scale,&hmi_scale_rect);
					hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
										&hmi_scale_rect,
										&(new_cliped_farther_rect ));
				}
				else
				{
					hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
										&hmi_screen_rect,
										&(new_cliped_farther_rect ));
				}
				#if 0 //lq
				hmi_get_intersec_rect_depth((HMI_RECT_STR CONST *)(&(new_cliped_farther_rect)),
								depth,(HMI_RECT_STR  *)(&pdirty_rect[depth]),&hmi_clip_rect);
				#endif
				hmi_get_intersec_rect_depth((HMI_RECT_STR CONST *)(&(new_cliped_farther_rect)),
								depth,(HMI_RECT_STR  *)(pdirty_rect),&hmi_clip_rect);
				#endif

				#ifdef HMI_GRAPHIC_AGG
				if(hmi_object_id< HMI_SXY_CONTAINERS_NUMBER)
				{
					hmi_engine_draw_container(&hmi_sxy_container_table[hmi_object_id],
						(HMI_RECT_STR CONST *)&(hmi_screen_rect ),
						pdirty_rect,depth,&new_cliped_farther_rect/*all father insection zone*/);
				}
				#endif
				
				#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
				if(hmi_object_id < HMI_SXY_CONTAINERS_NUMBER)
				{
					if((hmi_clip_rect.w != 0)&&
					(hmi_clip_rect.h != 0))
					{
						hmi_engine_draw_container(&hmi_sxy_container_table[hmi_object_id],
							(HMI_RECT_STR CONST *)(&(hmi_screen_rect )),
							pdirty_rect,depth,&new_cliped_farther_rect/*all father insection zone*/,pfather_alpha_scale);
					}
				}
				#endif

				#ifdef HMI_GRAPHIC_ST7513
				if(hmi_object_id< HMI_SXY_CONTAINERS_NUMBER)
				{
					hmi_engine_draw_container(&hmi_sxy_container_table[hmi_object_id],
						(HMI_RECT_STR CONST *)(&(hmi_screen_rect )),
						pdirty_rect,depth,&new_cliped_farther_rect/*all father insection zone*/);
				}
				#endif
				
				#ifdef HMI_GRAPHIC_TWLIB /*tw*/
				bMerge=hmi_get_is_merge(HMI_ELEM_TYPE_DYN_CONTAINER/*parent_object_id_type*/,HMI_ELEM_TYPE_SXY_CONTAINER);
				#if HMI_RENDER_ALL_EXCEPT_BCK==NO
				//if(((!only_draw_flag)||!hmi_is_render_mode())||
								//(only_draw_flag&&(flag)))
				#endif
				{
					if(hmi_sxy_container_table[hmi_object_id].container_object_table.object_number>0)
					{
						hmi_engine_draw_container(&(hmi_sxy_container_table[hmi_object_id]),
													(HMI_RECT_STR CONST *)(&(hmi_screen_rect )),
													HMI_ELEM_TYPE_SXY_CONTAINER,
													hmi_object_id_const,
													!bMerge
													#if HMI_RENDER_ALL_EXCEPT_BCK==NO
													,phmi_object_prop_table->object_id 
													,only_draw_flag&&(!flag)
													#endif
													); 
					}
				}
				#endif
				
				#if (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
				if(hmi_object_id< HMI_SXY_CONTAINERS_NUMBER)
				{
					hmi_engine_draw_container(&hmi_sxy_container_table[hmi_object_id],
												(HMI_RECT_STR CONST *)(&(hmi_screen_rect )),
												child_draw_flag
												);
				}
				#endif
			}
			else
			#endif
			{
			}
			
			#if (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
			next_layer =hmi_get_cur_win();
			next_text_index=hmi_get_text_cur_number();
			if(container_max_layer >next_layer)
			{
				for(free_index=next_layer;free_index<container_max_layer;free_index++)
				{
					hmi_driver_free_layer(free_index);
				}
			}
			hmi_set_cur_win(container_max_layer);
			if(container_max_text_index !=next_text_index)
			{
				hmi_set_text_cur_number(container_max_text_index);
			}
			#endif
			#endif
			}
			
		}
		else
		#endif
		#if HMI_DYN_FILL_PAGES_NUMBER > 0
	  	if(HMI_IS_DYN_NFILL(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_NFILL_INDEX(hmi_object_id );
			if(hmi_object_index< HMI_DYN_FILL_PAGES_NUMBER)
			{
				hmi_rect_temp.x	=	hmi_fills_dyn_xy_rect[hmi_object_index].x;
				hmi_rect_temp.y	=	hmi_fills_dyn_xy_rect[hmi_object_index].y;
				hmi_rect_temp.w	=	hmi_fills_dyn_xy_rect[hmi_object_index].w;
				hmi_rect_temp.h	=	hmi_fills_dyn_xy_rect[hmi_object_index].h;
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
					(&hmi_rect_temp)
					,(&(hmi_screen_rect)));
				#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
				/*element draw rect backups*/
				hmi_fills_dyn_xy_rect_bck[hmi_object_index].x	= hmi_fills_dyn_xy_rect[hmi_object_index].x;
				hmi_fills_dyn_xy_rect_bck[hmi_object_index].y	= hmi_fills_dyn_xy_rect[hmi_object_index].y;
				hmi_fills_dyn_xy_rect_bck[hmi_object_index].w	= hmi_fills_dyn_xy_rect[hmi_object_index].w;
				hmi_fills_dyn_xy_rect_bck[hmi_object_index].h	= hmi_fills_dyn_xy_rect[hmi_object_index].h;
				#endif
			}
			#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||	\
				defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||	\
				defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||	\
				defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))
			if((fabs(pfather_alpha_scale->scale - HMI_SCALE_1)) > HMI_FLOAT_TOLERANCE)
			{
				hmi_engine_get_scale_rect(&hmi_screen_rect,pfather_alpha_scale,&hmi_scale_rect);
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
									&hmi_scale_rect,
									&(new_cliped_farther_rect ));
			}
			else
			{
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
									&hmi_screen_rect,
									&(new_cliped_farther_rect ));
			}			
			hmi_get_intersec_rect_depth((HMI_RECT_STR CONST *)(&(new_cliped_farther_rect)),
								depth,(HMI_RECT_STR  *)pdirty_rect,&hmi_clip_rect);			
			#endif
			
			#ifdef HMI_GRAPHIC_AGG
			if(hmi_object_index< HMI_DYN_FILL_PAGES_NUMBER)
			{
				call_C_hmi_driver_draw_fill_page(&hmi_screen_rect,
												&hmi_fills_dyn_prop_table[hmi_object_index],
												&hmi_clip_rect);
				hmi_engine_draw_container(&hmi_dyn_fill_container_table[hmi_object_index],
											(HMI_RECT_STR CONST *)&(hmi_screen_rect),
											pdirty_rect,depth,&new_cliped_farther_rect/*all father insection zone*/);
			}
			#endif

			#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
			if(hmi_object_index< HMI_DYN_FILL_PAGES_NUMBER)
			{
				if((hmi_clip_rect.w != 0)&&
					(hmi_clip_rect.h != 0))
				{
				call_C_hmi_driver_draw_fill_page(&hmi_fills_dyn_prop_table[hmi_object_index ],
												&hmi_clip_rect,
												pdirty_rect,
	             								depth,
	             								pfather_alpha_scale);
				#if HMI_DXY_FILL_MAX_SON_CNT > 0
				hmi_alpha=HMI_RGL_ALLPHA(hmi_fills_dyn_prop_table[hmi_object_index ].color);
				if(container_alpha_scale.alpha > hmi_alpha)
				{
					container_alpha_scale.alpha =hmi_alpha;
				}
				hmi_engine_draw_container(&(hmi_dyn_fill_container_table[hmi_object_index]),
											(HMI_RECT_STR CONST *)(&(hmi_screen_rect)),
											pdirty_rect,
											depth,&new_cliped_farther_rect/*all father insection zone*/,
											&container_alpha_scale);	
				#endif
				}
			}
			#endif

			#ifdef HMI_GRAPHIC_ST7513
			if(hmi_object_index< HMI_DYN_FILL_PAGES_NUMBER)
			{
				call_C_hmi_driver_draw_fill_page((HMI_RECT_STR CONST *)(&(hmi_screen_rect)),
												(HMI_FILL_PAGE_STR CONST *)(&hmi_fills_dyn_prop_table[hmi_object_index]),
												(HMI_RECT_STR *)(&hmi_clip_rect));
	             								
				hmi_engine_draw_container(&(hmi_dyn_fill_container_table[hmi_object_index]),
											(HMI_RECT_STR CONST *)(&(hmi_screen_rect )),
											pdirty_rect,
											depth,&new_cliped_farther_rect/*all father insection zone*/);											
			}
			#endif
			
			#ifdef HMI_GRAPHIC_TWLIB
			bMerge=hmi_get_is_merge(parent_object_id_type,HMI_ELEM_TYPE_DFILL);
			#if HMI_RENDER_ALL_EXCEPT_BCK==NO
			if(((!only_draw_flag)||!hmi_is_render_mode())||/*simulate*/
								(only_draw_flag&&(flag)))
			#endif
			{
				call_C_hmi_driver_draw_fill_page(&hmi_fills_dyn_prop_table[hmi_object_index ],
												(HMI_RECT_STR CONST *)(&(hmi_screen_rect)),
	             								bMerge
	             								#if HMI_RENDER_ALL_EXCEPT_BCK==NO
												,parent_object_id
												,hmi_object_id_const
												#endif
	             								); 				
			}
			if(hmi_dyn_fill_container_table[hmi_object_index].container_object_table.object_number>0)
			{
				hmi_engine_draw_container(&(hmi_dyn_fill_container_table[hmi_object_index]),
												(HMI_RECT_STR CONST *)(&(hmi_screen_rect )),
												HMI_ELEM_TYPE_DFILL,
												hmi_object_id_const,
												FALSE
												#if HMI_RENDER_ALL_EXCEPT_BCK==NO
													,phmi_object_prop_table->object_id 
													,only_draw_flag&&(!flag)
												#endif
												);
			}
			#endif
			
			#if (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
			if(hmi_object_index< HMI_DYN_FILL_PAGES_NUMBER)
			{
				if(flag ||draw_flag)
				{
					//call_C_hmi_driver_draw_fill_page(&hmi_fills_dyn_prop_table[hmi_object_index]);
				}
				//hmi_set_cur_win_used();
				#if HMI_DXY_FILL_MAX_SON_CNT > 0
				hmi_engine_draw_container(&(hmi_dyn_fill_container_table[hmi_object_index]),
											(HMI_RECT_STR CONST *)(&(hmi_screen_rect)),
											child_draw_flag
											);
				#endif
			}
			#endif
		}
	  	else
		#endif
		
		#if HMI_DYN_GFILL_NUMBER > 0
		if(HMI_IS_DYN_GFILL(hmi_object_id))
		{
	        hmi_object_index = HMI_GET_DYN_GFILL_INDEX(hmi_object_id );
			if(hmi_object_index < HMI_DYN_GFILL_NUMBER)
			{
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,\
					(&hmi_gradient_dxy_fill_rect[hmi_object_index]),(&hmi_screen_rect));
				#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
				/*element draw rect backups*/
				hmi_gradient_dxy_fill_rect_bck[hmi_object_index].x	= hmi_gradient_dxy_fill_rect[hmi_object_index].x;
				hmi_gradient_dxy_fill_rect_bck[hmi_object_index].y	= hmi_gradient_dxy_fill_rect[hmi_object_index].y;
				hmi_gradient_dxy_fill_rect_bck[hmi_object_index].w	= hmi_gradient_dxy_fill_rect[hmi_object_index].w;
				hmi_gradient_dxy_fill_rect_bck[hmi_object_index].h	= hmi_gradient_dxy_fill_rect[hmi_object_index].h;
				#endif
			}
			#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||\
				defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||\
				defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||\
				defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))
			if((fabs(pfather_alpha_scale->scale - HMI_SCALE_1)) > HMI_FLOAT_TOLERANCE)
			{
				hmi_engine_get_scale_rect(&hmi_screen_rect,pfather_alpha_scale,&hmi_scale_rect);
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
									&hmi_scale_rect,
									&(new_cliped_farther_rect ));
			}
			else
			{
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
									&hmi_screen_rect,
									&(new_cliped_farther_rect ));
			}
			hmi_get_intersec_rect_depth((HMI_RECT_STR CONST *)(&(new_cliped_farther_rect)),
								depth,(HMI_RECT_STR  *)pdirty_rect,&hmi_clip_rect);
			#endif	
				
			#ifdef HMI_GRAPHIC_AGG
			if(hmi_object_index < HMI_DYN_GFILL_NUMBER)
			{
		        call_C_hmi_driver_gradient_fill_page(
		             &(hmi_screen_rect),
		             &hmi_gradient_dxy_fill_table[hmi_object_index],
		             &hmi_clip_rect);
				hmi_engine_draw_container(&hmi_dyn_gfill_container_table[hmi_object_index],
					(HMI_RECT_STR CONST *)&(hmi_screen_rect ),
					pdirty_rect,depth,&new_cliped_farther_rect/*all father insection zone*/);
			}
			#endif

			#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
			if(hmi_object_index < HMI_DYN_GFILL_NUMBER)
			{
				if((hmi_clip_rect.w != 0)&&
					(hmi_clip_rect.h != 0))
				{
					call_C_hmi_driver_gradient_fill_page(
							             &(hmi_screen_rect),
					 	&hmi_gradient_dxy_fill_table[hmi_object_index],
							             &hmi_clip_rect,
							             pdirty_rect,
		             					pfather_alpha_scale);
				#if HMI_DXY_GFILL_MAX_SON_CNT > 0
				hmi_alpha=HMI_RGL_ALLPHA(hmi_gradient_dxy_fill_table[hmi_object_index ].color1);
				if(container_alpha_scale.alpha > hmi_alpha)
				{
					container_alpha_scale.alpha = hmi_alpha;
				}
				hmi_engine_draw_container(&hmi_dyn_gfill_container_table[hmi_object_index],
					(HMI_RECT_STR CONST *)&(hmi_screen_rect ),
					pdirty_rect,depth,&new_cliped_farther_rect/*all father insection zone*/,
					&container_alpha_scale);
				#endif
				}
			}
			#endif

			#ifdef HMI_GRAPHIC_ST7513
			if(hmi_object_index < HMI_DYN_GFILL_NUMBER)
			{
		        call_C_hmi_driver_gradient_fill_page(
		             &(hmi_screen_rect),
		             &hmi_gradient_dxy_fill_table[hmi_object_index],
		             &hmi_clip_rect);
				hmi_engine_draw_container(&hmi_dyn_gfill_container_table[hmi_object_index],
					(HMI_RECT_STR CONST *)(&(hmi_screen_rect )),
					pdirty_rect,depth,&new_cliped_farther_rect/*all father insection zone*/);
			}
			#endif
			
			#ifdef HMI_GRAPHIC_TWLIB
			/*call_C_hmi_driver_gradient_fill_page();*/ /*tw not support gridient fill*/
			#endif
			#if (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
			
			#endif
			
		}
		else
		#endif
		#if HMI_DXY_CUBE_NUMBER > 0
		if(HMI_IS_DYN_CUBE(hmi_object_id))
		{			 
			hmi_object_index = HMI_GET_DYN_CUBE_INDEX(hmi_object_id);			
			if(hmi_object_index < HMI_DXY_CUBE_NUMBER)
			{
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
					(&hmi_cubes_dyn_xy_rect[hmi_object_index].cube_rect),
					(&(hmi_screen_rect)));
				#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
				/*element draw rect backups*/
				hmi_cubes_dyn_xy_rect_bck[hmi_object_index].x	= hmi_cubes_dyn_xy_rect[hmi_object_index].cube_rect.x;
				hmi_cubes_dyn_xy_rect_bck[hmi_object_index].y	= hmi_cubes_dyn_xy_rect[hmi_object_index].cube_rect.y;
				hmi_cubes_dyn_xy_rect_bck[hmi_object_index].w	= hmi_cubes_dyn_xy_rect[hmi_object_index].cube_rect.w;
				hmi_cubes_dyn_xy_rect_bck[hmi_object_index].h	= hmi_cubes_dyn_xy_rect[hmi_object_index].cube_rect.h;
				#endif
				#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
				
				if((fabs(pfather_alpha_scale->scale - HMI_SCALE_1)) > HMI_FLOAT_TOLERANCE)
				{
					hmi_engine_get_scale_rect(&hmi_screen_rect,pfather_alpha_scale,&hmi_scale_rect);
					hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
										&hmi_scale_rect,
										&(new_cliped_farther_rect ));
				}
				else
				{
					hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
										&hmi_screen_rect,
										&(new_cliped_farther_rect ));
				}
				hmi_get_intersec_rect_depth((HMI_RECT_STR CONST *)(&(new_cliped_farther_rect)),
									depth,(HMI_RECT_STR  *)pdirty_rect,&hmi_clip_rect);
				hmi_cube_get_container_prop(hmi_cubes_dyn_xy_face[hmi_object_index].container_object_table.p_object_table[0].object_id,
									&hmi_cube_textrue.cube_face1_texture,
									&hmi_bump_textrue.cube_face1_texture,
									NULL);
				hmi_cube_get_container_prop(hmi_cubes_dyn_xy_face[hmi_object_index].container_object_table.p_object_table[1].object_id,
									&hmi_cube_textrue.cube_face2_texture,
									&hmi_bump_textrue.cube_face2_texture,
									NULL);		
				hmi_cube_get_container_prop(hmi_cubes_dyn_xy_face[hmi_object_index].container_object_table.p_object_table[2].object_id,
									&hmi_cube_textrue.cube_face3_texture,
									&hmi_bump_textrue.cube_face3_texture,
									NULL);		
				hmi_cube_get_container_prop(hmi_cubes_dyn_xy_face[hmi_object_index].container_object_table.p_object_table[3].object_id,
									&hmi_cube_textrue.cube_face4_texture,
									&hmi_bump_textrue.cube_face4_texture,
									NULL);		
				hmi_cube_get_container_prop(hmi_cubes_dyn_xy_face[hmi_object_index].container_object_table.p_object_table[4].object_id,
									&hmi_cube_textrue.cube_face5_texture,
									&hmi_bump_textrue.cube_face5_texture,
									NULL);		
				hmi_cube_get_container_prop(hmi_cubes_dyn_xy_face[hmi_object_index].container_object_table.p_object_table[5].object_id,		
									&hmi_cube_textrue.cube_face6_texture,
									&hmi_bump_textrue.cube_face6_texture,
									NULL);
				if((hmi_clip_rect.w != 0)&&
					(hmi_clip_rect.h != 0))
				{
					//hmi_get_axis(axis,&hmi_cubes_dyn_xy_axis[hmi_object_index],
								//pfarther_rect);
					hmi_engine_get_cube_attr(cube_rotation_axis,
												&cube_camera,
												hmi_object_id,
												pfarther_rect);
					
					/*screen coordinate*/
					cube_camera.target.x	+= pfarther_rect->x;
					cube_camera.target.y	+= pfarther_rect->y;
	
					cube_camera.position.x	+= pfarther_rect->x;
					cube_camera.position.y	+= pfarther_rect->y;
					call_C_hmi_driver_draw_cube(&(hmi_screen_rect)/*node screen position*/, 
							(&hmi_cubes_dyn_xy_rect[hmi_object_index]),
							&hmi_cube_textrue,
							&hmi_bump_textrue,
							pcliped_farther_rect,
							pdirty_rect,
							depth,
							/*hmi_object_id,*/
							pfather_alpha_scale,
							&hmi_cubes_dyn_xy_axis[hmi_object_index], 
							//axis,
							cube_rotation_axis,
							&cube_camera
							);
				}
				#endif
			}
		}
		else
		#endif 
		#if HMI_DXY_3DCUBE_NUMBER > 0
		if(HMI_IS_DYN_3DCUBE(hmi_object_id))
		{			 
			hmi_object_index = HMI_GET_DYN_3DCUBE_INDEX(hmi_object_id);			
			if(hmi_object_index < HMI_DXY_3DCUBE_NUMBER)
			{
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
					(&hmi_3dcubes_dyn_xy_rect[hmi_object_index].cube_rect),
					(&(hmi_screen_rect)));
		#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
				/*element draw rect backups*/
				hmi_3Dcubes_dyn_xy_rect_bck[hmi_object_index].x	= hmi_3dcubes_dyn_xy_rect[hmi_object_index].cube_rect.x;
				hmi_3Dcubes_dyn_xy_rect_bck[hmi_object_index].y	= hmi_3dcubes_dyn_xy_rect[hmi_object_index].cube_rect.y;
				hmi_3Dcubes_dyn_xy_rect_bck[hmi_object_index].w	= hmi_3dcubes_dyn_xy_rect[hmi_object_index].cube_rect.w;
				hmi_3Dcubes_dyn_xy_rect_bck[hmi_object_index].h	= hmi_3dcubes_dyn_xy_rect[hmi_object_index].cube_rect.h;
		#endif
			#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
			if((fabs(pfather_alpha_scale->scale - HMI_SCALE_1)) > HMI_FLOAT_TOLERANCE)
			{
				hmi_engine_get_scale_rect(&hmi_screen_rect,pfather_alpha_scale,&hmi_scale_rect);
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
									&hmi_scale_rect,
									&(new_cliped_farther_rect ));
			}
			else
			{
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
									&hmi_screen_rect,
									&(new_cliped_farther_rect ));
			}
			hmi_get_intersec_rect_depth((HMI_RECT_STR CONST *)(&(new_cliped_farther_rect)),
								depth,(HMI_RECT_STR  *)pdirty_rect,&hmi_clip_rect);
			#endif	
			#if (defined(HMI_GRAPHIC_OPENGLES))
			if((hmi_clip_rect.x != HMI_INVALID_COOR)||
				(hmi_clip_rect.y != HMI_INVALID_COOR))
			{
					
				hmi_engine_draw_3dcube(
									&(hmi_screen_rect),
									pcliped_farther_rect,
									pdirty_rect,
									hmi_object_id,
									hmi_3dcubes_dyn_xy_rect[hmi_object_index].node_index);
			}
				
			#endif
			}
		}
		else
#endif 
		#if HMI_DXY_CONTAINERS_NUMBER > 0U  
		if(HMI_IS_DYN_XY_CONTAINER(hmi_object_id))
		{
			hmi_object_id = HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_object_id );
			if(hmi_object_id < HMI_DXY_CONTAINERS_NUMBER)
			{
				hmi_get_object_screen_coor((HMI_RECT_STR CONST *)pfarther_rect,
					&hmi_dyn_xy_container_rect[hmi_object_id],
					&(hmi_screen_rect));
				#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
				/*element draw rect backups*/
				hmi_dyn_xy_container_rect_bck[hmi_object_id].x	= hmi_dyn_xy_container_rect[hmi_object_id].x;
				hmi_dyn_xy_container_rect_bck[hmi_object_id].y	= hmi_dyn_xy_container_rect[hmi_object_id].y;
				hmi_dyn_xy_container_rect_bck[hmi_object_id].w	= hmi_dyn_xy_container_rect[hmi_object_id].w;
				hmi_dyn_xy_container_rect_bck[hmi_object_id].h	= hmi_dyn_xy_container_rect[hmi_object_id].h;
				#endif
			}
			#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||\
				defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||\
				defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||\
				defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))
				
			if((fabs(pfather_alpha_scale->scale - HMI_SCALE_1)) > HMI_FLOAT_TOLERANCE)
			{
				hmi_engine_get_scale_rect(&hmi_screen_rect,pfather_alpha_scale,&hmi_scale_rect);
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
									&hmi_scale_rect,
									&(new_cliped_farther_rect ));
			}
			else
			{
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
									&hmi_screen_rect,
									&(new_cliped_farther_rect ));
			}
			hmi_get_intersec_rect_depth((HMI_RECT_STR CONST *)(&(new_cliped_farther_rect)),
							depth,(HMI_RECT_STR  *)pdirty_rect,&hmi_clip_rect);
			#endif

			#ifdef HMI_GRAPHIC_AGG
			if(hmi_object_id < HMI_DXY_CONTAINERS_NUMBER)
			{
				hmi_engine_draw_container(&hmi_dyn_xy_container_table[hmi_object_id],
						(HMI_RECT_STR CONST *)&(hmi_screen_rect ),
						pdirty_rect,depth,&new_cliped_farther_rect/*all father insection zone*/);
			}
			#endif

			#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
			if(hmi_object_id < HMI_DXY_CONTAINERS_NUMBER)
			{	
				if((hmi_clip_rect.x!=HMI_INVALID_COOR)||
				(hmi_clip_rect.y!=HMI_INVALID_COOR))				
				{
					if(hmi_dxy_container_video_fmt[hmi_object_id].video_fmt_channel&HMI_VIDEO_WINDOW)
					{
						hmi_display_video(&hmi_dxy_container_video_fmt[hmi_object_id],
										&hmi_dyn_xy_container_rect[hmi_object_id],
										&hmi_dxy_container_video_status[hmi_object_id],
										depth);
					}/*2016.9.7. for support video capture in*/
					/*dxy container scale */
					#if HMI_DXY_CONTAINERS_SCALE_ALPHA_NUMBER >0U
					if(hmi_object_id < HMI_DXY_CONTAINERS_SCALE_ALPHA_NUMBER)
					{
						hmi_dxy_container_scale	= hmi_dyn_xy_container_alpha_scale[hmi_object_id].scale;
						if((fabs(hmi_dxy_container_scale - HMI_SCALE_1)) > HMI_FLOAT_TOLERANCE)/*scale*/
						{
							if((fabs(pfather_alpha_scale->scale - HMI_SCALE_1)) < HMI_FLOAT_TOLERANCE)/*father no scale*/
							{
								container_alpha_scale.point.x = hmi_screen_rect.x+(hmi_screen_rect.w >> 1);
								container_alpha_scale.point.y = hmi_screen_rect.y+(hmi_screen_rect.h >> 1);
							}
							container_alpha_scale.scale = hmi_dxy_container_scale * pfather_alpha_scale->scale;
						}
						if(pfather_alpha_scale->alpha >hmi_dyn_xy_container_alpha_scale[hmi_object_id].alpha)
						{
							container_alpha_scale.alpha =hmi_dyn_xy_container_alpha_scale[hmi_object_id].alpha;
						}
					}
					#endif

					
			#if (defined(HMI_GRAPHIC_OPENGLES)) // 2023 02 15 lq
					if(!(hmi_dxy_container_video_fmt[hmi_object_id].
						video_fmt_channel & HMI_SCENE_WINDOW)) // support 3d scene 2023 02 15 lq
					{
						hmi_engine_draw_container(&hmi_dyn_xy_container_table[hmi_object_id],
											(HMI_RECT_STR CONST *)&(hmi_screen_rect ),
											pdirty_rect,depth,
											&new_cliped_farther_rect/*all father insection zone*/,
											&container_alpha_scale);
					}
					else	// support 3d scene 2023 02 15 lq
					{
					#if HMI_DXY_CUSTOM_CNT + HMI_SXY_CUSTOM_CNT >0U
						hmi_draw_opengl_scene(&new_cliped_farther_rect,
											hmi_object_id_const);
					#endif
						hmi_engine_draw_container(&hmi_dyn_xy_container_table[hmi_object_id],
											(HMI_RECT_STR CONST *)&(hmi_screen_rect ),
											pdirty_rect,depth,
											&new_cliped_farther_rect/*all father insection zone*/,
											&container_alpha_scale);
					}
			#else
					hmi_engine_draw_container(&hmi_dyn_xy_container_table[hmi_object_id],
											(HMI_RECT_STR CONST *)&(hmi_screen_rect ),
											pdirty_rect,depth,
											&new_cliped_farther_rect/*all father insection zone*/,
											&container_alpha_scale);
			#endif
										
				}
			}
			#endif

			#ifdef HMI_GRAPHIC_ST7513
			if(hmi_object_id < HMI_DXY_CONTAINERS_NUMBER)
			{
				hmi_engine_draw_container(&hmi_dyn_xy_container_table[hmi_object_id],
						(HMI_RECT_STR CONST *)&(hmi_screen_rect ),
						pdirty_rect,depth,&new_cliped_farther_rect/*all father insection zone*/);
			}
			#endif

			#ifdef HMI_GRAPHIC_TWLIB
			bMerge=hmi_get_is_merge(parent_object_id_type,HMI_ELEM_TYPE_DXY_CONTINER);
			#if HMI_RENDER_ALL_EXCEPT_BCK==NO
			//if(((!only_draw_flag)||!hmi_is_render_mode())||/*simulate*/
								//(only_draw_flag&&(flag)))
								
			#endif
			{
				if(hmi_dyn_xy_container_table[hmi_object_id].container_object_table.object_number>0)
				{
					hmi_engine_draw_container(&(hmi_dyn_xy_container_table[hmi_object_id]),
												(HMI_RECT_STR CONST *)(&(hmi_screen_rect )),
												HMI_ELEM_TYPE_DXY_CONTINER,
												hmi_object_id_const,
												!bMerge
												#if HMI_RENDER_ALL_EXCEPT_BCK==NO
													,phmi_object_prop_table->object_id 
													,only_draw_flag&&(!flag)
												#endif
												);
				}
			}
			#endif
			
			#if (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
			if(hmi_object_id < HMI_DXY_CONTAINERS_NUMBER)
			{	
				hmi_engine_draw_container(&hmi_dyn_xy_container_table[hmi_object_id],
											(HMI_RECT_STR CONST *)&(hmi_screen_rect ),
											child_draw_flag);

			}
			#endif
		}
		else
		#endif
		
		#if HMI_DXY_BITMAPS_NUMBER > 0 
		if(HMI_IS_DYN_XY_BITMAP(hmi_object_id))
		{	  
			hmi_object_index = HMI_GET_DYN_XY_BITMAP_INDEX(hmi_object_id );
			
			hmi_rect_temp.x	=	hmi_bmp_dyn_xy_rect[hmi_object_index].x;
			hmi_rect_temp.y	=	hmi_bmp_dyn_xy_rect[hmi_object_index].y;
			hmi_rect_temp.w	=	hmi_bmp_dyn_xy_rect[hmi_object_index].w;
			hmi_rect_temp.h	=	hmi_bmp_dyn_xy_rect[hmi_object_index].h;
			HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,(&hmi_rect_temp),
											(&(hmi_screen_rect)));
			#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||\
				defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG))
			/*element draw rect backups*/ 
			hmi_bmp_dyn_xy_rect_bck[hmi_object_index].x	= hmi_rect_temp.x;
			hmi_bmp_dyn_xy_rect_bck[hmi_object_index].y	= hmi_rect_temp.y;
			hmi_bmp_dyn_xy_rect_bck[hmi_object_index].w	= hmi_rect_temp.w;
			hmi_bmp_dyn_xy_rect_bck[hmi_object_index].h	= hmi_rect_temp.h;
			hmi_bmp_dyn_angel_bck[hmi_object_index]		= hmi_bmp_dyn_xy_rect[hmi_object_index].angel; 
			#endif
			#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||\
				defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||\
				defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||\
				defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))
				
			if((fabs(pfather_alpha_scale->scale - HMI_SCALE_1)) > HMI_FLOAT_TOLERANCE)
			{
				hmi_engine_get_scale_rect(&hmi_screen_rect,pfather_alpha_scale,&hmi_scale_rect);
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
									&hmi_scale_rect,
									&(new_cliped_farther_rect ));
			}
			else
			{
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
									&hmi_screen_rect,
									&(new_cliped_farther_rect ));
			}
			if(hmi_bmp_dyn_xy_rect[hmi_object_index].angel != 0)
			{
				hmi_get_intersec_rect_depth((HMI_RECT_STR CONST *)(pcliped_farther_rect),
														depth,(HMI_RECT_STR  *)pdirty_rect,
														&hmi_clip_rect);
			}
			else
			{
				hmi_get_intersec_rect_depth((HMI_RECT_STR CONST *)(&(new_cliped_farther_rect)),
														depth,(HMI_RECT_STR  *)pdirty_rect,
														&hmi_clip_rect);
			}			
			#endif
			
			#ifdef HMI_GRAPHIC_AGG
			call_C_hmi_driver_draw_image(&hmi_screen_target, hmi_bigbitmap_table[0],hmi_bigbitmap_table[1], 
						&hmi_bmp_dyn_xy_prop_table[hmi_object_index],
						&hmi_clip_rect);
			hmi_engine_draw_container(&hmi_dxy_bitmap_container_table[hmi_object_index],
										(HMI_RECT_STR CONST *)(&hmi_screen_rect),
										pdirty_rect,
										depth,
										&new_cliped_farther_rect/*all father insection zone*/
										);    
			#endif

			#if (defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL)||defined(S6J3200_GRAPHIC))
			if((hmi_clip_rect.x != HMI_INVALID_COOR)||
				(hmi_clip_rect.y != HMI_INVALID_COOR))
			{	
				#if HMI_DXY_CENTER_BITMAPS_NUMBER > 0
				//if(hmi_object_index < HMI_DXY_CENTER_BITMAPS_NUMBER)/*rotation center*/
				#if HMI_DXY_CENTER_SINGLE_TEXTURE_BITMAPS_NUMBER >0
				if(HMI_IS_DXY_CENTER_SINGEL_TEXTURE_BITAMP(hmi_object_id))
				{	
					call_C_hmi_driver_draw_image(&(hmi_screen_rect)/*node screen position*/, 
							hmi_dxy_bitmap_attr_table[hmi_object_index]/*image format*/,
							&hmi_bmp_dyn_xy_rect[hmi_object_index]/*alpha,rotation*/,
							&hmi_bmp_dyn_xy_prop_table[hmi_object_index],
							&hmi_clip_rect,
							pdirty_rect,
							depth,
							NULL,/*rotation point is center*/
							hmi_object_id,
							pfather_alpha_scale);
				}
				else 
				#endif
				#if HMI_DXY_CENTER_MUL_TEXTURE_BITMAPS_NUMBER	>0
				if(HMI_IS_DXY_CENTER_MUL_TEXTURE_BITAMP(hmi_object_id))
				{
					hmi_object_index2 =HMI_GET_DXY_CENTER_MUL_TEXTURE_BITMAP_INDEX(hmi_object_id);
					call_C_hmi_driver_draw_mul_texture_image(&(hmi_screen_rect)/*node screen position*/, 
															hmi_dxy_bitmap_attr_table[hmi_object_index]/*image format*/,
															&hmi_bmp_dyn_xy_rect[hmi_object_index]/*alpha,rotation*/,
															&hmi_bmp_dyn_xy_prop_table[hmi_object_index],
															&hmi_clip_rect,
															pdirty_rect,
															depth,
															NULL,/*rotation point is center*/
															hmi_object_id,
															pfather_alpha_scale,
															&hmi_dxy_center_texture_rect_list[hmi_object_index2],
															hmi_texture_rect_list);
				}
				else 
				#endif
				/*rotation userdefine point*/
				#endif
					{
					#if HMI_DXY_ROTATION_BITMAPS_NUMBER > 0 
				if(hmi_object_index > HMI_DXY_CENTER_BITMAPS_NUMBER)
				{
					hmi_object_index2	= hmi_object_index-HMI_DXY_CENTER_BITMAPS_NUMBER;
				}
				else
				{
					hmi_object_index2	= 0U;
				}
					if(hmi_object_index2 < HMI_DXY_ROTATION_BITMAPS_NUMBER)
					{
						rotation.x	= pfarther_rect->x + hmi_dxy_bitmap_rotation[hmi_object_index2].x;	
						rotation.y	= pfarther_rect->y + hmi_dxy_bitmap_rotation[hmi_object_index2].y;
				
					#if HMI_DXY_ROTATION_SINGLE_TEXTURE_BITMAPS_NUMBER >0
					if(HMI_IS_DXY_ROTATION_SINGEL_TEXTURE_BITAMP(hmi_object_id))
					{
						hmi_trail_id=hmi_dxy_bitmap_rotation_trail[hmi_object_index2].texture;
						#if 1
						if((HMI_IS_DYN_XY_BITMAP(hmi_trail_id))||(HMI_IS_S_XY_BITMAP(hmi_trail_id)))
						{
							hmi_cube_get_container_prop(hmi_trail_id,
												&image_trail_texture,
												&image_trail_bump_texture,
												&trail_point);
							trail_point.x = 0;
							trail_point.y = 0;
						}
						else
							#endif
						{
							hmi_cube_get_container_prop(hmi_trail_id,
												&image_trail_texture,
												&image_trail_bump_texture,
												&trail_point);
						}
						
						hmi_get_object_wh(hmi_trail_id,
													&trail_rect.w,
													&trail_rect.h);
						hmi_trail_get_id_flag=hmi_get_container_child_id(&hmi_trail_id);
						if(hmi_trail_get_id_flag)
						{
							#if(defined(HMI_GRAPHIC_OPENVG))
							{
								hmi_get_object_wh(hmi_trail_id,
													&trail_bmp_rect.w,
													&trail_bmp_rect.h);
								trail_rect.y	= trail_point.y+trail_bmp_rect.h;
							}
							#else
							{
								trail_rect.y	= trail_point.y;
							}
							#endif
							trail_rect.x	= trail_point.x;
							
							trail_info.texture	= hmi_trail_id;
							trail_info.trail_en=hmi_dxy_bitmap_rotation_trail[hmi_object_index2].trail_en;
							//trail_info.cw=hmi_dxy_bitmap_rotation_trail[hmi_object_index2].cw;
							trail_info.begin_angel=hmi_dxy_bitmap_rotation_trail[hmi_object_index2].begin_angel;
							trail_info.end_angel=hmi_dxy_bitmap_rotation_trail[hmi_object_index2].end_angel;
							hmi_driver_animated_trail(&trail_info,
													hmi_dxy_bitmap_rotation_trail_attr[hmi_object_index2],
													&(hmi_screen_rect)/*node screen position*/,
													image_trail_texture.tex_attr,
													&image_trail_texture.tex_rect,
													&image_trail_texture.tex_prop,
													&hmi_clip_rect,
													pdirty_rect,
													depth,
													(HMI_ROTATION_STR *)(&rotation),/*rotation point is userdefine point*/
													hmi_bmp_dyn_xy_rect[hmi_object_index].angel,
													pfather_alpha_scale,
													&hmi_bmp_dyn_xy_prop_table[hmi_object_index],
													&trail_rect);/*support animated trail*/	
						
						}
						call_C_hmi_driver_draw_image(&(hmi_screen_rect)/*node screen position*/, 
								hmi_dxy_bitmap_attr_table[hmi_object_index]/*image format*/,
								&hmi_bmp_dyn_xy_rect[hmi_object_index]/*alpha,rotation*/,
								&hmi_bmp_dyn_xy_prop_table[hmi_object_index],
								&hmi_clip_rect,
								pdirty_rect,
								depth,
								(HMI_ROTATION_STR *)(&rotation),/*rotation point is userdefine point*/
								hmi_object_id,
								pfather_alpha_scale);
				}
					}
					else
					#endif
					#if HMI_DXY_ROTATION_MUL_TEXTURE_BITMAPS_NUMBER	>0
					if(HMI_IS_DXY_ROTATION_MUL_TEXTURE_BITAMP(hmi_object_id))
					{
						hmi_object_index2 =HMI_GET_DXY_ROTATION_MUL_TEXTURE_BITMAP_INDEX(hmi_object_id);
						call_C_hmi_driver_draw_mul_texture_image(&(hmi_screen_rect)/*node screen position*/, 
																hmi_dxy_bitmap_attr_table[hmi_object_index]/*image format*/,
																&hmi_bmp_dyn_xy_rect[hmi_object_index]/*alpha,rotation*/,
																&hmi_bmp_dyn_xy_prop_table[hmi_object_index],
																&hmi_clip_rect,
																pdirty_rect,
																depth,
																NULL,/*rotation point is center*/
																hmi_object_id,
																pfather_alpha_scale,
																&hmi_dxy_center_texture_rect_list[hmi_object_index2],
																hmi_texture_rect_list);
					}
					else 
					#endif
					{}
					#endif
				}
				
				#if HMI_DXY_IMAGE_MAX_SON_CNT > 0
				if(container_alpha_scale.alpha> hmi_bmp_dyn_xy_rect[hmi_object_index].alpha)
				{
					container_alpha_scale.alpha= hmi_bmp_dyn_xy_rect[hmi_object_index].alpha;
				}
				hmi_engine_draw_container(&hmi_dxy_bitmap_container_table[hmi_object_index],
									(HMI_RECT_STR CONST *)(&(hmi_screen_rect )),
									pdirty_rect,
									depth,&new_cliped_farther_rect/*all father insection zone*/,&container_alpha_scale);
				#endif
			}
			#endif

			#ifdef HMI_GRAPHIC_ST7513
			call_C_hmi_driver_draw_image(&(hmi_screen_rect)/*node screen position*/, 	       			
					&hmi_bmp_dyn_xy_prop_table[hmi_object_index],
					&hmi_clip_rect);	       			
			hmi_engine_draw_container(&hmi_dxy_bitmap_container_table[hmi_object_index],
								(HMI_RECT_STR CONST *)(&(hmi_screen_rect )),
								pdirty_rect,
								depth,&new_cliped_farther_rect/*all father insection zone*/);				
			#endif

			#ifdef HMI_GRAPHIC_TWLIB
			bMerge=hmi_get_is_merge(parent_object_id_type,HMI_ELEM_TYPE_DIMAGE);
			#if HMI_RENDER_ALL_EXCEPT_BCK==NO
			if(((!only_draw_flag)||!hmi_is_render_mode())||/*simulate*/
								(only_draw_flag&&(flag)))
			#endif
			{
				call_C_hmi_driver_draw_image(&(hmi_screen_rect ), HMI_IMAGE_COMPRESS_NONE,
										&hmi_bmp_dyn_xy_rect[hmi_object_index],
										&hmi_bmp_dyn_xy_prop_table[hmi_object_index]/*,
										TRUE*/,
										bMerge
										#if HMI_RENDER_ALL_EXCEPT_BCK==NO
										,parent_object_id
										,hmi_object_id_const
										#endif
										);									
			}
			if(hmi_dxy_bitmap_container_table[hmi_object_index].container_object_table.object_number>0)
			{
				hmi_engine_draw_container(&(hmi_dxy_bitmap_container_table[hmi_object_index]),
										(HMI_RECT_STR CONST *)(&(hmi_screen_rect )),
										HMI_ELEM_TYPE_DIMAGE ,
										hmi_object_id_const,
										FALSE
										#if HMI_RENDER_ALL_EXCEPT_BCK==NO
											,phmi_object_prop_table->object_id 
											,only_draw_flag&&(!flag)
										#endif
										); 
			}
			#endif 
			
			#if (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
			if(flag || draw_flag)
			{	
				call_C_hmi_driver_draw_image(&(hmi_screen_rect)/*node screen position*/, 
							&hmi_dxy_bitmap_attr_table[hmi_object_index]/*image format*/,
							&hmi_bmp_dyn_xy_rect[hmi_object_index]/*alpha,rotation*/,
							&hmi_bmp_dyn_xy_prop_table[hmi_object_index]
							);
			}
			hmi_set_cur_win_used();
			#if HMI_DXY_IMAGE_MAX_SON_CNT > 0
			hmi_engine_draw_container(&hmi_dxy_bitmap_container_table[hmi_object_index],
								(HMI_RECT_STR CONST *)(&(hmi_screen_rect ),
								child_draw_flag)
								);
			#endif
			
			#endif
		}
		else
		#endif
		
		#if HMI_DXY_SPLINE_NUMBER > 0 
		if(HMI_IS_DXY_SPLINE(hmi_object_id))
		{	 
			hmi_object_index = HMI_GET_DXY_SPLINE_INDEX(hmi_object_id );

			hmi_rect_temp.x =   hmi_dxy_spline_rect[hmi_object_index].x;
			hmi_rect_temp.y =   hmi_dxy_spline_rect[hmi_object_index].y;
			hmi_rect_temp.w =   hmi_dxy_spline_rect[hmi_object_index].w;
			hmi_rect_temp.h =   hmi_dxy_spline_rect[hmi_object_index].h;
			HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,(&hmi_rect_temp),
									   (&(hmi_screen_rect)));
#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
			/*element draw rect backups*/ 
			hmi_spline_dyn_xy_rect_bck[hmi_object_index].x = hmi_rect_temp.x;
			hmi_spline_dyn_xy_rect_bck[hmi_object_index].y = hmi_rect_temp.y;
			hmi_spline_dyn_xy_rect_bck[hmi_object_index].w = hmi_rect_temp.w;
			hmi_spline_dyn_xy_rect_bck[hmi_object_index].h = hmi_rect_temp.h;
#endif
#if (defined(HMI_GRAPHIC_AGG)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513)||defined(HMI_GRAPHIC_OPENGLES))
			if((fabs(pfather_alpha_scale->scale - HMI_SCALE_1)) > HMI_FLOAT_TOLERANCE)
			{
				hmi_engine_get_scale_rect(&hmi_screen_rect,pfather_alpha_scale,&hmi_scale_rect);
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
									&hmi_scale_rect,
									&(new_cliped_farther_rect ));
			}
			else
			{
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
									&hmi_screen_rect,
									&(new_cliped_farther_rect ));
			}
			hmi_get_intersec_rect_depth((HMI_RECT_STR CONST *)(&(new_cliped_farther_rect)),
								depth,(HMI_RECT_STR  *)pdirty_rect,&hmi_clip_rect);
#endif

#if (defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_OPENGLES))
			if((hmi_clip_rect.x != HMI_INVALID_COOR)||
					(hmi_clip_rect.y != HMI_INVALID_COOR))
			{
				hmi_spline_prop.line_w		= 0U;
				hmi_spline_prop.ppolygon	= NULL;
				hmi_spline_prop.scale		= 1.0f;
				hmi_spline_prop.spline_color.fill_color	= 0U;
				hmi_spline_prop.spline_color.fill_flag	= HMI_FILL_NONE;
				hmi_spline_prop.spline_color.spline_texture.ptex_attr	= NULL;
				hmi_spline_prop.spline_color.spline_texture.ptex_prop	= NULL;
				hmi_spline_prop.spline_color.spline_texture.ptex_rect	= NULL;
				hmi_spline_prop.spline_color.spline_texture.tex_id		= HMI_NB_ELEMENTS;
				
				hmi_spline_prop.spline_zone_color.fill_color	= 0U;
				hmi_spline_prop.spline_zone_color.fill_flag		= HMI_FILL_NONE;
				hmi_spline_prop.spline_zone_color.spline_texture.ptex_attr	= NULL;
				hmi_spline_prop.spline_zone_color.spline_texture.ptex_prop	= NULL;
				hmi_spline_prop.spline_zone_color.spline_texture.ptex_rect	= NULL;
				hmi_spline_prop.spline_zone_color.spline_texture.tex_id		= HMI_NB_ELEMENTS;
				hmi_spline_prop.point_anticlockwise				= FALSE;
				hmi_spline_prop.spline_attr 					= 0U;
				hmi_clear_spline_point_polyline(&hmi_spline_input_point);

				hmi_spline_prop_id	= hmi_dxy_spline_color[hmi_object_index];
				if(hmi_spline_prop_id !=HMI_NB_ELEMENTS)
				{
					hmi_get_color_success = hmi_spline_get_id_prop(hmi_spline_prop_id,&spline_color);
				}

				hmi_spline_prop_id	= hmi_dxy_spline_zone_color[hmi_object_index];
				if(hmi_spline_prop_id !=HMI_NB_ELEMENTS)
				{
					hmi_get_zone_color_success =hmi_spline_get_id_prop(hmi_spline_prop_id,&spline_zone_color);
				}

				hmi_spline_prop_id	= hmi_dxy_spline_ctrl_point[hmi_object_index];
				if(hmi_spline_prop_id !=HMI_NB_ELEMENTS)
				{
				 	hmi_get_object_wh(hmi_spline_prop_id,&spline_width,&height);
				}
				#if (HMI_DXY_SPLINE_NUMBER - HMI_DXY_ONE_POINT_SPLINE_NUMBER) > 0U
				if(HMI_IS_DXY_MUL_SPLINE(hmi_object_id))
				{ 
					if(hmi_spline_prop_id != HMI_NB_ELEMENTS)
					{
						hmi_get_ctrl_point_success	= hmi_write_mul_spline_point((HMI_RECT_STR CONST 	*)(&(hmi_screen_rect)),
																				hmi_spline_prop_id,
																			 	hmi_dxy_spline_attr_dir[hmi_object_index]);
						hmi_spline_prop.line_w		= spline_width;
						hmi_spline_prop.scale		= hmi_dxy_spline_scale[hmi_object_index];
						hmi_spline_prop.ppolygon	= (&hmi_spline_input_point);
						hmi_spline_prop.point_anticlockwise	= FALSE;
						hmi_spline_prop.spline_attr			= hmi_dxy_spline_attr_dir[hmi_object_index];
						if(hmi_get_color_success == TRUE)
						{
							hmi_spline_prop.spline_color.fill_color		= spline_color.fill_color;
							hmi_spline_prop.spline_color.fill_flag		= spline_color.fill_flag;
							hmi_spline_prop.spline_color.spline_texture = spline_color.spline_texture;
						}
						if(hmi_get_zone_color_success == TRUE)
						{
							hmi_spline_prop.spline_zone_color.fill_color		= spline_zone_color.fill_color;
							hmi_spline_prop.spline_zone_color.fill_flag			= spline_zone_color.fill_flag;
							hmi_spline_prop.spline_zone_color.spline_texture 	= spline_zone_color.spline_texture;
						}
						if(hmi_get_ctrl_point_success == TRUE)
						{
							   call_C_hmi_driver_draw_spline(&(hmi_screen_rect)/*node screen position*/, 
															   &hmi_spline_prop/*image format*/,
															   &hmi_clip_rect,
															   pdirty_rect,
															   depth,
															   hmi_object_id,
															   pfather_alpha_scale);
						}
					}
				}
				else 
				#endif
				#if HMI_DXY_ONE_POINT_SPLINE_NUMBER >0
				if(HMI_IS_DXY_ONE_SPLINE(hmi_object_id))
				{
					hmi_object_index2 =HMI_GET_DXY_ONE_SPLINE_INDEX(hmi_object_id);
					if(hmi_spline_prop_id !=HMI_NB_ELEMENTS)
					{
						hmi_get_ctrl_point_success	=hmi_write_one_spline_point((HMI_RECT_STR CONST 	*)(&(hmi_screen_rect)),
																				dxy_one_point_array,
																				(POINT_RANGE_TP	 CONST *)(&(dxy_one_point_range_array[hmi_object_index2])),
																				&(dxy_spline_head_tail_offset_array[hmi_object_index2]),
																				hmi_dxy_spline_attr_dir[hmi_object_index]);
						
						hmi_spline_prop.line_w				= spline_width;
						hmi_spline_prop.scale				= hmi_dxy_spline_scale[hmi_object_index];
						hmi_spline_prop.ppolygon			= &hmi_spline_input_point;
						hmi_spline_prop.point_anticlockwise	= FALSE;//TRUE;
						hmi_spline_prop.spline_attr			= hmi_dxy_spline_attr_dir[hmi_object_index];
						
						if(hmi_get_color_success == TRUE)
						{
							hmi_spline_prop.spline_color.fill_color		= spline_color.fill_color;
							hmi_spline_prop.spline_color.fill_flag		= spline_color.fill_flag;
							hmi_spline_prop.spline_color.spline_texture = spline_color.spline_texture;
						}
						if(hmi_get_zone_color_success == TRUE)
						{
							hmi_spline_prop.spline_zone_color.fill_color		= spline_zone_color.fill_color;
							hmi_spline_prop.spline_zone_color.fill_flag			= spline_zone_color.fill_flag;
							hmi_spline_prop.spline_zone_color.spline_texture	= spline_zone_color.spline_texture;
						}

						if(hmi_get_ctrl_point_success	== TRUE)
						{
						call_C_hmi_driver_draw_spline(&(hmi_screen_rect)/*node screen position*/, 
													   &hmi_spline_prop/*image format*/,
													   &hmi_clip_rect,
													   pdirty_rect,
													   depth,
													   hmi_object_id,
													   pfather_alpha_scale);
						}
					}
				}
				else 
				#endif
				{
				}
				
				#if HMI_DXY_SPLINE_MAX_SON_CNT > 0
				hmi_engine_draw_container(&hmi_dyn_xy_spline_table[hmi_object_index],
								   (HMI_RECT_STR CONST *)(&(hmi_screen_rect )),
								   pdirty_rect,
								   depth,&new_cliped_farther_rect/*all father insection zone*/,&container_alpha_scale);
				#endif
			}
			#endif
		}
		else
		#endif
		
		#if HMI_SXY_SPLINE_NUMBER > 0 
		if(HMI_IS_SXY_SPLINE(hmi_object_id))
		{	 
			hmi_object_index = HMI_GET_SXY_SPLINE_INDEX(hmi_object_id );

			hmi_rect_temp.x =	hmi_sxy_spline_rect[hmi_object_index].x;
			hmi_rect_temp.y =	hmi_sxy_spline_rect[hmi_object_index].y;
			hmi_rect_temp.w =	hmi_sxy_spline_rect[hmi_object_index].w;
			hmi_rect_temp.h =	hmi_sxy_spline_rect[hmi_object_index].h;
			HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,(&hmi_rect_temp),
									   (&(hmi_screen_rect)));
			#if (defined(HMI_GRAPHIC_AGG)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513)||defined(HMI_GRAPHIC_OPENGLES))
			if((fabs(pfather_alpha_scale->scale - HMI_SCALE_1)) > HMI_FLOAT_TOLERANCE)
			{
				hmi_engine_get_scale_rect(&hmi_screen_rect,pfather_alpha_scale,&hmi_scale_rect);
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
									&hmi_scale_rect,
									&(new_cliped_farther_rect ));
			}
			else
			{
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
									&hmi_screen_rect,
									&(new_cliped_farther_rect ));
			}
			hmi_get_intersec_rect_depth((HMI_RECT_STR CONST *)(&(new_cliped_farther_rect)),
								depth,(HMI_RECT_STR  *)pdirty_rect,&hmi_clip_rect);
			#endif

			#if (defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_OPENGLES))
			if((hmi_clip_rect.x != HMI_INVALID_COOR)||
					(hmi_clip_rect.y != HMI_INVALID_COOR))
			{
				hmi_spline_prop.line_w		= 0U;
				hmi_spline_prop.ppolygon	= NULL;
				hmi_spline_prop.scale		= 1.0f;
				hmi_spline_prop.spline_color.fill_color = 0U;
				hmi_spline_prop.spline_color.fill_flag	= HMI_FILL_NONE;
				hmi_spline_prop.spline_color.spline_texture.ptex_attr	= NULL;
				hmi_spline_prop.spline_color.spline_texture.ptex_prop	= NULL;
				hmi_spline_prop.spline_color.spline_texture.ptex_rect	= NULL;
				hmi_spline_prop.spline_color.spline_texture.tex_id		= HMI_NB_ELEMENTS;
				hmi_spline_prop.spline_zone_color.fill_color	= 0U;
				hmi_spline_prop.spline_zone_color.fill_flag		= HMI_FILL_NONE;
				hmi_spline_prop.spline_zone_color.spline_texture.ptex_attr	= NULL;
				hmi_spline_prop.spline_zone_color.spline_texture.ptex_prop	= NULL;
				hmi_spline_prop.spline_zone_color.spline_texture.ptex_rect	= NULL;
				hmi_spline_prop.spline_zone_color.spline_texture.tex_id		= HMI_NB_ELEMENTS;
				hmi_spline_prop.point_anticlockwise							= FALSE;
				hmi_spline_prop.spline_attr = 0U;
				hmi_clear_spline_point_polyline(&hmi_spline_input_point);
				
				hmi_spline_prop_id	= hmi_sxy_spline_color[hmi_object_index];
				if(hmi_spline_prop_id !=HMI_NB_ELEMENTS)
				{
					hmi_get_color_success = hmi_spline_get_id_prop(hmi_spline_prop_id,&spline_color);
				}

				hmi_spline_prop_id	= hmi_sxy_spline_zone_color[hmi_object_index];
				if(hmi_spline_prop_id !=HMI_NB_ELEMENTS)
				{
					hmi_get_zone_color_success =hmi_spline_get_id_prop(hmi_spline_prop_id,&spline_zone_color);
				}

				hmi_spline_prop_id	= hmi_sxy_spline_ctrl_point[hmi_object_index];
				if(hmi_spline_prop_id !=HMI_NB_ELEMENTS)
				{
					hmi_get_object_wh(hmi_spline_prop_id,&spline_width,&height);
				}
				#if (HMI_SXY_SPLINE_NUMBER -HMI_SXY_ONE_POINT_SPLINE_NUMBER) > 0
				if(HMI_IS_SXY_MUL_SPLINE(hmi_object_id))
				{ 
					if(hmi_spline_prop_id != HMI_NB_ELEMENTS)
					{
						hmi_get_ctrl_point_success	= hmi_write_mul_spline_point(&(hmi_screen_rect),hmi_spline_prop_id,
																				hmi_sxy_spline_attr_dir[hmi_object_index]);
						hmi_spline_prop.line_w		= spline_width;
						hmi_spline_prop.scale		= 1.0;
						hmi_spline_prop.ppolygon	= &hmi_spline_input_point;
						hmi_spline_prop.point_anticlockwise = FALSE;
						hmi_spline_prop.spline_attr	= hmi_sxy_spline_attr_dir[hmi_object_index];
						if(hmi_get_color_success == TRUE)
						{
							hmi_spline_prop.spline_color.fill_color 	= spline_color.fill_color;
							hmi_spline_prop.spline_color.fill_flag		= spline_color.fill_flag;
							hmi_spline_prop.spline_color.spline_texture = spline_color.spline_texture;
						}
						if(hmi_get_zone_color_success == TRUE)
						{
							hmi_spline_prop.spline_zone_color.fill_color		= spline_zone_color.fill_color;
							hmi_spline_prop.spline_zone_color.fill_flag			= spline_zone_color.fill_flag;
							hmi_spline_prop.spline_zone_color.spline_texture	= spline_zone_color.spline_texture;
						}
						
						if(hmi_get_ctrl_point_success == TRUE)
						{
							   call_C_hmi_driver_draw_spline(&(hmi_screen_rect)/*node screen position*/, 
															   &hmi_spline_prop/*image format*/,
															   &hmi_clip_rect,
															   pdirty_rect,
															   depth,
															   hmi_object_id,
															   pfather_alpha_scale);
						}
					}
				}
				else 
				#endif
				#if HMI_SXY_ONE_POINT_SPLINE_NUMBER	>0
				if(HMI_IS_SXY_ONE_SPLINE(hmi_object_id))
				{
					hmi_object_index2 =HMI_GET_SXY_ONE_SPLINE_INDEX(hmi_object_id);
					if(hmi_spline_prop_id !=HMI_NB_ELEMENTS)
					{
						hmi_get_ctrl_point_success =hmi_write_one_spline_point(&(hmi_screen_rect),
																				sxy_one_point_array,
																				&sxy_one_point_range_array[hmi_object_index2],
																				&sxy_spline_head_tail_offset_array[hmi_object_index2],
																				hmi_sxy_spline_attr_dir[hmi_object_index]);
						hmi_spline_prop.line_w		= spline_width;
						hmi_spline_prop.scale		= 1.0f;
						hmi_spline_prop.ppolygon	= &hmi_spline_input_point;
						hmi_spline_prop.point_anticlockwise = FALSE;//TRUE;
						hmi_spline_prop.spline_attr	= hmi_sxy_spline_attr_dir[hmi_object_index];
						if(hmi_get_color_success == TRUE)
						{
							hmi_spline_prop.spline_color.fill_color		= spline_color.fill_color;
							hmi_spline_prop.spline_color.fill_flag		= spline_color.fill_flag;
							hmi_spline_prop.spline_color.spline_texture	= spline_color.spline_texture;
						}
						if(hmi_get_zone_color_success == TRUE)
						{
							hmi_spline_prop.spline_zone_color.fill_color		= spline_zone_color.fill_color;
							hmi_spline_prop.spline_zone_color.fill_flag			= spline_zone_color.fill_flag;
							hmi_spline_prop.spline_zone_color.spline_texture	= spline_zone_color.spline_texture;
						}
						
						if(hmi_get_ctrl_point_success	== TRUE)
						{
						   call_C_hmi_driver_draw_spline(&(hmi_screen_rect)/*node screen position*/, 
														   &hmi_spline_prop/*image format*/,
														   &hmi_clip_rect,
														   pdirty_rect,
														   depth,
														   hmi_object_id,
														   pfather_alpha_scale);
						}
					}
				}
				else 
				#endif
				{
				}
				
				#if HMI_SXY_SPLINE_MAX_SON_CNT > 0
				hmi_engine_draw_container(&hmi_sxy_spline_table[hmi_object_index],
								   (HMI_RECT_STR CONST *)(&(hmi_screen_rect )),
								   pdirty_rect,
								   depth,&new_cliped_farther_rect/*all father insection zone*/,&container_alpha_scale);
				#endif
			}
			#endif
		}
		else
		#endif
		
		#if HMI_DXY_CUSTOM_CNT > 0
		if(HMI_IS_DYN_XY_CUSTOM(hmi_object_id))
		{
		//hmi_object_index = HMI_GET_DXY_CUSTOM_INDEX(hmi_object_id);
		if(hmi_object_id > HMI_SXY_SPLINE_MAX_ID)
		{
			hmi_object_index = HMI_GET_DXY_CUSTOM_INDEX(hmi_object_id);
		}
		else
		{
			hmi_object_index = 0U;
		}
	#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
			/*element draw rect backups*/
			get_success = hmi_engine_get_custom_prop(hmi_object_id,&custom_object_prop);
			if((get_success == TRUE)&&(hmi_object_index < HMI_DXY_CUSTOM_CNT))
			{
				hmi_custom_dyn_xy_rect_bck[hmi_object_index].x = custom_object_prop.x;
				hmi_custom_dyn_xy_rect_bck[hmi_object_index].y = custom_object_prop.y;
				hmi_custom_dyn_xy_rect_bck[hmi_object_index].w = custom_object_prop.w;
				hmi_custom_dyn_xy_rect_bck[hmi_object_index].h = custom_object_prop.h;
			}
	#endif
			hmi_get_success = hmi_engine_get_dyn_custom_index(hmi_object_index,&hmi_custom_type_index);
			if(hmi_get_success == TRUE)
			{
				phmi_custom_function = hmi_dxy_custom_widget_info[hmi_custom_type_index].attr_fun.pmanager_fun;
				if(phmi_custom_function != NULL)
				{
				#ifdef HMI_GRAPHIC_OPENGLES										
					phmi_custom_function(pfarther_rect,
											pdirty_rect,
											depth,
											pcliped_farther_rect,
											hmi_object_id,
											pfather_alpha_scale,
											NULL);					
				#else
					phmi_custom_function(pfarther_rect,
												pdirty_rect,
												depth,
												pcliped_farther_rect,
												hmi_object_id,
												pfather_alpha_scale);												
				#endif
				}
			}
			
		}
		else
		#endif
		#if HMI_STATIC_TEXTS_NUMBER/*uneditable text*/ > 0 
		if(HMI_IS_STATIC_TEXTS(hmi_object_id)) 
		{	         
			#if HMI_UNEDIT_TEXTS_DYN_XY_NUMBER > 0
			if(HMI_IS_UNEDIT_TEXT_DYN_XY(hmi_object_id)/*HMI_UNEDIT_TEXTS_DYN_XY_NUMBER*/)
			{	
				hmi_object_index  = HMI_GET_DYN_XY_UNEDIT_POS_COLOR_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_UNEDIT_TEXTS_DYN_XY_NUMBER)
				{
					hmi_get_object_screen_coor((HMI_RECT_STR CONST *)pfarther_rect,
											&hmi_dyn_xy_unedit_text_prop_table[hmi_object_index].text_rect,
											&(hmi_screen_rect));
					#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
					/*element draw rect backups*/
					hmi_dyn_xy_unedit_text_prop_table_bck[hmi_object_index].x	= hmi_dyn_xy_unedit_text_prop_table[hmi_object_index].text_rect.x;
					hmi_dyn_xy_unedit_text_prop_table_bck[hmi_object_index].y	= hmi_dyn_xy_unedit_text_prop_table[hmi_object_index].text_rect.y;
					hmi_dyn_xy_unedit_text_prop_table_bck[hmi_object_index].w	= hmi_dyn_xy_unedit_text_prop_table[hmi_object_index].text_rect.w;
					hmi_dyn_xy_unedit_text_prop_table_bck[hmi_object_index].h	= hmi_dyn_xy_unedit_text_prop_table[hmi_object_index].text_rect.h;
					#endif
				}
				#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||\
					defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||\
					defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||\
					defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))
					
				if((fabs(pfather_alpha_scale->scale - HMI_SCALE_1)) > HMI_FLOAT_TOLERANCE)
				{
					hmi_engine_get_scale_rect(&hmi_screen_rect,pfather_alpha_scale,&hmi_scale_rect);
					hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
										&hmi_scale_rect,
										&(new_cliped_farther_rect ));
				}
				else
				{
					hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
										&hmi_screen_rect,
										&(new_cliped_farther_rect ));
				}
				hmi_get_intersec_rect_depth((HMI_RECT_STR CONST *)(&(new_cliped_farther_rect)),
								depth,(HMI_RECT_STR  *)pdirty_rect,&hmi_clip_rect);

				#endif
				text_prop.text_rect.x=hmi_screen_rect.x;
				text_prop.text_rect.y=hmi_screen_rect.y;
				text_prop.text_rect.w=hmi_screen_rect.w;
				text_prop.text_rect.h=hmi_screen_rect.h;
				if(hmi_object_index < HMI_UNEDIT_TEXTS_DYN_XY_NUMBER)
				{
					text_prop.color=hmi_dyn_xy_unedit_text_prop_table[hmi_object_index].color;
				}

				#if HMI_UNEDIT_TEXT_DXY_DYN_FONT_NUM > 0
				if(HMI_IS_UNEDIT_TEXT_DYN_XY_DYN_FONT(hmi_object_id))
				{
					hmi_object_index	= HMI_GET_DYN_XY_UNEDIT_DYN_FONT_INDEX(hmi_object_id);
					if(hmi_object_index < HMI_UNEDIT_TEXT_DXY_DYN_FONT_NUM)
					{
						font_id	= hmi_unedit_text_dxy_dyn_font_table[hmi_object_index];
					}
				}
				#endif
				hmi_object_index = HMI_GET_STATIC_TEXT_ID_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_STATIC_TEXTS_NUMBER)
				{
					text_info.properties=hmi_unedit_text_table[hmi_object_index].properties;
					text_info.font_id	=hmi_unedit_text_table[hmi_object_index].font_id;
					text_info.length	=hmi_unedit_text_table[hmi_object_index].length;
					pp_str_list=(HMI_CHAR_STR **)(hmi_unedit_text_table[hmi_object_index].hmi_string);
					if(HMI_IS_NB_LANGUAGE(hmi_cur_language))
					{
						hmi_object_index=HMI_GET_NB_LANGUAGE_ID_INDEX(hmi_cur_language);
						if(hmi_object_index < HMI_LANGUAGE_NUMBER)
						{									
							text_info.hmi_string=(void *)pp_str_list[hmi_object_index];
						}
					}
				}
				#ifdef HMI_GRAPHIC_AGG
				call_C_hmi_driver_draw_text(
				 &text_prop,
				 &text_info/*hmi_edit_text_table[hmi_object_index]*/,
				 &hmi_clip_rect
				 #ifdef HMI_CLIP_TEXT 
				,hmi_clip_text
				#endif
				);
				hmi_object_index = HMI_GET_STATIC_TEXT_ID_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_STATIC_TEXTS_NUMBER)
				{
					#if HMI_UNEDIT_TEXT_MAX_SON_CNT > 0
					hmi_engine_draw_container(&hmi_static_text_container_table[hmi_object_index],
										(HMI_RECT_STR CONST *)&(hmi_screen_rect ),											
										pdirty_rect,depth,
										&new_cliped_farther_rect/*all father insection zone*/);
					#endif
				}
				#endif

				#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
				if((hmi_clip_rect.x!=HMI_INVALID_COOR)||
				(hmi_clip_rect.y!=HMI_INVALID_COOR))
				{
						#if HMI_ALL_FONT_NUMBER > 0
					call_C_hmi_driver_draw_text(
					     (HMI_TEXT_RECT_STR CONST *)(&text_prop),
					     (HMI_TEXT_PROP_STR CONST * )(&text_info)/*&hmi_edit_text_table[hmi_object_index]*/,
					     &hmi_clip_rect,
					     pdirty_rect,
					     font_id,
					      depth
					     #ifdef HMI_CLIP_TEXT 
						,hmi_clip_text
						#endif
						,pfather_alpha_scale
						);
						#endif
					hmi_object_index = HMI_GET_STATIC_TEXT_ID_INDEX(hmi_object_id);
					if(hmi_object_index < HMI_STATIC_TEXTS_NUMBER)
					{
						#if HMI_UNEDIT_TEXT_MAX_SON_CNT > 0
						hmi_engine_draw_container(&hmi_static_text_container_table[hmi_object_index],
												(HMI_RECT_STR CONST *)&(hmi_screen_rect ),
												pdirty_rect,
												depth,&new_cliped_farther_rect/*all father insection zone*/
												,&container_alpha_scale);
						#endif
					}
				}
				#endif

				#ifdef HMI_GRAPHIC_ST7513
				call_C_hmi_driver_draw_text(
							(HMI_TEXT_RECT_STR CONST *)(&text_prop),
							(HMI_TEXT_PROP_STR CONST *)(&text_info)/*&hmi_edit_text_table[hmi_object_index]*/,
							&hmi_clip_rect,
							font_id
							#ifdef HMI_CLIP_TEXT 
							,hmi_clip_text
							#endif
							);
				hmi_object_index = HMI_GET_STATIC_TEXT_ID_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_STATIC_TEXTS_NUMBER)
				{
					#if HMI_UNEDIT_TEXT_MAX_SON_CNT > 0
					hmi_engine_draw_container(&hmi_static_text_container_table[hmi_object_index],
										(HMI_RECT_STR CONST *)&(hmi_screen_rect ),
										pdirty_rect,
										depth,&new_cliped_farther_rect/*all father insection zone*/);
					#endif
				}
				#endif

				#ifdef HMI_GRAPHIC_TWLIB
				if(hmi_object_index < HMI_STATIC_TEXTS_NUMBER)
				{
					#if HMI_RENDER_ALL_EXCEPT_BCK==NO
					if(((!only_draw_flag)||!hmi_is_render_mode())||/*simulate*/
							(only_draw_flag&&(flag)))
						#endif
					{
						call_C_hmi_driver_draw_text(
						     (HMI_TEXT_RECT_STR CONST *)(&text_prop),
						     &hmi_unedit_text_table[hmi_object_index]
						     #if HMI_RENDER_ALL_EXCEPT_BCK==NO
							,parent_object_id
							,hmi_object_id_const
							#endif
						     );
					}
				}			
				#if 0
				hmi_engine_draw_container(&hmi_static_text_container_table[hmi_object_index],
								(HMI_RECT_STR CONST *)&(text_prop.text_rect),
								&hmi_union_rect,
								pdirty_rect);
				#endif /*tw not support*/
				#endif

				#if (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
				if(flag || draw_flag)
				{
					#if HMI_ALL_FONT_NUMBER > 0
					call_C_hmi_driver_draw_text(
					     (HMI_TEXT_RECT_STR CONST *)(&text_prop),
					     (HMI_TEXT_PROP_STR CONST * )(&text_info)/*&hmi_edit_text_table[hmi_object_index]*/,
					     font_id
			     		#ifdef HMI_CLIP_TEXT 
						,hmi_clip_text
						#endif
						);
					#endif
				}
				hmi_set_cur_win_used();
				hmi_set_text_cur_index_used();
				hmi_object_index = HMI_GET_STATIC_TEXT_ID_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_STATIC_TEXTS_NUMBER)
				{
					#if HMI_UNEDIT_TEXT_MAX_SON_CNT > 0
					hmi_engine_draw_container(&hmi_static_text_container_table[hmi_object_index],
										(HMI_RECT_STR CONST *)&(hmi_screen_rect ),
										child_draw_flag
										);
					#endif
				}
				#endif
			}
			else 
			#endif
			{
				#if HMI_UNEDIT_TEXTS_STATIC_XY_NUMBER > 0					
				if(HMI_IS_UNEDIT_TEXT_STATIC_XY(hmi_object_id)/*hmi_object_index < HMI_UNEDIT_TEXTS_STATIC_XY_NUMBER*/)
				{
					hmi_object_index = HMI_GET_S_XY_UNEDIT_POS_COLOR_INDEX(hmi_object_id);
					if(hmi_object_index < HMI_UNEDIT_TEXTS_STATIC_XY_NUMBER)
					{
						hmi_get_object_screen_coor((HMI_RECT_STR CONST *)pfarther_rect,
											&hmi_static_xy_unedit_text_prop_table[hmi_object_index].text_rect,
											&(hmi_screen_rect));
					}										
					#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||\
						defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||\
						defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||\
						defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))
						
					if((fabs(pfather_alpha_scale->scale - HMI_SCALE_1)) > HMI_FLOAT_TOLERANCE)
					{
						hmi_engine_get_scale_rect(&hmi_screen_rect,pfather_alpha_scale,&hmi_scale_rect);
						hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
												&hmi_scale_rect,
												&(new_cliped_farther_rect ));
					}
					else
					{
						hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
									&hmi_screen_rect,
									&(new_cliped_farther_rect ));
					}
					hmi_get_intersec_rect_depth((HMI_RECT_STR CONST *)(&(new_cliped_farther_rect)),
								depth,(HMI_RECT_STR  *)pdirty_rect,&hmi_clip_rect);
					#endif	
					text_prop.text_rect.x=hmi_screen_rect.x;
					text_prop.text_rect.y=hmi_screen_rect.y;
					text_prop.text_rect.w=hmi_screen_rect.w;
					text_prop.text_rect.h=hmi_screen_rect.h;
					if(hmi_object_index < HMI_UNEDIT_TEXTS_STATIC_XY_NUMBER)
					{
						text_prop.color=hmi_static_xy_unedit_text_prop_table[hmi_object_index].color;
					}
					hmi_object_index = HMI_GET_STATIC_TEXT_ID_INDEX(hmi_object_id);
					if(hmi_object_index < HMI_STATIC_TEXTS_NUMBER)
					{
						text_info.properties=hmi_unedit_text_table[hmi_object_index].properties;
						text_info.font_id	=hmi_unedit_text_table[hmi_object_index].font_id;
						text_info.length	=hmi_unedit_text_table[hmi_object_index].length;
						pp_str_list=(HMI_CHAR_STR **)(hmi_unedit_text_table[hmi_object_index].hmi_string);
						if(HMI_IS_NB_LANGUAGE(hmi_cur_language))
						{
							hmi_object_index	= HMI_GET_NB_LANGUAGE_ID_INDEX(hmi_cur_language);
							if(hmi_object_index < HMI_LANGUAGE_NUMBER)
							{		
								text_info.hmi_string=(void *)pp_str_list[hmi_object_index];
							}
						}
					}
					#ifdef HMI_GRAPHIC_AGG
					hmi_object_index = HMI_GET_UNEDIT_INDEX(hmi_object_id);
					if(hmi_object_index < HMI_STATIC_TEXTS_NUMBER)
					{
			            call_C_hmi_driver_draw_text(&text_prop,
													text_info/*&hmi_unedit_text_table[hmi_object_index]*/,
													&hmi_clip_rect
												#ifdef HMI_CLIP_TEXT 
													,hmi_clip_text
												#endif
													);
						#if HMI_UNEDIT_TEXT_MAX_SON_CNT > 0
						hmi_engine_draw_container(&hmi_static_text_container_table[hmi_object_index],
												(HMI_RECT_STR CONST *)(&(hmi_screen_rect )),												
												pdirty_rect,depth,
												&new_cliped_farther_rect/*all father insection zone*/);
						#endif
					}
					#endif	
					
					#if HMI_UNEDIT_TEXT_SXY_DYN_FONT_NUM > 0
					if(HMI_IS_UNEDIT_TEXT_STATIC_XY_DYN_FONT(hmi_object_id))
					{
						hmi_object_index	= HMI_GET_S_XY_UNEDIT_DYN_FONT_INDEX(hmi_object_id);
						if(hmi_object_index < HMI_UNEDIT_TEXT_SXY_DYN_FONT_NUM)
						{
							font_id	= hmi_unedit_text_sxy_dyn_font_table[hmi_object_index];
						}
					}
					#endif

					
					#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
					hmi_object_index = HMI_GET_UNEDIT_INDEX(hmi_object_id);
					if(hmi_object_index < HMI_STATIC_TEXTS_NUMBER)
					{
						if((hmi_clip_rect.x!=HMI_INVALID_COOR)||
						(hmi_clip_rect.y!=HMI_INVALID_COOR))
						{
						#if HMI_ALL_FONT_NUMBER > 0
						call_C_hmi_driver_draw_text((HMI_TEXT_RECT_STR CONST *)(&text_prop),
													(HMI_TEXT_PROP_STR CONST *)(&text_info)/*&hmi_unedit_text_table[hmi_object_index]*/,
													&hmi_clip_rect,
													pdirty_rect,
													font_id,
													 depth
												#ifdef HMI_CLIP_TEXT 
													,hmi_clip_text
												#endif
												,pfather_alpha_scale
													);
						#endif
						#if HMI_UNEDIT_TEXT_MAX_SON_CNT > 0
						hmi_engine_draw_container(&hmi_static_text_container_table[hmi_object_index],
												(HMI_RECT_STR CONST *)(&(hmi_screen_rect )),
												pdirty_rect,
												depth,&new_cliped_farther_rect/*all father insection zone*/
												,&container_alpha_scale);
						
						#endif
						}
					}
					#endif

					#ifdef HMI_GRAPHIC_ST7513
					hmi_object_index = HMI_GET_UNEDIT_INDEX(hmi_object_id);
					if(hmi_object_index < HMI_STATIC_TEXTS_NUMBER)
					{
						call_C_hmi_driver_draw_text((HMI_TEXT_RECT_STR CONST *)(&text_prop),
													(HMI_TEXT_PROP_STR CONST *)(&text_info),
													&hmi_clip_rect,
													font_id
												#ifdef HMI_CLIP_TEXT 
													,hmi_clip_text
												#endif
													);
						#if HMI_UNEDIT_TEXT_MAX_SON_CNT > 0					
						hmi_engine_draw_container(&hmi_static_text_container_table[hmi_object_index],
												(HMI_RECT_STR CONST *)(&(hmi_screen_rect )),
												pdirty_rect,
												depth,&new_cliped_farther_rect/*all father insection zone*/);
						#endif
					}
					#endif
					
					#ifdef HMI_GRAPHIC_TWLIB	
					hmi_object_index = HMI_GET_UNEDIT_INDEX(hmi_object_id);
					if(hmi_object_index < HMI_STATIC_TEXTS_NUMBER)
					{
						#if HMI_RENDER_ALL_EXCEPT_BCK==NO
						if(((!only_draw_flag)||!hmi_is_render_mode())||/*simulate*/
								(only_draw_flag&&(flag)))
						#endif
						{
				            call_C_hmi_driver_draw_text((HMI_TEXT_RECT_STR CONST *)(&text_prop),
				                     (HMI_TEXT_PROP_STR CONST *)(&text_info)
				                     #if HMI_RENDER_ALL_EXCEPT_BCK==NO
									,parent_object_id
									,hmi_object_id_const
									#endif
				                     );
						}
					}
					#if 0
					hmi_engine_draw_container(&hmi_edit_text_container_table[hmi_object_id],
											(HMI_RECT_STR CONST *)&(text_prop.text_rect),
											&hmi_union_rect,
											pdirty_rect);
					#endif  /*tw not support */
					#endif
					
					#if (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
					hmi_object_index = HMI_GET_UNEDIT_INDEX(hmi_object_id);
					if(hmi_object_index < HMI_STATIC_TEXTS_NUMBER)
					{
						if(flag || draw_flag)
						{
						#if HMI_ALL_FONT_NUMBER > 0
						call_C_hmi_driver_draw_text((HMI_TEXT_RECT_STR CONST *)(&text_prop),
													(HMI_TEXT_PROP_STR CONST *)(&text_info)/*&hmi_unedit_text_table[hmi_object_index]*/,
													font_id
													#ifdef HMI_CLIP_TEXT 
													,hmi_clip_text
													#endif
													);
						#endif
						}
						hmi_set_cur_win_used();
						hmi_set_text_cur_index_used();
						#if HMI_UNEDIT_TEXT_MAX_SON_CNT > 0
						hmi_engine_draw_container(&hmi_static_text_container_table[hmi_object_index],
												(HMI_RECT_STR CONST *)(&(hmi_screen_rect )),
												child_draw_flag
												);
						#endif
					}
					#endif
				}
			#endif
			}
		} 
		else
	   #endif
		
	   /*draw static element*/
		#if HMI_STATIC_FILL_PAGES_NUMBER > 0 
		if(HMI_IS_SXY_FILL_PAGES(hmi_object_id))
		{
			hmi_object_index  = HMI_GET_SXY_FILL_PAGES_ID_INDEX(hmi_object_id );
			if(hmi_object_index < HMI_STATIC_FILL_PAGES_NUMBER)
			{
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
					(&hmi_fills_static_xy_rect[hmi_object_index]),
					(&(hmi_screen_rect)));
			}
			#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||\
				defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||\
				defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||	\
				defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))			
				if((fabs(pfather_alpha_scale->scale - HMI_SCALE_1)) > HMI_FLOAT_TOLERANCE)
				{
					hmi_engine_get_scale_rect(&hmi_screen_rect,pfather_alpha_scale,&hmi_scale_rect);
					hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
											&hmi_scale_rect,
											&(new_cliped_farther_rect ));
				}
				else
				{
					hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
								&hmi_screen_rect,
								&(new_cliped_farther_rect ));
				}
			hmi_get_intersec_rect_depth((HMI_RECT_STR CONST *)(&(new_cliped_farther_rect)),
								depth,(HMI_RECT_STR  *)pdirty_rect,&hmi_clip_rect);
			#endif

			#ifdef HMI_GRAPHIC_AGG
			call_C_hmi_driver_draw_fill_page(&hmi_fills_static_prop_table[hmi_object_index ],
			 								&hmi_clip_rect);
			hmi_engine_draw_container(&hmi_static_fill_container_table[hmi_object_index],
							(HMI_RECT_STR CONST *)(&(hmi_screen_rect )),
							pdirty_rect,depth,
							&new_cliped_farther_rect/*all father insection zone*/);
			#endif
			
			#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
			if((hmi_clip_rect.x!=HMI_INVALID_COOR)||
				(hmi_clip_rect.y!=HMI_INVALID_COOR))
			{
			call_C_hmi_driver_draw_fill_page(&hmi_fills_static_prop_table[hmi_object_index ],
			 								&hmi_clip_rect,
			 								pdirty_rect,
			 								depth,
			 								pfather_alpha_scale);
			#if HMI_SXY_FILL_MAX_SON_CNT > 0
			hmi_alpha=HMI_RGL_ALLPHA(hmi_fills_static_prop_table[hmi_object_index ].color);
			if(container_alpha_scale.alpha > hmi_alpha)
			{
				container_alpha_scale.alpha = hmi_alpha;
			}
			hmi_engine_draw_container(&hmi_static_fill_container_table[hmi_object_index],
							(HMI_RECT_STR CONST *)(&(hmi_screen_rect )),
							pdirty_rect,
							depth,&new_cliped_farther_rect/*all father insection zone*/
							,&container_alpha_scale);
			#endif
			}
			#endif

			#ifdef HMI_GRAPHIC_ST7513
			call_C_hmi_driver_draw_fill_page((HMI_RECT_STR CONST *)(&(hmi_screen_rect)),
											&hmi_fills_static_prop_table[hmi_object_index],
			 								&hmi_clip_rect);
			hmi_engine_draw_container(&hmi_static_fill_container_table[hmi_object_index],
							(HMI_RECT_STR CONST *)(&(hmi_screen_rect )),
							pdirty_rect,
							depth,&new_cliped_farther_rect/*all father insection zone*/);
			#endif

			#ifdef HMI_GRAPHIC_TWLIB 
			bMerge=hmi_get_is_merge(parent_object_id_type,HMI_ELEM_TYPE_SFILL); 
			#if HMI_RENDER_ALL_EXCEPT_BCK==NO
			if(((!only_draw_flag)||!hmi_is_render_mode())||/*simulate*/
								(only_draw_flag&&(flag)))
			#endif
			{
				call_C_hmi_driver_draw_fill_page(&hmi_fills_static_prop_table[hmi_object_index ],	             
												&(hmi_screen_rect),
				 								bMerge
				 								#if HMI_RENDER_ALL_EXCEPT_BCK==NO
												,parent_object_id
												,hmi_object_id_const
												#endif
				 							);				
			}
			if(hmi_static_fill_container_table[hmi_object_index].container_object_table.object_number>0)
			{
				hmi_engine_draw_container(&(hmi_static_fill_container_table[hmi_object_index]),
												(HMI_RECT_STR CONST *)(&(hmi_screen_rect)),
												HMI_ELEM_TYPE_SFILL ,
												hmi_object_id_const,
												FALSE
												#if HMI_RENDER_ALL_EXCEPT_BCK==NO
													,phmi_object_prop_table->object_id 
													,only_draw_flag&&(!flag)
												#endif
												); 
			}
			#endif
			
			#if (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
			if(flag ||draw_flag)
			{
				//call_C_hmi_driver_draw_fill_page(&hmi_fills_static_prop_table[hmi_object_id ]);
			}
			//hmi_set_cur_win_used();
			
			#if HMI_SXY_FILL_MAX_SON_CNT > 0
			hmi_engine_draw_container(&hmi_static_fill_container_table[hmi_object_id],
							(HMI_RECT_STR CONST *)(&(hmi_screen_rect )),
							child_draw_flag
							);
			#endif
			#endif
		}
		else
	   #endif
	   
		#if HMI_STATIC_GFILL_NUMBER> 0
		if(HMI_IS_SXY_GFILL_PAGES(hmi_object_id))
		{
			hmi_object_index  = HMI_GET_SXY_GFILL_PAGES_ID_INDEX(hmi_object_id);
			HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
						(&hmi_gradient_sxy_fill_rect[hmi_object_index]),
						(&hmi_screen_rect));
			
			#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||\
				defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||\
				defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||\
				defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))			
				if((fabs(pfather_alpha_scale->scale - HMI_SCALE_1)) > HMI_FLOAT_TOLERANCE)
				{
					hmi_engine_get_scale_rect(&hmi_screen_rect,pfather_alpha_scale,&hmi_scale_rect);
					hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
											&hmi_scale_rect,
											&(new_cliped_farther_rect ));
				}
				else
				{
					hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
								&hmi_screen_rect,
								&(new_cliped_farther_rect ));
				}
			hmi_get_intersec_rect_depth((HMI_RECT_STR CONST *)(&(new_cliped_farther_rect)),
								depth,(HMI_RECT_STR  *)pdirty_rect,&hmi_clip_rect);
			#endif
			#ifdef  HMI_GRAPHIC_AGG
			call_C_hmi_driver_gradient_fill_page(
									&(hmi_screen_rect),
									&hmi_gradient_sxy_fill_table[hmi_object_index],
									&hmi_clip_rect);
			hmi_engine_draw_container(&hmi_static_gfill_container_table[hmi_object_index],
				(HMI_RECT_STR CONST *)&(text_prop.text_rect),&hmi_union_rect,pdirty_rect);
			#endif

			#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
			if((hmi_clip_rect.x!=HMI_INVALID_COOR)||
				(hmi_clip_rect.y!=HMI_INVALID_COOR))
			{
				call_C_hmi_driver_gradient_fill_page(&(hmi_screen_rect)/*node screen position*/,
						&hmi_gradient_sxy_fill_table[hmi_object_index],
						&hmi_clip_rect,
						pdirty_rect,
						0,
						pfather_alpha_scale);
			#if HMI_SXY_GFILL_MAX_SON_CNT > 0
			hmi_alpha=HMI_RGL_ALLPHA(hmi_gradient_sxy_fill_table[hmi_object_index ].color1);
			if(container_alpha_scale.alpha > hmi_alpha)
			{
				container_alpha_scale.alpha = hmi_alpha;
			}
			hmi_engine_draw_container(&hmi_static_gfill_container_table[hmi_object_index],
					(HMI_RECT_STR CONST *)&(hmi_screen_rect ),pdirty_rect,depth,
					&new_cliped_farther_rect/*all father insection zone*/
					,&container_alpha_scale);
			#endif
			}
			#endif

			#ifdef HMI_GRAPHIC_ST7513
			call_C_hmi_driver_gradient_fill_page(&(hmi_screen_rect)/*node screen position*/,
						&hmi_gradient_sxy_fill_table[hmi_object_index],
						&hmi_clip_rect);
			hmi_engine_draw_container(&hmi_static_gfill_container_table[hmi_object_index],
					(HMI_RECT_STR CONST *)&(hmi_screen_rect ),pdirty_rect,depth,
					&new_cliped_farther_rect/*all father insection zone*/);
			#endif

			#ifdef  HMI_GRAPHIC_TWLIB
			#if 0
			call_C_hmi_driver_gradient_fill_page(
			&(text_prop.text_rect),
			&hmi_gradient_sxy_fill_table[hmi_object_id],&hmi_draw_rect);
			hmi_engine_draw_container(&hmi_static_gfill_container_table[hmi_object_id],(HMI_RECT_STR CONST *)&(text_prop.text_rect),&hmi_union_rect,pdirty_rect);
			#endif
			#endif
			
			#if (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
			
			#endif
			 
		}
		else
		#endif
		
		#if HMI_SXY_CUBE_NUMBER> 0
		if(HMI_IS_SXY_CUBE(hmi_object_id))
		{	         
	        hmi_object_index = HMI_GET_SXY_CUBE_ID_INDEX(hmi_object_id);			
		    if(hmi_object_index < HMI_SXY_CUBE_NUMBER)
		    {
		       	HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
					(&hmi_cubes_static_xy_rect[hmi_object_index].cube_rect),
					(&(hmi_screen_rect)));				
				#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
				if((fabs(pfather_alpha_scale->scale - HMI_SCALE_1)) > HMI_FLOAT_TOLERANCE)
				{
					hmi_engine_get_scale_rect(&hmi_screen_rect,pfather_alpha_scale,&hmi_scale_rect);
					hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
											&hmi_scale_rect,
											&(new_cliped_farther_rect ));
				}
				else
				{
					hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
								&hmi_screen_rect,
								&(new_cliped_farther_rect ));
				}
				hmi_get_intersec_rect_depth((HMI_RECT_STR CONST *)(&(new_cliped_farther_rect)),
								depth,(HMI_RECT_STR  *)pdirty_rect,&hmi_clip_rect);
				hmi_cube_get_container_prop(hmi_cubes_static_xy_face[hmi_object_index].container_object_table.p_object_table[0].object_id,
									&hmi_cube_textrue.cube_face1_texture,
									&hmi_bump_textrue.cube_face1_texture,
									NULL);
				hmi_cube_get_container_prop(hmi_cubes_static_xy_face[hmi_object_index].container_object_table.p_object_table[1].object_id,
									&hmi_cube_textrue.cube_face2_texture,
									&hmi_bump_textrue.cube_face2_texture,
									NULL);
				hmi_cube_get_container_prop(hmi_cubes_static_xy_face[hmi_object_index].container_object_table.p_object_table[2].object_id,
									&hmi_cube_textrue.cube_face3_texture,
									&hmi_bump_textrue.cube_face3_texture,
									NULL);
				hmi_cube_get_container_prop(hmi_cubes_static_xy_face[hmi_object_index].container_object_table.p_object_table[3].object_id,
									&hmi_cube_textrue.cube_face4_texture,
									&hmi_bump_textrue.cube_face4_texture,
									NULL);
				hmi_cube_get_container_prop(hmi_cubes_static_xy_face[hmi_object_index].container_object_table.p_object_table[4].object_id,
									&hmi_cube_textrue.cube_face5_texture,
									&hmi_bump_textrue.cube_face5_texture,
									NULL);
				hmi_cube_get_container_prop(hmi_cubes_static_xy_face[hmi_object_index].container_object_table.p_object_table[5].object_id,		
									&hmi_cube_textrue.cube_face6_texture,
									&hmi_bump_textrue.cube_face6_texture,
									NULL);
							
				if((hmi_clip_rect.x!=HMI_INVALID_COOR)||
				(hmi_clip_rect.y!=HMI_INVALID_COOR))
				{
					//hmi_get_axis(axis,&hmi_cubes_static_xy_axis[hmi_object_index],
								//pfarther_rect);
					hmi_engine_get_cube_attr(cube_rotation_axis,
									&cube_camera,
									hmi_object_id,
									pfarther_rect);
					/*screen coordinate*/
					cube_camera.target.x	+= pfarther_rect->x;
					cube_camera.target.y	+= pfarther_rect->y;
	
					cube_camera.position.x	+= pfarther_rect->x;
					cube_camera.position.y	+= pfarther_rect->y;
			       	call_C_hmi_driver_draw_cube(&(hmi_screen_rect)/*node screen position*/, 
			       			&hmi_cubes_static_xy_rect[hmi_object_index],
			       			&hmi_cube_textrue,
			       			&hmi_bump_textrue,
			       			pcliped_farther_rect,
			       			pdirty_rect,
			       			depth,
			       			//hmi_object_id,
			       			pfather_alpha_scale,
			       			&hmi_cubes_static_xy_axis[hmi_object_index],
			       			//axis,
			       			cube_rotation_axis,
							&cube_camera);
				}
				#endif
		    }
		}
		else
#endif 
		#if HMI_SXY_3DCUBE_NUMBER> 0
				if(HMI_IS_SXY_3DCUBE(hmi_object_id))
				{			 
					hmi_object_index = HMI_GET_SXY_3DCUBE_ID_INDEX(hmi_object_id);			
					if(hmi_object_index < HMI_SXY_3DCUBE_NUMBER)
					{
						HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
							(&hmi_3dcubes_static_xy_rect[hmi_object_index].cube_rect),
							(&(hmi_screen_rect)));				
				#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
				if((fabs(pfather_alpha_scale->scale - HMI_SCALE_1)) > HMI_FLOAT_TOLERANCE)
				{
					hmi_engine_get_scale_rect(&hmi_screen_rect,pfather_alpha_scale,&hmi_scale_rect);
					hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
											&hmi_scale_rect,
											&(new_cliped_farther_rect ));
				}
				else
				{
					hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
								&hmi_screen_rect,
								&(new_cliped_farther_rect ));
				}
				hmi_get_intersec_rect_depth((HMI_RECT_STR CONST *)(&(new_cliped_farther_rect)),
								depth,(HMI_RECT_STR  *)pdirty_rect,&hmi_clip_rect);
				#endif
				#if (defined(HMI_GRAPHIC_OPENGLES))
				if((hmi_clip_rect.x != HMI_INVALID_COOR)||
					(hmi_clip_rect.y != HMI_INVALID_COOR))
				{
						
				hmi_engine_draw_3dcube(
										&(hmi_screen_rect),
										pcliped_farther_rect,
										pdirty_rect,
										hmi_object_id,
										hmi_3dcubes_static_xy_rect[hmi_object_index].node_index);
				}
					
				#endif
				}
				}
				else
#endif 
		#if HMI_SXY_BITMAPS_NUMBER> 0
		if(HMI_IS_S_XY_BITMAP(hmi_object_id))
		{	         
	        hmi_object_index = HMI_GET_SXY_BITMAPS_ID_INDEX(hmi_object_id);			
		    if(hmi_object_index < HMI_SXY_BITMAPS_NUMBER)
		    {
				hmi_rect_temp.x	=	hmi_bmp_static_xy_rect[hmi_object_index].x;
				hmi_rect_temp.y	=	hmi_bmp_static_xy_rect[hmi_object_index].y;
				hmi_rect_temp.w	=	hmi_bmp_static_xy_rect[hmi_object_index].w;
				hmi_rect_temp.h	=	hmi_bmp_static_xy_rect[hmi_object_index].h;
		       	HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
					(&hmi_rect_temp),
					(&(hmi_screen_rect)));				
				#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||\
					defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||\
					defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||\
					defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))
					if((fabs(pfather_alpha_scale->scale - HMI_SCALE_1)) > HMI_FLOAT_TOLERANCE)
					{
						hmi_engine_get_scale_rect(&hmi_screen_rect,pfather_alpha_scale,&hmi_scale_rect);
						hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
												&hmi_scale_rect,
												&(new_cliped_farther_rect ));
					}
					else
					{
						hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
									&hmi_screen_rect,
									&(new_cliped_farther_rect ));
					}
				hmi_get_intersec_rect_depth((HMI_RECT_STR CONST *)(&(new_cliped_farther_rect)),
									depth,(HMI_RECT_STR  *)pdirty_rect,&hmi_clip_rect);
				#endif
				#ifdef HMI_GRAPHIC_AGG
		       	call_C_hmi_driver_draw_image(&(hmi_screen_rect), hmi_bigbitmap_table[0],hmi_bigbitmap_table[1],&hmi_bmp_static_xy_prop_table[hmi_object_index],&hmi_draw_rect);
				hmi_engine_draw_container(&hmi_sxy_bitmap_container_table[hmi_object_index],
										(HMI_RECT_STR CONST *)&(hmi_screen_rect),
										(HMI_RECT_STR CONST *)pdirty_rect,depth,
										&new_cliped_farther_rect/*all father insection zone*/);
				#endif				
				
				#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
				if((hmi_clip_rect.x!=HMI_INVALID_COOR)||
				(hmi_clip_rect.y!=HMI_INVALID_COOR))
				{
					#if 0
					#if HMI_SXY_MUL_TEXTURE_BITMAPS_NUMBER	>0
					if(HMI_IS_SXY_MUL_TEXTURE_BITAMP(hmi_object_id))
					{
						hmi_object_index2 =HMI_GET_SXY_MUL_TEXTURE_BITMAP_ID_INDEX(hmi_object_id);
						call_C_hmi_driver_draw_mul_texture_image(&(hmi_screen_rect)/*node screen position*/, 
																hmi_sxy_bitmap_attr_table[hmi_object_index]/*image format*/,
																(HMI_RECT_ALPHA_ANGEL_STR *)(&hmi_bmp_static_xy_rect[hmi_object_index])/*alpha,rotation*/,
																&hmi_bmp_static_xy_prop_table[hmi_object_index],
																&hmi_clip_rect,
																pdirty_rect,
																depth,
																NULL/*rotation point is center*/,
																hmi_object_id,
																father_alpha,
																&hmi_sxy_texture_rect_list[hmi_object_index2],
																hmi_texture_rect_list);
					}
					else 
					#endif
					#endif
					{
				       	call_C_hmi_driver_draw_image(&(hmi_screen_rect)/*node screen position*/, 
				       			hmi_sxy_bitmap_attr_table[hmi_object_index]/*image format*/,
				       			(HMI_RECT_ALPHA_ANGEL_STR *)(&hmi_bmp_static_xy_rect[hmi_object_index])/*alpha,rotation*/,
				       			&hmi_bmp_static_xy_prop_table[hmi_object_index],
				       			&hmi_clip_rect,
				       			pdirty_rect,
				       			depth,
				       			NULL/*rotation point is center*/,
				       			hmi_object_id,
				       			pfather_alpha_scale);
					}

				#if HMI_SXY_IMAGE_MAX_SON_CNT > 0
				if(container_alpha_scale.alpha >hmi_bmp_static_xy_rect[hmi_object_index].alpha)
				{
					container_alpha_scale.alpha =hmi_bmp_static_xy_rect[hmi_object_index].alpha;
				}
				hmi_engine_draw_container(&hmi_sxy_bitmap_container_table[hmi_object_index],
										(HMI_RECT_STR CONST *)(&(hmi_screen_rect)),
										(HMI_RECT_STR  *)pdirty_rect,depth,
										&new_cliped_farther_rect/*all father insection zone*/
										,&container_alpha_scale);
				#endif
				}
				#endif

				#ifdef HMI_GRAPHIC_ST7513
		       	call_C_hmi_driver_draw_image(&(hmi_screen_rect)/*node screen position*/, 		       			
		       			&hmi_bmp_static_xy_prop_table[hmi_object_index],
		       			&hmi_clip_rect);		       			       			
				hmi_engine_draw_container(&hmi_sxy_bitmap_container_table[hmi_object_index],
										(HMI_RECT_STR CONST *)(&(hmi_screen_rect)),
										(HMI_RECT_STR *)pdirty_rect,depth,
										&new_cliped_farther_rect/*all father insection zone*/);
				#endif
				
				#ifdef HMI_GRAPHIC_TWLIB
				bMerge=hmi_get_is_merge(parent_object_id_type,HMI_ELEM_TYPE_SIMAGE);
				#if HMI_RENDER_ALL_EXCEPT_BCK==NO
		       	if(((!only_draw_flag)||!hmi_is_render_mode())||/*simulate*/
								(only_draw_flag&&(flag)))
				#endif
		       	{
					call_C_hmi_driver_draw_image(&(hmi_screen_rect), FALSE/*&hmi_sxy_bitmap_attr_table[hmi_object_id]*/,
											(HMI_RECT_ALPHA_ANGEL_STR *)(&hmi_bmp_static_xy_rect[hmi_object_index]),
											&hmi_bmp_static_xy_prop_table[hmi_object_index]/*,
											TRUE*/,
											bMerge										
											#if HMI_RENDER_ALL_EXCEPT_BCK==NO
												,parent_object_id
												,hmi_object_id_const
											#endif
											);					
				}
				if(hmi_sxy_bitmap_container_table[hmi_object_index].container_object_table.object_number>0)
				{
					hmi_engine_draw_container(&(hmi_sxy_bitmap_container_table[hmi_object_index]),
												(HMI_RECT_STR CONST *)(&(hmi_screen_rect)),
												HMI_ELEM_TYPE_SIMAGE ,
												hmi_object_id_const,
												FALSE
												#if HMI_RENDER_ALL_EXCEPT_BCK==NO
													,phmi_object_prop_table->object_id 
													,only_draw_flag&&(!flag)
												#endif
												); 
				}
				#endif
				
				#if (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
				if(flag ||draw_flag)
				{
		
		       	call_C_hmi_driver_draw_image(&(hmi_screen_rect)/*node screen position*/, 
		       			&hmi_sxy_bitmap_attr_table[hmi_object_index]/*image format*/,
		       			(HMI_RECT_ALPHA_ANGEL_STR *)(&hmi_bmp_static_xy_rect[hmi_object_index])/*alpha,rotation*/,
		       			&hmi_bmp_static_xy_prop_table[hmi_object_index]
		       			);
				}
				hmi_set_cur_win_used();

				#if HMI_SXY_IMAGE_MAX_SON_CNT > 0
				hmi_engine_draw_container(&hmi_sxy_bitmap_container_table[hmi_object_index],
										(HMI_RECT_STR CONST *)(&(hmi_screen_rect)),
										child_draw_flag
										);
				#endif
				#endif
		    }
		}
		else
		#endif 
	    #if HMI_SXY_CUSTOM_CNT > 0
		if(HMI_IS_CUSTOM_SXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_CUSTOM_SXY_ID_INDEX(hmi_object_id);
			hmi_get_success = hmi_engine_get_static_custom_index(hmi_object_index,&hmi_custom_type_index);
			if(hmi_get_success == TRUE)
			{
				phmi_custom_function = hmi_sxy_custom_widget_info[hmi_custom_type_index].attr_fun.pmanager_fun;
				if(phmi_custom_function != NULL)
				{
				#ifdef HMI_GRAPHIC_OPENGLES
					phmi_custom_function(pfarther_rect,
										pdirty_rect,
										depth,
										pcliped_farther_rect,
										hmi_object_id,
										pfather_alpha_scale,
										NULL);
				#else
					phmi_custom_function(pfarther_rect,
										pdirty_rect,
										depth,
										pcliped_farther_rect,
										hmi_object_id,
										pfather_alpha_scale,
										NULL);
				#endif
				}
			}
			
		}
		else
		#endif	
		#if HMI_SXY_CONTAINERS_NUMBER > 0 
		if(HMI_IS_CONTAINERS_SXY(hmi_object_id))
		{
		  	#if HMI_SXY_CONTAINERS_NUMBER >0
			hmi_object_id = HMI_GET_CONTAINERS_SXY_ID_INDEX(hmi_object_id );
			hmi_get_object_screen_coor((HMI_RECT_STR CONST *)pfarther_rect,
							&hmi_static_container_rect[hmi_object_id],
							&(hmi_screen_rect));
			
			#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||\
				defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||\
				defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||\
				defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))
			if((fabs(pfather_alpha_scale->scale - HMI_SCALE_1)) > HMI_FLOAT_TOLERANCE)
			{
				hmi_engine_get_scale_rect(&hmi_screen_rect,pfather_alpha_scale,&hmi_scale_rect);
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
										&hmi_scale_rect,
										&(new_cliped_farther_rect ));
			}
			else
			{
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcliped_farther_rect,
							&hmi_screen_rect,
							&(new_cliped_farther_rect ));
			}
			hmi_get_intersec_rect_depth((HMI_RECT_STR CONST *)(&(new_cliped_farther_rect)),
								depth,(HMI_RECT_STR  *)pdirty_rect,&hmi_clip_rect);					
			#endif

			#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
			if((hmi_clip_rect.x!=HMI_INVALID_COOR)||
				(hmi_clip_rect.y!=HMI_INVALID_COOR))
			{
			hmi_engine_draw_container(&hmi_sxy_container_table[hmi_object_id ],
					&(hmi_screen_rect),	
					pdirty_rect,
					depth,&new_cliped_farther_rect/*all father insection zone*/
					,pfather_alpha_scale);
			}
			#endif

				#ifdef HMI_GRAPHIC_ST7513
				hmi_engine_draw_container(&hmi_sxy_container_table[hmi_object_id ],
						&(hmi_screen_rect),	
						pdirty_rect,
						depth,&new_cliped_farther_rect/*all father insection zone*/);
				#endif
				
				#ifdef HMI_GRAPHIC_TWLIB /*tw*/
				bMerge=hmi_get_is_merge(parent_object_id_type,HMI_ELEM_TYPE_SXY_CONTAINER);
				#if HMI_RENDER_ALL_EXCEPT_BCK==NO
				//if(((!only_draw_flag)||!hmi_is_render_mode())||/*simulate*/
									//(only_draw_flag&&(flag)))
				#endif
				{
					if(hmi_sxy_container_table[hmi_object_id].container_object_table.object_number>0)
					{
						hmi_engine_draw_container(&hmi_sxy_container_table[hmi_object_id],
													(HMI_RECT_STR CONST *)(&(hmi_screen_rect)),
													HMI_ELEM_TYPE_SXY_CONTAINER,
													hmi_object_id_const,
													!bMerge
													#if HMI_RENDER_ALL_EXCEPT_BCK==NO
														,phmi_object_prop_table->object_id 
														,only_draw_flag&&(!flag)
													#endif
													); 
					}
				}
				#endif
				
				#if (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
				hmi_engine_draw_container(&hmi_sxy_container_table[hmi_object_id],
						&(hmi_screen_rect),
						child_draw_flag
						);
				#endif
				#endif
			}
			else
			#endif	  
		{
			;
		}	   	   
		}	   	   
	}
}
#if 0
HMI_HEIGHT_STR hmi_get_layer_height(UINT8 layer)
{
	#if HMI_ALL_LAYERS_NUMBER >0U
	return(hmi_layer_table[layer].h);
	#else
	return (0);
	#endif
}
HMI_WIDTH_STR hmi_get_layer_width(UINT8 layer)
{
	#if HMI_ALL_LAYERS_NUMBER >0U
	return(hmi_layer_table[layer].w);
	#else
	return (0);
	#endif
}
#endif


#ifdef HMI_TOUCH_PANEL
#if HMI_PAGES_NUMBER>0U
HMI_OBJECT_ID_STR hmi_search_page(POINT_TP *phmi_point,BOOLEAN *phmi_is_button)
{

	//UINT8						loop=0U;lq
	BOOLEAN						stop_search=FALSE;
	HMI_OBJECT_ID_STR 			press_button_id=HMI_ALL_OBJECT;
	#if HMI_LAYER_0_HIGHEST_PRIORITY> 1U
	UINT8						hmi_highest_active_priority_U8=HMI_LAYER_0_HIGHEST_PRIORITY;
	#endif
	HMI_PAGE_ID_STR				hmi_page_id=0U;
	
	HMI_RECT_STR 				hmi_screen_rect={0,0,HMI_MAX_WIDTH,HMI_MAX_HEIGHT};

	#if HMI_LAYER_0_HIGHEST_PRIORITY > 1
	{
		hmi_highest_active_priority_U8 = hmi_highest_active_page_priority[0];
	}
	if(hmi_highest_active_priority_U8 < HMI_LAYER_0_HIGHEST_PRIORITY)
	#endif
	{
		#if HMI_LAYER_0_HIGHEST_PRIORITY > 1   
		while((hmi_highest_active_priority_U8 >=0)&&(stop_search==FALSE))
		#endif
		{
			#if HMI_LAYER_0_HIGHEST_PRIORITY > 1
			hmi_page_id = hmi_layer_0_active_page_id[hmi_highest_active_priority_U8].new_page;
			#else
			hmi_page_id = hmi_layer_0_active_page_id[0].new_page;
			#endif
			#if HMI_DXY_PAGES_NUMBER>0U
			if(HMI_IS_DXY_PAGE(hmi_page_id)/*hmi_page_id<HMI_DXY_PAGES_NUMBER*/)
			{				
				hmi_page_id=HMI_GET_PAGE_DXY_ID_INDEX(hmi_page_id);
				HMI_GET_OBJECT_SCREEN_COOR((&hmi_screen_rect),
											(&hmi_dxy_page_rect[hmi_page_id]),
											(&hmi_screen_rect));
				press_button_id=hmi_search_container(&(hmi_dxy_page_table[hmi_page_id].container),
													(HMI_RECT_STR CONST *)(&hmi_screen_rect),
													phmi_point,
													phmi_is_button);
			}
			else
			#endif
			#if HMI_SXY_PAGES_NUMBER>0
			{
				if(HMI_IS_SXY_PAGE(hmi_page_id)/*hmi_page_id<HMI_PAGE_SXY_MAX_ID*/)
				{
					hmi_page_id=HMI_GET_PAGE_SXY_ID_INDEX(hmi_page_id);
					HMI_GET_OBJECT_SCREEN_COOR((&hmi_screen_rect),
												(&hmi_sxy_page_rect[hmi_page_id]),
												(&hmi_screen_rect));
					press_button_id=hmi_search_container(&(hmi_sxy_page_table[hmi_page_id].container),
														(HMI_RECT_STR CONST *)(&hmi_screen_rect),
														phmi_point,
														phmi_is_button);
				}
			}
			#endif
			{
			}
			
			if(press_button_id!=HMI_ALL_OBJECT)
			{
				stop_search=TRUE;
			}
			#if HMI_LAYER_0_HIGHEST_PRIORITY > 1
			hmi_highest_active_priority_U8--;
			#endif
		}
	}
	return press_button_id;
}
#endif
#endif

#ifdef HMI_TOUCH_PANEL
HMI_OBJECT_ID_STR  hmi_search_container(HMI_CONTAINER_STR CONST * phmi_container_info,HMI_RECT_STR CONST *phmi_father_rect,POINT_TP *phmi_point,BOOLEAN *phmi_is_button) REENTRANT
{
	UINT8						hmi_number_object = phmi_container_info->container_object_table.object_number;
	HMI_OBJECT_PROP_STR CONST * phmi_container_object_table= phmi_container_info->container_object_table.p_object_table;
	BOOLEAN						success			= FALSE;
	HMI_OBJECT_ID_STR			object_id		= HMI_ALL_OBJECT;
	POINT_TP					origin_hmi_point	= {0,0};

	if(phmi_point != NULL)	/*2022 06 13*/
	{
		origin_hmi_point.x	= phmi_point->x;
		origin_hmi_point.y	= phmi_point->y;
	}
	if(hmi_judge_point_zone(phmi_father_rect,
					&origin_hmi_point/*phmi_point 2022 07 27*/))
	{
	    while((hmi_number_object > 0u)&&(success == FALSE))
	  	{
			hmi_number_object--;
			object_id	= hmi_search_object(&phmi_container_object_table[hmi_number_object],
										phmi_father_rect,
										//phmi_point,
										&origin_hmi_point,/*2022 06 13*/
										phmi_is_button);
			if(object_id != HMI_ALL_OBJECT)
			{
			#if HMI_TOUCH_EXCLUDE_TEXT	== YES
				if(HMI_IS_DYN_TEXTS(object_id)||
					HMI_IS_STATIC_TEXTS(object_id))
				{
					success				= FALSE;
					origin_hmi_point.x	= phmi_point->x;/*2022 06 13*/
					origin_hmi_point.y	= phmi_point->y;
				}
				else
				{
					success	= TRUE;
					phmi_point->x	= origin_hmi_point.x;
					phmi_point->y	= origin_hmi_point.y;
				}
			#else
				success			= TRUE;
				phmi_point->x	= origin_hmi_point.x;
				phmi_point->y	= origin_hmi_point.y;
			#endif
			}
			else
			{
				success	= FALSE;
				origin_hmi_point.x	= phmi_point->x;/*2022 06 13*/
				origin_hmi_point.y	= phmi_point->y;
			}
			
		#if 0
		 success=hmi_judge_point_zone(&object_rect, press_point);
		 if(success)
		 {
			object_id=phmi_container_object_table[hmi_number_object-1].object_id;
		 }
		 else
		 {
	     hmi_number_object--;
		 }
		#endif
	  	}
	}
	return object_id;
}
#endif

#ifdef HMI_TOUCH_PANEL
BOOLEAN hmi_judge_point_zone(HMI_RECT_STR CONST *rect_t,POINT_TP *point_t)
{
	BOOLEAN ret=FALSE;
	if((point_t->x>=rect_t->x)&&(point_t->x<(rect_t->x+rect_t->w)))
	{
		if((point_t->y>=rect_t->y)&&(point_t->y<(rect_t->y+rect_t->h)))
		{
			ret=TRUE;
		}
	}
	
	return ret;
}
#endif




#ifdef HMI_TOUCH_PANEL
HMI_OBJECT_ID_STR  hmi_search_object(HMI_OBJECT_PROP_STR CONST * phmi_object_prop_table,HMI_RECT_STR CONST* phmi_father_rect,POINT_TP *phmi_point,BOOLEAN *phmi_is_button) REENTRANT
{
	HMI_OBJECT_ID_STR hmi_object_id= phmi_object_prop_table->object_id;
	HMI_OBJECT_ID_STR hmi_object_id_index=0U;
	HMI_RECT_STR hmi_temp_rect={0,0,0,0};
	HMI_OBJECT_ID_STR hmi_id_index=HMI_ALL_OBJECT;
	HMI_OBJECT_ID_STR press_button_id=HMI_ALL_OBJECT;
	#if HMI_DXY_CUSTOM_CNT +HMI_SXY_CUSTOM_CNT > 0U
	HMI_CUSTOM_PROP_STR	custom_object_prop  ={0U};
	BOOLEAN				get_success			=FALSE;
	HMI_RECT_STR		hmi_custom_rect		={0U};
	#endif
	#if HMI_DXY_BUTTON_NUMBER + HMI_SXY_BUTTON_NUMBER >0
	INT32				btn_index			= 0;// 2022 10 29
	HMI_RANGE_STR		btn_status			= 0;// 2022 10 29
	#endif
	
	if(hmi_object_id >= HMI_PAGE_SXY_MAX_ID)
	{
		#if HMI_DXY_IMAGELIST_NUMBER>0
		if(HMI_IS_DYN_IMAGELIST_DXY(hmi_object_id)/*hmi_object_id  < HMI_DYN_IMAGELIST_DXY_MAX_ID*/)
		{
			hmi_object_id_index = HMI_GET_DYN_IMAGELIST_DXY_ID_INDEX(hmi_object_id);
			HMI_GET_OBJECT_SCREEN_COOR(((HMI_RECT_STR CONST *)phmi_father_rect),
										(&hmi_dxy_imagelist_rect[hmi_object_id_index]),
										(&hmi_temp_rect));
			if(hmi_judge_point_zone(&hmi_temp_rect,phmi_point))
			{
				#if HMI_DXY_IMAGE_LIST_MAX_SON_CNT >0
				press_button_id=hmi_search_container(&hmi_dxy_imagelist_container_table[hmi_object_id_index],
													&hmi_temp_rect,phmi_point,
													phmi_is_button);
				#endif
				if(press_button_id==HMI_ALL_OBJECT)
				{
					press_button_id=hmi_object_id;
					phmi_point->x =phmi_point->x - hmi_temp_rect.x;
					phmi_point->y =phmi_point->y - hmi_temp_rect.y;
					
				}
			}
		}
		else
		#endif
		#if HMI_SXY_IMAGELIST_NUMBER>0
		if(HMI_IS_DYN_IMAGELIST_SXY(hmi_object_id)/*hmi_object_id  < HMI_DYN_IMAGELIST_SXY_MAX_ID*/)
		{
			hmi_object_id_index = HMI_GET_DYN_IMAGELIST_SXY_ID_INDEX(hmi_object_id);
			HMI_GET_OBJECT_SCREEN_COOR(((HMI_RECT_STR CONST *)phmi_father_rect)
				,(&hmi_sxy_imagelist_rect[hmi_object_id_index])
				,(&hmi_temp_rect));
			if(hmi_judge_point_zone(&hmi_temp_rect,phmi_point))
			{
				#if HMI_SXY_IMAGE_LIST_MAX_SON_CNT >0
				press_button_id=hmi_search_container(&hmi_sxy_imagelist_container_table[hmi_object_id_index],
														&hmi_temp_rect,phmi_point,
														phmi_is_button);
				#endif
				if(press_button_id==HMI_ALL_OBJECT)
				{
					press_button_id=hmi_object_id;
					
					phmi_point->x =phmi_point->x - hmi_temp_rect.x;
					phmi_point->y =phmi_point->y - hmi_temp_rect.y;
				}
			}

		}	
		else
		#endif
		#if HMI_DXY_SCROLLBAR_NUMBER>0
		if(HMI_IS_DYN_SCROLLBAR_DXY(hmi_object_id)/*hmi_object_id  < HMI_DYN_SCROLLBAR_DXY_MAX_ID*/)
		{
			hmi_object_id_index = HMI_GET_DYN_SCROLLBAR_DXY_ID_INDEX(hmi_object_id);
			HMI_GET_OBJECT_SCREEN_COOR(((HMI_RECT_STR CONST *)phmi_father_rect),(&hmi_dxy_scrollbar_rect[hmi_object_id_index]),(&hmi_temp_rect));
			if(hmi_judge_point_zone(&hmi_temp_rect,phmi_point))
			{
				#if HMI_DXY_SCROLLBAR_MAX_SON_CNT >0
				press_button_id=hmi_search_container(&hmi_dxy_scrollbar_container_table[hmi_object_id_index],
													&hmi_temp_rect,phmi_point,
													phmi_is_button);
				#endif
				if(press_button_id==HMI_ALL_OBJECT)
				{
					press_button_id=hmi_object_id;
					
					phmi_point->x =phmi_point->x - hmi_temp_rect.x;
					phmi_point->y =phmi_point->y - hmi_temp_rect.y;
				}
			}

		}	
		else
		#endif
		#if HMI_SXY_SCROLLBAR_NUMBER>0
		if(HMI_IS_DYN_SCROLLBAR_SXY(hmi_object_id)/*hmi_object_id  < HMI_DYN_SCROLLBAR_SXY_MAX_ID*/)
		{
			hmi_object_id_index = HMI_GET_DYN_SCROLLBAR_SXY_ID_INDEX(hmi_object_id);
			HMI_GET_OBJECT_SCREEN_COOR(((HMI_RECT_STR CONST *)phmi_father_rect),(&hmi_sxy_scrollbar_rect[hmi_object_id_index]),(&hmi_temp_rect));
			if(hmi_judge_point_zone(&hmi_temp_rect,phmi_point))
			{
				#if HMI_SXY_SCROLLBAR_MAX_SON_CNT >0
				press_button_id=hmi_search_container(&hmi_sxy_scrollbar_container_table[hmi_object_id_index],
													&hmi_temp_rect,phmi_point,
													phmi_is_button);
				#endif
				if(press_button_id==HMI_ALL_OBJECT)
				{
					press_button_id=hmi_object_id;
					
					phmi_point->x =phmi_point->x - hmi_temp_rect.x;
					phmi_point->y =phmi_point->y - hmi_temp_rect.y;
				}
			}

		}	
		else
		#endif
		#if HMI_DXY_BUTTON_NUMBER>0		
		if(HMI_IS_DYN_BUTTON_DXY(hmi_object_id)/*hmi_object_id  < HMI_DYN_BUTTON_DXY_MAX_ID*/)
		{
			hmi_object_id_index	= HMI_GET_DYN_BUTTON_DXY_ID_INDEX(hmi_object_id);	// 2022 10 29		
			btn_index			= (hmi_object_id_index*HMI_BTN_MAX_STATUS_BIT_LEN);								
			btn_status			= hmi_dxy_button_press_status[btn_index >> 3];
			btn_index			= btn_index&0x07/*%8*/;
			btn_status			= (HMI_RANGE_STR)((btn_status >> btn_index)&(1u << btn_index));
					
			if(btn_status!= ((HMI_RANGE_STR)HMI_BUTTON_DISABLE_INDEX))
			{
				HMI_GET_OBJECT_SCREEN_COOR(((HMI_RECT_STR CONST *)phmi_father_rect),(&hmi_dxy_button_rect[hmi_object_id_index]),(&hmi_temp_rect));						
				if(hmi_judge_point_zone(&hmi_temp_rect,phmi_point))
				{
					*phmi_is_button=TRUE;
					#if HMI_DXY_BUTTON_MAX_SON_CNT >0
					press_button_id=hmi_search_container(&hmi_dxy_button_container_table[hmi_object_id_index],
														&hmi_temp_rect,phmi_point,
														phmi_is_button);
					#endif
					if((press_button_id<HMI_DYN_SCROLLBAR_SXY_MAX_ID)||(press_button_id>=HMI_DYN_BUTTON_SXY_MAX_ID))
					{
						press_button_id=hmi_object_id;
						
						phmi_point->x =phmi_point->x - hmi_temp_rect.x;
						phmi_point->y =phmi_point->y - hmi_temp_rect.y;
					}
				}
			}
		}
		else
		#endif
		#if HMI_SXY_BUTTON_NUMBER>0
		if(HMI_IS_DYN_BUTTON_SXY(hmi_object_id)/*hmi_object_id  < HMI_DYN_BUTTON_SXY_MAX_ID*/)
		{
			hmi_object_id_index = HMI_GET_DYN_BUTTON_SXY_ID_INDEX(hmi_object_id);				
			btn_index			= (hmi_object_id_index*HMI_BTN_MAX_STATUS_BIT_LEN);	// 2022 10 29							
			btn_status			= hmi_sxy_button_press_status[btn_index >> 3];
			btn_index			= btn_index&0x07/*%8*/;
			btn_status			= (HMI_RANGE_STR)((btn_status >> btn_index)&(1u << btn_index));
			if((btn_status != ((HMI_RANGE_STR)HMI_BUTTON_DISABLE_INDEX)))	// 2022 10 29	
			{
				hmi_get_object_screen_coor((HMI_RECT_STR CONST *)phmi_father_rect,(HMI_RECT_STR CONST *)(&hmi_sxy_button_rect[hmi_object_id_index]),&hmi_temp_rect);
				if(hmi_judge_point_zone(&hmi_temp_rect,phmi_point))
				{
					*phmi_is_button=TRUE;
					#if HMI_SXY_BUTTON_MAX_SON_CNT >0
					press_button_id=hmi_search_container(&hmi_sxy_button_container_table[hmi_object_id_index],
														&hmi_temp_rect,phmi_point,
														phmi_is_button);
					#endif
					if((press_button_id<HMI_DYN_SCROLLBAR_SXY_MAX_ID)||(press_button_id>=HMI_DYN_BUTTON_SXY_MAX_ID))
					{
						press_button_id=hmi_object_id;
						
						phmi_point->x =phmi_point->x - hmi_temp_rect.x;
						phmi_point->y =phmi_point->y - hmi_temp_rect.y;
					}
				}
			}
		}
		else
		#endif
		#if HMI_DYN_EDIT_TEXTS_NUMBER > 0 
		if(hmi_object_id  < HMI_DYN_TEXTS_MAX_ID)
		{
			hmi_object_id_index  = HMI_GET_DYN_TEXTS_INDEX(hmi_object_id);
			#if HMI_DYN_XY_EDIT_TEXTS_NUMBER > 0
			if(hmi_object_id_index < HMI_DYN_XY_EDIT_TEXTS_NUMBER)
			{
				hmi_get_object_screen_coor((HMI_RECT_STR CONST *)phmi_father_rect,&hmi_dyn_xy_edit_text_prop_table[hmi_object_id_index].text_rect,&hmi_temp_rect);
				
			}
			else
			#endif
			{
				#if HMI_DYN_EDIT_TEXTS_NUMBER-HMI_DYN_XY_EDIT_TEXTS_NUMBER>0
				hmi_get_object_screen_coor((HMI_RECT_STR CONST *)phmi_father_rect,&hmi_static_xy_edit_text_prop_table[hmi_object_id_index -HMI_DYN_XY_EDIT_TEXTS_NUMBER].text_rect,&hmi_temp_rect);
				#endif
				
			}
			if(hmi_judge_point_zone(&hmi_temp_rect,phmi_point))
			{
				#if HMI_EDIT_TEXT_MAX_SON_CNT > 0
				press_button_id=hmi_search_container(&hmi_edit_text_container_table[hmi_object_id_index],
													&hmi_temp_rect,phmi_point,
													phmi_is_button);
				#else
				press_button_id=HMI_ALL_OBJECT;
				#endif
				if(press_button_id==HMI_ALL_OBJECT)
				{
					press_button_id=hmi_object_id;
					
					phmi_point->x =phmi_point->x - hmi_temp_rect.x;
					phmi_point->y =phmi_point->y - hmi_temp_rect.y;
				}
			}
			
		}
		else
		#endif
		#if HMI_DYN_CONTAINERS_NUMBER > 0
		if(HMI_IS_DYN_CONTAINER(hmi_object_id)/*hmi_object_id  < HMI_DYN_CONTAINERS_MAX_ID*/)
		{
			#if HMI_ALL_STATIC_CONTAINERS_NUMBER > 0   
			hmi_object_id  = HMI_GET_DYN_CONTAINERS_ID_INDEX(hmi_object_id);
			hmi_engine_get_static_container_id(&hmi_object_id );
			#if HMI_DXY_CONTAINERS_NUMBER>0U
			if(HMI_IS_DYN_XY_CONTAINER(hmi_object_id)/*(hmi_object_id  >= HMI_DYN_GFILL_MAX_ID) && (hmi_object_id  < HMI_DYN_XY_CONTAINER_MAX_ID)*/)
			{
				hmi_object_id_index = HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_object_id );
				hmi_get_object_screen_coor((HMI_RECT_STR CONST *)phmi_father_rect,&hmi_dyn_xy_container_rect[hmi_object_id_index],&hmi_temp_rect);

				if(hmi_judge_point_zone(&hmi_temp_rect,phmi_point))
				{
					press_button_id=hmi_search_container(&hmi_dyn_xy_container_table[hmi_object_id_index],
													&hmi_temp_rect,phmi_point,
													phmi_is_button);
					if(press_button_id==HMI_ALL_OBJECT)
					{
						press_button_id=hmi_object_id;
						
						phmi_point->x =phmi_point->x - hmi_temp_rect.x;
						phmi_point->y =phmi_point->y - hmi_temp_rect.y;
					}
				}
			}
			#endif
			#if HMI_SXY_CONTAINERS_NUMBER>0
			if(HMI_IS_CONTAINERS_SXY(hmi_object_id)/*(hmi_object_id  >= HMI_BITMAPS_MAX_ID) && (hmi_object_id  < HMI_CONTAINERS_MAX_ID)*/)
			{
				hmi_object_id_index = HMI_GET_CONTAINERS_SXY_ID_INDEX(hmi_object_id );
				hmi_get_object_screen_coor((HMI_RECT_STR CONST *)phmi_father_rect,&hmi_static_container_rect[hmi_object_id_index],&hmi_temp_rect);
				if(hmi_judge_point_zone(&hmi_temp_rect,phmi_point))
				{
					press_button_id=hmi_search_container(&hmi_sxy_container_table[hmi_object_id_index],
												&hmi_temp_rect,phmi_point,
												phmi_is_button);
					if(press_button_id==HMI_ALL_OBJECT)
					{
						press_button_id=hmi_object_id;
						
						phmi_point->x =phmi_point->x - hmi_temp_rect.x;
						phmi_point->y =phmi_point->y - hmi_temp_rect.y;
					}
				}

				
			}
			else
			#endif
			{
			}
			#endif
		}
		else
		#endif
		#if HMI_DYN_FILL_PAGES_NUMBER > 0
		if(HMI_IS_DYN_NFILL(hmi_object_id)/*hmi_object_id  < HMI_DYN_NFILL_MAX_ID*/)
		{
			hmi_object_id_index = HMI_GET_DYN_NFILL_INDEX(hmi_object_id );
			HMI_GET_OBJECT_SCREEN_COOR(phmi_father_rect,
						(&hmi_fills_dyn_xy_rect[hmi_object_id_index])
						,(&hmi_temp_rect));
			if(hmi_judge_point_zone(&hmi_temp_rect,phmi_point))
			{
				#if HMI_DXY_FILL_MAX_SON_CNT >0
				press_button_id=hmi_search_container(&hmi_dyn_fill_container_table[hmi_object_id_index],
													&hmi_temp_rect,phmi_point,
													phmi_is_button);
				#endif
				if(press_button_id==HMI_ALL_OBJECT)
				{
					press_button_id=hmi_object_id;
					
					phmi_point->x =phmi_point->x - hmi_temp_rect.x;
					phmi_point->y =phmi_point->y - hmi_temp_rect.y;
				}
			}
		}
		else
		#endif
		#if HMI_DYN_GFILL_NUMBER > 0
		if(HMI_IS_DYN_GFILL(hmi_object_id)/*hmi_object_id  < HMI_DYN_GFILL_MAX_ID*/)
		{
			hmi_object_id_index = HMI_GET_DYN_GFILL_INDEX(hmi_object_id );
			HMI_GET_OBJECT_SCREEN_COOR(phmi_father_rect,
									(&hmi_gradient_dxy_fill_rect[hmi_object_id_index]),
									(&hmi_temp_rect));
			if(hmi_judge_point_zone(&hmi_temp_rect,phmi_point))
			{
				press_button_id=hmi_search_container(&hmi_dyn_gfill_container_table[hmi_object_id_index],
													&hmi_temp_rect,phmi_point,
													phmi_is_button);
				if(press_button_id==HMI_ALL_OBJECT)
				{
					press_button_id=hmi_object_id;
					
					phmi_point->x =phmi_point->x - hmi_temp_rect.x;
					phmi_point->y =phmi_point->y - hmi_temp_rect.y;
				}
			}
		}
		else
		#endif
		#if HMI_DXY_CUBE_NUMBER > 0
		if(HMI_IS_DYN_CUBE(hmi_object_id)/*hmi_object_id  < HMI_DYN_GFILL_MAX_ID*/)
		{
			hmi_object_id_index = HMI_GET_DYN_CUBE_INDEX(hmi_object_id );
			hmi_get_object_screen_coor((HMI_RECT_STR CONST *)phmi_father_rect,&hmi_cubes_dyn_xy_rect[hmi_object_id_index].cube_rect,&hmi_temp_rect);
			if(hmi_judge_point_zone(&hmi_temp_rect,phmi_point))
			{
				press_button_id=hmi_object_id;
				
				phmi_point->x =phmi_point->x - hmi_temp_rect.x;
				phmi_point->y =phmi_point->y - hmi_temp_rect.y;

			}
		}
		else
		#endif
		#if HMI_DXY_3DCUBE_NUMBER > 0
		if(HMI_IS_DYN_3DCUBE(hmi_object_id)/*hmi_object_id  < HMI_DYN_GFILL_MAX_ID*/)
		{
			hmi_object_id_index = HMI_GET_DYN_3DCUBE_INDEX(hmi_object_id );
			hmi_get_object_screen_coor((HMI_RECT_STR CONST *)phmi_father_rect,&hmi_3dcubes_dyn_xy_rect[hmi_object_id_index].cube_rect,&hmi_temp_rect);
			if(hmi_judge_point_zone(&hmi_temp_rect,phmi_point))
			{
				press_button_id=hmi_object_id;
				
				phmi_point->x =phmi_point->x - hmi_temp_rect.x;
				phmi_point->y =phmi_point->y - hmi_temp_rect.y;

			}
		}
		else
		#endif
		#if HMI_DXY_CONTAINERS_NUMBER>0U
		if(HMI_IS_DYN_XY_CONTAINER(hmi_object_id)/*hmi_object_id  < HMI_DYN_XY_CONTAINER_MAX_ID*/)
		{
			hmi_object_id_index = HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_object_id );
			hmi_get_object_screen_coor((HMI_RECT_STR CONST *)phmi_father_rect,&hmi_dyn_xy_container_rect[hmi_object_id_index],&hmi_temp_rect);
			if(hmi_judge_point_zone(&hmi_temp_rect,phmi_point))
			{
				press_button_id=hmi_search_container(&hmi_dyn_xy_container_table[hmi_object_id_index],
												&hmi_temp_rect,phmi_point,
												phmi_is_button);
				if(press_button_id==HMI_ALL_OBJECT)
				{
					press_button_id=hmi_object_id;
					
					phmi_point->x =phmi_point->x - hmi_temp_rect.x;
					phmi_point->y =phmi_point->y - hmi_temp_rect.y;
				}
			}
		}
		else
		#endif
		#if HMI_DXY_BITMAPS_NUMBER> 0
		if(HMI_IS_DYN_XY_BITMAP(hmi_object_id)/*hmi_object_id < HMI_DYN_XY_BITMAP_MAX_ID*/)
		{
			hmi_object_id_index = HMI_GET_DYN_XY_BITMAP_INDEX(hmi_object_id);
			if(hmi_object_id_index < HMI_DXY_BITMAPS_NUMBER)
			{
				HMI_GET_OBJECT_SCREEN_COOR(phmi_father_rect,
									(&hmi_bmp_dyn_xy_rect[hmi_object_id_index]),
									(&hmi_temp_rect));
				if(hmi_judge_point_zone(&hmi_temp_rect,phmi_point))
				{
					#if HMI_DXY_IMAGE_MAX_SON_CNT >0
					press_button_id=hmi_search_container(&hmi_dxy_bitmap_container_table[hmi_object_id_index],
														&hmi_temp_rect,phmi_point,
														phmi_is_button);
					#endif
					if(press_button_id==HMI_ALL_OBJECT)
					{
						press_button_id=hmi_object_id;
						
						phmi_point->x =phmi_point->x - hmi_temp_rect.x;
						phmi_point->y =phmi_point->y - hmi_temp_rect.y;
					}
				}
			}

		}
		else
		#endif
		#if HMI_DXY_SPLINE_NUMBER > 0 
		if(HMI_IS_DXY_SPLINE(hmi_object_id))
		{	 
			hmi_object_id_index = HMI_GET_DXY_SPLINE_INDEX(hmi_object_id);
			if(hmi_object_id_index < HMI_DXY_SPLINE_NUMBER)
			{
				HMI_GET_OBJECT_SCREEN_COOR(phmi_father_rect,
									(&hmi_dxy_spline_rect[hmi_object_id_index]),
									(&hmi_temp_rect));
				if(hmi_judge_point_zone(&hmi_temp_rect,phmi_point))
				{
					#if HMI_DXY_SPLINE_MAX_SON_CNT >0
					press_button_id=hmi_search_container(&hmi_dyn_xy_spline_table[hmi_object_id_index],
														&hmi_temp_rect,phmi_point,
														phmi_is_button);
					#endif
					if(press_button_id==HMI_ALL_OBJECT)
					{
						press_button_id=hmi_object_id;
						
						phmi_point->x =phmi_point->x - hmi_temp_rect.x;
						phmi_point->y =phmi_point->y - hmi_temp_rect.y;
					}
				}
			}
		}
		else
		#endif
		#if HMI_SXY_SPLINE_NUMBER > 0 
		if(HMI_IS_SXY_SPLINE(hmi_object_id))
		{	 
			hmi_object_id_index = HMI_GET_SXY_SPLINE_INDEX(hmi_object_id );
			if(hmi_object_id_index < HMI_SXY_SPLINE_NUMBER)
			{
				HMI_GET_OBJECT_SCREEN_COOR(phmi_father_rect,
									(&hmi_sxy_spline_rect[hmi_object_id_index]),
									(&hmi_temp_rect));
				if(hmi_judge_point_zone(&hmi_temp_rect,phmi_point))
				{
					#if HMI_SXY_SPLINE_MAX_SON_CNT >0
					press_button_id=hmi_search_container(&hmi_sxy_spline_table[hmi_object_id_index],
														&hmi_temp_rect,phmi_point,
														phmi_is_button);
					#endif
					if(press_button_id==HMI_ALL_OBJECT)
					{
						press_button_id=hmi_object_id;
						
						phmi_point->x =phmi_point->x - hmi_temp_rect.x;
						phmi_point->y =phmi_point->y - hmi_temp_rect.y;
					}
				}
			}
		}
		else
		#endif
		#if HMI_DXY_CUSTOM_CNT > 0
		if(HMI_IS_DYN_XY_CUSTOM(hmi_object_id))
		{
			//hmi_object_id_index = HMI_GET_DXY_CUSTOM_INDEX(hmi_object_id);
			if(hmi_object_id > HMI_SXY_SPLINE_MAX_ID)
			{
				hmi_object_id_index = HMI_GET_DXY_CUSTOM_INDEX(hmi_object_id);
			}
			else
			{
				hmi_object_id_index = 0U;
			}
			if(hmi_object_id_index < HMI_DXY_CUSTOM_CNT)
			{
				get_success = hmi_engine_get_custom_prop(hmi_object_id,&custom_object_prop);
				if(get_success == TRUE)
				{
					hmi_custom_rect.x	= custom_object_prop.x;
					hmi_custom_rect.y	= custom_object_prop.y;
					hmi_custom_rect.w	= custom_object_prop.w;
					hmi_custom_rect.h	= custom_object_prop.h;
				}
				HMI_GET_OBJECT_SCREEN_COOR(phmi_father_rect,
									(&hmi_custom_rect),
									(&hmi_temp_rect));
				if(hmi_judge_point_zone(&hmi_temp_rect,phmi_point))
				{
					press_button_id=hmi_object_id;
				
				phmi_point->x =phmi_point->x - hmi_temp_rect.x;
				phmi_point->y =phmi_point->y - hmi_temp_rect.y;

				}
			}
		}
		else
		#endif
		#if HMI_STATIC_FILL_PAGES_NUMBER > 0
		if(HMI_IS_SXY_FILL_PAGES(hmi_object_id)/*hmi_object_id  < HMI_FILL_PAGES_MAX_ID*/)
		{
			hmi_object_id_index= HMI_GET_SXY_FILL_PAGES_ID_INDEX(hmi_object_id );
			HMI_GET_OBJECT_SCREEN_COOR(phmi_father_rect,
								(&hmi_fills_static_xy_rect[hmi_object_id_index])
								,(&hmi_temp_rect));
			if(hmi_judge_point_zone(&hmi_temp_rect,phmi_point))
			{
				#if HMI_SXY_FILL_MAX_SON_CNT >0
				press_button_id=hmi_search_container(&hmi_static_fill_container_table[hmi_object_id_index],&hmi_temp_rect,phmi_point,phmi_is_button);
				#endif
				if(press_button_id==HMI_ALL_OBJECT)
				{
					press_button_id=hmi_object_id;
					
					phmi_point->x =phmi_point->x - hmi_temp_rect.x;
					phmi_point->y =phmi_point->y - hmi_temp_rect.y;
				}
			}

		}
		else
		#endif
		#if HMI_STATIC_GFILL_NUMBER> 0
		if(HMI_IS_SXY_GFILL_PAGES(hmi_object_id)/*hmi_object_id  < HMI_GFILL_PAGES_MAX_ID*/)
		{
			hmi_object_id_index  = HMI_GET_SXY_GFILL_PAGES_ID_INDEX(hmi_object_id);
			HMI_GET_OBJECT_SCREEN_COOR(phmi_father_rect,
										(&hmi_gradient_sxy_fill_rect[hmi_object_id_index]),
										(&hmi_temp_rect));
			if(hmi_judge_point_zone(&hmi_temp_rect,phmi_point))
			{
				press_button_id=hmi_search_container(&hmi_static_gfill_container_table[hmi_object_id_index],
												&hmi_temp_rect,phmi_point,
												phmi_is_button);
				if(press_button_id==HMI_ALL_OBJECT)
				{
					press_button_id=hmi_object_id;
					
					phmi_point->x =phmi_point->x - hmi_temp_rect.x;
					phmi_point->y =phmi_point->y - hmi_temp_rect.y;
				}
			}

		}
		else
		#endif
		#if HMI_SXY_CUBE_NUMBER > 0
		if(HMI_IS_SXY_CUBE(hmi_object_id)/*hmi_object_id  < HMI_DYN_GFILL_MAX_ID*/)
		{
			hmi_object_id_index = HMI_GET_SXY_CUBE_ID_INDEX(hmi_object_id );
			hmi_get_object_screen_coor((HMI_RECT_STR CONST *)phmi_father_rect,&hmi_cubes_static_xy_rect[hmi_object_id_index].cube_rect,&hmi_temp_rect);
			if(hmi_judge_point_zone(&hmi_temp_rect,phmi_point))
			{
				press_button_id=hmi_object_id;
				
				phmi_point->x =phmi_point->x - hmi_temp_rect.x;
				phmi_point->y =phmi_point->y - hmi_temp_rect.y;

			}
		}
		else
		#endif
		#if HMI_SXY_3DCUBE_NUMBER > 0
		if(HMI_IS_SXY_3DCUBE(hmi_object_id)/*hmi_object_id  < HMI_DYN_GFILL_MAX_ID*/)
		{
			hmi_object_id_index = HMI_GET_SXY_3DCUBE_ID_INDEX(hmi_object_id );
			hmi_get_object_screen_coor((HMI_RECT_STR CONST *)phmi_father_rect,&hmi_3dcubes_static_xy_rect[hmi_object_id_index].cube_rect,&hmi_temp_rect);
			if(hmi_judge_point_zone(&hmi_temp_rect,phmi_point))
			{
				press_button_id=hmi_object_id;
				
				phmi_point->x =phmi_point->x - hmi_temp_rect.x;
				phmi_point->y =phmi_point->y - hmi_temp_rect.y;

			}
		}
		else
		#endif
		#if HMI_SXY_BITMAPS_NUMBER> 0
		if(HMI_IS_S_XY_BITMAP(hmi_object_id)/*hmi_object_id  < HMI_BITMAPS_MAX_ID*/)
		{
			hmi_object_id_index = HMI_GET_SXY_BITMAPS_ID_INDEX(hmi_object_id);
			if(hmi_object_id_index < HMI_SXY_BITMAPS_NUMBER)
			{
				HMI_GET_OBJECT_SCREEN_COOR(phmi_father_rect,
										(&hmi_bmp_static_xy_rect[hmi_object_id_index]),
										(&hmi_temp_rect));
				if(hmi_judge_point_zone(&hmi_temp_rect,phmi_point))
				{
					#if HMI_SXY_IMAGE_MAX_SON_CNT >0
					press_button_id=hmi_search_container(&hmi_sxy_bitmap_container_table[hmi_object_id_index],
														&hmi_temp_rect,phmi_point,
														phmi_is_button);
					#endif
					if(press_button_id==HMI_ALL_OBJECT)
					{
						press_button_id=hmi_object_id;
				
				phmi_point->x =phmi_point->x - hmi_temp_rect.x;
				phmi_point->y =phmi_point->y - hmi_temp_rect.y;

					}
				}
			}
			
		}
		else
		#endif
		#if HMI_SXY_CUSTOM_CNT > 0
		if(HMI_IS_CUSTOM_SXY(hmi_object_id))
		{
			hmi_object_id_index = HMI_GET_CUSTOM_SXY_ID_INDEX(hmi_object_id);
			if(hmi_object_id_index < HMI_SXY_CUSTOM_CNT)
			{
				get_success = hmi_engine_get_custom_prop(hmi_object_id,&custom_object_prop);
				if(get_success == TRUE)
				{
					hmi_custom_rect.x	= custom_object_prop.x;
					hmi_custom_rect.y	= custom_object_prop.y;
					hmi_custom_rect.w	= custom_object_prop.w;
					hmi_custom_rect.h	= custom_object_prop.h;
				}
				HMI_GET_OBJECT_SCREEN_COOR(phmi_father_rect,
									(&hmi_custom_rect),
									(&hmi_temp_rect));
				if(hmi_judge_point_zone(&hmi_temp_rect,phmi_point))
				{
					press_button_id=hmi_object_id;
				
				phmi_point->x =phmi_point->x - hmi_temp_rect.x;
				phmi_point->y =phmi_point->y - hmi_temp_rect.y;

				}
			}
		}
		else
		#endif	
		#if HMI_SXY_CONTAINERS_NUMBER > 0
		if(HMI_IS_CONTAINERS_SXY(hmi_object_id)/*hmi_object_id  < HMI_CONTAINERS_MAX_ID*/)
		{
			#if HMI_SXY_CONTAINERS_NUMBER >0
			hmi_object_id_index = HMI_GET_CONTAINERS_SXY_ID_INDEX(hmi_object_id );
			hmi_get_object_screen_coor((HMI_RECT_STR CONST *)phmi_father_rect,&hmi_static_container_rect[hmi_object_id_index],&hmi_temp_rect);
			if(hmi_judge_point_zone(&hmi_temp_rect,phmi_point))
			{
				press_button_id=hmi_search_container(&hmi_sxy_container_table[hmi_object_id_index],
												&hmi_temp_rect,phmi_point,
												phmi_is_button);
				if(press_button_id==HMI_ALL_OBJECT)
				{
					press_button_id=hmi_object_id;
					
					phmi_point->x =phmi_point->x - hmi_temp_rect.x;
					phmi_point->y =phmi_point->y - hmi_temp_rect.y;
				}
			}
			#endif
		}
		else
		#endif
		#if 0/*HMI_STATIC_TEXTS_NUMBER > 0 */ //test ??
		if(hmi_object_id < HMI_STATIC_TEXTS_MAX_ID)
		{
			hmi_object_id_index  = HMI_GET_STATIC_TEXT_ID_INDEX(hmi_object_id );
			hmi_get_object_screen_coor((HMI_RECT_STR CONST *)phmi_father_rect,
								&(hmi_static_xy_static_text_prop_table[hmi_object_id_index].text_rect),
								&hmi_temp_rect);
			if(hmi_judge_point_zone(&hmi_temp_rect,phmi_point))
			{
				#if HMI_UNEDIT_TEXT_MAX_SON_CNT > 0
				press_button_id=hmi_search_container(&hmi_static_text_container_table[hmi_object_id_index],&hmi_temp_rect,phmi_point,phmi_is_button);
				#else
				press_button_id=HMI_ALL_OBJECT;
				#endif
				if(press_button_id==HMI_ALL_OBJECT)
				{
					press_button_id=hmi_object_id;
				}
			}

		}
		else
		#endif
		#if HMI_STATIC_TEXTS_NUMBER > 0 
		if(HMI_IS_STATIC_TEXTS(hmi_object_id))
		{			
			if(HMI_IS_UNEDIT_TEXT_DYN_XY(hmi_object_id))
			{
				hmi_object_id_index  = HMI_GET_DYN_XY_UNEDIT_POS_COLOR_INDEX(hmi_object_id);
				#if HMI_UNEDIT_TEXTS_DYN_XY_NUMBER > 0
				if(hmi_object_id_index < HMI_UNEDIT_TEXTS_DYN_XY_NUMBER)
				{
					hmi_get_object_screen_coor((HMI_RECT_STR CONST *)phmi_father_rect,
									&hmi_dyn_xy_unedit_text_prop_table[hmi_object_id_index].text_rect,
									&hmi_temp_rect);
					hmi_object_id_index=HMI_GET_STATIC_TEXT_ID_INDEX(hmi_object_id);/*sxy,dxy uneditable container save at one array*/
					if(hmi_judge_point_zone(&hmi_temp_rect,phmi_point))
					{
						if(hmi_object_id_index<HMI_STATIC_TEXTS_NUMBER)
						{
							#if HMI_UNEDIT_TEXT_MAX_SON_CNT > 0
							press_button_id=hmi_search_container(&hmi_static_text_container_table[hmi_object_id_index],
															&hmi_temp_rect,phmi_point,phmi_is_button);
							#else
							press_button_id=HMI_ALL_OBJECT;
							#endif
							if(press_button_id==HMI_ALL_OBJECT)
							{
								press_button_id=hmi_object_id;
								
								phmi_point->x =phmi_point->x - hmi_temp_rect.x;
								phmi_point->y =phmi_point->y - hmi_temp_rect.y;
							}
						}
					}					
				}				
				#endif
			}
			else if(HMI_IS_UNEDIT_TEXT_STATIC_XY(hmi_object_id))
			{
				hmi_object_id_index  = HMI_GET_S_XY_UNEDIT_POS_COLOR_INDEX(hmi_object_id);
				#if HMI_UNEDIT_TEXTS_STATIC_XY_NUMBER > 0
				if(hmi_object_id_index < HMI_UNEDIT_TEXTS_STATIC_XY_NUMBER)
				{
					hmi_get_object_screen_coor((HMI_RECT_STR CONST *)phmi_father_rect,
										&hmi_static_xy_unedit_text_prop_table[hmi_object_id_index].text_rect,
										&hmi_temp_rect);
					hmi_object_id_index=HMI_GET_STATIC_TEXT_ID_INDEX(hmi_object_id);
					if(hmi_judge_point_zone(&hmi_temp_rect,phmi_point))
					{
						if(hmi_object_id_index<HMI_STATIC_TEXTS_NUMBER)
						{
							#if HMI_UNEDIT_TEXT_MAX_SON_CNT > 0
							press_button_id=hmi_search_container(&hmi_static_text_container_table[hmi_object_id_index],
																&hmi_temp_rect,phmi_point,phmi_is_button);
							#else
							press_button_id=HMI_ALL_OBJECT;
							#endif
							if(press_button_id==HMI_ALL_OBJECT)
							{
								press_button_id=hmi_object_id;
								
								phmi_point->x =phmi_point->x - hmi_temp_rect.x;
								phmi_point->y =phmi_point->y - hmi_temp_rect.y;
							}
						}
					}
					
				}				
				#endif
			}
			else{}
			
			
		}
		else
		#endif
		{
		}
		
	}
   return (press_button_id);
 }
#endif

#if  0
void hmi_get_button_id(POINT_TP *pPoint,UINT8 press_status)
{
	HMI_OBJECT_ID_STR button_id=HMI_ALL_OBJECT;
	#if HMI_PAGES_NUMBER >0U
	button_id=hmi_search_page(pPoint);
	#endif
	if(button_id !=HMI_ALL_OBJECT)
	{	
		hmi_engine_set_object_info(button_id,press_status);
		#if HMI_EVENT_STAND_NUMBER>0
		hmi_do_event(button_id,press_status);
		#endif
	}
}
#endif


/*
get element property id
*/
#if HMI_SET_ACTION_NUMBER+HMI_DXY_CUSTOM_CNT+HMI_SXY_CUSTOM_CNT>0 /*Set action number>0*/
HMI_OBJECT_ID_STR hmi_get_obj_pro_id(HMI_OBJECT_ID_STR hmi_object_id,UINT8 pro_order)
{
	HMI_OBJECT_ID_STR element_pro_id=HMI_NB_ELEMENTS;

	 if(hmi_object_id < HMI_DYN_ELEMENT_END_ID) /*dyn element*/
	 {
#if (HMI_DYN_CONTAINERS_NUMBER > 0)
		  if(HMI_IS_DYN_CONTAINER(hmi_object_id))
		  {   
			  hmi_object_id  = HMI_GET_DYN_CONTAINERS_ID_INDEX(hmi_object_id);
			  hmi_engine_get_static_container_id(&hmi_object_id);/*get static container ID*/ 
		  }
	#endif
	 	#if HMI_DXY_PAGES_NUMBER>0
	 	if(HMI_IS_DXY_PAGE(hmi_object_id))
	 	{
			hmi_object_id	= HMI_GET_PAGE_DXY_ID_INDEX(hmi_object_id);
			element_pro_id	= (HMI_DYN_TRAIL_BITMAP_MAX_ID+pro_order*HMI_DXY_PAGES_NUMBER);
	 	}
		else
		#endif
		#if HMI_DXY_IMAGELIST_NUMBER>0
		if(HMI_IS_DYN_IMAGELIST_DXY(hmi_object_id))
		{
			hmi_object_id	= HMI_GET_DYN_IMAGELIST_DXY_ID_INDEX(hmi_object_id);
			element_pro_id	=(HMI_DYN_ALPHA_PAGES_MAX_ID+pro_order*HMI_DXY_IMAGELIST_NUMBER);
		}
		else
		#endif
		#if HMI_DXY_BITMAPS_NUMBER>0
		if(HMI_IS_DYN_XY_BITMAP(hmi_object_id))
		{
			hmi_object_id	= HMI_GET_DYN_XY_BITMAP_INDEX(hmi_object_id);
			element_pro_id	=(HMI_DYN_SCALE_CONTAINER_MAX_ID+pro_order*HMI_DXY_BITMAPS_NUMBER);
		}
		else
		#endif
		#if HMI_DXY_SCROLLBAR_NUMBER>0
		if(HMI_IS_DYN_SCROLLBAR_DXY(hmi_object_id))
		{
			hmi_object_id	= HMI_GET_DYN_SCROLLBAR_DXY_ID_INDEX(hmi_object_id);
			element_pro_id	= (HMI_DYN_ALPHA_IMAGELIST_MAX_ID+pro_order*HMI_DXY_SCROLLBAR_NUMBER);
		}
	    else
		#endif
		#if HMI_DXY_BUTTON_NUMBER>0
		if(HMI_IS_DYN_BUTTON_DXY(hmi_object_id))
		{						
			hmi_object_id	= HMI_GET_DYN_BUTTON_DXY_ID_INDEX(hmi_object_id);
			element_pro_id	= (HMI_DYN_ALPHA_SCROLLBAR_MAX_ID+pro_order*HMI_DXY_BUTTON_NUMBER);
		
		}
		else
		#endif
		#if HMI_DYN_XY_EDIT_TEXTS_NUMBER>0
		if(HMI_IS_DYN_TEXT_DXY(hmi_object_id))
		{
			hmi_object_id	= HMI_GET_DYN_TEXTS_DXY_POS_INDEX(hmi_object_id);
			element_pro_id	= (HMI_DYN_COLOR_UNEDIT_MAX_ID+pro_order*HMI_DYN_XY_EDIT_TEXTS_NUMBER);
		}
		else
		#endif
		#if HMI_DYN_FILL_PAGES_NUMBER > 0 /*dxy gfill*/
		if(HMI_IS_DYN_NFILL(hmi_object_id) )
		{
			hmi_object_id	= HMI_GET_DYN_NFILL_INDEX(hmi_object_id);
			element_pro_id	= (HMI_DYN_DCOLOR_DTEXT_MAX_ID+pro_order*HMI_DYN_FILL_PAGES_NUMBER);
		}
		else
		#endif
		#if HMI_DYN_GFILL_NUMBER>0
		if(HMI_IS_DYN_GFILL(hmi_object_id))
		{
			hmi_object_id	= HMI_GET_DYN_GFILL_INDEX(hmi_object_id);
			element_pro_id	= (HMI_DYN_Z_NFILL_MAX_ID+pro_order*HMI_DYN_GFILL_NUMBER);			
		}
		else
   		#endif
		#if HMI_DXY_CUBE_NUMBER>0
		if(HMI_IS_DYN_CUBE(hmi_object_id))
		{
			hmi_object_id	= HMI_GET_DYN_CUBE_INDEX(hmi_object_id);
			element_pro_id	= (HMI_DYN_Z_GFILL_MAX_ID+pro_order*HMI_DXY_CUBE_NUMBER);			
		}
		else
   		#endif
		#if HMI_DXY_3DCUBE_NUMBER>0
		if(HMI_IS_DYN_3DCUBE(hmi_object_id))
		{
			hmi_object_id	= HMI_GET_DYN_3DCUBE_INDEX(hmi_object_id);
			element_pro_id	= (HMI_DYN_SCALE_CUBE_MAX_ID+pro_order*HMI_DXY_3DCUBE_NUMBER);			
		}
		else
   		#endif
		#if HMI_DXY_CONTAINERS_NUMBER>0
		if(HMI_IS_DYN_XY_CONTAINER(hmi_object_id))
		{
			hmi_object_id	= HMI_GET_DYN_XY_CONTAINER_INDEX(hmi_object_id);
			element_pro_id	= (HMI_DYN_ELEMENT_END_ID+pro_order*HMI_DXY_CONTAINERS_NUMBER);	
		}
		else
		#endif
		#if HMI_UNEDIT_TEXTS_DYN_XY_NUMBER>0
		if(HMI_IS_UNEDIT_TEXT_DYN_XY(hmi_object_id))
		{
			hmi_object_id	= HMI_GET_DYN_XY_UNEDIT_POS_COLOR_INDEX(hmi_object_id);
			element_pro_id	= (HMI_DYN_ALPHA_BUTTON_MAX_ID+pro_order*HMI_UNEDIT_TEXTS_DYN_XY_NUMBER);			
		}
		else
		#endif
		#if HMI_DXY_CUSTOM_CNT > 0U
		if(HMI_IS_DYN_XY_CUSTOM(hmi_object_id))
		{
			//hmi_object_id	= HMI_GET_DXY_CUSTOM_INDEX(hmi_object_id);
			if(hmi_object_id > HMI_SXY_SPLINE_MAX_ID)
			{
				hmi_object_id = HMI_GET_DXY_CUSTOM_INDEX(hmi_object_id);
			}
			else
			{
				hmi_object_id = 0U;
			}
			element_pro_id	= (HMI_DYN_SCALE_SPLINE_MAX_ID+pro_order*HMI_DXY_CUSTOM_CNT);			
		}
		else
		#endif
		{
			element_pro_id	=HMI_NB_ELEMENTS;
			hmi_object_id	=0;
		}
	 }
	element_pro_id+=hmi_object_id;
	
	return element_pro_id;
}
#endif

#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC) || defined(HMI_GRAPHIC_VGLITE) ||defined(HMI_GRAPHIC_OPENVG)
void  hmi_get_nextFrame(void)
{
}
#endif


#if ((HMI_DXY_IMAGELIST_NUMBER+HMI_SXY_IMAGELIST_NUMBER+	\
		HMI_DXY_SCROLLBAR_NUMBER+HMI_SXY_SCROLLBAR_NUMBER+	\
		HMI_DXY_BUTTON_NUMBER+HMI_SXY_BUTTON_NUMBER+	\
		HMI_DXY_BITMAPS_NUMBER+HMI_SXY_BITMAPS_NUMBER)>0) 
void call_C_hmi_driver_remove_load_file(HMI_CONTAINER_U16_STR CONST * phmi_container_info)
{
	UINT16 hmi_number_object  = phmi_container_info->container_object_table.object_number;
	HMI_OBJECT_PROP_STR CONST * phmi_container_object_table= phmi_container_info->container_object_table.p_object_table;
	HMI_OBJECT_ID_STR		hmi_object_id	= 0;
	while(hmi_number_object > 0)	
	{
		hmi_object_id=phmi_container_object_table->object_id;
		free_image_res_manager(hmi_object_id);
		hmi_number_object--;	
		phmi_container_object_table++;
	}
}


/*
Release vgliste source buffer
*/
#if	defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)
void call_C_hmi_driver_remove_buf_file(HMI_CONTAINER_U16_STR CONST * phmi_container_info)
{
	UINT16						hmi_number_object  			= phmi_container_info->container_object_table.object_number;
	HMI_OBJECT_PROP_STR CONST * phmi_container_object_table	= phmi_container_info->container_object_table.p_object_table;
	HMI_OBJECT_ID_STR			hmi_object_id				= 0;
	SINT16						buffer_info_index			= VGLITE_BUFFER_END_FLAG;
	rgl_buffer_img_str			*pbuf						= NULL;
	
	while(hmi_number_object > 0u)	
	{
		hmi_object_id		= phmi_container_object_table->object_id;		
		pbuf	= hmi_rgl_get_element_pointer(hmi_object_id);
		if(pbuf != NULL)
		{
			buffer_info_index	= pbuf->vg_buf_id;
		}
		else
		{
			buffer_info_index	= VGLITE_BUFFER_END_FLAG;
		}
	#if	(HMI_LOAD_ONE_FRAME_IMGLIST == YES)
		hmi_release_pixel_buf(buffer_info_index);
	#else
		hmi_release_pixel_buf_multi(buffer_info_index);
	#endif
		hmi_number_object--;	
		phmi_container_object_table++;
	}
}

void hmi_load_file_to_pixel_buf(HMI_OBJECT_ID_STR	hmi_object_id,BOOLEAN brel_buf/*if load failed,release other buffer*/)
{
	HMI_COMPRESS_IMAGE_LIST_STR	*pImage_data			= NULL;
	UINT8						data_len				= 0u;		
	HMI_OBJECT_ID_STR			hmi_object_index		= 0;
	HMI_COMPRESS_IMAGE_LIST_STR	hmi_mul_image			= {0};
	rgl_image_type_str			image_format			= HMI_IMAGE_NO_COMPRESS;
	VGImage						texture					= VG_INVALID_HANDLE;
	vg_lite_buffer_t			*ptexture			= NULL;

	UINT8						bmp_alpha_rotation_flag	= 0u;
	rgl_image_type_str			bmp_compress			= HMI_IMAGE_NO_COMPRESS;
	HMI_WIDTH_STR				img_frame_w				= 0u;// 2021 04 04	
#if (HMI_LOAD_ONE_FRAME_IMGLIST == NO)
	UINT16						i					= 0u;
#endif
	
	if(hmi_object_id  >= HMI_PAGE_SXY_MAX_ID)/*not a page*/
	{				
	#if HMI_DXY_IMAGELIST_NUMBER > 0
		if(HMI_IS_DYN_IMAGELIST_DXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_IMAGELIST_DXY_ID_INDEX(hmi_object_id);												
		
			if(hmi_object_index < HMI_DXY_IMAGELIST_NUMBER)
			{
				image_format			= (rgl_image_type_str)(get_compress_fmt(hmi_dxy_imglist_attr_table[hmi_object_index]));
				pImage_data				= (HMI_COMPRESS_IMAGE_LIST_STR *)(hmi_dxy_imagelist_table[hmi_object_index].file.pimagelist_attr);
				data_len				= hmi_dxy_imagelist_table[hmi_object_index].list_len;
				bmp_alpha_rotation_flag	= (hmi_dxy_imglist_attr_table[hmi_object_index].image_attr)& 0xf0;
				bmp_compress			= get_compress_fmt(hmi_dxy_imglist_attr_table[hmi_object_index]);

				img_frame_w	= hmi_dxy_imagelist_table[hmi_object_index].file.w; // 2021 04 04
				if(data_len != 0u)
				{
					img_frame_w	= (HMI_WIDTH_STR)(hmi_dxy_imagelist_table[hmi_object_index].file.w / data_len);
				}
				
				if(brel_buf == TRUE)
				{
				#if	(HMI_LOAD_ONE_FRAME_IMGLIST == NO)
					for(i = 0u; i < data_len;i++)
				#endif
					{				
						get_texture_res_manager(hmi_object_id,										
												&ptexture,
												(HMI_COMPRESS_IMAGE_LIST_STR *)pImage_data,
												data_len,
												img_frame_w,
												hmi_dxy_imagelist_table[hmi_object_index].file.h,
												bmp_alpha_rotation_flag & HMI_ROTATION_IMAGE_FLAG,										
												bmp_compress,
												0,/*no used*/
											#if	(HMI_LOAD_ONE_FRAME_IMGLIST == NO)
												i,/*imagelist index*/
											#else
												0,
											#endif
												(hmi_dxy_imglist_attr_table[hmi_object_index].pixel_fmt) & 0x1f
												);
					}
				}
				else
				{
				#if	(HMI_LOAD_ONE_FRAME_IMGLIST == NO)
					for(i = 0u; i < data_len;i++)
				#endif
					{
						get_texture_res_manager(hmi_object_id,										
												NULL,/*if load failed,not try,not free other resource*/
												(HMI_COMPRESS_IMAGE_LIST_STR *)pImage_data,
												data_len,
												img_frame_w,//hmi_dxy_imagelist_table[hmi_object_index].file.w,
												hmi_dxy_imagelist_table[hmi_object_index].file.h,
												bmp_alpha_rotation_flag & HMI_ROTATION_IMAGE_FLAG,										
												bmp_compress,
												0,/*no used*/
											#if	(HMI_LOAD_ONE_FRAME_IMGLIST == NO)
												i,/*imagelist index*/
											#else
												0,
											#endif
												(hmi_dxy_imglist_attr_table[hmi_object_index].pixel_fmt) & 0x1f
												);
					}
				}
			}										
		}		
		else
	#endif
	#if HMI_SXY_IMAGELIST_NUMBER>0
		if(HMI_IS_DYN_IMAGELIST_SXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_IMAGELIST_SXY_ID_INDEX(hmi_object_id);			
		
			if(hmi_object_index < HMI_SXY_IMAGELIST_NUMBER)
			{
				image_format= (rgl_image_type_str)(get_compress_fmt(hmi_sxy_imglist_attr_table[hmi_object_index]));
				pImage_data = (HMI_COMPRESS_IMAGE_LIST_STR *)(hmi_sxy_imagelist_table[hmi_object_index].file.pimagelist_attr);
				data_len	= hmi_sxy_imagelist_table[hmi_object_index].list_len;				
				bmp_alpha_rotation_flag	= (hmi_sxy_imglist_attr_table[hmi_object_index].image_attr)& 0xf0;
				bmp_compress			= get_compress_fmt(hmi_sxy_imglist_attr_table[hmi_object_index]);
				
				img_frame_w	= hmi_sxy_imagelist_table[hmi_object_index].file.w; // 2021 04 04
				if(data_len != 0u)
				{
					img_frame_w	= (HMI_WIDTH_STR)(hmi_sxy_imagelist_table[hmi_object_index].file.w / data_len);
				}
				if(brel_buf == TRUE)
				{
				#if	(HMI_LOAD_ONE_FRAME_IMGLIST == NO)
					for(i = 0u; i < data_len;i++)
				#endif
					{
						get_texture_res_manager(hmi_object_id,										
												&ptexture,/*if load failed,not try,not free other resource*/
												(HMI_COMPRESS_IMAGE_LIST_STR *)pImage_data,
												data_len,
												img_frame_w,//hmi_sxy_imagelist_table[hmi_object_index].file.w,
												hmi_sxy_imagelist_table[hmi_object_index].file.h,
												bmp_alpha_rotation_flag & HMI_ROTATION_IMAGE_FLAG,										
												bmp_compress,
												0,/*no used*/
											#if	(HMI_LOAD_ONE_FRAME_IMGLIST == NO)
												i,/*imagelist index*/
											#else
												0,
											#endif												
												(hmi_sxy_imglist_attr_table[hmi_object_index].pixel_fmt)& 0x1f
												);
					}
				}
				else
				{
				#if	(HMI_LOAD_ONE_FRAME_IMGLIST == NO)
					for(i = 0u; i < data_len;i++)
				#endif
					{
						get_texture_res_manager(hmi_object_id,										
											NULL,/*if load failed,not try,not free other resource*/
											(HMI_COMPRESS_IMAGE_LIST_STR *)pImage_data,
											data_len,
											img_frame_w,//hmi_sxy_imagelist_table[hmi_object_index].file.w,
											hmi_sxy_imagelist_table[hmi_object_index].file.h,
											bmp_alpha_rotation_flag & HMI_ROTATION_IMAGE_FLAG,										
											bmp_compress,
											0,/*no used*/
										#if	(HMI_LOAD_ONE_FRAME_IMGLIST == NO)
											i,/*imagelist index*/
										#else
											0,
										#endif
											(hmi_sxy_imglist_attr_table[hmi_object_index].pixel_fmt) & 0x1f
											);
					}
				}
			}																		
		}
		else
	#endif			
	#if HMI_DXY_SCROLLBAR_NUMBER>0
		if(HMI_IS_DYN_SCROLLBAR_DXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_SCROLLBAR_DXY_ID_INDEX(hmi_object_id);									
		#if HMI_SCROLLBAR_MAX_STATUS >= 0														
			if(hmi_object_index < HMI_DXY_SCROLLBAR_NUMBER)
			{
				image_format= (rgl_image_type_str)(get_compress_fmt(hmi_dxy_scrollbar_attr_table[hmi_object_index]));
				hmi_mul_image.pbitmap_data	= (U08 *)(hmi_dxy_scrollbar_table[hmi_object_index].file.pbitmap_data);
				hmi_mul_image.data_len		= hmi_dxy_scrollbar_table[hmi_object_index].file.data_len;				
				bmp_alpha_rotation_flag		= (hmi_dxy_scrollbar_attr_table[hmi_object_index].image_attr) & 0xf0;
				bmp_compress				= get_compress_fmt(hmi_dxy_scrollbar_attr_table[hmi_object_index]);
				data_len					= 1u;
				if(brel_buf == TRUE)
				{
					get_texture_res_manager(hmi_object_id,										
											&ptexture,
											(HMI_COMPRESS_IMAGE_LIST_STR *)(&hmi_mul_image),
											data_len,
											hmi_dxy_scrollbar_table[hmi_object_index].file.w,
											hmi_dxy_scrollbar_table[hmi_object_index].file.h,
											bmp_alpha_rotation_flag & HMI_ROTATION_IMAGE_FLAG,										
											bmp_compress,
											0,/*no used*/
											0,/*no used*/
											(hmi_dxy_scrollbar_attr_table[hmi_object_index].pixel_fmt)& 0x1f
											);
				}
				else
				{					
					get_texture_res_manager(hmi_object_id,										
											NULL,/*if load failed,not try,not free other resource*/
											(HMI_COMPRESS_IMAGE_LIST_STR *)(&hmi_mul_image),
											data_len,
											hmi_dxy_scrollbar_table[hmi_object_index].file.w,
											hmi_dxy_scrollbar_table[hmi_object_index].file.h,
											bmp_alpha_rotation_flag & HMI_ROTATION_IMAGE_FLAG,										
											bmp_compress,
											0,/*no used*/
											0,/*no used*/
											(hmi_dxy_scrollbar_attr_table[hmi_object_index].pixel_fmt)& 0x1f
											);
				}
				
			}									
		#endif														
		}
		else		
	#endif
	#if HMI_SXY_SCROLLBAR_NUMBER>0
		if(HMI_IS_DYN_SCROLLBAR_SXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_SCROLLBAR_SXY_ID_INDEX(hmi_object_id);						
		#if HMI_SCROLLBAR_MAX_STATUS >= 0						
			if(hmi_object_index < HMI_SXY_SCROLLBAR_NUMBER)
			{
				image_format= (rgl_image_type_str)(get_compress_fmt(hmi_sxy_scrollbar_attr_table[hmi_object_index]));
				hmi_mul_image.pbitmap_data	= (U08 *)(hmi_sxy_scrollbar_table[hmi_object_index].file.pbitmap_data);
				hmi_mul_image.data_len		= hmi_sxy_scrollbar_table[hmi_object_index].file.data_len;										
				bmp_alpha_rotation_flag		= (hmi_sxy_scrollbar_attr_table[hmi_object_index].image_attr) & 0xf0;
				bmp_compress				= get_compress_fmt(hmi_sxy_scrollbar_attr_table[hmi_object_index]);
				data_len					= 1u;
				if(brel_buf == TRUE)
				{
					get_texture_res_manager(hmi_object_id,										
											&ptexture,
											(HMI_COMPRESS_IMAGE_LIST_STR *)(&hmi_mul_image),
											data_len,
											hmi_sxy_scrollbar_table[hmi_object_index].file.w,
											hmi_sxy_scrollbar_table[hmi_object_index].file.h,
											bmp_alpha_rotation_flag & HMI_ROTATION_IMAGE_FLAG,										
											bmp_compress,
											0,/*no used*/
											0,/*no used*/
											(hmi_sxy_scrollbar_attr_table[hmi_object_index].pixel_fmt)& 0x1f
											);
				}
				else
				{
					get_texture_res_manager(hmi_object_id,										
										NULL,/*if load failed,not try,not free other resource*/
										(HMI_COMPRESS_IMAGE_LIST_STR *)(&hmi_mul_image),
										data_len,
										hmi_sxy_scrollbar_table[hmi_object_index].file.w,
										hmi_sxy_scrollbar_table[hmi_object_index].file.h,
										bmp_alpha_rotation_flag & HMI_ROTATION_IMAGE_FLAG,										
										bmp_compress,
										0,/*no used*/
										0,/*no used*/
										(hmi_sxy_scrollbar_attr_table[hmi_object_index].pixel_fmt)& 0x1f
										);
				}
			}				
		#endif															
		}
		else
	#endif 
	#if HMI_DXY_BUTTON_NUMBER > 0  
		if(HMI_IS_DYN_BUTTON_DXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_BUTTON_DXY_ID_INDEX(hmi_object_id);					
			if(hmi_object_index < HMI_DXY_BUTTON_NUMBER)
			{		
				image_format= (rgl_image_type_str)(get_compress_fmt(hmi_dxy_button_attr_table[hmi_object_index]));
				pImage_data = (HMI_COMPRESS_IMAGE_LIST_STR *)(hmi_dxy_button_table[hmi_object_index].button_image.file.pimagelist_attr);
				data_len	= hmi_dxy_button_table[hmi_object_index].button_image.list_len;							
				bmp_alpha_rotation_flag	= (hmi_dxy_button_attr_table[hmi_object_index].image_attr)& 0xf0;
				bmp_compress			= get_compress_fmt(hmi_dxy_button_attr_table[hmi_object_index]);
				
				img_frame_w	= hmi_dxy_button_table[hmi_object_index].button_image.file.w; // 2021 04 04
				if(data_len != 0u)
				{
					img_frame_w	= (HMI_WIDTH_STR)(hmi_dxy_button_table[hmi_object_index].button_image.file.w / data_len);
				}
				if(brel_buf == TRUE)
				{
				#if	(HMI_LOAD_ONE_FRAME_IMGLIST == NO)
					for(i = 0u; i < data_len;i++)
				#endif
					{
						get_texture_res_manager(hmi_object_id,										
												&ptexture,
												(HMI_COMPRESS_IMAGE_LIST_STR *)pImage_data,
												data_len,
												img_frame_w,//hmi_dxy_button_table[hmi_object_index].button_image.file.w,
												hmi_dxy_button_table[hmi_object_index].button_image.file.h,
												bmp_alpha_rotation_flag & HMI_ROTATION_IMAGE_FLAG,										
												bmp_compress,
												0,/*no used*/
											#if	(HMI_LOAD_ONE_FRAME_IMGLIST == NO)
												i,/*imagelist index*/
											#else
												0,
											#endif
												(hmi_dxy_button_attr_table[hmi_object_index].pixel_fmt)& 0x1f
												);
					}
				}
				else
				{
				#if	(HMI_LOAD_ONE_FRAME_IMGLIST == NO)
					for(i = 0u; i < data_len;i++)
				#endif
					{
						get_texture_res_manager(hmi_object_id,										
												NULL,/*if load failed,not try,not free other resource*/
												(HMI_COMPRESS_IMAGE_LIST_STR *)pImage_data,
												data_len,
												img_frame_w,//hmi_dxy_button_table[hmi_object_index].button_image.file.w,
												hmi_dxy_button_table[hmi_object_index].button_image.file.h,
												bmp_alpha_rotation_flag & HMI_ROTATION_IMAGE_FLAG,										
												bmp_compress,
												0,/*no used*/
											#if	(HMI_LOAD_ONE_FRAME_IMGLIST == NO)
												i,/*imagelist index*/
											#else
												0,
											#endif
												(hmi_dxy_button_attr_table[hmi_object_index].pixel_fmt)& 0x1f
												);
					}
				}
			}				
		}		
		else
	#endif		
	#if HMI_SXY_BUTTON_NUMBER>0 		
		if(HMI_IS_DYN_BUTTON_SXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_BUTTON_SXY_ID_INDEX(hmi_object_id);										
			if(hmi_object_index < HMI_SXY_BUTTON_NUMBER)
			{
				image_format			= (rgl_image_type_str)(get_compress_fmt(hmi_sxy_button_attr_table[hmi_object_index]));
				pImage_data				= (HMI_COMPRESS_IMAGE_LIST_STR *)(hmi_sxy_button_table[hmi_object_index].button_image.file.pimagelist_attr);
				data_len				= hmi_sxy_button_table[hmi_object_index].button_image.list_len;				
				bmp_alpha_rotation_flag	= (hmi_sxy_button_attr_table[hmi_object_index].image_attr)& 0xf0;
				bmp_compress			= get_compress_fmt(hmi_sxy_button_attr_table[hmi_object_index]);

				img_frame_w	= hmi_sxy_button_table[hmi_object_index].button_image.file.w; // 2021 04 04
				if(data_len != 0u)
				{
					img_frame_w	= (HMI_WIDTH_STR)(hmi_sxy_button_table[hmi_object_index].button_image.file.w / data_len);
				}
				if(brel_buf == TRUE)
				{
				#if	(HMI_LOAD_ONE_FRAME_IMGLIST == NO)
					for(i = 0u; i < data_len;i++)
				#endif
					{
						get_texture_res_manager(hmi_object_id,										
												&ptexture,
												(HMI_COMPRESS_IMAGE_LIST_STR *)pImage_data,
												data_len,
												img_frame_w,//hmi_sxy_button_table[hmi_object_index].button_image.file.w,
												hmi_sxy_button_table[hmi_object_index].button_image.file.h,
												bmp_alpha_rotation_flag & HMI_ROTATION_IMAGE_FLAG,										
												bmp_compress,
												0,/*no used*/
											#if	(HMI_LOAD_ONE_FRAME_IMGLIST == NO)
												i,/*imagelist index*/
											#else
												0,
											#endif
												(hmi_sxy_button_attr_table[hmi_object_index].pixel_fmt)& 0x1f
												);
					}
				}
				else
				{
				#if	(HMI_LOAD_ONE_FRAME_IMGLIST == NO)
					for(i = 0u; i < data_len;i++)
				#endif
					{
						get_texture_res_manager(hmi_object_id,										
											NULL,/*if load failed,not try,not free other resource*/
											(HMI_COMPRESS_IMAGE_LIST_STR *)pImage_data,
											data_len,
											img_frame_w,//hmi_sxy_button_table[hmi_object_index].button_image.file.w,
											hmi_sxy_button_table[hmi_object_index].button_image.file.h,
											bmp_alpha_rotation_flag & HMI_ROTATION_IMAGE_FLAG,										
											bmp_compress,
											0,/*no used*/
										#if	(HMI_LOAD_ONE_FRAME_IMGLIST == NO)
											i,/*imagelist index*/
										#else
											0,
										#endif
											(hmi_sxy_button_attr_table[hmi_object_index].pixel_fmt)& 0x1f
											);
					}
				}
			}																			
		}		
		else		
	#endif
	#if HMI_DXY_BITMAPS_NUMBER> 0 
		if(HMI_IS_DYN_XY_BITMAP(hmi_object_id))
		{	  
			hmi_object_index = HMI_GET_DYN_XY_BITMAP_INDEX(hmi_object_id ); 													
			if(hmi_object_index < HMI_DXY_BITMAPS_NUMBER)
			{
				image_format= (rgl_image_type_str)(get_compress_fmt(hmi_dxy_bitmap_attr_table[hmi_object_index]));
								
				hmi_mul_image.pbitmap_data	= (U08 *)(hmi_bmp_dyn_xy_prop_table[hmi_object_index].pbitmap_data);
				hmi_mul_image.data_len		= hmi_bmp_dyn_xy_prop_table[hmi_object_index].data_len;				
				bmp_alpha_rotation_flag		= (hmi_dxy_bitmap_attr_table[hmi_object_index].image_attr)& 0xf0;
				bmp_compress				= get_compress_fmt(hmi_dxy_bitmap_attr_table[hmi_object_index]);
				data_len					= 1u;
				if(brel_buf == TRUE)
				{
					get_texture_res_manager(hmi_object_id,										
											&ptexture,
											(HMI_COMPRESS_IMAGE_LIST_STR *)(&hmi_mul_image),
											data_len,
											hmi_bmp_dyn_xy_prop_table[hmi_object_index].w,
											hmi_bmp_dyn_xy_prop_table[hmi_object_index].h,
											bmp_alpha_rotation_flag & HMI_ROTATION_IMAGE_FLAG,										
											bmp_compress,
											0,/*no used*/
											0,/*no used*/
											(hmi_dxy_bitmap_attr_table[hmi_object_index].pixel_fmt)& 0x1f
											);
				}
				else
				{
					get_texture_res_manager(hmi_object_id,										
										NULL,/*if load failed,not try,not free other resource*/
										(HMI_COMPRESS_IMAGE_LIST_STR *)(&hmi_mul_image),
										data_len,
										hmi_bmp_dyn_xy_prop_table[hmi_object_index].w,
										hmi_bmp_dyn_xy_prop_table[hmi_object_index].h,
										bmp_alpha_rotation_flag & HMI_ROTATION_IMAGE_FLAG,										
										bmp_compress,
										0,/*no used*/
										0,/*no used*/
										(hmi_dxy_bitmap_attr_table[hmi_object_index].pixel_fmt)& 0x1f
										);
				}								
			}																							
		}
		else
	#endif					     	   	   		
	#if HMI_SXY_BITMAPS_NUMBER> 0
		if(HMI_IS_S_XY_BITMAP(hmi_object_id))
		{			 
			hmi_object_index = HMI_GET_SXY_BITMAPS_ID_INDEX(hmi_object_id); 				
			if(hmi_object_index < HMI_SXY_BITMAPS_NUMBER)
			{
				image_format= (rgl_image_type_str)(get_compress_fmt(hmi_sxy_bitmap_attr_table[hmi_object_index]));
								
				hmi_mul_image.pbitmap_data = (U08 *)(hmi_bmp_static_xy_prop_table[hmi_object_index].pbitmap_data);
				hmi_mul_image.data_len	= hmi_bmp_static_xy_prop_table[hmi_object_index].data_len;				
				bmp_alpha_rotation_flag	= (hmi_sxy_bitmap_attr_table[hmi_object_index].image_attr)& 0xf0;
				bmp_compress			= get_compress_fmt(hmi_sxy_bitmap_attr_table[hmi_object_index]);
				data_len				= 1u;
				if(brel_buf == TRUE)
				{
					get_texture_res_manager(hmi_object_id,										
											&ptexture,
											(HMI_COMPRESS_IMAGE_LIST_STR *)(&hmi_mul_image),
											data_len,
											hmi_bmp_static_xy_prop_table[hmi_object_index].w,
											hmi_bmp_static_xy_prop_table[hmi_object_index].h,
											bmp_alpha_rotation_flag & HMI_ROTATION_IMAGE_FLAG,										
											bmp_compress,
											0,/*no used*/
											0,/*no used*/
											(hmi_sxy_bitmap_attr_table[hmi_object_index].pixel_fmt)& 0x1f
											);	
				}
				else
				{
					get_texture_res_manager(hmi_object_id,										
										NULL,/*if load failed,not try,not free other resource*/
										(HMI_COMPRESS_IMAGE_LIST_STR *)(&hmi_mul_image),
										data_len,
										hmi_bmp_static_xy_prop_table[hmi_object_index].w,
										hmi_bmp_static_xy_prop_table[hmi_object_index].h,
										bmp_alpha_rotation_flag & HMI_ROTATION_IMAGE_FLAG,										
										bmp_compress,
										0,/*no used*/
										0,/*no used*/
										(hmi_sxy_bitmap_attr_table[hmi_object_index].pixel_fmt)& 0x1f
										);	
				}
			}		
		}		
		else
	#endif 	   		 
		{
			;
		}		   	
	}
}

#if (HMI_LOAD_SOURCE_BUF_MODE == HMI_LOAD_RES_BUF_ALL_INIT) 
void	hmi_load_all_to_buf_pixel(void)
{
	HMI_OBJECT_ID_STR		hmi_object_id	= 0U;	
	while(hmi_object_id < HMI_EVENT_ACT_BEGIN_INDEX)
	{
		hmi_load_file_to_pixel_buf(hmi_object_id,FALSE);
		hmi_object_id++;
	}
}
#endif

void hmi_unload_file_to_pixel_buf(HMI_OBJECT_ID_STR	hmi_object_id)
{	
	rgl_buffer_img_str		*pbuf				= NULL;
	SINT16					buffer_info_index	= VGLITE_BUFFER_END_FLAG;

	pbuf	= hmi_rgl_get_element_pointer(hmi_object_id);
	if(pbuf != NULL)
	{
		buffer_info_index	= pbuf->vg_buf_id;	
#if	(HMI_LOAD_ONE_FRAME_IMGLIST == YES)
		hmi_release_pixel_buf(buffer_info_index);
#else
		hmi_release_pixel_buf_multi(buffer_info_index);
#endif
	}
}

#endif	/*undef vglite*/

void hmi_load_file_to_vram(HMI_OBJECT_ID_STR	hmi_object_id)
{
#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)/*|| defined(HMI_GRAPHIC_VGLITE)*/

	#if HMI_DXY_IMAGELIST_NUMBER		+	\
		HMI_SXY_IMAGELIST_NUMBER		+	\
		HMI_DXY_BUTTON_NUMBER			+	\
		HMI_SXY_BUTTON_NUMBER	> 0
	HMI_COMPRESS_IMAGE_LIST_STR	*pImage_data		= NULL;
	UINT8						data_len			= 0;
	
	#endif
	//HMI_BITMAP_STR	CONST		*pImage_data		= NULL;
	HMI_OBJECT_ID_STR		hmi_object_index	= 0;
	HMI_COMPRESS_IMAGE_LIST_STR  hmi_mul_image			={0};
	rgl_image_type_str		image_format= HMI_IMAGE_NO_COMPRESS;
	if(hmi_object_id  >= HMI_PAGE_SXY_MAX_ID)/*not a page*/
	{				
	#if HMI_DXY_IMAGELIST_NUMBER>0
		if(HMI_IS_DYN_IMAGELIST_DXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_IMAGELIST_DXY_ID_INDEX(hmi_object_id);												
		#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)|| defined(S6J3200_GRAPHIC)
			if(hmi_object_index < HMI_DXY_IMAGELIST_NUMBER)
			{
				pImage_data = (HMI_COMPRESS_IMAGE_LIST_STR *)(hmi_dxy_imagelist_table[hmi_object_index].file.pimagelist_attr);
				data_len	= hmi_dxy_imagelist_table[hmi_object_index].list_len;
				hmi_rgl_load_to_vram(hmi_object_id,(HMI_COMPRESS_IMAGE_LIST_STR *)pImage_data,data_len);
			}
		#elif(defined(HMI_GRAPHIC_OPENGLES))
			if(hmi_object_index < HMI_DXY_IMAGELIST_NUMBER)
			{
				pImage_data = &(hmi_dxy_imagelist_table[hmi_object_index].file);
				//data_len	= hmi_dxy_imagelist_table[hmi_object_index].list_len;
				hmi_rgl_load_to_vram(hmi_object_id,pImage_data,NULL);
			}
		#endif									
		}		
		else
	#endif
	#if HMI_SXY_IMAGELIST_NUMBER>0
		if(HMI_IS_DYN_IMAGELIST_SXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_IMAGELIST_SXY_ID_INDEX(hmi_object_id);			
		#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)|| defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)
			if(hmi_object_index < HMI_SXY_IMAGELIST_NUMBER)
			{
				pImage_data = (HMI_COMPRESS_IMAGE_LIST_STR *)(hmi_sxy_imagelist_table[hmi_object_index].file.pimagelist_attr);
				data_len	= hmi_sxy_imagelist_table[hmi_object_index].list_len;
				hmi_rgl_load_to_vram(hmi_object_id,(HMI_COMPRESS_IMAGE_LIST_STR *)pImage_data,data_len);
			}
		#elif(defined(HMI_GRAPHIC_OPENGLES))
			if(hmi_object_index < HMI_SXY_IMAGELIST_NUMBER)
			{
				pImage_data = &(hmi_sxy_imagelist_table[hmi_object_index].file);
				//data_len	= hmi_sxy_imagelist_table[hmi_object_index].list_len;
				hmi_rgl_load_to_vram(hmi_object_id,pImage_data,NULL);
			}
		#endif												
		}
		else
	#endif			
	#if HMI_DXY_SCROLLBAR_NUMBER>0
		if(HMI_IS_DYN_SCROLLBAR_DXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_SCROLLBAR_DXY_ID_INDEX(hmi_object_id);									
		#if HMI_SCROLLBAR_MAX_STATUS >= 0			
			{				
				#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)|| defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)
				if(hmi_object_index < HMI_DXY_SCROLLBAR_NUMBER)
				{
					hmi_mul_image.pbitmap_data= (U08 *)(hmi_dxy_scrollbar_table[hmi_object_index].file.pbitmap_data);
					hmi_mul_image.data_len= hmi_dxy_scrollbar_table[hmi_object_index].file.data_len;
					hmi_rgl_load_to_vram(hmi_object_id,(HMI_COMPRESS_IMAGE_LIST_STR *)(&hmi_mul_image),1);
				}
				#elif(defined(HMI_GRAPHIC_OPENGLES))
				if(hmi_object_index < HMI_DXY_SCROLLBAR_NUMBER)
				{
					pImage_data	= &(hmi_dxy_scrollbar_table[hmi_object_index].file);
					//hmi_mul_image.data_len= hmi_dxy_scrollbar_table[hmi_object_index].file.data_len;
					hmi_rgl_load_to_vram(hmi_object_id,pImage_data,NULL);
				}
				#endif
			}		
		#endif														
		}
		else		
	#endif
	#if HMI_SXY_SCROLLBAR_NUMBER>0
		if(HMI_IS_DYN_SCROLLBAR_SXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_SCROLLBAR_SXY_ID_INDEX(hmi_object_id);						
		#if HMI_SCROLLBAR_MAX_STATUS >= 0			
			#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)|| defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)
			if(hmi_object_index < HMI_SXY_SCROLLBAR_NUMBER)
			{
				hmi_mul_image.pbitmap_data = (U08 *)(hmi_sxy_scrollbar_table[hmi_object_index].file.pbitmap_data);
				hmi_mul_image.data_len	= hmi_sxy_scrollbar_table[hmi_object_index].file.data_len;
				hmi_rgl_load_to_vram(hmi_object_id,(HMI_COMPRESS_IMAGE_LIST_STR *)(&hmi_mul_image),1);						
			}	
			#elif(defined(HMI_GRAPHIC_OPENGLES))
			if(hmi_object_index < HMI_SXY_SCROLLBAR_NUMBER)
			{
				pImage_data = &(hmi_sxy_scrollbar_table[hmi_object_index].file);
				//hmi_mul_image.data_len	= hmi_sxy_scrollbar_table[hmi_object_index].file.data_len;
				hmi_rgl_load_to_vram(hmi_object_id,pImage_data,NULL);						
			}	
			#endif
		#endif															
		}
		else
	#endif 
	#if HMI_DXY_BUTTON_NUMBER > 0  
		if(HMI_IS_DYN_BUTTON_DXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_BUTTON_DXY_ID_INDEX(hmi_object_id);							
			#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)|| defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)
			if(hmi_object_index < HMI_DXY_BUTTON_NUMBER)
			{				
				pImage_data = (HMI_COMPRESS_IMAGE_LIST_STR *)(hmi_dxy_button_table[hmi_object_index].button_image.file.pimagelist_attr);
				data_len	= hmi_dxy_button_table[hmi_object_index].button_image.list_len;
				hmi_rgl_load_to_vram(hmi_object_id,(HMI_COMPRESS_IMAGE_LIST_STR *)pImage_data,data_len);					
				
			}	
			#elif(defined(HMI_GRAPHIC_OPENGLES))
			if(hmi_object_index < HMI_DXY_BUTTON_NUMBER)
			{				
				pImage_data = &(hmi_dxy_button_table[hmi_object_index].button_image.file);
				//data_len	= hmi_dxy_button_table[hmi_object_index].button_image.list_len;
				hmi_rgl_load_to_vram(hmi_object_id,pImage_data,NULL);					
				
			}	
			#endif
		}		
		else
	#endif		
	#if HMI_SXY_BUTTON_NUMBER>0 		
		if(HMI_IS_DYN_BUTTON_SXY(hmi_object_id))
		{
			hmi_object_index = HMI_GET_DYN_BUTTON_SXY_ID_INDEX(hmi_object_id);			
		#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)|| defined(HMI_GRAPHIC_VGLITE)	||defined(HMI_GRAPHIC_OPENVG)					
			if(hmi_object_index < HMI_SXY_BUTTON_NUMBER)
			{
				pImage_data = (HMI_COMPRESS_IMAGE_LIST_STR *)(hmi_sxy_button_table[hmi_object_index].button_image.file.pimagelist_attr);
				data_len	= hmi_sxy_button_table[hmi_object_index].button_image.list_len;
				hmi_rgl_load_to_vram(hmi_object_id,(HMI_COMPRESS_IMAGE_LIST_STR *)pImage_data,data_len);
			}	
		#elif(defined(HMI_GRAPHIC_OPENGLES))
			if(hmi_object_index < HMI_SXY_BUTTON_NUMBER)
			{
				pImage_data = &(hmi_sxy_button_table[hmi_object_index].button_image.file);
				//data_len	= hmi_sxy_button_table[hmi_object_index].button_image.list_len;
				hmi_rgl_load_to_vram(hmi_object_id,pImage_data,NULL);
			}	
		#endif																	
		}		
		else		
	#endif
	#if HMI_DXY_BITMAPS_NUMBER> 0 
		if(HMI_IS_DYN_XY_BITMAP(hmi_object_id))
		{	  
			hmi_object_index = HMI_GET_DYN_XY_BITMAP_INDEX(hmi_object_id ); 											
		#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)|| defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)
			if(hmi_object_index < HMI_DXY_BITMAPS_NUMBER)
			{
				image_format= (rgl_image_type_str)(get_compress_fmt(hmi_dxy_bitmap_attr_table[hmi_object_index]));
				#if (HMI_LOAD_JPG_IMAGE==HMI_YES)
				hmi_mul_image.pbitmap_data = (U08 *)(hmi_bmp_dyn_xy_prop_table[hmi_object_index].pbitmap_data);
				hmi_mul_image.data_len	= hmi_bmp_dyn_xy_prop_table[hmi_object_index].data_len;
				hmi_rgl_load_to_vram(hmi_object_id,(HMI_COMPRESS_IMAGE_LIST_STR *)(&hmi_mul_image),1);
				#else
				if(image_format !=HMI_IMAGE_JPG)
				{
					hmi_mul_image.pbitmap_data = (U08 *)(hmi_bmp_dyn_xy_prop_table[hmi_object_index].pbitmap_data);
					hmi_mul_image.data_len	= hmi_bmp_dyn_xy_prop_table[hmi_object_index].data_len;
					hmi_rgl_load_to_vram(hmi_object_id,(HMI_COMPRESS_IMAGE_LIST_STR *)(&hmi_mul_image),1);
				}
				#endif
			}																					
		#elif(defined(HMI_GRAPHIC_OPENGLES))
			if(hmi_object_index < HMI_DXY_BITMAPS_NUMBER)
			{
				pImage_data = &(hmi_bmp_dyn_xy_prop_table[hmi_object_index]);
				//hmi_mul_image.data_len	= hmi_bmp_dyn_xy_prop_table[hmi_object_index].data_len;
				hmi_rgl_load_to_vram(hmi_object_id,pImage_data,NULL);
			}	
		#endif

		}
		else
	#endif					     	   	   		
	#if HMI_SXY_BITMAPS_NUMBER> 0
		if(HMI_IS_S_XY_BITMAP(hmi_object_id))
		{			 
			hmi_object_index = HMI_GET_SXY_BITMAPS_ID_INDEX(hmi_object_id); 		
		#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)|| defined(S6J3200_GRAPHIC)
			if(hmi_object_index < HMI_SXY_BITMAPS_NUMBER)
			{
				image_format= (rgl_image_type_str)(get_compress_fmt(hmi_sxy_bitmap_attr_table[hmi_object_index]));
				#if (HMI_LOAD_JPG_IMAGE==HMI_YES)
				hmi_mul_image.pbitmap_data = (U08 *)(hmi_bmp_static_xy_prop_table[hmi_object_index].pbitmap_data);
				hmi_mul_image.data_len	= hmi_bmp_static_xy_prop_table[hmi_object_index].data_len;
				hmi_rgl_load_to_vram(hmi_object_id,(HMI_COMPRESS_IMAGE_LIST_STR *)(&hmi_mul_image),1);
				#else
				if(image_format !=HMI_IMAGE_JPG)
				{
					hmi_mul_image.pbitmap_data = (U08 *)(hmi_bmp_static_xy_prop_table[hmi_object_index].pbitmap_data);
					hmi_mul_image.data_len	= hmi_bmp_static_xy_prop_table[hmi_object_index].data_len;
					hmi_rgl_load_to_vram(hmi_object_id,(HMI_COMPRESS_IMAGE_LIST_STR *)(&hmi_mul_image),1);
				}
				#endif
				
			}
		#elif(defined(HMI_GRAPHIC_OPENGLES))
			if(hmi_object_index < HMI_SXY_BITMAPS_NUMBER)
			{	
				pImage_data = &(hmi_bmp_static_xy_prop_table[hmi_object_index]);
				//hmi_mul_image.data_len	= hmi_bmp_static_xy_prop_table[hmi_object_index].data_len;
				hmi_rgl_load_to_vram(hmi_object_id,pImage_data,NULL);
			}
		#endif

		}		
		else
	#endif 	   		 
		{
			;
		}		   
	
	}

#endif	
}

void call_C_hmi_driver_load_file(HMI_CONTAINER_U16_STR CONST * phmi_container_info)
{
#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)
	UINT16 hmi_number_object  = phmi_container_info->container_object_table.object_number;
	HMI_OBJECT_PROP_STR CONST * phmi_container_object_table= phmi_container_info->container_object_table.p_object_table;
	HMI_OBJECT_ID_STR		hmi_object_id	= 0; 

	while(hmi_number_object > 0)	
	{
		hmi_object_id=phmi_container_object_table->object_id;
		hmi_load_file_to_vram(hmi_object_id);
		hmi_number_object--;	
		phmi_container_object_table++;
	}
#endif	
}

#if defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)
	#if (HMI_LOAD_SOURCE_BUF_MODE == HMI_LOAD_RES_BUF_SEGMEN) 
void call_C_hmi_driver_load_buf_file(HMI_CONTAINER_U16_STR CONST * phmi_container_info)
{
	UINT16 hmi_number_object  = phmi_container_info->container_object_table.object_number;
	HMI_OBJECT_PROP_STR CONST * phmi_container_object_table= phmi_container_info->container_object_table.p_object_table;
	HMI_OBJECT_ID_STR		hmi_object_id	= 0; 

	while(hmi_number_object > 0u)	
	{
		hmi_object_id	= phmi_container_object_table->object_id;		
		hmi_load_file_to_pixel_buf(hmi_object_id,TRUE/*FALSE 2021 08 02*/);
		hmi_number_object--;	
		phmi_container_object_table++;
	}
}
	#endif
#endif

#if 0
void call_C_hmi_driver_load_file(HMI_CONTAINER_U16_STR CONST * phmi_container_info)
{
	UINT16 hmi_number_object  = phmi_container_info->container_object_table.object_number;
	HMI_OBJECT_PROP_STR CONST * phmi_container_object_table= phmi_container_info->container_object_table.p_object_table;
	HMI_OBJECT_ID_STR		hmi_object_id	= 0; 

	while(hmi_number_object > 0)	
	{
		hmi_object_id=phmi_container_object_table->object_id;
		hmi_load_file_to_vram(hmi_object_id);
		hmi_number_object--;	
		phmi_container_object_table++;
	}
}
#endif

#if (HMI_LOAD_SOURCE_MODE == HMI_LOAD_RES_ALL_INIT)
void call_C_hmi_driver_load_all_file(void)
{
#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)/*|| defined(S6J3200_GRAPHIC)*/
	HMI_OBJECT_ID_STR		hmi_object_id	= 0U;	
	while(hmi_object_id < HMI_EVENT_ACT_BEGIN_INDEX)
	{
		hmi_load_file_to_vram(hmi_object_id);
		hmi_object_id++;
	}
#elif defined(HMI_GRAPHIC_OPENGLES)	
	hmi_load_bmp_all_segment();
		
#else
	
#endif
}
#endif

#if (HMI_LOAD_SOURCE_MODE == HMI_LOAD_RES_ONLY_ROTATION)
void call_C_hmi_driver_load_rotation_file(void)
{
#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)
	#if (HMI_DXY_BITMAPS_NUMBER	!= 0) 
	HMI_OBJECT_ID_STR		hmi_object_id	= HMI_DYN_XY_CONTAINER_MAX_ID;	
	HMI_OBJECT_ID_STR		hmi_object_index= 0;		
	BOOLEAN 				rotation		= FALSE;
	UINT8					bmp_alpha_rotation_flag = 0;
	HMI_COMPRESS_IMAGE_LIST_STR  hmi_mul_image			={0};
	while(HMI_IS_DYN_XY_BITMAP(hmi_object_id))
	{
		hmi_object_index = HMI_GET_DYN_XY_BITMAP_INDEX(hmi_object_id );												
		if(hmi_object_index < HMI_DXY_BITMAPS_NUMBER)
		{
			hmi_mul_image.pbitmap_data= (U08 *)(hmi_bmp_dyn_xy_prop_table[hmi_object_index].pbitmap_data);
			hmi_mul_image.data_len= hmi_bmp_dyn_xy_prop_table[hmi_object_index].data_len;
			bmp_alpha_rotation_flag = hmi_dxy_bitmap_attr_table[hmi_object_index].image_attr&0xf0;
			rotation	= bmp_alpha_rotation_flag&HMI_ROTATION_IMAGE_FLAG;
			if(rotation == TRUE)
			{
				hmi_rgl_load_to_vram(hmi_object_id,
					(HMI_COMPRESS_IMAGE_LIST_STR *)(&hmi_mul_image),1);
			}
		}						
		hmi_object_id++;
	}
	#endif
#endif
}
#endif

#if(HMI_LOAD_SOURCE_MODE == HMI_LOAD_RES_SHEET)
BOOLEAN hmi_load_sheet_to_vram(HMI_OBJECT_ID_STR hmi_object_id,
								UINT32 *phmi_image_loaded_len)
{
	BOOLEAN		hmi_load_success	= FALSE;
#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)
	#if HMI_DXY_IMAGELIST_NUMBER		+	\
		HMI_SXY_IMAGELIST_NUMBER		+	\
		HMI_DXY_BUTTON_NUMBER			+	\
		HMI_SXY_BUTTON_NUMBER	> 0
	HMI_COMPRESS_IMAGE_LIST_STR	*pdata			= NULL;
	UINT8						img_index_len	= 0;
	#endif
	HMI_OBJECT_ID_STR		hmi_object_index	= 0;
	HMI_COMPRESS_IMAGE_LIST_STR  hmi_mul_image	= {0};
	rgl_image_type_str		image_format		= HMI_IMAGE_NO_COMPRESS;
	
	#if HMI_DXY_IMAGELIST_NUMBER > 0
	if(HMI_IS_DYN_IMAGELIST_DXY(hmi_object_id))
    {
		hmi_object_index = HMI_GET_DYN_IMAGELIST_DXY_ID_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_DXY_IMAGELIST_NUMBER)
		{
			pdata		= (HMI_COMPRESS_IMAGE_LIST_STR *)(hmi_dxy_imagelist_table[hmi_object_index].file.pimagelist_attr);
			img_index_len		= hmi_dxy_imagelist_table[hmi_object_index].list_len;
			hmi_load_success	= hmi_rgl_load_sheet_to_vram(hmi_object_id,pdata,
									img_index_len,phmi_image_loaded_len);
		}

	}		
	else
	#endif
	#if HMI_SXY_IMAGELIST_NUMBER > 0
	if(HMI_IS_DYN_IMAGELIST_SXY(hmi_object_id))
    {
		hmi_object_index = HMI_GET_DYN_IMAGELIST_SXY_ID_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_SXY_IMAGELIST_NUMBER)
		{
			pdata		= (HMI_COMPRESS_IMAGE_LIST_STR *)(hmi_sxy_imagelist_table[hmi_object_index].file.pimagelist_attr);
			img_index_len		= hmi_sxy_imagelist_table[hmi_object_index].list_len;
			hmi_load_success	= hmi_rgl_load_sheet_to_vram(hmi_object_id,pdata,
											img_index_len,phmi_image_loaded_len);
		}
	}
	else
	#endif
	#if HMI_DXY_SCROLLBAR_NUMBER > 0
	if(HMI_IS_DYN_SCROLLBAR_DXY(hmi_object_id))
    {
		hmi_object_index = HMI_GET_DYN_SCROLLBAR_DXY_ID_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_DXY_SCROLLBAR_NUMBER)
		{
			hmi_mul_image.pbitmap_data	= (U08 *)(hmi_dxy_scrollbar_table[hmi_object_index].file.pbitmap_data);
			hmi_mul_image.data_len		= hmi_dxy_scrollbar_table[hmi_object_index].file.data_len;			
			hmi_load_success			= hmi_rgl_load_sheet_to_vram(hmi_object_id,&hmi_mul_image,
											1,phmi_image_loaded_len);
		}	
	}
	else		
	#endif
	#if HMI_SXY_SCROLLBAR_NUMBER > 0
	if(HMI_IS_DYN_SCROLLBAR_SXY(hmi_object_id))
    {
		hmi_object_index = HMI_GET_DYN_SCROLLBAR_SXY_ID_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_SXY_SCROLLBAR_NUMBER)
		{
			hmi_mul_image.pbitmap_data	= (U08 *)(hmi_sxy_scrollbar_table[hmi_object_index].file.pbitmap_data);
			hmi_mul_image.data_len		= hmi_sxy_scrollbar_table[hmi_object_index].file.data_len;	
			hmi_load_success			= hmi_rgl_load_sheet_to_vram(hmi_object_id,&hmi_mul_image,
											1,phmi_image_loaded_len);
		}
	}
	else
	#endif
	#if HMI_DXY_BUTTON_NUMBER > 0  
	if(HMI_IS_DYN_BUTTON_DXY(hmi_object_id))
    {
		hmi_object_index = HMI_GET_DYN_BUTTON_DXY_ID_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_DXY_BUTTON_NUMBER)
		{
			pdata				= (HMI_COMPRESS_IMAGE_LIST_STR *)(hmi_dxy_button_table[hmi_object_index].button_image.file.pimagelist_attr);
			img_index_len		= hmi_dxy_button_table[hmi_object_index].button_image.list_len;
			hmi_load_success	= hmi_rgl_load_sheet_to_vram(hmi_object_id,
								pdata,img_index_len,phmi_image_loaded_len);
		}
	}		
	else
	#endif
	#if HMI_SXY_BUTTON_NUMBER>0 		
	if(HMI_IS_DYN_BUTTON_SXY(hmi_object_id))
    {
		hmi_object_index = HMI_GET_DYN_BUTTON_SXY_ID_INDEX(hmi_object_id);
		if(hmi_object_index < HMI_SXY_BUTTON_NUMBER)
		{
			pdata				= (HMI_COMPRESS_IMAGE_LIST_STR *)(hmi_sxy_button_table[hmi_object_index].button_image.file.pimagelist_attr);
			img_index_len		= hmi_sxy_button_table[hmi_object_index].button_image.list_len;
			hmi_load_success	= hmi_rgl_load_sheet_to_vram(hmi_object_id,pdata,
									img_index_len,phmi_image_loaded_len);
		}
	}		
	else 		
	#endif 
	#if HMI_DXY_BITMAPS_NUMBER> 0 
	if(HMI_IS_DYN_XY_BITMAP(hmi_object_id))
	{	  
		hmi_object_index = HMI_GET_DYN_XY_BITMAP_INDEX(hmi_object_id );
		if(hmi_object_index < HMI_DXY_BITMAPS_NUMBER)
		{
			image_format				= (rgl_image_type_str)(get_compress_fmt(hmi_dxy_bitmap_attr_table[hmi_object_index]));
			hmi_mul_image.pbitmap_data	= (U08 *)(hmi_bmp_dyn_xy_prop_table[hmi_object_index].pbitmap_data);
			hmi_mul_image.data_len		= hmi_bmp_dyn_xy_prop_table[hmi_object_index].data_len;
			#if (HMI_LOAD_JPG_IMAGE==HMI_YES)
			hmi_load_success	= hmi_rgl_load_sheet_to_vram(hmi_object_id,&hmi_mul_image,
									1,phmi_image_loaded_len);		
			#else
			if(image_format !=HMI_IMAGE_JPG)
			{
				hmi_load_success	= hmi_rgl_load_sheet_to_vram(hmi_object_id,&hmi_mul_image,
									1,phmi_image_loaded_len);	
			}
			else
			{
				hmi_load_success	= TRUE;
				*phmi_image_loaded_len	= 0;
			}
			#endif
		}
	}
	else
	#endif
	#if HMI_SXY_BITMAPS_NUMBER > 0
	if(HMI_IS_S_XY_BITMAP(hmi_object_id))
	{	         
        hmi_object_index = HMI_GET_SXY_BITMAPS_ID_INDEX(hmi_object_id);			
	    if(hmi_object_index < HMI_SXY_BITMAPS_NUMBER)
		{
			image_format= (rgl_image_type_str)(get_compress_fmt(hmi_sxy_bitmap_attr_table[hmi_object_index]));
			hmi_mul_image.pbitmap_data = (U08 *)(hmi_bmp_static_xy_prop_table[hmi_object_index].pbitmap_data);
			hmi_mul_image.data_len	= hmi_bmp_static_xy_prop_table[hmi_object_index].data_len;
			
			#if (HMI_LOAD_JPG_IMAGE==HMI_YES)
			hmi_load_success	= hmi_rgl_load_sheet_to_vram(hmi_object_id,&hmi_mul_image,
								1,phmi_image_loaded_len);		
			#else
			if(image_format !=HMI_IMAGE_JPG)
			{
				hmi_load_success	= hmi_rgl_load_sheet_to_vram(hmi_object_id,&hmi_mul_image,
								1,phmi_image_loaded_len);	
			}
			else
			{
				hmi_load_success	= TRUE;
				*phmi_image_loaded_len	= 0;
			}
			#endif
		}
	}
	else
	#endif
	{
		hmi_load_success		= TRUE;
		*phmi_image_loaded_len	= 0;
	}
#endif

	return hmi_load_success;
}

void hmi_load_sheet_add_all_id(void)
{
#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)
	HMI_OBJECT_ID_STR		hmi_object_id	= 0;
	
	hmi_load_fifo.head	= 0;
	hmi_load_fifo.tail	= 0;
	
	while((hmi_object_id < HMI_EVENT_ACT_BEGIN_INDEX)&&
			(hmi_load_fifo.tail < HMI_LOAD_LIST_MAX_NB))
	{
	#if HMI_DXY_IMAGELIST_NUMBER > 0
		if(HMI_IS_DYN_IMAGELIST_DXY(hmi_object_id))
		{
			hmi_load_id_list[hmi_load_fifo.tail]	= hmi_object_id;
			hmi_load_fifo.tail++;
		}		
		else
	#endif
	#if HMI_SXY_IMAGELIST_NUMBER > 0
		if(HMI_IS_DYN_IMAGELIST_SXY(hmi_object_id))
		{
			hmi_load_id_list[hmi_load_fifo.tail]	= hmi_object_id;
			hmi_load_fifo.tail++;
		}
		else
	#endif
	#if HMI_DXY_SCROLLBAR_NUMBER > 0
		if(HMI_IS_DYN_SCROLLBAR_DXY(hmi_object_id))
		{
			hmi_load_id_list[hmi_load_fifo.tail]	= hmi_object_id;
			hmi_load_fifo.tail++;
		}
		else		
	#endif
	#if HMI_SXY_SCROLLBAR_NUMBER > 0
		if(HMI_IS_DYN_SCROLLBAR_SXY(hmi_object_id))
		{
			hmi_load_id_list[hmi_load_fifo.tail]	= hmi_object_id;
			hmi_load_fifo.tail++;
		}
		else
	#endif
	#if HMI_DXY_BUTTON_NUMBER > 0  
		if(HMI_IS_DYN_BUTTON_DXY(hmi_object_id))
		{
			hmi_load_id_list[hmi_load_fifo.tail]	= hmi_object_id;
			hmi_load_fifo.tail++;
		}		
		else
	#endif
	#if HMI_SXY_BUTTON_NUMBER>0 		
		if(HMI_IS_DYN_BUTTON_SXY(hmi_object_id))
		{
			hmi_load_id_list[hmi_load_fifo.tail]	= hmi_object_id;
			hmi_load_fifo.tail++;
		}		
		else		
	#endif 
	#if HMI_DXY_BITMAPS_NUMBER> 0 
		if(HMI_IS_DYN_XY_BITMAP(hmi_object_id))
		{	  
			hmi_load_id_list[hmi_load_fifo.tail]	= hmi_object_id;
			hmi_load_fifo.tail++;
		}
		else
	#endif
	#if HMI_SXY_BITMAPS_NUMBER > 0
		if(HMI_IS_S_XY_BITMAP(hmi_object_id))
		{			 
			hmi_load_id_list[hmi_load_fifo.tail]	= hmi_object_id;
			hmi_load_fifo.tail++;
		}
		else
	#endif
		{
		}
		if(hmi_load_fifo.tail >= HMI_LOAD_SHEET_MAX_LEN)
		{
			hmi_load_fifo.tail	= 0;
		}
		hmi_object_id++;
	}
#endif	
}



void call_C_hmi_driver_load_sheet(void)	
{
#if defined(HMI_GRAPHIC_RGL	)|| defined(S6J3200_GRAPHIC)
	BOOLEAN hmi_load_success		= FALSE;
	UINT32	hmi_total_loaded_len	= 0U;
	UINT32	hmi_image_loaded_len	= 0U;
	UINT32  hmi_remained_len		= 0U;
	
	while((hmi_load_fifo.head != hmi_load_fifo.tail)&&
		(hmi_total_loaded_len < HMI_LOAD_SHEET_MAX_LEN ))
	{
		hmi_image_loaded_len = 0;
		hmi_load_success = hmi_load_sheet_to_vram(hmi_load_id_list[hmi_load_fifo.head],
							&hmi_image_loaded_len);
		
		hmi_total_loaded_len	+= hmi_image_loaded_len;
		hmi_remained_len	= HMI_LOAD_SHEET_MAX_LEN - hmi_total_loaded_len;
		if(hmi_remained_len < HMI_LOAD_SHEET_MIN_LEN)
		{
			hmi_total_loaded_len = HMI_LOAD_SHEET_MAX_LEN;
		}
		
		if(hmi_load_success == TRUE)
		{
			hmi_load_fifo.head ++;
			if(hmi_load_fifo.head >= HMI_LOAD_LIST_MAX_NB)
			{
				hmi_load_fifo.head = 0U;
			}
		}
	}
#endif	
}	

#endif
#endif 
//#endif lq




#if 0
void  hmi_send_cmdlist(
						#if defined(HMI_GRAPHIC_TWLIB)
						BOOLEAN	draw_all
						#endif
						)
{
	hmi_driver_send_cmdlist(draw_all);
}
#endif

#ifdef HMI_GRAPHIC_TWLIB
#if 0
static void hmi_engine_check_dynamic_object_changed(
								HMI_CONTAINER_STR CONST * phmi_container_info,
								HMI_RECT_STR CONST *pfarther_rect,																
								HMI_RECT_STR *pcliped_farther_rect)
{
	UINT8 hmi_number_object			= phmi_container_info->container_object_table.object_number;
	UINT8 hmi_number_object_const	= hmi_number_object;
	UINT8 hmi_page_redrawed_flag	= FALSE;
	//UINT8 hmi_object_dirty_flag		= FALSE;
	HMI_OBJECT_PROP_STR CONST * phmi_container_object_table = phmi_container_info->container_object_table.p_object_table;
	HMI_OBJECT_ID_STR	hmi_object_id	= 0U;
	HMI_OBJECT_ID_STR	hmi_object_index= 0U;
	HMI_OBJECT_ID_STR	hmi_check_index	=0U;
	HMI_RECT_STR hmi_node_screen_rect	= {HMI_INVALID_COOR,HMI_INVALID_COOR,0,0};
	HMI_RECT_STR hmi_insect_rect		= {HMI_INVALID_COOR,HMI_INVALID_COOR,0,0};	
	
	
	while((hmi_number_object > 0U) && (hmi_page_redrawed_flag == FALSE))
	{		
		hmi_object_id = phmi_container_object_table[hmi_check_index].object_id;
	  #if HMI_DXY_IMAGELIST_NUMBER>0
		if(hmi_object_id  < HMI_DYN_IMAGELIST_DXY_MAX_ID)
	    {
			hmi_object_index = HMI_GET_DYN_IMAGELIST_DXY_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DXY_IMAGELIST_NUMBER)
			{
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
										(&hmi_dxy_imagelist_rect[hmi_object_index]),
										(&hmi_node_screen_rect));
			}
			hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
								(HMI_RECT_STR CONST *)pcliped_farther_rect,
								&hmi_insect_rect);
			if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
		 	{					
				hmi_set_dirty_zone_layer((HMI_RECT_STR *)pcliped_farther_rect/*dxy node dirty zone is father zone*/,
										pdirty_rect,depth);
				hmi_page_redrawed_flag	= TRUE;
				hmi_object_dirty_flag	= TRUE;

				hmi_engine_draw_object
				
			}
			else
			{

				if(hmi_object_index < HMI_DXY_IMAGELIST_NUMBER)
				{
					hmi_object_dirty_flag=hmi_engine_check_dynamic_object_changed(
												&hmi_dxy_imagelist_container_table[hmi_object_index],
												(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),											
												pdirty_rect,
												depth,
												&hmi_insect_rect);
				}
			}
		}
		else
		#endif
		#if HMI_SXY_IMAGELIST_NUMBER>0
		if(hmi_object_id  < HMI_DYN_IMAGELIST_SXY_MAX_ID)
	    {
			hmi_object_index = HMI_GET_DYN_IMAGELIST_SXY_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_SXY_IMAGELIST_NUMBER)
			{
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
									(&hmi_sxy_imagelist_rect[hmi_object_index]),
									(&hmi_node_screen_rect));
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
									(HMI_RECT_STR CONST *)pcliped_farther_rect,
									&hmi_insect_rect);
			}
			if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id)!=0)
		 	{								
				hmi_set_dirty_zone_layer((HMI_RECT_STR *)(&hmi_insect_rect),pdirty_rect,depth);
				/*hmi_page_redrawed_flag = TRUE;*/
				hmi_object_dirty_flag=TRUE;
			}
			else
			{				
				if(hmi_object_index < HMI_SXY_IMAGELIST_NUMBER)
				{
					hmi_object_dirty_flag = hmi_engine_check_dynamic_object_changed(
										&hmi_sxy_imagelist_container_table[hmi_object_index],
										(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),									
										pdirty_rect,
										depth,
										&hmi_insect_rect);
				}
			}
		}
		else
		#endif
		#if HMI_DXY_SCROLLBAR_NUMBER>0
		if(hmi_object_id  < HMI_DYN_SCROLLBAR_DXY_MAX_ID)
	    {
			hmi_object_index = HMI_GET_DYN_SCROLLBAR_DXY_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DXY_SCROLLBAR_NUMBER)
			{
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
							(&hmi_dxy_scrollbar_rect[hmi_object_index]),
							(&hmi_node_screen_rect));
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
					(HMI_RECT_STR CONST *)pcliped_farther_rect,&hmi_insect_rect);
			}
			if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
		 	{
				hmi_set_dirty_zone_layer((HMI_RECT_STR *)pcliped_farther_rect/*dxy node dirty zone is father zone*/,
										pdirty_rect,depth);	
				hmi_page_redrawed_flag = TRUE;
				hmi_object_dirty_flag = TRUE;
			}
			else
			{				
				if(hmi_object_index < HMI_DXY_SCROLLBAR_NUMBER)
				{
					hmi_object_dirty_flag=hmi_engine_check_dynamic_object_changed(
										&hmi_dxy_scrollbar_container_table[hmi_object_index],
										(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),										
										pdirty_rect,
										depth,
										&hmi_insect_rect);
				}
			}
		}
		else
		#endif
		#if HMI_SXY_SCROLLBAR_NUMBER>0
		if(hmi_object_id  < HMI_DYN_SCROLLBAR_SXY_MAX_ID)
	    {
			hmi_object_index = HMI_GET_DYN_SCROLLBAR_SXY_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_SXY_SCROLLBAR_NUMBER)
			{
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
					(&hmi_sxy_scrollbar_rect[hmi_object_index]),
					(&hmi_node_screen_rect));
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
					(HMI_RECT_STR CONST *)pcliped_farther_rect,&hmi_insect_rect);
			}
			if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
		 	{
				hmi_set_dirty_zone_layer(&hmi_insect_rect,pdirty_rect,depth);	
				/*hmi_page_redrawed_flag = TRUE;*/
				hmi_object_dirty_flag=TRUE;
			}
			else
			{
				if(hmi_object_index < HMI_SXY_SCROLLBAR_NUMBER)
				{
					hmi_object_dirty_flag=hmi_engine_check_dynamic_object_changed(
						&hmi_sxy_scrollbar_container_table[hmi_object_index],
						(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),						
						pdirty_rect,
						depth,
						&hmi_insect_rect);
				}
			}
		}
		else
		#endif
		#if HMI_DXY_BUTTON_NUMBER>0
		if(hmi_object_id  < HMI_DYN_BUTTON_DXY_MAX_ID)
	    {
			hmi_object_index = HMI_GET_DYN_BUTTON_DXY_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DXY_BUTTON_NUMBER)
			{
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
					(&hmi_dxy_button_rect[hmi_object_id]),
					(&hmi_node_screen_rect));
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
					(HMI_RECT_STR CONST *)pcliped_farther_rect,
					&hmi_insect_rect);
			}
			if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
		 	{
				hmi_set_dirty_zone_layer((HMI_RECT_STR *)pcliped_farther_rect/*dxy button dirty zone is father zone*/,
										pdirty_rect,depth);
				hmi_page_redrawed_flag = TRUE;
				hmi_object_dirty_flag = TRUE;
			}
			else
			{		
				if(hmi_object_index < HMI_DXY_BUTTON_NUMBER)
				{
					hmi_object_dirty_flag=hmi_engine_check_dynamic_object_changed(
								&hmi_dxy_button_container_table[hmi_object_index],
								(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
								pdirty_rect,
								depth,
								&hmi_insect_rect);
				}
			}
		}
		else
		#endif
		#if HMI_SXY_BUTTON_NUMBER>0
		if(hmi_object_id  < HMI_DYN_BUTTON_SXY_MAX_ID)
	    {
			hmi_object_index = HMI_GET_DYN_BUTTON_SXY_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_SXY_BUTTON_NUMBER)
			{
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
						(&hmi_sxy_button_rect[hmi_object_index]),
						(&hmi_node_screen_rect));
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
					(HMI_RECT_STR CONST *)pcliped_farther_rect,&hmi_insect_rect);
			}
			if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
		 	{
				hmi_set_dirty_zone_layer(&hmi_insect_rect,pdirty_rect,depth);	
				/*hmi_page_redrawed_flag = TRUE;*/
				hmi_object_dirty_flag=TRUE;
			}
			else
			{
				if(hmi_object_index < HMI_SXY_BUTTON_NUMBER)
				{
					hmi_object_dirty_flag=hmi_engine_check_dynamic_object_changed(
						&hmi_sxy_button_container_table[hmi_object_index],
						(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
						pdirty_rect,
						depth,
						&hmi_insect_rect);
				}
			}
		}
		else
		#endif
	   #if HMI_DYN_EDIT_TEXTS_NUMBER/*editable text*/ > 0 
	    if(hmi_object_id  < HMI_DYN_TEXTS_MAX_ID)
		{		
			#if HMI_DYN_XY_EDIT_TEXTS_NUMBER > 0
			if(HMI_IS_DYN_TEXT_DXY(hmi_object_id))
			{
				if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
				{
					hmi_set_dirty_zone_layer((HMI_RECT_STR *)pcliped_farther_rect/*dxy text dirty zone is father zone*/,
						pdirty_rect,depth);	 
					hmi_page_redrawed_flag = TRUE;
					hmi_object_dirty_flag = TRUE;
				}
				else
				{
					hmi_object_index = HMI_GET_DYN_TEXTS_DXY_POS_INDEX(hmi_object_id);
					if(hmi_object_index < HMI_DYN_XY_EDIT_TEXTS_NUMBER)
					{
						HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
							(&(hmi_dyn_xy_edit_text_prop_table[hmi_object_index].text_rect)),
							(&hmi_node_screen_rect));
						hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
							(HMI_RECT_STR CONST *)pcliped_farther_rect,&hmi_insect_rect);
						hmi_object_index = HMI_GET_DYN_TEXTS_INDEX(hmi_object_id);/*all editable text container info at one array*/
						if(hmi_object_index < HMI_DYN_EDIT_TEXTS_NUMBER)
						{
							hmi_object_dirty_flag=hmi_engine_check_dynamic_object_changed(
											&hmi_edit_text_container_table[hmi_object_index],
											(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
											pdirty_rect,
											depth,&hmi_insect_rect);
						}

					}
				}
				
			}
			else
			#endif
			#if HMI_S_XY_EDIT_TEXTS_NUMBER > 0
			if(HMI_IS_DYN_TEXT_SXY(hmi_object_id))
			{
				#if HMI_S_XY_EDIT_TEXTS_NUMBER > 0	
				hmi_object_index = HMI_GET_DYN_TEXTS_SXY_POS_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_S_XY_EDIT_TEXTS_NUMBER)
				{
					HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
						(&(hmi_static_xy_edit_text_prop_table[hmi_object_index].text_rect)),
						(&hmi_node_screen_rect));
					hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
						(HMI_RECT_STR CONST *)pcliped_farther_rect,
						&hmi_insect_rect);
				}	
				#endif
				if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
				{
					hmi_set_dirty_zone_layer(&hmi_insect_rect,pdirty_rect,depth);	
					/*hmi_page_redrawed_flag = TRUE;*/
					hmi_object_dirty_flag = TRUE;
				}
				else
				{
					#if HMI_DYN_EDIT_TEXTS_NUMBER > 0									
					hmi_object_index = HMI_GET_DYN_TEXTS_INDEX(hmi_object_id);	
					if(hmi_object_index < HMI_DYN_EDIT_TEXTS_NUMBER)
					{
						hmi_object_dirty_flag=hmi_engine_check_dynamic_object_changed(
							&hmi_edit_text_container_table[hmi_object_index],
							(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
							pdirty_rect,depth,
							&hmi_insect_rect);
					}
					#endif
				}
			}
			else
			#endif
			{
			}			 			
		}
	    else
	   #endif
	   #if HMI_DYN_CONTAINERS_NUMBER/*dyn container*/ > 0 
		if(hmi_object_id  < HMI_DYN_CONTAINERS_MAX_ID)
		{
			#if HMI_ALL_STATIC_CONTAINERS_NUMBER > 0  
			if(hmi_engine_check_container_changed(hmi_object_id,
				(HMI_RECT_STR CONST *)pfarther_rect,
				pdirty_rect,depth,pcliped_farther_rect) != FALSE)
			{
				hmi_page_redrawed_flag	= TRUE;
				hmi_object_dirty_flag	= TRUE;
			}
		    #endif
		  }
		else
	   #endif
	   #if HMI_DYN_FILL_PAGES_NUMBER > 0
		if(hmi_object_id  < HMI_DYN_NFILL_MAX_ID)
		{
			hmi_object_index = HMI_GET_DYN_NFILL_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DYN_FILL_PAGES_NUMBER)
			{
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
					(&hmi_fills_dyn_xy_rect[hmi_object_index]),
					(&hmi_node_screen_rect));
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
					(HMI_RECT_STR CONST *)pcliped_farther_rect,
					&hmi_insect_rect);
			}
			if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
		 	{
				hmi_set_dirty_zone_layer((HMI_RECT_STR *)pcliped_farther_rect/*dxy fill dirty zone is father zone*/,
										pdirty_rect,depth);	
				hmi_page_redrawed_flag = TRUE;
				hmi_object_dirty_flag = TRUE;
			}
			else
			{
				if(hmi_object_index < HMI_DYN_FILL_PAGES_NUMBER)
				{
					hmi_object_dirty_flag=hmi_engine_check_dynamic_object_changed(
						&hmi_dyn_fill_container_table[hmi_object_index],
						(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
						pdirty_rect,
						depth,&hmi_insect_rect);
				}
			}
		}
		else
	   #endif
	   #if HMI_DYN_GFILL_NUMBER > 0
		if(hmi_object_id  < HMI_DYN_GFILL_MAX_ID)
		{
			hmi_object_index = HMI_GET_DYN_GFILL_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DYN_GFILL_NUMBER)
			{
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
					(&hmi_gradient_dxy_fill_rect[hmi_object_index]),
					(&hmi_node_screen_rect));
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
					(HMI_RECT_STR CONST *)pcliped_farther_rect,&hmi_insect_rect);
			}
			if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
		 	{
				hmi_set_dirty_zone_layer((HMI_RECT_STR *)pcliped_farther_rect/*dxy gfill dirty zone is father zone*/,
										pdirty_rect,depth);
				hmi_page_redrawed_flag	= TRUE;
				hmi_object_dirty_flag	= TRUE;
			}
			else
			{
				if(hmi_object_index < HMI_DYN_GFILL_NUMBER)
				{
					hmi_object_dirty_flag=hmi_engine_check_dynamic_object_changed(
						&hmi_dyn_gfill_container_table[hmi_object_index],
						(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
						pdirty_rect,
						depth,&hmi_insect_rect);
				}
			}
		}
		else
	   #endif
	   #if HMI_DXY_CONTAINERS_NUMBER > 0U 
		if(hmi_object_id < HMI_DYN_XY_CONTAINER_MAX_ID)
		{
			if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
			{
				hmi_set_dirty_zone_layer((HMI_RECT_STR *)pcliped_farther_rect/*dxy gfill dirty zone is father zone*/,
										pdirty_rect,depth);
				hmi_page_redrawed_flag	= TRUE;
				hmi_object_dirty_flag	= TRUE;
			}
			else
			{
				if(hmi_engine_check_container_changed(hmi_object_id,
					(HMI_RECT_STR CONST *)pfarther_rect,pdirty_rect,depth,pcliped_farther_rect) != FALSE)
				{
					hmi_object_dirty_flag  = TRUE;
				}
			}
		}
	    else
	   #endif
	   #if HMI_DXY_BITMAPS_NUMBER > 0
	   	if(hmi_object_id < HMI_DYN_XY_BITMAP_MAX_ID)
	   	{
			hmi_object_index = HMI_GET_DYN_XY_BITMAP_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_DXY_BITMAPS_NUMBER)
			{
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
					(&hmi_bmp_dyn_xy_rect[hmi_object_index]),
					(&hmi_node_screen_rect));
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
					(HMI_RECT_STR CONST *)(pcliped_farther_rect),&hmi_insect_rect);
			}
			if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
			{
				hmi_set_dirty_zone_layer((HMI_RECT_STR *)pcliped_farther_rect/*dxy image dirty zone is father zone*/,
							pdirty_rect,depth);	
				hmi_page_redrawed_flag	= TRUE;
				hmi_object_dirty_flag	= TRUE;
			}
			else
			{				
			    if(hmi_object_index < HMI_DXY_BITMAPS_NUMBER)
			    {					
					hmi_object_dirty_flag = hmi_engine_check_dynamic_object_changed(
						&hmi_dxy_bitmap_container_table[hmi_object_index],
						(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
						pdirty_rect,
						depth,&hmi_insect_rect);
			    }
			}
	   	}
		else
	   #endif
		#if HMI_STATIC_TEXTS_NUMBER  > 0 
	    if(hmi_object_id < HMI_STATIC_TEXTS_MAX_ID/*unedit text*/)
		{
			#if HMI_UNEDIT_TEXTS_DYN_XY_NUMBER>0
			if(HMI_IS_UNEDIT_TEXT_DYN_XY(hmi_object_id))
			{
				hmi_object_index  = HMI_GET_DYN_XY_UNEDIT_POS_COLOR_INDEX(hmi_object_index);
				if(hmi_object_index < HMI_UNEDIT_TEXTS_DYN_XY_NUMBER)
				{
					HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
						(&(hmi_dyn_xy_unedit_text_prop_table[hmi_object_id].text_rect)),
						(&hmi_node_screen_rect));
					hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
						(HMI_RECT_STR CONST *)pcliped_farther_rect,
						&hmi_insect_rect);
				}
				if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
				{
					hmi_set_dirty_zone_layer((HMI_RECT_STR *)pcliped_farther_rect/*dxy text dirty zone is father zone*/,
										pdirty_rect,depth);	
					hmi_page_redrawed_flag	= TRUE;
					hmi_object_dirty_flag	= TRUE;
				}
				else
				{
					hmi_object_index = HMI_GET_UNEDIT_INDEX(hmi_object_id);/*all uneditable text contain at one array*/
					if(hmi_object_index < HMI_UNEDIT_TEXTS_DYN_XY_NUMBER)
					{
						hmi_object_dirty_flag = hmi_engine_check_dynamic_object_changed(
							&hmi_static_text_container_table[hmi_object_index],
							(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
							pdirty_rect,
							depth,&hmi_insect_rect);
					}
				}
			}
			else
			#endif
			#if HMI_UNEDIT_TEXTS_STATIC_XY_NUMBER > 0
			if(HMI_IS_UNEDIT_TEXT_STATIC_XY(hmi_object_id))
			{
				hmi_object_index = HMI_GET_S_XY_UNEDIT_POS_COLOR_INDEX(hmi_object_id);
				if(hmi_object_index < HMI_UNEDIT_TEXTS_STATIC_XY_NUMBER)
				{
					HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
									(&hmi_static_xy_unedit_text_prop_table[hmi_object_index].text_rect),
									(&hmi_node_screen_rect));
					hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
						(HMI_RECT_STR CONST *)pcliped_farther_rect,
						&hmi_insect_rect);
				}
				if(HMI_IS_UNEDIT_TEXT_STATIC_XY_DYN_FONT(hmi_object_id))
				{
					if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_object_id) != 0)
					{
						hmi_set_dirty_zone_layer(&hmi_insect_rect/*dfont image dirty zone is self zone*/,
									pdirty_rect,depth);	
						/*hmi_page_redrawed_flag	= TRUE;*/
						hmi_object_dirty_flag	= TRUE;
					}
					else
					{
						hmi_object_index = HMI_GET_UNEDIT_INDEX(hmi_object_id);/*all uneditable text contain at one array*/
						if(hmi_object_index < HMI_STATIC_TEXTS_NUMBER)
						{
							hmi_object_dirty_flag=hmi_engine_check_dynamic_object_changed(
								&hmi_static_text_container_table[hmi_object_index],
								(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
								pdirty_rect,
								depth,&hmi_insect_rect);
						}
					}
				}
				else
				{
					hmi_object_index = HMI_GET_UNEDIT_INDEX(hmi_object_id);/*all uneditable text contain at one array*/
					if(hmi_object_index < HMI_STATIC_TEXTS_NUMBER)
					{
						hmi_object_dirty_flag=hmi_engine_check_dynamic_object_changed(
							&hmi_static_text_container_table[hmi_object_index],
							(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
							pdirty_rect,
							depth,&hmi_insect_rect);
					}
				}
			}
			else
			#endif
			{
			}						
		}
	    else	
		#endif
	   #if HMI_STATIC_FILL_PAGES_NUMBER > 0
		if(HMI_IS_SXY_FILL_PAGES(hmi_object_id))
		{
			hmi_object_index  = HMI_GET_SXY_FILL_PAGES_ID_INDEX(hmi_object_id );
			if(hmi_object_index<HMI_STATIC_FILL_PAGES_NUMBER)
			{
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
					(&hmi_fills_static_xy_rect[hmi_object_index]),
					(&hmi_node_screen_rect));
			}
			hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
				(HMI_RECT_STR CONST *)pcliped_farther_rect,&hmi_insect_rect);
			if(hmi_object_index < HMI_STATIC_FILL_PAGES_NUMBER)
			{
				hmi_object_dirty_flag = hmi_engine_check_dynamic_object_changed(
					&hmi_static_fill_container_table[hmi_object_index],
					(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
					pdirty_rect,
					depth,&hmi_insect_rect);
			}
		}
		else
	   #endif
	   #if HMI_STATIC_GFILL_NUMBER> 0
		if(HMI_IS_SXY_GFILL_PAGES(hmi_object_id))
		{
			hmi_object_index = HMI_GET_SXY_GFILL_PAGES_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_STATIC_GFILL_NUMBER)
			{
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
							(&hmi_gradient_sxy_fill_rect[hmi_object_index]),
							(&hmi_node_screen_rect));
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
					(HMI_RECT_STR CONST *)pcliped_farther_rect,
					&hmi_insect_rect);
				hmi_object_dirty_flag = hmi_engine_check_dynamic_object_changed(
					&hmi_static_gfill_container_table[hmi_object_index],
					(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
					pdirty_rect,
					depth,&hmi_insect_rect);
			}
		}
		else
	   #endif
	   #if HMI_SXY_BITMAPS_NUMBER> 0
		if(HMI_IS_S_XY_BITMAP(hmi_object_id))
		{
			hmi_object_index  = HMI_GET_SXY_BITMAPS_ID_INDEX(hmi_object_id);
			if(hmi_object_index < HMI_SXY_BITMAPS_NUMBER)
			{
				HMI_GET_OBJECT_SCREEN_COOR(pfarther_rect,
					(&hmi_bmp_static_xy_rect[hmi_object_index]),
					(&hmi_node_screen_rect));
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
					(HMI_RECT_STR CONST *)pcliped_farther_rect,
					&hmi_insect_rect);
				hmi_object_dirty_flag = hmi_engine_check_dynamic_object_changed(
							&hmi_sxy_bitmap_container_table[hmi_object_index],
							(HMI_RECT_STR CONST *)(&hmi_node_screen_rect),
							pdirty_rect,
							depth,&hmi_insect_rect);
			}
		}
		else
	   #endif
	   #if HMI_SXY_CONTAINERS_NUMBER > 0
		if(HMI_IS_CONTAINERS_SXY(hmi_object_id))
		{
			if(hmi_engine_check_container_changed(hmi_object_id,
				(HMI_RECT_STR CONST *)pfarther_rect,
				pdirty_rect,depth,pcliped_farther_rect) != FALSE)
			{
				hmi_object_dirty_flag = TRUE;
			}
		}
		else
	   #endif	   
		{
		}
		hmi_number_object--;	
		hmi_check_index++;
	}
		
   //return	hmi_object_dirty_flag;
}
#endif

#if 0
UINT8 hmi_engine_disp_flag_element_page()
{	
	BOOLEAN						is_video_page = FALSE;	
	BOOLEAN						dxy_page = FALSE;
	HMI_RECT_STR				hmi_screen_rect={0,0,HMI_MAX_WIDTH,HMI_MAX_HEIGHT};
	HMI_RECT_STR CONST			*phmi_page_rect = NULL;
	#if HMI_LAYER_0_HIGHEST_PRIORITY> 1U
	UINT8                 		hmi_priority_cnt = 0U;
	UINT8                 		hmi_layer_highest_active_priority = 0U;
	#endif
	UINT8                 		hmi_object_index = 0U;
	HMI_LAYER_TABLE_STR CONST	*phmi_active_layer_info = &hmi_layer_table[0];
	
	HMI_PAGE_TABLE_STR CONST * phmi_page_info = NULL;	
	HMI_PAGE_ID_STR hmi_page_id		= 0;
		
	#if HMI_LAYER_0_HIGHEST_PRIORITY > 1
	{
		hmi_layer_highest_active_priority = hmi_highest_active_page_priority;
	}
	if(hmi_layer_highest_active_priority < HMI_LAYER_0_HIGHEST_PRIORITY)
	#endif
	{				
		{			
			#if HMI_LAYER_0_HIGHEST_PRIORITY > 1  
			/*Search all priority page table.*/			
			hmi_layer_highest_active_priority=HMI_LAYER_0_HIGHEST_PRIORITY-1;			
			for(hmi_priority_cnt=0U; hmi_priority_cnt <= hmi_layer_highest_active_priority; hmi_priority_cnt++)
			#endif
			{
				#if HMI_LAYER_0_HIGHEST_PRIORITY > 1
				hmi_page_id		= hmi_layer_0_active_page_id[hmi_priority_cnt].new_page;				
				#else
				hmi_page_id		= hmi_layer_0_active_page_id[0].new_page;				
				#endif				
				#if HMI_DXY_PAGES_NUMBER>0U	/*Get current dxy page info*/
				if(HMI_IS_DXY_PAGE(hmi_page_id))
				{
					dxy_page=TRUE;
					if(HMI_IS_VIDEO_DXY_PAGE(hmi_page_id))
					{
						is_video_page=TRUE;
						phmi_page_info=NULL;												
					}
					else
					{
						hmi_object_index=HMI_GET_PAGE_DXY_ID_INDEX(hmi_page_id);
						if(hmi_object_index<HMI_DXY_PAGES_NUMBER)
						{							
							phmi_page_rect = &hmi_dxy_page_rect[hmi_object_index];/*&hmi_screen_rect;*//*dxy page need refresh all screen*/
							phmi_page_info = &hmi_dxy_page_table[hmi_object_index];
						}
					}					
				}
				else
				#endif
				#if HMI_SXY_PAGES_NUMBER>0U	/*Get current Sxy page info*/
				if(HMI_IS_SXY_PAGE(hmi_page_id))					
				{
					dxy_page = FALSE;
					if(HMI_IS_VIDEO_SXY_PAGE(hmi_page_id))
					{
						is_video_page = TRUE;
						phmi_page_info = NULL;						
					}
					else
					{
						hmi_object_index=HMI_GET_PAGE_SXY_ID_INDEX(hmi_page_id);
						if(hmi_object_index<HMI_SXY_PAGES_NUMBER)
						{
							phmi_page_rect = &hmi_sxy_page_rect[hmi_object_index];
							phmi_page_info = &hmi_sxy_page_table[hmi_object_index];
						}
					}					
				}
				else
				#endif
				{
				}
				if(hmi_page_id!=HMI_PAGES_NUMBER)
				{
					if((HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_page_id) != 0))					
					{
						HMI_DYNAMIC_OBJECT_CANCEL_CHANGED_FLAG(hmi_page_id); 
						hmi_engine_draw_container((HMI_CONTAINER_STR CONST *)(&(phmi_page_info->container),
												(HMI_RECT_STR CONST *)(&hmi_screen_rect),
												);						
					}
					else	/*search every element refresh flag */
					{
						if((!is_video_page)&&(phmi_page_rect!=NULL))
						{						
							hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_screen_rect),
									(HMI_RECT_STR CONST *)phmi_page_rect,
									&cliped_farther_rect);
							hmi_engine_check_dynamic_object_changed((HMI_CONTAINER_STR CONST *)(&(phmi_page_info->container)),
														(HMI_RECT_STR CONST *)phmi_page_rect,
														pdirty_zone,
														HMI_PAGE_BEGIN_DEPTH/*page node tree depth*/,
														&cliped_farther_rect);
						}
					}
				}
								
				if((hmi_old_page!=HMI_PAGES_NUMBER)	)/*Get old page info*/
				{	
										
					{
						#if HMI_DXY_PAGES_NUMBER>0U					
						if(HMI_IS_DXY_PAGE(hmi_old_page))					
						{
							dxy_page = TRUE;
							if(HMI_IS_VIDEO_DXY_PAGE(hmi_old_page))
							{
								is_video_page=TRUE;
								pold_page_rect=NULL;	
							}
							else
							{
								is_video_page=FALSE;							
								hmi_object_index=HMI_GET_PAGE_DXY_ID_INDEX(hmi_old_page);
								if(hmi_object_index<HMI_DXY_PAGES_NUMBER)
								{
									if((HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_old_page)!=0U))/*set ,refresh flag not render*/
									{
										HMI_DYNAMIC_OBJECT_CANCEL_CHANGED_FLAG(hmi_old_page);
										pold_page_rect=&hmi_screen_rect;
									}
									else
									{
										pold_page_rect=&hmi_dxy_page_rect[hmi_object_index];
									}
								}
							}
						}
						#endif

						#if HMI_SXY_PAGES_NUMBER>0U
						if(HMI_IS_SXY_PAGE(hmi_old_page))					
						{
							dxy_page = FALSE;
							if(HMI_IS_VIDEO_SXY_PAGE(hmi_old_page))
							{
								is_video_page=TRUE;
								pold_page_rect=NULL;
							}
							else
							{
								is_video_page=FALSE;
								hmi_object_index=HMI_GET_PAGE_SXY_ID_INDEX(hmi_old_page);
								if(hmi_object_index<HMI_SXY_PAGES_NUMBER)
								{
									pold_page_rect=&hmi_sxy_page_rect[hmi_object_index];
								}
							}
						}
						#endif
						if(pold_page_rect!=NULL)
						{
							#if HMI_LAYER_0_HIGHEST_PRIORITY > 1U
							if(hmi_priority_cnt!=0)/*only pop up window layer need refresh*/
							{
								#if defined(HMI_GRAPHIC_RGL)
								pop_up_window_layer = get_popup_window_layer();
								#else
								pop_up_window_layer = 0;/*0 layer of GDI*/
								#endif
								hmi_set_dirty_zone_layer((HMI_RECT_STR *)pold_page_rect,pdirty_zone,
														pop_up_window_layer);
							}
							else
							{						
								hmi_set_dirty_zone_layer((HMI_RECT_STR *)pold_page_rect,pdirty_zone,
														HMI_PAGE_ALL_LAYER);/*all layer is dirty*/
							}
							#else
							hmi_set_dirty_zone_layer((HMI_RECT_STR *)pold_page_rect,pdirty_zone,
														HMI_PAGE_ALL_LAYER);/*all layer is dirty*/
							#endif
						}
					}
				}				
			}
		}
	}
	#if defined(HMI_GRAPHIC_RGL)
	if(exist_video_page==TRUE)
	{
		hmi_enable_video_layer(TRUE);
	}
	else
	{
		
	}
	#endif
	return(hmi_driver_woking_status_flag);
}
#endif

#endif



#if 0
UINT8 hmi_engine_get_multi_dirty_zone_page(HMI_RECT_STR *pdirty_zone)
{				
	HMI_RECT_STR CONST			*phmi_page_rect = NULL;	
	#if HMI_LAYER_0_HIGHEST_PRIORITY > 1U	
	UINT8                 		hmi_priority_cnt= 0U;
	UINT8                 		hmi_layer_highest_active_priority = 0U;
	#endif
	UINT8                 		hmi_object_index		= 0U;
	HMI_LAYER_TABLE_STR CONST	*phmi_active_layer_info = &hmi_layer_table[0];
	#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))	
	HMI_RECT_STR				hmi_insec_rect		= {HMI_INVALID_COOR,HMI_INVALID_COOR,0,0};
	//HMI_RECT_STR CONST			*pold_page_rect		= NULL;
	HMI_RECT_STR				hmi_screen_rect		= {0,0,HMI_MAX_WIDTH,HMI_MAX_HEIGHT};	
	HMI_RECT_STR				cliped_farther_rect	= {HMI_INVALID_COOR,HMI_INVALID_COOR,0,0};
	#endif
	HMI_PAGE_TABLE_STR CONST * phmi_page_info	= NULL;	
	HMI_PAGE_ID_STR				hmi_page_id		= 0;
	HMI_PAGE_ID_STR				hmi_old_page	= 0;	
			
	#if HMI_LAYER_0_HIGHEST_PRIORITY > 1
	{
		hmi_layer_highest_active_priority = hmi_highest_active_page_priority;
	}
	if(hmi_layer_highest_active_priority < HMI_LAYER_0_HIGHEST_PRIORITY)
	#endif
	{		
		#if HMI_DYN_LANGUAGE_NUMBER > 0	/*language number >1*/
		if((HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(HMI_LANGUAGE) != 0U))/*if change language setting,all active page need refresh*/
		{				
			hmi_set_dirty_zone_layer(&hmi_screen_rect,
							pdirty_zone,HMI_PAGE_ALL_LAYER);
			HMI_DYNAMIC_OBJECT_CANCEL_CHANGED_FLAG(HMI_LANGUAGE);
		}
		else		
		#endif
		{			
			#if HMI_LAYER_0_HIGHEST_PRIORITY > 1  
			/*Search all priority page table.*/			
			hmi_layer_highest_active_priority	= HMI_LAYER_0_HIGHEST_PRIORITY-1;			
			for(hmi_priority_cnt=0U; hmi_priority_cnt <= hmi_layer_highest_active_priority; hmi_priority_cnt++)
			#endif
			{
				#if HMI_LAYER_0_HIGHEST_PRIORITY > 1
				hmi_page_id		= hmi_layer_0_active_page_id[hmi_priority_cnt].new_page;
				hmi_old_page	= hmi_layer_0_active_page_id[hmi_priority_cnt].old_page;
				hmi_layer_0_active_page_id[hmi_priority_cnt].old_page	= HMI_PAGES_NUMBER;
				#else
				hmi_page_id		= hmi_layer_0_active_page_id[0].new_page;
				hmi_old_page	= hmi_layer_0_active_page_id[0].old_page;
				hmi_layer_0_active_page_id[0].old_page	= HMI_PAGES_NUMBER;
				#endif	
				
				if(hmi_page_id != hmi_old_page)/*switch page*/
				{
					#if HMI_DXY_PAGES_NUMBER > 0U	
					if(HMI_IS_DXY_PAGE(hmi_old_page))
					{
						hmi_object_index	= HMI_GET_PAGE_DXY_ID_INDEX(hmi_old_page);
						if(hmi_object_index < HMI_DXY_PAGES_NUMBER)
						{
							phmi_page_rect = &hmi_dxy_page_rect_bck[hmi_object_index];
							hmi_set_dirty_zone_layer(phmi_page_rect,
											pdirty_zone,HMI_PAGE_ALL_LAYER);
						}
					}
					else
					#endif
					#if HMI_SXY_PAGES_NUMBER > 0U	/*Get current static xy page info*/
					if(HMI_IS_SXY_PAGE(hmi_old_page))					
					{
						hmi_object_index	= HMI_GET_PAGE_SXY_ID_INDEX(hmi_old_page);
						if(hmi_object_index < HMI_SXY_PAGES_NUMBER)
						{
							phmi_page_rect=&hmi_sxy_page_rect[hmi_object_index];
							hmi_set_dirty_zone_layer(phmi_page_rect,
											pdirty_zone,HMI_PAGE_ALL_LAYER);
						}
					}else
					#endif
					{
					}					
				}
				#if HMI_DXY_PAGES_NUMBER > 0U
				if(HMI_IS_DXY_PAGE(hmi_page_id))
				{
					if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_page_id) != 0U)
					{
						hmi_object_index	= HMI_GET_PAGE_DXY_ID_INDEX(hmi_page_id);
						if(hmi_object_index < HMI_DXY_PAGES_NUMBER)
						{
							phmi_page_rect = &hmi_dxy_page_rect[hmi_object_index];
							phmi_page_rect	= &hmi_dxy_page_rect_bck[hmi_object_index];
							
							hmi_set_dirty_zone_layer(phmi_page_rect,
										pdirty_zone,HMI_PAGE_ALL_LAYER);							
							hmi_set_dirty_zone_layer(phmi_page_rect,
										pdirty_zone,HMI_PAGE_ALL_LAYER);
						}
					}
					else
					{
						hmi_object_index	= HMI_GET_PAGE_DXY_ID_INDEX(hmi_page_id);
						if(hmi_object_index < HMI_DXY_PAGES_NUMBER)
						{
							phmi_page_rect = &hmi_sxy_page_rect[hmi_object_index];
							phmi_page_info = &hmi_dxy_page_table[hmi_object_index];
							hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_screen_rect),
									(HMI_RECT_STR CONST *)phmi_page_rect,
									&cliped_farther_rect);
						
							hmi_engine_check_dynamic_object_changed((HMI_CONTAINER_STR CONST *)(&(phmi_page_info->container)),
														(HMI_RECT_STR CONST *)phmi_page_rect,
														pdirty_zone,
														HMI_PAGE_BEGIN_DEPTH/*page node tree depth*/,
														&cliped_farther_rect);
						}
						
					}														
				}
				else
				#endif
				#if HMI_SXY_PAGES_NUMBER > 0U	/*Get current static xy page info*/
				if(HMI_IS_SXY_PAGE(hmi_page_id))					
				{
					hmi_object_index	= HMI_GET_PAGE_SXY_ID_INDEX(hmi_page_id);
					if(hmi_object_index < HMI_SXY_PAGES_NUMBER)
					{
						phmi_page_rect=&hmi_sxy_page_rect[hmi_object_index];
						hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_screen_rect),
										(HMI_RECT_STR CONST *)phmi_page_rect,
										&cliped_farther_rect);
						phmi_page_info = &hmi_sxy_page_table[hmi_object_index];
						hmi_engine_check_dynamic_object_changed((HMI_CONTAINER_STR CONST *)(&(phmi_page_info->container)),
											(HMI_RECT_STR CONST *)phmi_page_rect,
											pdirty_zone,
											HMI_PAGE_BEGIN_DEPTH/*page node tree depth*/,
											&cliped_farther_rect);
					}											
				}
				else
				#endif
				{
				}																			
			}
		}
	}
	
	return(hmi_driver_woking_status_flag);
}

#endif

#if (defined(HMI_GRAPHIC_AGG)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))	
UINT8 hmi_engine_get_multi_dirty_zone_page(HMI_RECT_STR *pdirty_zone)
{				
	HMI_RECT_STR 				hmi_page_rect = {HMI_INVALID_COOR,HMI_INVALID_COOR,0,0};	
	#if HMI_LAYER_0_HIGHEST_PRIORITY > 1U	
	UINT8                 		hmi_priority_cnt= 0U;
	UINT8                 		hmi_layer_highest_active_priority = 0U;
	UINT8						hmi_layer_highest_prior			=0U;
	#endif
	UINT8                 		hmi_object_index		= 0U;	
	#if (defined(HMI_GRAPHIC_AGG)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL)||defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))			
	HMI_RECT_STR				hmi_screen_rect		= {0,0,HMI_MAX_WIDTH,HMI_MAX_HEIGHT};	
	HMI_RECT_STR				cliped_farther_rect	= {HMI_INVALID_COOR,HMI_INVALID_COOR,0,0};
	#endif
	HMI_PAGE_TABLE_STR CONST * phmi_page_info	= NULL;	
	HMI_PAGE_ID_STR				hmi_page_id		= 0;
	HMI_PAGE_ID_STR				hmi_old_page	= 0;	
	UINT8						hmi_screen_id	= 0U;
	HMI_PRIOR_PAGE_STR			*phmi_active_page_id = NULL;
	
	hmi_screen_id =hmi_driver_get_render_screen();
	if(hmi_screen_id == HMI_LAYER_SCREEN0)
	{
		phmi_active_page_id = hmi_layer_0_active_page_id;
	}
	
#if HMI_ALL_LAYERS_NUMBER > 1
	else if(hmi_screen_id == HMI_LAYER_SCREEN1)
	{
		
		phmi_active_page_id = hmi_layer_1_active_page_id;
	}
	else
	{
		phmi_active_page_id = hmi_layer_0_active_page_id;
	}
#endif
	#if (HMI_LAYER_0_HIGHEST_PRIORITY > 1)||(HMI_LAYER_1_HIGHEST_PRIORITY > 1)
	if(hmi_screen_id == HMI_LAYER_SCREEN1)
	{
		hmi_layer_highest_prior = HMI_LAYER_1_HIGHEST_PRIORITY;
	}
	else
	{
		hmi_layer_highest_prior = HMI_LAYER_0_HIGHEST_PRIORITY;
	}
	{
		hmi_layer_highest_active_priority = hmi_highest_active_page_priority[hmi_screen_id];
	}
	if(hmi_layer_highest_active_priority < hmi_layer_highest_prior)
	#endif
	{	
		if(phmi_active_page_id != NULL)
		{
		#if HMI_DYN_LANGUAGE_NUMBER > 0	/*language number >1*/
		if((HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(HMI_LANGUAGE) != 0U))/*if change language setting,all active page need refresh*/
		{				
			hmi_set_dirty_zone_layer(&hmi_screen_rect,
							pdirty_zone,HMI_PAGE_ALL_LAYER);
			HMI_DYNAMIC_OBJECT_CANCEL_CHANGED_FLAG(HMI_LANGUAGE);
		}
		else		
		#endif
		{			
			#if (HMI_LAYER_0_HIGHEST_PRIORITY > 1)||(HMI_LAYER_1_HIGHEST_PRIORITY > 1)
			/*Search all priority page table.*/			
			hmi_layer_highest_active_priority	= HMI_LAYER_0_HIGHEST_PRIORITY-1;			
			for(hmi_priority_cnt=0U; hmi_priority_cnt <= hmi_layer_highest_active_priority; hmi_priority_cnt++)
			#endif
			{
				#if (HMI_LAYER_0_HIGHEST_PRIORITY > 1)||(HMI_LAYER_1_HIGHEST_PRIORITY > 1)
				hmi_page_id		= phmi_active_page_id[hmi_priority_cnt].new_page;
				hmi_old_page	= phmi_active_page_id[hmi_priority_cnt].old_page;
				phmi_active_page_id[hmi_priority_cnt].old_page	= HMI_PAGES_NUMBER;
				#else
				hmi_page_id		= phmi_active_page_id[0].new_page;
				hmi_old_page	= phmi_active_page_id[0].old_page;
				phmi_active_page_id[0].old_page	= HMI_PAGES_NUMBER;
				#endif	
				
				if((hmi_page_id != hmi_old_page) && (hmi_old_page != HMI_PAGES_NUMBER))/*switch page*/
				{
					#if HMI_DXY_PAGES_NUMBER > 0U	
					if(HMI_IS_DXY_PAGE(hmi_old_page))
					{
						hmi_object_index	= HMI_GET_PAGE_DXY_ID_INDEX(hmi_old_page);
						if(hmi_object_index < HMI_DXY_PAGES_NUMBER)
						{
							hmi_page_rect.x = hmi_dxy_page_rect_bck[hmi_object_index].x;
							hmi_page_rect.y = hmi_dxy_page_rect_bck[hmi_object_index].y;
							hmi_page_rect.w = hmi_dxy_page_rect_bck[hmi_object_index].w;
							hmi_page_rect.h = hmi_dxy_page_rect_bck[hmi_object_index].h;
							hmi_set_dirty_zone_layer((HMI_RECT_STR *)(&hmi_page_rect),
											pdirty_zone,HMI_PAGE_ALL_LAYER);
						}
					}
					else
					#endif
					#if HMI_SXY_PAGES_NUMBER > 0U	/*Get current static xy page info*/
					if(HMI_IS_SXY_PAGE(hmi_old_page))					
					{
						hmi_object_index	= HMI_GET_PAGE_SXY_ID_INDEX(hmi_old_page);
						if(hmi_object_index < HMI_SXY_PAGES_NUMBER)
						{
							hmi_page_rect.x = hmi_sxy_page_rect[hmi_object_index].x;
							hmi_page_rect.y = hmi_sxy_page_rect[hmi_object_index].y;
							hmi_page_rect.w = hmi_sxy_page_rect[hmi_object_index].w;
							hmi_page_rect.h = hmi_sxy_page_rect[hmi_object_index].h;
							hmi_set_dirty_zone_layer((HMI_RECT_STR *)(&hmi_page_rect),
											pdirty_zone,HMI_PAGE_ALL_LAYER);
						}
					}else
					#endif
					{
					}					
				}
				#if HMI_DXY_PAGES_NUMBER > 0U
				if(HMI_IS_DXY_PAGE(hmi_page_id))
				{
					if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_page_id) != 0U)
					{
						hmi_object_index	= HMI_GET_PAGE_DXY_ID_INDEX(hmi_page_id);
						if(hmi_object_index < HMI_DXY_PAGES_NUMBER)
						{
							hmi_page_rect.x = hmi_dxy_page_rect[hmi_object_index].x;
							hmi_page_rect.y = hmi_dxy_page_rect[hmi_object_index].y;
							hmi_page_rect.w = hmi_dxy_page_rect[hmi_object_index].w;
							hmi_page_rect.h = hmi_dxy_page_rect[hmi_object_index].h;
							hmi_set_dirty_zone_layer((HMI_RECT_STR *)(&hmi_page_rect),
										pdirty_zone,HMI_PAGE_ALL_LAYER);
							hmi_page_rect.x = hmi_dxy_page_rect_bck[hmi_object_index].x;
							hmi_page_rect.y = hmi_dxy_page_rect_bck[hmi_object_index].y;
							hmi_page_rect.w = hmi_dxy_page_rect_bck[hmi_object_index].w;
							hmi_page_rect.h = hmi_dxy_page_rect_bck[hmi_object_index].h;				
							hmi_set_dirty_zone_layer((HMI_RECT_STR *)(&hmi_page_rect),
										pdirty_zone,HMI_PAGE_ALL_LAYER);
						}
					}
					else
					{
						hmi_object_index	= HMI_GET_PAGE_DXY_ID_INDEX(hmi_page_id);
						if(hmi_object_index < HMI_DXY_PAGES_NUMBER)
						{
							phmi_page_info = &hmi_dxy_page_table[hmi_object_index];
							hmi_page_rect.x = hmi_dxy_page_rect[hmi_object_index].x;
							hmi_page_rect.y = hmi_dxy_page_rect[hmi_object_index].y;
							hmi_page_rect.w = hmi_dxy_page_rect[hmi_object_index].w;
							hmi_page_rect.h = hmi_dxy_page_rect[hmi_object_index].h;
							hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_screen_rect),
									(HMI_RECT_STR CONST *)(&hmi_page_rect),
									&cliped_farther_rect);
						
							hmi_engine_check_dynamic_object_changed((HMI_CONTAINER_STR CONST *)(&(phmi_page_info->container)),
														(HMI_RECT_STR CONST *)(&hmi_page_rect),
														pdirty_zone,
														HMI_PAGE_BEGIN_DEPTH/*page node tree depth*/,
														&cliped_farther_rect);
						}
						
					}														
				}
				else
				#endif
				#if HMI_SXY_PAGES_NUMBER > 0U	/*Get current static xy page info*/
				if(HMI_IS_SXY_PAGE(hmi_page_id))					
				{
					if(HMI_DYNAMIC_OBJECT_GET_CHANGED_FLAG(hmi_page_id) != 0U)
					{
						hmi_object_index	= HMI_GET_PAGE_SXY_ID_INDEX(hmi_page_id);
						if(hmi_object_index < HMI_SXY_PAGES_NUMBER)
						{
							hmi_page_rect.x = hmi_sxy_page_rect[hmi_object_index].x;
							hmi_page_rect.y = hmi_sxy_page_rect[hmi_object_index].y;
							hmi_page_rect.w = hmi_sxy_page_rect[hmi_object_index].w;
							hmi_page_rect.h = hmi_sxy_page_rect[hmi_object_index].h;
							hmi_set_dirty_zone_layer((HMI_RECT_STR *)(&hmi_page_rect),
										pdirty_zone,HMI_PAGE_ALL_LAYER);							
						}
					}
					else
					{										
						hmi_object_index	= HMI_GET_PAGE_SXY_ID_INDEX(hmi_page_id);
						if(hmi_object_index < HMI_SXY_PAGES_NUMBER)
						{
							phmi_page_info= &hmi_sxy_page_table[hmi_object_index];
							hmi_page_rect.x = hmi_sxy_page_rect[hmi_object_index].x;
							hmi_page_rect.y = hmi_sxy_page_rect[hmi_object_index].y;
							hmi_page_rect.w = hmi_sxy_page_rect[hmi_object_index].w;
							hmi_page_rect.h = hmi_sxy_page_rect[hmi_object_index].h;
							hmi_get_intersec_rect((HMI_RECT_STR CONST *)(&hmi_screen_rect),
											(HMI_RECT_STR CONST *)(&hmi_page_rect),
											&cliped_farther_rect);
							phmi_page_info = &hmi_sxy_page_table[hmi_object_index];
							hmi_engine_check_dynamic_object_changed((HMI_CONTAINER_STR CONST *)(&(phmi_page_info->container)),
												(HMI_RECT_STR CONST *)(&hmi_page_rect),
												pdirty_zone,
												HMI_PAGE_BEGIN_DEPTH/*page node tree depth*/,
												&cliped_farther_rect);
						}	
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
	
	return(hmi_driver_woking_status_flag);
}
#endif

void	hmi_union_rect(HMI_RECT_STR *prect1,HMI_RECT_STR *prect2,HMI_RECT_STR *prect_union)
{
	HMI_WIDTH_STR	min_x	= 0;
	HMI_WIDTH_STR	max_x	= 0;
	HMI_HEIGHT_STR	min_y	= 0;
	HMI_HEIGHT_STR	max_y	= 0;
	
	if((prect1 != NULL) && (prect2 != NULL) && (prect_union != NULL))
	{
		if((prect1->w != 0)&&(prect1->h != 0))
		{
			if((prect2->w != 0)&&((prect2->h != 0)))
			{
				if(prect1->x < prect2->x)
				{
					min_x	= prect1->x;
				}
				else
				{
					min_x	= prect2->x;
				}

				if((prect1->x + prect1->w) < (prect2->x + prect2->w))
				{
					max_x	= prect2->x + prect2->w;
				}
				else
				{
					max_x	= prect1->x + prect1->w;
				}

				if(prect1->y < prect2->y)
				{
					min_y	= prect1->y;
				}
				else
				{
					min_y	= prect2->y;
				}

				if((prect1->y + prect1->h) < (prect2->y + prect2->h))
				{
					max_y	= prect2->y + prect2->h;
				}
				else
				{
					max_y	= prect1->y + prect1->h;
				}

				prect_union->x	= min_x;
				prect_union->y	= min_y;
				prect_union->w	= max_x - min_x;
				prect_union->h	= max_y - min_y;
			}
			else
			{
				prect_union->x	= prect1->x;
				prect_union->y	= prect1->y;
				prect_union->w	= prect1->w;
				prect_union->h	= prect1->h;
			}
		}
		else
		{
			if((prect2->w != 0)&&((prect2->h != 0)))
			{
				prect_union->x	= prect2->x;
				prect_union->y	= prect2->y;
				prect_union->w	= prect2->w;
				prect_union->h	= prect2->h;
			}
		}
	}
}

BOOLEAN	hmi_intersection_rect(HMI_RECT_STR *prect1,HMI_RECT_STR *prect2)
{
	BOOLEAN	intersection=TRUE;
	
	if((prect1 != NULL) && (prect2 != NULL))
	{
		if((prect1->x + prect1->w) < prect2->x)
		{
			intersection	= FALSE;
		}
		else
		{
			if(prect1->x > (prect2->x + prect2->w))
			{
				intersection	= FALSE;
			}
			else
			{
				if((prect1->y + prect1->h) < prect2->y)
				{
					intersection	= FALSE;
				}
				else
				{
					if(prect1->y > (prect2->y + prect2->h))
					{
						intersection	= FALSE;
					}
					else
					{
						intersection=TRUE;
					}
				}
			}
		}
	}
	return	intersection;
}

UINT32	hmi_get_area_rect(HMI_RECT_STR *prect1,HMI_RECT_STR *prect2)
{
	UINT32	area	= 0;
	
	if((prect1 != NULL) && (prect2 != NULL))
	{
		area	= (prect1->w * prect1->h) + (prect2->w * prect2->h);
	}
	return	area;
}

void hmi_add_dirty(HMI_RECT_STR *pnew,HMI_RECT_STR *pdirty_list,U08 depth)
{
	UINT16	index	= 0;
	UINT16	start	= 0;
	BOOLEAN	end		= FALSE;
	BOOLEAN	intersec= FALSE;
	BOOLEAN	unionsec= FALSE;
	BOOLEAN	insert	= FALSE;
	UINT32	union_area	= 0;
	UINT8	union_index	= 0;
	#if HMI_INTERSECTION_UNION == HMI_NO
	UINT32	two_area	= 0;
	#endif
	UINT32	min_area	= 0xffffffff;
	UINT32	min_area_index	= 0;
	HMI_RECT_STR union_rect	= {0,0,0,0};	
	
	if((pnew != NULL)&&(pdirty_list != NULL))
	{
		if((pnew->w != 0) && (pnew->h != 0))
		{
			/*union */
			if(depth < HMI_LAYER_MAX_CNT)
			{
				hmi_union_rect(pnew,&pdirty_list[depth],&pdirty_list[depth]);
				start	= HMI_LAYER_MAX_CNT + depth * HMI_DIRTY_FIFO_LEN;
			}
			
			/*add multi dirty zone list*/				
			while(((start + index) < (UINT16)(start + HMI_DIRTY_FIFO_LEN)) && (!end))
			{
				if((pdirty_list[start + index].w != 0) && (pdirty_list[start + index].h != 0))
				{
					intersec	= hmi_intersection_rect(&pdirty_list[start + index],
														pnew);
					if(intersec	== TRUE)
					{	
					#if HMI_INTERSECTION_UNION == HMI_NO
						two_area	= hmi_get_area_rect(&pdirty_list[start + index],
												pnew);
					#endif
						hmi_union_rect(&pdirty_list[start + index],
												pnew,
												&union_rect); 
					#if HMI_INTERSECTION_UNION == HMI_NO
						union_area	= (union_rect.w) * (union_rect.h);
						if(union_area < two_area)
					#endif
						{
							pdirty_list[start + index].x	= union_rect.x;
							pdirty_list[start + index].y	= union_rect.y;
							pdirty_list[start + index].w	= union_rect.w;
							pdirty_list[start + index].h	= union_rect.h;
							union_index	= (UINT8)(start + index);
							unionsec= TRUE;
							end		= TRUE;
						}
												
					}
				}
				else
				{
					pdirty_list[start + index].x	= pnew->x;
					pdirty_list[start + index].y	= pnew->y;
					pdirty_list[start + index].w	= pnew->w;
					pdirty_list[start + index].h	= pnew->h;
					end		= TRUE;
					insert	= TRUE;
				}
				index++;
			}
			if(insert == TRUE)
			{
				if(index < /*start + */HMI_DIRTY_FIFO_LEN)
				{
					pdirty_list[start + index].x	= 0;/*set end flag*/
					pdirty_list[start + index].y	= 0;
					pdirty_list[start + index].w	= 0;
					pdirty_list[start + index].h	= 0;
				}
			}		
			else if(unionsec == TRUE)
			{
				do
				{
					pdirty_list[union_index].x	= pdirty_list[union_index+1].x;
					pdirty_list[union_index].y	= pdirty_list[union_index+1].y;
					pdirty_list[union_index].w	= pdirty_list[union_index+1].w;
					pdirty_list[union_index].h	= pdirty_list[union_index+1].h;
					union_index++;
				}while((pdirty_list[union_index].w != 0)&&
						(union_index < (start + HMI_DIRTY_FIFO_LEN)));/**/
				if(union_index >= (start + HMI_DIRTY_FIFO_LEN))
				{
					pdirty_list[union_index-1].x	= 0;
					pdirty_list[union_index-1].y	= 0;
					pdirty_list[union_index-1].w	= 0;
					pdirty_list[union_index-1].h	= 0;
				}
				hmi_add_dirty(&union_rect,pdirty_list,depth);
			}
			else /*fifo full,not add success*/
			{
				index	= 0;			
				while((start + index < (UINT16)(start + HMI_DIRTY_FIFO_LEN)) && (!end))
				{
					if((pdirty_list[start + index].w != 0) && (pdirty_list[start + index].h != 0))
					{										 
						hmi_union_rect(&pdirty_list[start + index],
												pnew,
												&union_rect);
						union_area	= (union_rect.w) * (union_rect.h);
						if(union_area < min_area)
						{
							min_area_index	= start + index;
							min_area		= union_area;
						}																	
					}
					else
					{					
						end		= TRUE;
						
					}
					index++;
				}
				if(min_area_index < ((UINT32)(start + HMI_DIRTY_FIFO_LEN)))
				{
					hmi_union_rect(&pdirty_list[min_area_index],
												pnew,
												&union_rect);					
					do
					{
						pdirty_list[min_area_index].x	= pdirty_list[min_area_index+1].x;
						pdirty_list[min_area_index].y	= pdirty_list[min_area_index+1].y;
						pdirty_list[min_area_index].w	= pdirty_list[min_area_index+1].w;
						pdirty_list[min_area_index].h	= pdirty_list[min_area_index+1].h;
						min_area_index++;
					}while((pdirty_list[min_area_index].w != 0)&&
						(min_area_index < ((UINT32)(start + HMI_DIRTY_FIFO_LEN))));/**/
					if(min_area_index >= ((UINT32)(start + HMI_DIRTY_FIFO_LEN)))
					{
						pdirty_list[min_area_index-1].x	= 0;
						pdirty_list[min_area_index-1].y	= 0;
						pdirty_list[min_area_index-1].w	= 0;
						pdirty_list[min_area_index-1].h	= 0;
					}
					hmi_add_dirty(&union_rect,pdirty_list,depth);					
				}
			}
		}
	}
}

#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
#if (HMI_DYN_CONTAINERS_NUMBER > 0)  && (HMI_ALL_STATIC_CONTAINERS_NUMBER > 0)
static BOOLEAN hmi_engine_get_bck_static_container_id(HMI_OBJECT_ID_STR * phmi_dyn_contain_id)
{
	BOOLEAN hmi_changed_flag = FALSE;
	UINT8	loop=0;
	
	if((*phmi_dyn_contain_id) < HMI_DYN_CONTAINERS_NUMBER)
   	{
		*phmi_dyn_contain_id = hmi_dyn_container_table_bck[*phmi_dyn_contain_id];
   	}
	else
	{
		*phmi_dyn_contain_id = HMI_NB_ELEMENTS;
	}
	while((HMI_IS_DYN_CONTAINER(*phmi_dyn_contain_id))&&(loop<HMI_DYN_CONTAINER_NESTED_DEPTH))
	{
		*phmi_dyn_contain_id = HMI_GET_DYN_CONTAINERS_ID_INDEX(*phmi_dyn_contain_id);
		loop++;
	}
	if(loop<HMI_DYN_CONTAINER_NESTED_DEPTH)
	{
		hmi_changed_flag=TRUE;
	}
	else
	{
		HMI_GFX_SET_STATUS(HMI_DYN_CONTAINER_NESTED);
	}
   return(hmi_changed_flag);
}
#endif


BOOLEAN hmi_get_rotation_rect(HMI_RECT_STR *p_rect,HMI_ROTATION_STR *p_axis,
									float_32 angel,HMI_RECT_STR *p_rect_rotation)
{
	BOOLEAN			success		= FALSE;
	//HMI_RECT_STR	rect_axis	= {0,0,0,0};
	HMI_ROTATION_STR	center	= {0,0};
	HMI_ROTATION_STR	rotation= {0,0};
	float_32			cosA	= 0;
	float_32			sinA	= 0;
	HMI_WIDTH_STR width_height	= 0;
	
	if((p_rect != NULL)&&(p_axis != NULL)&&
		(p_rect_rotation != NULL))
	{
		center.x	= p_rect->x + ((p_rect->w)>>1)/*/2*/;
		center.y	= p_rect->y + ((p_rect->h)>>1)/*/2*/;
		/*convert coordinate p_axis as origin*/
		center.x	= center.x - p_axis->x;
		center.y	= center.y - p_axis->y;
		/* rotation 
		x0=xcosA-ysinA
		y0=xsinA+ycosA
		*/
		angel		= (float_32)(HMI_ANGEL_TO_RADIAN(angel));
		if(fabs(angel) < HMI_FLOAT_TOLERANCE)
		{
			cosA		= 1.0f;
			sinA		= 0;
		}
		else
		{
			cosA		= (float_32)cos(angel);
			sinA		= (float_32)sin(angel);
		}
		rotation.x	= (HMI_X_STR)(center.x * cosA - center.y * sinA);
		rotation.y	= (HMI_Y_STR)(center.x * sinA + center.y * cosA);
		/*convert coordinate */
		rotation.x	= rotation.x	+ p_axis->x;
		rotation.y	= rotation.y	+ p_axis->y;
		/*get rotation rection*/
		width_height		= p_rect->w + p_rect->h + HMI_DEL_ANGLE_OFFSET_W;
		p_rect_rotation->x	= rotation.x - (width_height>>1)/*/2*/;
		p_rect_rotation->y	= rotation.y - (width_height>>1)/*/2*/;
		p_rect_rotation->w	= width_height;
		p_rect_rotation->h	= width_height;
								
		success	= TRUE;
	}
	return success;
}



BOOLEAN hmi_get_rotation_pointer_dirty(HMI_RECT_STR *p_pointer_rect,
										HMI_RECT_STR *p_pointer_rect_old,
										HMI_ROTATION_STR *p_axis,HMI_ROTATION_STR *p_axis_old,
										float_32 new_angel,
										float_32 old_angel,HMI_RECT_STR *p_dirty,
										BOOLEAN	trail,
										HMI_RECT_STR *p_father_rect)
{
	BOOLEAN			success		= FALSE;
	HMI_RECT_STR	old_dirty	= {0,0,0,0};
	HMI_RECT_STR	new_dirty	= {0,0,0,0};
	HMI_RECT_STR	final_dirty	= {0,0,0,0};
	float_32		dt			= 0;
	
	if((p_pointer_rect != NULL)&&(p_axis != NULL)&&
		(p_dirty != NULL) && (p_pointer_rect_old != NULL)&&
		(p_dirty != p_father_rect)&&(p_axis_old != NULL))
	{
		dt=(float_32)(fabs(old_angel - new_angel));
		if(dt < HMI_DEL_ANGLE)
		{
			success	= hmi_get_rotation_rect(p_pointer_rect_old,
									p_axis_old,
									old_angel,
									&old_dirty);

			if(success)
			{
				success	= hmi_get_rotation_rect(p_pointer_rect,
									p_axis,
									new_angel,
									&new_dirty);
				if(success)
				{
					if(trail)
					{
						hmi_union_rect(&old_dirty,&new_dirty,&final_dirty);
						/*set min rect which contain p_axis pointer*/
						old_dirty.x	= p_axis->x - HMI_MIN_RECT_HALF_W; /*only rotation at userdefine point can trail enable,*/
						old_dirty.y	= p_axis->y - HMI_MIN_RECT_HALF_H;
						old_dirty.w	= HMI_MIN_RECT_W;
						old_dirty.h	= HMI_MIN_RECT_H;
						/*union */
						hmi_union_rect(&old_dirty,&final_dirty,p_dirty);
					}
					else
					{
						hmi_union_rect(&old_dirty,&new_dirty,p_dirty);
					}
				}
			}
		}
		else /*del angel > HMI_DEL_ANGLE ,dirty zone is father*/
		{
			p_dirty->x	= p_father_rect->x;
			p_dirty->y	= p_father_rect->y;
			p_dirty->w	= p_father_rect->w;
			p_dirty->h	= p_father_rect->h;
			success 	= TRUE;
		}
	}
	
	return	success;	
}


void hmi_add_2buffer_dirty_zone(HMI_RECT_STR pdirty_free[],
							HMI_RECT_STR *pdirty_bck,
							HMI_RECT_STR *pdirty_2buffer,
							UINT8			layer)
{
	UINT8		loop	= 0;

	/*copy union dirty*/
	hmi_union_rect(&pdirty_free[layer],
						&pdirty_bck[layer],
						&pdirty_2buffer[layer]); 
	/*copy the layer dirty*/
	for(loop =(HMI_LAYER_MAX_CNT+layer * HMI_DIRTY_FIFO_LEN);
			loop < (HMI_LAYER_MAX_CNT + layer * HMI_DIRTY_FIFO_LEN + HMI_DIRTY_FIFO_LEN);
			loop++)
	{
		pdirty_2buffer[loop].x	= pdirty_free[loop].x;
		pdirty_2buffer[loop].y	= pdirty_free[loop].y;
		pdirty_2buffer[loop].w	= pdirty_free[loop].w;
		pdirty_2buffer[loop].h	= pdirty_free[loop].h;
	}
			
	loop	= (HMI_LAYER_MAX_CNT + layer * HMI_DIRTY_FIFO_LEN);
	while((loop	< (HMI_LAYER_MAX_CNT+layer * HMI_DIRTY_FIFO_LEN + HMI_DIRTY_FIFO_LEN))&&
		(pdirty_bck[loop].w > 0U)&&
		(pdirty_bck[loop].h > 0U))
	{
		hmi_add_dirty((&pdirty_bck[loop]),pdirty_2buffer,layer);
		loop++;
	}
}


/*copy back dirty to free dirty*/
void hmi_copy_last_dirty_zone(HMI_RECT_STR dirty_bck[],
							HMI_RECT_STR dirty_free[],
							UINT8			layer)
{
	UINT8		loop	= 0;
	
	if((dirty_bck[layer].w > 0)&&(dirty_bck[layer].h > 0))
	{
		dirty_free[layer].x	= dirty_bck[layer].x;
		dirty_free[layer].y	= dirty_bck[layer].y;
		dirty_free[layer].w	= dirty_bck[layer].w;
		dirty_free[layer].h	= dirty_bck[layer].h;
		
		for(loop =(HMI_LAYER_MAX_CNT+layer*HMI_DIRTY_FIFO_LEN);
			loop < (HMI_LAYER_MAX_CNT+layer*HMI_DIRTY_FIFO_LEN+HMI_DIRTY_FIFO_LEN);
			loop++)
		{
			dirty_free[loop].x	= dirty_bck[loop].x;
			dirty_free[loop].y	= dirty_bck[loop].y;
			dirty_free[loop].w	= dirty_bck[loop].w;
			dirty_free[loop].h	= dirty_bck[loop].h;
		}
	}
}
#endif


//modify by sv
#ifdef HMI_LOAD_RES_SEPARATE
#if 0
static void hmi_driver_load_one_file(HMI_OBJECT_ID_STR hmi_object_id)
{
    HMI_OBJECT_ID_STR		hmi_object_index= 0;		
    r_drw2d_Texture_t		tex;
    U08						alpha		= 0;	
    UINT8					*pImage_data= 0;
    rgl_image_type_str		image_format= HMI_IMAGE_NO_COMPRESS;
    U32						data_len	= 0;	
    U32						width		= 0;
    U32						height		= 0;	
    BOOLEAN 				rotation	= FALSE;
    UINT8					bmp_alpha_rotation_flag = 0;

	#if HMI_DXY_IMAGELIST_NUMBER>0
	if(HMI_IS_DYN_IMAGELIST_DXY(hmi_object_id))
    {
		hmi_object_index = HMI_GET_DYN_IMAGELIST_DXY_ID_INDEX(hmi_object_id);												
		#ifdef HMI_GRAPHIC_RGL
		if(hmi_object_index < HMI_DXY_IMAGELIST_NUMBER)
		{
			pImage_data	= (U08 *)(hmi_dxy_imagelist_table[hmi_object_index].file.pbitmap_data);
			image_format=(rgl_image_type_str)(get_compress_fmt(hmi_dxy_imglist_attr_table[hmi_object_index]));
			alpha		= (((hmi_dxy_imglist_attr_table[hmi_object_index].image_attr)&0xf0)&HMI_ALPHA_IMAGE_FLAG);
			data_len	= hmi_dxy_imagelist_table[hmi_object_index].file.data_len;
			width		= hmi_dxy_imagelist_table[hmi_object_index].file.w;
			height		= hmi_dxy_imagelist_table[hmi_object_index].file.h;
			bmp_alpha_rotation_flag = hmi_dxy_imglist_attr_table[hmi_object_index].image_attr&0xf0;
			rotation	= bmp_alpha_rotation_flag&HMI_ROTATION_IMAGE_FLAG;
			get_texture_res_manager(hmi_object_id,
							&tex,(U08 *)pImage_data,
							data_len,
							width,height,
							rotation,image_format,
							alpha,
							0);				 												
		}
		#endif									
	}		
	else
	#endif
	#if HMI_SXY_IMAGELIST_NUMBER>0
	if(HMI_IS_DYN_IMAGELIST_SXY(hmi_object_id))
    {
		hmi_object_index = HMI_GET_DYN_IMAGELIST_SXY_ID_INDEX(hmi_object_id);			
		#ifdef HMI_GRAPHIC_RGL
		if(hmi_object_index < HMI_SXY_IMAGELIST_NUMBER)
		{
			pImage_data	= (U08 *)(hmi_sxy_imagelist_table[hmi_object_index].file.pbitmap_data);
			image_format= (rgl_image_type_str)(get_compress_fmt(hmi_sxy_imglist_attr_table[hmi_object_index]));
			alpha	= (((hmi_sxy_imglist_attr_table[hmi_object_index].image_attr)&0xf0)&HMI_ALPHA_IMAGE_FLAG);
			data_len	= hmi_sxy_imagelist_table[hmi_object_index].file.data_len;
			width		= hmi_sxy_imagelist_table[hmi_object_index].file.w;
			height		= hmi_sxy_imagelist_table[hmi_object_index].file.h;
			bmp_alpha_rotation_flag = hmi_sxy_imglist_attr_table[hmi_object_index].image_attr&0xf0;
			rotation	= bmp_alpha_rotation_flag&HMI_ROTATION_IMAGE_FLAG;
			get_texture_res_manager(hmi_object_id,
							&tex,(U08 *)pImage_data,
							data_len,
							width,height,
							rotation,image_format,
							alpha,
							0);									
		}
		#endif												
	}
	else
	#endif			
	#if HMI_DXY_SCROLLBAR_NUMBER>0
	if(HMI_IS_DYN_SCROLLBAR_DXY(hmi_object_id))
    {
		hmi_object_index = HMI_GET_DYN_SCROLLBAR_DXY_ID_INDEX(hmi_object_id);									
		#if HMI_SCROLLBAR_MAX_STATUS >= 0			
		{				
			if(hmi_object_index < HMI_DXY_SCROLLBAR_NUMBER)
			{
				pImage_data	= (U08 *)(hmi_dxy_scrollbar_table[hmi_object_index].file.pbitmap_data);
				image_format= (rgl_image_type_str)(get_compress_fmt(hmi_dxy_scrollbar_attr_table[hmi_object_index]));
				alpha	= (((hmi_dxy_scrollbar_attr_table[hmi_object_index].image_attr)&0xf0)&HMI_ALPHA_IMAGE_FLAG);
				data_len	= hmi_dxy_scrollbar_table[hmi_object_index].file.data_len;
				width		= hmi_dxy_scrollbar_table[hmi_object_index].file.w;
				height		= hmi_dxy_scrollbar_table[hmi_object_index].file.h;
				bmp_alpha_rotation_flag = hmi_dxy_scrollbar_attr_table[hmi_object_index].image_attr&0xf0;
				rotation	= bmp_alpha_rotation_flag&HMI_ROTATION_IMAGE_FLAG;
				get_texture_res_manager(hmi_object_id,
								&tex,(U08 *)pImage_data,
								data_len,
								width,height,
								rotation,image_format,
								alpha,
								0);														
			}
		}		
		#endif														
	}
	else		
	#endif
	#if HMI_SXY_SCROLLBAR_NUMBER>0
	if(HMI_IS_DYN_SCROLLBAR_SXY(hmi_object_id))
    {
		hmi_object_index = HMI_GET_DYN_SCROLLBAR_SXY_ID_INDEX(hmi_object_id);						
		#if HMI_SCROLLBAR_MAX_STATUS >= 0			
		if(hmi_object_index < HMI_SXY_SCROLLBAR_NUMBER)
		{
			pImage_data	= (U08 *)(hmi_sxy_scrollbar_table[hmi_object_index].file.pbitmap_data);
			image_format= (rgl_image_type_str)(get_compress_fmt(hmi_sxy_scrollbar_attr_table[hmi_object_index]));
			alpha	= (((hmi_sxy_scrollbar_attr_table[hmi_object_index].image_attr)&0xf0)&HMI_ALPHA_IMAGE_FLAG);
			data_len	= hmi_sxy_scrollbar_table[hmi_object_index].file.data_len;
			width		= hmi_sxy_scrollbar_table[hmi_object_index].file.w;
			height		= hmi_sxy_scrollbar_table[hmi_object_index].file.h;
			bmp_alpha_rotation_flag = hmi_sxy_scrollbar_attr_table[hmi_object_index].image_attr&0xf0;
			rotation	= bmp_alpha_rotation_flag&HMI_ROTATION_IMAGE_FLAG;
			get_texture_res_manager(hmi_object_id,
							&tex,(U08 *)pImage_data,
							data_len,
							width,height,
							rotation,image_format,
							alpha,
							0);										
		}			
		#endif															
	}
	else
	#endif
	#if HMI_DXY_BUTTON_NUMBER > 0  
	if(HMI_IS_DYN_BUTTON_DXY(hmi_object_id))
    {
		hmi_object_index = HMI_GET_DYN_BUTTON_DXY_ID_INDEX(hmi_object_id);							
		if(hmi_object_index < HMI_DXY_BUTTON_NUMBER)
		{				
			pImage_data	= (U08 *)(hmi_dxy_button_table[hmi_object_index].button_image.file.pbitmap_data);
			image_format= (rgl_image_type_str)(get_compress_fmt(hmi_dxy_button_attr_table[hmi_object_index]));
			alpha	= (((hmi_dxy_button_attr_table[hmi_object_index].image_attr)&0xf0)&HMI_ALPHA_IMAGE_FLAG);
			data_len	= hmi_dxy_button_table[hmi_object_index].button_image.file.data_len;
			width		= hmi_dxy_button_table[hmi_object_index].button_image.file.w;
			height		= hmi_dxy_button_table[hmi_object_index].button_image.file.h;
			bmp_alpha_rotation_flag = hmi_dxy_button_attr_table[hmi_object_index].image_attr&0xf0;
			rotation	= bmp_alpha_rotation_flag&HMI_ROTATION_IMAGE_FLAG;
			get_texture_res_manager(hmi_object_id,
							&tex,(U08 *)pImage_data,
							data_len,
							width,height,
							rotation,image_format,
							alpha,
							0);										
			
		}												
	}		
	else
	#endif		
	#if HMI_SXY_BUTTON_NUMBER>0 		
	if(HMI_IS_DYN_BUTTON_SXY(hmi_object_id))
    {
		hmi_object_index = HMI_GET_DYN_BUTTON_SXY_ID_INDEX(hmi_object_id);			
		#ifdef HMI_GRAPHIC_RGL						
		if(hmi_object_index < HMI_SXY_BUTTON_NUMBER)
		{
			pImage_data	= (U08 *)(hmi_sxy_button_table[hmi_object_index].button_image.file.pbitmap_data);
			image_format= (rgl_image_type_str)(get_compress_fmt(hmi_sxy_button_attr_table[hmi_object_index]));
			alpha	= (((hmi_sxy_button_attr_table[hmi_object_index].image_attr)&0xf0)&HMI_ALPHA_IMAGE_FLAG);
			data_len	= hmi_sxy_button_table[hmi_object_index].button_image.file.data_len;
			width		= hmi_sxy_button_table[hmi_object_index].button_image.file.w;
			height		= hmi_sxy_button_table[hmi_object_index].button_image.file.h;
			bmp_alpha_rotation_flag = hmi_sxy_button_attr_table[hmi_object_index].image_attr&0xf0;
			rotation	= bmp_alpha_rotation_flag&HMI_ROTATION_IMAGE_FLAG;
			get_texture_res_manager(hmi_object_id,
							&tex,(U08 *)pImage_data,
							data_len,
							width,height,
							rotation,image_format,
							alpha,
							0);	
		}			
		#endif																	
	}		
	else 		
	#endif
	#if HMI_DXY_BITMAPS_NUMBER> 0 
	if(HMI_IS_DYN_XY_BITMAP(hmi_object_id))
	{	  
		hmi_object_index = HMI_GET_DYN_XY_BITMAP_INDEX(hmi_object_id );												
		if(hmi_object_index < HMI_DXY_BITMAPS_NUMBER)
		{
			pImage_data	= (U08 *)(hmi_bmp_dyn_xy_prop_table[hmi_object_index].pbitmap_data);
			image_format= (rgl_image_type_str)(get_compress_fmt(hmi_dxy_bitmap_attr_table[hmi_object_index]));
			alpha	= (((hmi_dxy_bitmap_attr_table[hmi_object_index].image_attr)&0xf0)&HMI_ALPHA_IMAGE_FLAG);
			data_len	= hmi_bmp_dyn_xy_prop_table[hmi_object_index].data_len;
			width		= hmi_bmp_dyn_xy_prop_table[hmi_object_index].w;
			height		= hmi_bmp_dyn_xy_prop_table[hmi_object_index].h;
			bmp_alpha_rotation_flag = hmi_dxy_bitmap_attr_table[hmi_object_index].image_attr&0xf0;
			rotation	= bmp_alpha_rotation_flag&HMI_ROTATION_IMAGE_FLAG;
			get_texture_res_manager(hmi_object_id,
							&tex,(U08 *)pImage_data,
							data_len,
							width,height,
							rotation,image_format,
							alpha,
							0);									
		}																					
	}
	else
	#endif					     	   	   		
	#if HMI_SXY_BITMAPS_NUMBER> 0
	if(HMI_IS_S_XY_BITMAP(hmi_object_id))
	{	         
        hmi_object_index = HMI_GET_SXY_BITMAPS_ID_INDEX(hmi_object_id);			
	    if(hmi_object_index < HMI_SXY_BITMAPS_NUMBER)
	    {	
			pImage_data	= (U08 *)(hmi_bmp_static_xy_prop_table[hmi_object_index].pbitmap_data);
			image_format= (rgl_image_type_str)(get_compress_fmt(hmi_sxy_bitmap_attr_table[hmi_object_index]));
			alpha	= (((hmi_sxy_bitmap_attr_table[hmi_object_index].image_attr)&0xf0)&HMI_ALPHA_IMAGE_FLAG);
			data_len	= hmi_bmp_static_xy_prop_table[hmi_object_index].data_len;
			width		= hmi_bmp_static_xy_prop_table[hmi_object_index].w;
			height		= hmi_bmp_static_xy_prop_table[hmi_object_index].h;
			bmp_alpha_rotation_flag = hmi_sxy_bitmap_attr_table[hmi_object_index].image_attr&0xf0;
			rotation	= bmp_alpha_rotation_flag&HMI_ROTATION_IMAGE_FLAG;
			get_texture_res_manager(hmi_object_id,
							&tex,(U08 *)pImage_data,
							data_len,
							width,height,
							rotation,image_format,
							alpha,
							0);																												
	    }
	}		
	else
	#endif 	   		 
	{
		;
	}	   	   
}
#endif
#endif

#if 0
UINT8 hmi_driver_load_files(void)
{	
    UINT8		LoadEnd						= 0;
	UINT8		hmi_load_segment_index		= 0;// lq
#if HMI_LOAD_SOURCE_MODE == HMI_LOAD_RES_ALL_INIT
    if(hmi_load_object_index < HMI_EVENT_ACT_BEGIN_INDEX)
    {
       // hmi_driver_load_one_file(hmi_load_object_index);//removed by pxguo 
       hmi_load_file_to_vram(hmi_load_object_index);
        hmi_load_object_index++;	
    }
    else
    {
        LoadEnd = 1;
    }
#elif HMI_LOAD_SOURCE_MODE == HMI_LOAD_RES_SEGMEN
    if(hmi_load_segment_index < HMI_SEGMENT_MAX)
    {
        HMI_OBJECT_PROP_STR CONST * phmi_container_object_table = hmi_seg_info_list[hmi_load_segment_index].container_object_table.p_object_table;
        UINT8 hmi_object_number = hmi_seg_info_list[hmi_load_segment_index].container_object_table.object_number;
        if(hmi_load_object_index < hmi_object_number)
        {
            HMI_OBJECT_ID_STR hmi_object_id	= phmi_container_object_table[hmi_load_object_index].object_id;
            hmi_driver_load_one_file(hmi_object_id);
	        hmi_load_object_index++;	
        }
        else
        {
            LoadEnd = 1;
            hmi_load_segment_index = HMI_SEGMENT_MAX;
        }
    }
    else
    {
        LoadEnd = 1;
    }
#endif
    return LoadEnd;
}
#endif


#if HMI_DXY_CUSTOM_CNT+HMI_SXY_CUSTOM_CNT >0U
/*
v2 interpoint(Line a,Line b)
{
    v2 u=a.P-b.P;
    double t=(b.v*u)/(a.v*b.v);
    return a.P+a.v*t;
}

*/

/*
double k = fArea(p1,p2,p3) / fArea(p1,p2,p4);
return Point((p3.x + k*p4.x)/(1+k),(p3.y + k*p4.y)/(1+k));
*/
SPOINT_TP hmi_get_interpoint_2_vector(SPOINT_TP *pp1,SPOINT_TP *pp2,SPOINT_TP *pp3,SPOINT_TP *pp4)
{
	SPOINT_TP	interpoint	= {0,0};
	INT32		area_p123	= 0;
	INT32		area_p124	= 0;
	double_64	k			= 0.0;
	float_32	f_value 	= 0.0f;
	
	if((pp1 != NULL)&&(pp2 != NULL)&&
		(pp3 != NULL)&&(pp4 != NULL))
	{	
		interpoint.x	= pp1->x;
		interpoint.y	= pp1->y;
		
		area_p123	= HMI_GET_AREA((*pp1),(*pp2),(*pp3));	
		if(area_p123 < 0)
		{
			area_p123	= -area_p123;
		}

		area_p124	= HMI_GET_AREA((*pp1),(*pp2),(*pp4));	
		if(area_p124 < 0)
		{
			area_p124	= -area_p124;
		}
		if((area_p124 > HMI_FLOAT_TOLERANCE)||(area_p124 < -HMI_FLOAT_TOLERANCE))
		{
			k		= area_p123 / ((float_32)area_p124);
			f_value = (float_32)((pp3->x + k * pp4->x) / (1 + k));
			if(f_value > 0)
			{
				interpoint.x	= (SINT16)(f_value + 0.5f);
			}
			else
			{
				interpoint.x	= (SINT16)(f_value - 0.5f);
			}

			f_value = (float_32)((pp3->y + k * pp4->y) / (1 + k));
			if(f_value > 0)
			{
				interpoint.y	= (SINT16)(f_value + 0.5f);
			}
			else
			{
				interpoint.y	= (SINT16)(f_value - 0.5f);
			}
	#if 0
			interpoint.x	= (SINT16)((pp3->x + k * pp4->x) / (1 + k) + 0.5f);
			interpoint.y	= (SINT16)((pp3->y + k * pp4->y) / (1 + k)+ 0.5f);
	#endif
		}
	}

	return	interpoint;
}



//extern 	HMI_COORDINATE_STR		hmi_screen_size[HMI_SCREEN_NO];
/*
	p1--scale
	p2--angel
	p3--alpha
	id1--container
	
*/

#if 0 /*lq*/
void hmi_engine_draw_scale_container(HMI_RECT_STR CONST 	*pcustom_farther_rect,
											HMI_RECT_STR			*pcustom_dirty_rect,
											U08						custom_depth,
											HMI_RECT_STR			*pcustom_cliped_farther_rect,
											HMI_OBJECT_ID_STR		custom_id,
											//UINT8					custom_alpha
											HMI_ALPHA_SCALE_PT_STR *pfather_alpha_scale)
{
	BOOLEAN				get_success					= FALSE;
	BOOLEAN				success_frmbuf			= FALSE;
	HMI_OPENGL_FB_STR	hmi_fb					= {0};
	HMI_OBJECT_PROP_STR container_table			= {0};
	HMI_OBJECT_ID_STR	hmi_container_id		= 0;
	HMI_RECT_STR		hmi_screen_rect			= {0};
	HMI_RECT_STR		hmi_scale_rect			= {0};
	HMI_RECT_STR		new_cliped_farther_rect	= {0};
	HMI_RECT_STR		hmi_clip_rect			= {0};
	UINT8				container_alpha			= 0U;
	float_32			container_angle			= 0.0f;
	float_32			container_scale			= 0.0f;
	HMI_CONTAINER_SCALE_POINT	container_scale_point = CONTAINER_SCALE_CNT;
	BYTE					win_no				= 0;
	HMI_CUSTOM_PROP_STR	custom_object_prop  ={0U};
	HMI_ELEMENT_PROP_STR hmi_object_prop	= {0U};
	HMI_RECT_STR		hmi_container_rect	= {0};
	get_success = hmi_engine_get_custom_prop(custom_id,&custom_object_prop);

	if(get_success == TRUE)
	{
		if((custom_object_prop.attr & HMI_ID1) !=0)
		{
			/*draw at another framebuffer*/
			success_frmbuf	= Draw_frambuffer_begin(&hmi_fb.framebuffer,
										&hmi_fb.depthRenderbuffer,
										&hmi_fb.texture_frmbuf,
										custom_object_prop.w,
										custom_object_prop.h,
										&hmi_fb.old_framebuffer);
			
			if(success_frmbuf == TRUE)
			{
				hmi_container_id = custom_object_prop.id1;

				#if HMI_DYN_CONTAINERS_NUMBER > 0
				if(HMI_IS_DYN_CONTAINER(hmi_container_id))
				{
					hmi_container_id  = HMI_GET_DYN_CONTAINERS_ID_INDEX(hmi_container_id);
			  		hmi_engine_get_static_container_id(&hmi_container_id);/*get static container ID*/	
				}
				#endif

				hmi_engine_get_object_prop(hmi_container_id,&hmi_object_prop);
				hmi_container_rect.x	= hmi_object_prop.x;
				hmi_container_rect.y	= hmi_object_prop.y;
				hmi_container_rect.w	= hmi_object_prop.w;
				hmi_container_rect.h	= hmi_object_prop.h;

				hmi_get_object_screen_coor((HMI_RECT_STR CONST *)pcustom_farther_rect,
							&hmi_container_rect,
							&(hmi_screen_rect));
				hmi_get_intersec_rect((HMI_RECT_STR CONST *)pcustom_cliped_farther_rect,
								&hmi_screen_rect,
								&(new_cliped_farther_rect ));
				hmi_get_intersec_rect_depth((HMI_RECT_STR CONST *)(&(new_cliped_farther_rect)),
								custom_depth,(HMI_RECT_STR  *)pcustom_dirty_rect,&hmi_clip_rect);
				container_table.object_id =custom_object_prop.id1;
				/*draw_container*/
				hmi_engine_draw_object((HMI_OBJECT_PROP_STR CONST * )(&container_table),
						(HMI_RECT_STR CONST *)pcustom_farther_rect,
						pcustom_dirty_rect,
						custom_depth,
						pcustom_cliped_farther_rect
						#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_RGL))
						,pfather_alpha_scale
						#endif
						);
				if((custom_object_prop.attr & HMI_P1) !=0)
				{	
					if((custom_object_prop.attr & HMI_P1_F) !=0)
					{
						container_scale			= (float_32)(HMI_U32_TO_F32(custom_object_prop.p1));
					}
					else
					{
						container_scale			= (float_32)(custom_object_prop.p1);
					}
				}
				if((custom_object_prop.attr & HMI_P2) !=0)
				{
					if((custom_object_prop.attr & HMI_P2_F) !=0)
					{
						container_angle			= (HMI_U32_TO_F32(custom_object_prop.p2));
					}
					else
					{
						container_angle			= (float_32)(custom_object_prop.p2);
					}
				}
				if((custom_object_prop.attr & HMI_P3) !=0)
				{
					if((custom_object_prop.attr & HMI_P3_F) !=0)
					{
						container_alpha			= (UINT8)(HMI_U32_TO_F32(custom_object_prop.p3));
					}
					else
					{
						container_alpha			= (UINT8)(custom_object_prop.p3);
					}
				}
				if((custom_object_prop.attr & HMI_CON_P1) !=0)
				{
					if((custom_object_prop.attr & HMI_CON_F1) !=0)
					{
						container_scale_point	= (HMI_U32_TO_F32(custom_object_prop.const_p1));
					}
					else
					{
						container_scale_point	= (custom_object_prop.const_p1);
					}
					if(container_scale_point == CONTAINER_SCALE_LEFTUP)
					{
						hmi_scale_rect.w	= (HMI_WIDTH_STR)(hmi_screen_rect.w*container_scale);
						hmi_scale_rect.h	= (HMI_HEIGHT_STR)(hmi_screen_rect.h*container_scale);
					}
					else if(container_scale_point == CONTAINER_SCALE_RIGHTUP)
					{
						hmi_scale_rect.w	= (HMI_WIDTH_STR)(hmi_screen_rect.w*container_scale);
						hmi_scale_rect.h	= (HMI_HEIGHT_STR)(hmi_screen_rect.h*container_scale);
						hmi_scale_rect.x	= hmi_screen_rect.x+(hmi_screen_rect.w -hmi_scale_rect.w);
					}
					else if(container_scale_point == CONTAINER_SCALE_CENTER)
					{
						hmi_scale_rect.w	= (HMI_WIDTH_STR)(hmi_screen_rect.w*container_scale);
						hmi_scale_rect.h	= (HMI_HEIGHT_STR)(hmi_screen_rect.h*container_scale);
						hmi_scale_rect.x	= hmi_screen_rect.x+(hmi_screen_rect.w>>1) -(hmi_scale_rect.w>>1);
						hmi_scale_rect.y	= hmi_screen_rect.y+(hmi_screen_rect.h>>1) -(hmi_scale_rect.h>>1);
					}
					else if(container_scale_point == CONTAINER_SCALE_LEFTDOWN)
					{
						hmi_scale_rect.w	= (HMI_WIDTH_STR)(hmi_screen_rect.w*container_scale);
						hmi_scale_rect.h	= (HMI_HEIGHT_STR)(hmi_screen_rect.h*container_scale);
						hmi_scale_rect.y	= hmi_screen_rect.y+(hmi_screen_rect.h -hmi_scale_rect.h);
					}
					else if(container_scale_point == CONTAINER_SCALE_RIGHTDOWN)
					{
						hmi_scale_rect.w	= (HMI_WIDTH_STR)(hmi_screen_rect.w*container_scale);
						hmi_scale_rect.h	= (HMI_HEIGHT_STR)(hmi_screen_rect.h*container_scale);
						hmi_scale_rect.x	= hmi_screen_rect.x+(hmi_screen_rect.w -hmi_scale_rect.w);
						hmi_scale_rect.y	= hmi_screen_rect.y+(hmi_screen_rect.h -hmi_scale_rect.h);
					}
					else
					{
					}
				}
				
				/*Restory framebuffer,set texture sampler*/
				restory_frambuffer_win_no_texture_sampler(
										hmi_fb.old_framebuffer,
										win_no,
										HMI_TEXURE_UINT_FRMBUF);
				/*draw to old framebuffer*/
				hmi_driver_draw_image(&hmi_scale_rect,			
									pcustom_dirty_rect,
									&hmi_clip_rect,
									hmi_fb.texture_frmbuf,
									0,
									container_alpha,
  									container_angle,
									container_scale);
				/*after finish ,release framebuffer resource*/						
				release_frambuffer_res(&hmi_fb.framebuffer,
									&hmi_fb.depthRenderbuffer,
									&hmi_fb.texture_frmbuf,
									&hmi_fb.old_framebuffer);
			}
		}
	}

	
}
#endif







/*
Move distortion,get left index at distortion zone
*/
void	hmi_get_element_index_begin_at_distortion(INT32				motion_distance,
												HMI_CONTAINER_LAYOUT_STR		*playout_attr,											
												INT16							distortion_begin_index/*begin at 0,last head distortion begin index*/,
												SPOINT_TP						*pdistortion_begin_index_offset/*first element index,first element center offset at distortion*/
												)
{
	INT16	container_at_view_begin_index	= -1;
	INT16	container_at_view_begin_offset	= 0;
	INT32	all_container_width				= 0;
	INT32	distortion_container_begin		= 0;
	INT16	container_w						= 0;
	INT16	container_distance				= 0;
	INT16	half_container_w				= 0;

	if((pdistortion_begin_index_offset != NULL)&&
		(playout_attr != NULL))
	{
		container_w				= playout_attr->container_w;
		container_distance		= (INT16)(container_w + playout_attr->container_interval);
		all_container_width		= container_distance * (playout_attr->container_element_cnt) ;
		distortion_container_begin	= motion_distance + container_distance * distortion_begin_index ;
		half_container_w		=  (INT16)(container_w >> 1);
		if(all_container_width !=0)
		{
			distortion_container_begin	= distortion_container_begin % all_container_width;
		}
		if(distortion_container_begin < 0)
		{			
			distortion_container_begin	= all_container_width + distortion_container_begin;
		}
		
		if(container_distance != 0U)
		{
			container_at_view_begin_index	= (INT16)(distortion_container_begin / container_distance);									
			container_at_view_begin_offset	= (INT16)(distortion_container_begin % container_distance);
			/*center at distortion*/
			if(container_at_view_begin_offset != 0)
			{
				container_at_view_begin_index++;
				if(container_at_view_begin_index > (playout_attr->container_element_cnt))
				{
					container_at_view_begin_index= 0;
				}
				container_at_view_begin_offset = container_distance - container_at_view_begin_offset + half_container_w;
			}
			else
			{
				container_at_view_begin_offset	= half_container_w ;
			}
			
		}
		else
		{
			container_at_view_begin_index	= 0;
			container_at_view_begin_offset	= 0;
		}
		pdistortion_begin_index_offset->x/*first element index at distortion*/	= container_at_view_begin_index;
		pdistortion_begin_index_offset->y/*first element offset at distortion*/	= container_at_view_begin_offset;
	}	
}


#define	HMI_MAX_CHILD_CNT	256

/*
get center of container.origin is distration left.
*/
BOOLEAN	hmi_get_center_container_rect(INT16				index/*index at distortion*/,
									HMI_CONTAINER_LAYOUT_STR	*playout_attr,
									SPOINT_TP		*pdistortion_begin_index_offset,
									SPOINT_TP		*pcenter_rect
									)
{
	BOOLEAN		success						= FALSE;	
	SINT16		x							= 0;
	SINT16		right_x						= 0;
	INT16		half_container_w			= 0;
	INT32		distortion_container_width	= 0;
	INT16		container_distance			= 0;
	
	if((pdistortion_begin_index_offset != NULL)&&
		(pcenter_rect != NULL)&&(playout_attr != NULL))
	{					
		half_container_w	= (playout_attr->container_w >> 1);		
		container_distance	= (INT16)(playout_attr->container_w + playout_attr->container_interval);
		distortion_container_width	= container_distance * (playout_attr->distortion_element_cnt);
		/*center position */
		x				= (SINT16)(pdistortion_begin_index_offset->y/*center offset at distortion*/ +	
							(playout_attr->container_w + playout_attr->container_interval) * index);		
				
		/*at distortion zone*/
		right_x			= x  - half_container_w + container_distance;
		if(right_x <=  distortion_container_width)
		{
			pcenter_rect->x	= x;
			pcenter_rect->y	= 0;
			
			success	= TRUE;
		}
	}

	return	success;
}









/************Custom lib********/
//#if	(HMI_DXY_CUSTOM_CNT + HMI_SXY_CUSTOM_CNT > 0) 


BOOLEAN	hmi_get_element_at_distortion_position(float_32			dt/*[1--0] or [-1,0]*/,
												HMI_CONTAINER_LAYOUT_STR	*playout_attr,
												HMI_RECT_STR				*pdistortion_rect/*screen coordinate*/,
												SINT16						head_index,
												SINT16						last_head_index,																												
												float_32					first_element_scale,
												BYTE						first_element_alpha,													
												HMI_CONTAINER_CENTER_ALPHA_CLIP_STR *pcontainer_attr,
												SINT16						index/*container_at_distortion*/
												)
{	
	BOOLEAN			success						= FALSE;
	SINT16			begin_distortion				= 0;
	SPOINT_TP		distortion_begin_index_offset	= {0,0};
	SINT16			distortion_begin_index			= 0;
	SINT16			last_distortion_begin_index		= 0;
	SINT32			motion_distance					= 0;
	SINT32			motion_distance1					= 0;
	SINT32			motion_distance2					= 0;
	SPOINT_TP		scale_origin					= {0,0};	
	float_32		scale							= 1.0f;	
	SPOINT_TP		container_center_point			= {0,0};
	SPOINT_TP		origin_point					= {0,0};
	SPOINT_TP		center_distortion_rect			= {0,0};
	HMI_WIDTH_STR	distortion_half_w				= 0;
	HMI_WIDTH_STR	distortion_display_half_w		= 0;
	HMI_HEIGHT_STR	distortion_display_half_h		= 0;
	HMI_WIDTH_STR	container_half_w				= 0;
	HMI_HEIGHT_STR	container_half_h				= 0;	
	HMI_WIDTH_STR	container_w						= 0U;
	HMI_HEIGHT_STR	container_h						= 0U;
	SINT16			distortion_w					= 0;
	SINT32			container_interval				= 0;	
	SINT16			container_element_cnt			= 0;
	SINT16			distortion_element_cnt			= 0;
	SINT32			layerout_mode					= 0;
	SINT16			x1								= 0;
	float_32		y1_float						= 0;
	SINT16			x2								= 0;
	float_32		y2_float						= 0;
	BOOLEAN			bscale							= FALSE;
	INT16			container_distance				= 0;
	UINT8			alpha					= HMI_MAX_ALPHA_VALUE;	
	

	if(playout_attr != NULL)
	{
		container_w					= playout_attr->container_w;
		container_h					= playout_attr->container_h;
		container_interval			= playout_attr->container_interval;	
		container_element_cnt		= playout_attr->container_element_cnt;
		distortion_element_cnt		= playout_attr->distortion_element_cnt;
		layerout_mode				= playout_attr->layerout_mode;		
	}

	if((pdistortion_rect != NULL)&&(distortion_element_cnt > 0)&&
		(index < distortion_element_cnt)&&(index < HMI_MAX_CHILD_CNT)&&
		((pcontainer_attr != NULL)))
	{	
		container_distance	= (INT16)(container_w + container_interval);
		distortion_w		= container_distance * (distortion_element_cnt ) ;
		
		distortion_half_w	= distortion_w >>1;

		distortion_display_half_w		= (pdistortion_rect->w)>>1;
		distortion_display_half_h		= (pdistortion_rect->h)>>1;
		
		
		container_half_w	= container_w >>1;
		container_half_h	= container_h >>1;
		
		
		/*check time elapse*/
		//dt= 1.0f -dt;
		if(dt > HMI_MAX_TIME_DELTA_ACTION)
		{
			dt = HMI_MAX_TIME_DELTA_ACTION;
		}
		else if(dt < (-HMI_MAX_TIME_DELTA_ACTION))
		{
			dt = -HMI_MAX_TIME_DELTA_ACTION;
		}
		else
		{
		}
		
		/*last head index,current head index */
		if(layerout_mode == 0)/*left container at top level*/
		{			
			distortion_begin_index		= head_index - (distortion_element_cnt - 1);
			last_distortion_begin_index	= last_head_index- (distortion_element_cnt - 1);
		}
		else if(layerout_mode == 1)/*right container at top level*/
		{
			distortion_begin_index		= head_index;
			last_distortion_begin_index	= last_head_index;
		}
		else if(layerout_mode == 2)/*center container at top level*/
		{
			distortion_begin_index		= head_index - (distortion_element_cnt >> 1);
			last_distortion_begin_index	= last_head_index - (distortion_element_cnt >> 1);
		}
		else
		{
			distortion_begin_index		= head_index;
			last_distortion_begin_index	= last_head_index;
		}
		/*at one circle*/
		if(last_distortion_begin_index < 0)
		{
			last_distortion_begin_index	+= container_element_cnt;
		}
		if(distortion_begin_index < 0)
		{
			distortion_begin_index	+= container_element_cnt;
		}
		/*Get shortest path*/
		#if 0 /*not find abs() lq*/
		motion_distance1	= abs((distortion_begin_index - last_distortion_begin_index) *
							container_distance);
		#endif 

		motion_distance1	= (distortion_begin_index - last_distortion_begin_index) * container_distance;
		if(motion_distance1 < 0)
		{
			motion_distance1	= -motion_distance1;
		}
		
		motion_distance2	= container_distance * container_element_cnt -  motion_distance1;

		if(last_distortion_begin_index < distortion_begin_index)
		{
			if(motion_distance1 < motion_distance2)
			{
				motion_distance	= motion_distance1;
			}
			else
			{
				motion_distance	= -motion_distance2;
			}
		}
		else
		{
			if(motion_distance1 < motion_distance2)
			{
				motion_distance	= -motion_distance1;
			}
			else
			{
				motion_distance	= motion_distance2;
			}
		}
		
		motion_distance	= (INT32)(motion_distance * dt) ; 

		/*get left element at distortion --last head index+motion_distance at distortion*/		
		hmi_get_element_index_begin_at_distortion(motion_distance,
											playout_attr,
											last_distortion_begin_index/*begin at 0*/,
											&distortion_begin_index_offset);
				
		/*get container center position  at distortion.origin is distration left middle*/			
		begin_distortion	= index;
		success	= hmi_get_center_container_rect(begin_distortion/*index at distortion*/,
							playout_attr,
							&distortion_begin_index_offset,
							&container_center_point
							);
		if(success == TRUE)
		{
			/*Distortion*/				
			if(layerout_mode == 0)/*left container at top level*/
			{	
				/*convert coordinate. origin is center right elemant at distortion */
				scale_origin.x	= (SINT16)(distortion_w - container_distance + container_half_w); 
				scale_origin.y	= 0;
				HMI_TRANSFORM_COORDINATE(container_center_point,scale_origin);
			
				x1			= -(distortion_w - container_distance);
				y1_float	= first_element_scale;
				x2			= 0;	
				y2_float	= 1.0f;
				/* y= ((x-x2)/(x1-x2))*(y1-y2) +y2   */
				if(x1 != x2)
				{
					scale	= (((container_center_point.x - x2)/((float_32)(x1- x2)) )) * (y1_float - y2_float) + y2_float;					
				}
				else
				{
					scale	= 1.0f;					
				}

				x1		= -(distortion_w - container_distance);
				y1_float	= first_element_alpha;
				x2			= 0;	
				y2_float	= HMI_MAX_ALPHA_VALUE;
				/* y= ((x-x2)/(x1-x2))*(y1-y2) +y2   */
				if(x1 != x2)
				{
					alpha	= (BYTE)((((container_center_point.x - x2)/((float_32)(x1- x2)) )) * (y1_float - y2_float) + y2_float);					
				}
				else
				{
					alpha	= HMI_MAX_ALPHA_VALUE;					
				}
				/*get scale position*/
				container_center_point.x	= (SINT16)(container_center_point.x * scale);
				/*convert coordinate. origin is center of display distortion */
				center_distortion_rect.x	= -(distortion_half_w - container_distance + container_half_w);
				center_distortion_rect.y	= 0;
				HMI_TRANSFORM_COORDINATE(container_center_point,center_distortion_rect);
			}
			else if(layerout_mode == 1)/*right container at top level*/
			{					
				/*convert coordinate. origin is center left elemant at distortion */
				scale_origin.x	= (SINT16)(container_half_w); 
				scale_origin.y	= 0;
				HMI_TRANSFORM_COORDINATE(container_center_point,scale_origin);

				x1			= 0;
				y1_float 		= 1.0f;
				x2			= (SINT16)(distortion_w - container_w - container_interval);	
				y2_float		= first_element_scale;
							
				/* y= ((x-x2)/(x1-x2))*(y1-y2) +y2   */
				if(x1 != x2)
				{
					scale	= (((container_center_point.x - x2)/((float_32)(x1- x2)) )) * (y1_float - y2_float) + y2_float;
				}
				else
				{
					scale	= 1.0f;
				}

				x1			= 0;
				y1_float		= HMI_MAX_ALPHA_VALUE;
				x2			= (SINT16)(distortion_w - container_w - container_interval);	
				y2_float		= first_element_alpha;
							
				/* y= ((x-x2)/(x1-x2))*(y1-y2) +y2   */
				if(x1 != x2)
				{
					alpha	= (BYTE)((((container_center_point.x - x2)/((float_32)(x1- x2)) )) * (y1_float - y2_float) + y2_float);
				}
				else
				{
					alpha	= HMI_MAX_ALPHA_VALUE;
				}
				/*get scale position*/
				container_center_point.x	= (SINT16)(container_center_point.x * scale);
				/*convert coordinate. origin is center of display distortion */
				center_distortion_rect.x	= distortion_half_w - container_half_w;
				center_distortion_rect.y	= 0;
				HMI_TRANSFORM_COORDINATE(container_center_point,center_distortion_rect);
													
			}
			else if(layerout_mode == 2)/*center container at top level*/
			{
				/*convert coordinate. origin is center of distortion */
				center_distortion_rect.x	= (SINT16)(distortion_half_w);
				center_distortion_rect.y	= 0;
				HMI_TRANSFORM_COORDINATE(container_center_point,center_distortion_rect);
			
				if(container_center_point.x < 0)
				{
					x1			= -(SINT16)(distortion_half_w - container_half_w);
					y1_float		= first_element_scale;
					x2			= 0;	
					y2_float		= 1.0f;								
				}
				else
				{
					x1			= 0;
					y1_float		= 1.0f;
					x2			= (SINT16)(distortion_half_w - container_half_w -container_interval);	
					y2_float		= first_element_scale;			
				}	

				/* y= ((x-x2)/(x1-x2))*(y1-y2) +y2   */
				if(x1 != x2)
				{
					scale	= (((container_center_point.x - x2)/((float_32)(x1- x2)) )) * (y1_float - y2_float) + y2_float;
				}
				else
				{
					scale	= 1.0f;
				}	
				
				if(container_center_point.x < 0)
				{
					x1			= -(SINT16)(distortion_half_w - container_half_w);
					y1_float		= first_element_alpha;
					x2			= 0;	
					y2_float		= HMI_MAX_ALPHA_VALUE;								
				}
				else
				{
					x1			= 0;
					y1_float		= HMI_MAX_ALPHA_VALUE;
					x2			= (SINT16)(distortion_half_w - container_half_w -container_interval);	
					y2_float		= first_element_alpha;			
				}	

				/* y= ((x-x2)/(x1-x2))*(y1-y2) +y2   */
				if(x1 != x2)
				{
					alpha	=(UINT8) ((((container_center_point.x - x2)/((float_32)(x1- x2)) )) * (y1_float - y2_float) + y2_float);
				}
				else
				{
					alpha	= HMI_MAX_ALPHA_VALUE;
				}	
				/*get scale position*/
				container_center_point.x	= (SINT16)(container_center_point.x * scale);
				
			}
			else
			{
				scale	= 1.0f;
				alpha	= HMI_MAX_ALPHA_VALUE;
			}
			
			/*get new position--not scale*/			
			if(fabs(scale) > HMI_FLOAT_TOLERANCE)
			{
				bscale	= TRUE;					
			}
			if(bscale == TRUE)
			{
				container_center_point.x	= (SINT16)(container_center_point.x / scale );
			}
			/*convert to coordinate.distortion display zone left top as origin*/
			origin_point.x	= (SINT16)(-distortion_display_half_w);
			origin_point.y	= (SINT16)(-distortion_display_half_h);
			HMI_TRANSFORM_COORDINATE(container_center_point,origin_point);

			/*convert to screen coordinate*/		
			container_center_point.x	+= pdistortion_rect->x;
			container_center_point.y	+= pdistortion_rect->y;
			/*return sum*/
			pcontainer_attr->alpha	= alpha;
			pcontainer_attr->scale	= scale;
			/*rect left top coordinate*/		
			pcontainer_attr->container_rect.x	= container_center_point.x - container_half_w;
			pcontainer_attr->container_rect.y	= container_center_point.y - container_half_h;
			pcontainer_attr->container_rect.w	= container_w;
			pcontainer_attr->container_rect.h	= container_h;
			/*clip zone*/
			pcontainer_attr->clip_zone.x	= pdistortion_rect->x;
			pcontainer_attr->clip_zone.y	= pdistortion_rect->y;
			pcontainer_attr->clip_zone.w	= pdistortion_rect->w;
			pcontainer_attr->clip_zone.h	= pdistortion_rect->h;
			/*left container index at distortion*/
			pcontainer_attr->index_left_distortion	= distortion_begin_index_offset.x;
		}
	}

	return	success;
}


/*
pbezier_step intersection with vector p1-->p2

*/

BOOLEAN hmi_intersection_line(SPOINT_TP *p1,SPOINT_TP *p2,SPOINT_TP *q1,SPOINT_TP *q2)
{
	BOOLEAN intersection	= TRUE;
	SINT16	min_rx			= 0;
	SINT16	min_ry			= 0;
	SINT16	max_rx			= 0;
	SINT16	max_ry			= 0;

	SINT16	min_tx			= 0;
	SINT16	min_ty			= 0;
	SINT16	max_tx			= 0;
	SINT16	max_ty			= 0;

	SINT16	min_fx			= 0;
	SINT16	min_fy			= 0;
	SINT16	max_fx			= 0;
	SINT16	max_fy			= 0;

	if((p1 != NULL)&&(p2 != NULL)&&(q1 != NULL)&&((q2 != NULL)))
	{
		/*p1--p2 ,r rect*/
		if(p1->x < p2->x)
		{
			min_rx	= p1->x;
		}
		else
		{
			min_rx	= p2->x;
		}

		if(p1->y < p2->y)
		{
			min_ry	= p1->y;
		}
		else
		{
			min_ry	= p2->y;
		}

		if(p1->x < p2->x)
		{
			max_rx	= p2->x;
		}
		else
		{
			max_rx	= p1->x;
		}

		if(p1->y < p2->y)
		{
			max_ry	= p2->y;
		}
		else
		{
			max_ry	= p1->y;
		}

		/*q1--q2 , t rect*/
		if(q1->x < q2->x)
		{
			min_tx	= q1->x;
		}
		else
		{
			min_tx	= q2->x;
		}

		if(q1->y < q2->y)
		{
			min_ty	= q1->y;
		}
		else
		{
			min_ty	= q2->y;
		}

		if(q1->x < q2->x)
		{
			max_tx	= q2->x;
		}
		else
		{
			max_tx	= q1->x;
		}

		if(q1->y < q2->y)
		{
			max_ty	= q2->y;
		}
		else
		{
			max_ty	= q1->y;
		}

		/*r rect interface t rect ---f rect*/
		if(min_rx > min_tx)
		{
			min_fx	= min_rx;
		}
		else
		{
			min_fx	= min_tx;
		}

		if(min_ry > min_ty)
		{
			min_fy	= min_ry;
		}
		else
		{
			min_fy	= min_ty;
		}

		if(max_rx > max_tx)
		{
			max_fx	= max_tx;
		}
		else
		{
			max_fx	= max_rx;
		}

		if(max_ry > max_ty)
		{
			max_fy	= max_ty;
		}
		else
		{
			max_fy	= max_ry;
		}

		if((min_fx > max_fx)||(min_fy > max_fy))
		{
			intersection	= FALSE;
		}
	}

	return	intersection;
}


/*
trail container border join with bezier begin/end point
rect centr is coordinate origin
*/
#define		HMI_RECT_BORDER_CNT		4
#define		HMI_INVALIDE_ZONE_NO	4
U08	get_joint_with_trail_container_boder(SPOINT_TP *pleft_top,SPOINT_TP *pright_bottom,
												SPOINT_TP	*pbezier_joint_normal_out,
												SPOINT_TP	*pbezier_joint_normal_in,
												SPOINT_TP	*pjoint_point)

{
	U08			joint_no			= HMI_INVALIDE_ZONE_NO;
	SPOINT_TP	prolong_point		= {0,0};
	POINT_FLOAT_TP	prolong_point_f	= {0.0f,0.0f};
	BOOLEAN		rect_intersection	= FALSE;
	BOOLEAN		line_intersection	= FALSE;
	SPOINT_TP	p1					= {0,0};
	SPOINT_TP	p2					= {0,0};	
	SPOINT_TP	joint				= {0,0};	
	SINT16		max_w_h				= 0;
	UINT16		rect_w				= 0;
	UINT16		rect_h				= 0;
	U08			loop				= 0u;

	if((pleft_top != NULL)&&(pright_bottom != NULL)&&(pbezier_joint_normal_in != NULL)&&
		(pbezier_joint_normal_out != NULL)&&(pjoint_point != NULL))
	{
		rect_w	= (UINT16)(pright_bottom->x - pleft_top->x);
		rect_h	= (UINT16)(pright_bottom->y - pleft_top->y);
		/*prolong ppoint vector*/
		if(rect_w > rect_h)
		{
			max_w_h	= rect_w;
		}
		else
		{
			max_w_h	= rect_h;
		}
		/*get vector */
		HMI_SUB_POINT2((*pbezier_joint_normal_out),(*pbezier_joint_normal_in),prolong_point);
		prolong_point_f.x	= (float_32)(prolong_point.x);
		prolong_point_f.y	= (float_32)(prolong_point.y);
		hmi_normalise_2d(&prolong_point_f);
		prolong_point.x		= (SINT16)(prolong_point_f.x * max_w_h);
		prolong_point.y		= (SINT16)(prolong_point_f.y * max_w_h);

		HMI_ADD_POINT2((*pbezier_joint_normal_in),prolong_point,prolong_point);

		/*get joint(prolong_point vector and rect border)*/
		for(loop = 0u; (line_intersection == FALSE)&&(loop < HMI_RECT_BORDER_CNT/*rect boder*/);loop++)
		{
			switch(loop)
			{
				case 0:/*Rect bottom line*/
					p1.x	= (max_w_h >> 1u);
					p1.y	= (max_w_h >> 1u);

					p2.x	= -(max_w_h >> 1u);
					p2.y	= (max_w_h >> 1u);
					break;
				case 1:/*Rect left line*/
					p1.x	= -(max_w_h >> 1u);
					p1.y	= (max_w_h >> 1u);

					p2.x	= -(max_w_h >> 1u);
					p2.y	= -(max_w_h >> 1u);
					break;
				case 2:/*Rect top line*/
					p1.x	= -(max_w_h >> 1u);
					p1.y	= -(max_w_h >> 1u);

					p2.x	= (max_w_h >> 1u);
					p2.y	= -(max_w_h >> 1u);
					break;
				case 3:/*Rect right line*/
					p1.x	= (max_w_h >> 1u);
					p1.y	= -(max_w_h >> 1u);

					p2.x	= (max_w_h >> 1u);
					p2.y	= (max_w_h >> 1u);
					break;
				default:
					p1.x	= (max_w_h >> 1u);
					p1.y	= (max_w_h >> 1u);

					p2.x	= -(max_w_h >> 1u);
					p2.y	= (max_w_h >> 1u);					
			}
			rect_intersection	= hmi_intersection_line(&p1,&p2,
												pbezier_joint_normal_in,
												&prolong_point);
			if(rect_intersection == TRUE)
			{
				line_intersection	= hmi_straddle_line(&p1,&p2,
													pbezier_joint_normal_in,
													&prolong_point);
				
			}
			if(line_intersection == TRUE)
			{
				joint		= hmi_get_interpoint_2_vector(&p1,&p2,
													pbezier_joint_normal_in,
													&prolong_point);
				joint_no	= loop;
			}
		}

		if(line_intersection == TRUE)
		{
			pjoint_point->x	= joint.x;
			pjoint_point->y	= joint.y;
		}
	}

	return	joint_no;
}

/*
CW angle
*/
void hmi_rotation_vector_2d(float_32 angle/*CW angle*/,SPOINT_TP vector[],U08 vector_len)
{	
	U08					vector_loop		= 0u;
	VECTOR_FLOAT_TP		rotation_vec_f4 = { 0,0,0,1};
	ES_MATRIX_STR		model 			= { { { 0.0f,0.0f,0.0f,0.0f },
											{ 0.0f,0.0f,0.0f,0.0f },
											{ 0.0f,0.0f,0.0f,0.0f },
											{ 0.0f,0.0f,0.0f,0.0f }
										} };

	matrix_load_identity(&model);
	matrix_rotate(&model,(angle)/*CW*/,0.0f,0.0f,1.0f,TRUE);	
	for(vector_loop = 0u;vector_loop < vector_len;vector_loop++)
	{
		rotation_vec_f4.x	= (float_32)(vector[vector_loop].x);
		rotation_vec_f4.y	= (float_32)(vector[vector_loop].y);
									
		matrix_multiply_vector2(&model,&rotation_vec_f4);
		if(rotation_vec_f4.x > 0)
		{
			vector[vector_loop].x	= (SINT16)(rotation_vec_f4.x + 0.5f);
		}
		else
		{
			vector[vector_loop].x	= (SINT16)(rotation_vec_f4.x - 0.5f);
		}

		if(rotation_vec_f4.y > 0)
		{
			vector[vector_loop].y	= (SINT16)(rotation_vec_f4.y + 0.5f);
		}
		else
		{
			vector[vector_loop].y	= (SINT16)(rotation_vec_f4.y - 0.5f);
		}		
	}
}



#ifdef HMI_SIMULATE_QD
#define HMI_GET_IMG_TEXTURE_BUF_LEN	10
void hmi_get_img_texture(HMI_OBJECT_ID_STR	element_id,HMI_IMAGE_TEXTURE_PROP *pimg_tex_prop)
{
	HMI_ELEMENT_PROP_STR object_prop	= {0};
	HMI_ELEMENT_PROP2_STR object_prop2	= {0};
	static	HMI_RECT_ALPHA_ANGEL_STR tex_rect[HMI_GET_IMG_TEXTURE_BUF_LEN];	
	static	HMI_IMAGE_ATTR_STR 		tex_attr[HMI_GET_IMG_TEXTURE_BUF_LEN];
	static	HMI_OBJECT_ID_STR		tex_id[HMI_GET_IMG_TEXTURE_BUF_LEN];
	static	U08						index=0;

	if(pimg_tex_prop != NULL)
	{
		hmi_engine_get_object_prop(element_id,&object_prop);
		hmi_engine_get_object_prop2(element_id,&object_prop2);
		
		pimg_tex_prop->ptex_prop	= object_prop2.pimage_prop_info;
		
		pimg_tex_prop->ptex_attr	= &tex_attr[index];		
		pimg_tex_prop->ptex_rect	= &tex_rect[index];
		
		/*Get texture info*/
		tex_attr[index].image_attr	= HMI_ALPHA_IMAGE_FLAG;	
		
		tex_rect[index].alpha	= object_prop.alpha;
		tex_rect[index].angel	= object_prop.angle;
		tex_rect[index].x		= object_prop.x;
		tex_rect[index].y		= object_prop.y;
		tex_rect[index].w		= object_prop.w;
		tex_rect[index].h		= object_prop.h;
		tex_rect[index].attr	= 0;

		pimg_tex_prop->	tex_id	= element_id;
		
		index++;
		if(index >= HMI_GET_IMG_TEXTURE_BUF_LEN)
		{
			index	= 0u;
		}
			
	}
}
#else
void hmi_get_img_texture(HMI_OBJECT_ID_STR	element_id,HMI_IMAGE_TEXTURE_PROP *pimg_tex_prop)
{
	
}

#endif

/*
p1,p2 vector at one line.
p1,p2 arrow is same
*/
BOOLEAN	hmi_same_arrow_2vec_at_one_line(SPOINT_TP 	*p1,SPOINT_TP	*p2)
{
	BOOLEAN		same	= FALSE;

	if((p1 != NULL)&&(p2 != NULL))
	{
		if((p1->x != 0)&&(p1->y != 0))
		{
			if((p1->x * p2->y) == (p1->y * p2->x))
			{
				same	= TRUE;/*2 Vector arrow same*/
			}
			else
			{
				same	= FALSE;/*2 Vector arrow not same*/
			}
		}
		else if(p1->x != 0)
		{
			if((p1->x * p2->x) > 0)
			{
				same	= TRUE;/*2 Vector arrow same*/
			}
			else
			{
				same	= FALSE;/*2 Vector arrow not same*/
			}
		}
		else
		{
			if((p1->y * p2->y) > 0)
			{
				same	= TRUE;/*2 Vector arrow same*/
			}
			else
			{
				same	= FALSE;/*2 Vector arrow not same*/
			}
		}
	}
	
	return	same;
}






void hmi_engine_draw_custom_lib(HMI_RECT_STR CONST 	*pcustom_farther_rect,
											HMI_RECT_STR			*pcustom_dirty_rect,
											U08						custom_depth,
											HMI_RECT_STR			*pcustom_cliped_farther_rect,
											HMI_OBJECT_ID_STR		custom_id,											
											HMI_ALPHA_SCALE_PT_STR *pfather_alpha_scale
										#if defined(HMI_GRAPHIC_OPENGLES)
											,void					*puser_data
										#endif
											)


{

}


#endif


#if HMI_DXY_CUSTOM_CNT >0U
HMI_CUSTOM_INFO_STR CONST* hmi_engine_get_dxy_custom_addr(void)
{
	return hmi_dxy_custom_widget_info;
}
#endif

#if HMI_SXY_CUSTOM_CNT >0U
HMI_CUSTOM_INFO_STR CONST* hmi_engine_get_sxy_custom_addr(void)
{
	return hmi_sxy_custom_widget_info;
}
#endif

//#endif




#define HMI_CANE_BEZIER_STEP		  		500//100//200//100//38

BOOLEAN hmi_judge_point_zone2(HMI_RECT_STR 			 *prect_t,POINT32_TP *point_t)
{
	BOOLEAN ret		= FALSE;

	if((prect_t != NULL)&&(point_t != NULL))
	{
		if((point_t->x >= prect_t->x)&&(point_t->x < (prect_t->x + prect_t->w)))
		{
			if((point_t->y >= prect_t->y)&&(point_t->y<(prect_t->y + prect_t->h)))
			{
				ret		= TRUE;
			}
		}
	}
	
	return ret;
}


BOOLEAN hmi_judge_point_zone2_f(HMI_RECT_STR 			 *prect_t,POINT_FLOAT_TP *point_t)
{
	BOOLEAN ret		= FALSE;

	if((prect_t != NULL)&&(point_t != NULL))
	{
		if((point_t->x >= prect_t->x)&&(point_t->x < (prect_t->x + prect_t->w)))
		{
			if((point_t->y >= prect_t->y)&&(point_t->y<(prect_t->y + prect_t->h)))
			{
				ret		= TRUE;
			}
		}
	}
	
	return ret;
}







#if 1



void	hmi_one_bezier_clip_rect(POINT32_TP	point[],INT32	point_len,HMI_RECT_STR	*phmi_lane_zone_clip)
{
	BOOLEAN						bbegin_in_clip		= FALSE;
	BOOLEAN						bend_in_clip		= FALSE;
	BOOLEAN						bbegin_new			= FALSE;
	BOOLEAN						bend_new			= FALSE;
	POINT32_TP					p0					= {0};
	POINT32_TP					pi					= {0};
	SINT32						dx					= 0u;
	SINT32						dy					= 0u;	
	SINT32						sum					= 0u;	
	float_32					tt					= 0.0f;
	float_32					step				= 0.0f;
	INT32						i					= 0;
	float_32					begin_t				= 0.0f;
	float_32					end_t				= 1.0f;
	INT32						rect_right			= 0;	
	INT32						rect_bottom			= 0;
	INT32						rect_left			= 0;	
	INT32						rect_top			= 0;
	SINT32						sum_t1				= -1;	
	POINT32_TP					new_beg_ctl			= {0};
	POINT32_TP					new_end_ctl			= {0};
	POINT32_TP					mid					= {0};
	POINT32_TP					mid_ctrl0			= {0};
	POINT32_TP					mid_ctrl1			= {0};
	POINT32_TP					new_mid				= {0};
	POINT32_TP					new_mid_ctrl0		= {0};
	POINT32_TP					new_mid_ctrl1		= {0};
	
	if((phmi_lane_zone_clip != NULL)&&(point_len >= 4/*cublic bezier point cnt*/))
	{
		rect_left		= (INT32)(phmi_lane_zone_clip->x);
		rect_top		= (INT32)(phmi_lane_zone_clip->y);
		rect_right		= (INT32)(phmi_lane_zone_clip->x + phmi_lane_zone_clip->w);
		rect_bottom		= (INT32)(phmi_lane_zone_clip->y + phmi_lane_zone_clip->h);
		
		bbegin_in_clip	= hmi_judge_point_zone2(phmi_lane_zone_clip,&point[0/*begin*/]);		
		bend_in_clip	= hmi_judge_point_zone2(phmi_lane_zone_clip,&point[3/*end*/]);
		step	= (float_32)(1.0f / HMI_CANE_BEZIER_STEP);	
		
		if((bbegin_in_clip == FALSE)||(bend_in_clip == FALSE))
		{
			p0	= hor(3,point,tt);
			i	= 1;
			for(tt = step;tt < 1.0f + HMI_FLOAT_TOLERANCE;tt = tt + step)
			{
				pi	=	hor(3,point,tt);
				if(p0.x > pi.x)
				{
					dx	= (p0.x - pi.x);
				}
				else
				{
					dx	= (pi.x - p0.x);
				}
				if(p0.y > pi.y)
				{
					dy	= (p0.y - pi.y);
				}
				else
				{
					dy	= (pi.y - p0.y);
				}
							
				sum	+= (dx + dy);
				if((bbegin_in_clip == FALSE)&&(bend_in_clip == TRUE))
				{
					if((pi.x >= rect_left)&&(pi.x < rect_right))
					{
						if((pi.y >= rect_top)&&(pi.y < rect_bottom))
						{
							begin_t	=  i * step;
							if(begin_t > 1.0f)
							{
								begin_t	= 1.0f;
							}														
							
							new_beg_ctl.x	= point[1/*begin ctrl*/].x;
							new_beg_ctl.y	= point[1/*begin ctrl*/].y;
							new_end_ctl.x	= point[2/*end ctrl*/].x;
							new_end_ctl.y	= point[2/*end ctrl*/].y;

							hmi_get_midpoint_ctrl(begin_t,
									&point[0]/*begin*/,
									&new_beg_ctl/*begin ctrl*/,
									&new_end_ctl/*end ctrl*/,
									&point[3]/*end*/,
									&mid/*mid*/,
									&mid_ctrl0/*mid ctrl 0*/,
									&mid_ctrl1/*mid ctrl 1*/);
								
							point[0].x	= mid.x;/*begin*/
							point[0].y	= mid.y;/*begin*/
							
							point[1].x	= mid_ctrl1.x;/*begin ctrl*/
							point[1].y	= mid_ctrl1.y;/*begin ctrl*/
							
							point[2].x	= new_end_ctl.x;/*end ctrl*/
							point[2].y	= new_end_ctl.y;/*end ctrl*/
							/*point[3] not change*/
							tt	= 2.0f;/*end for loop*/
						}
					}
				}
				else if((bbegin_in_clip == TRUE)&&(bend_in_clip == FALSE))
				{
					if((pi.x < rect_left)||(pi.x > rect_right)
						||(pi.y < rect_top)||(pi.y > rect_bottom))
					{		
						begin_t	=  i * step;
						if(begin_t > 1.0f)
						{
							begin_t	= 1.0f;
						}
												
						new_beg_ctl.x	= point[1/*begin ctrl*/].x;
						new_beg_ctl.y	= point[1/*begin ctrl*/].y;
						new_end_ctl.x	= point[2/*end ctrl*/].x;
						new_end_ctl.y	= point[2/*end ctrl*/].y;

						hmi_get_midpoint_ctrl(begin_t,
								&point[0]/*begin*/,
								&new_beg_ctl/*begin ctrl*/,
								&new_end_ctl/*end ctrl*/,
								&point[3]/*end*/,
								&mid/*mid*/,
								&mid_ctrl0/*mid ctrl 0*/,
								&mid_ctrl1/*mid ctrl 1*/);
						/*point[0] not change*/													
						point[1].x	= new_beg_ctl.x;/*begin ctrl*/
						point[1].y	= new_beg_ctl.y;/*begin ctrl*/

						point[2].x	= mid_ctrl0.x;/*begin ctrl*/
						point[2].y	= mid_ctrl0.y;/*begin ctrl*/
						
						point[3].x	= mid.x;/*end */
						point[3].y	= mid.y;/*end */
						tt	= 2.0f;/*end for loop*/
					}	
				}
				else if((bbegin_in_clip == FALSE)&&(bend_in_clip == FALSE))
				{
					if(bbegin_new == FALSE)
					{
						if((pi.x >= rect_left)&&(pi.x < rect_right))
						{
							if((pi.y >= rect_top)&&(pi.y < rect_bottom))
							{
								begin_t	=  i * step;
								if(begin_t > 1.0f)
								{
									begin_t	= 1.0f;
								}																						
								bbegin_new			= TRUE;
								sum					= 0;
							}
						}
					}
					else
					{
						if(sum_t1 < 0)
						{
							if((pi.x < rect_left)||(pi.x > rect_right)
								||(pi.y < rect_top)||(pi.y > rect_bottom))
							{
								sum_t1	= sum;								
							}
						}
					}
				}
				else{}																				
				p0	= pi;
				i++;
			}

			if((bbegin_in_clip == FALSE)&&(bend_in_clip == FALSE))
			{
				new_beg_ctl.x	= point[1/*begin ctrl*/].x;
				new_beg_ctl.y	= point[1/*begin ctrl*/].y;
				new_end_ctl.x	= point[2/*end ctrl*/].x;
				new_end_ctl.y	= point[2/*end ctrl*/].y;
																		
				hmi_get_midpoint_ctrl(begin_t,
									&point[0]/*begin*/,
									&new_beg_ctl/*begin ctrl*/,
									&new_end_ctl/*end ctrl*/,
									&point[3]/*end*/,
									&mid/*mid*/,
									&mid_ctrl0/*mid ctrl 0*/,
									&mid_ctrl1/*mid ctrl 1*/);
				
				if(sum > 0)
				{
					end_t	= (float_32)(sum_t1 / (float_32)sum);					
					if(end_t > 1.0f)
					{
						end_t	= 1.0f;
					}					
					hmi_get_midpoint_ctrl(end_t,
										&mid/*begin*/,
										&mid_ctrl1/*begin ctrl*/,
										&new_end_ctl/*end ctrl*/,
										&point[3]/*end*/,
										&new_mid/*mid*/,
										&new_mid_ctrl0/*mid ctrl 0*/,
										&new_mid_ctrl1/*mid ctrl 1*/);
					
					point[0].x	= mid.x;/*begin*/
					point[0].y	= mid.y;/*begin*/
					
					point[1].x	= mid_ctrl1.x;/*begin ctrl*/
					point[1].y	= mid_ctrl1.y;/*begin ctrl*/
					
					point[2].x	= new_mid_ctrl0.x;/*end ctrl*/
					point[2].y	= new_mid_ctrl0.y;/*end ctrl*/
					
					point[3].x	= new_mid.x;/*end*/
					point[3].y	= new_mid.y;/*end*/					
				}										
			}		
			}
		}
}

BOOLEAN	hmi_get_midpoint_ctrl_f(float_32 e,POINT_FLOAT_TP	*pbegin,
									POINT_FLOAT_TP	*pbeginControl,
									POINT_FLOAT_TP	*pendControl,
									POINT_FLOAT_TP	*pend,
									POINT_FLOAT_TP	*pmid,
									POINT_FLOAT_TP	*pmidControl1,
									POINT_FLOAT_TP	*pmidControl2);


void	hmi_one_bezier_clip_rect_f(POINT_FLOAT_TP	point[],INT32	point_len,HMI_RECT_STR	*phmi_lane_zone_clip)
{		
	BOOLEAN						bbegin_in_clip		= FALSE;
	BOOLEAN						bend_in_clip		= FALSE;
	BOOLEAN						bbegin_new			= FALSE;
	BOOLEAN						bend_new			= FALSE;
	POINT_FLOAT_TP				p0					= {0.0f};
	POINT_FLOAT_TP				pi					= {0.0f};
	float_32					dx					= 0.0f;
	float_32					dy					= 0.0f;	
	float_32					sum					= 0.0f;	
	float_32					tt					= 0.0f;
	float_32					step				= 0.0f;
	INT32						i					= 0;
	float_32					begin_t				= 0.0f;
	float_32					end_t				= 1.0f;
	INT32						rect_right			= 0;	
	INT32						rect_bottom			= 0;
	INT32						rect_left			= 0;	
	INT32						rect_top			= 0;
	float_32					sum_t1				= -1.0f;	
	POINT_FLOAT_TP				new_beg_ctl			= {0};
	POINT_FLOAT_TP				new_end_ctl			= {0};
	POINT_FLOAT_TP				mid					= {0};
	POINT_FLOAT_TP				mid_ctrl0			= {0};
	POINT_FLOAT_TP				mid_ctrl1			= {0};
	POINT_FLOAT_TP				new_mid				= {0};
	POINT_FLOAT_TP				new_mid_ctrl0		= {0};
	POINT_FLOAT_TP				new_mid_ctrl1		= {0};
	
	if((phmi_lane_zone_clip != NULL)&&(point_len >= 4/*cublic bezier point cnt*/))
	{
		rect_left		= (INT32)(phmi_lane_zone_clip->x);
		rect_top		= (INT32)(phmi_lane_zone_clip->y);
		rect_right		= (INT32)(phmi_lane_zone_clip->x + phmi_lane_zone_clip->w);
		rect_bottom		= (INT32)(phmi_lane_zone_clip->y + phmi_lane_zone_clip->h);

		
			
		bbegin_in_clip	= hmi_judge_point_zone2_f(phmi_lane_zone_clip,&point[0/*begin*/]);		
		bend_in_clip	= hmi_judge_point_zone2_f(phmi_lane_zone_clip,&point[3/*end*/]);
		step	= (float_32)(1.0f / HMI_CANE_BEZIER_STEP);	
		
		if((bbegin_in_clip == FALSE)||(bend_in_clip == FALSE))
		{
			p0	= hor_f_point_f(3,point,tt);
			i	= 1;
			for(tt = step;tt < 1.0f + HMI_FLOAT_TOLERANCE;tt = tt + step)
			{
				pi	=	hor_f_point_f(3,point,tt);
				if(p0.x > pi.x)
				{
					dx	= (p0.x - pi.x);
				}
				else
				{
					dx	= (pi.x - p0.x);
				}
				if(p0.y > pi.y)
				{
					dy	= (p0.y - pi.y);
				}
				else
				{
					dy	= (pi.y - p0.y);
				}
							
				sum	+= (dx + dy);
				if((bbegin_in_clip == FALSE)&&(bend_in_clip == TRUE))
				{
					if((pi.x >= rect_left)&&(pi.x < rect_right))
					{
						if((pi.y >= rect_top)&&(pi.y < rect_bottom))
						{
							begin_t	=  i * step;
							if(begin_t > 1.0f)
							{
								begin_t	= 1.0f;
							}														
							
							new_beg_ctl.x	= point[1/*begin ctrl*/].x;
							new_beg_ctl.y	= point[1/*begin ctrl*/].y;
							new_end_ctl.x	= point[2/*end ctrl*/].x;
							new_end_ctl.y	= point[2/*end ctrl*/].y;

							hmi_get_midpoint_ctrl_f(begin_t,
									&point[0]/*begin*/,
									&new_beg_ctl/*begin ctrl*/,
									&new_end_ctl/*end ctrl*/,
									&point[3]/*end*/,
									&mid/*mid*/,
									&mid_ctrl0/*mid ctrl 0*/,
									&mid_ctrl1/*mid ctrl 1*/);
								
							point[0].x	= mid.x;/*begin*/
							point[0].y	= mid.y;/*begin*/
							
							point[1].x	= mid_ctrl1.x;/*begin ctrl*/
							point[1].y	= mid_ctrl1.y;/*begin ctrl*/
							
							point[2].x	= new_end_ctl.x;/*end ctrl*/
							point[2].y	= new_end_ctl.y;/*end ctrl*/
							/*point[3] not change*/
							tt	= 2.0f;/*end for loop*/
						}
					}
				}
				else if((bbegin_in_clip == TRUE)&&(bend_in_clip == FALSE))
				{
					if((pi.x < rect_left)||(pi.x > rect_right)
						||(pi.y < rect_top)||(pi.y > rect_bottom))
					{		
						begin_t	=  i * step;
						if(begin_t > 1.0f)
						{
							begin_t	= 1.0f;
						}
												
						new_beg_ctl.x	= point[1/*begin ctrl*/].x;
						new_beg_ctl.y	= point[1/*begin ctrl*/].y;
						new_end_ctl.x	= point[2/*end ctrl*/].x;
						new_end_ctl.y	= point[2/*end ctrl*/].y;

						hmi_get_midpoint_ctrl_f(begin_t,
								&point[0]/*begin*/,
								&new_beg_ctl/*begin ctrl*/,
								&new_end_ctl/*end ctrl*/,
								&point[3]/*end*/,
								&mid/*mid*/,
								&mid_ctrl0/*mid ctrl 0*/,
								&mid_ctrl1/*mid ctrl 1*/);
						/*point[0] not change*/													
						point[1].x	= new_beg_ctl.x;/*begin ctrl*/
						point[1].y	= new_beg_ctl.y;/*begin ctrl*/

						point[2].x	= mid_ctrl0.x;/*begin ctrl*/
						point[2].y	= mid_ctrl0.y;/*begin ctrl*/
						
						point[3].x	= mid.x;/*end */
						point[3].y	= mid.y;/*end */
						tt	= 2.0f;/*end for loop*/
					}	
				}
				else if((bbegin_in_clip == FALSE)&&(bend_in_clip == FALSE))
				{
					if(bbegin_new == FALSE)
					{
						if((pi.x >= rect_left)&&(pi.x < rect_right))
						{
							if((pi.y >= rect_top)&&(pi.y < rect_bottom))
							{
								begin_t	=  i * step;
								if(begin_t > 1.0f)
								{
									begin_t	= 1.0f;
								}																						
								bbegin_new			= TRUE;
								sum					= 0;
							}
						}
					}
					else
					{
						if(sum_t1 < 0)
						{
							if((pi.x < rect_left)||(pi.x > rect_right)
								||(pi.y < rect_top)||(pi.y > rect_bottom))
							{
								sum_t1	= sum;								
							}
						}
					}
				}
				else{}																				
				p0	= pi;
				i++;
			}

			if((bbegin_in_clip == FALSE)&&(bend_in_clip == FALSE))
			{
				new_beg_ctl.x	= point[1/*begin ctrl*/].x;
				new_beg_ctl.y	= point[1/*begin ctrl*/].y;
				new_end_ctl.x	= point[2/*end ctrl*/].x;
				new_end_ctl.y	= point[2/*end ctrl*/].y;
																		
				hmi_get_midpoint_ctrl_f(begin_t,
									&point[0]/*begin*/,
									&new_beg_ctl/*begin ctrl*/,
									&new_end_ctl/*end ctrl*/,
									&point[3]/*end*/,
									&mid/*mid*/,
									&mid_ctrl0/*mid ctrl 0*/,
									&mid_ctrl1/*mid ctrl 1*/);
				
				if(sum > 0)
				{
					end_t	= (float_32)(sum_t1 / (float_32)sum);					
					if(end_t > 1.0f)
					{
						end_t	= 1.0f;
					}					
					hmi_get_midpoint_ctrl_f(end_t,
										&mid/*begin*/,
										&mid_ctrl1/*begin ctrl*/,
										&new_end_ctl/*end ctrl*/,
										&point[3]/*end*/,
										&new_mid/*mid*/,
										&new_mid_ctrl0/*mid ctrl 0*/,
										&new_mid_ctrl1/*mid ctrl 1*/);
					
					point[0].x	= mid.x;/*begin*/
					point[0].y	= mid.y;/*begin*/
					
					point[1].x	= mid_ctrl1.x;/*begin ctrl*/
					point[1].y	= mid_ctrl1.y;/*begin ctrl*/
					
					point[2].x	= new_mid_ctrl0.x;/*end ctrl*/
					point[2].y	= new_mid_ctrl0.y;/*end ctrl*/
					
					point[3].x	= new_mid.x;/*end*/
					point[3].y	= new_mid.y;/*end*/					
				}										
			}		
			}
		}
}



#define HMI_INT_TOLERANCE	3
INT32	hmi_get_one_bezier_to_centery(POINT32_TP	point[],INT32	point_len,POINT_FLOAT_TP	*pcenter)
{
	POINT32_TP					p0					= {0};
	POINT32_TP					pi					= {0};
	SINT32						dx					= 0u;
	SINT32						dy					= 0u;	
	SINT32						sum					= 0u;	
	float_32					tt					= 0.0f;
	float_32					step				= 0.0f;
	float_32					dt					= 0.0f;
	if((pcenter != NULL)&&(point_len >= 4/*cublic bezier point cnt*/))
	{				
		step	= (float_32)(1.0f / HMI_CANE_BEZIER_STEP);					
		p0		= hor(3,point,tt);			
		for(tt = step;tt < 1.0f + HMI_FLOAT_TOLERANCE;tt = tt + step)
		{
			pi	=	hor(3,point,tt);
			if(p0.x > pi.x)
			{
				dx	= (p0.x - pi.x);
			}
			else
			{
				dx	= (pi.x - p0.x);
			}
			if(p0.y > pi.y)
			{
				dy	= (p0.y - pi.y);
			}
			else
			{
				dy	= (pi.y - p0.y);
			}
			sum	+= (dx + dy);
			p0	= pi;
			dt	= pi.y - pcenter->y;
			if((dt > (-HMI_INT_TOLERANCE))&&
				(dt < HMI_INT_TOLERANCE))
			{
				tt	= 2.0f;// end for
			}																													
		}							
	}
	return	sum;
}

INT32	hmi_get_one_bezier_to_centery_f(POINT_FLOAT_TP	point[],INT32	point_len,POINT_FLOAT_TP	*pcenter)
{
	INT32						sum_i				= 0;
	POINT_FLOAT_TP				p0					= {0.0f};
	POINT_FLOAT_TP				pi					= {0.0f};
	float_32					dx					= 0.0f;
	float_32					dy					= 0.0f;	
	float_32					sum					= 0.0f;	
	float_32					tt					= 0.0f;
	float_32					step				= 0.0f;
	float_32					dt					= 0.0f;
	
		
	if((pcenter != NULL)&&(point_len >= 4/*cublic bezier point cnt*/))
	{				
		step	= (float_32)(1.0f / HMI_CANE_BEZIER_STEP);					
		p0		= hor_f_point_f(3,point,tt);			
		for(tt = step;tt < 1.0f + HMI_FLOAT_TOLERANCE;tt = tt + step)
		{
			pi	=	hor_f_point_f(3,point,tt);
			if(p0.x > pi.x)
			{
				dx	= (p0.x - pi.x);
			}
			else
			{
				dx	= (pi.x - p0.x);
			}
			if(p0.y > pi.y)
			{
				dy	= (p0.y - pi.y);
			}
			else
			{
				dy	= (pi.y - p0.y);
			}
						
			sum	+= (dx + dy);
			p0	= pi;
			dt	= pi.y - pcenter->y;
			if((dt > (-HMI_INT_TOLERANCE))&&
				(dt < HMI_INT_TOLERANCE))
			{
				tt	= 2.0f;// end for
			}																													
		}							
	}

	sum_i	= (INT32)(sum + 0.5f);

	return	sum_i;
}


static float_32 DegreeToPI = 3.14159265f / 180.0f; 

U08	hmi_circle_to_squre_bezier(POINT_FLOAT_TP *pcircle_center,float_32 radius,
									POINT_FLOAT_TP	path_data32[],U08	path_data32_array_len)
{
	U08		path_data32_len	= 0u;
	float_32 angle = 0;
	float_32 r2 = 0;
	static const U08 cnt = 8;
	U08 i;
	POINT_FLOAT_TP p1;
	POINT_FLOAT_TP p2;
	U32 index =0;

	if(pcircle_center != NULL)
	{		   
	    r2 = radius / cosf((DegreeToPI * 180.0f) / (float_32)cnt);
	    for (i = 0; i < cnt; i++)
	    {
	        angle = (360.0f * ((float_32)i+0.5f)) / (float_32)cnt;
	        p1.x = pcircle_center->x + (sinf(DegreeToPI * angle) * r2);
	        p1.y = pcircle_center->y + (cosf(DegreeToPI * angle) * r2);

	        angle = (360.0f * ((float_32)i+1)) / (float_32)cnt;
	        p2.x = pcircle_center->x + (sinf(DegreeToPI * angle) * radius);
	        p2.y = pcircle_center->y + (cosf(DegreeToPI * angle) * radius);
			path_data32[index].x =p1.x;
			path_data32[index].y =p1.y;
			index++;
			path_data32[index].x =p2.x;
			path_data32[index].y =p2.y;
			index++;
	    }
	
	}

	return	path_data32_len;
}

/*
circle coordinate y up
*/
#if 0
U08	hmi_circle_to_cubic_bezier(POINT_FLOAT_TP *pcircle_center,float_32 radius,
									POINT_FLOAT_TP	path_data_f[],U08	path_data_f_array_len,
									INT32 w,HMI_PATH_DOTTED_STR	*ppath_dotted_type,
									HMI_RECT_STR	*phmi_lane_zone_clip)
{
	U08					path_data32_len									= 0u;	
	U08					one_bezier_data32_len							= 0u;	
	float_32			contrl_len 										= 0.0f;
	float_32			ctrl_p											= 0.0f;
	INT32				index											= 0u;
	INT32				path_data_f_index								= 0u;
	POINT32_TP			point[HMI_HALF_CIRCLE_CTL_CNT]					= {{0,0},{0,0},{0,0},{0,0}};	
	BOOLEAN  			bleft_line										= FALSE;	
	POINT32_TP			*ppoint											= NULL;
	INT32				point_len										= 0;
	INT32				expand_bezier_index								= 0;
	INT32				i												= 0;	
	POINT32_TP			lane_expand_path_data32[8]						= {0};	
	BOOLEAN				bdot_line										= FALSE;	
	POINT32_TP			dot_line_path_data32[HMI_CANE_SOLID_PATH_MAX_LEN ]		= {{0,0}};	
	
	if(radius > 0.0f)
	{
		bleft_line			= FALSE;
	}
	else
	{
		bleft_line			= TRUE;
		radius				= -radius;
	}
	
	if(pcircle_center != NULL)
	{
		contrl_len = (float_32)((4 / 3.0) * radius + 0.5f);
		if(radius ==0)
		{
			contrl_len	= 2.0f;
			radius		= HMI_LANE_SOLID_LINE_LEN; 
			
		}
			
		if(bleft_line == TRUE)
		{
			if(pcircle_center->x > 0.0f)
			{
				point[3].x	= (SINT32)(pcircle_center->x + 0.5f);
			}
			else
			{
				point[3].x	= (SINT32)(pcircle_center->x - 0.5f);
			}

			ctrl_p	= pcircle_center->y - radius;
			if(ctrl_p > 0.0f)
			{
				point[3].y	= (SINT32)(ctrl_p + 0.5f);
			}
			else
			{
				point[3].y	= (SINT32)(ctrl_p - 0.5f);
			}
			//
			ctrl_p	= pcircle_center->x - contrl_len;
			if(ctrl_p > 0.0f)
			{
				point[2].x	=(SINT32)(ctrl_p + 0.5f);
			}
			else
			{
				point[2].x	=(SINT32)(ctrl_p - 0.5f);
			}

			point[2].y	= point[3].y;
			//
			point[1].x	= point[2].x;			

			ctrl_p	= pcircle_center->y + radius;
			if(ctrl_p > 0.0f)
			{
				point[1].y	=(SINT32)(ctrl_p + 0.5f);
			}
			else
			{
				point[1].y	=(SINT32)(ctrl_p - 0.5f);
			}
			//
			point[0].x = point[3].x;

			ctrl_p	= pcircle_center->y + radius;
			if(ctrl_p > 0.0f)
			{
				point[0].y	=(SINT32)(ctrl_p + 0.5f);
			}
			else
			{
				point[0].y	=(SINT32)(ctrl_p - 0.5f);
			}							
		}
		else
		{
			if(pcircle_center->x > 0.0f)
			{
				point[0].x	= (SINT32)(pcircle_center->x + 0.5f);
			}
			else
			{
				point[0].x	= (SINT32)(pcircle_center->x - 0.5f);
			}

			ctrl_p	= pcircle_center->y + radius;
			if(ctrl_p > 0.0f)
			{
				point[0].y	= (SINT32)(ctrl_p + 0.5f);
			}
			else
			{
				point[0].y	= (SINT32)(ctrl_p - 0.5f);
			}
			//
			ctrl_p	= pcircle_center->x + contrl_len;
			if(ctrl_p > 0.0f)
			{
				point[1].x	=(SINT32)(ctrl_p + 0.5f);
			}
			else
			{
				point[1].x	=(SINT32)(ctrl_p - 0.5f);
			}

			point[1].y	= point[0].y;
			
			//
			point[2].x	= point[1].x;			
			ctrl_p	= pcircle_center->y - radius;
			if(ctrl_p > 0.0f)
			{
				point[2].y	=(SINT32)(ctrl_p + 0.5f);
			}
			else
			{
				point[2].y	=(SINT32)(ctrl_p - 0.5f);
			}
			//
			point[3].x	= point[0].x;			
			ctrl_p	= pcircle_center->y - radius;
			if(ctrl_p > 0.0f)
			{
				point[3].y	=(SINT32)(ctrl_p + 0.5f);
			}
			else
			{
				point[3].y	=(SINT32)(ctrl_p - 0.5f);
			}	
		}
		/*y axia down*/
		point[0].y = -point[0].y;
		point[1].y = -point[1].y;
		point[2].y = -point[2].y;
		point[3].y = -point[3].y;
		/*phmi_lane_zone_clip	clipe	Bezier point */
		/*if((ppath_dotted_type->path_solid_len == 0)||
				(ppath_dotted_type->path_dotted_len == 0))*/
		{
			hmi_one_bezier_clip_rect(point,HMI_HALF_CIRCLE_CTL_CNT,phmi_lane_zone_clip);
		}

		point_len	= HMI_HALF_CIRCLE_CTL_CNT;
		ppoint		= point;
		/*if dot line,split int multi bezier*/
		if(ppath_dotted_type != NULL)
		{
			if((ppath_dotted_type->path_solid_len > 0)&&
				(ppath_dotted_type->path_dotted_len > 0))
			{
				point_len	= hmi_cubic_bezier_to_mul_bezier(point,
											HMI_HALF_CIRCLE_CTL_CNT,
											dot_line_path_data32,
											HMI_CANE_SOLID_PATH_MAX_LEN,
											ppath_dotted_type->path_solid_len,
											ppath_dotted_type->path_dotted_len,
											ppath_dotted_type->path_offset);				
				ppoint	= dot_line_path_data32;
				bdot_line	= TRUE;
			}
		}

						
		for(index = 0;index < point_len;index += HMI_CUBLIC_BEZIER_CTL_CNT)
		{					
			one_bezier_data32_len		= hmi_line_to_zone(&ppoint[index], 
												HMI_CUBLIC_BEZIER_CTL_CNT, 
												lane_expand_path_data32, 
												8/*2 cublic bezier ctrl cnt*/,
												w);
			
			path_data32_len	+= one_bezier_data32_len;
			expand_bezier_index++;
			for(i = 0;i < 8;i++)
			{
				if(path_data_f_index  < path_data_f_array_len)
				{
					path_data_f[path_data_f_index].x	= (float_32)(lane_expand_path_data32[i].x);
					path_data_f[path_data_f_index].y	= (float_32)(lane_expand_path_data32[i].y);
					path_data_f_index++;
				}
			}							
		}
		if(path_data32_len > path_data_f_array_len)
		{
			path_data32_len	= path_data_f_array_len;
		}													
	}

	return	path_data32_len;
}
#endif

static float_32 line_len =0.48f;
U08	hmi_circle_to_cubic_bezier(POINT_FLOAT_TP *pcircle_center,float_32 radius,
									POINT_FLOAT_TP	path_data_f[],U08	path_data_f_array_len,
									INT32 w,HMI_PATH_DOTTED_STR	*ppath_dotted_type,
									HMI_RECT_STR	*phmi_lane_zone_clip)
{
	U08					path_data32_len									= 0u;	
	U08					one_bezier_data32_len							= 0u;	
	float_32			contrl_len 										= 0.0f;
	float_32			ctrl_p											= 0.0f;
	INT32				index											= 0u;
	INT32				path_data_f_index								= 0u;
	POINT_FLOAT_TP		point[HMI_HALF_CIRCLE_CTL_CNT]					= {{0.0f,0.0f},{0.0f,0.0f},{0.0f,0.0f},{0.0f,0.0f}};	
	BOOLEAN  			bleft_line										= FALSE;	
	POINT_FLOAT_TP		*ppoint											= NULL;
	INT32				point_len										= 0;
	INT32				expand_bezier_index								= 0;
	INT32				i												= 0;	
	POINT_FLOAT_TP		lane_expand_path_data32[HMI_LANE_LINE_POINT_NB]	= {0.0f};	
	BOOLEAN				bdot_line										= FALSE;	
	POINT_FLOAT_TP		dot_line_path_data32[HMI_CANE_SOLID_PATH_MAX_LEN ]		= {{0.0f,0.0f}};	
	BOOLEAN				bline											= FALSE;	
	SINT32				center_length									= 0;
	POINT_FLOAT_TP		new_beg_ctl		= {0.0f};
	POINT_FLOAT_TP		new_end_ctl		= {0.0f};
	POINT_FLOAT_TP		mid				= {0.0f};
	POINT_FLOAT_TP		mid_ctrl0		= {0.0f};
	POINT_FLOAT_TP		mid_ctrl1		= {0.0f};
	HMI_RECT_STR		dot_line_zone ={0};
	if(radius > 0.0f)
	{
		bleft_line			= FALSE;
	}
	else if(radius < 0.0f)
	{
		bleft_line			= TRUE;
		radius				= -radius;
	}
	else
	{
		bline	= TRUE;
	}
	
	if(pcircle_center != NULL)
	{
		if(bline == FALSE)
		{
			contrl_len = (float_32)((4 / 3.0) * radius + 0.5f);		
				
			if(bleft_line == TRUE)
			{
				point[3].x	= pcircle_center->x ;
				
				ctrl_p	= pcircle_center->y - radius;
				point[3].y	= ctrl_p ;
				
				//
				ctrl_p	= pcircle_center->x - contrl_len;
				point[2].x	=ctrl_p ;
				
				point[2].y	= point[3].y;
				//
				point[1].x	= point[2].x;			

				ctrl_p	= pcircle_center->y + radius;
				point[1].y	=ctrl_p ;
				
				//
				point[0].x = point[3].x;

				ctrl_p	= pcircle_center->y + radius;
				point[0].y	=ctrl_p ;
										
			}
			else
			{
				point[0].x	= pcircle_center->x ;								

				ctrl_p	= pcircle_center->y + radius;
				point[0].y	= ctrl_p ;
				
				//
				ctrl_p	= pcircle_center->x + contrl_len;
				point[1].x	=ctrl_p ;
				

				point[1].y	= point[0].y;
				//
				point[2].x	= point[1].x;			

				ctrl_p	= pcircle_center->y - radius;
				point[2].y	=ctrl_p ;
				
				//
				point[3].x = point[0].x;

				ctrl_p	= pcircle_center->y - radius;
				point[3].y	=ctrl_p ;				
			}
		}
		else	/*line*/
		{
			if(phmi_lane_zone_clip != NULL)
			{
				point[0].x	= (pcircle_center->x/*half road width*/);
				point[0].y	= (float_32)(phmi_lane_zone_clip->y);
				point[0].y++;/*point[0],in phmi_lane_zone_clip zone*/

				point[1].x	= point[0].x;
				point[1].y	= point[0].y + 1/*ctrl point*/;

				point[3].x	= point[1].x;
				point[3].y	= (float_32)((phmi_lane_zone_clip->y + phmi_lane_zone_clip->h)) ;
				point[3].y--;/*point[3],in phmi_lane_zone_clip zone*/

				point[2].x	= point[3].x;
				point[2].y	= point[3].y - 1/*ctrl point*/;
			}
		}
		/*y axia down*/
		if(bline == FALSE)
		{
			point[0].y = -point[0].y;
			point[1].y = -point[1].y;
			point[2].y = -point[2].y;
			point[3].y = -point[3].y;
		}
		point_len	= HMI_HALF_CIRCLE_CTL_CNT;
		ppoint		= point;
		/*if dot line,split int multi bezier*/
		if(ppath_dotted_type != NULL)
		{
			if((ppath_dotted_type->path_solid_len > 0)&&
				(ppath_dotted_type->path_dotted_len > 0))
			{
			#if 1
				{
					hmi_one_bezier_clip_rect_f(point,HMI_HALF_CIRCLE_CTL_CNT,phmi_lane_zone_clip);
				}
			#endif
				center_length	= hmi_get_one_bezier_to_centery_f(point,
											HMI_HALF_CIRCLE_CTL_CNT,
											pcircle_center);
				center_length	+= ppath_dotted_type->path_offset;
				point_len	= hmi_cubic_bezier_to_mul_bezier(point,
											HMI_HALF_CIRCLE_CTL_CNT,
											dot_line_path_data32,
											HMI_CANE_SOLID_PATH_MAX_LEN,
											ppath_dotted_type->path_solid_len,
											ppath_dotted_type->path_dotted_len,
										center_length);
				ppoint	= dot_line_path_data32;
				bdot_line	= TRUE;
						
				for(index = 0;index < point_len;index += HMI_CUBLIC_BEZIER_CTL_CNT)
				{					
					one_bezier_data32_len		= hmi_line_to_zone_float(&ppoint[index], 
																	HMI_CUBLIC_BEZIER_CTL_CNT, 
																	lane_expand_path_data32, 
																	8/*2 cublic bezier ctrl cnt*/,
																	w);
											
					path_data32_len	+= one_bezier_data32_len;
					expand_bezier_index++;
					for(i = 0;i < 8;i++)
					{
						if(path_data_f_index  < path_data_f_array_len)
						{
							path_data_f[path_data_f_index].x	= (float_32)(lane_expand_path_data32[i].x);
							path_data_f[path_data_f_index].y	= (float_32)(lane_expand_path_data32[i].y);
							path_data_f_index++;
						}
					}							
			
				}
			}
			else	/*2023 07 14 lq*/
			{
				new_beg_ctl.x 	= point[1].x;
				new_beg_ctl.y 	= point[1].y;
				new_end_ctl.x 	= point[2].x;
				new_end_ctl.y 	= point[2].y;
				hmi_get_midpoint_ctrl_f(line_len/* 1/4 circle*/,
											&point[0],
											&new_beg_ctl,
											&new_end_ctl,
											&point[3],
											&mid,
											&mid_ctrl0,
											&mid_ctrl1);
				if(HMI_CANE_SOLID_PATH_MAX_LEN >= 7)
				{
					dot_line_path_data32[0].x	= point[0].x;
					dot_line_path_data32[0].y	= point[0].y;
					dot_line_path_data32[1].x	= new_beg_ctl.x;
					dot_line_path_data32[1].y	= new_beg_ctl.y;

					dot_line_path_data32[2].x	= mid_ctrl0.x;
					dot_line_path_data32[2].y	= mid_ctrl0.y;

					dot_line_path_data32[3].x	= mid.x;
					dot_line_path_data32[3].y	= mid.y; 

					dot_line_path_data32[4].x	= mid_ctrl1.x;
					dot_line_path_data32[4].y	= mid_ctrl1.y;

					dot_line_path_data32[5].x	= new_end_ctl.x;
					dot_line_path_data32[5].y	= new_end_ctl.y;

					dot_line_path_data32[6].x	= point[3].x;
					dot_line_path_data32[6].y	= point[3].y;	

					point_len	= 7;
					ppoint		= dot_line_path_data32;	
					
					{
						hmi_one_bezier_clip_rect_f(ppoint,HMI_HALF_CIRCLE_CTL_CNT,phmi_lane_zone_clip);

						hmi_one_bezier_clip_rect_f(&ppoint[3],HMI_HALF_CIRCLE_CTL_CNT,phmi_lane_zone_clip);
					
					}
				}
				ppoint		= dot_line_path_data32;	
				one_bezier_data32_len		= hmi_line_to_zone_float(&dot_line_path_data32[index], 
																	point_len, 
																	lane_expand_path_data32, 
																	HMI_LANE_LINE_POINT_NB/*2 cublic bezier ctrl cnt*/,
																	w);
											
				path_data32_len	+= one_bezier_data32_len;
				
				for(i = 0;i < one_bezier_data32_len;i++)
				{
					if(path_data_f_index  < path_data_f_array_len)
					{
						path_data_f[path_data_f_index].x	= (float_32)(lane_expand_path_data32[i].x);
						path_data_f[path_data_f_index].y	= (float_32)(lane_expand_path_data32[i].y);
						path_data_f_index++;
					}
				}			
			}
			
				
		}
		if(path_data32_len > path_data_f_array_len)
		{
			path_data32_len	= path_data_f_array_len;
		}													
	}

	return	path_data32_len;
}



U08	hmi_parabola_to_squre_bezier(POINT_FLOAT_3D_TP *pabc,float_32 radius,
									POINT_FLOAT_TP	path_data32[],U08	path_data32_array_len)
{
	U08		path_data32_len	= 0u;

	if(pabc != NULL)
	{
		
	}

	return	path_data32_len;
}

/*
fill rect z ,in screen
*/

BOOLEAN	hmi_get_midpoint_ctrl(float_32 e,SPOINT32_TP	*pbegin,
									SPOINT32_TP	*pbeginControl,
									SPOINT32_TP	*pendControl,
									SPOINT32_TP	*pend,
									SPOINT32_TP	*pmid,
									SPOINT32_TP	*pmidControl1,
									SPOINT32_TP	*pmidControl2)
{
	BOOLEAN			success			= FALSE;
	float_32 		e_1				= (1.0f - e);
	float_32		valx			= 0;
	float_32		valy			= 0;

	if(e_1 < 0.0f)
	{
		e_1	= (-e_1);
	}
	if(e_1 < (1.0f + HMI_FLOAT_TOLERANCE))
	{		
		//i
		valx	=(pbegin->x/*A*/*e_1*e_1+
					pbeginControl->x/*B*/*2*e_1*e+pendControl->x/*C*/*e*e);
		valy	= (pbegin->y/*A*/*e_1*e_1+
					pbeginControl->y/*B*/*2*e_1*e+pendControl->y/*C*/*e*e);
		if(valx > 0)
		{
			pmidControl1->x	= (SINT32)(valx + 0.5f);
		}
		else
		{
			pmidControl1->x	= (SINT32)(valx - 0.5f);
		}
		if(valy > 0)
		{
			pmidControl1->y	= (SINT32)(valy + 0.5f);
		}
		else
		{
			pmidControl1->y	= (SINT32)(valy - 0.5f);
		}
		//j
		valx	= (pbeginControl->x/*B*/*e_1*e_1+
					pendControl->x/*C*/*2*e_1*e+pend->x/*D*/*e*e);
		valy	= (pbeginControl->y/*B*/*e_1*e_1+
					pendControl->y/*C*/*2*e_1*e+pend->y/*D*/*e*e);
		if(valx > 0)
		{
			pmidControl2->x	= (SINT32)(valx + 0.5f);
		}
		else
		{
			pmidControl2->x	= (SINT32)(valx - 0.5f);
		}
		if(valy > 0)
		{
			pmidControl2->y	= (SINT32)(valy + 0.5f);
		}
		else
		{
			pmidControl2->y	= (SINT32)(valy - 0.5f);
		}
		//f
		valx	= (pbegin->x/*A*/*e_1+pbeginControl->x/*B*/*e);
		valy	= (pbegin->y/*A*/*e_1+pbeginControl->y/*B*/*e);
		if(valx > 0)
		{
			pbeginControl->x	= (SINT32)(valx + 0.5f);
		}
		else
		{
			pbeginControl->x	= (SINT32)(valx - 0.5f);
		}
		if(valy > 0)
		{
			pbeginControl->y	= (SINT32)(valy + 0.5f);
		}
		else
		{
			pbeginControl->y	= (SINT32)(valy - 0.5f);
		}
		//h
		valx	= (pendControl->x/*C*/*e_1+pend->x/*D*/*e);
		valy	= (pendControl->y/*C*/*e_1+pend->y/*D*/*e);
		if(valx > 0)
		{
			pendControl->x	= (SINT32)(valx + 0.5f);
		}
		else
		{
			pendControl->x	= (SINT32)(valx - 0.5f);
		}
		if(valy > 0)
		{
			pendControl->y	= (SINT32)(valy + 0.5f);
		}
		else
		{
			pendControl->y	= (SINT32)(valy - 0.5f);
		}
		//e
		valx	= (pmidControl1->x/*i*/*e_1+pmidControl2->x*e/*j*/);
		valy	= (pmidControl1->y/*i*/*e_1+pmidControl2->y*e/*j*/);
		if(valx > 0)
		{
			pmid->x	= (SINT32)(valx + 0.5f);
		}
		else
		{
			pmid->x	= (SINT32)(valx - 0.5f);
		}
		if(valy > 0)
		{
			pmid->y	= (SINT32)(valy + 0.5f);
		}
		else
		{
			pmid->y	= (SINT32)(valy - 0.5f);
		}
		
		success	= TRUE;
	}
	
	return success;
}


BOOLEAN	hmi_get_midpoint_ctrl_f(float_32 e,POINT_FLOAT_TP	*pbegin,
									POINT_FLOAT_TP	*pbeginControl,
									POINT_FLOAT_TP	*pendControl,
									POINT_FLOAT_TP	*pend,
									POINT_FLOAT_TP	*pmid,
									POINT_FLOAT_TP	*pmidControl1,
									POINT_FLOAT_TP	*pmidControl2)
{
	BOOLEAN			success			= FALSE;
	float_32 		e_1				= (1.0f - e);
	float_32		valx			= 0;
	float_32		valy			= 0;

	if(e_1 < 0.0f)
	{
		e_1	= (-e_1);
	}
	if(e_1 < (1.0f + HMI_FLOAT_TOLERANCE))
	{		
		//i
		valx	=(pbegin->x/*A*/*e_1*e_1+
					pbeginControl->x/*B*/*2*e_1*e+pendControl->x/*C*/*e*e);
		valy	= (pbegin->y/*A*/*e_1*e_1+
					pbeginControl->y/*B*/*2*e_1*e+pendControl->y/*C*/*e*e);
		
		pmidControl1->x	= valx ;
		pmidControl1->y	= valy;
				
		//j
		valx	= (pbeginControl->x/*B*/*e_1*e_1+
					pendControl->x/*C*/*2*e_1*e+pend->x/*D*/*e*e);
		valy	= (pbeginControl->y/*B*/*e_1*e_1+
					pendControl->y/*C*/*2*e_1*e+pend->y/*D*/*e*e);

		pmidControl2->x	= valx;
		pmidControl2->y	= valy;
				
		//f
		valx	= (pbegin->x/*A*/*e_1+pbeginControl->x/*B*/*e);
		valy	= (pbegin->y/*A*/*e_1+pbeginControl->y/*B*/*e);
		pbeginControl->x	= valx ;
		pbeginControl->y	= valy ;
				
		//h
		valx	= (pendControl->x/*C*/*e_1+pend->x/*D*/*e);
		valy	= (pendControl->y/*C*/*e_1+pend->y/*D*/*e);
		pendControl->x	= valx;
		pendControl->y	= valy;
				
		//e
		valx	= (pmidControl1->x/*i*/*e_1+pmidControl2->x*e/*j*/);
		valy	= (pmidControl1->y/*i*/*e_1+pmidControl2->y*e/*j*/);
		pmid->x	= valx ;
		pmid->y	= valy ;
				
		success	= TRUE;
	}
	
	return success;
}



typedef enum
{
	HMI_BEZIER_DOT_ENUM,	
	HMI_BEZIER_SOLD_ENUM,/*Cube container*/
	/*Count*/
	HMI_BEZIER_DOT_SOLD_CNT
}HMI_BEZIER_DOT_SOLD_ENUM;

typedef struct
{	
	float_32			e;
	float_32			length;
}HMI_DOT_BEZIER_E_STR;

#define HMI_DOT_E_MAX_CNT		60//30//10
#define	HMI_LANE_DOT_LINE_UNIT	(1)


SINT32 hmi_cubic_bezier_to_mul_bezier(POINT_FLOAT_TP		    		one_bezier_vertex[], 
					                            UINT32        		one_bezier_len,
					                            POINT_FLOAT_TP		mul_bezier_vertex[],
					                            UINT32        		mul_bezier_vertex_len, 
					                            SINT32				sold_line_len,
					                            SINT32				dot_line_len,
					                            SINT32				offset)

{
	SINT32						index			= 0;
	SINT32						index_e			= 0;
	INT32						i				= 0;	
	float_32					tt				= 0.0f;
	float_32					step			= 0.0f;
	float_32					e				= 0.0f;
	float_32					e_last			= 0.0f;
	POINT_FLOAT_TP				p0				= {0};
	POINT_FLOAT_TP				pi				= {0};
	float_32					dx				= 0.0f;
	float_32					dy				= 0.0f;	
	float_32					sum				= 0.0f;
	float_32					bezier_sum		= 0.0f;
	float_32					cur_len			= 0.0f;	
	HMI_BEZIER_DOT_SOLD_ENUM	line_type		= HMI_BEZIER_DOT_SOLD_CNT;
	POINT_FLOAT_TP				mid_ctrl0		= {0.0f,0.0f};
	POINT_FLOAT_TP				mid_ctrl1		= {0.0f,0.0f};
	POINT_FLOAT_TP				mid				= {0.0f,0.0f};
	POINT_FLOAT_TP				new_beg			= {0.0f,0.0f};
	POINT_FLOAT_TP				new_beg_ctl		= {0.0f,0.0f};
	POINT_FLOAT_TP				new_end_ctl		= {0.0f,0.0f};
	POINT_FLOAT_TP				new_mid_ctrl0	= {0.0f,0.0f};
	POINT_FLOAT_TP				new_mid_ctrl1	= {0.0f,0.0f};
	POINT_FLOAT_TP				new_mid			= {0.0f,0.0f};
	HMI_DOT_BEZIER_E_STR		dot_bezier[HMI_DOT_E_MAX_CNT]= {0};
	float_32					fdelta			= 0.0f;/*lq*/
			
	step	= (float_32)(1.0f / HMI_CANE_BEZIER_STEP);	
	if((one_bezier_len >= HMI_ONE_BEZIER_POINT_CNT)&&
		(dot_line_len > 0)&&(sold_line_len > 0))
	{		
		offset		= offset % (dot_line_len + sold_line_len);
		if(offset == 0)
		{									
			line_type	= HMI_BEZIER_SOLD_ENUM;
			sum			= 0;
			cur_len		= (float_32)sold_line_len;		
		}	
		else
		{
			if(offset > 0)
			{	
				if(offset >= dot_line_len)
				{
					sum			= (float_32)(dot_line_len + sold_line_len - offset);
					line_type	= HMI_BEZIER_SOLD_ENUM;
					cur_len		= (float_32)sold_line_len;				
				}
				else
				{
					sum			= (float_32)(dot_line_len - offset);
					line_type	= HMI_BEZIER_DOT_ENUM;
					cur_len		= (float_32)dot_line_len;								
				}	
			}
			else
			{
				offset	= -offset;
				sum		= (float_32)offset;
				if(offset >= sold_line_len)
				{
					sum			= sum - sold_line_len;
					line_type	= HMI_BEZIER_DOT_ENUM;
					cur_len		= (float_32)dot_line_len;				
				}
				else
				{					
					line_type	= HMI_BEZIER_SOLD_ENUM;
					cur_len		= (float_32)sold_line_len;								
				}	
			}									
		}
						
		p0	= hor_f_point_f(3,one_bezier_vertex,tt);
		for(tt = step;tt < 1.0f + HMI_FLOAT_TOLERANCE;tt = tt + step)
		{
			pi	=	hor_f_point_f(3,one_bezier_vertex,tt);
			if(p0.x > pi.x)
			{
				dx	= (p0.x - pi.x);
			}
			else
			{
				dx	= (pi.x - p0.x);
			}
			if(p0.y > pi.y)
			{
				dy	= (p0.y - pi.y);
			}
			else
			{
				dy	= (pi.y - p0.y);
			}
			dx	+= dy;
			bezier_sum	+= dx;
			sum	+= dx;
			if((sum >= cur_len))
			{
				sum	= 0.0f;
				e	= i * step;
				if(e > 1.0f)
				{
					e	= 1.0f;
				}

				if(line_type	== HMI_BEZIER_SOLD_ENUM)
				{
					if(index_e < HMI_DOT_E_MAX_CNT)
					{
						dot_bezier[index_e].e		= e_last;
						dot_bezier[index_e].length	= bezier_sum;
						index_e++;
					}
					
				}

				if(line_type	== HMI_BEZIER_SOLD_ENUM)
				{
					line_type	= HMI_BEZIER_DOT_ENUM;
					cur_len		= (float_32)dot_line_len;
				}
				else
				{
					line_type	= HMI_BEZIER_SOLD_ENUM;
					cur_len		= (float_32)sold_line_len;					
				}
				e_last		= e;
			}			
			p0	= pi;
			i++;
		}	

		if(sum > 0)
		{			
			if(line_type	== HMI_BEZIER_SOLD_ENUM)
			{
				if(index_e < HMI_DOT_E_MAX_CNT)
				{
					dot_bezier[index_e].e		= e_last;
					dot_bezier[index_e].length	= bezier_sum ;	
					index_e++;
				}
				
			}

			if(line_type	== HMI_BEZIER_SOLD_ENUM)
			{
				line_type	= HMI_BEZIER_DOT_ENUM;
				cur_len		= (float_32)dot_line_len;
			}
			else
			{
				line_type	= HMI_BEZIER_SOLD_ENUM;
				cur_len		= (float_32)sold_line_len;					
			}
		}
		
		for(i = 0;i < index_e;i++)
		{		
			new_beg_ctl.x	= (float_32)(one_bezier_vertex[1/*begin ctrl*/].x);
			new_beg_ctl.y	= (float_32)(one_bezier_vertex[1/*begin ctrl*/].y);
			new_end_ctl.x	= (float_32)(one_bezier_vertex[2/*end ctrl*/].x);
			new_end_ctl.y	= (float_32)(one_bezier_vertex[2/*end ctrl*/].y);
			
			if(dot_bezier[i].e != 0.0f)
			{						
				hmi_get_midpoint_ctrl_f(dot_bezier[i].e,
									&one_bezier_vertex[0]/*begin*/,
									&new_beg_ctl/*begin ctrl*/,
									&new_end_ctl/*end ctrl*/,
									&one_bezier_vertex[3]/*end*/,
									&mid/*mid*/,
									&mid_ctrl0/*mid ctrl 0*/,
									&mid_ctrl1/*mid ctrl 1*/);
			}
			else
			{
				new_beg_ctl.x	= one_bezier_vertex[1/*begin ctrl*/].x;
				new_beg_ctl.y	= one_bezier_vertex[1/*begin ctrl*/].y;
				new_end_ctl.x	= one_bezier_vertex[2/*end ctrl*/].x;
				new_end_ctl.y	= one_bezier_vertex[2/*end ctrl*/].y;				
				mid.x			= one_bezier_vertex[0/*begin*/].x;
				mid.y			= one_bezier_vertex[0/*begin*/].y;
				mid_ctrl0.x		= one_bezier_vertex[0/*begin*/].x;
				mid_ctrl0.y		= one_bezier_vertex[0/*begin*/].y;												
				mid_ctrl1.x		= one_bezier_vertex[1/*begin ctrl*/].x;
				mid_ctrl1.y		= one_bezier_vertex[1/*begin ctrl*/].y;
			}

			if(dot_bezier[i].length > 0.0f)
			{
				if(bezier_sum > dot_bezier[i].length)
				{
					fdelta	= (float_32)(bezier_sum - dot_bezier[i].length + sold_line_len);/*lq*/
					if((fdelta > HMI_FLOAT_TOLERANCE) ||
						(fdelta < -HMI_FLOAT_TOLERANCE))
					{
						e	= (float_32)(((float_32)sold_line_len) / fdelta); 
					}
					else
					{
						e	= 1.0f;
					}
				}
				else
				{
					e	= 1.0f;
				}
				if(e > 1.0f)
				{
					e	= 1.0f;
				}
				else if(e < 0)
				{
					e	= 0.0f;
				}
				else{}
				
				
				hmi_get_midpoint_ctrl_f(e,
									&mid/*begin*/,
									&mid_ctrl1/*begin ctrl*/,
									&new_end_ctl/*end ctrl*/,
									&one_bezier_vertex[3]/*end*/,
									&new_mid/*mid*/,
									&new_mid_ctrl0/*mid ctrl 0*/,
									&new_mid_ctrl1/*mid ctrl 1*/);
				if(index + 4/*cublic bezier cnt*/ < (INT32)mul_bezier_vertex_len)
				{
					mul_bezier_vertex[index].x	= mid.x;/*begin*/
					mul_bezier_vertex[index].y	= mid.y;/*begin*/
					index++;

					mul_bezier_vertex[index].x	= mid_ctrl1.x;/*begin ctrl*/
					mul_bezier_vertex[index].y	= mid_ctrl1.y;/*begin ctrl*/
					index++;

					mul_bezier_vertex[index].x	= new_mid_ctrl0.x;/*end ctrl*/
					mul_bezier_vertex[index].y	= new_mid_ctrl0.y;/*end ctrl*/
					index++;
					
					mul_bezier_vertex[index].x	= new_mid.x;/*end*/
					mul_bezier_vertex[index].y	= new_mid.y;/*end*/
					index++;
				}
			}									
		}		
	}
	
	return index;
}


#endif
/************Custom lib********/

#include "hmi_custom_func.rom"

void hmi_normalise_2d(POINT_FLOAT_TP *pa)
{	
	float_32		len 	= 0U;

	if(pa != NULL)
	{
		len		= (float_32)((*pa).x * (*pa).x + (*pa).y * (*pa).y);
		len 	= (float_32)sqrt(len);
		if(len > HMI_FLOAT_TOLERANCE)
		{
			(*pa).x = (float_32)((*pa).x / len);
			(*pa).y = (float_32)((*pa).y / len);
		}
		else
		{
			(*pa).x = 0.0f;
			(*pa).y = 1.0f; 		
		}
	}
}


static HMI_QD_PUBLIC_BUFFER_STR			hmi_qd_public_buffer_list[HMI_QD_PUBLIC_BUFFER_CNT];
static UINT32	hmi_qd_public_buffer_size;					
void	hmi_init_qd_buffer_list(void)
{
	memset(hmi_qd_public_buffer_list,0,
		sizeof(HMI_QD_PUBLIC_BUFFER_STR) * HMI_QD_PUBLIC_BUFFER_CNT);
	hmi_qd_public_buffer_size	= 0u;
	
}

void	hmi_destory_qd_buffer_list(void)
{
	INT32		i					= 0;
	
	for(i = 0;i < HMI_QD_PUBLIC_BUFFER_CNT;i++)
	{
		if(hmi_qd_public_buffer_list[i].ppublic_buffer != NULL)
		{
			HMI_FREE(hmi_qd_public_buffer_list[i].ppublic_buffer);
		}					
	}

	memset(hmi_qd_public_buffer_list,0,sizeof(HMI_QD_PUBLIC_BUFFER_STR) * HMI_QD_PUBLIC_BUFFER_CNT);
	hmi_qd_public_buffer_size	= 0u;
}


void	hmi_release_qd_buf(void	*pbuf)
{
	INT32		i					= 0;
	BOOLEAN		finished			= FALSE;

	if(pbuf != NULL)
	{
		for(i = 0;(i < HMI_QD_PUBLIC_BUFFER_CNT)&&(finished == FALSE);i++)
		{
			if(hmi_qd_public_buffer_list[i].ppublic_buffer == pbuf)
			{
				hmi_qd_public_buffer_list[i].attribute	= hmi_qd_public_buffer_list[i].attribute & 0xfffffffe;
				finished								= TRUE;
			}					
		}
	}
}

void	*hmi_get_qd_buffer(U32	size)
{
	void	*pbuf					= NULL;
	INT32	i						= 0;
	U32		free_size				= 0;
	U32		min_free_size			= HMI_MAX_RGL_32BIT;
	INT32	min_index				= -1;
	BOOLEAN	finished				= FALSE;
	INT32	free_index_nobuf		= -1;
	INT32	free_index_existbuf		= -1;
	BOOLEAN	bmalloc					= FALSE;
	static INT32 max_free_cnt	= HMI_QD_PUBLIC_BUFFER_CNT;
	INT32	free_cnt	 = 0;
	/*HMI_QD_PUBLIC_MIN_BLOCK_SIZE size align size */	
	size	= (U32)(((size + HMI_QD_PUBLIC_MIN_BLOCK_SIZE - 1) / HMI_QD_PUBLIC_MIN_BLOCK_SIZE) * HMI_QD_PUBLIC_MIN_BLOCK_SIZE);

	for(i = 0;(i < HMI_QD_PUBLIC_BUFFER_CNT)&&(finished == FALSE);i++)
	{
		if((hmi_qd_public_buffer_list[i].attribute & 0x1) == 0) /*free*/ 
		{			
			if(hmi_qd_public_buffer_list[i].buffer_len >= size)
			{
				free_size	= hmi_qd_public_buffer_list[i].buffer_len - size;
				if(free_size < min_free_size)
				{
					min_index		= i;
					min_free_size	= free_size;
				}
				if(free_size == 0)
				{

					min_index		= i;
					min_free_size	= free_size;
					finished		= TRUE;
				}
			}
			if(free_index_nobuf < 0)
			{
				if(hmi_qd_public_buffer_list[i].ppublic_buffer == NULL)
				{
					free_index_nobuf	= i;
				}
			}
			if(free_index_existbuf < 0)
			{
				if(hmi_qd_public_buffer_list[i].ppublic_buffer != NULL)
				{
					free_index_existbuf	= i;
				}
			}
		}		
	}
	if((min_index >= 0) && (min_index < HMI_QD_PUBLIC_BUFFER_CNT))
	{
		pbuf											= (void *)(hmi_qd_public_buffer_list[min_index].ppublic_buffer);
		hmi_qd_public_buffer_list[min_index].attribute	= (hmi_qd_public_buffer_list[min_index].attribute | 0x01);
	}
	else
	{		
		if((free_index_nobuf >= 0)&&(free_index_nobuf < HMI_QD_PUBLIC_BUFFER_CNT))
		{
			if(size < HMI_QD_PUBLIC_MIN_BLOCK_SIZE)
			{
				size = HMI_QD_PUBLIC_MIN_BLOCK_SIZE;
			}
			
			hmi_qd_public_buffer_list[free_index_nobuf].ppublic_buffer	= (BYTE	*)HMI_MALLOC(size);
			
			//printf("hmi_get_qd_buffer malloc address %p size =%ld\n", 
			//				hmi_qd_public_buffer_list[free_index_nobuf].ppublic_buffer,size);
			bmalloc			= TRUE;
			if(hmi_qd_public_buffer_list[free_index_nobuf].ppublic_buffer != NULL)
			{
				hmi_qd_public_buffer_list[free_index_nobuf].buffer_len	= size;
				hmi_qd_public_buffer_list[free_index_nobuf].attribute		= hmi_qd_public_buffer_list[free_index_nobuf].attribute | 0x01;
				hmi_qd_public_buffer_size	+= size;
				//hmi_printf_time("hmi get qd buffer malloc buffer,cur buffer size ",hmi_qd_public_buffer_size);
			}	
			else
			{
				hmi_printf_time("hmi get qd buffer malloc buffer2 failed ",0);	// lq
			}
			pbuf	= (void *)hmi_qd_public_buffer_list[free_index_nobuf].ppublic_buffer;
			
		}
		else if((free_index_existbuf >= 0)&&(free_index_existbuf < HMI_QD_PUBLIC_BUFFER_CNT))
		{
			if(hmi_qd_public_buffer_list[free_index_existbuf].ppublic_buffer != NULL)
			{
				hmi_printf_time("hmi get qd buffer free address ",(SINT32)(hmi_qd_public_buffer_list[free_index_existbuf].ppublic_buffer));
				HMI_FREE(hmi_qd_public_buffer_list[free_index_existbuf].ppublic_buffer);
				//printf("hmi_get_qd_buffer free %d\n",hmi_qd_public_buffer_list[free_index_existbuf].buffer_len);
				hmi_qd_public_buffer_list[free_index_existbuf].ppublic_buffer	= NULL;
				//hmi_qd_public_buffer_list[free_index_existbuf].buffer_len		= 0;lq
				hmi_qd_public_buffer_list[free_index_existbuf].attribute		= hmi_qd_public_buffer_list[free_index_existbuf].attribute & 0xfffffffe;
				if(hmi_qd_public_buffer_size >= hmi_qd_public_buffer_list[free_index_existbuf].buffer_len)
				{
					hmi_qd_public_buffer_size	-= hmi_qd_public_buffer_list[free_index_existbuf].buffer_len;
				}
				else
				{
					hmi_qd_public_buffer_size	= 0u;
				}
				hmi_qd_public_buffer_list[free_index_existbuf].buffer_len		= 0; /*lq*/
			}
		#if 0
			if(size < HMI_QD_PUBLIC_MIN_BLOCK_SIZE)
			{
				size = HMI_QD_PUBLIC_MIN_BLOCK_SIZE;
			}
		#endif
			
			hmi_qd_public_buffer_list[free_index_existbuf].ppublic_buffer	= (BYTE *)HMI_MALLOC(size);
			//printf("hmi_get_qd_buffer malloc %d\n",size);
			bmalloc			= TRUE;
			if(hmi_qd_public_buffer_list[free_index_existbuf].ppublic_buffer != NULL)
			{
				hmi_qd_public_buffer_list[free_index_existbuf].buffer_len	= size;
				hmi_qd_public_buffer_list[free_index_existbuf].attribute	= hmi_qd_public_buffer_list[free_index_existbuf].attribute | 0x01;
				hmi_qd_public_buffer_size	+= hmi_qd_public_buffer_list[free_index_existbuf].buffer_len;
				hmi_printf_time("hmi get qd buffer malloc buffer2,cur buffer size ",hmi_qd_public_buffer_size);
			}
			else
			{
				hmi_printf_time("hmi get qd buffer malloc buffer2 failed ",0);			
			}
			
			pbuf	= (void *)hmi_qd_public_buffer_list[free_index_existbuf].ppublic_buffer;
		}
		else
		{
			hmi_printf_time("hmi get qd buffer free buffer failed. bufferlist full.",0);// lq
		}
	}

	if( (bmalloc == TRUE) && (hmi_qd_public_buffer_size > HMI_QD_MAX_HEAP_SIZE))
	{
		hmi_printf_time("hmi_qd_public_buffer_size over size,release buffer",0);
		finished	= FALSE;
		for(i = 0;(i < HMI_QD_PUBLIC_BUFFER_CNT)&&(finished == FALSE);i++)
		{
			if((hmi_qd_public_buffer_list[i].attribute & 0x1) == 0u) /*free*/ 
			{			
				if(hmi_qd_public_buffer_list[i].buffer_len > 0u)
				{					
					if(hmi_qd_public_buffer_list[i].ppublic_buffer != NULL)
					{
						hmi_printf_time("hmi_get_qd_buffer free ",(SINT32)(hmi_qd_public_buffer_list[i].ppublic_buffer));
						HMI_FREE(hmi_qd_public_buffer_list[i].ppublic_buffer);
						
						//printf("hmi_get_qd_buffer free %d\n",
						//	hmi_qd_public_buffer_list[i].buffer_len);
						
						hmi_qd_public_buffer_list[i].ppublic_buffer	= NULL;
						hmi_qd_public_buffer_list[i].buffer_len		= 0u;
						hmi_qd_public_buffer_list[i].attribute		= hmi_qd_public_buffer_list[i].attribute & 0xfffffffe;
						
						if(hmi_qd_public_buffer_size >= hmi_qd_public_buffer_list[i].buffer_len)
						{
							hmi_qd_public_buffer_size	-= hmi_qd_public_buffer_list[i].buffer_len;
						}
						else
						{
							hmi_qd_public_buffer_size	= 0u;
						}
						if(hmi_qd_public_buffer_size <= HMI_QD_MAX_HEAP_SIZE)
						{
							finished	= TRUE;
						}
					}
				}				
			}		
		}
		hmi_printf_time("hmi get qd buffer free buffer,cur buffer size ",hmi_qd_public_buffer_size);
	}

	for(i = 0;(i < HMI_QD_PUBLIC_BUFFER_CNT);i++)
	{
		if((hmi_qd_public_buffer_list[i].attribute & 0x1) == 0) /*free*/ 
		{
			free_cnt++;	
		}
	}

	if(free_cnt < max_free_cnt)
	{
		hmi_printf_time("hmi get qd buffer free cnt nb ",free_cnt);
		max_free_cnt =free_cnt;
	}
	return	pbuf;
}





