/** 极简 CAN HAL -- Linux SocketCAN 后端（板级回调优先）
 *
 * 默认走 AF_CAN/SOCK_RAW/CAN_RAW：通道名映射网络接口（如 "can0"）。
 * 波特率/模式由内核侧配置（ip link set can0 type can bitrate 500000），
 * 本后端不重复造轮子。socket 懒创建：首次收发才 socket+bind
 * （can0 未 up 时不阻塞 init，接口就绪后自愈），出错关 fd 下次重试。
 */
#include "hal_can.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>

#define HAL_CAN_NAME_MAX 16
#define HAL_CAN_WAIT_FOREVER 0xFFFFFFFFu /* 同 OSAL_WAIT_FOREVER */

struct hal_can {
    char name[HAL_CAN_NAME_MAX];
    int fd; /* -1=未打开（懒打开/出错自愈后重开） */
};

static const hal_can_ops_t* s_board_ops = NULL;

void hal_can_register_board(const hal_can_ops_t* ops) { s_board_ops = ops; }

const char* hal_can_name(const hal_can_t* can) {
    return can ? can->name : NULL;
}

/** 单调时钟 ms（本层与 OSAL 解耦，直接用 POSIX） */
static uint64_t now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000u + (uint64_t)ts.tv_nsec / 1000000u;
}

/** 懒打开：socket + bind 到 name 对应网络接口 */
static int can_ensure_open(struct hal_can* c) {
    if (c->fd >= 0) return 0;

    int fd = socket(AF_CAN, SOCK_RAW, CAN_RAW);
    if (fd < 0) return -1;

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", c->name);
    if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0 || ifr.ifr_ifindex <= 0) {
        close(fd);
        return -1;
    }

    struct sockaddr_can addr;
    memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(fd);
        return -1;
    }

    c->fd = fd;
    return 0;
}

hal_can_t* hal_can_open(const char* name) {
    if (!name || !name[0]) return NULL;
    struct hal_can* c = (struct hal_can*)calloc(1, sizeof(*c));
    if (!c) return NULL;
    snprintf(c->name, sizeof(c->name), "%s", name);
    c->fd = -1;
    return c;
}

void hal_can_close(hal_can_t* can) {
    if (!can) return;
    if (can->fd >= 0) close(can->fd);
    free(can);
}

static void hal_to_kframe(const struct hal_can_frame* f, struct can_frame* k) {
    memset(k, 0, sizeof(*k));
    k->can_id = f->id;
    if (f->flags & HAL_CAN_FRAME_EXT) k->can_id |= CAN_EFF_FLAG;
    if (f->flags & HAL_CAN_FRAME_RTR) k->can_id |= CAN_RTR_FLAG;
    k->can_dlc = f->dlc > HAL_CAN_DATA_LEN ? HAL_CAN_DATA_LEN : f->dlc;
    memcpy(k->data, f->data, k->can_dlc);
}

static void kframe_to_hal(const struct can_frame* k, struct hal_can_frame* f) {
    memset(f, 0, sizeof(*f));
    if (k->can_id & CAN_EFF_FLAG) {
        f->flags |= HAL_CAN_FRAME_EXT;
        f->id = k->can_id & CAN_EFF_MASK;
    } else {
        f->id = k->can_id & CAN_SFF_MASK;
    }
    if (k->can_id & CAN_RTR_FLAG) f->flags |= HAL_CAN_FRAME_RTR;
    f->dlc = k->can_dlc > HAL_CAN_DATA_LEN ? HAL_CAN_DATA_LEN : k->can_dlc;
    memcpy(f->data, k->data, f->dlc);
}

/** 收一帧。wait_ms 同 OSAL 语义；返回 1=收到，0=超时/无数据 */
static int recv_one(int fd, struct can_frame* k, uint32_t wait_ms) {
    /* 0=非阻塞排空，0xFFFFFFFF=永久阻塞，其余设 SO_RCVTIMEO */
    int flags = (wait_ms == 0) ? MSG_DONTWAIT : 0;
    if (wait_ms != 0 && wait_ms != HAL_CAN_WAIT_FOREVER) {
        struct timeval tv;
        tv.tv_sec = (time_t)(wait_ms / 1000u);
        tv.tv_usec = (suseconds_t)((wait_ms % 1000u) * 1000u);
        if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
            return 0;
    }
    ssize_t n = recv(fd, k, sizeof(*k), flags);
    return (n == (ssize_t)sizeof(*k)) ? 1 : 0;
}

int hal_can_send(hal_can_t* can, const struct hal_can_frame* frames, int num,
                 uint32_t timeout_ms) {
    if (!can || !frames || num <= 0) return -1;
    if (s_board_ops && s_board_ops->send)
        return s_board_ops->send(can, frames, num, timeout_ms);
    if (can_ensure_open(can) != 0) return -1;

    /* 发送超时：仅有限值时设置；0=非阻塞，FOREVER=默认阻塞 */
    if (timeout_ms != 0 && timeout_ms != HAL_CAN_WAIT_FOREVER) {
        struct timeval tv;
        tv.tv_sec = (time_t)(timeout_ms / 1000u);
        tv.tv_usec = (suseconds_t)((timeout_ms % 1000u) * 1000u);
        if (setsockopt(can->fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0)
            return -1;
    }

    for (int i = 0; i < num; i++) {
        struct can_frame k;
        int flags = (timeout_ms == 0) ? MSG_DONTWAIT : 0;
        hal_to_kframe(&frames[i], &k);
        ssize_t n = send(can->fd, &k, sizeof(k), flags);
        if (n != (ssize_t)sizeof(k)) {
            close(can->fd); /* 总线未 up/缓冲满：关 fd 自愈，下次重试 */
            can->fd = -1;
            return -1;
        }
    }
    return 0;
}

int hal_can_recv(hal_can_t* can, struct hal_can_frame* frames, int num,
                 uint32_t timeout_ms) {
    if (!can || !frames || num <= 0) return -1;
    if (s_board_ops && s_board_ops->recv)
        return s_board_ops->recv(can, frames, num, timeout_ms);
    if (can_ensure_open(can) != 0) return 0; /* 接口未就绪视作无帧 */

    /* 第一帧按 timeout_ms 等待；后续帧非阻塞排空同批连帧 */
    int got = 0;
    while (got < num) {
        struct can_frame k;
        uint32_t wait = (got == 0) ? timeout_ms : 0;
        if (recv_one(can->fd, &k, wait) != 1) break;
        kframe_to_hal(&k, &frames[got++]);
    }
    return got;
}

int hal_can_set_filter(hal_can_t* can, const uint32_t* ids, int num) {
    if (!can) return -1;
    if (s_board_ops && s_board_ops->set_filter)
        return s_board_ops->set_filter(can, ids, num);
    if (can_ensure_open(can) != 0) return -1;
    if (!ids || num <= 0) return 0; /* 未指定：保持当前过滤不变 */

    struct can_filter* f =
        (struct can_filter*)malloc(sizeof(struct can_filter) * (size_t)num);
    if (!f) return -1;
    for (int i = 0; i < num; i++) {
        /* 11bit 标准帧精确匹配（EFF 位须为 0，扩展帧不收） */
        f[i].can_id = ids[i] & CAN_SFF_MASK;
        f[i].can_mask = CAN_SFF_MASK | CAN_EFF_FLAG;
    }
    int ret = setsockopt(can->fd, SOL_CAN_RAW, CAN_RAW_FILTER, f,
                         (socklen_t)(sizeof(struct can_filter) * (size_t)num));
    free(f);
    return (ret == 0) ? 0 : -1;
}
