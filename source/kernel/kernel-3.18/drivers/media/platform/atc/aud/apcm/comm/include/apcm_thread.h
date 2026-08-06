
/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of AutoChips Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AutoChips SOFTWARE") RECEIVED
 *     FROM AutoChips AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. AutoChips EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES AutoChips PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE AutoChips SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. AutoChips SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY AutoChips SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND AutoChips'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE AutoChips SOFTWARE RELEASED HEREUNDER WILL BE, AT AutoChips'S OPTION,
 *     TO REVISE OR REPLACE THE AutoChips SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO AutoChips FOR SUCH AutoChips SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

/******************************************************************************
*[File]			apcm_thread.h
*[Author]		atc6013
*[Description]
*
******************************************************************************/
#ifndef __APCM_THREAD_H__
#define __APCM_THREAD_H__

#include "apcm_log.h"

#define APCM_WAIT_OBJECT_0		WAIT_OBJECT_0
#define APCM_WAIT_TIMEOUT		WAIT_TIMEOUT
#define APCM_INFINITE			(0xFFFFFFFFul)

#define apcm_event_wait_for_objects	x_event_wait_for_objects

typedef struct
{
	s32			wq_flag;
	wait_queue_head_t	wait_queue;
} apcm_event_t;


typedef struct
{
	char name[50];
	bool running;

	struct task_struct 	*task;
	s32			wq_flag;
	wait_queue_head_t	wait_queue;

} apcm_thread_t;


typedef s32 (*apcm_pfn_thread_func)(void *data);

//=================================================//

void *event_open(char *name, u32 idx);
void *event_close(void *this);

void event_wakeup(void *this);
u64 event_wait(void *this, u64 time);

//=================================================//

void thread_open(apcm_thread_t **thread, apcm_pfn_thread_func pfn, void *data, char *name);
void *thread_close(apcm_thread_t *this);

bool thread_should_stop(apcm_thread_t *this);
void thread_stop(apcm_thread_t *this);

void thread_wakeup(apcm_thread_t *this);
u64  thread_wait(apcm_thread_t *this, u64 time);

void thread_stop_wakeup(apcm_thread_t *this);
u64  thread_stop_wait(apcm_thread_t *this);



#endif // #ifndef __APCM_THREAD_H__

