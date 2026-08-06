#ifndef __CNN_LOG_H
#define __CNN_LOG_H

#include <linux/printk.h>

/* message levels */
#define LOGERROR        0x0001
#define LOGINFO         0x0002
#define LOGTRACE        0x0004
#define LOGDEBUG        0x0008
#define LOGDATA         0x0010
#define LOGSTEER        0x0020
#define LOGSDPWRC       0x0040
#define LOGWAKELOCK     0x0080
#define LOGRXPOLL       0x0100
#define LOGIRQ          0x0200
#define LOGFW           0x0400

#define AIC_LOG_LEVEL_DEFAULT   AIC_LOG_LEVEL_INFO
#define AIC_LOG_LEVEL_INFO      (LOGERROR | LOGINFO)
#define AIC_LOG_LEVEL_DEBUG     (AIC_LOG_LEVEL_INFO | LOGTRACE | LOGDEBUG)
#define AIC_LOG_LEVEL_LOUD      0xffff

extern int aicwf_dbg_level;

static inline char __aicfwdbg_l2c(int level)
{
	switch (level) {
		case LOGERROR:      return 'E';
		case LOGINFO:       return 'I';
		case LOGTRACE:      return 'D';
		case LOGDEBUG:      return 'D';
		case LOGDATA:       return 'L';
		case LOGIRQ:        return 'L';
		case LOGSDPWRC:     return 'L';
		case LOGWAKELOCK:   return 'L';
		case LOGRXPOLL:     return 'L';
		default:            return 'N';
	}
}

#define AICWFDBG(level, fmt, args...) do { \
	if (aicwf_dbg_level & level) { \
		if (level == LOGERROR) { \
			pr_err("[%c] " fmt, __aicfwdbg_l2c(level), ##args); \
		} else { \
			pr_info("[%c] " fmt, __aicfwdbg_l2c(level), ##args); \
		} \
	} \
} while (0)

#define RWNX_DBG(fmt, args...) AICWFDBG(LOGTRACE, fmt, ##args)

#endif // __CNN_LOG_H
