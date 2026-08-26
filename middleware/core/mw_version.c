/** 中间件版本信息 -- 值在本 TU 编译（链入 lib_mw.a）时固化 */
#include "mw_version.h"

#if defined(OSAL_OS_LINUX)
#define MW_OS_NAME "linux"
#elif defined(OSAL_OS_RTTTHREAD)
#define MW_OS_NAME "rtthread"
#elif defined(OSAL_OS_FREERTOS)
#define MW_OS_NAME "freertos"
#else
#define MW_OS_NAME "unknown"
#endif

#define MW_BUILD_DATE (__DATE__ " " __TIME__)

static const mw_version_info_t s_mw_version = {
    MW_VERSION_MAJOR,
    MW_VERSION_MINOR,
    MW_VERSION_PATCH,
    MW_VERSION,
    MW_BUILD_ID,
    MW_BUILD_DATE,
    MW_OS_NAME,
};

const mw_version_info_t* mw_get_version_info(void) { return &s_mw_version; }

uint32_t mw_version_num(void) { return MW_VERSION_NUM; }
