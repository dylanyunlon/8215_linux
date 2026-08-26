
#ifndef __DEV_STATE_H__
#define __DEV_STATE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief 开机自检状态
 */
typedef enum {
    CHECK_SELF_STATE_INIT = 0,
    CHECK_SELF_STATE_START,
    CHECK_SELF_STATE_SUCCESS,
} check_self_state_e;

/**
 * @brief 开机动画状态
 */
typedef enum {
    ANIMATION_STATE_IDLE = 0,
    ANIMATION_STATE_RUNNING,
    ANIMATION_STATE_END,
} animation_state_e;

/**
 * @brief 设置参数状态
 */
typedef enum {
    SET_PARAM_STATE_FAILED = 0,
    SET_PARAM_STATE_SUCCESS
} set_param_state_e;

/**
 * @brief 定义系统电源的状态
 */
typedef enum {
    /**
     * @brief 系统关闭状态
     * 系统处于关闭状态，无任何操作执行
     */
    POWER_STATE_OFF = 0,

    /**
     * @brief 系统在下载数据，准备数据
     * 此时数据未准备好，不可使用
     */
    POWER_STATE_DATA_LOADING,

     /**
     * @brief 系统运行状态
     * 数据准备完成，系统运行正常，自检结束，系统运行显示
     */
    POWER_STATE_ON,

     /**
     * @brief 系统处于待机状态
     * 系统处于准备关机模式，停止数据处理操作，关屏，存储设置参数等
     */
    POWER_STATE_STANDBY,

    /**
     * @brief 系统已关机
     * 系统已掉电，不可运行
     */
    POWER_STATE_SHUTDOWN
} power_state_e;

/**
 * @brief 通信口状态，如串口，spi,一般是与MCU进行通信使用
 */
typedef enum {
    /**
     * @brief 通信通道未建立
     */
    COM_PORT_STATE_NONE,

    /**
     * @brief 通信通道连接成功
     */
    COM_PORT_STATE_CONNECTED,

     /**
     * @brief 通信通道超时
     */
    COM_PORT_STATE_TIMEOUT,

     /**
     * @brief 通信通道未连接
     */
    COM_PORT_STATE_DISCONNECT
} com_port_state_e;

/**
 * @brief 状态种类（分发事件用；接入新状态在此追加一项）
 */
typedef enum {
    DEV_STATE_CHECK_SELF = 0,  ///< 开机自检状态，值=check_self_state_e
    DEV_STATE_BOOT_ANIM,       ///< 开机动画状态，值=animation_state_e
    DEV_STATE_SET_PARAM,       ///< 参数状态，值=set_param_state_e
    DEV_STATE_POWER,           ///< 电源状态，值=power_state_e
    DEV_STATE_COM_PORT,        ///< 通信口状态，值=com_port_state_e
} dev_state_type_e;

/**
 * @brief 设备状态变更回调（锁内快照、锁外调用）
 */
typedef void (*dev_state_cb_t)(dev_state_type_e type, uint32_t value);

/**
 * @brief  设置自检状态
 * @param  state 自检状态
 * @return 无
 */
void set_check_self_state(check_self_state_e state);

/**
 * @brief  获取自检状态
 * @param  none
 * @return check_self_state_e 自检状态
 */
check_self_state_e get_check_self_state(void);

/**
 * @brief  获取开机动画状态
 * @param  none
 * @return animation_state_e 开机动画状态
 */
animation_state_e get_boot_animation_status(void);

/**
 * @brief  设置开机动画状态
 * @param  none
 * @return animation_state_e 开机动画状态
 */
void set_boot_animation_status(animation_state_e state);

/**
 * @brief  设置参数状态
 * @param  state 参数状态
 * @return 无
 */
void set_param_state(set_param_state_e state);

/**
 * @brief  获取参数状态
 * @param  none
 * @return set_param_state_e 参数状态
 */
set_param_state_e get_param_state(void);

/**
 * @brief  设置系统电源状态
 * @param  state 电源状态
 * @return 无
 */
void set_power_state(power_state_e state);

/**
 * @brief  获取系统电源状态
 * @param  none
 * @return power_state_e 电源状态
 */
power_state_e get_power_state(void);

/**
 * @brief  设置通信口状态
 * @param  state 通信口状态
 * @return 无
 */
void set_com_port_state(com_port_state_e state);

/**
 * @brief  获取通信口状态
 * @param  none
 * @return com_port_state_e 通信口状态
 */
com_port_state_e get_com_port_state(void);

/**
 * @brief  注册状态变更回调（可注册多个，去重）
 * @param  event_cb 状态变更回调函数
 * @return 0:成功  1:已注册过或已达上限(MW_MAX_CALLBACK_NUM)  -1:参数错误
 */
int set_dev_state_cb(dev_state_cb_t event_cb);

/**
 * @brief  分发状态变更事件到所有已注册回调（锁内快照、锁外调用，
 *         回调内可安全调用 set/remove_dev_state_cb）
 * @param  type  状态种类
 * @param  value 新状态值（按 type 解释为对应枚举）
 * @return none
 */
void send_dev_state_event(dev_state_type_e type, uint32_t value);

/**
 * @brief  移除状态变更回调（前移收缩数组，容量可复用）
 * @param  event_cb 已注册的回调函数
 * @return 0:成功  -1:未找到
 */
int remove_dev_state_cb(dev_state_cb_t event_cb);

#ifdef __cplusplus
}
#endif  //__cplusplus

#endif  // __DEV_STATE_H__