/*
 * dev_storage.c —— 里程参数 + 系统时间（device 后端）
 *
 * 对应 platform/pc/mock_vehicle_data.c 里的同名符号，但数据来自真实持久化：
 *   get/set_mile_param, get_recovery_mile_param   ← 本地文件
 *   get/set_os_datetime                           ← 系统时钟
 *
 * 持久化落本地文件：中间件目前没有可用的 EEPROM 驱动（dev_config.h 的
 * MW_CAN_MILE_MSG_ENABLE 默认 0），用文件保证"断电不丢"；
 * 将来接上 EEPROM 只需替换本文件下面的 IO 函数。
 *
 * 数据布局沿用 mile_param.h 的 mile_param_t（48 字节，含 magic + checksum），
 * 也就是设备端真实结构，不做额外包装。
 *
 * 文件路径：环境变量 HCN_APPLICATION_MILE_PARAM_FILE，默认 "./hcn_application_mile_param.dat"。
 */
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "mile_param.h"
#include "mw_sys_datetime.h"

#define MILE_FILE_ENV      "HCN_APPLICATION_MILE_PARAM_FILE"
#define MILE_FILE_DEFAULT  "hcn_application_mile_param.dat"

static mile_param_t s_mile;
static bool         s_mile_loaded = false;
static bool         s_mile_ready  = false;   /* get_recovery_mile_param 的返回值 */

static const char *mile_file_path(void)
{
    const char *p = getenv(MILE_FILE_ENV);
    return (p != NULL && *p != 0) ? p : MILE_FILE_DEFAULT;
}

static uint32_t mile_checksum(const mile_param_t *m)
{
    const uint8_t *p = (const uint8_t *)m;
    uint32_t sum = 0;
    size_t   i;
    for (i = 0; i + sizeof(m->checksum) < sizeof(*m); i++) sum += p[i];
    return sum;
}

static void mile_set_defaults(void)
{
    memset(&s_mile, 0, sizeof(s_mile));
    s_mile.checksum = mile_checksum(&s_mile);
    s_mile_ready    = false;   /* 没读到过 → UI 走"参数未准备好"分支 */
}

static void mile_load(void)
{
    FILE *fp;

    if (s_mile_loaded) return;
    s_mile_loaded = true;

    fp = fopen(mile_file_path(), "rb");
    if (fp == NULL) {
        printf("[dev_storage] mile file not found (%s), use defaults\n", mile_file_path());
        fflush(stdout);
        mile_set_defaults();
        return;
    }

    if (fread(&s_mile, 1, sizeof(s_mile), fp) != sizeof(s_mile)) {
        printf("[dev_storage] mile file truncated, use defaults\n");
        fflush(stdout);
        fclose(fp);
        mile_set_defaults();
        return;
    }
    fclose(fp);

    if (s_mile.checksum != mile_checksum(&s_mile)) {
        printf("[dev_storage] mile checksum mismatch, use defaults\n");
        fflush(stdout);
        mile_set_defaults();
        return;
    }

    s_mile_ready = true;
    printf("[dev_storage] mile loaded: odo=%u m trip=%u m\n", s_mile.odo, s_mile.trip);
    fflush(stdout);
}

static void mile_store(void)
{
    FILE *fp;

    s_mile.checksum = mile_checksum(&s_mile);

    fp = fopen(mile_file_path(), "wb");
    if (fp == NULL) {
        printf("[dev_storage] cannot open %s for write\n", mile_file_path());
        fflush(stdout);
        return;
    }
    fwrite(&s_mile, 1, sizeof(s_mile), fp);
    fclose(fp);
    s_mile_ready = true;
}

/* ---------- 字段表：把 mile_param_handle_e 映射到结构体成员 ---------- */
typedef struct {
    size_t off;
    size_t len;
} mile_field_t;

static const mile_field_t s_mile_fields[] = {
    [MILE_PARAM_ODO]          = { offsetof(mile_param_t, odo),           sizeof(uint32_t) },
    [MILE_PARAM_TRIP]         = { offsetof(mile_param_t, trip),          sizeof(uint32_t) },
    [MILE_PARAM_RIDE_TIME]    = { offsetof(mile_param_t, ride_time),     sizeof(uint32_t) },
    [MILE_PARAM_AVG_ELEC_CONS]= { offsetof(mile_param_t, avg_elec_cons), sizeof(uint16_t) },
    [MILE_PARAM_RANGE]        = { offsetof(mile_param_t, range),         sizeof(uint16_t) },
    [MILE_PARAM_NFC_UID1]     = { offsetof(mile_param_t, nfc_uid1),      sizeof(uint32_t) },
    [MILE_PARAM_NFC_UID2]     = { offsetof(mile_param_t, nfc_uid2),      sizeof(uint32_t) },
    [MILE_PARAM_WIRELESS_KEY1]= { offsetof(mile_param_t, wireless_key1), sizeof(uint32_t) },
    [MILE_PARAM_WIRELESS_KEY2]= { offsetof(mile_param_t, wireless_key2), sizeof(uint32_t) },
};

bool set_mile_param(mile_param_handle_e id, void *param)
{
    const mile_field_t *f;

    if (param == NULL) return false;
    if ((size_t)id >= sizeof(s_mile_fields) / sizeof(s_mile_fields[0])) return false;

    f = &s_mile_fields[id];
    if (f->len == 0) return false;

    mile_load();
    memcpy((uint8_t *)&s_mile + f->off, param, f->len);
    mile_store();          /* 写穿：参数是低频写，先保证"掉电不丢" */
    return true;
}

bool get_mile_param(mile_param_handle_e id, void *param)
{
    const mile_field_t *f;

    if (param == NULL) return false;
    if ((size_t)id >= sizeof(s_mile_fields) / sizeof(s_mile_fields[0])) return false;

    f = &s_mile_fields[id];
    if (f->len == 0) return false;

    mile_load();
    memcpy(param, (const uint8_t *)&s_mile + f->off, f->len);
    return true;
}

bool get_recovery_mile_param(void)
{
    mile_load();
    return s_mile_ready;
}

int mile_param_init(void)
{
    mile_load();
    return 0;
}

/* ------------------------------------------------------------- 系统时间 --- */

SystemTime_t get_os_datetime(void)
{
    SystemTime_t out;
    time_t       now = time(NULL);
    struct tm    lt;

    memset(&out, 0, sizeof(out));
    if (now == (time_t)-1) return out;
    if (localtime_r(&now, &lt) == NULL) return out;

    out.tm_sec  = lt.tm_sec;
    out.tm_min  = lt.tm_min;
    out.tm_hour = lt.tm_hour;
    out.tm_mday = lt.tm_mday;
    out.tm_mon  = lt.tm_mon;
    out.tm_year = lt.tm_year;    /* 与 struct tm 一致：自 1900 起的年数 */
    return out;
}

int set_os_datetime(SystemTime_t datetime)
{
    struct tm lt;
    time_t    t;

    memset(&lt, 0, sizeof(lt));
    lt.tm_sec  = datetime.tm_sec;
    lt.tm_min  = datetime.tm_min;
    lt.tm_hour = datetime.tm_hour;
    lt.tm_mday = datetime.tm_mday;
    lt.tm_mon  = datetime.tm_mon;
    lt.tm_year = datetime.tm_year;
    lt.tm_isdst = -1;

    t = mktime(&lt);
    if (t == (time_t)-1) return -1;

    /* 用 clock_settime 而不是已废弃的 stime()。嵌入式板子通常是 root，能改；
     * 改不动（无权限）也不算失败 —— UI 只在设置页回读，回读走 get_os_datetime。
     */
    {
        struct timespec ts;
        ts.tv_sec  = t;
        ts.tv_nsec = 0;
        (void)clock_settime(CLOCK_REALTIME, &ts);
    }
    return 0;
}
