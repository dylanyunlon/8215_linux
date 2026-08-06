#ifndef __CNN_LOG_H
#define __CNN_LOG_H

#include <linux/printk.h>

#ifndef LOG_TAG
#define LOG_TAG "wlan"
#endif

#define LOGX(level, tag, fmt, args...) \
	pr_info("[%s][%c] %s,%d: " fmt, \
			tag, level, __func__, __LINE__, ##args)

#define LOGE(fmt, args...) \
	pr_err("[%s][%c] %s,%d: " fmt, \
			LOG_TAG, 'E', __func__, __LINE__, ##args)

#define LOGW(fmt, args...) \
	pr_warn("[%s][%c] %s,%d: " fmt, \
			LOG_TAG, 'W', __func__, __LINE__, ##args)

#define LOGI(fmt, args...) LOGX('I', LOG_TAG, fmt, ##args) // info
#define LOGD(fmt, args...) LOGX('D', LOG_TAG, fmt, ##args) // debug
#define LOGT(fmt, args...) LOGX('T', LOG_TAG, fmt, ##args) // trace
#define LOGA(fmt, args...) LOGX('A', LOG_TAG, fmt, ##args) // action
#define LOGS(fmt, args...) LOGX('S', LOG_TAG, fmt, ##args) // scan

#endif // __CNN_LOG_H
