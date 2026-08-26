/** 极简 I2C HAL -- Linux i2c-dev 后端（板级回调优先）
 *
 * 默认走标准 /dev/i2c-N + I2C_RDWR ioctl：msg 数组一次提交，
 * msg 间 repeated-start 由内核保证。注意当前 ac83xx defconfig
 * 未启用 I2C（无 /dev/i2c-N），此类板须 BSP 桥接并注册板级操作表。
 */
#include "hal_i2c.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>    /* struct i2c_msg + I2C_M_RD 定义在此 */
#include <linux/i2c-dev.h>

struct hal_i2c_bus {
    char name[16];
    int idx; /* 总线号（/dev/i2c-idx） */
    int fd;  /* -1=未打开（懒打开/出错自愈后重开） */
};

static const hal_i2c_ops_t* s_board_ops = NULL;

void hal_i2c_register_board(const hal_i2c_ops_t* ops) { s_board_ops = ops; }

const char* hal_i2c_bus_name(const hal_i2c_bus_t* bus) {
    return bus ? bus->name : NULL;
}

/** 解析总线号：接受 "i2c0"/"i2c-1"/"1"/"/dev/i2c-2"；失败 -1 */
static int parse_bus_idx(const char* name) {
    if (!name || !name[0]) return -1;
    const char* p = name;
    if (p[0] == '/') {
        p = strrchr(p, '-');
        if (p) p++;
    } else {
        if (strncmp(p, "i2c", 3) == 0) p += 3;
        if (*p == '-') p++;
    }
    if (*p < '0' || *p > '9') return -1;
    return (int)strtol(p, NULL, 10);
}

hal_i2c_bus_t* hal_i2c_open(const char* name) {
    int idx = parse_bus_idx(name);
    if (idx < 0) return NULL;
    struct hal_i2c_bus* b = (struct hal_i2c_bus*)calloc(1, sizeof(*b));
    if (!b) return NULL;
    snprintf(b->name, sizeof(b->name), "i2c%d", idx);
    b->idx = idx;
    b->fd = -1; /* 懒打开：首次 transfer 才 open（fd 缓存复用） */
    return b;
}

void hal_i2c_close(hal_i2c_bus_t* bus) {
    if (!bus) return;
    if (bus->fd >= 0) close(bus->fd);
    free(bus);
}

int hal_i2c_transfer(hal_i2c_bus_t* bus, struct hal_i2c_msg* msgs, int num) {
    if (!bus || !msgs || num <= 0 || num > HAL_I2C_MSG_MAX) return -1;
    if (s_board_ops && s_board_ops->transfer)
        return s_board_ops->transfer(bus, msgs, num);

    if (bus->fd < 0) {
        char path[20];
        snprintf(path, sizeof(path), "/dev/i2c-%d", bus->idx);
        bus->fd = open(path, O_RDWR);
        if (bus->fd < 0) return -1;
    }

    struct i2c_msg km[HAL_I2C_MSG_MAX];
    struct i2c_rdwr_ioctl_data data = {km, (uint32_t)num};
    for (int i = 0; i < num; i++) {
        km[i].addr = msgs[i].addr;
        km[i].flags = msgs[i].flags; /* HAL_I2C_RD 位值同 I2C_M_RD */
        km[i].len = msgs[i].len;
        km[i].buf = msgs[i].buf;
    }
    if (ioctl(bus->fd, I2C_RDWR, &data) < 0) {
        close(bus->fd); /* 总线/器件异常：关 fd 自愈，下次 transfer 重开 */
        bus->fd = -1;
        return -1;
    }
    return 0;
}
