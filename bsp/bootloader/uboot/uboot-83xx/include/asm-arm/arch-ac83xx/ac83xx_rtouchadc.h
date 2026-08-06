#ifndef _AC83XX_TOUCHADC_H
#define _AC83XX_TOUCHADC_H

#include <asm/arch/x_typedef.h>
/** 
  * @Description:   
  *  This function get called only in case of checking entrance of recovery mode  
  *  The checking way is to read data channel to see if there is any touch  
  *  pressed for a while.  
  * * @Return 
  *  1- if any touch is pressed 
  *  0- if not 
  * 
*/
extern int check_rtouch_pressed(void);
extern void touchadc_init();
extern void touchadc_dinit();

#endif