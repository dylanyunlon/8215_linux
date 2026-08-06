#ifndef _X_IRQ_H_
#define _X_IRQ_H_


#if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3360) 
#include "3360_irqs_vector.h"

#elif(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3356) 
#include "3356_irqs_vector.h"

#elif(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363) 
#include "3363_irqs_vector.h"

#endif



#if(IRQ_ENABLE)


typedef void(*ISR_FUN)(UINT32 vectorID);

#define REGISTER_IRQ    Register_ISR
#define UNREGISTER_IRQ  Unregiser_ISR
#define ENABLE_IRQ      Enable_IRQ
#define DISABLE_IRQ     Disable_IRQ


extern void Register_ISR(UINT32 vectorID,ISR_FUN pISR);
extern void Unregiser_ISR(UINT32 vectorID);
extern void Enable_IRQ(void);
extern void Disable_IRQ(void);
extern void Relocate_IRQ_Entry(void);


#else

#define REGISTER_IRQ(vector,fun)
#define UNREGISTER_IRQ(vector)
#define ENABLE_IRQ()
#define DISABLE_IRQ()


#endif

#endif



