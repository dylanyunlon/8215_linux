#ifndef __DEV_CONFIG_H__
#define __DEV_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>

#define APP_VERSION_HEAD "HCN_8215E_01_"
#define MANUFACTURER_NAME "HCN"
#define CUSTMOER_NAME "HCN"
#define CAR_MODEL "8215E"
#define CONFIG_PARAM_VER "v001"
#define CAR_SUB_MODEL "01"
#define UI_VERSION "V1.0.0"

#define GPIO_LIGHT_ENABLE 0
#define MW_BL_PWM_ENABLE 0

/* ---------- 背光 PWM 通道（hal_pwm 板级语义）---------- */
#ifndef MW_LCD_PWM_CH
#define MW_LCD_PWM_CH 0
#endif

#define MW_SET_PARAM_ENABLE 1

/* ---------- 看门狗+线程心跳(module/watchdog) ----------
 * 1=启用（硬件狗不可用自动退化为纯软件监测）
 * 0=关闭：mw_watchdog_* 变空转内联桩（调用点无需 #if），注册表 off */
#ifndef MW_WATCHDOG_ENABLE
#define MW_WATCHDOG_ENABLE 1
#endif

/* ---------- 按键(key_module)板级配置 ----------
 * ADC 模式(MW_KEY_USE_GPIO=0)：MW_ADC_KEY_CHANNEL 一路电压梯，
 *   窗口表(UP/DOWN/ENTER/BACK_AD)在 adc_key.c。
 * GPIO 模式(MW_KEY_USE_GPIO=1)：每键一个 GPIO（hal_gpio 逻辑名），
 *   按下电平 = MW_KEY_GPIO_ACTIVE_LEVEL。
 */
#ifndef MW_ADC_KEY_ENABLE
#define MW_ADC_KEY_ENABLE 0
#endif
#ifndef MW_ADC_KEY_CHANNEL
#define MW_ADC_KEY_CHANNEL 0 /* ADC 通道号（板级语义）*/
#endif
#ifndef MW_KEY_USE_GPIO
#define MW_KEY_USE_GPIO 0 /* 1=GPIO 独立按键；0=ADC 电压梯按键 */
#endif
#ifndef MW_KEY_GPIO_ACTIVE_LEVEL
#define MW_KEY_GPIO_ACTIVE_LEVEL 0 /* GPIO 模式按下电平：0=低 */
#endif

#ifndef MW_KEY_GPIO_UP
#define MW_KEY_GPIO_UP "PG.0"
#endif
#ifndef MW_KEY_GPIO_DOWN
#define MW_KEY_GPIO_DOWN "PG.1"
#endif
#ifndef MW_KEY_GPIO_ENTER
#define MW_KEY_GPIO_ENTER "PG.2"
#endif
#ifndef MW_KEY_GPIO_BACK
#define MW_KEY_GPIO_BACK "PG.3"
#endif

#ifndef MW_LIGHT_SENSOR_ENABLE
#define MW_LIGHT_SENSOR_ENABLE 1
#endif
#ifndef MW_LIGHT_ADC_CHANNEL
#define MW_LIGHT_ADC_CHANNEL 7 
#endif


/* ---------- CAN 模块(module/can_module) ----------
 * MW_CAN_MODULE_ENABLE  : 1=启用(编译进库, mw_init 拉起收发线程)
 * MW_CAN_DEV_NAME       : hal_can 通道名(Linux 映射 SocketCAN 接口名)
 * MW_CAN_BAUD           : 波特率(仅诊断输出; Linux 由内核侧 iproute2 配置)
 * MW_CAN_STB_ENABLE     : 收发器 STB 引脚控制(经 hal_gpio 逻辑名拉低)
 * MW_CAN_MILE_MSG_ENABLE: 0x776~0x779 里程/清 EEPROM 报文
 *   (依赖原工程 storage_param2 与 uart 下发命令模块, 未迁入前默认关闭;
 *    迁入后置 1 即启用, 报文解析逻辑已就位)
 */
#ifndef MW_CAN_MODULE_ENABLE
#define MW_CAN_MODULE_ENABLE 0
#endif
#ifndef MW_CAN_DEV_NAME
#define MW_CAN_DEV_NAME "can0"
#endif
#ifndef MW_CAN_BAUD
#define MW_CAN_BAUD 500000
#endif
#ifndef MW_CAN_STB_ENABLE
#define MW_CAN_STB_ENABLE 0
#endif
#ifndef MW_CAN_STB_PIN
#define MW_CAN_STB_PIN "CAN_STB"
#endif
#ifndef MW_CAN_MILE_MSG_ENABLE
#define MW_CAN_MILE_MSG_ENABLE 0
#endif


/* ---------- 存储插拔消息(module/msg_manage) ----------
 * MW_MSG_MANAGE_ENABLE  : 1=启用(/proc/mounts 轮询 worker 线程)
 * MW_MSG_POLL_MS        : 轮询周期(ms)
 * MW_MSG_MAX_DEVS       : 快照表最大跟踪条目数(分区数)
 * MW_MSG_MOUNT_PREFIXES : 关注挂载点前缀列表(逗号分隔字符串，默认 /media/ /mnt/ /sdcard/ /storage/)
 */
#ifndef MW_MSG_MANAGE_ENABLE
#define MW_MSG_MANAGE_ENABLE 1
#endif
#ifndef MW_MSG_POLL_MS
#define MW_MSG_POLL_MS 500
#endif
#ifndef MW_MSG_MAX_DEVS
#define MW_MSG_MAX_DEVS 8
#endif
#ifndef MW_MSG_MOUNT_PREFIXES
///< ATCMountService 实际挂在 /media/(udisk*/ext_sdcard*)，兼容 /mnt/ 等传统路径 
#define MW_MSG_MOUNT_PREFIXES "/media/", "/mnt/", "/sdcard/", "/storage/"
#endif

/* ---------- 里程保养提醒(module/maintence) ----------
 * MW_MILEAGE_MAINTENCE_ENABLE: 1=启用(轮询线程计算剩余保养里程)
 * MW_MAINT_POLL_MS            : 里程轮询周期(ms)
 * MW_MAINT_FIRST_KM           : 首保间隔(km，保养次数=0 时)
 * MW_MAINT_NEXT_KM            : 常规保养间隔(km)
 */
#ifndef MW_MILEAGE_MAINTENCE_ENABLE
#define MW_MILEAGE_MAINTENCE_ENABLE 0
#endif
#ifndef MW_MAINT_POLL_MS
#define MW_MAINT_POLL_MS 1000
#endif
#ifndef MW_MAINT_FIRST_KM
#define MW_MAINT_FIRST_KM 1000
#endif
#ifndef MW_MAINT_NEXT_KM
#define MW_MAINT_NEXT_KM 3000
#endif

#ifdef __cplusplus
}
#endif

#endif /* __DEV_CONFIG_H__ */