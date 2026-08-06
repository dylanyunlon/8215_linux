/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of AutoChips Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE") RECEIVED
 *     FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. AUTOCHIPS SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE AUTOCHIPS SOFTWARE RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION,
 *     TO REVISE OR REPLACE THE AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#ifndef __AC83XX_KEYADC_H
#define __AC83XX_KEYADC_H

#include <asm/arch/x_typedef.h>

//============================================================================
// Public functions
//============================================================================

/**
 * @Description:  
 *  This function get called only in case of checking entrance of recovery mode 
 *  The checking way is to read data channel to see if there is any key
 *  pressed for a while. 
 *
 * @Return
 *  true if any key is pressed
 *  false if not
 *
 */
extern BOOL check_key_pressing(void);
extern void keyadc_init(void);
extern void keyadc_exit(void);

#endif  // __AC83XX_KEYADC_H

