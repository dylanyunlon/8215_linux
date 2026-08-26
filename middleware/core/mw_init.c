#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mw_log.h"
#include "mw_init.h"
#include "mw_version.h"
#include "mw_modules.h"
#include "hal_bsp.h"
#include "mw_common.h"
#include "set_param.h"
#include "osal.h"
#include "dev_config.h"
#if GPIO_LIGHT_ENABLE
#include "gpio_light.h"
#endif
#if MW_LIGHT_SENSOR_ENABLE
#include "mw_display_mode.h"
#endif
#if MW_CAN_MODULE_ENABLE
#include "mw_can.h"
#endif
#if MW_MSG_MANAGE_ENABLE
#include "msg_manage.h"
#endif
#if MW_MILEAGE_MAINTENCE_ENABLE
#include "mw_mileage_maintence.h"
#endif

int mw_init(void) {
    if (mw_common_init() != 0) {
        mw_log_error("mw_common_init failed!\n");
        goto error;
    }
    /* OSAL 初始化：Linux 启动软件定时器守护线程；RTOS 由内核托管。幂等。 */
    if (osal_init() != 0) {
        mw_log_error("osal_init failed!\n");
        goto error;
    }
    
#if GPIO_LIGHT_ENABLE
    if (light_gpio_init() != 0) {
        mw_log_error("light_gpio_init failed!\n");
    }
#endif

#if MW_LIGHT_SENSOR_ENABLE
    if (display_mode_init() != 0) {
        mw_log_error("display_mode_init failed!\n");
    }
#endif

#if MW_CAN_MODULE_ENABLE
    if (can_module_init() != 0) {
        mw_log_error("can_module_init failed!\n");
    }
#endif

#if MW_MSG_MANAGE_ENABLE
    if (msg_manage_init() != 0) {
        mw_log_error("msg_manage_init failed!\n");
    }
#endif
#if MW_MILEAGE_MAINTENCE_ENABLE
    if (mw_maintence_init() != 0) {
        mw_log_error("mw_maintence_init failed!\n");
    }
#endif

    const mw_version_info_t* v = mw_get_version_info();
    printf("[mw] mw_init OK\n");
    printf("[mw] version %s (build:%s %s os:%s board:%s modules:%d/%d)\n",
           v->version, v->build_id, v->build_date, v->os_name,
           hal_bsp_board_name(), mw_module_enabled_count(),
           (int)MW_MOD_ID_MAX);
    return 0;

error:
    return -1;
}
