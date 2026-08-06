/* test_exception.c
 *
 * 用 FreeRTOS Task 主动触发 Data Abort、Prefetch Abort、Undefined 指令异常
 * 异常后会进入你已有的 ARM9 异常 dump 流程。
 */

#include "FreeRTOS.h"
#include "task.h"
#include <stdint.h>
#include <stdio.h>
#include "printf.h"

#define TAGS "TEST_EXCPTION"
/* -------------------------------
 * 主动触发 Data Abort
 * ------------------------------- */
void trigger_data_abort(void)
{
    pr_info("[TEST] Trigger Data Abort...\n");

    volatile uint32_t *bad = (uint32_t*)0x0; // 明显的非法地址
    *bad = 0x12345678;  // 写入 → Data Abort
}

/* -------------------------------
 * 主动触发 Prefetch Abort
 * ------------------------------- */
void trigger_prefetch_abort(void)
{
    pr_info("[TEST] Trigger Prefetch Abort...\n");

    void (*bad_func)(void) = (void (*)(void))0xFFFFFFF0; // 不可执行指令地址
    bad_func();  // 执行 → Prefetch Abort
}

/* -------------------------------
 * 主动触发 Undefined Instruction
 * ------------------------------- */
void trigger_undef(void)
{
    pr_info("[TEST] Trigger Undefined Instruction...\n");

    __asm__ volatile(
        ".word 0xFFFFFFFF\n"   // 非法指令 → Undefined Instruction
    );
}

int g_test=0;
void trigger_reset(int b)
{
    int a = 1;
    int c;
    pr_info("[TEST] Trigger trigger_reset...\n");

    c = a / b;
    g_test = c;
}
/* -------------------------------
 * FreeRTOS test Task
 * 顺序触发三种错误
 * ------------------------------- */
static void ExceptionTestTask(void* pvParameters)
{
    pr_info("[TEST] Trigger Undefined Instruction...\n");

}

/* -------------------------------
 * 对外接口：创建测试任务
 * ------------------------------- */
void start_exception_test_task(void)
{
    xTaskCreate(ExceptionTestTask,
                "ExTest",
                1024,
                NULL,
                tskIDLE_PRIORITY + 2,
                NULL);
}
