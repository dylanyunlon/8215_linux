/*
* Copyright (C) 2016 MediaTek Inc.
*
* This program is free software: you can redistribute it and/or modify it under the terms of the
* GNU General Public License version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with this program.
* If not, see <http://www.gnu.org/licenses/>.
*/

/*
** Id: //Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3/include/queue.h#1
*/

/*
 * ! \file   queue.h
 * \brief  Definition for singly queue operations.
 *
 *  In this file we define the singly queue data structure and its
 * queue operation MACROs.
 */

#ifndef _QUEUE_H
#define _QUEUE_H

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#include "gl_typedef.h"

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/
/* Singly Queue Structures - Entry Part */
typedef struct _QUE_ENTRY_T {
	struct _QUE_ENTRY_T *prNext;
	struct _QUE_ENTRY_T *prPrev;	/* For Rx buffer reordering used only */
} QUE_ENTRY_T, *P_QUE_ENTRY_T;

/* Singly Queue Structures - Queue Part */
typedef struct _QUE_T {
	P_QUE_ENTRY_T prHead;
	P_QUE_ENTRY_T prTail;
	UINT_32 u4NumElem;
} QUE_T, *P_QUE_T;

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/
#define MAXNUM_TDLS_PEER            4U

#define QUEUE_INITIALIZE(_prQueue) \
	{ \
	    (_prQueue)->prHead = (P_QUE_ENTRY_T)NULL; \
	    (_prQueue)->prTail = (P_QUE_ENTRY_T)NULL; \
	    (_prQueue)->u4NumElem = 0; \
	}

#define QUEUE_IS_EMPTY(_prQueue) \
	(((P_QUE_T)(_prQueue))->prHead == (P_QUE_ENTRY_T)NULL)

#define QUEUE_IS_NOT_EMPTY(_prQueue)         ((_prQueue)->u4NumElem > 0U)

#define QUEUE_GET_HEAD(_prQueue)             ((_prQueue)->prHead)

#define QUEUE_GET_TAIL(_prQueue)             ((_prQueue)->prTail)

#define QUEUE_GET_NEXT_ENTRY(_prQueueEntry)  ((_prQueueEntry)->prNext)

#define QUEUE_INSERT_HEAD(_prQueue, _prQueueEntry) \
	{ \
		ASSERT(_prQueue); \
		ASSERT(_prQueueEntry); \
		(_prQueueEntry)->prNext = (_prQueue)->prHead; \
		(_prQueue)->prHead = (_prQueueEntry); \
		if ((_prQueue)->prTail == (P_QUE_ENTRY_T)NULL) { \
		(_prQueue)->prTail = (_prQueueEntry); \
		} \
		((_prQueue)->u4NumElem)++; \
	}

#define QUEUE_INSERT_TAIL(_prQueue, _prQueueEntry) \
	{ \
		INT_32 i = 0, j = 0; \
		P_QUE_ENTRY_T p = (_prQueue)->prHead; \
		ASSERT(_prQueue); \
		ASSERT(_prQueueEntry); \
		for (i = 0; (i < (_prQueue)->u4NumElem) && p != NULL; i++) { \
			if (_prQueueEntry == p) { \
				j = 1; \
				break; \
			} \
			p = p->prNext; \
		} \
		if (j == 0) { \
		(_prQueueEntry)->prNext \
		= (P_QUE_ENTRY_T)NULL; \
		if ((_prQueue)->prTail \
		!= NULL) { \
		((_prQueue)->prTail)->prNext \
		= (_prQueueEntry); \
		} else { \
		(_prQueue)->prHead \
		= (_prQueueEntry); \
		} \
		(_prQueue)->prTail \
		= (_prQueueEntry); \
		((_prQueue)->u4NumElem)++; \
		} \
	}

/* NOTE: We assume the queue entry located at the beginning of "prQueueEntry Type",
 * so that we can cast the queue entry to other data type without doubts.
 * And this macro also decrease the total entry count at the same time.
 */
#define QUEUE_REMOVE_HEAD(_prQueue, _prQueueEntry, _P_TYPE) \
	{ \
		ASSERT(_prQueue); \
		(_prQueueEntry) = (_P_TYPE)((_prQueue)->prHead); \
		if ((_prQueueEntry) != NULL) { \
			(_prQueue)->prHead = \
			((P_QUE_ENTRY_T)(_prQueueEntry))->prNext; \
			if ((_prQueue)->prHead == NULL) { \
				(_prQueue)->prTail = NULL; \
			} \
			((P_QUE_ENTRY_T)(_prQueueEntry))->prNext \
			= NULL; \
			((_prQueue)->u4NumElem)--; \
		} \
	}

#define QUEUE_MOVE_ALL(prDestQueue, prSrcQueue) \
	{ \
		ASSERT(prDestQueue); \
		ASSERT(prSrcQueue); \
	    *(P_QUE_T)(prDestQueue) = *(P_QUE_T)prSrcQueue; \
	    QUEUE_INITIALIZE(prSrcQueue); \
	}

#define QUEUE_CONCATENATE_QUEUES(prDestQueue, prSrcQueue) \
	{ \
		ASSERT(prDestQueue); \
		ASSERT(prSrcQueue); \
		if ((prSrcQueue)->u4NumElem > 0U) { \
			if ((prDestQueue)->prTail != NULL) { \
				((prDestQueue)->prTail)->prNext = (prSrcQueue)->prHead; \
			} else { \
				(prDestQueue)->prHead = (prSrcQueue)->prHead; \
			} \
			(prDestQueue)->prTail = (prSrcQueue)->prTail; \
			((prDestQueue)->u4NumElem) += ((prSrcQueue)->u4NumElem); \
			QUEUE_INITIALIZE(prSrcQueue); \
	    } \
	}

#define QUEUE_CONCATENATE_QUEUES_HEAD(prDestQueue, prSrcQueue) \
	{ \
		ASSERT(prDestQueue); \
		ASSERT(prSrcQueue); \
		if ((prSrcQueue)->u4NumElem > 0U) { \
			((prSrcQueue)->prTail)->prNext = (prDestQueue)->prHead; \
			(prDestQueue)->prHead = (prSrcQueue)->prHead; \
			((prDestQueue)->u4NumElem) += ((prSrcQueue)->u4NumElem); \
			if ((prDestQueue)->prTail == NULL) {                 \
				(prDestQueue)->prTail = (prSrcQueue)->prTail;      \
			}  \
			QUEUE_INITIALIZE(prSrcQueue); \
		} \
	}

/*******************************************************************************
*                            E X T E R N A L  D A T A
********************************************************************************
*/
extern UINT_8 g_arTdlsLink[MAXNUM_TDLS_PEER];

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

#endif /* _QUEUE_H */
