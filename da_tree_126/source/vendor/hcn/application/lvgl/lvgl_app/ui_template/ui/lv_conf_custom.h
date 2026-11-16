/*
 * lv_conf_custom.h —— AiUIBuilder 生成代码里 #include 的"配置覆盖"文件。
 *
 * 原设备端用它对 lv_conf.h 做 #undef/#define 覆盖（FreeType=1、QRCODE=1、
 * LV_COLOR_DEPTH=16、性能监视器=0）。这些值现在已经在工程根目录的 lv_conf.h 里配好：
 *   LV_USE_FREETYPE 0   （PC/设备端都不编 FreeType）
 *   LV_USE_QRCODE   1
 *   LV_COLOR_FORMAT_DEFAULT LV_COLOR_FORMAT_RGB565   （= 原 16bit）
 *   LV_USE_PERF_MONITOR 0
 * 因此这里保持空实现即可；注意 LVGL 9.6 已弃用 LV_COLOR_DEPTH，不要再定义它。
 */
#ifndef LV_CONF_CUSTOM_H
#define LV_CONF_CUSTOM_H

/* code begin */

/* code end */

#endif  /* LV_CONF_CUSTOM_H */