#ifndef __MW_MODULES_H__
#define __MW_MODULES_H__

/**
 * @file mw_modules.h
 * @brief 模块注册表 -- 对外自描述：本 lib_mw.a 编入了哪些模块
 *
 * 中央静态表（mw_modules.c），enabled 由 dev_config.h 裁剪宏编译期
 * 决定。编译期裁剪代码路径仍用原 dev_config 宏；本表用于运行时
 * 查询/诊断/外部降级判断（如 UI 探测按键模块是否存在）。
 *
 * 对外用法：
 *   #include <mw/mw_modules.h>
 *   if (!mw_module_enabled(MW_MOD_KEY_MODULE)) { ... }
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum {
    MW_MOD_OSAL = 0,       ///< OSAL 系统抽象层
    MW_MOD_HAL_GPIO,       ///< GPIO HAL
    MW_MOD_HAL_KEY,        ///< 按键采集 HAL
    MW_MOD_HAL_PWM,        ///< PWM HAL
    MW_MOD_HAL_I2C,        ///< I2C HAL
    MW_MOD_HAL_ADC,        ///< ADC HAL
    MW_MOD_HAL_CAN,        ///< CAN HAL
    MW_MOD_HAL_BSP,        ///< 板级参数统一注册
    MW_MOD_TASK_SERVER,    ///< 后台任务服务器
    MW_MOD_WATCHDOG,       ///< 看门狗+线程心跳健康
    MW_MOD_SET_PARAM,      ///< 用户参数管理
    MW_MOD_VEHICLE_PARAM,  ///< 车辆数据
    MW_MOD_DEV_STATE,      ///< 设备状态(含变更分发)
    MW_MOD_CAN_MODULE,     ///< CAN 收发(解析/周期发送/超时)
    MW_MOD_KEY_MODULE,     ///< 按键模块
    MW_MOD_UART_TPMS,      ///< TPMS 串口
    MW_MOD_IO_LIGHT,       ///< 仪表灯 IO
    MW_MOD_BACKLIGHT,      ///< 背光 PWM 调光
    MW_MOD_LIGHT_SENSOR,   ///< 环境光 ADC 采集
    MW_MOD_DISPLAY_MODE,   ///< 日/夜间模式+自动背光/大灯
    MW_MOD_CJSON,          ///< cJSON/JSON 工具(第三方)
    MW_MOD_MSG_MANAGE,     /*< 存储插拔消息(SD/USB, /proc/mounts 轮询) */
    MW_MOD_MAINTENCE,      ///< 里程保养提醒(轮询计算剩余里程)
    MW_MOD_ID_MAX
} mw_module_id_e;

typedef struct {
    const char* name;  ///< 模块名
    const char* desc;  ///< 一句话描述
    uint8_t enabled;   ///< 1=已编入本库
} mw_module_info_t;

/** 取模块信息（id 越界返回 NULL） */
const mw_module_info_t* mw_module_get_info(mw_module_id_e id);

/** 模块是否编入本库（id 越界返回 0） */
int mw_module_enabled(mw_module_id_e id);

/** 已编入模块数（诊断汇总用） */
int mw_module_enabled_count(void);

/** 打印整表（对齐格式，诊断/CLI 用） */
void mw_module_dump(void);

/* ---------------- 运行时自检钩子 ---------------- */

/** 自检函数原型：返回 0=正常，<0=异常（模块各自定义判定标准） */
typedef int (*mw_module_check_fn_t)(void);

/** 自检结果 */
typedef enum {
    MW_CHK_NONE = 0, ///< 模块未注册自检
    MW_CHK_OK,       ///< 自检通过
    MW_CHK_FAULT,    ///< 自检失败
} mw_chk_e;

/** 注册模块自检钩子（模块 init 时调用；id 越界忽略）。
 *  core 不反向依赖各模块 -- 钩子由模块自己挂上来。 */
void mw_module_register_check(mw_module_id_e id, mw_module_check_fn_t fn);

/** 执行模块自检（id 越界/未注册返回 MW_CHK_NONE） */
mw_chk_e mw_module_check(mw_module_id_e id);

#ifdef __cplusplus
}
#endif
#endif /* __MW_MODULES_H__ */
