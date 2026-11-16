/*
 * ui_backend.c（device 后端）—— 见 port/ui_backend.h
 *
 * 两个后端的 ui_backend_init() 第一行都是 vs_init()（唯一入口，见 port/ui_backend.h）。
 * 之后车辆数据由中间件的 CAN/MCU 模块写入 vehicle_param 总表，ui/proxy 轮询读取。
 *
 * ⚠ 这里**不注册** vs_subscribe 回调：ui/logic/logic.c 是每 50ms 轮询 proxy 的模型，
 *   回调驱动需要额外的跨线程消息队列。
 */
#include <stdio.h>
#include "ui_backend.h"
#include "vehicle_services.h"      /* vs_init：与 pc 共用的那条入口 */

void ui_backend_init(void)
{
    /* vs_init() 内部拉起中间件（gpio/背光/msg_manage/coms_mcu…），各自去开 /dev 节点。
     * /dev 打不开的报错，属预期现象。 */
    printf("[ui_backend] vs_init() ...\n");
    fflush(stdout);

    if (vs_init() != 0) {
        printf("[ui_backend] vs_init() failed; UI 继续跑，数据停在初值\n");
        fflush(stdout);
    }
}
