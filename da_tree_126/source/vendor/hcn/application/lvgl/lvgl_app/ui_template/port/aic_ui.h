/**
 * @file aic_ui.h
 * @brief AiUIBuilder 生成代码的资源路径适配头
 *
 * ui/ 在 pc 与 device 两端共用；port/ 同时进入两端 include 路径，因此本头两端都需要。
 * 资源根在运行时解析（见 aic_ui_assets.c）：HCN_APPLICATION_ASSETS ->
 * <exe目录>/assets -> <cwd>/assets -> 编译期默认值，命中即用。
 * 字体路径由 ui_asset_root() 拼接。
 * 不依赖 ArtInChip SDK / RT-Thread 符号。
 */

#ifndef AIC_UI_H
#define AIC_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include <stdio.h>
#include <stdlib.h>

#ifndef LVGL_DIR
#error "LVGL_DIR 未定义：请在 hcn_application/CMakeLists.txt 里检查 HCN_APPLICATION_ASSETS 相关定义"
#endif
/* LVGL_STORAGE_PATH 已无消费方（字体路径走 ui_asset_root()），故此处不做 #ifndef 自检。 */

/* 板级 lv_conf.h 自检（判定条件：任意按 'A' 注册的 lv_fs 后端均可）：
 *   本 UI 的图片/字体全部走 "<盘符>:<路径>" 字符串（LVGL_IMAGE_PATH / LVGL_STORAGE_PATH），
 *   依赖 lv_fs 里**有一个后端注册了盘符 'A'**。后端是谁不重要：
 *     pc 后端   -> 根 lv_conf.h 的 LV_USE_FS_STDIO 1 / LV_FS_STDIO_LETTER 'A'
 *     device 后端 -> AC8215 板级 lv_conf.h 的 LV_USE_FS_POSIX 1 / LV_FS_POSIX_LETTER 'A'
 *   （lv_fs_posix.c 拼的是 `LV_FS_POSIX_PATH "%s"`，板级 PATH 为空串，所以 A:/usr/... 就是绝对路径）
 *   一个后端都没开的话，编译仍会通过，但运行时整屏空白，所以在这里编译期直接报错。
 *   盘符要改的话是两处一起改：本文件的比较值 + hcn_application/CMakeLists.txt 的 LVGL_DIR（现都是 A）。
 */
#if LV_USE_FS_STDIO
#  define HCN_APPLICATION_FS_LETTER LV_FS_STDIO_LETTER
#elif LV_USE_FS_POSIX
#  define HCN_APPLICATION_FS_LETTER LV_FS_POSIX_LETTER
#elif LV_USE_FS_FATFS
#  define HCN_APPLICATION_FS_LETTER LV_FS_FATFS_LETTER
#elif LV_USE_FS_WIN32
#  define HCN_APPLICATION_FS_LETTER LV_FS_WIN32_LETTER
#else
#  error "板级 lv_conf.h: STDIO/POSIX/FATFS/WIN32 四个 lv_fs 后端全是 0 —— 本 UI 用 A:<path> 加载 483 张图与字体，上板会整屏空白。AC8215 板级请开 LV_USE_FS_POSIX 1 且 LV_FS_POSIX_LETTER 'A'。"
#  define HCN_APPLICATION_FS_LETTER 'A'   /* 保证后续盘符检查可见；实际错误已在上一条 #error 报出 */
#endif

#if (HCN_APPLICATION_FS_LETTER) != 'A'
#  error "板级 lv_conf.h: 已启用的那个 lv_fs 后端盘符不是 'A' —— 改成 'A'，或同步改 hcn_application/CMakeLists.txt 的 LVGL_DIR 与本行"
#endif

/* ---- 资源路径构造（运行时解析）----------------------------------------
 * LVGL_IMAGE_PATH(x) 展开成一次函数调用，实参靠 `#x` **字符串化**传入（与原 CONN 宏的 `x#y` 同一套语义）。
 * 注意 "image/" 前缀由本宏补上：
 * 生成代码传进来的只是 "wallpaper.png" 这种相对于 image/ 的名字，
 * 丢了这一段就会变成 <root>/wallpaper.png —— **每张图都会打不开**。
 *     LVGL_IMAGE_PATH(wallpaper.png)      -> ui_asset_path_site("image/wallpaper.png", __FILE__, __LINE__)
 *     LVGL_IMAGE_PATH(navi/icon_%02d.png) -> ui_asset_path_site("image/navi/icon_%02d.png", __FILE__, __LINE__)
 * 所以实参写的是**裸 token 而非字符串字面量** —— 这是 AiUIBuilder 生成代码的写法，
 * 别画蛇添足去加引号（加了反而会变成 \"\.png\" 这种带引号的路径）。
 * 两种用法都吃：直接当 lv_image_set_src / lv_obj_set_style_bg_image_src 的 src，
 * 或当 lv_snprintf 的格式串（ui/ 里有 4 处这种用法）。
 *
 * 返回的指针**长期有效**：按调用点(__FILE__,__LINE__)缓存一次、永不释放。
 * 原因是 LVGL 的 style 属性**不复制字符串**（src/misc/lv_style.c 的
 * lv_style_set_prop 直接存指针），ui/ 里大量
 * lv_obj_set_style_bg_image_src(..., LVGL_IMAGE_PATH(...), ...) 存的就是这个指针，
 * 它必须活到进程退出（只有 lv_image_set_src 会 lv_strdup 拷一份）。
 *
 * 资源根探测顺序（首次调用时定下）：
 *     1) 环境变量 HCN_APPLICATION_ASSETS     2) <可执行文件目录>/assets
 *     3) <当前工作目录>/assets       4) 编译期默认值 LVGL_DIR
 * 命中判据：<dir>/image/start_animation/000.png 能用 fopen 打开；该动画首帧为模板 UI 独有资源。
 */
#define LVGL_IMAGE_PATH(y) ui_asset_path_site("image/" #y, __FILE__, __LINE__)

/* 返回值终身有效，无需调用方释放。两个函数都在 port/aic_ui_assets.c 里实现。 */
const char *ui_asset_root(void);
const char *ui_asset_path_site(const char *rel, const char *file, int line);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //AIC_UI_H
