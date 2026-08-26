/** 看门狗 HAL -- Linux 后端：/dev/watchdog（板级 ops 覆盖优先）
 * 惰性打开；打开后故意不 close（close 会停狗，生命周期与进程一致） */
#include "hal_wdg.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/watchdog.h> /* WDIOC_SETTIMEOUT / WDIOC_KEEPALIVE */

static const hal_wdg_ops_t* s_board_ops = NULL;
static int s_fd = -1;

void hal_wdg_register_board(const hal_wdg_ops_t* ops) { s_board_ops = ops; }

static int board_init(uint32_t timeout_ms) {
    if (s_fd < 0) {
        s_fd = open("/dev/watchdog", O_RDWR);
        if (s_fd < 0) return -1;
    }
    int sec = (int)((timeout_ms + 999) / 1000);
    if (sec < 1) sec = 1;
    if (ioctl(s_fd, WDIOC_SETTIMEOUT, &sec) != 0) {
        /* 部分驱动不支持设超时：沿用驱动默认超时继续用 */
    }
    return 0;
}

int hal_wdg_init(uint32_t timeout_ms) {
    if (s_board_ops && s_board_ops->init)
        return s_board_ops->init(timeout_ms);
    return board_init(timeout_ms);
}

int hal_wdg_feed(void) {
    if (s_board_ops && s_board_ops->feed) return s_board_ops->feed();
    if (s_fd < 0) return -1;
    return ioctl(s_fd, WDIOC_KEEPALIVE, 0);
}
