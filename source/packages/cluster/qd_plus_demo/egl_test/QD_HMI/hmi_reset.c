/*****************************************************************************

File Name        :  hmi_reset.c
Organization    :  Zhuli Electronics Co.Ltd in Shanghai (www.shzldz.com)
******************************************************************************/

#include "hmi_all_struct_include.h"

/*static BOOLEAN cold_reset=TRUE;*/
BOOLEAN hmi_is_cold_reset(void);

void hmi_gfx_cold_system(void)
{
#ifdef	HMI_TOUCH_PANEL
	hmi_touch_panel_cold_Init();
#endif

#ifdef HMI_8_DIRECT
	/*8_direct*/
#endif
	//hmi_user_process_init();//removed by pxguo init by hmi user process
#if	HMI_ALL_EVENT_NUMBER+HMI_ACTION_GROUP_NUMBER>0
	hmi_action_manager_init();
#endif
	HMI_GFX_STATUS_INIT();
	/*hmi_extern_interface_init();*/
#ifdef	HMI_MEMORY_MANAGER
	ssd_mem_init();
	hmi_memory_manager_init();
	hmi_resource_manager_init();
#endif
	/*hmi_miscellaneous_init();*/
	/*hmi_render_init();*/
	/*hmi_miscellaneous_driver_init();*/
	HMI_GFX_SET_STATUS(HMI_UNLOCK_DISPLAY_BUFFER);/* unlock display buffer*/
	//hmi_loop_init();
	hmi_engine_init();
	/*do the type of event  run*/
	#if HMI_EVENT_STAND_NUMBER>0
	hmi_do_event(0,HMI_EVENT_RUN);
	#endif
	hmi_user_process_init();
}
void hmi_gfx_warm_system(void)
{
}

void hmi_gfx_cold_init(void)
{
	hmi_gfx_cold_system();
	hmi_gfx_cold_user();
	HMI_GFX_SET_STATUS(HMI_COLD_RESET);
	/*cold_reset=FALSE;*/
}

void hmi_gfx_warm_init(void)
{
	hmi_gfx_warm_system();
	hmi_gfx_warm_user();
}

void hmi_init(void)
{
#if(HMI_ALLWAYS_COLOD_ENABLE ==	HMI_YES)
	hmi_gfx_cold_init();
#else
	if(hmi_is_cold_reset())/*cold reset*/
	{
		hmi_gfx_cold_init();
	}
	else /*warn reset*/
	{
		hmi_gfx_warm_init();
	}
#endif
}
/*return true is cold reset,else warm reset*/
BOOLEAN hmi_is_cold_reset(void)
{
	BOOLEAN cold_reset=FALSE;

	cold_reset=!(HMI_GFX_GET_STATUS(HMI_COLD_RESET));
	return cold_reset;
}

