/*
 * dev_ota.c —— OTA 升级信息（device 后端）
 *
 * 对应 platform/pc/mock_sdk.c 的 get_current_update_info()。
 *
 * 数据模型直接沿用 hcn_application 自己的 platform/include/ota_manage.h（update_info_t）。
 * 中间件侧没有 OTA 模块，本文件提供"可被写入的状态容器"：
 */
#include <stdio.h>
#include <string.h>
#include "ota_manage.h"

static update_info_t s_update_info = {
    .type     = UPDATE_NONE,
    .status   = UPDATE_STATUS_IDLE,
    .error    = UPDATE_ERROR_NONE,
    .progress = 0,
};

update_info_t *get_current_update_info(void)
{
    return &s_update_info;
}

void set_update_state_reset(void)
{
    s_update_info.type     = UPDATE_NONE;
    s_update_info.status   = UPDATE_STATUS_IDLE;
    s_update_info.error    = UPDATE_ERROR_NONE;
    s_update_info.progress = 0;
}

void set_update_state_start(update_type_e type)
{
    s_update_info.type     = type;
    s_update_info.status   = UPDATE_STATUS_IN_PROGRESS;
    s_update_info.error    = UPDATE_ERROR_NONE;
    s_update_info.progress = 0;
}

void set_update_state_progress(uint8_t progress, update_error_e error_type)
{
    s_update_info.status   = UPDATE_STATUS_IN_PROGRESS;
    s_update_info.progress = (progress > 100) ? 100 : progress;
    s_update_info.error    = error_type;
}

void set_update_state_finish(update_state_e final_status, update_error_e error_type)
{
    if (final_status == UPDATE_STATUS_SUCCESS) s_update_info.progress = 100;
    s_update_info.status = final_status;
    s_update_info.error  = error_type;
}
