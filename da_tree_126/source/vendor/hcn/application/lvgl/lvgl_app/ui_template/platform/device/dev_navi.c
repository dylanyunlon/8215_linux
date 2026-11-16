/*
 * dev_navi.c -- 简易导航 + 手机互联（device 后端）
 *
 * 提供 UI 使用的同名数据/控制接口：
 *   get_easy_navi_info / get_easy_navi_bt_name        简易导航(BLE)
 *   get_carlink_url / restart_carlink_video / stop_carlink_video   EasyConn 投屏
 *
 * 当前中间件没有导航与投屏数据源：导航返回内部缓存，可由 set_easy_navi_info() 更新；
 * 投屏三个接口是桩，正式接入时转 ec_link。UI_EASY_NAVI_ENABLE 默认 0，但符号必须存在。
 */
#include <stdio.h>
#include <string.h>
#include "mw_easy_navi.h"
#include "ec_link.h"

/* ------------------------------------------------------------ 简易导航 --- */

static easy_navi_data_t s_easy_navi;
static char             s_easy_navi_bt_name[64] = "AC8215-Navi";

const easy_navi_data_t *get_easy_navi_info(void)
{
    return &s_easy_navi;
}

void set_easy_navi_info(const easy_navi_data_t *data)
{
    if (data == NULL) return;
    s_easy_navi = *data;
}

const char *get_easy_navi_bt_name(void)
{
    return s_easy_navi_bt_name;
}

/* --------------------------------------------------------- 手机互联投屏 --- */

static char s_carlink_url[256] = "";

const char *get_carlink_url(void)
{
    return s_carlink_url;
}

void restart_carlink_video(void)
{
    printf("[dev_navi] restart_carlink_video: 占位实现（中间件无投屏模块）\n");
    fflush(stdout);
}

void stop_carlink_video(void)
{
    printf("[dev_navi] stop_carlink_video: 占位实现（中间件无投屏模块）\n");
    fflush(stdout);
}
