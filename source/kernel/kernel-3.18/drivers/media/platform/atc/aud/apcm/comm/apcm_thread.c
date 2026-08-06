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
*[File]				apcm_thread.c
*[Author]			atc6013
*[Description]
*
******************************************************************************/
#include "apcm_thread.h"
#include "aud_if.h"

#define LOG_TAG  "[Thread]"

#define THREAD_WQ_WAIT		0
#define THREAD_WQ_WAKEUP	1


void *event_open(char *name, u32 idx)
{
	void *this = NULL;
	char event_name[50];

	sprintf(event_name, "%s_%d", name, idx);
	this = x_event_create(NULL, false, false, event_name);

	return (this);
}


void *event_close(void *this)
{
	if (this) {
		x_event_destroy(this);
	}
	return (NULL);
}


void event_wakeup(void *this)
{
	if (this) {
		x_event_set(this);
	}
}


u64 event_wait(void *this, u64 time)
{
	u64 timeout = 0;
	if (this) {
		timeout = x_event_wait_for_objects(1, &this, false, time);
	}
	return (timeout);
}


//====================================================//


void thread_open(apcm_thread_t **thread, apcm_pfn_thread_func pfn, void *data, char *name)
{
	apcm_thread_t *this = (apcm_thread_t *)apcm_mem_alloc(sizeof(apcm_thread_t));
	if (this && thread)
	{
		*thread = this;
		sprintf(this->name, "%s", name);
		this->task = kthread_create(pfn, data, name);

		if (!IS_ERR(this->task)) {
			PR_D("[open] Create '%s' success (0x%p)!\r\n", name, this);
			this->wq_flag = 0;
			init_waitqueue_head(&this->wait_queue);

			this->running = true;
			wake_up_process(this->task);
		} else {
			PR_E("[open] Create event or task ERR '%s'!\r\n", name);
			thread_close(this);
			*thread = NULL;
		}
	} else {
		PR_E("[open] Create this class fail!  (%s)\r\n", name);
	}
}


void *thread_close(apcm_thread_t *this)
{
	if (this) {
		PR_D("[close] (%s, 0x%p)", this->name, this);
		this->running = false;
		apcm_mem_free(this);
	}

	return (NULL);
}


bool thread_should_stop(apcm_thread_t *this)
{
	bool running = this ? this->running : false;
	if (running == false && this) {
		PR_D("[should stop] (%s, 0x%p) \n", this->name, this);
	}

	return (running == false);
}


void thread_stop(apcm_thread_t *this)
{
	if (this) {
		PR_D("[stop] (%s, 0x%p) \n", this->name, this);
		this->running = false;
		thread_wakeup(this);
	}
}


void thread_wakeup(apcm_thread_t *this)
{
	if (this) {
		PR_D4("[wakeup] (%s, 0x%p) \n", this->name, this);
		this->wq_flag = THREAD_WQ_WAKEUP;
		wake_up_interruptible(&this->wait_queue);
	}
}


u64 thread_wait(apcm_thread_t *this, u64 time)
{
	u64 timeout = 0;
	if (this) {
		this->wq_flag = THREAD_WQ_WAIT;
		if (time == APCM_INFINITE) {
			PR_D4("[wait] (%s, 0x%p) \n", this->name, this);
			timeout = wait_event_interruptible(this->wait_queue, this->wq_flag);
		} else {
			timeout = wait_event_interruptible_timeout(this->wait_queue, this->wq_flag, time);
		}
	}
	return (timeout);
}


