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

#ifndef ATC_DRV_POLLING_H
#define ATC_DRV_POLLING_H
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/delay.h>

#define WAIT_FOR_STATUS(status, timeout, timeoutstr) \
{\
    unsigned long timeo = jiffies + msecs_to_jiffies(2);\
    bool gotstatus = true;\
    while (!(status))\
    {\
        if (time_after(jiffies, timeo))\
        {\
            gotstatus = false;\
            timeo = 0;\
            if ((timeout) >= 1)\
                timeo = (timeout) - 1;\
        }\
    }\
    if (!gotstatus)\
    {\
        if (timeo)\
        {\
            timeo = jiffies + msecs_to_jiffies(timeo);\
            do \
            {\
                msleep(1);\
                if (status)\
                {\
                     gotstatus = true;\
                     break;\
                }\
            } \
            while(time_after(timeo, jiffies));\
        }\
        if (!gotstatus)\
        {\
            printk("[Error] %s timeout\n", (timeoutstr));\
        }\
    }\
}

#define WAIT_FOR_ZERO(status, timeout, timeoutstr) \
{\
   unsigned long timeo = jiffies + msecs_to_jiffies(2);\
   bool gotstatus = true;\
   while (status)\
   {\
       if (time_after(jiffies, timeo))\
       {\
           gotstatus = false;\
           timeo = 0;\
           if ((timeout) >= 1)\
               timeo = (timeout) - 1;\
       }\
   }\
   if (!gotstatus)\
   {\
       if (timeo)\
       {\
           timeo = jiffies + msecs_to_jiffies(timeo);\
           do \
           {\
               msleep(1);\
               if (!(status))\
               {\
                    gotstatus = true;\
                    break;\
               }\
           } \
           while(time_after(timeo, jiffies));\
       }\
       if (!gotstatus)\
       {\
           printk("[Error] %s timeout\n", (timeoutstr));\
       }\
   }\
}

#define WAIT_FOR_STATUS_FLAG(status, timeout, timeoutstr, toflag) \
{\
    unsigned long timeo = jiffies + msecs_to_jiffies(2);\
    bool gotstatus = true;\
    toflag = 0;\
    while (!(status))\
    {\
        if (time_after(jiffies, timeo))\
        {\
            gotstatus = false;\
            timeo = 0;\
            if ((timeout) >= 1)\
                timeo = (timeout) - 1;\
        }\
    }\
    if (!gotstatus)\
    {\
        if (timeo)\
        {\
            timeo = jiffies + msecs_to_jiffies(timeo);\
            do \
            {\
                msleep(1);\
                if (status)\
                {\
                     gotstatus = true;\
                     break;\
                }\
            } \
            while(time_after(timeo, jiffies));\
        }\
        if (!gotstatus)\
        {\
            toflag = 1;\
            printk("[Error] %s timeout\n", (timeoutstr));\
        }\
    }\
}

#define WAIT_FOR_ZERO_FLAG(status, timeout, timeoutstr, toflag) \
{\
   unsigned long timeo = jiffies + msecs_to_jiffies(2);\
   bool gotstatus = true;\
   toflag = 0;\
   while (status)\
   {\
       if (time_after(jiffies, timeo))\
       {\
           gotstatus = false;\
           timeo = 0;\
           if ((timeout) >= 1)\
               timeo = (timeout) - 1;\
       }\
   }\
   if (!gotstatus)\
   {\
       if (timeo)\
       {\
           timeo = jiffies + msecs_to_jiffies(timeo);\
           do \
           {\
               msleep(1);\
               if (!(status))\
               {\
                    gotstatus = true;\
                    break;\
               }\
           } \
           while(time_after(timeo, jiffies));\
       }\
       if (!gotstatus)\
       {\
                       toflag = 1;\
           printk("[Error] %s timeout\n", (timeoutstr));\
       }\
   }\
}



#endif  // ATC_DRV_POLLING_H

