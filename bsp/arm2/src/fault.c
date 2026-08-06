#include "FreeRTOS.h"
#include "task.h"
#include "fault.h"
#include "stdio.h"
#include <stdarg.h>
#include "printf.h"
#include "83xx.inc"

#define TAGS "ARM2-PANIC"

#define  xprint_fault(tags,  fmt,  ...) \
do { \
    Printf("[E][%s] " fmt, TAGS, ##__VA_ARGS__); \
} while (0)
#define pr_fault(fmt, ...)  xprint_fault(TAGS, fmt, ##__VA_ARGS__)
#define ROUNDUP(a, b)           (((unsigned)(a) + ((unsigned)(b)-1)) & ~((unsigned)(b)-1))
#define MIN(x, y)               (((x) < (y)) ? (x) : (y))
#define BIT(x, bit) ((x) & (1UL << (bit)))
#define BIT_SHIFT(x, bit) (((x) >> (bit)) & 1)
#define BITS(x, high, low) ((x) & (((1UL<<((high)+1))-1) & ~((1UL<<(low))-1)))
#define BITS_SHIFT(x, high, low) (((x) >> (low)) & ((1UL<<((high)-(low)+1))-1))
#define BIT_SET(x, bit) (((x) & (1UL << (bit))) ? 1 : 0)
#define MAX_STACK_DUMP_SIZE (32 * 1024)
extern const char *xPrintCurrentTaskName( void );
unsigned long section1_start ,section2_start, section1_end, section2_end;
extern unsigned long __dram_section_start;
extern unsigned long __dram_section_end;
static volatile unsigned int curirq = 0xffffffff;
void  set_cur_irqid(unsigned int id)
{
    curirq = id;
}

static void init_section(void)
{
    section1_start = (unsigned long)&__dram_section_start;
    section1_end = (unsigned long)&__dram_section_end;
}
static int check_addr_valid(unsigned int swap_addr)
{
    int ret = 0;
    ret = ( (swap_addr < 0x500000) && (swap_addr > 0) ) ? 1 : 0;
    return ret;
}

static int find_func_call(unsigned int address)
{
    unsigned int swap_addr ;
    size_t i = 0;
    unsigned int buf = 0;
    while (1) {
        swap_addr = address + i * 4;
        if( i * 4 >= MAX_STACK_DUMP_SIZE )
            break;
        if( check_addr_valid(swap_addr) )
        {
            buf = ((const unsigned int *)address)[i];
            /* only dump function address which located at code section */
            if(buf == portTOP_FLGA)
            {
                pr_fault("reach flag:0x%x\n", buf);
                break;
            }
        }
        else
        {
            break;
        }
        i++;
    }
    return (int)(swap_addr - address);
}

void stack_hexdump(const void *ptr)
{
    unsigned int address = (unsigned int)ptr;
    char stack_per_line[256];
    char *p = (char*)stack_per_line;
    size_t count;
    size_t len = 0;

    len = (size_t)find_func_call(address);
    pr_fault("stack_hexdump :%x len :%d\n",address, len);

    for (count = 0 ; count < len; count += 16) {
        union {
            unsigned int buf[4];
            uint8_t  cbuf[16];
        } u;
        size_t s = (size_t)(ROUNDUP(MIN(len - count, 16), 4));
        size_t i;

        sprintf(p, "0x%08x>> ", address);
        p += 13;

        for (i = 0; i < s / 4; i++) {
            u.buf[i] = ((const unsigned int *)address)[i];
            sprintf(p, "0x%08x ", u.buf[i]);
            p += 11;
        }

        for (; i < 4; i++) {
            sprintf(p, "      ");
            p += 6;
        }
        sprintf(p, "|");
        p++;

        for (i=0; i < 16; i++) {
            char c = u.cbuf[i];
            if (i < s && ((c >= 0x20) && (c < 0x7f))) {
                sprintf(p, "%c", c);
                p++;
            } else {
                sprintf(p, ".");
                p++;
            }
        }
        sprintf(p, "|\n");
        pr_fault("%s", stack_per_line);
        p = (char*)stack_per_line;
        address += 16;
        if(0 == check_addr_valid(address + 16))
            break;
    }
}
/*******************************************************
 * Print register frame
 *******************************************************/
static void fault_print_frame(const arm_fault_frame_t *f)
{
    uint32_t addr = 0;
    uint32_t stack_end = 0;
    uint8_t is_irq = 0;
    for (int i = 0; i < 13; i++) {
        pr_fault("r%d: 0x%x\n", i, f->r[i]);
    }

    pr_fault("lr  : 0x%x\n", f->lr);
    pr_fault("spsr: 0x%x\n", f->spsr);

    // Add mode judgment
    uint32_t mode = f->spsr & 0x1F;
    switch(mode) {
        case 0x10: pr_fault("panic in User\n");
            addr = f->sys_sp;
        break;
        case 0x11: pr_fault("panic in FIQ\n");
            addr = f->fiq_sp;
        break;
        case 0x12: pr_fault("panic in IRQ\n");
            addr = f->irq_sp;
            stack_end = ARM2_IRQ_STACK_BASE;
            is_irq = 1;
            pr_fault("current_id %u\n", curirq);
        break;
        case 0x13: pr_fault("panic in Supervisor\n");
            addr = f->svc_sp;
            stack_end = SVC_STACK_BASE;
        break;
        case 0x17: pr_fault("panic in Abort\n");
            addr = f->abt_sp;
        break;
        case 0x1B: pr_fault("panic in Undefined\n");
            addr = f->und_sp;
        break;
        case 0x1F: pr_fault("panic in System\n");
            addr = f->sys_sp;
            stack_end = xTaskGetTopStack(NULL);
        break;
        default: pr_fault("Unknown(%x)\n", mode);
            addr = f->sys_sp;
        break;
    }

    pr_fault("fiq_sp: 0x%x\n", f->fiq_sp);
    pr_fault("fiq_lr: 0x%x\n", f->fiq_lr);
    pr_fault("irq_sp: 0x%x\n", f->irq_sp);
    pr_fault("irq_lr: 0x%x\n", f->irq_lr);
    pr_fault("svc_sp: 0x%x\n", f->svc_sp);
    pr_fault("svc_lr: 0x%x\n", f->svc_lr);
    pr_fault("abt_sp: 0x%x\n", f->abt_sp);
    pr_fault("abt_lr: 0x%x\n", f->abt_lr);
    pr_fault("und_sp: 0x%x\n", f->und_sp);
    pr_fault("und_lr: 0x%x\n", f->und_lr);
    pr_fault("sys_sp: 0x%x\n", f->sys_sp);
    pr_fault("sys_lr: 0x%x\n", f->sys_lr);

    pr_fault("current_task name %s\n", xPrintCurrentTaskName());

    arm_unwind_print_from(f->r[11] , stack_end);
    if (mode != 0x1F) {
        /* If failed not in task, than we should dump prevous task stack */
        uint32_t *task_frame = Get_SysSP(mode);
        arm_unwind_print_from(task_frame[-5], xTaskGetTopStack(NULL));
    }

    init_section();
    stack_hexdump((void*)addr);
    if(is_irq)
    {
        pr_fault("current_task name %s, irq err, start dump task sp\n", xPrintCurrentTaskName());
        stack_hexdump((void*)(f->sys_sp));    
    }
}

/*******************************************************
 * Parse exception entry
 *******************************************************/
void fault_entry_c(const arm_fault_frame_t *frame, fault_type_t type)
{
    uint32_t dfsr = read_DFSR();
    uint32_t dfar = read_DFAR();
    uint32_t ifsr = read_IFSR();
    uint32_t ifar = read_IFAR();

    pr_fault("\n========== ARM Fault ==========\n");

    uint32_t fault_pc = 0;

    switch (type)
    {
    case FAULT_UNDEF:
        pr_fault("Fault : Undefined Instruction\n");
        fault_pc = frame->lr - 4;
        break;

    case FAULT_PREFETCH:
        pr_fault("Fault : Prefetch Abort\n");
        pr_fault("IFSR  : 0x%x  IFAR : %x\n", ifsr, ifar);
        fault_pc = frame->lr - 4;
        break;

    case FAULT_DATA:
        pr_fault("Fault : Data Abort\n");
        pr_fault("DFSR  : 0x%x  DFAR : 0x%x\n", dfsr, dfar);
        fault_pc = frame->lr - 8;
        break;
    }

    pr_fault("--pc:0x%x\n", fault_pc);

    pr_fault("\n-- Register Dump --\n");
    fault_print_frame(frame);

    pr_fault("========== END Fault ==========\n");

    /* Infinite loop to halt the system */
}

void dump_callback(void)
{
    uint32_t sp = get_sp();

    dump_unwind_callback();
    pr_fault("-- Stack Dump --\n");
    stack_hexdump(sp);
}

void dump_unwind_callback(void)
{
    uint32_t cpsr = get_cpsr() & 0x1F;
    uint32_t spsr = get_spsr() & 0x1F;
    uint32_t stack_base, stack_end;

    switch(cpsr) {
        case 0x12: pr_fault("dump backtrace in [IRQ]\n");
            stack_end = ARM2_IRQ_STACK_BASE;
        break;
        case 0x13: pr_fault("dump backtrace in [SVC]\n");
            stack_end = SVC_STACK_BASE;
        break;
        case 0x17: pr_fault("dump backtrace in [Abort]\n");
            stack_end = ARM2_EXCPTION_STACK_BASE;
        break;
        case 0x1B: pr_fault("dump backtrace in [Undefined]\n");
            stack_end = ARM2_EXCPTION_STACK_BASE;
        break;
        case 0x1F: pr_fault("dump backtrace in [System]\n");
            stack_end = xTaskGetTopStack(NULL);
        break;
        default: pr_fault("dump backtrace in Unknown(%x) mode\n", cpsr);
            return;
        break;
    }
    arm_unwind_print_from(get_fp() , stack_end);
    if (cpsr != 0x1F) {
        /* If failed not in task, than we should dump prevous task stack */
        uint32_t *task_frame = Get_SysSP(spsr);
        arm_unwind_print_from(task_frame[-5], xTaskGetTopStack(NULL));
    }

    pr_fault("current_task name %s\n", xPrintCurrentTaskName());
}
