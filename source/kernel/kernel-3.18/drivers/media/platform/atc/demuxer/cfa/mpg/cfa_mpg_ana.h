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



#ifndef CFA_MPG_ANA_H
#define CFA_MPG_ANA_H
#include "x_typedef.h"
#include "cfa_mpg.h"

/* MPG CFA state control for transfer done*/
/*@return None*/
/* @note 1. This function will be called after a transfer is complete.*/
/*	  2. This function is not protected by semphore. Before/after calling this function, semaphore should be used*/
/*< [IN] handle of fdmx*/
/*< [IN] Actual transferred data length.
	Normally this value should be equal to the u4Len in the previous transfer issue,
	unless file end is hit.*/
/*< [IN] pointer to CfaMpgInst*/
EXTERN void CfaMpgTxDoneStCtrl(void *pvSptHdl, u64 u8TxLen, CfaMpgInst *prCfaMpg);

#endif				/* CFA_MPG_ANA_H*/
