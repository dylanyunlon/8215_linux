#include <stdint.h>
#include <stdio.h>
#include "addr2func.h"
#include "fault.h"

#define TAGS "ARM2-PANIC"

#define  xprint_fault(tags,  fmt,  ...) \
do { \
    Printf("[E][%s] " fmt, TAGS, ##__VA_ARGS__); \
} while (0)
#define pr_fault(fmt, ...)  xprint_fault(TAGS, fmt, ##__VA_ARGS__)

/*
 * ARMv6/v7 frame-pointer based stack unwinder (-fno-omit-frame-pointer)
 *
 * This unwinder walks the chain built by typical GCC prologue:
 *   push {fp, lr}
 *   add  fp, sp, #4
 * so that:
 *   [fp - 4] holds the previous frame pointer
 *   [fp + 0] holds the saved link register (return address)
 *
 * After resolving a return address, it prints the function name(s) using
 * addr2func_chain(), including inline frames when present.
 */
#ifndef UNW_MAX_STACK_DEPTH
#define UNW_MAX_STACK_DEPTH   (16)
#endif

/*
 * Configuration knobs for heuristic scanning when FP chain breaks.
 */
#ifndef UNW_MAX_STACK_SCAN_BYTES
#define UNW_MAX_STACK_SCAN_BYTES   (64 * 1024)
#endif

#ifndef UNW_FALLBACK_SCAN_WORDS
#define UNW_FALLBACK_SCAN_WORDS    1024
#endif

#ifndef UNW_MAX_FRAME_DELTA_BYTES
#define UNW_MAX_FRAME_DELTA_BYTES  (8 * 1024)
#endif

#ifndef UNW_SYS_STACK_BASE
#define UNW_SYS_STACK_BASE         (0x1000) // refer from 83xx.inc
#endif

#ifndef UNW_SYS_STACK_END
#define UNW_SYS_STACK_END          (0x30000) // refer from 83xx.inc
#endif

#ifndef UNW_TASK_STACK_BASE
extern const unsigned char __heap_start[];
#define UNW_TASK_STACK_BASE         ((uint32_t)__heap_start) // locate in unHeap
#endif

#ifndef UNW_TASK_STACK_END
extern const unsigned char __heap_end[];
#define UNW_TASK_STACK_END          ((uint32_t)__heap_end) // locate in unHeap
#endif

#ifndef UNW_CODE_BASE
extern const unsigned char _start[];
#define UNW_CODE_BASE               ((uint32_t)_start) // refer from lds
#endif

#ifndef UNW_CODE_END
extern const unsigned char __dram_rodata_start[];
#define UNW_CODE_END                ((uint32_t)__dram_rodata_start) // refer from lds
#endif

/*
 * is_probable_stack_addr - quick check that an address lies near SP
 * @addr: address to check
 *
 * Return: 1 if addr within [sp0 - window, sp0 + window], else 0
 */
static int is_probable_stack_addr(uint32_t addr)
{
    if ((addr >= UNW_SYS_STACK_BASE && addr <= UNW_SYS_STACK_END) ||
        (addr >= UNW_TASK_STACK_BASE && addr <= UNW_TASK_STACK_END)) {
        return 1;
    }

    return 0;
}

/*
 * is_probable_code_addr - quick check that an address lies on code
 * @addr: address to check
 *
 * Return: 1 if addr within [Text], else 0
 */
static int is_probable_code_addr(uint32_t addr)
{
    /* don't support thumb code at default */
    if (addr & 3)
        return 0;
    return (addr >= UNW_CODE_BASE && addr <= UNW_CODE_END);
}

/*
 * print_chain_for_addr - print function chain with optional heuristic tag
 * @addr:      instruction address (PC) to resolve
 */
static void print_chain_for_addr(uint32_t addr)
{
    const char *names[8];
    uint8_t flags[8];
    size_t n, i;

    if (ft_get_total_size() == 0) {
        pr_fault("---> 0x%08x | (None)", addr);
        return;
    }

    n = addr2func_chain(addr, names, flags, 8);
    if (n == 0) {
        n = 1;
        names[0] = "not-found";
        flags[0] = 0;
    }

    for (i = 0; i < n; i++) {
        pr_fault("%s---> 0x%08x |(%s) %s",
              i == 0 ? "" : "   ",
              addr,
              flags[i] & FT_FLAG_INLINE ? "inline" : "func",
              names[i]);
        if (i + 1 < n)
            pr_fault(" -> ");
    }
}

/*
 * heuristic_scan_and_print - after FP break, scan stack for code-looking words
 * @base_ptr: starting pointer to scan from (FP if plausible, else SP)
 * @sp0:      baseline SP captured at unwind start
 * @budget:   remaining frames budget to print
 *
 * Return: number of heuristic frames printed
 */
static uint32_t heuristic_scan_and_print(uint32_t base_ptr, uint32_t sp_end)
{
    uint32_t printed = 0;
    uint32_t i, size;
    volatile uint32_t *p = (volatile uint32_t *)base_ptr;

    size = (sp_end - base_ptr) / sizeof(uint32_t);
    for (i = 0; i < size; i++) {
        uint32_t addr = p[i];

        if (!is_probable_code_addr(addr)) {
            continue;
        }

        if (ft_get_total_size() == 0) {
            print_chain_for_addr(addr);
        } else {
            /* Only print if it maps into a known code range */
            const char *name = addr2func(addr);
            if (name) {
                print_chain_for_addr(addr);
                printed++;
            }
        }
    }

    return printed;
}

/*
 * arm_unwind_print_from - unwind and print from a specific frame pointer
 * @fp:         starting frame pointer
 * @sp_top:     ending frame pointer
 *
 * The unwinder reads saved LR at [fp] and previous FP at [fp - 4]. For each
 * frame, it resolves (lr - 4) to place the address within the caller.
 *
 * Return: number of frames printed
 */
int arm_unwind_print_from(uint32_t fp, uint32_t sp_top)
{
    volatile uint32_t *p;
    uint32_t lr;
    uint32_t prev_fp;
    uint32_t addr;
    int chain_broken = 0;
    int unwind_num = 0;

    if (ft_get_total_size() == 0) {
        pr_fault("--- Can't parse function name: DBG_FUNCTION != enable ---");
    }

    //pr_fault("============== backtrace:[0x%08x~0x%08x]===============", fp, sp_top);
    /* limit the scan size to prevent infinite loops */
    if (sp_top - fp >= UNW_FALLBACK_SCAN_WORDS * sizeof(uint32_t))
        sp_top = fp + UNW_FALLBACK_SCAN_WORDS * sizeof(uint32_t);

    /* Basic sanity: avoid NULL and misaligned FP */
    if (fp == 0 || (fp & 3)) {
        chain_broken = 1;
    }

    if (!is_probable_stack_addr(fp)) {
        return;
    }

    /* Parsing FP chain */
    while (!chain_broken && fp) {
        p = (volatile uint32_t *)fp;
        lr = p[0];          /* saved LR */
        prev_fp = p[-1];    /* previous FP at [fp - 4] */
        addr = (lr > 4) ? (lr - 4) : lr;

        if (!is_probable_code_addr(lr)) {
            chain_broken = 1;
            break;
        }

        /* Resolve and print current frame */
        print_chain_for_addr(addr);
        unwind_num++;

        if (prev_fp == sp_top) {
            goto unwind_end;
        }

        /* Validate next FP before advancing */
        if (!is_probable_stack_addr(prev_fp)) {
            chain_broken = 1;
            break;
        }

        fp = prev_fp;
    }

    if (chain_broken) {
        pr_fault("[unwind] FP chain broken");
        pr_fault("Switching to heuristic scan. Subsequent frames may be inaccurate");
        /* Prefer scanning from current FP if it's at least near SP; else start at SP */
        unwind_num += heuristic_scan_and_print(fp, sp_top);
    }

unwind_end:
    pr_fault("----- [ end trace ] -----");
    return unwind_num;
}
