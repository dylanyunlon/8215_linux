/*
 * dev_pending.c —— NFC + 无线钥匙（device 后端，**占位实现**）
 *
 * 对应 platform/pc/mock_sdk.c 里的这两组：
 *   NFC 7 个：nfc_learn / nfc_stop_cur / nfc_delete / nfc_manage_get_card_valid /
 *             nfc_manage_get_nfc_learn_status / nfc_manager_get_card_count /
 *             nfc_manager_get_card_uid
 *   无线钥匙 6 个：get_wireless_key_state / get_wireless_key_count /
 *             get_wireless_key_addr / start_key_learn / stop_key_learn / delete_key
 *
 * 这两个模块中间件未提供实现（NFC 走 SPI/I2C + 中断，钥匙走 433MHz EV1527 中断），
 * 本文件只保留接口与持久化语义。
 *
 * 已学到的用法：这两个模块的"持久化"部分其实落在里程参数结构里 ——
 * mile_param_t 的 nfc_uid1/nfc_uid2/wireless_key1/wireless_key2 四个字段。
 * 所以这里的桩**不是**纯空转：卡号/钥匙槽位会真的读写 dev_storage.c 的
 * 那份文件，重启不丢。等 NFC/433 驱动到位，只需要把"扫卡/收帧"接上，
 * 存储路径不用动。
 */
#include <stdio.h>
#include <string.h>
#include "nfc_manage.h"
#include "ev1527.h"
#include "mile_param.h"

#define PENDING_MAX_KEYS   (2)   /* 与 mile_param_t 的槽位数一致 */

static uint8_t s_nfc_learn_status = 0;
static uint8_t s_key_learn_active = 0;

static void pending_log(const char *fn)
{
    printf("[dev_pending] %s: 占位实现（NFC/433 模块未接入）\n", fn);
    fflush(stdout);
}

/* ------------------------------------------------------------------ NFC --- */

rt_err_t nfc_manager_init(void)
{
    pending_log("nfc_manager_init");
    return RT_EOK;
}

rt_err_t nfc_manager_send_event(nfc_event_t event, rt_uint8_t param)
{
    (void)event; (void)param;
    pending_log("nfc_manager_send_event");
    return RT_EOK;
}

rt_err_t nfc_manager_enter_lpcd_sync(rt_int32_t timeout_ms)
{
    (void)timeout_ms;
    pending_log("nfc_manager_enter_lpcd_sync");
    return RT_EOK;
}

nfc_state_t nfc_manager_get_state(void)
{
    nfc_state_t st;
    memset(&st, 0, sizeof(st));
    return st;
}

rt_uint8_t nfc_manager_get_card_count(void)
{
    uint32_t uid1 = 0, uid2 = 0;

    if (!get_mile_param(MILE_PARAM_NFC_UID1, &uid1)) return 0;
    if (!get_mile_param(MILE_PARAM_NFC_UID2, &uid2)) return 0;
    return (rt_uint8_t)((uid1 ? 1 : 0) + (uid2 ? 1 : 0));
}

int nfc_manage_get_card_valid(uint8_t index)
{
    uint32_t uid = 0;
    mile_param_handle_e id = (index == 0) ? MILE_PARAM_NFC_UID1 : MILE_PARAM_NFC_UID2;

    if (index >= PENDING_MAX_KEYS) return 0;
    if (!get_mile_param(id, &uid)) return 0;
    return uid != 0;
}

rt_err_t nfc_manager_get_card_uid(rt_uint8_t index, rt_uint8_t *uid, rt_uint8_t *len)
{
    uint32_t v = 0;
    mile_param_handle_e id = (index == 0) ? MILE_PARAM_NFC_UID1 : MILE_PARAM_NFC_UID2;

    if (uid == NULL || len == NULL || index >= PENDING_MAX_KEYS) return -RT_ERROR;
    if (!get_mile_param(id, &v)) return -RT_ERROR;

    memcpy(uid, &v, sizeof(v));
    *len = (rt_uint8_t)sizeof(v);
    return RT_EOK;
}

rt_uint8_t nfc_manager_get_power_state(void)
{
    return 0;
}

uint8_t nfc_manage_get_nfc_learn_status(void)
{
    return s_nfc_learn_status;
}

void nfc_learn(void)
{
    pending_log("nfc_learn");
    s_nfc_learn_status = 1;   /* 让 UI 的"学习中"状态能显示出来 */
}

void nfc_stop_cur(void)
{
    pending_log("nfc_stop_cur");
    s_nfc_learn_status = 0;
}

void nfc_delete(uint8_t index)
{
    uint32_t zero = 0;
    if (index >= PENDING_MAX_KEYS) return;
    set_mile_param((index == 0) ? MILE_PARAM_NFC_UID1 : MILE_PARAM_NFC_UID2, &zero);
}

int nfc_lpcd(void)
{
    pending_log("nfc_lpcd");
    return 0;
}

/* ------------------------------------------------------ 433 无线钥匙 --- */

uint8_t get_wireless_key_state(void)
{
    return s_key_learn_active;
}

uint8_t get_wireless_key_count(void)
{
    uint32_t k1 = 0, k2 = 0;

    if (!get_mile_param(MILE_PARAM_WIRELESS_KEY1, &k1)) return 0;
    if (!get_mile_param(MILE_PARAM_WIRELESS_KEY2, &k2)) return 0;
    return (uint8_t)((k1 ? 1 : 0) + (k2 ? 1 : 0));
}

int get_wireless_key_addr(uint8_t index, uint32_t *addr)
{
    if (addr == NULL || index >= PENDING_MAX_KEYS) return -1;
    if (!get_mile_param((index == 0) ? MILE_PARAM_WIRELESS_KEY1 : MILE_PARAM_WIRELESS_KEY2, addr)) return -1;
    return 0;
}

void start_key_learn(void)
{
    pending_log("start_key_learn");
    s_key_learn_active = 1;
}

void stop_key_learn(void)
{
    pending_log("stop_key_learn");
    s_key_learn_active = 0;
}

void delete_key(uint32_t addr)
{
    uint32_t k1 = 0, k2 = 0;
    uint32_t zero = 0;

    if (addr == 0) return;
    if (get_mile_param(MILE_PARAM_WIRELESS_KEY1, &k1) && k1 == addr) {
        set_mile_param(MILE_PARAM_WIRELESS_KEY1, &zero);
    }
    if (get_mile_param(MILE_PARAM_WIRELESS_KEY2, &k2) && k2 == addr) {
        set_mile_param(MILE_PARAM_WIRELESS_KEY2, &zero);
    }
}

/* 433 IRQ 总开关：真实实现要在关机路径关中断防误唤醒，当前无中断可关 */
void ev1527_irq_enable(bool enable)
{
    (void)enable;
}

int ev1527_init(void)
{
    pending_log("ev1527_init");
    return 0;
}

rt_err_t ev1527_recv(ev1527_frame_t *frame, rt_int32_t timeout)
{
    (void)frame; (void)timeout;
    return -RT_ETIMEOUT;   /* 没有接收源：一直超时，不假装收到按键 */
}

void start_wifi_ap(void)
{
    pending_log("start_wifi_ap");
}
