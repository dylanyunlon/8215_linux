#ifndef _AC83XX_TP_
#define _AC83XX_TP_
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
extern int check_ctouch_pressed();
/** 
  * @Description:   
  *  This function get called only in case of checking entrance of recovery mode  
  *  The checking way is to read data channel to see if there is any touch  
  *  pressed for a while.  suppot multi-touch max number is five.
  * * @Return 
  *  1-5 if any touch is pressed 
  *  0- if not 
  * 
*/
extern int check_ctouch_pressed_th_num();
/** 
  * @Description:   
  *  This function get called only in case of checking entrance of recovery mode  
  *  The checking way is to read data channel to see if there is any touch  
  *  pressed for a while.
  * * @Return 
  *  1-if any touch is pressed ,point_x&point_y can return touch piont position
  *  0- if not 
  * 
*/
extern int check_ctouch_pressed_pos_xy(unsigned int *point_x, unsigned *point_y);
#endif