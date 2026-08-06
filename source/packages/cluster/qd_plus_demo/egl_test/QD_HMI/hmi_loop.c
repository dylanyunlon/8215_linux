/*****************************************************************************

File Name        :  hmi_loop.c
Organization    :  Zhuli Electronics Co.Ltd in Shanghai (www.shzldz.com)
******************************************************************************/
#if 0
void hmi_Loop(void);
#endif
#include "hmi_all_struct_include.h"

/*#define MS_PER_SEC  1000*/
void add_tick_res_manager(void);
typedef enum
{
	HMI_CUR_DIRTY,
	HMI_BCK_DIRTY,
	HMI_2BUFFER_DIRTY,
	/***COUNT**/
	HMI_BUFFER_DIRTY_CNT
}HMI_BUFFER_DIRTY_STR;

#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))
static	HMI_RECT_STR dirty_zone[HMI_ALL_LAYERS_NUMBER][HMI_BUFFER_DIRTY_CNT][HMI_ALL_DIRTY_FIFO_LEN]={{HMI_INVALID_COOR,HMI_INVALID_COOR,0,0}};
#endif
#if defined(S6J3200_GRAPHIC)
#ifdef HMI_ONE_BUFFER_DISP_PART
static	HMI_RECT_STR dirty_part_zone[HMI_ALL_DIRTY_FIFO_LEN]={{HMI_INVALID_COOR,HMI_INVALID_COOR,0,0}};
#endif
#endif

#if 0  //he 2021-03-03
void hmi_swap_dirty_buffer(HMI_RECT_STR *buffer1 ,HMI_RECT_STR *buffer2)
{
	HMI_RECT_STR *	swap_buffer		= NULL;
	
	swap_buffer		= buffer1;
	buffer1			= buffer2;
	buffer2			= swap_buffer;
}  
#endif

void hmi_gfx_update(HMI_TIME dt)
{
  //printf("hmi_gfx_update ss\r\n");
#ifdef HMI_TOUCH_PANEL
	TOUCH_BUTTON_STR hmi_cur_touch[HMI_SOFTKEY_LEN]={
		{HMI_NO_TOUCH_KEY,FALSE,HMI_KEY_CNT_OSD,{0,0},{0,0},FALSE,HMI_ROTATION_INVALIDE_ANGLE/*angle*/,-1.0f/*scale*/,{0,0},{0,0}},
		{HMI_NO_TOUCH_KEY,FALSE,HMI_KEY_CNT_OSD,{0,0},{0,0},FALSE,HMI_ROTATION_INVALIDE_ANGLE/*angle*/,-1.0f/*scale*/,{0,0},{0,0}},
	};	
#endif



#if defined(HMI_GRAPHIC_VGLITE	)||defined(HMI_GRAPHIC_OPENVG)
	add_tick_res_manager();
#elif defined( HMI_GRAPHIC_OPENGLES )
	add_tick_res_manager();//removed by pxguo 160827
#endif

	/*dt=dt/MS_PER_SEC;*/
#ifdef HMI_SOFT_TIMER
	hmi_tick_manager(dt);
#endif

#ifdef HMI_TOUCH_PANEL		
	hmi_touch_panel(dt,hmi_cur_touch);	
#endif

#ifdef HMI_8_DIRECT
	/*hmi_8direct(dt);*/
#endif

#ifdef HMI_MEMORY_MANAGER 
	/*continue to load resource*/
#endif

#ifdef HMI_TOUCH_PANEL
	#ifndef HMI_GRAPHIC_ST7513
	hmi_user_process(dt,hmi_cur_touch);
	#else
	hmi_user_process(dt,NULL);
	#endif
#else
	hmi_user_process(dt,NULL);
#endif

#if HMI_ALL_EVENT_NUMBER + HMI_ACTION_GROUP_NUMBER > 0 /*lq*/
	hmi_action_manager(dt);
#endif
  //printf("hmi_gfx_update ee\r\n");
}

#if (defined(HMI_GRAPHIC_OPENGLES)||defined(S6J3200_GRAPHIC)||defined(HMI_GRAPHIC_VGLITE)||defined(HMI_GRAPHIC_OPENVG)||defined(HMI_GRAPHIC_RGL))

void hmi_clear_all_dirty_zone( HMI_BUFFER_DIRTY_STR index_buffer)
{
	UINT8			i	= 0;
	UINT8			hmi_screen_id	= 0U;
	
	hmi_screen_id =hmi_driver_get_render_screen();
	for(i=0;i < HMI_LAYER_MAX_CNT;i++)
	{
		dirty_zone[hmi_screen_id][index_buffer][i].x	= HMI_INVALID_COOR;
		dirty_zone[hmi_screen_id][index_buffer][i].y	= HMI_INVALID_COOR;
		dirty_zone[hmi_screen_id][index_buffer][i].w	= 0;
		dirty_zone[hmi_screen_id][index_buffer][i].h	= 0;
		dirty_zone[hmi_screen_id][HMI_2BUFFER_DIRTY][i].x	= HMI_INVALID_COOR;
		dirty_zone[hmi_screen_id][HMI_2BUFFER_DIRTY][i].y	= HMI_INVALID_COOR;
		dirty_zone[hmi_screen_id][HMI_2BUFFER_DIRTY][i].w	= 0;
		dirty_zone[hmi_screen_id][HMI_2BUFFER_DIRTY][i].h	= 0;
	}
	for(i = HMI_LAYER_MAX_CNT;i < HMI_ALL_DIRTY_FIFO_LEN;i += HMI_DIRTY_FIFO_LEN)
	{
		dirty_zone[hmi_screen_id][index_buffer][i].x	= HMI_INVALID_COOR;
		dirty_zone[hmi_screen_id][index_buffer][i].y	= HMI_INVALID_COOR;
		dirty_zone[hmi_screen_id][index_buffer][i].w	= 0;
		dirty_zone[hmi_screen_id][index_buffer][i].h	= 0;
		dirty_zone[hmi_screen_id][HMI_2BUFFER_DIRTY][i].x	= HMI_INVALID_COOR;
		dirty_zone[hmi_screen_id][HMI_2BUFFER_DIRTY][i].y	= HMI_INVALID_COOR;
		dirty_zone[hmi_screen_id][HMI_2BUFFER_DIRTY][i].w	= 0;
		dirty_zone[hmi_screen_id][HMI_2BUFFER_DIRTY][i].h	= 0;
	}
}
#endif

#if	defined(HMI_GRAPHIC_RGL)
void hmi_gfx_render(void)
{
	#if (defined(HMI_GRAPHIC_AGG)||defined(HMI_GRAPHIC_ST)||defined(HMI_GRAPHIC_ST7513))
	HMI_RECT_STR dirty_zone[1]={{HMI_INVALID_COOR,HMI_INVALID_COOR,0,0}};
	#elif defined(HMI_GRAPHIC_RGL)
	UINT8			i			= 0;
	BOOLEAN			dirty		= FALSE;
	//static HMI_BUFFER_DIRTY_STR index_free	= HMI_CUR_DIRTY;
	//static HMI_BUFFER_DIRTY_STR index_bck	= HMI_BCK_DIRTY;
	static HMI_BUFFER_DIRTY_STR index_free[HMI_ALL_LAYERS_NUMBER]	= {HMI_CUR_DIRTY,HMI_CUR_DIRTY};
	static HMI_BUFFER_DIRTY_STR index_bck[HMI_ALL_LAYERS_NUMBER]	= {HMI_BCK_DIRTY,HMI_BCK_DIRTY};
	
	#if(HMI_RENDER_ENABLE_FIFO == HMI_YES)
	U16				load		= 0;
	U16			total_load		= 0;
	HwUpdateState_t	cmd_type	= HwUpdate_StateMax;
	#endif
	UINT8						hmi_screen_id	= 0U;	
	#endif
	#ifdef HMI_GRAPHIC_RGL
	#ifdef HMI_R_ASYNCHRONOUS_HW_UPDATE
	if(HMI_RENDERING())
	{
		
		#if(HMI_RENDER_ENABLE_FIFO == HMI_YES)
		if(hmi_is_empty_pop_up_draw_fifo())/* empty*/
		{
		#endif
#endif
#endif
		if((HMI_GFX_GET_STATUS(HMI_SEND_EVENT))&&
			(hmi_driver_woking_status_flag == HMI_FRAMEBUFFER_FREE))
		{				
			
		#if defined(HMI_GRAPHIC_AGG)
		#if HMI_PAGES_NUMBER>0				
			hmi_engine_get_multi_dirty_zone_page(dirty_zone);
			if((dirty_zone[0].w > 0)&&(dirty_zone[0].h > 0))
			{			
				hmi_engine_draw_page(dirty_zone);			
			}
		#endif
		
		#elif defined( HMI_GRAPHIC_RGL ) 
		#if HMI_PAGES_NUMBER>0	
		
			hmi_screen_id =hmi_driver_get_render_screen();
			hmi_clear_all_dirty_zone(index_free[hmi_screen_id]);		
			hmi_engine_get_multi_dirty_zone_page(dirty_zone[hmi_screen_id][index_free[hmi_screen_id]]);
			for(i=0;i < HMI_LAYER_MAX_CNT;i++)
			{
			#if(HMI_BUFFER_NUMBER == HMI_TWO_BUFFER)
				if((dirty_zone[hmi_screen_id][index_free[hmi_screen_id]][i].w > 0)&&(dirty_zone[hmi_screen_id][index_free[hmi_screen_id]][i].h > 0))
				{
					dirty			= TRUE;
					hmi_add_2buffer_dirty_zone(dirty_zone[hmi_screen_id][index_free[hmi_screen_id]],dirty_zone[hmi_screen_id][index_bck[hmi_screen_id]],
										dirty_zone[hmi_screen_id][HMI_2BUFFER_DIRTY],i);
				}
				else
				{
					hmi_copy_last_dirty_zone(dirty_zone[hmi_screen_id][index_bck[hmi_screen_id]],dirty_zone[hmi_screen_id][index_free[hmi_screen_id]],i);
				}
			#else
				if((dirty_zone[hmi_screen_id][index_free[hmi_screen_id]][i].w > 0)&&(dirty_zone[hmi_screen_id][index_free[hmi_screen_id]][i].h > 0))
				{
					dirty			= TRUE;
					hmi_copy_last_dirty_zone(dirty_zone[hmi_screen_id][index_free[hmi_screen_id]],dirty_zone[hmi_screen_id][HMI_2BUFFER_DIRTY],i);
				}
			#endif
			}
			if(dirty == TRUE)
			{
				hmi_engine_create_rgl_window( );
				/* get new free buffer for every dirty layer  and clear dirty layer first*/
				hmi_driver_set_render_buffer(dirty_zone[hmi_screen_id][HMI_2BUFFER_DIRTY]);
				hmi_clear_dirty_layer(dirty_zone[hmi_screen_id][HMI_2BUFFER_DIRTY]);
				/*draw at new free buffer*/
				hmi_engine_draw_page(dirty_zone[hmi_screen_id][HMI_2BUFFER_DIRTY]);
			#if(HMI_BUFFER_NUMBER==HMI_TWO_BUFFER)
				index_free[hmi_screen_id]	= (HMI_BUFFER_DIRTY_STR)(HMI_BCK_DIRTY - index_free[hmi_screen_id]);
				index_bck[hmi_screen_id]	= (HMI_BUFFER_DIRTY_STR)(HMI_BCK_DIRTY - index_bck[hmi_screen_id]);
			#endif
			}
			#if ((HMI_SCREEN0_ENABLE + HMI_SCREEN1_ENABLE) < HMI_ALL_LAYERS_NUMBER)
			{
				HMI_GFX_CLEAR_STATUS(HMI_SEND_EVENT);
				HMI_GFX_CLEAR_STATUS(HMI_RGL_PROP_EVENT);
			}
			#else
			if(hmi_screen_id == HMI_LAYER_SCREEN1)
			{
				HMI_GFX_CLEAR_STATUS(HMI_SEND_EVENT);
				HMI_GFX_CLEAR_STATUS(HMI_RGL_PROP_EVENT);
			}
			#endif
		#endif
#elif defined( HMI_GRAPHIC_ST7513 ) 
		#if HMI_PAGES_NUMBER>0	
			hmi_engine_get_dirty_zone_page(dirty_zone);
			if((dirty_zone[0].w > 0)&&(dirty_zone[0].h > 0))
			{			
				hmi_engine_draw_page(dirty_zone);			
			}
					
		#endif
#elif defined(HMI_GRAPHIC_TWLIB)
		#if HMI_PAGES_NUMBER>0	
			#if HMI_RENDER_ALL_EXCEPT_BCK==NO
				hmi_not_render_mode();/*find not used win*/
				hmi_clr_used_flag();/*clr used flag*/
				hmi_engine_draw_page();/*find not used win*/
				hmi_clear_no_used_win();/*clear not used win*/
				hmi_clr_refrence_flag();/*clr refrence flag*/
				hmi_render_mode();/*render mode*/
				hmi_engine_draw_page();
				hmi_off_no_used_win(); 
			#else
				hmi_engine_draw_page();
			#endif
		#endif
#elif (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
#if HMI_PAGES_NUMBER>0			
			hmi_clear_page_layer();/*must before hmi_clear_win_used() */
			hmi_clear_win_used();
			hmi_clear_text_index();
			hmi_engine_draw_page();
			HMI_GFX_CLEAR_STATUS(HMI_SEND_EVENT);
#endif	
#endif
			//hmi_driver_woking_status_flag=HMI_FRAMEBUFFER_FREE;/*frame buffer free flag*/
		}
		else /*no flag,example page alpha*/
		{
		#ifdef HMI_GRAPHIC_RGL
			if(HMI_GFX_GET_STATUS(HMI_RGL_PROP_EVENT))
			{
				hmi_engine_draw_page_prop();
				#if ((HMI_SCREEN0_ENABLE + HMI_SCREEN1_ENABLE) < HMI_ALL_LAYERS_NUMBER)
				{
					HMI_GFX_CLEAR_STATUS(HMI_RGL_PROP_EVENT);
				}
				#else
				if(hmi_screen_id == HMI_LAYER_SCREEN1)
				{
					HMI_GFX_CLEAR_STATUS(HMI_RGL_PROP_EVENT);
				}
				#endif
			}
		#endif
		}
		#ifdef HMI_GRAPHIC_RGL
		#ifdef HMI_R_ASYNCHRONOUS_HW_UPDATE
		#if(HMI_RENDER_ENABLE_FIFO == HMI_YES)
		}
		if(HMI_RENDERING())
		{
			while((total_load < HMI_REND_MAX_TIME)&&(HMI_RENDERING()))
			{	
				load		= hmi_pop_up_draw_fifo(&cmd_type);
				total_load	+= load;	
				if((cmd_type==HwUpdate_WaitScanline)&&(!HMI_EXCUTING()))
				{
					hmi_set_sync_status(HwUpdate_WaitScanline);
					total_load=HMI_REND_MAX_TIME;
				}				
			}
			if(HMI_RENDERING())
			{
				hmi_set_sync_status(HwUpdate_WaitScanline);
			}
		}
		#endif
		#endif
		#endif
		
#ifdef HMI_GRAPHIC_RGL
#ifdef HMI_R_ASYNCHRONOUS_HW_UPDATE
	}
	else if(HMI_EXCUTING())
	{ 
		hmi_execute_cmd();
	}
	else
	{
		;
	}
#endif
#endif
}
#elif defined( HMI_GRAPHIC_OPENGLES )
void hmi_gfx_render(void)
{
	UINT8			i				= 0u;
	BOOLEAN			dirty			= FALSE;
	U08				hmi_screen_id	= 0U;
	
	static HMI_BUFFER_DIRTY_STR index_free[HMI_ALL_LAYERS_NUMBER]	= {HMI_CUR_DIRTY,HMI_CUR_DIRTY};
	static HMI_BUFFER_DIRTY_STR index_bck[HMI_ALL_LAYERS_NUMBER]	= {HMI_BCK_DIRTY,HMI_BCK_DIRTY};
	
	if((HMI_GFX_GET_STATUS(HMI_SEND_EVENT))&&
		(hmi_driver_woking_status_flag == HMI_FRAMEBUFFER_FREE))
	{						
		#if HMI_PAGES_NUMBER > 0	
			hmi_screen_id =hmi_driver_get_render_screen();
			hmi_clear_all_dirty_zone(index_free[hmi_screen_id]);		
			hmi_engine_get_multi_dirty_zone_page(dirty_zone[hmi_screen_id][index_free[hmi_screen_id]]);
		for(i=0;i < HMI_LAYER_MAX_CNT;i++)
		{
			#if(HMI_BUFFER_NUMBER == HMI_TWO_BUFFER)
				if((dirty_zone[hmi_screen_id][index_free[hmi_screen_id]][i].w > 0)&&(dirty_zone[hmi_screen_id][index_free[hmi_screen_id]][i].h > 0))
			{
				dirty			= TRUE;
					hmi_add_2buffer_dirty_zone(dirty_zone[hmi_screen_id][index_free[hmi_screen_id]],dirty_zone[hmi_screen_id][index_bck[hmi_screen_id]],
										dirty_zone[hmi_screen_id][HMI_2BUFFER_DIRTY],i);
			}
			else
			{
					hmi_copy_last_dirty_zone(dirty_zone[hmi_screen_id][index_bck[hmi_screen_id]],dirty_zone[hmi_screen_id][index_free[hmi_screen_id]],i);
			}
			#else
				#error HMI_LOOP.C: OpenglES only support 2 buffer.
			#endif
		}
		if(dirty == TRUE)
		{		
			/*clear dirty zone*/
				hmi_clear_dirty_layer(dirty_zone[hmi_screen_id][HMI_2BUFFER_DIRTY],hmi_screen_id);
			/*draw at new free buffer*/
				hmi_engine_draw_page(dirty_zone[hmi_screen_id][HMI_2BUFFER_DIRTY]);
			#if(HMI_BUFFER_NUMBER==HMI_TWO_BUFFER)
				index_free[hmi_screen_id]	= (HMI_BUFFER_DIRTY_STR)(HMI_BCK_DIRTY - index_free[hmi_screen_id]);
				index_bck[hmi_screen_id]	= (HMI_BUFFER_DIRTY_STR)(HMI_BCK_DIRTY - index_bck[hmi_screen_id]);
			#endif
		}
		HMI_GFX_CLEAR_STATUS(HMI_SEND_EVENT);
		HMI_GFX_CLEAR_STATUS(HMI_RGL_PROP_EVENT);
		#endif
	
		//hmi_driver_woking_status_flag=HMI_FRAMEBUFFER_FREE;/*frame buffer free flag*/
	}
	else /*no flag,example page alpha*/
	{
		
	}
}
#elif defined(S6J3200_GRAPHIC)
void hmi_gfx_render(void)
{
	UINT8			i									= 0u;
	UINT8			j									= 0u;
	BOOLEAN			dirty								= FALSE;
	U08				hmi_screen_id						= 0U;
	BOOLEAN			render_finished						= FALSE;
	static HMI_BUFFER_DIRTY_STR index_free[HMI_ALL_LAYERS_NUMBER]	= {HMI_CUR_DIRTY,HMI_CUR_DIRTY};
	static HMI_BUFFER_DIRTY_STR index_bck[HMI_ALL_LAYERS_NUMBER]	= {HMI_BCK_DIRTY,HMI_BCK_DIRTY};
	#ifdef HMI_ONE_BUFFER_DISP_PART
	HMI_RECT_STR	hmi_draw_part_rect					= {0U};
	HMI_RECT_STR	hmi_dirty_rect						= {0U};
	#endif
	UINT8			layer_buf_nb						= 0U;
	
	
	if((HMI_GFX_GET_STATUS(HMI_SEND_EVENT))&&
		(hmi_driver_woking_status_flag == HMI_FRAMEBUFFER_FREE))
	{	
#if HMI_PAGES_NUMBER > 0
		hmi_screen_id = hmi_driver_get_render_screen();
		hmi_clear_all_dirty_zone(index_free[hmi_screen_id]);		
		hmi_engine_get_multi_dirty_zone_page(dirty_zone[hmi_screen_id][index_free[hmi_screen_id]]);
		for(i=0;i < HMI_LAYER_MAX_CNT;i++)
		{
			layer_buf_nb	= hmi_get_layer_buf_nb(i);
			if(layer_buf_nb ==HMI_TWO_BUFFER)
			{
				if((dirty_zone[hmi_screen_id][index_free[hmi_screen_id]][i].w > 0)&&(dirty_zone[hmi_screen_id][index_free[hmi_screen_id]][i].h > 0))
				{
					dirty			= TRUE;
					hmi_add_2buffer_dirty_zone(dirty_zone[hmi_screen_id][index_free[hmi_screen_id]],dirty_zone[hmi_screen_id][index_bck[hmi_screen_id]],
										dirty_zone[hmi_screen_id][HMI_2BUFFER_DIRTY],i);
				}
				else
				{
					hmi_copy_last_dirty_zone(dirty_zone[hmi_screen_id][index_bck[hmi_screen_id]],dirty_zone[hmi_screen_id][index_free[hmi_screen_id]],i);
				}
			}
			else
			{
				if((dirty_zone[hmi_screen_id][index_free[hmi_screen_id]][i].w > 0)&&(dirty_zone[hmi_screen_id][index_free[hmi_screen_id]][i].h > 0))
				{
					dirty			= TRUE;
					hmi_copy_last_dirty_zone(dirty_zone[hmi_screen_id][index_free[hmi_screen_id]],dirty_zone[hmi_screen_id][HMI_2BUFFER_DIRTY],i);
				}
			}
		}
		
		if(dirty == TRUE)
		{
			/*Create window and bind buffer*/
			hmi_engine_create_rgl_window();
			render_finished	 = hmi_allow_render(dirty_zone[hmi_screen_id][HMI_2BUFFER_DIRTY]);
			if(render_finished == TRUE)
			{				
				//hmi_driver_set_render_buffer(dirty_zone[hmi_screen_id][HMI_2BUFFER_DIRTY]);
			/*clear dirty zone*/
			#if (HMI_CLEAR_DIRTY_ENABLE == YES)
				hmi_clear_dirty_layer(dirty_zone[hmi_screen_id][HMI_2BUFFER_DIRTY]);
			#endif
			/* get new free buffer for every dirty layer  and clear dirty layer first*/
				//hmi_driver_set_render_buffer(dirty_zone[hmi_screen_id][HMI_2BUFFER_DIRTY]);
			/*draw at new free buffer*/
				hmi_engine_draw_page(dirty_zone[hmi_screen_id][HMI_2BUFFER_DIRTY]);
			
				index_free[hmi_screen_id]	= (HMI_BUFFER_DIRTY_STR)(HMI_BCK_DIRTY - index_free[hmi_screen_id]);
				index_bck[hmi_screen_id]	= (HMI_BUFFER_DIRTY_STR)(HMI_BCK_DIRTY - index_bck[hmi_screen_id]);
			}
		}
		HMI_GFX_CLEAR_STATUS(HMI_SEND_EVENT);
		HMI_GFX_CLEAR_STATUS(HMI_RGL_PROP_EVENT);
	#endif
	}
	else /*no flag,example page alpha*/
	{
		
	}
}

#elif defined(HMI_GRAPHIC_VGLITE) ||defined(HMI_GRAPHIC_OPENVG)
void hmi_gfx_render(void)
{
	UINT8			i									= 0u;	
	BOOLEAN 		dirty								= FALSE;
	U08 			hmi_screen_id						= 0U;
	BOOLEAN 		render_finished 					= FALSE;	
	UINT8			layer_buf_nb						= 0U;	
	static HMI_BUFFER_DIRTY_STR index_free[HMI_ALL_LAYERS_NUMBER]	= {HMI_CUR_DIRTY,HMI_CUR_DIRTY};
	static HMI_BUFFER_DIRTY_STR index_bck[HMI_ALL_LAYERS_NUMBER]	= {HMI_BCK_DIRTY,HMI_BCK_DIRTY};	
	
	if((HMI_GFX_GET_STATUS(HMI_SEND_EVENT))&&
		(hmi_driver_woking_status_flag == HMI_FRAMEBUFFER_FREE))
	{	
		render_finished  = TRUE;
		if(render_finished == TRUE)
		{
#if HMI_PAGES_NUMBER > 0
		hmi_screen_id = hmi_driver_get_render_screen();
		hmi_clear_all_dirty_zone(index_free[hmi_screen_id]);		
		hmi_engine_get_multi_dirty_zone_page(dirty_zone[hmi_screen_id][index_free[hmi_screen_id]]);
		for(i = 0u;i < HMI_LAYER_MAX_CNT;i++)
		{
			layer_buf_nb	= HMI_TWO_BUFFER;
			if(layer_buf_nb == HMI_TWO_BUFFER)
			{
				if((dirty_zone[hmi_screen_id][index_free[hmi_screen_id]][i].w > 0)&&(dirty_zone[hmi_screen_id][index_free[hmi_screen_id]][i].h > 0))
				{
					dirty			= TRUE;
					hmi_add_2buffer_dirty_zone(dirty_zone[hmi_screen_id][index_free[hmi_screen_id]],dirty_zone[hmi_screen_id][index_bck[hmi_screen_id]],
										dirty_zone[hmi_screen_id][HMI_2BUFFER_DIRTY],i);
				}
				else
				{
					hmi_copy_last_dirty_zone(dirty_zone[hmi_screen_id][index_bck[hmi_screen_id]],dirty_zone[hmi_screen_id][index_free[hmi_screen_id]],i);
				}
			}			
		}
		
		if(dirty == TRUE)
		{	
			/*get render from memory pool*/
		#if defined(HMI_GRAPHIC_VGLITE) 
			hmi_get_render_buf(0/*0 layer*/);
		#endif
			/*clear dirty zone*/
		#if (HMI_CLEAR_DIRTY_ENABLE == YES)
			hmi_clear_dirty_layer(dirty_zone[hmi_screen_id][HMI_2BUFFER_DIRTY]);
		#endif
			/* get new free buffer for every dirty layer  and clear dirty layer first*/
				//hmi_driver_set_render_buffer(dirty_zone[hmi_screen_id][HMI_2BUFFER_DIRTY]);
			/*draw at new free buffer*/
			hmi_engine_draw_page(dirty_zone[hmi_screen_id][HMI_2BUFFER_DIRTY]);
			
			index_free[hmi_screen_id]	= (HMI_BUFFER_DIRTY_STR)(HMI_BCK_DIRTY - index_free[hmi_screen_id]);
			index_bck[hmi_screen_id]	= (HMI_BUFFER_DIRTY_STR)(HMI_BCK_DIRTY - index_bck[hmi_screen_id]);
		}
		HMI_GFX_CLEAR_STATUS(HMI_SEND_EVENT);
		HMI_GFX_CLEAR_STATUS(HMI_RGL_PROP_EVENT);
#endif
		}
	}
	else /*no flag,example page alpha*/
	{
		
	}
}




#endif

void hmi_gfx_loop(HMI_TIME dt)
{
	//printf("hmi_gfx_loop ss\r\n");
	hmi_gfx_update(dt);
	#if HMI_SCREEN0_ENABLE==1U
		hmi_driver_set_render_screen(HMI_LAYER_SCREEN0);
		hmi_gfx_render();
	#endif
	#if HMI_SCREEN1_ENABLE==1U
		hmi_driver_set_render_screen(HMI_LAYER_SCREEN1);
		hmi_gfx_render();
	#endif
       //printf("hmi_gfx_loop ee\r\n");         
}

#if 0
void hmi_Loop(void)
{
	hmi_gfx_loop(0.04);
}
#endif
