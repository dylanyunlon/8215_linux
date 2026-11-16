
#ifndef __MW_SYS_DATETIME_H__
#define __MW_SYS_DATETIME_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
 
typedef struct {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
} SystemTime_t;

void system_datatime_init(void);

/**
 * @brief  设置系统时间
 * @param  none
 * @return 0:success -1:failed
 */
int set_os_datetime(SystemTime_t datetime);

/**
 * @brief  获取系统时
 * @param  none
 * @return 系统时间
 */
SystemTime_t get_os_datetime(void);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // __MW_SYS_DATETIME_H__