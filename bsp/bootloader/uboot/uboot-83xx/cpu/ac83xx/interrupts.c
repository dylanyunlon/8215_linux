/*
 * (C) Copyright 2006, NXP BV
 * Jean-Paul Saman
 *
 * (C) Copyright 2004
 * Texas Instruments
 * Richard Woodruff <r-woodruff2@ti.com>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 * Alex Zuepke <azu@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/proc-armv/ptrace.h>
#include <asm/arch/ac83xx_basic.h>
#include <asm/arch/x_typedef.h>

#include <asm/arch/x_bim.h>

extern void reset_cpu(ulong addr);
#define TIMER_LOAD_VAL 0

/* macro to read the 32 bit timer */
//#define READ_TIMER timer
#define READ_TIMER (REG_TIMER_CNT)
static volatile UINT32 timer = 0;

#ifdef CONFIG_USE_IRQ
/* enable IRQ interrupts */
void enable_interrupts (void)
{
	unsigned long temp;
	__asm__ __volatile__("mrs %0, cpsr\n"
			 "bic %0, %0, #0x80\n"
			 "msr cpsr_c, %0"
			 : "=r" (temp)
			 :
			 : "memory");
}

/*
 * disable IRQ/FIQ interrupts
 * returns true if interrupts had been enabled before we disabled them
 */
int disable_interrupts (void)
{
	unsigned long old,temp;
	__asm__ __volatile__("mrs %0, cpsr\n"
			 "orr %1, %0, #0xc0\n"
			 "msr cpsr_c, %1"
			 : "=r" (old), "=r" (temp)
			 :
			 : "memory");
	return(old & 0x80) == 0;
}
#else
void enable_interrupts (void)
{
	return;
}
int disable_interrupts (void)
{
	return 0;
}
#endif

UINT32 u4_get_irq_vector_id(void)
{
    UINT32 u4IrqVec;
    UINT32 u4Id;

    // level 1
    u4IrqVec = REG_IRQ_STATUS & REG_IRQ_ENABLE;
    if(u4IrqVec)
    {
        for(u4Id = 0; u4Id < 32; u4Id++)
        {
            if(u4IrqVec & (0x1 << u4Id))
            {
            	  return u4Id;
            }
        }
    }

    // level 2
    u4IrqVec = REG_IRQ_STATUS2 & REG_IRQ_ENABLE2;
    if(u4IrqVec)
    {
        for(u4Id = 0; u4Id < 32; u4Id++)
        {
            if(u4IrqVec & (0x1 << u4Id))
            {
            	  return u4Id + 32;
            }
        }
    }

    // level 3
    u4IrqVec = REG_IRQ_STATUS3 & REG_IRQ_ENABLE3;
    if(u4IrqVec)
    {
        for(u4Id = 0; u4Id < 32; u4Id++)
        {
            if(u4IrqVec & (0x1 << u4Id))
            {
            	  return u4Id + 64;
            }
        }
    }

    return IRQ_VECTOR_MAX_NUM;
}

void v_enable_bim_irq(UINT32 u4Id)
{
    // level 1
    if(u4Id < 32)
    {
        REG_IRQ_ENABLE = REG_IRQ_ENABLE | (0x1 << u4Id);
	 return;
    }

    // level 2
    if(u4Id < 64)
    {
        u4Id -= 32;
        REG_IRQ_ENABLE2 = REG_IRQ_ENABLE2 | (0x1 << u4Id);
	 return;
    }

    // level 3
    if(u4Id < 96)
    {
        u4Id -= 64;
        REG_IRQ_ENABLE3 = REG_IRQ_ENABLE3 | (0x1 << u4Id);
	 return;
    }

}

void v_disable_bim_irq(UINT32 u4Id)
{
     // level 1
    if(u4Id < 32)
    {
        REG_IRQ_ENABLE = REG_IRQ_ENABLE & ~(0x1 << u4Id);
	 return;
    }

    // level 2
    if(u4Id < 64)
    {
        u4Id -= 32;
        REG_IRQ_ENABLE2 = REG_IRQ_ENABLE2 & ~(0x1 << u4Id);
	 return;
    }

    // level 3
    if(u4Id < 96)
    {
        u4Id -= 64;
        REG_IRQ_ENABLE3 = REG_IRQ_ENABLE3 & ~(0x1 << u4Id);
	 return;
    }

}

void v_clear_bim_irq(UINT32 u4Id)
{
    // level 1
    if(u4Id < 32)
    {
        REG_IRQ_CLEAR = (0x1 << u4Id);
        REG_IRQ_STATUS = (0x1 << u4Id);
	 return;
    }

    // level 2
    if(u4Id < 64)
    {
        u4Id -= 32;
        REG_IRQ_CLEAR2 = (0x1 << u4Id);
        REG_IRQ_STATUS2 = (0x1 << u4Id);
	 return;
    }

    // level 3
    if(u4Id < 96)
    {
        u4Id -= 64;
        REG_IRQ_CLEAR3= (0x1 << u4Id);
        REG_IRQ_STATUS3= (0x1 << u4Id);
	 return;
    }
}

void bad_mode (void)
{
	hang();
	panic ("Resetting CPU ...\n");
	reset_cpu (0);
}

void show_regs (struct pt_regs *regs)
{
	unsigned long flags;
	const char *processor_modes[] = {
		"USER_26",  "FIQ_26",   "IRQ_26",   "SVC_26",
		"UK4_26",   "UK5_26",   "UK6_26",   "UK7_26",
		"UK8_26",   "UK9_26",   "UK10_26",  "UK11_26",
		"UK12_26",  "UK13_26",  "UK14_26",  "UK15_26",
		"USER_32",  "FIQ_32",   "IRQ_32",   "SVC_32",
		"UK4_32",   "UK5_32",   "UK6_32",   "ABT_32",
		"UK8_32",   "UK9_32",   "UK10_32",  "UND_32",
		"UK12_32",  "UK13_32",  "UK14_32",  "SYS_32",
	};

	flags = condition_codes (regs);

	printf ("pc : [<%08lx>]    lr : [<%08lx>]\n"
			"sp : %08lx  ip : %08lx  fp : %08lx\n",
			instruction_pointer (regs),
			regs->ARM_lr, regs->ARM_sp, regs->ARM_ip, regs->ARM_fp);
	printf ("r10: %08lx  r9 : %08lx  r8 : %08lx\n",
			regs->ARM_r10, regs->ARM_r9, regs->ARM_r8);
	printf ("r7 : %08lx  r6 : %08lx  r5 : %08lx  r4 : %08lx\n",
			regs->ARM_r7, regs->ARM_r6, regs->ARM_r5, regs->ARM_r4);
	printf ("r3 : %08lx  r2 : %08lx  r1 : %08lx  r0 : %08lx\n",
			regs->ARM_r3, regs->ARM_r2, regs->ARM_r1, regs->ARM_r0);
	printf ("Flags: %c%c%c%c",
			flags & CC_N_BIT ? 'N' : 'n',
			flags & CC_Z_BIT ? 'Z' : 'z',
			flags & CC_C_BIT ? 'C' : 'c', flags & CC_V_BIT ? 'V' : 'v');
	printf ("  IRQs %s  FIQs %s  Mode %s%s\n",
			interrupts_enabled (regs) ? "on" : "off",
			fast_interrupts_enabled (regs) ? "on" : "off",
			processor_modes[processor_mode (regs)],
			thumb_mode (regs) ? " (T)" : "");
}

void do_undefined_instruction (struct pt_regs *pt_regs)
{
	printf ("undefined instruction\n");
	show_regs (pt_regs);
	bad_mode ();
}

void do_software_interrupt (struct pt_regs *pt_regs)
{
	printf ("software interrupt\n");
	show_regs (pt_regs);
	bad_mode ();
}

void do_prefetch_abort (struct pt_regs *pt_regs)
{
	printf ("prefetch abort\n");
	show_regs (pt_regs);
	bad_mode ();
}

void do_data_abort (struct pt_regs *pt_regs)
{
	printf ("data abort\n");
	show_regs (pt_regs);
	bad_mode ();
}

void do_not_used (struct pt_regs *pt_regs)
{
	printf ("not used\n");
	show_regs (pt_regs);
	bad_mode ();
}

void do_fiq (struct pt_regs *pt_regs)
{
	printf ("fast interrupt request\n");
	show_regs (pt_regs);
	bad_mode ();
}

void do_irq (struct pt_regs *pt_regs)
{
        UINT32 u4VectorId;

	u4VectorId = u4_get_irq_vector_id();
	v_clear_bim_irq(u4VectorId);

       switch(u4VectorId){
               case VECTOR_T0:
			timer = timer + 1;
			break;
		default:
			break;
       }
}


static ulong timestamp;
static ulong lastinc;

/* nothing really to do with interrupts, just starts up a counter. */

void v_timer_interrupt_init(void)
{
	/* Start timer irq */
	REG_TIMER_CONTROL = 0x0;
	REG_TIMER_CNT = CFG_CLOCK_PER_TICKS - 1;
	REG_TIMER_LIMIT = CFG_CLOCK_PER_TICKS;
	REG_TIMER_CONTROL = 0x3;
	v_enable_bim_irq(VECTOR_T0);
	//REG_IRQ_ENABLE = REG_IRQ_ENABLE | IRQ_TIMER0;

	reset_timer_masked(); /* init the timestamp and lastinc value */
}

int interrupt_init (void)
{
	/*
	 * setup up stacks if necessary
	 */
	IRQ_STACK_START = _armboot_start - CONFIG_SYS_MALLOC_LEN - CONFIG_SYS_GBL_DATA_SIZE - 4;
	FIQ_STACK_START = IRQ_STACK_START - CONFIG_STACKSIZE_IRQ;

	/* timer interrupt init */
	v_timer_interrupt_init();


	/* enable arm irq interrupt */
	enable_interrupts();
	return(0);
}

/*
 * timer without interrupts
 */
void reset_timer (void)
{
	reset_timer_masked ();
}

ulong get_timer (ulong base)
{
	return ((get_timer_masked () - base) / 27000);
}

void set_timer (ulong t)
{
	timestamp = t;
}
#if 0
/* delay x useconds AND perserve advance timstamp value */
void udelay (unsigned long usec)
{
	ulong tmo, tmp;

	if (usec >= 1000) {			/* if "big" number, spread normalization to seconds */
		tmo = usec / 1000;		/* start to normalize for usec to ticks per sec */
		tmo *= CONFIG_SYS_HZ;			/* find number of "ticks" to wait to achieve target */
		tmo /= 1000;			/* finish normalize. */
	} else {				/* else small number, don't kill it prior to HZ multiply */
		tmo = usec * CONFIG_SYS_HZ;
		tmo /= (1000*1000);
	}

	tmp = get_timer (0);		/* get current timestamp */
	if ( (tmo + tmp + 1) < tmp )	/* if setting this forward will roll time stamp */
		reset_timer_masked ();	/* reset "advancing" timestamp to 0, set lastinc value */
	else
		tmo	+= tmp;		/* else, set advancing stamp wake up time */
	while (get_timer_masked () < tmo)/* loop till event */
		/*NOP*/;
}
#endif
void reset_timer_masked (void)
{
	/* reset time */
	lastinc = 0xFFFFFFFF - READ_TIMER;		/* capture current incrementer value time */
	timestamp = 0;			/* start "advancing" time stamp from 0 */
}

ulong get_timer_masked (void)
{
	ulong now = 0xFFFFFFFF - READ_TIMER;		/* current tick value */

	if (now >= lastinc)			/* normal mode (non roll) */
		timestamp += (now - lastinc);	/* move stamp fordward with absoulte diff ticks */
	else					/* we have rollover of incrementer */
		timestamp += (0xFFFFFFFF - lastinc) + now;
	lastinc = now;
	return timestamp;
}

/* waits specified delay value and resets timestamp */
void udelay_masked (unsigned long usec)
{
	ulong tmo;

	if (usec >= 1000) {		/* if "big" number, spread normalization to seconds */
		tmo = usec / 1000;	/* start to normalize for usec to ticks per sec */
		tmo *= CONFIG_SYS_HZ;		/* find number of "ticks" to wait to achieve target */
		tmo /= 1000;		/* finish normalize. */
	} else {			/* else small number, don't kill it prior to HZ multiply */
		tmo = usec * CONFIG_SYS_HZ;
		tmo /= (1000*1000);
	}
	reset_timer_masked ();		/* set "advancing" timestamp to 0, set lastinc vaule */
	while (get_timer_masked () < tmo) /* wait for time stamp to overtake tick number.*/
		/* NOP */;
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return get_timer(0);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk (void)
{
	ulong tbclk;
	tbclk = CONFIG_SYS_HZ;
	return tbclk;
}


#define REG_RW_T64b_LO_0   0x0728
#define REG_RW_T64b_HI_0   0x072C
#define T64B_GET_LOW()	BIM_READ32(REG_RW_T64b_LO_0)
#define T64B_GET_HIGH()	BIM_READ32(REG_RW_T64b_HI_0)
#define USEC_TICKS 27

UINT64 GetHiTimerTick(void)
{
    UINT32 high = T64B_GET_HIGH();
    UINT32 low  = T64B_GET_LOW();

    while (high != T64B_GET_HIGH())
    {
        high = T64B_GET_HIGH();
        low = T64B_GET_LOW();
    }

    return((((UINT64)high << 32) | low));
}

UINT32 GetMinisecond (VOID)
{
    UINT64 u8Ticks = 0;
	UINT32 second = 0;
    u8Ticks = GetHiTimerTick();
    second = (UINT32)(u8Ticks >> 4);
	second = second / (27000000 / 16);
    return (UINT32) second;
}

void udelay (unsigned long usec)
{
	UINT64 tmo, tmp;

  tmp = usec * 27;
  tmo = GetHiTimerTick() + tmp;
  while(GetHiTimerTick() < tmo);

}

