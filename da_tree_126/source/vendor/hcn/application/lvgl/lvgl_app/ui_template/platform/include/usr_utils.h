/*
 * usr_utils.h —— hcn_application 侧「类型契约头」（只提供类型/宏）
 *
 * 本头不声明任何中间件函数（那 12 个接口属于 <mw/usr_utils.h>）。
 * ui/ 对它们的调用一律走 vehicle_services 的 vs_*；pc 侧的替身声明在 platform/pc/mock_mw_api.h。
 */
#ifndef __USR_UTILS_H__
#define __USR_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>

#define TEXT_PARAM_LEN      	(128)    ///< 字符串数据最大长度

typedef struct {
    int year;
    int month;
    int day;
    char version[20]; ///< 版本号
} parse_info_t;

/* ---- 以下为 hcn_application 自有（非中间件）接口 ---- */

/**
 * @brief  解析 MCU 版本号字符串
 * @param  ver 版本号字符串
 * @param  info 解析结果
 * @return 0:成功 -1:失败
 */
int parse_mcu_ver_format_by_strtok(const char *ver, parse_info_t *info);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // USR_UTILS_H__
