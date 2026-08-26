/** 模块注册表 -- 中央静态表，enabled 由 dev_config.h 裁剪宏编译期决定 */
#include "mw_modules.h"
#include <stdio.h>
#include "dev_config.h"
#include "mw_lock.h"

#define ENABLED(expr) ((expr) ? 1 : 0)

static const mw_module_info_t s_mw_modules[MW_MOD_ID_MAX] = {
    [MW_MOD_OSAL] = {"osal", "系统抽象(线程/互斥/信号量/定时器/MQ)", 1},
    [MW_MOD_HAL_GPIO] = {"hal_gpio", "极简 GPIO HAL", 1},
    [MW_MOD_HAL_KEY] = {"hal_key", "按键采集 HAL(ADC/GPIO)", 1},
    [MW_MOD_HAL_PWM] = {"hal_pwm", "极简 PWM HAL(背光)", 1},
    [MW_MOD_HAL_I2C] = {"hal_i2c", "极简 I2C HAL", 1},
    [MW_MOD_HAL_ADC] = {"hal_adc", "极简 ADC HAL(iio/板级回调)", 1},
    [MW_MOD_HAL_CAN] = {"hal_can", "极简 CAN HAL(SocketCAN/板级)", 1},
    [MW_MOD_HAL_BSP] = {"hal_bsp", "板级参数统一注册", 1},
    [MW_MOD_TASK_SERVER] = {"task_server", "后台任务服务器", 1},
    [MW_MOD_WATCHDOG] = {"watchdog", "看门狗+线程心跳健康",
                         ENABLED(MW_WATCHDOG_ENABLE)},
    [MW_MOD_SET_PARAM] = {"set_param", "用户参数管理(读写/JSON/迁移)",
                          ENABLED(MW_SET_PARAM_ENABLE)},
    [MW_MOD_VEHICLE_PARAM] = {"vehicle_param", "车辆数据", 1},
    [MW_MOD_DEV_STATE] = {"dev_state", "设备状态+变更分发", 1},
    [MW_MOD_CAN_MODULE] = {"can_module", "CAN 收发(解析/周期发送/超时清零)",
                         ENABLED(MW_CAN_MODULE_ENABLE)},
    [MW_MOD_KEY_MODULE] = {
        "key_module",
#if MW_KEY_USE_GPIO
        "GPIO 独立按键",
#else
        "ADC 电压梯按键",
#endif
        ENABLED(MW_ADC_KEY_ENABLE || MW_KEY_USE_GPIO)},
    [MW_MOD_UART_TPMS] = {"uart_tpms", "TPMS 胎压串口协议", 1},
    [MW_MOD_IO_LIGHT] = {"io_light", "仪表灯 GPIO 检测",
                         ENABLED(GPIO_LIGHT_ENABLE)},
    [MW_MOD_BACKLIGHT] = {"backlight", "背光 PWM 调光",
                          ENABLED(MW_BL_PWM_ENABLE)},
    [MW_MOD_LIGHT_SENSOR] = {"light_sensor", "环境光 ADC 采集",
                             ENABLED(MW_LIGHT_SENSOR_ENABLE)},
    [MW_MOD_DISPLAY_MODE] = {"display_mode", "日/夜间模式+自动背光/大灯",
                             ENABLED(MW_LIGHT_SENSOR_ENABLE)},
    [MW_MOD_CJSON] = {"cjson", "cJSON/JSON 解析(第三方)", 1},
    [MW_MOD_MSG_MANAGE] = {"msg_manage", "存储插拔消息(/proc/mounts 轮询)",
                           ENABLED(MW_MSG_MANAGE_ENABLE)},
    [MW_MOD_MAINTENCE] = {"maintence", "里程保养提醒(轮询计算剩余里程)",
                          ENABLED(MW_MILEAGE_MAINTENCE_ENABLE)},
};

const mw_module_info_t* mw_module_get_info(mw_module_id_e id) {
    if (id < 0 || id >= MW_MOD_ID_MAX) return NULL;
    return &s_mw_modules[id];
}

int mw_module_enabled(mw_module_id_e id) {
    const mw_module_info_t* m = mw_module_get_info(id);
    return m ? m->enabled : 0;
}

int mw_module_enabled_count(void) {
    int n = 0;
    for (int i = 0; i < (int)MW_MOD_ID_MAX; i++) {
        if (s_mw_modules[i].enabled) n++;
    }
    return n;
}

/* ---------------- 运行时自检钩子 ---------------- */

static mw_module_check_fn_t s_checks[MW_MOD_ID_MAX];
static osal_mutex_t s_check_lock = OSAL_MUTEX_INIT;

void mw_module_register_check(mw_module_id_e id, mw_module_check_fn_t fn) {
    if (id < 0 || id >= MW_MOD_ID_MAX) return;
    MW_MUTEX_GUARD(&s_check_lock);
    s_checks[id] = fn;
}

mw_chk_e mw_module_check(mw_module_id_e id) {
    if (id < 0 || id >= MW_MOD_ID_MAX) return MW_CHK_NONE;
    mw_module_check_fn_t fn;
    {
        MW_MUTEX_GUARD(&s_check_lock);
        fn = s_checks[id];
    }
    if (!fn) return MW_CHK_NONE;
    return fn() == 0 ? MW_CHK_OK : MW_CHK_FAULT;
}

void mw_module_dump(void) {
    printf("%-14s %-4s %-6s %s\n", "module", "on", "chk", "desc");
    for (int i = 0; i < (int)MW_MOD_ID_MAX; i++) {
        const mw_module_info_t* m = &s_mw_modules[i];
        const char* chk = "-";
        if (m->enabled) {
            switch (mw_module_check((mw_module_id_e)i)) {
            case MW_CHK_OK: chk = "ok"; break;
            case MW_CHK_FAULT: chk = "FAULT"; break;
            default: break;
            }
        }
        printf("%-14s %-4s %-6s %s\n", m->name,
               m->enabled ? "yes" : "no", chk, m->desc);
    }
    printf("modules: %d/%d enabled\n", mw_module_enabled_count(),
           (int)MW_MOD_ID_MAX);
}
