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
#include<stdbool.h>
#include<stdio.h>

#include "async_queue.h"
#include "atcdtlog.h"

AsyncQueue *
async_queue_new (void)
{
  AsyncQueue *queue;

  queue = (AsyncQueue *)malloc(sizeof(AsyncQueue));
  if (queue == NULL)
    return NULL;

  memset(queue, 0, sizeof(AsyncQueue));

  pthread_cond_init(&queue->condition, NULL);
  pthread_mutex_init(&queue->mutex, NULL);
  queue->enabled = true;

  return queue;
}

void
async_queue_free (AsyncQueue * queue)
{
  pthread_cond_destroy(&queue->condition);
  pthread_mutex_destroy(&queue->mutex);
  free(queue);
}

void
async_queue_push (AsyncQueue * queue, void *data)
{
/*
	if (NULL != g_list_find(queue->head, data)) {
		GList *entry = queue->head;
		guint n = 0;
		g_print("[async_queue_push] error, the data(%p) has been in the queue(%p)\r\n", data, queue);
		while (entry != NULL) {
			g_print("[async_queue_push] data[%d]: %p\r\n", n, entry->data);
			n++;
			entry = entry->next;
	  }
	}
*/

  pthread_mutex_lock(&queue->mutex);
  if (queue->length < MAX_QSIZE) {
    queue->data[queue->m_write] = data;
    queue->m_write++;
    if (queue->m_write >= MAX_QSIZE) {
      queue->m_write = 0;
    }
    queue->length++;
    pthread_cond_signal(&queue->condition);
  } else {
    PRINT_ERROR(" queue is full!\r\n");
  }

  pthread_mutex_unlock (&queue->mutex);
}

void* async_queue_pop (AsyncQueue *queue)
{
  void *data = NULL;

  pthread_mutex_lock(&queue->mutex);
  if (!queue->enabled) {
      /* g_warning ("not enabled!"); */
      goto leave;
  }

  if (queue->length == 0) {
    pthread_cond_wait(&queue->condition, &queue->mutex);
  }

  if (queue->length > 0) {
    data = queue->data[queue->m_read];
    ++queue->m_read;
    --queue->length;
    if (queue->m_read >= MAX_QSIZE) {
      queue->m_read = 0;
    }
  }

leave:
  pthread_mutex_unlock(&queue->mutex);

  return data;
}

void* async_queue_pop_nb(AsyncQueue * queue)
{
  void *data = NULL;

  pthread_mutex_lock(&queue->mutex);
  if (queue->length > 0) {
    data = queue->data[queue->m_read];
    ++queue->m_read;
    --queue->length;
    if (queue->m_read >= MAX_QSIZE) {
      queue->m_read = 0;
    }
  }
  pthread_mutex_unlock(&queue->mutex);

  return data;
}

void
async_queue_disable (AsyncQueue * queue)
{
  pthread_mutex_lock(&queue->mutex);
  queue->enabled = false;
  pthread_cond_broadcast(&queue->condition);
  pthread_mutex_unlock(&queue->mutex);
}

void
async_queue_enable (AsyncQueue * queue)
{
  pthread_mutex_lock(&queue->mutex);
  queue->enabled = true;
  pthread_mutex_unlock(&queue->mutex);
}

void
async_queue_flush (AsyncQueue * queue)
{
  pthread_mutex_lock(&queue->mutex);
  queue->m_read = queue->m_write = 0;
  queue->length = 0;
  pthread_mutex_unlock(&queue->mutex);
}

unsigned
async_queue_length(AsyncQueue * queue)
{
  unsigned length = 0;
  pthread_mutex_lock(&queue->mutex);
  length = queue->length;
  pthread_mutex_unlock(&queue->mutex);
  return length;
}

