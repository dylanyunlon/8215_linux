#ifndef DAL_AEE_H
#define DAL_AEE_H

//#include "DAL_hw.h"
#include "x_lint.h"

#include "x_hal_io.h"
/*#include "x_hal_8520.h"*/
/*#include "x_hal_926.h"*/


#ifndef __ARM2__
#include <media/atc/drv_osd_if.h>
#else
#include "drv_osd_if.h"
#endif
#include "chip_ver.h"

#define DAL_LOG_LVL_OFF                         0
#define DAL_LOG_LVL_ERR                         1
#define DAL_LOG_LVL_WARN                        2
#define DAL_LOG_LVL_CLI                         3
#define DAL_LOG_LVL_INFO                        4
#define DAL_LOG_LVL_HAL                         5
#define DAL_LOG_LVL_IRQ                         6
#define DAL_LOG_LVL_TRACE                       7
#define DAL_LOG_LVL_DBG                         8
#define DAL_LOG_LVL_REGRW                       9

EXTERN __s32 _OSD_RGN_SetAlpha(__u32 u4Region, __u32 u4Value);
EXTERN __s32 _OSD_RGN_SetBlendMode(__u32 u4Region, __u32 u4Value);
extern __s32 OSD_BASE_SetOsdPosition_DAL(__u32 u4Plane, __u32 u4X, __u32 u4Y);
extern __s32 OSD_SC_Scale_DAL(__u32 u4Scaler, __u32 u4Enable, __u32 u4SrcWidth,
		   __u32 u4SrcHeight, __u32 u4DstWidth, __u32 u4DstHeight);
extern __s32 OSD_RGN_LIST_Create_DAL(__u32 *pu4RgnList);
extern __s32 OSD_RGN_Create_DAL(__u32 *pu4Region, __u32 u4BmpWidth, __u32 u4BmpHeight,
		     void *pvBitmap, __u32 eColorMode, __u32 u4BmpPitch,
		     __u32 u4DispX, __u32 u4DispY,
		     __u32 u4DispW, __u32 u4DispH);
extern __s32 OSD_RGN_LIST_DetachAll_DAL(__u32 u4RgnList);
extern __s32 OSD_RGN_Insert_DAL(__u32 u4Region, __u32 u4RgnList);
extern void SetPlaneRgnDAL(__u32 u4Plane, __u32 u4Rgn);
extern __s32 i4OsdPlaneFlipToDAL(__u32 u4Plane, __u32 u4RgnList);
extern __s32 i4OsdPlaneEnbleDAL(__u32 u4Plane, __u32 fgEnble);

#ifdef CONFIG_ATC_OS_linux
__u32 dal_base = 0;
__u32 dal_size = 0;
#endif
__u32 fbm_base = 0;
__u32 fbm_size = 0;

__u32 _u4Tm070ddhg=0;

__u8 *DAL_lvl_str[] = {
	"OFF",
	"ERR",
	"WARN",
	"CLI",
	"INFO",
	"HAL",
	"IRQ",
	"TRACE",
	"DBG",
	"REGRW",
};


__u32 DAL_log_lvl = DAL_LOG_LVL_HAL;

#define DAL_PRINT(lvl, tag, format, ...) \
{ \
	if (lvl <= DAL_log_lvl) { \
                if(strcmp(tag,"") == 0) { \
                        if (lvl == DAL_LOG_LVL_ERR) { \
        			pr_err("[DAL] %s: "format, DAL_lvl_str[lvl], ##__VA_ARGS__); \
        		} else if (lvl == DAL_LOG_LVL_WARN) { \
        			pr_warn("[DAL] %s: "format, DAL_lvl_str[lvl], ##__VA_ARGS__); \
        		} else if (lvl == DAL_LOG_LVL_INFO) { \
        			pr_info("[DAL] %s: "format, DAL_lvl_str[lvl], ##__VA_ARGS__); \
        		} else if (lvl == DAL_LOG_LVL_DBG) { \
        			pr_debug("[DAL] %s: "format, DAL_lvl_str[lvl], ##__VA_ARGS__); \
        		} else { \
        			pr_debug("[DAL] %s: "format, DAL_lvl_str[lvl], ##__VA_ARGS__); \
        		} \
                } else { \        
        		if (lvl == DAL_LOG_LVL_ERR) { \
        			pr_err("[DAL][%s] %s: "format, tag, DAL_lvl_str[lvl], ##__VA_ARGS__); \
        		} else if (lvl == DAL_LOG_LVL_WARN) { \
        			pr_warn("[DAL][%s] %s: "format, tag, DAL_lvl_str[lvl], ##__VA_ARGS__); \
        		} else if (lvl == DAL_LOG_LVL_INFO) { \
        			pr_info("[DAL][%s] %s: "format, tag, DAL_lvl_str[lvl], ##__VA_ARGS__); \
        		} else if (lvl == DAL_LOG_LVL_DBG) { \
        			pr_debug("[DAL][%s] %s: "format, tag, DAL_lvl_str[lvl], ##__VA_ARGS__); \
        		} else { \
        			pr_debug("[DAL][%s] %s: "format, tag, DAL_lvl_str[lvl], ##__VA_ARGS__); \
        		} \
                } \
	} \
}
#endif
