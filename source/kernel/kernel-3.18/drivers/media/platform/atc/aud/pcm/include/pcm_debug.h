#ifndef _PCM_DEBUG_H_
#define _PCM_DEBUG_H_

extern u32 g_PcmLogLevel;

#define PCM_TAG "[APCM]"

#define LOG_FATAL   0
#define LOG_ERROR   1
#define LOG_WARNING 2
#define LOG_INFO    8
#define LOG_DEBUG   9

#define PCM_LOG(ftag, level, sFmt, ...)              \
    do {                                            \
        if (level == LOG_FATAL) {                   \
            pr_err(PCM_TAG "["ftag"]" "%s:%d: " sFmt, FILE_ONLY, __LINE__, ##__VA_ARGS__);    \
        } else if (level == LOG_ERROR) {            \
            pr_err(PCM_TAG "["ftag"]" "%s:%d: " sFmt, FILE_ONLY, __LINE__, ##__VA_ARGS__);    \
        } else if (level == LOG_WARNING) {          \
            pr_warn(PCM_TAG "["ftag"]" sFmt, ##__VA_ARGS__);   \
        } else if (level == LOG_INFO) {             \
            pr_info(PCM_TAG "["ftag"]" sFmt, ##__VA_ARGS__);   \
        } else if (level == LOG_DEBUG) {            \
            pr_debug(PCM_TAG "["ftag"]" sFmt, ##__VA_ARGS__);  \
        }                                           \
    } while(0)

#define PCM_DEBUG(ftag, sFmt, ...)               \
    PCM_LOG(ftag, LOG_DEBUG, sFmt, ##__VA_ARGS__)
#define PCM_INFO(ftag, sFmt, ...)                    \
    PCM_LOG(ftag, LOG_INFO, sFmt, ##__VA_ARGS__)
#define PCM_WARN(ftag, sFmt, ...)                \
    PCM_LOG(ftag, LOG_WARNING, sFmt, ##__VA_ARGS__)
#define PCM_ERROR(ftag, sFmt, ...)               \
    PCM_LOG(ftag, LOG_ERROR, sFmt, ##__VA_ARGS__)
#define PCM_FATAL(ftag, sFmt, ...)               \
    PCM_LOG(ftag, LOG_FATAL, sFmt, ##__VA_ARGS__)

#endif
