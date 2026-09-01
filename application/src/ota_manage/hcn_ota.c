/**
 *
 * @file hcn_ota.c
 *
 * @brief This message displayed in Doxygen Files index
 *
 * @ingroup PackageName
 * (note: this needs exactly one @defgroup somewhere)
 *
 * @date	2025/11/18 17:19
 * @author och
 *
 */

#include <string.h>
#include "ota_manage/hcn_ota.h"


static hcn_update_info_t g_update_info = {
    .type = UPDATE_NONE,
    .status = UPDATE_STATUS_IDLE,
    .error = UPDATE_ERROR_NONE,
};

static const char* get_update_type_str(update_type_e type) {
    switch (type) {
        case UPDATE_USB_SOC:
            return "USB-SOC";
        case UPDATE_USB_MCU:
            return "USB-MCU";
        case UPDATE_SD_CARD:
            return "SD_CARD";
        case UPDATE_OTA:
            return "OTA";
        default:
            return "NONE";
    }
}

/**
 * @brief  更新进度回调函数
 * @param  type 更新类型
 * @param  error 错误码
 * @param  progress 更新进度，单位：百分比(0-100)
 * @return none
 */
static void on_update_process(update_type_e type, uint8_t error,
                              uint8_t progress) {
    return  0 ;
}


hcn_update_info_t* get_hcn_current_update_info(void) { return &g_update_info; }

void set_update_state_reset(void) {
    g_update_info.type = UPDATE_NONE;
    g_update_info.status = UPDATE_STATUS_IDLE;
    g_update_info.error = UPDATE_ERROR_NONE;
    g_update_info.progress = 0;
}
