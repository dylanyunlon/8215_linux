#ifndef __AICWF_DEBUG_H
#define __AICWF_DEBUG_H

#define RWNX_FN_ENTRY_STR ">>> %s()\n", __func__

#ifdef CONFIG_ATC_AOSP_ENHANCEMENT
#include "../aic8800_bsp/cnn_log.h"
#else

/* message levels */
#define LOGERROR		0x0001
#define LOGINFO			0x0002
#define LOGTRACE		0x0004
#define LOGDEBUG		0x0008
#define LOGDATA			0x0010
#define LOGSTEER		0x0020
#define LOGSDPWRC		0x0040
#define LOGWAKELOCK		0x0080
#define LOGRXPOLL		0x0100
#define LOGIRQ			0x0200
#define LOGFW			0x0400

extern int aicwf_dbg_level;
void rwnx_data_dump(char* tag, void* data, unsigned long len);

#define AICWF_LOG		"AICWFDBG("

#define AICWFDBG(level, args, arg...)	\
do {	\
	if (aicwf_dbg_level & level) {	\
		pr_info(AICWF_LOG#level")\t" args, ##arg); \
	}	\
} while (0)

#define RWNX_DBG(fmt, ...)	\
do {	\
	if (aicwf_dbg_level & LOGTRACE) {	\
		pr_info(AICWF_LOG"LOGTRACE)\t"fmt , ##__VA_ARGS__); 	\
	}	\
} while (0)

#endif // CONFIG_ATC_AOSP_ENHANCEMENT

#endif // __AICWF_DEBUG_H
