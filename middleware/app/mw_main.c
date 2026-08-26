/**
 * @file mw_main.c
 * @brief mw_app -- HCN 中间件独立调试进程（make MW_BUILD=exe 产物）
 *
 * 不打包进正式工程：adb push mw_app /tmp && adb shell /tmp/mw_app 运行，
 * 交互式调试中间件各模块：
 *   - mw_init()/usr_param_init() 全链路（含 /data/set_param -> /config 迁移）
 *   - set_param      ：用户参数 get/set/save
 *   - vehicle_param  ：车辆数据注入/读取
 *   - key_module     ：按键事件注入与回调分发（真机 ADC 按键同时生效）
 *   - dev_state      ：自检/开机动画状态
 *
 * 退出：quit / Ctrl+C（SIGINT/SIGTERM，fgets 被打断后优雅退出）
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>

#include "mw_init.h"
#include "mw_version.h"
#include "mw_modules.h"
#include "hal_bsp.h"
#include "mw_log.h"
#include "set_param.h"
#include "usr_utils.h"
#include "vehicle_param.h"
#include "dev_state.h"
#include "key_common.h"
#include "mw_timer.h"
#include "mw_common.h"
#include "mw_watchdog.h"
#include "mw_pthread.h"
#include "mw_display_mode.h"

/* ---------------- 参数表（与 usr_param_handle_e 一一对应） ---------------- */

typedef enum {
    K_U8,    ///< 8 位标量
    K_U16,   ///< 16 位标量
    K_U32,   ///< 32 位标量
    K_STR,   ///< 字符串（uuid）
    K_DATE,  ///< maintain_date_t（只读展示）
    K_TPMS,  ///< tpms_param_t（只读展示）
} param_kind_e;

typedef struct {
    const char* name;       ///< CLI 参数名
    usr_param_handle_e id;  ///< 参数句柄
    param_kind_e kind;      ///< 数据类型（决定 get/set 分发）
    const char* desc;       ///< 说明
} param_desc_t;

static const param_desc_t s_params[] = {
    {"maintain_counts", USR_PARAM_MAINTAIN_COUNTS, K_U16, "保养次数"},
    {"cur_maintain_mileage", USR_PARAM_CUR_MAINTAIN_MILEAGE, K_U16,
     "当前段保养距离(Km)"},
    {"last_maintain_mileage", USR_PARAM_LAST_MAINTAIN_MILEAGE, K_U32,
     "上次保养总里程"},
    {"maintain_days", USR_PARAM_MAINTAIN_DAYS, K_U16, "保养天数(默认365)"},
    {"maintain_date", USR_PARAM_MAINTAIN_DATE, K_DATE, "上次保养日期(只读)"},
    {"maintain_sync", USR_PARAM_MAINTAIN_IS_SYNC_TIME, K_U8,
     "是否同步过互联网时间"},
    {"mile_format", USR_PARAM_MILE_FORMAT, K_U8, "里程格式 0:公制 1:英制"},
    {"mile_display", USR_PARAM_MILE_DISPLAY, K_U8, "0:ODO 1:tripA 2:tripB"},
    {"theme", USR_PARAM_THEME, K_U8, "主题 0:白天 1:黑夜 2:自动"},
    {"brightness", USR_PARAM_BRIGHTNESS_LEVEL, K_U8, "亮度 0:自动 1-5级"},
    {"language", USR_PARAM_LANGUAGE, K_U8, "语言 0:中文 1:英文"},
    {"bt_switch", USR_PARAM_BT_SWITCH, K_U8, "蓝牙开关"},
    {"uuid_active", USR_PARAM_UUID_REGISTER, K_U8, "uuid激活状态"},
    {"meter_start_src", USR_PARAM_METER_START_SRC, K_U8, "启动源 0:bat 1:ign"},
    {"tcs_switch", USR_PARAM_TCS_SWITCH, K_U8, "TCS开关"},
    {"temp_unit", USR_PARAM_TEMP_UNIT, K_U8, "温度单位 0:摄氏 1:华氏"},
    {"time_format", USR_PARAM_TIME_FORMAT, K_U8, "0:12小时 1:24小时"},
    {"tpms_unit", USR_PARAM_TPMS_PRESSURE_UNIT, K_U8, "胎压单位"},
    {"auto_headlight", USR_PARAM_AUTO_HEAD_LIGHT, K_U8, "自动大灯"},
    {"phone_type", USR_PARAM_PHONE_TYPE, K_U8, "手机类型 0:安卓 1:ios"},
    {"sys_log", USR_PARAM_SYS_LOG, K_U8, "系统日志开关"},
    {"drive_mode", USR_PARAM_DRIVE_MODE, K_U8, "0:eco 1:运动"},
    {"ec_uuid", USR_PARAM_EC_UUID, K_STR, "carlink uuid"},
    {"ride_time_a", USR_PARAM_RIDE_TIME_A, K_U32, "骑行时间a(s)"},
    {"ride_time_b", USR_PARAM_RIDE_TIME_B, K_U32, "骑行时间b(s)"},
    {"tpms_lf", USR_PARAM_LEFT_FRONT_TIRE_INFO, K_TPMS, "左前胎压(只读)"},
    {"tpms_rf", USR_PARAM_RIGHT_FRONT_TIRE_INFO, K_TPMS, "右前胎压(只读)"},
    {"tpms_lr", USR_PARAM_LEFT_REAR_TIRE_INFO, K_TPMS, "左后胎压(只读)"},
    {"tpms_rr", USR_PARAM_RIGHT_REAR_TIRE_INFO, K_TPMS, "右后胎压(只读)"},
    {"mcu_update_len", USR_PARAM_MCU_UPDATE_LEN, K_U16, "mcu更新文件长度"},
    {"mcu_update_type", USR_PARAM_MCU_UPDATE_TYPE, K_U8, "0:无 1:usb 2:ota"},
    {"start_src", USR_PARAM_START_SRC, K_U8, "启动源"},
};

#define PARAM_NUM (sizeof(s_params) / sizeof(s_params[0]))

/* ---------------- 信号处理：Ctrl+C / kill 优雅退出 ---------------- */

static volatile sig_atomic_t s_running = 1;

static void on_signal(int sig) {
    (void)sig;
    s_running = 0;
}

/* ---------------- 按键事件回调：打印事件名 ---------------- */

static const char* key_event_name(uint8_t ev) {
    switch ((key_event_e)ev) {
        case MODE_KEY_SHORT_PR: return "MODE_KEY_SHORT";
        case MODE_KEY_LONG_PR: return "MODE_KEY_LONG";
        case SET_KEY_SHORT_PR: return "SET_KEY_SHORT";
        case SET_KEY_LONG_PR: return "SET_KEY_LONG";
        case COM_KEY_SHORT_PR: return "COM_KEY_SHORT";
        case COM_KEY_LONG_PR: return "COM_KEY_LONG";
        case UP_KEY_SHORT_PR: return "UP_KEY_SHORT";
        case UP_KEY_LONG_PR: return "UP_KEY_LONG";
        case DOWN_KEY_SHORT_PR: return "DOWN_KEY_SHORT";
        case DOWN_KEY_LONG_PR: return "DOWN_KEY_LONG";
        case ENTER_KEY_SHORT_PR: return "ENTER_KEY_SHORT";
        case ENTER_KEY_LONG_PR: return "ENTER_KEY_LONG";
        case BACK_KEY_SHORT_PR: return "BACK_KEY_SHORT";
        case BACK_KEY_LONG_PR: return "BACK_KEY_LONG";
        case COM_KEY_SHORT_PR1: return "COM_KEY_SHORT1";
        case COM_KEY_LONG_PR1: return "COM_KEY_LONG1";
        case SET_KEY_SUPER_LONG_PR: return "SET_KEY_SUPER_LONG";
        case UP_KEY_SUPER_LONG_PR: return "UP_KEY_SUPER_LONG";
        default: return "UNKNOWN";
    }
}

static void on_key_event(uint8_t ev) {
    printf("[key] %s (0x%02X)\n", key_event_name(ev), ev);
}

/* ---------------- 参数表操作 ---------------- */

/**
 * @brief  按名字查参数描述
 * @return 找到返回描述指针，未找到返回 NULL
 */
static const param_desc_t* find_param(const char* name) {
    for (unsigned int i = 0; i < PARAM_NUM; i++) {
        if (strcmp(s_params[i].name, name) == 0) {
            return &s_params[i];
        }
    }
    return NULL;
}

static const char* kind_name(param_kind_e kind) {
    switch (kind) {
        case K_U8: return "u8";
        case K_U16: return "u16";
        case K_U32: return "u32";
        case K_STR: return "str";
        case K_DATE: return "date";
        case K_TPMS: return "tpms";
    }
    return "?";
}

/**
 * @brief  读取并打印一个用户参数（按 kind 分发指针类型）
 */
static void print_param(const param_desc_t* p) {
    if (!get_recovery_usr_param()) {
        printf("  %-20s [未就绪:usr_param_init未成功]\n", p->name);
        return;
    }

    uint8_t v8 = 0;
    uint16_t v16 = 0;
    uint32_t v32 = 0;
    char str[MAX_UUID_LEN] = {0};
    maintain_date_t date;
    tpms_param_t tpms;
    memset(&date, 0, sizeof(date));
    memset(&tpms, 0, sizeof(tpms));

    switch (p->kind) {
        case K_U8:
            if (get_usr_param(p->id, &v8)) {
                printf("  %-20s = %u\n", p->name, v8);
            } else {
                printf("  %-20s 读失败\n", p->name);
            }
            break;
        case K_U16:
            if (get_usr_param(p->id, &v16)) {
                printf("  %-20s = %u\n", p->name, v16);
            } else {
                printf("  %-20s 读失败\n", p->name);
            }
            break;
        case K_U32:
            if (get_usr_param(p->id, &v32)) {
                printf("  %-20s = %u\n", p->name, v32);
            } else {
                printf("  %-20s 读失败\n", p->name);
            }
            break;
        case K_STR:
            if (get_usr_param(p->id, str)) {
                printf("  %-20s = %s\n", p->name, str);
            } else {
                printf("  %-20s 读失败\n", p->name);
            }
            break;
        case K_DATE:
            if (get_usr_param(p->id, &date)) {
                printf("  %-20s = %04u-%02u-%02u\n", p->name, date.year,
                       date.mon, date.day);
            } else {
                printf("  %-20s 读失败\n", p->name);
            }
            break;
        case K_TPMS:
            if (get_usr_param(p->id, &tpms)) {
                printf("  %-20s = id:0x%08X p:%u t:%d st:0x%02X\n", p->name,
                       tpms.tpms_id, tpms.tpms_pressure, tpms.tpms_temp,
                       *(uint8_t*)&tpms.tpms_state);
            } else {
                printf("  %-20s 读失败\n", p->name);
            }
            break;
    }
}

/**
 * @brief  写一个用户参数（日期/胎压结构体不支持 CLI 写入）
 * @return 0 成功  -1 失败
 */
static int write_param(const param_desc_t* p, const char* val) {
    if (p->kind == K_DATE || p->kind == K_TPMS) {
        printf("  %s 是结构体，CLI 只读\n", p->name);
        return -1;
    }

    char* end = NULL;
    unsigned long v = strtoul(val, &end, 0);
    if (end == val || *end != '\0') {
        printf("  非法数值: %s\n", val);
        return -1;
    }

    uint8_t v8 = (uint8_t)v;
    uint16_t v16 = (uint16_t)v;
    uint32_t v32 = (uint32_t)v;
    char str[MAX_UUID_LEN] = {0};

    bool ok = false;
    switch (p->kind) {
        case K_U8: ok = set_usr_param(p->id, &v8); break;
        case K_U16: ok = set_usr_param(p->id, &v16); break;
        case K_U32: ok = set_usr_param(p->id, &v32); break;
        case K_STR:
            snprintf(str, sizeof(str), "%s", val);
            ok = set_usr_param(p->id, str);
            break;
        default: break;
    }
    printf("  set %s %s\n", p->name, ok ? "OK" : "FAIL");
    return ok ? 0 : -1;
}

/* 看门狗心跳演示线程：每 500ms kick 一次（真实工程由各关键线程自 kick） */
static int s_wdg_demo_id = -1;
static void* wdg_demo_thread(void* arg) {
    (void)arg;
    while (s_running) {
        if (s_wdg_demo_id >= 0) mw_watchdog_kick(s_wdg_demo_id);
        osal_delay_ms(500);
    }
    return NULL;
}

/* ---------------- 命令实现 ---------------- */

static void cmd_help(void) {
    printf(
        "命令:\n"
        "  get <name>          读用户参数\n"
        "  set <name> <val>    写用户参数\n"
        "  save                保存用户参数落盘\n"
        "  dump                打印全部用户参数\n"
        "  params              列出参数名表(name 类型 说明)\n"
        "  veh <id> <val>      注入车辆数据(如 veh 6 60 -> 当前车速)\n"
        "  vehget <id>         读车辆数据(6=车速 24=总里程)\n"
        "  vehdump             打印全部车辆数据\n"
        "  key <event>         注入按键事件(如 key 1 -> MODE短按)\n"
        "  state               打印设备状态(自检/动画/ACC/参数就绪)\n"
        "  devcb               开/关注册状态变更分发演示回调\n"
        "  selfstate <v>       设置自检状态(0/1/2)\n"
        "  anim <v>            设置开机动画状态(0/1/2)\n"
        "  paramstate <v>      设置参数状态(0/1)\n"
        "  power <v>           设置电源状态(0~4)\n"
        "  comport <v>         设置通信口状态(0~3)\n"
        "  ver                 打印版本信息\n"
        "  modules             打印模块清单(哪些编入库, 含健康度)\n"
        "  wdg                 打印线程心跳健康表\n"
        "  quit                退出\n");
}

static void cmd_state(void) {
    printf("  check_self  = %d\n", (int)get_check_self_state());
    printf("  animation   = %d\n", (int)get_boot_animation_status());
    printf("  param_state = %d\n", (int)get_param_state());
    printf("  power       = %d\n", (int)get_power_state());
    printf("  com_port    = %d\n", (int)get_com_port_state());
    printf("  acc_start   = %d\n", is_acc_start() ? 1 : 0);
    printf("  param_ready = %d\n", get_recovery_usr_param() ? 1 : 0);
}

static void demo_dev_state_cb(dev_state_type_e type, uint32_t value) {
    printf("  [devcb] type=%d val=%lu\n", (int)type, (unsigned long)value);
}

static void cmd_devcb(void) {
    static int on = 0;
    if (on) {
        remove_dev_state_cb(demo_dev_state_cb);
        printf("  devcb off\n");
        on = 0;
    } else {
        int r = set_dev_state_cb(demo_dev_state_cb);
        printf("  devcb %s (r=%d)\n", r == 0 ? "on" : "busy", r);
        on = (r == 0);
    }
}

static void cmd_vehdump(void) {
    for (int i = 0; i < (int)VEH_DATA_END; i++) {
        printf("[%3d]%9d  ", i, vehicle_get_data((veh_data_e)i));
        if ((i % 4) == 3) {
            printf("\n");
        }
    }
    printf("\n");
}

/**
 * @brief  车辆数据 id 合法性检查（0 <= id < VEH_DATA_END）
 * @return true 合法
 */
static bool veh_id_ok(unsigned long id) {
    if (id >= (unsigned long)VEH_DATA_END) {
        printf("  id 超范围 [0,%d]\n", (int)VEH_DATA_END - 1);
        return false;
    }
    return true;
}

static void mw_timer_period(void *arg) {
    //mw_log_info("mw timer running....\n");
}

static int mw_timer_test(void) {
    static mw_timer_t* s_mw_timer_test = NULL; 
    s_mw_timer_test = mw_timer_create(mw_timer_period, NULL, true);
    if (s_mw_timer_test) {
        mw_timer_start(s_mw_timer_test, 5000);
        return 0;
    } 
    return -1;
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

#ifdef _WIN32
    signal(SIGINT, on_signal);
#else
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
#endif

    set_build_date_time();

    /* 演示板级注册：真实工程由 BSP 在更早阶段调用（见 hal_bsp_example.c） */
    static const hal_bsp_desc_t s_demo_bsp = {
        .board_name = "mw_app-demo",
    };
    hal_bsp_register(&s_demo_bsp);

    if (mw_init() != 0) {
        printf("[mw_app] mw_init failed\n");
        return 1;
    }
    
    if (usr_param_init() != 0) {
        printf("[mw_app] usr_param_init failed(参数未就绪,读 /data/set_param ?)\n");
    } else {
        printf("[mw_app] usr_param_init OK\n");
    }

    if (set_key_event_cb(on_key_event) != 0) {
        printf("[mw_app] set_key_event_cb failed\n");
    }
    
    if (mw_timer_test() != 0) {
        mw_log_error("mw timer start failed!\n");
    }

    /* 看门狗+线程心跳：init 即挂注册表 self_check，demo 线程演示 kick */
    if (mw_watchdog_init() == 0) {
        s_wdg_demo_id = mw_watchdog_register("demo", 2000);
        mw_pthread_create("wdg_demo", NULL, wdg_demo_thread);
    }

    printf("[mw_app] input 'help' for commands\n");
    cmd_help();

    char line[256];
    char cmd[32];
    char a1[64];
    char a2[64];
    while (s_running) {
        printf("mw> ");
        fflush(stdout);
        if (fgets(line, sizeof(line), stdin) == NULL) {
            if (!s_running || feof(stdin)) {
                break;
            }
            clearerr(stdin);
            continue;
        }
        cmd[0] = a1[0] = a2[0] = '\0';
        int n = sscanf(line, "%31s %63s %63s", cmd, a1, a2);
        if (n <= 0 || cmd[0] == '\0' || cmd[0] == '#') {
            continue;
        }

        if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0) {
            break;
        } else if (strcmp(cmd, "help") == 0) {
            cmd_help();
        } else if (strcmp(cmd, "get") == 0 && n >= 2) {
            const param_desc_t* p = find_param(a1);
            if (p) {
                print_param(p);
            } else {
                printf("  未知参数名(用 params 查看参数名表)\n");
            }
        } else if (strcmp(cmd, "set") == 0 && n >= 3) {
            const param_desc_t* p = find_param(a1);
            if (p) {
                write_param(p, a2);
            } else {
                printf("  未知参数名(用 params 查看参数名表)\n");
            }
        } else if (strcmp(cmd, "save") == 0) {
            printf("  save %s\n", save_usr_param() == 0 ? "OK" : "FAIL");
        } else if (strcmp(cmd, "dump") == 0) {
            for (unsigned int i = 0; i < PARAM_NUM; i++) {
                print_param(&s_params[i]);
            }
        } else if (strcmp(cmd, "params") == 0) {
            for (unsigned int i = 0; i < PARAM_NUM; i++) {
                printf("  %-20s %-4s %s\n", s_params[i].name,
                       kind_name(s_params[i].kind), s_params[i].desc);
            }
        } else if (strcmp(cmd, "veh") == 0 && n >= 3) {
            unsigned long id = strtoul(a1, NULL, 0);
            long val = strtol(a2, NULL, 0);
            if (veh_id_ok(id)) {
                vehicle_set_data((veh_data_e)id, (int32_t)val);
                printf("  veh[%lu] = %ld\n", id, val);
            }
        } else if (strcmp(cmd, "vehget") == 0 && n >= 2) {
            unsigned long id = strtoul(a1, NULL, 0);
            if (veh_id_ok(id)) {
                printf("  veh[%lu] = %d\n", id,
                       vehicle_get_data((veh_data_e)id));
            }
        } else if (strcmp(cmd, "vehdump") == 0) {
            cmd_vehdump();
        } else if (strcmp(cmd, "key") == 0 && n >= 2) {
            send_key_event((uint8_t)strtoul(a1, NULL, 0));
        } else if (strcmp(cmd, "state") == 0) {
            cmd_state();
        } else if (strcmp(cmd, "devcb") == 0) {
            cmd_devcb();
        } else if (strcmp(cmd, "selfstate") == 0 && n >= 2) {
            set_check_self_state((check_self_state_e)strtoul(a1, NULL, 0));
            printf("  check_self = %d\n", (int)get_check_self_state());
        } else if (strcmp(cmd, "anim") == 0 && n >= 2) {
            set_boot_animation_status(
                (animation_state_e)strtoul(a1, NULL, 0));
            printf("  animation = %d\n", (int)get_boot_animation_status());
        } else if (strcmp(cmd, "paramstate") == 0 && n >= 2) {
            set_param_state((set_param_state_e)strtoul(a1, NULL, 0));
            printf("  param_state = %d\n", (int)get_param_state());
        } else if (strcmp(cmd, "power") == 0 && n >= 2) {
            set_power_state((power_state_e)strtoul(a1, NULL, 0));
            printf("  power = %d\n", (int)get_power_state());
        } else if (strcmp(cmd, "comport") == 0 && n >= 2) {
            set_com_port_state((com_port_state_e)strtoul(a1, NULL, 0));
            printf("  com_port = %d\n", (int)get_com_port_state());
        } else if (strcmp(cmd, "ver") == 0) {
            const mw_version_info_t* v = mw_get_version_info();
            printf("  mw version : %s\n", v->version);
            printf("  build id   : %s\n", v->build_id);
            printf("  lib build  : %s\n", v->build_date);
            printf("  os backend : %s\n", v->os_name);
            printf("  board      : %s\n", hal_bsp_board_name());
            printf("  app build  : %s\n", get_build_date_time());
            printf("  hdr/lib    : %s\n",
                   mw_version_check() == 0 ? "match" : "MISMATCH");
        } else if (strcmp(cmd, "modules") == 0) {
            mw_module_dump();
        } else if (strcmp(cmd, "wdg") == 0) {
            mw_watchdog_dump();
        } else {
            printf("  未知命令(输入 help)\n");
        }
    }

    printf("[mw_app] exit\n");
    return 0;
}
