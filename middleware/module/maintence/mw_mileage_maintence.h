#ifndef __MW_MILEAGE_MAINTENCE_H__
#define __MW_MILEAGE_MAINTENCE_H__

/**
 * @file mw_mileage_maintence.h
 * @brief 里程保养提醒 -- 轮询总里程计算剩余保养里程/到期提醒
 *
 * 原理：worker 线程周期读 vehicle_param 总里程(VEH_MILEAGE_TOTAL, 单位米)，
 *       折算 km 后与 set_param 保存的"上次保养总里程"求差，剩余量发布到
 *       vehicle_param(VEH_MAINT_REMAIN_MILEAGE)；行驶里程达到保养间隔时置
 *       VEH_MAINT_REMINDER=1，用户确认保养完成后调 mw_maintence_reset()。
 *
 * 间隔取值：set_param 的 cur_maintain_mileage 参数非 0 用之；否则保养次数=0
 *       取首保 MW_MAINT_FIRST_KM，否则取常规 MW_MAINT_NEXT_KM。
 *
 * 依赖：mw_init 之后调用(set_param 需已 usr_param_init)；参数未加载
 *       (get_recovery_usr_param()=false)时只轮询不计算。
 * 内部单位：一律 km(set_param 存储约定)；总里程米->km 四舍五入。
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "dev_config.h"

#if MW_MILEAGE_MAINTENCE_ENABLE

/** 初始化(幂等)：启动里程轮询线程，注册模块自检钩子。0=成功 -1=失败 */
int mw_maintence_init(void);

/** 停止轮询线程(done-sem join)。0=成功 */
int mw_maintence_deinit(void);

/**
 * @brief  用户确认保养完成
 * @note   保养次数+1、记录当前总里程、立即落盘、清除提醒位并重算剩余里程；
 *         总里程无效(<=0)或参数未就绪时不动作
 */
void mw_maintence_reset(void);

/** 查询剩余保养里程(km)；-1=参数未就绪/尚无有效计算 */
int mw_maintence_get_remain_km(void);

#else /* MW_MILEAGE_MAINTENCE_ENABLE=0：转接内联桩 */

static inline int mw_maintence_init(void) { return -1; }
static inline int mw_maintence_deinit(void) { return 0; }
static inline void mw_maintence_reset(void) {}
static inline int mw_maintence_get_remain_km(void) { return -1; }

#endif /* MW_MILEAGE_MAINTENCE_ENABLE */

#ifdef __cplusplus
}
#endif

#endif /* __MW_MILEAGE_MAINTENCE_H__ */
