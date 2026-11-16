/**
 * NFC Manager - Card Learning & Power Control Application Layer
 *
 * State machine: IDLE → LEARNING → NORMAL → LPCD → (IRQ wake) → NORMAL
 */

#ifndef NFC_MANAGER_H
#define NFC_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <rtthread.h>
#include <stdint.h>
#include "dev_config.h"

#if NFC_ENABLE
/* ======================== Configuration ======================== */

#define NFC_MAX_CARDS          2      /* max learned cards */
#define NFC_MAX_UID_LEN        10     /* max UID length (triple cascade) */
#define NFC_LEARN_TIMEOUT_MS   60000  /* learning timeout 60s */
#define NFC_SWIPE_DEBOUNCE_MS  2000  /* swipe debounce 3s */
#define NFC_IDLE_TIMEOUT_MS    60000  /* enter LPCD after 60s idle */
#define NFC_POLL_INTERVAL_MS   100    /* card poll interval in NORMAL mode */
#define NFC_THREAD_STACK_SIZE  3072
#define NFC_THREAD_PRIORITY    20

/* ======================== Data Types ======================== */

typedef enum {
    NFC_STATE_IDLE     = 0,  /* standby, no card detection */
    NFC_STATE_LEARNING = 1,  /* waiting for card to learn */
    NFC_STATE_NORMAL   = 2,  /* normal card detection + power control */
    NFC_STATE_LPCD     = 3,  /* low-power card detection, MCU sleeping */
} nfc_state_t;

typedef enum {
    NFC_EVENT_NONE       = 0,
    NFC_EVENT_START_LEARN = 1,  /* UI: start learning */
    NFC_EVENT_STOP_LEARN  = 2,  /* UI: stop learning / timeout */
    NFC_EVENT_DELETE_CARD = 3,  /* UI: delete card */
    NFC_EVENT_DELETE_ALL  = 4,  /* UI: delete card by index */
    NFC_EVENT_WAKEUP      = 5,  /* LPCD IRQ wakeup */
    NFC_EVENT_SLEEP       = 6,  /* request NFC LPCD; param: 0=echo PM_EVENT_NFC_OFF, 1=silent */
} nfc_event_t;

/* Event message with optional parameter */
typedef struct {
    nfc_event_t event;
    rt_uint8_t  param;           /* extra param: card index for DELETE_CARD */
} nfc_event_msg_t;
typedef struct {
    rt_uint8_t uid[NFC_MAX_UID_LEN]; /* card UID */
    rt_uint8_t uid_len;              /* UID length in bytes */
    rt_uint8_t valid;                /* 0xFF = valid */
} card_record_t;

typedef struct {
    nfc_state_t   state;
    card_record_t cards[NFC_MAX_CARDS];
    rt_uint8_t    card_count;
    rt_uint8_t    card_learn_status;   /*0:not learned 1:learning  2:learned 3:learn timeout 4:nfc alredy learned*/
    rt_uint32_t   last_swipe_tick;    /* tick of last valid swipe */
    rt_uint8_t    last_uid[NFC_MAX_UID_LEN];
    rt_uint8_t    last_uid_len;
    rt_uint8_t    power_on;           /* DEPRECATED: 已废弃，电源状态以 rt_pm_get_sleep_mode() 为准 */
    struct rt_messagequeue event_mq;  /* event message queue (承载完整结构体) */
    rt_uint8_t    event_mq_pool[sizeof(nfc_event_msg_t) * 8];  /* 8 条消息容量 */
} nfc_manager_t;

/* ======================== API ======================== */

/**
 * Initialize NFC manager (creates thread, loads cards from storage)
 * @return RT_EOK on success
 */
rt_err_t nfc_manager_init(void);

/**
 * Send event to NFC manager (thread-safe)
 * @param event  event type
 * @param param  extra parameter (e.g. card index for NFC_EVENT_DELETE_CARD)
 * @return RT_EOK on success
 */
rt_err_t nfc_manager_send_event(nfc_event_t event, rt_uint8_t param);

/**
 * 同步进入 LPCD: 派 NFC_EVENT_SLEEP(silent), 等 nfc_thread 配完后返回。
 * 用在必须保证 LPCD 就绪的场景 (主要是 IGN_OFF 路径, SOC 进低功耗前
 * 必须配完, 否则慢时钟下 I2C 操作会失败)。
 *
 * @param timeout_ms  0=不等; <0=永久等; >0=最多等 N ms (nfc_enter_lpcd
 *                    内部最多 3s 等移卡 + 配置时间, 建议 5000ms)
 * @return RT_EOK 成功; -RT_ETIMEOUT 超时; 其它错误码
 */
rt_err_t nfc_manager_enter_lpcd_sync(rt_int32_t timeout_ms);

/**
 * Get current NFC state (thread-safe)
 * @return current state
 */
nfc_state_t nfc_manager_get_state(void);

/**
 * Get learned card count
 * @return number of learned cards
 */
rt_uint8_t nfc_manager_get_card_count(void);

/**
 * @brief judge nfc cardf is valid
 * @param index nfc card index, MAX: NFC_MAX_CARDS -1
 * @return 0:valid -1:invalid
 */
int nfc_manage_get_card_valid(uint8_t index);

/**
 * Get card UID by index
 * @param index  card index (0 ~ card_count-1)
 * @param uid    output buffer (NFC_MAX_UID_LEN bytes)
 * @param len    output: UID length
 * @return RT_EOK on success
 */
rt_err_t nfc_manager_get_card_uid(rt_uint8_t index, rt_uint8_t *uid, rt_uint8_t *len);

/**
 * Get current power state
 * @return 1=on, 0=off
 */
rt_uint8_t nfc_manager_get_power_state(void);


/**
 * @brief nfc card leatning status
 * @param none
 * @return 0:not learn  1:learning 2:learned 3:learn timeout 4:nfc alredy learned
 */
uint8_t nfc_manage_get_nfc_learn_status(void);

/**
 * @brief learn nfc card
 * @param none
 * @return none
 */
void nfc_learn(void);

/**
 * @brief stop cur nfc card
 * @param none
 * @return none
 */
void nfc_stop_cur(void);

/**
 * @brief delete a nfc card
 * @param none
 * @return none
 */
void nfc_delete(uint8_t index);

int nfc_lpcd(void);

#else  /* !NFC_ENABLE */

/* NFC 关闭时的桩接口: 仅声明 UI 层 (nfc_view.c) 等未做 #if NFC_ENABLE 保护的
 * 调用点所需的函数, 使其在 NFC_ENABLE=0 时仍能编译链接通过.
 * 实现见 nfc_manage.c 的 #else 分支 (no-op / 安全默认值).
 * 注: 这里只声明不依赖 NFC 专有类型 (nfc_state_t/nfc_event_t 等) 的接口,
 *     那些类型相关接口的调用方 (mw_init/app_key_demo/ign_check) 已自带
 *     #if NFC_ENABLE 保护, 关闭时不会引用, 故无需桩. */
rt_uint8_t nfc_manager_get_card_count(void);
int nfc_manage_get_card_valid(uint8_t index);
rt_err_t nfc_manager_get_card_uid(rt_uint8_t index, rt_uint8_t *uid, rt_uint8_t *len);
uint8_t nfc_manage_get_nfc_learn_status(void);
void nfc_learn(void);
void nfc_stop_cur(void);
void nfc_delete(uint8_t index);

#endif  /* NFC_ENABLE */

#ifdef __cplusplus
}
#endif

#endif /* NFC_MANAGER_H */
