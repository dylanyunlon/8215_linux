#ifndef _HMI_ALL_STRUCT_INCLUDE_H
#define _HMI_ALL_STRUCT_INCLUDE_H

/* include all file.h*/
#include "string.h"
#include "stdio.h"
#include "HMI_Data/hmi_element_id.h"

#if (defined(HMI_MCU_RT1170))
#if (HMI_JPG_BITMAPS_NUMBER > 0) 
#include "jpeglib.h"
#endif
#endif
#if (defined(HMI_MCU_AMT630H))
#if (HMI_JPG_BITMAPS_NUMBER > 0) 
#include "jpegdec.h"
#endif
#endif

#include "hmi_system.h"

#if defined (HMI_WINDOWS)	
#include <stdlib.h>

#include <wtypes.h>
#include <stdarg.h>
#include <tchar.h>

//#include "esUtil.h"
//#include "gl2ext.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#define GL_GLEXT_PROTOTYPES
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#elif defined (HMI_LINUX)
	#if defined( HMI_MCU_IMX6 )
		#include <stdio.h>
		#include <stdlib.h>
		#include <fcntl.h>
		#include <assert.h>
		#include <math.h>
		#include <signal.h>
		#define GL_GLEXT_PROTOTYPES
		#include "GLES2/gl2.h"
		#include "GLES2/gl2ext.h"
		//#include <GL/gl.h>     
		//#include "glad/glad.h"
		#include "EGL/egl.h"
		//#include "esUtil.h"
		//#include "GLESv2.h"
		//#include "glfw.h"
		//#include "FSL/fsl_egl.h"
			
		//#include <GL/glext.h>

	#endif
	#if defined( HMI_MCU_R_CAR )
		#include <stdio.h>
		#include <stdlib.h>
		#include <fcntl.h>
		#include <assert.h>
		#include <math.h>
		#include <signal.h>
		
		#define GL_GLEXT_PROTOTYPES
		#include <EGL/egl.h>
		#include <GLES2/gl2.h>
	#endif

#elif defined (HMI_QNX)
	
		#include <stdio.h>
		#include <stdlib.h>
		#include <fcntl.h>
		#include <assert.h>
		#include <math.h>
		#include <signal.h>

		#include "GLES2/gl2.h"
		#include "GLES2/gl2ext.h"

		#include "glad/glad.h"
		#include "EGL/egl.h"
		#include "hmi_engine.h"
		#include "hmi_opengles2_driver.h"
		
		//#include "GLESv2.h"
		//#include "FSL/fsl_egl.h"
			
//		#include <GL/gl.h>
//		#include <GL/glext.h>


#elif defined (HMI_FREE_RTOS)
	#if	defined( HMI_MCU_RT1170 )|| defined(HMI_MCU_RT1172 )
		#include "FreeRTOS.h"
		#include "task.h"

		#include "fsl_debug_console.h"
		#include "board.h"

		#include "vglite_support.h"
		#include "vglite_window.h"
	#elif defined( HMI_MCU_AMT630H )
		#include "FreeRTOS.h"
		#include "rtos.h"
		//#include "vg_driver.h"
		#include <vg/openvg.h>
		#include <vg/vgu.h>
		#include <EGL/egl.h>
		#include <stdio.h>
		#include <assert.h>
		#include <string.h>
		#include <time.h>
		#include <math.h>
		#include <vg_lcdc.h>
		#include <itu.h>
	#endif

#elif defined (HMI_SYLIXOS)
	#if defined( HMI_MCU_IMX6 )
		#include <stdio.h>
		#include <stdlib.h>
		#include <fcntl.h>
		#include <assert.h>
		#include <math.h>
		#include <sys/select.h>

		#include "GLES2/gl2.h"
		#include "GLES2/gl2ext.h"

		#include "EGL/egl.h"
		#include "FSL/fsl_egl.h"
	//#elif defined (HMI_QNX)
	#endif
	
#if defined( HMI_MCU_ANY )
	#include <stdlib.h>
	#include <stdio.h>
	#include <unistd.h>
	#include <memory.h>
	#include <EGL/egl.h>
	#include <GLES2/gl2.h>
	#include <sys/time.h>
	#include <EGL/eglext.h>
#endif

#if defined( HMI_MCU_T3 )
	#include <stdlib.h>
	#include <stdio.h>
	#include <unistd.h>
	#include <memory.h>
	#include <EGL/egl.h>
	#include <GLES2/gl2.h>
	#include <sys/time.h>
	#include <EGL/eglext.h>
#endif


#elif defined (HMI_NO_OS)

#else
	#error hmi_opengles2_driver.c: Not support th OS.
#endif

#ifdef WIN32
 
#endif 

#if	 defined(S6J3200_GRAPHIC)
#if 0
#include "base_types.h"
#include "mm_types.h"
#include "mm_defines.h"

#include "mcu_select.h"
#include "mcu_settings.h"
#include "start.h"
#include "abstract.h"
#include "pdl.h"

#include "app/app_stateManager.h"
#include "app/app_modeManager.h"
#include "app/centralCtrl_Manager.h"
#include "modules/timer/timer.h"
#include "appUI/setup.h"

//#include "modules.h"
#include "config.h"


#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/*************/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "mml_gdc_driver_api.h"

#include "sm_util.h"
#include "pe_matrix.h"
#include "ut_compatibility.h"
#include "de_util.h"
#include "ut_memman.h"
#include "ut_printf.h"
#include "ut_config.h"
#include "dbg_win.h"
#include "pe_matrix.h"
#include "pe_drawing.h"
#endif
#include "cy_project.h"
#include "cy_device_headers.h"
#include "cygfx_driver_api.h"

#include "sm_util.h"
#include "pe_matrix.h"
#include "ut_compatibility.h"
#include "de_util.h"
#include "ut_memman.h"
#include "ut_printf.h"
#include "ut_config.h"
#include "ut_disp.h"
#include "ut_disp_panels.h"

#endif

#include "hmi_engine_cfg.h"
#include "hmi_driver_cfg.h"
#include "hmi_engine.h"

#include "HMI_Data/hmi_res_font.h"

#if 0
#if  defined(HMI_GRAPHIC_ST7513)
	#include "hmi_driver.h"
#elif defined( HMI_GRAPHIC_OPENGLES )
	#include "hmi_opengles2_driver.h"
#endif
#endif
#include "hmi_reset.h"
//#include "hmi_gfx_reset_driver.h" //lyq
#include "hmi_status.h"

#include "hmi_loop_driver.h"
#include "hmi_loop.h"/* after loop driver*/
#include "hmi_task_touchpanel.h"
#include "hmi_user_interface.h" 

#include "hmi_action_engine.h"
#if  defined(HMI_GRAPHIC_ST7513)
#include "hmi_driver.h"
#elif defined( HMI_GRAPHIC_OPENGLES )
#include "hmi_opengles2_driver.h"
#endif

/*#include "hmi_task_touchpanel.h"*/
#if  defined(HMI_MCU_TW36)
#include "hmi_tw8836_driver.h"
#endif

#if	 defined(S6J3200_GRAPHIC)
#include "hmi_rgl_driver.h"
#ifndef WIN32
#define MM_GDC_HWEB_MA_READ
#define MM_GDC_HWEB_MA_WRITE
#define MM_GDC_HWEB_MA_READ_WRITE
#define MM_GDC_LOCK(ADD, CNT, ACCESS) 
#define MM_GDC_UNLOCK(ADD)
#endif
#endif


#ifdef HMI_GRAPHIC_RGL
#include "r_typedefs.h"         /* Renesas basic types, e.g. uint32_t */
#ifdef USE_ROS
#include "fw_osal_api.h"
#endif
#endif

#if ((defined( HMI_MCU_RH850_D1MX ))||(defined( HMI_MCU_RH850_D1HX ))|| \
		(defined( HMI_MCU_RH850_D1LX )))

//#ifdef USE_ROS
#include "r_drw2d_os.h"
//#endif
#include "r_drw2d_api.h"
#include "r_config_drw2d.h"
//#ifdef R_DRW2D_SYS_DHD
#include "r_cdi_api.h"
#endif

#if ((defined( HMI_MCU_RH850_D1MX ))||(defined( HMI_MCU_RH850_D1HX ))|| \
		(defined( HMI_MCU_RH850_D1LX )))
#if ((defined( HMI_MCU_RH850_D1MX ))||(defined( HMI_MCU_RH850_D1HX )))
#include "davehd_driver.h"
//#include "r_util_dhd.h"//removed by pxguo
#include "r_drw2d_ctx_dhd.h"
#include "r_jcua_api.h"
#else
#include "r_drw2d_ctx_cpu.h"
#endif
#include "r_ddb_api.h"//befor r_wm_api.h
#include "r_wm_api.h"//befor the wm.h
//#include "wm.h"//added by pxguo
#include "r_sfma_api.h"

#include "r_config_wm.h"

#include "hmi_rgl_driver.h"
#endif




#if defined(HMI_GRAPHIC_YGV641)
#include "Yvc641.h"
#elif defined(HMI_GRAPHIC_YGV642)
#include "Yvc642.h"
#endif

#if (defined(HMI_GRAPHIC_YGV641)||defined(HMI_GRAPHIC_YGV642))
#include "Yvc1Ddrv.h"
#include "Yvc1Lyr.h"
#include "VC1N_includes.h"
#include "hmi_yamaha_driver.h"
#endif

#if defined(HMI_GRAPHIC_VGLITE)
#include "vg_lite.h"
#endif

#if (defined(HMI_MCU_RT1170)||defined(HMI_MCU_RT1172))
#include "pin_mux.h"
#include "fsl_soc_src.h"
//#include "hmi_rgl_driver.h"

//#if (HMI_JPG_BITMAPS_NUMBER > 0) 
//#include "jpeglib.h"
//#endif
#include "hmi_rgl_driver.h"


#endif

#if	defined( HMI_MCU_AMT630H )
#include "hmi_rgl_driver.h"
	#if(HMI_ENABLE_BIN == HMI_YES)
		#include	"Sfud_def.h"	
	#endif
#endif


#ifdef WIN32
	/*#include "win32_redraw.h"*/
	/*#include "hmi_gfx_gdi_status.h"*/
#endif

#endif 


