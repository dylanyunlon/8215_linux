#ifndef __DEV_CONFIG_H__
#define __DEV_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include "rtconfig.h"

///< 型号相关配置
#define APP_VERSION_HEAD    "HCN_BB001_01_"
#define MANUFACTURER_NAME   "HCN"
#define CUSTMOER_NAME       "HCN"
#define CAR_MODEL           "bb001"
#define CONFIG_PARAM_VER    "v001"
#define CAR_SUB_MODEL       "01"
#define UI_VERSION          "V1.0.0"
#define DATA_SRC_PATH       "/data"
#define USE_CUSTOM_UI       (1)

///< 手机互联宽度和高度设置
#define CARLINK_EC_WIDTH    (480)
#define CARLINK_EC_HEIGHT   (272)

///< 设置参数使能
#define MW_SET_PARAM_ENABLE 1

///< 里程参数使能
#define MILEAGE_PARAM_ENABLE 1

///< PWM背光使能
#define MW_BL_PW_ENABLE   1

#if MW_BL_PW_ENABLE
#define LCD_PWM_CH  1
#endif

///< can功能使能
#define MW_CAN_ENABLE  1
///< adc按键使能
#define MW_ADC_KEY_ENABLE   1

#if MW_ADC_KEY_ENABLE
#define MW_ADC_KEY_CHANNEL  (0)
#endif

///< 手机互联使能
#define  MW_CARLINK_EC_ENABLE 1
#define  MW_CARLINK_EC_UUID  ("HNCX04XXXXXXHCN012")

///< io灯光使能
#define GPIO_LIGHT_ENABLE 1

#define AVG_CONSUMPTION_ENABLE 0

#define POWER_MANAGE_ENABLE 1

#define WIRELESS_UNLOCK_ENABLE 1

#define SPEED_PWM_ENABLE 0

#define TRACE_POINT_ENABLE 0

///< OTA升级管理使能
#define MW_OTA_MANAGE_ENABLE 1

#if MW_OTA_MANAGE_ENABLE
///< OTA WiFi默认配置
#define OTA_DEFAULT_SSID    "ota630HCN"
#define OTA_DEFAULT_PASSWD  "66666666"
///< OTA触发：开机自检后长按上键等待时间(ms)
#define OTA_KEY_WAIT_TIMEOUT_MS (30000)
///< OTA WiFi连接重试次数
#define OTA_STA_CONNECT_RETRY   (1000)
#endif

///< NFC功能使能
#define NFC_ENABLE  1

#if NFC_ENABLE
///< NFC I2C通信设备名
#define NFC_I2C_DEV_NAME   "i2c0"
///< NFC I2C从机地址
#define NFC_I2C_ADDR       0x00
///< NFC NPD管脚 (Power Down)
#define NFC_NPD_PIN        "PD.4"
///< NFC IRQ管脚
#define NFC_IRQ_PIN        "PA.10"
///< NFC学习模式最大卡片数
#define NFC_LEARNED_CARD_MAX  2
#endif

///< 简易导航使能
#if !MW_CARLINK_EC_ENABLE
#define MW_EASY_NAVI_ENABLE  1
#endif

#if MW_CARLINK_EC_ENABLE && MW_EASY_NAVI_ENABLE
#error "can't enable uart ble and calink"
#endif

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // __DEV_CONFIG_H__
