#ifndef __MW_VERSION_H__
#define __MW_VERSION_H__

/**
 * @file mw_version.h
 * @brief 中间件版本管理 -- 单一版本源 + 头/库一致性检测
 *
 * 版本源：本头文件三段宏，字符串由宏拼出（勿手写字符串）。
 * 升版规则：不兼容变更 MAJOR，新增特性 MINOR，修复 PATCH。
 * 构建标识：构建系统可用 -DMW_BUILD_ID=<git rev/流水线号> 注入。
 * 一致性：应用侧 mw_version_check() 对比本头宏与库内固化值，
 * 可检出 头新库旧/头旧库新（静态库典型坑）。
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define MW_VERSION_MAJOR 1
#define MW_VERSION_MINOR 1
#define MW_VERSION_PATCH 0

#define MW_STR_(x) #x
#define MW_STR(x) MW_STR_(x)
#define MW_VERSION                                                      \
    MW_STR(MW_VERSION_MAJOR) "." MW_STR(MW_VERSION_MINOR) "." MW_STR(   \
        MW_VERSION_PATCH)

/** 数值版本 (MAJOR<<16)|(MINOR<<8)|PATCH，用于比较 */
#define MW_VERSION_NUM                                                    \
    (((uint32_t)MW_VERSION_MAJOR << 16) | ((uint32_t)MW_VERSION_MINOR << 8) | \
     (uint32_t)MW_VERSION_PATCH)

#ifndef MW_BUILD_ID
#define MW_BUILD_ID "dev"  ///< 构建标识（可由构建系统 -D 注入）
#endif

/** 版本信息（实现见 mw_version.c，值在库编译时固化） */
typedef struct {
    uint16_t major;         ///< 主版本
    uint16_t minor;         ///< 次版本
    uint16_t patch;         ///< 补丁版本
    const char* version;    ///< "1.1.0"
    const char* build_id;   ///< 构建标识
    const char* build_date; ///< 库编译日期时间 __DATE__ __TIME__
    const char* os_name;    ///< OSAL 后端："linux"/"rtthread"/"freertos"
} mw_version_info_t;

/** 取库编译时固化的版本信息（单例常量，无需释放） */
const mw_version_info_t* mw_get_version_info(void);

/** 库编译时固化的数值版本（见 MW_VERSION_NUM） */
uint32_t mw_version_num(void);

/** 头/库一致性检测：0=一致 -1=漂移（应用侧头宏 vs 库内固化值） */
static inline int mw_version_check(void) {
    return (MW_VERSION_NUM == mw_version_num()) ? 0 : -1;
}

#ifdef __cplusplus
}
#endif
#endif /* __MW_VERSION_H__ */
