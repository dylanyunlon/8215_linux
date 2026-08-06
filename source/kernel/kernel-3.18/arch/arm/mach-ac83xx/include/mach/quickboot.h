/*
* Copyright (c) 2016 AutoChips Inc.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

#ifndef __QUICKBOOT_PARAM_
#define __QUICKBOOT_PARAM_

#define GPIO_POLARITY_LOW  0
#define GPIO_POLARITY_HIGH 1



#define GPIO_WAKEUP_STS    0
#define GPIO_WAKEUP_SRC    1
#define GPIO_PAD_IR        2


struct quickboot_param {
 
       unsigned int version;

       /*ddr calibration address */
       unsigned int ddr_cal_addr;
       /*os resume entry(physical address) */
       unsigned int nw_resume_entry;
	   /* secure os resume entry(physical address) */
	   unsigned int sw_resume_entry;
       /* wakeup source gpio & polarity config*/ 
       unsigned int wakeup_src_gpio;
       unsigned int wakeup_src_polarity;
       /*wakeup state gpio & polarity config*/
       unsigned int wakeup_sts_gpio;
       unsigned int wakeup_sts_polarity;
       
       /* delay time for system broad power off*/ 
       unsigned int power_off_delay;
       
       /* delay time for cpu reset*/
       unsigned int power_on_delay;

};


#endif



