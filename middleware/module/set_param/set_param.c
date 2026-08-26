#define _GNU_SOURCE /* glibc 扩展（递归锁已改 OSAL；此宏保留供其它 glibc 调用）*/
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "set_param.h"
#include "vehicle_param.h"
#include "json_param_parse.h"
#include "mw_log.h"
#include "mw_lock.h"
#include "usr_utils.h"
// #include "usr_backlight.h"
// #include "mw_sys_datetime.h"

#define USE_PARAM_PRINTF (1)

static usr_param_t usr_param;
static usr_param_t usr_param_pre = {
    .usr_set = {1},
    .carlink_uuid = {0xff},
};

static bool is_recovery_usr_param = false;
static bool start_by_acc = false;

/** usr_param 等被多线程并发访问，且存在嵌套调用(shutdown_save->set+save、
 * usr_param_init->save、clean_eeprom_operate->save)，使用递归互斥量避免自死锁。
 * 锁序: usr -> {json, vehicle}，json -> find，无环。 */
static osal_mutex_t s_usr_mutex; /* 递归互斥，惰性初始化（见 usr_mutex）*/
static osal_mutex_t s_init_lock =
    OSAL_MUTEX_INIT; /* 非递归，仅保护一次性 init */
static bool s_usr_mutex_inited = false;

/** 递归互斥不能静态初始化（RTOS 不支持），故首次使用经 s_init_lock
 * 保护惰性创建为递归锁。 s_init_lock 为非递归（Linux 静态初始化 / RTOS
 * 惰性创建），临界区极短。 */
static osal_mutex_t* usr_mutex(void) {
    MW_MUTEX_GUARD(&s_init_lock);
    if (!s_usr_mutex_inited) {
        if (osal_mutex_init(&s_usr_mutex, true) != OSAL_OK) return NULL;
        s_usr_mutex_inited = true;
    }
    return &s_usr_mutex;
}

/** 递归自锁 RAII 守卫：先确保初始化，再加锁；出作用域自动解锁（复用 mw_lock
 * cleanup）*/
#define USR_MUTEX_GUARD()                                           \
    __attribute__((cleanup(_mw_mutex_cleanup)))                     \
    osal_mutex_t* _mw_g_##__COUNTER__ = ({                          \
        osal_mutex_t* _m = usr_mutex();                             \
        (_m != NULL && osal_mutex_lock(_m) == OSAL_OK) ? _m : NULL; \
    })

#if USE_PARAM_PRINTF
static void printf_usr_param(usr_param_t* param) {
    if (param) {
        printf("\r\n ...............usr param start......................\r\n");
        printf("maintain_count:%d\r\n",
               param->maintain_info.maintain_mile.maintain_count);
        printf("maintain_mileage:%u\r\n",
               param->maintain_info.maintain_mile.last_maintain_total_mileage);
        printf("cur_maintain_mileage:%d\r\n",
               param->maintain_info.maintain_mile.cur_maintain_mileage);

        printf("maintain_days:%d\r\n",
               param->maintain_info.maintain_time.maintain_days);
        printf("is sync time:%d\r\n",
               param->maintain_info.maintain_time.is_sync_time);
        printf("maintain data:%04d/%02d/%02d\r\n",
               param->maintain_info.maintain_time.last_maintain_date.year,
               param->maintain_info.maintain_time.last_maintain_date.mon,
               param->maintain_info.maintain_time.last_maintain_date.day);

        printf("mile_format :%d\r\n", param->usr_set.mile_format);
        printf("mile display:%d\r\n", param->usr_set.mile_display);
        printf("theme :%d\r\n", param->usr_set.theme);
        printf("start_src :%d\r\n", param->usr_set.start_src);
        printf("brightness level :%d\r\n", param->usr_set.brightness);
        printf("system language :%d\r\n", param->usr_set.language);
        printf("bt_switch:%d\r\n", param->usr_set.bt_switch);
        printf("uuid_active_status:%d\r\n", param->usr_set.uuid_active_staus);
        printf("meter start src:%d\r\n", param->usr_set.start_src);
        printf("tpms Press Unit:%d\r\n", param->usr_set.tpms_unit);
        printf("tcs_switch:%d\r\n", param->usr_set.tcs_switch);
        printf("time_format:%d\r\n", param->usr_set.time_format);
        printf("temp_unit:%d\r\n", param->usr_set.temp_unit);
        printf("phone_type:%d\r\n", param->usr_set.phone_type);
        printf("sys_log:%d\r\n", param->usr_set.sys_log);
        printf("drive_mode:%d\r\n", param->usr_set.drive_mode);

        printf("ride_time_a:%u\r\n", param->ride_info.ride_time_a);
        printf("ride_time_b:%u\r\n", param->ride_info.ride_time_b);
        printf("mcu_update:%d\r\n", param->mcu_update.update_type);
        printf("mcu update_len:%d\r\n", param->mcu_update.update_mcu_len);
        for (int i = 0; i < MAX_WHEEL_POS_NUM; i++) {
            printf("tpms_info[%d].id:0x%x\n", i, param->tpms[i].tpms_id);
            printf("tpms_info[%d].pressure:%d\n", i,
                   param->tpms[i].tpms_pressure);
            printf("tpms_info[%d].temp:%d\n", i, param->tpms[i].tpms_temp);
        }

        printf("\r\nuuid:");
        for (int i = 0; i < 20; i++) {
            printf("%c", param->carlink_uuid[i]);
        }
        printf("\r\n");
        printf("\r\n ...............usr param end......................\r\n");
    }
}
#endif

static void check_usr_param(void) {
    if (usr_param.usr_set.mile_format > 1) {
        usr_param.usr_set.mile_format = 0;
    }

    if (usr_param.usr_set.mile_display > 2) {
        usr_param.usr_set.mile_display = 0;
    }

    if (usr_param.usr_set.theme > 2) {
        usr_param.usr_set.theme = 0;
    }

    if (usr_param.usr_set.brightness > 5) {
        usr_param.usr_set.brightness = 3;
    }

    if (usr_param.usr_set.language > 1) {
        usr_param.usr_set.language = 1;
    }

    if (usr_param.usr_set.uuid_active_staus > 1) {
        usr_param.usr_set.uuid_active_staus = 0;
    }

    if (usr_param.usr_set.tpms_unit > 2) {
        usr_param.usr_set.tpms_unit = 0;
    }

    if (usr_param.usr_set.tcs_switch > 1) {
        usr_param.usr_set.tcs_switch = 0;
    }

    if (usr_param.usr_set.time_format > 1) {
        usr_param.usr_set.time_format = 1;
    }

    if (usr_param.usr_set.temp_unit > 1) {
        usr_param.usr_set.temp_unit = 0;
    }

    if (usr_param.usr_set.auto_headlight > 1) {
        usr_param.usr_set.auto_headlight = 0;
    }

    if (usr_param.usr_set.phone_type > 1) {
        usr_param.usr_set.phone_type = 0;
    }

    if (usr_param.usr_set.drive_mode > 1) {
        usr_param.usr_set.drive_mode = 0;
    }

    if (usr_param.maintain_info.maintain_mile.maintain_count == 0xffff) {
        usr_param.maintain_info.maintain_mile.maintain_count = 0;
    }

    if (usr_param.maintain_info.maintain_mile.cur_maintain_mileage == 0xffff) {
        usr_param.maintain_info.maintain_mile.cur_maintain_mileage = 0;
    }

    if (usr_param.maintain_info.maintain_mile.last_maintain_total_mileage ==
        0xffffffff) {
        usr_param.maintain_info.maintain_mile.last_maintain_total_mileage = 0;
    }

    if (usr_param.maintain_info.maintain_time.maintain_days == 0xffff) {
        usr_param.maintain_info.maintain_time.maintain_days = 0;
    }

    if (usr_param.maintain_info.maintain_time.is_sync_time > 1) {
        usr_param.maintain_info.maintain_time.is_sync_time = 0;
    }

#if USE_PARAM_PRINTF
    printf_usr_param(&usr_param);
#endif

#ifdef DEFAULT_START_UI_THEME
    usr_param.usr_set.theme = 1;
#endif

    vehicle_set_data(VEH_LICENSE_AUTH_STATUS,
                     (int)usr_param.usr_set.uuid_active_staus);
#if 1
    if (usr_param.usr_set.brightness != 0) {
        // set_backlight_level(usr_param.usr_set.brightness);
    } else {
        mw_log_info("cureent platform not support autobacklight\n");
        // set_backlight_level(3);
        // init_auto_backlight(3);
    }
#endif
}

static void check_start_source(uint8_t start_src) {
    if (start_src) {
        start_by_acc = true;
        mw_log_info("\r\nStart by acc\n");
    } else {
        mw_log_info("\r\nStart by bat\n");
        usr_param.usr_set.mile_format = 0;
        usr_param.usr_set.mile_display = 0, usr_param.usr_set.language = 0;
        usr_param.usr_set.theme = 0;
        usr_param.usr_set.bt_switch = 1;
        usr_param.usr_set.tpms_unit = 0;
        usr_param.usr_set.brightness = 3;
        usr_param.usr_set.auto_headlight = 0;

        ///< 暂不设定时间
        // system_datatime_init();
    }

    check_usr_param();
    is_recovery_usr_param = true;
}

int save_usr_param(void) {
    USR_MUTEX_GUARD();
    if (!is_recovery_usr_param) {
        mw_log_info("Usr param is not get ready!\n");
        return -1;
    }

    if (memcmp(&usr_param_pre, &usr_param, sizeof(usr_param_t)) == 0) {
        mw_log_info("param is same, do not save!\n");
        return 0;
    }

    memcpy(&usr_param_pre, &usr_param, sizeof(usr_param_t));
    set_set_param(usr_param);
    if (!save_set_param_file()) {
        mw_log_error("save usr set param file failed!\n");
        return -1;
    }

    mw_log_info("save usr param success!\n");

    return 0;
}

/**
 * @brief  关机前保存用户参数: 启动源置 1(下次开机识别为 ACC 启动)并落盘.
 *         IGN / NFC / 无线钥匙关机前都应调用, 保证关机源被正确记录.
 *         抽出此函数避免三处关机路径重复同一段 set+save 逻辑.
 * @return 0:保存成功  -1:保存失败(失败时已延时 100ms)
 */
int shutdown_save_usr_param(void) {
    USR_MUTEX_GUARD();
    uint8_t stat_src = 1;
    set_usr_param(USR_PARAM_START_SRC, &stat_src);
    mw_log_info("save usr param..\n");
    if (save_usr_param() != 0) {
        mw_log_info("save usr param failed!\n");
        mw_delay_ms(100);
        return -1;
    }
    mw_log_info("save usr param success!\n");
    return 0;
}

bool get_usr_param(usr_param_handle_e id, void* param) {
    USR_MUTEX_GUARD();
    if (!get_recovery_usr_param()) {
        return false;
    }

    if (!param) {
        mw_log_error("Get usr param id:%d, param pointer null!\n", id);
        return false;
    }

    bool status = true;

    switch (id) {
        case USR_PARAM_MAINTAIN_COUNTS:
            *((uint16_t*)param) =
                usr_param.maintain_info.maintain_mile.maintain_count;
            break;

        case USR_PARAM_CUR_MAINTAIN_MILEAGE:
            *((uint16_t*)param) =
                usr_param.maintain_info.maintain_mile.cur_maintain_mileage;
            break;

        case USR_PARAM_LAST_MAINTAIN_MILEAGE:
            *((uint32_t*)param) = usr_param.maintain_info.maintain_mile
                                      .last_maintain_total_mileage;
            break;

        case USR_PARAM_MAINTAIN_DAYS:
            *((uint16_t*)param) =
                usr_param.maintain_info.maintain_time.maintain_days;
            break;

        case USR_PARAM_MAINTAIN_DATE:
            memcpy(param,
                   &usr_param.maintain_info.maintain_time.last_maintain_date,
                   sizeof(maintain_date_t));
            break;

        case USR_PARAM_MAINTAIN_IS_SYNC_TIME:
            *((uint8_t*)param) =
                usr_param.maintain_info.maintain_time.is_sync_time;
            break;

        case USR_PARAM_MILE_FORMAT:
            *((uint8_t*)param) = usr_param.usr_set.mile_format;
            break;

        case USR_PARAM_MILE_DISPLAY:
            *((uint8_t*)param) = usr_param.usr_set.mile_display;
            break;

        case USR_PARAM_THEME:
            *((uint8_t*)param) = usr_param.usr_set.theme;
            break;

        case USR_PARAM_BRIGHTNESS_LEVEL:
            *((uint8_t*)param) = usr_param.usr_set.brightness;
            break;

        case USR_PARAM_LANGUAGE:
            *((uint8_t*)param) = usr_param.usr_set.language;
            break;

        case USR_PARAM_BT_SWITCH:
            *((uint8_t*)param) = usr_param.usr_set.bt_switch;
            break;

        case USR_PARAM_UUID_REGISTER:
            *((uint8_t*)param) = usr_param.usr_set.uuid_active_staus;
            break;

        case USR_PARAM_METER_START_SRC:
            *((uint8_t*)param) = usr_param.usr_set.start_src;
            break;

        case USR_PARAM_TCS_SWITCH:
            *((uint8_t*)param) = usr_param.usr_set.tcs_switch;
            break;

        case USR_PARAM_TEMP_UNIT:
            *((uint8_t*)param) = usr_param.usr_set.temp_unit;
            break;

        case USR_PARAM_TIME_FORMAT:
            *((uint8_t*)param) = usr_param.usr_set.time_format;
            break;

        case USR_PARAM_TPMS_PRESSURE_UNIT:
            *((uint8_t*)param) = usr_param.usr_set.tpms_unit;
            break;

        case USR_PARAM_AUTO_HEAD_LIGHT:
            *((uint8_t*)param) = usr_param.usr_set.auto_headlight;
            break;

        case USR_PARAM_PHONE_TYPE:
            *((uint8_t*)param) = usr_param.usr_set.phone_type;
            break;

        case USR_PARAM_SYS_LOG:
            *((uint8_t*)param) = usr_param.usr_set.sys_log;
            break;

        case USR_PARAM_DRIVE_MODE:
            *((uint8_t*)param) = usr_param.usr_set.drive_mode;
            break;

        case USR_PARAM_EC_UUID:
            /* param 须指向 >= MAX_UUID_LEN 字节的缓冲区(见 set_param.h)。
             * carlink_uuid 可能无 NUL 终止: 按长度拷贝并强制补 '\0'。 */
            memcpy(param, usr_param.carlink_uuid, MAX_UUID_LEN - 1);
            ((char*)param)[MAX_UUID_LEN - 1] = '\0';
            break;

        case USR_PARAM_RIDE_TIME_A:
            *((uint32_t*)param) = usr_param.ride_info.ride_time_a;
            break;

        case USR_PARAM_RIDE_TIME_B:
            *((uint32_t*)param) = usr_param.ride_info.ride_time_b;
            break;

        case USR_PARAM_LEFT_FRONT_TIRE_INFO:
            memcpy(param, &usr_param.tpms[TPMS_LEFT_FRONT],
                   sizeof(tpms_param_t));
            break;

        case USR_PARAM_RIGHT_FRONT_TIRE_INFO:
            memcpy(param, &usr_param.tpms[TPMS_RIGHT_FRONT],
                   sizeof(tpms_param_t));
            break;

        case USR_PARAM_LEFT_REAR_TIRE_INFO:
            memcpy(param, &usr_param.tpms[TPMS_LEFT_REAR],
                   sizeof(tpms_param_t));
            break;

        case USR_PARAM_RIGHT_REAR_TIRE_INFO:
            memcpy(param, &usr_param.tpms[TPMS_RIGHT_REAR],
                   sizeof(tpms_param_t));
            break;

        case USR_PARAM_MCU_UPDATE_LEN:
            *((uint16_t*)param) = usr_param.mcu_update.update_mcu_len;
            break;

        case USR_PARAM_MCU_UPDATE_TYPE:
            *((uint8_t*)param) = usr_param.mcu_update.update_type;
            break;

        case USR_PARAM_START_SRC:
            *((uint8_t*)param) = usr_param.usr_set.start_src;
            break;

        default:
            status = false;
            break;
    }

    return status;
}

bool set_usr_param(usr_param_handle_e id, void* param) {
    USR_MUTEX_GUARD();
    if (!get_recovery_usr_param()) {
        return false;
    }

    if (!param) {
        mw_log_error("Set usr param id:%d, param pointer null!\n", id);
        return false;
    }

    bool is_save = false;

    switch (id) {
        case USR_PARAM_MAINTAIN_COUNTS:
            if (usr_param.maintain_info.maintain_mile.maintain_count !=
                *((uint16_t*)param)) {
                usr_param.maintain_info.maintain_mile.maintain_count =
                    *((uint16_t*)param);
                is_save = true;
            }
            break;

        case USR_PARAM_CUR_MAINTAIN_MILEAGE:
            if (usr_param.maintain_info.maintain_mile.cur_maintain_mileage !=
                *((uint16_t*)param)) {
                usr_param.maintain_info.maintain_mile.cur_maintain_mileage =
                    *((uint16_t*)param);
                is_save = true;
            }
            break;

        case USR_PARAM_LAST_MAINTAIN_MILEAGE:
            if (usr_param.maintain_info.maintain_mile
                    .last_maintain_total_mileage != *((uint32_t*)param)) {
                usr_param.maintain_info.maintain_mile
                    .last_maintain_total_mileage = *((uint32_t*)param);
                is_save = true;
            }
            break;

        case USR_PARAM_MAINTAIN_DAYS:
            if (usr_param.maintain_info.maintain_time.maintain_days !=
                *((uint16_t*)param)) {
                usr_param.maintain_info.maintain_time.maintain_days =
                    *((uint16_t*)param);
                is_save = true;
            }
            break;

        case USR_PARAM_MAINTAIN_DATE:
            if (memcmp(
                    &usr_param.maintain_info.maintain_time.last_maintain_date,
                    param, sizeof(maintain_date_t)) != 0) {
                memcpy(
                    &usr_param.maintain_info.maintain_time.last_maintain_date,
                    param, sizeof(maintain_date_t));
                is_save = true;
            }
            break;

        case USR_PARAM_MAINTAIN_IS_SYNC_TIME:
            if (usr_param.maintain_info.maintain_time.is_sync_time !=
                *((uint8_t*)param)) {
                usr_param.maintain_info.maintain_time.is_sync_time =
                    *((uint8_t*)param);
                is_save = true;
            }
            break;

        case USR_PARAM_MILE_FORMAT:
            if (usr_param.usr_set.mile_format != *((uint8_t*)param)) {
                usr_param.usr_set.mile_format = *((uint8_t*)param);
                is_save = true;
            }
            break;

        case USR_PARAM_MILE_DISPLAY:
            if (usr_param.usr_set.mile_display != *((uint8_t*)param)) {
                usr_param.usr_set.mile_display = *((uint8_t*)param);
                is_save = true;
            }
            break;

        case USR_PARAM_THEME:
            if (usr_param.usr_set.theme != *((uint8_t*)param)) {
                usr_param.usr_set.theme = *((uint8_t*)param);
#ifdef DEFAULT_START_UI_THEME
                is_save = false;
#else
                is_save = true;
#endif
            }
            break;

        case USR_PARAM_BRIGHTNESS_LEVEL:
            if (usr_param.usr_set.brightness != *((uint8_t*)param)) {
                usr_param.usr_set.brightness = *((uint8_t*)param);
                is_save = true;
            }
            break;

        case USR_PARAM_LANGUAGE:
            if (usr_param.usr_set.language != *((uint8_t*)param)) {
                usr_param.usr_set.language = *((uint8_t*)param);
                is_save = true;
            }
            break;

        case USR_PARAM_BT_SWITCH:
            if (usr_param.usr_set.bt_switch != *((uint8_t*)param)) {
                usr_param.usr_set.bt_switch = *((uint8_t*)param);
                is_save = true;
            }
            break;

        case USR_PARAM_UUID_REGISTER:
            if (usr_param.usr_set.uuid_active_staus != *((uint8_t*)param)) {
                usr_param.usr_set.uuid_active_staus = *((uint8_t*)param);
                is_save = true;
            }
            break;

        case USR_PARAM_METER_START_SRC:
            if (usr_param.usr_set.start_src != *((uint8_t*)param)) {
                usr_param.usr_set.start_src = *((uint8_t*)param);
                is_save = true;
            }
            break;

        case USR_PARAM_TCS_SWITCH:
            if (usr_param.usr_set.tcs_switch != *((uint8_t*)param)) {
                usr_param.usr_set.tcs_switch = *((uint8_t*)param);
                is_save = true;
            }
            break;

        case USR_PARAM_TEMP_UNIT:
            if (usr_param.usr_set.temp_unit != *((uint8_t*)param)) {
                usr_param.usr_set.temp_unit = *((uint8_t*)param);
                is_save = true;
            }
            break;

        case USR_PARAM_TIME_FORMAT:
            if (usr_param.usr_set.time_format != *((uint8_t*)param)) {
                usr_param.usr_set.time_format = *((uint8_t*)param);
                is_save = true;
            }
            break;

        case USR_PARAM_TPMS_PRESSURE_UNIT:
            if (usr_param.usr_set.tpms_unit != *((uint8_t*)param)) {
                usr_param.usr_set.tpms_unit = *((uint8_t*)param);
                is_save = true;
            }
            break;

        case USR_PARAM_AUTO_HEAD_LIGHT:
            if (usr_param.usr_set.auto_headlight != *((uint8_t*)param)) {
                usr_param.usr_set.auto_headlight = *((uint8_t*)param);
                is_save = true;
            }
            break;

        case USR_PARAM_PHONE_TYPE:
            if (usr_param.usr_set.phone_type != *((uint8_t*)param)) {
                usr_param.usr_set.phone_type = *((uint8_t*)param);
                is_save = true;
            }
            break;

        case USR_PARAM_SYS_LOG:
            if (usr_param.usr_set.sys_log != *((uint8_t*)param)) {
                usr_param.usr_set.sys_log = *((uint8_t*)param);
                is_save = true;
            }
            break;

        case USR_PARAM_DRIVE_MODE:
            if (usr_param.usr_set.drive_mode != *((uint8_t*)param)) {
                usr_param.usr_set.drive_mode = *((uint8_t*)param);
                is_save = true;
            }
            break;

        case USR_PARAM_EC_UUID: {
            size_t uuid_len =
                strnlen((const char*)param, sizeof(usr_param.carlink_uuid));
            if (strncmp((const char*)usr_param.carlink_uuid,
                        (const char*)param, uuid_len) != 0) {
                memset(usr_param.carlink_uuid, 0,
                       sizeof(usr_param.carlink_uuid));
                memcpy(usr_param.carlink_uuid, param, uuid_len);
                is_save = true;
            }
            break;
        }

        case USR_PARAM_RIDE_TIME_A:
            is_save = false;
            if (usr_param.ride_info.ride_time_a != *((uint32_t*)param)) {
                usr_param.ride_info.ride_time_a = *((uint32_t*)param);
            }
            break;

        case USR_PARAM_RIDE_TIME_B:
            is_save = false;
            if (usr_param.ride_info.ride_time_b != *((uint32_t*)param)) {
                usr_param.ride_info.ride_time_b = *((uint32_t*)param);
            }
            break;

        case USR_PARAM_LEFT_FRONT_TIRE_INFO:
            is_save = false;
            if (memcmp(&usr_param.tpms[TPMS_LEFT_FRONT], param,
                       sizeof(tpms_param_t)) != 0) {
                memcpy(&usr_param.tpms[TPMS_LEFT_FRONT], param,
                       sizeof(tpms_param_t));
            }
            break;
        case USR_PARAM_RIGHT_FRONT_TIRE_INFO:
            is_save = false;
            if (memcmp(&usr_param.tpms[TPMS_RIGHT_FRONT], param,
                       sizeof(tpms_param_t)) != 0) {
                memcpy(&usr_param.tpms[TPMS_RIGHT_FRONT], param,
                       sizeof(tpms_param_t));
            }
            break;

        case USR_PARAM_LEFT_REAR_TIRE_INFO:
            is_save = false;
            if (memcmp(&usr_param.tpms[TPMS_LEFT_REAR], param,
                       sizeof(tpms_param_t)) != 0) {
                memcpy(&usr_param.tpms[TPMS_LEFT_REAR], param,
                       sizeof(tpms_param_t));
            }
            break;

        case USR_PARAM_RIGHT_REAR_TIRE_INFO:
            is_save = false;
            if (memcmp(&usr_param.tpms[TPMS_RIGHT_REAR], param,
                       sizeof(tpms_param_t)) != 0) {
                memcpy(&usr_param.tpms[TPMS_RIGHT_REAR], param,
                       sizeof(tpms_param_t));
            }
            break;

        case USR_PARAM_MCU_UPDATE_LEN:
            if (usr_param.mcu_update.update_mcu_len != *((uint16_t*)param)) {
                usr_param.mcu_update.update_mcu_len = *((uint16_t*)param);
                is_save = true;
            }
            break;

        case USR_PARAM_MCU_UPDATE_TYPE:
            if (usr_param.mcu_update.update_type != *((uint8_t*)param)) {
                usr_param.mcu_update.update_type = *((uint8_t*)param);
                is_save = true;
            }
            break;

        case USR_PARAM_START_SRC:
            if (usr_param.usr_set.start_src != *((uint8_t*)param)) {
                usr_param.usr_set.start_src = *((uint8_t*)param);
                is_save = true;
            }
            break;

        default:
            is_save = false;
            break;
    }

#ifdef PARAM_WEAR_LEVEL_ENABLE
    if (is_save) {
        is_save = false;
        if (save_usr_param() != 0) {
            return false;
        }
    }
#else
    (void)is_save;
#endif

    return true;
}

int usr_param_init(void) {
    USR_MUTEX_GUARD();
    if (!read_set_param()) {
        mw_log_error("read json config param failed\n");
        return -1;
    }

    usr_param = get_set_param();
    usr_param_pre = usr_param;

    if (usr_param.usr_set.start_src == 1) {
        usr_param.usr_set.start_src = 0;
        check_start_source(1);
        if (save_usr_param() != 0) {
            mw_log_error("save start src param failed!\n");
        }
    } else {
        check_start_source(0);
    }

    return 0;
}

bool get_recovery_usr_param(void) {
    USR_MUTEX_GUARD();
    return is_recovery_usr_param;
}

bool is_acc_start(void) {
    USR_MUTEX_GUARD();
    return start_by_acc;
}

void clean_eeprom_operate(void) {
    USR_MUTEX_GUARD();
    printf("Clear eeprom operate...\r\n");
    maintain_info_t maintence_temp;
    memset(&maintence_temp, 0, sizeof(maintain_info_t));
    memcpy(&usr_param.maintain_info, &maintence_temp, sizeof(maintain_info_t));
    if (save_usr_param() != 0) {
        mw_log_info("Clear eeprom save usr param failed!\r\n");
    }
}
