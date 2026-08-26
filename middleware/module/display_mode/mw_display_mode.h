#ifndef __MW_DISPLAY_MODE_H__
#define __MW_DISPLAY_MODE_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "dev_config.h"

#if MW_LIGHT_SENSOR_ENABLE

#define LIGHT_SENSOR_ARR_LEVEL (6)  ///< 光感判定级数(阈值表项数)
#define LIGHT_SENSOR_ARR_SIZE (5)   ///< 采样环形缓冲深度

typedef enum {
    DAY_MODE = 0x00,   ///< 白天
    NIGHT_MODE = 0x01, ///< 夜间
    AUTO_MODE = 0x02   ///< 自动（跟随环境光）
} display_mode_e;

/**
 * @brief  显示模式初始化（幂等）：拉起周期判定线程（50ms 节拍）
 * @return 0:成功 -1:失败
 */
int display_mode_init(void);

/**
 * @brief  休眠前停止判定线程: 停标志 + done 信号量优雅退出
 *         （OSAL 线程为 detached，不可 delete；同 adc_key_sleep 模式）
 * @return none
 */
void display_mode_sleep(void);

#endif

#ifdef __cplusplus
}
#endif
#endif /* __MW_DISPLAY_MODE_H__ */
