#ifndef __MW_INIT_H__
#define __MW_INIT_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * hcn 中间件初始化（实现在 mw_init.c，链入 lib_mw.a）
 * @return 0 成功
 */
int mw_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __MW_INIT_H__ */
