#ifndef __CALLBACK_CONFIG_H__
#define __CALLBACK_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define MW_RET_OK 0      ///< 成功
#define MW_RET_ERR -1     ///< 一般错误
#define MW_RET_PARAM_ERR -2  ///< 参数错误
#define MW_RET_MEM_ERR -3    ///< 内存分配失败
#define MW_RET_ERR_BUSY -4       ///< 资源忙
#define MW_RET_ERR_TIMEOUT -5    ///< 超时
#define MW_RET_ERR_NOT_SUPPORT -6///< 不支持

#define MW_MAX_CALLBACK_NUM (10) ///< 最大回调注册数量

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // __CALLBACK_CONFIG_H__