/*
 * mt6630_reg.h - mt6630 register definition
 *
 * Copyright (c) 2018 AutoChips Inc.
 * Author: Rocky Pan <changle.pan@autochips.com>
 *
 * This file is released under the GPLv2
 *
 */

#ifndef _MT6630_REG_H
#define _MT6630_REG_H

/* MT6630 MCR Definition */

/* 4 Test Mode Data Port */
#define MCR_WTMDR                           0x00b0

/* 4 Test Mode Control Register */
#define MCR_WTMCR                           0x00b4

/* 4 Test Mode Data Pattern Control Register #0 */
#define MCR_WTMDPCR0                        0x00b8

/* 4 Test Mode Data Pattern Control Register #1 */
#define MCR_WTMDPCR1                        0x00bc

/* Definition in each register */

#ifndef BIT
	#define BIT(n) (1U << (n))
#endif

#ifndef BITS
	/* bits range: for example BITS(16,23) = 0xFF0000
	 *   ==>  (BIT(m)-1)   =  0x0000FFFF
	 *       ~(BIT(m)-1)   => 0xFFFF0000
	 *   ==>  (BIT(n+1)-1) =  0x00FFFFFF
	 */
	#define BITS(m, n) (~(BIT(m)-1) & ((BIT(n) - 1) | BIT(n)))
#endif

/* WTMCR 0x00b4 */
#define WMTCR_TEST_MODE_FW_OWN              BIT(24)
#define WMTCR_PRBS_INIT_VAL                 BITS(16, 23)
#define WMTCR_TEST_MODE_STATUS              BIT(8)
#define WMTCR_TEST_MODE_SELECT              BITS(0, 1)

#endif /* _MT6628_REG_H */
