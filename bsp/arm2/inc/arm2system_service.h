/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * AutoChips Inc. (C) 2016. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */
#ifndef __ARM2SYSTEM_SERVICE_H_
#define __ARM2SYSTEM_SERVICE_H_
typedef enum arm2system_service_msg
{
	ARM2SYSTEM_SERVICE_INV = -1,
    ARM2SYSTEM_SERVICE_HEARTBEAT = 0,
    ARM2SYSTEM_SERVICE_KERNEL_PANIC,
    ARM2SYSTEM_SERVICE_HEARTBEAT_START,
    ARM2SYSTEM_SERVICE_REBOOT,
    ARM2SYSTEM_SERVICE_AWTK_START,
    ARM2SYSTEM_SERVICE_DISPLAY_VSYNC,
	ARM2SYSTEM_SERVICE_SHUTDOWN,
	ARM2SYSTEM_SERVICE_MAX,
}arm2system_service_msg_t;

typedef int (*arm2system_service_func)(void *args);
typedef enum 
{
	STATUS_INV = -1,
	STATUS_ANI_ON = 0,
	STATUS_ANI_OFF,
	STATUS_ANI_STOP,
	STATUS_ANI_END,
	STATUS_MAX,
}arm2system_service_status_t;

typedef struct 
{
	arm2system_service_func  handler_status[STATUS_MAX];
	arm2system_service_func  handler_msg[STATUS_MAX];
	void * args_status[STATUS_MAX];
	void * args_msg[ARM2SYSTEM_SERVICE_MAX];
}arm2system_service_manger_t;

#define TASK_IDLE    (0x1)
#define TASK_BUSY    (0x2)
int arm2system_service_status_handler_register(arm2system_service_func handler, int status, void *arg);
int arm2system_service_msg_handler_register(arm2system_service_func handler, int msg_type, void *arg);
unsigned int arm2system_service_checkheartbeat(void);
unsigned int arm2system_service_keep_heart(void);
unsigned int arm2system_service_statemach();
unsigned int arm2system_service_callback(unsigned int u4ModuleID, unsigned int u4Param1, unsigned int u4Param2, unsigned int u4Param3);
unsigned int arm2system_service_init();


#endif
