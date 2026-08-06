#ifndef __FAULT_H__
#define __FAULT_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Exception types */
typedef enum {
    FAULT_UNDEF       = 1,
    FAULT_PREFETCH    = 2,
    FAULT_DATA        = 3,
} fault_type_t;

/* Saved register frame (must be consistent with the saving order in exception.S) */
typedef struct
{
    uint32_t r[13];      /* r0-r12 */
    uint32_t lr;         /* Exception mode LR */
    uint32_t spsr;       /* Exception mode SPSR */
    uint32_t fiq_sp;     /* FIQ mode SP */
    uint32_t fiq_lr;     /* FIQ mode LR */
    uint32_t irq_sp;     /* IRQ mode SP */
    uint32_t irq_lr;     /* IRQ mode LR */
    uint32_t svc_sp;     /* SVC mode SP */
    uint32_t svc_lr;     /* SVC mode LR */
    uint32_t abt_sp;     /* ABT mode SP */
    uint32_t abt_lr;     /* ABT mode LR */
    uint32_t und_sp;     /* UND mode SP */
    uint32_t und_lr;     /* UND mode LR */
    uint32_t sys_sp;     /* SYS mode SP */
    uint32_t sys_lr;     /* SYS mode LR */
} arm_fault_frame_t;

/* C exception handler entry */
void fault_entry_c(const arm_fault_frame_t *frame, fault_type_t type);

/* Platform output function (implement by yourself) */
void platform_printf(const char *fmt, ...);

/* ARM coprocessor register read */
static inline uint32_t read_DFSR(void)
{
    uint32_t v;
    __asm__ volatile("mrc p15, 0, %0, c5, c0, 0" : "=r"(v));
    return v;
}

static inline uint32_t read_DFAR(void)
{
    uint32_t v;
    __asm__ volatile("mrc p15, 0, %0, c6, c0, 0" : "=r"(v));
    return v;
}

static inline uint32_t read_IFSR(void)
{
    uint32_t v;
    __asm__ volatile("mrc p15, 0, %0, c5, c0, 1" : "=r"(v));
    return v;
}

static inline uint32_t read_IFAR(void)
{
    return 0;
}


/*
 * get_fp - read current frame pointer (r11)
 *
 * Return: current FP value
 */
static inline uint32_t get_fp(void)
{
    uint32_t fp;
    __asm__ volatile ("mov %0, fp" : "=r"(fp));
    return fp;
}

/*
 * get_lr - read current link register (r14)
 *
 * Return: current LR value
 */
static inline uint32_t get_lr(void)
{
    uint32_t lr;
    __asm__ volatile ("mov %0, lr" : "=r"(lr));
    return lr;
}

/*
 * get_sp - read current stack pointer (r13)
 *
 * Return: current SP value
 */
static inline uint32_t get_sp(void)
{
    uint32_t sp;
    __asm__ volatile ("mov %0, sp" : "=r"(sp));
    return sp;
}

/*
 * get_cpsr - read current state (cpsr)
 *
 * Return: current CPSR value
 */
static inline uint32_t get_cpsr(void)
{
    uint32_t cpsr;
    __asm__ volatile ("mrs %0, cpsr" : "=r"(cpsr));
    return cpsr;
}

/*
 * get_spsr - read saved state (spsr)
 *
 * Return: current SPSR value
 */
static inline uint32_t get_spsr(void)
{
    uint32_t spsr;
    __asm__ volatile ("mrs %0, spsr" : "=r"(spsr));
    return spsr;
}
void  set_cur_irqid(unsigned int id);
#ifdef __cplusplus
}
#endif

#endif
