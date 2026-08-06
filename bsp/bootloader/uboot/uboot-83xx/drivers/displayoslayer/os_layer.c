#include "u_os.h"
#include "types.h"


//typedef unsigned int CRIT_STATE_T;
//typedef void (*x_os_isr_fct) (unsigned short ui2_vector_id);


#define TRUE                (0 == 0)

int printk(const char *format, ...)
{
    //VA_LIST args;
	
    int   r = 0;

    //VA_START(args, format);
    //r = Printf(format, args);
    //VA_END(args);

    return r;
}
extern void *malloc (unsigned int len);
extern void free(void*);;
void * x_mem_alloc(unsigned long u4Size)
{
    void *  lpVirAddr = malloc(u4Size);
   /* DWORD   dwOffset = 0;
    DWORD   dwRemainSize = 0;
    
    lpVirAddr = (LPVOID)(ARM1PHY2ARM2UCV(PA_START) + (dwCurrentPoint - PA_START));
    dwRemainSize = (u4Size & 0x3)?(4 -(u4Size & 0x3)):0;
    dwCurrentPoint += u4Size + dwRemainSize;	
    Printf("call x_mem_alloc\r\n");*/
    
    return lpVirAddr;
}

void x_mem_free(void *pv_mem_block)
{
    free(pv_mem_block);
    return;
}

CRIT_STATE_T x_crit_start(void)
{
    return 0;
}
void x_crit_end(CRIT_STATE_T handle)
{
    return;
}

unsigned int x_va_to_pa(unsigned int u4VirAdd)
{
    return 0;//ARM2UCV2ARM1PHY(u4VirAdd);  
}

unsigned char BIM_ClearIrq(unsigned int u4Vector)
{
    return  TRUE;
}

extern void udelay (unsigned long usec);

void Sleep(unsigned long u4MiniSecond)
{
/*
    while(u4MiniSecond --)
    {
        volatile unsigned long u4Temp = 459000/6;
        while (u4Temp --);
    }*/
    udelay(u4MiniSecond *100);
    
}

void msleep(unsigned long u4Second)
{
      Sleep(u4Second);
}

unsigned short x_reg_isr(unsigned short  ui2_vec_id,
          x_os_isr_fct   pf_isr,
          x_os_isr_fct   *ppf_old_isr)
{
    return 0;
}

INT32 x_sema_create(HANDLE_T *ph_sema_hdl,
					SEMA_TYPE_T  e_types,
					UINT32       ui4_init_value)
{
    return OSR_OK;
}


INT32 x_sema_lock(HANDLE_T h_sema_hdl,
					SEMA_OPTION_T e_options)
{	
    return OSR_OK;
}

INT32 x_sema_delete(HANDLE_T h_sema_hdl)
{
    return OSR_OK;
}


extern void * memset(void * s,int c,unsigned int count);
void * __memzero(void* s, int n)
{
    return memset(s, 0, n);
}

DWORD WaitForMultipleObjects(
	DWORD nCount,
	const HANDLE *lpHandles,
	BOOL bWaitAll,
	DWORD dwMilliseconds)
{
    return 0;
}

void* __kmalloc(int size, INT8 flags)
{
    return NULL;
}

void kfree(const void *x)
{
    return;
}

HANDLE CreateEvent(
  LPSECURITY_ATTRIBUTES lpEventAttributes, 
  BOOL bManualReset, 
  BOOL bInitialState, 
  LPTSTR lpName 
)
{
    return 0xFFF000;  
}

VOID SetEvent(HANDLE pEvent)
{
    return;
}

VOID x_thread_delay (UINT32  u4Delay)
{
    return;
}


