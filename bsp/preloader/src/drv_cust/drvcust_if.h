
#ifndef DRVCUST_IF_H
#define DRVCUST_IF_H

//-----------------------------------------------------------------------------
// Include files
//-----------------------------------------------------------------------------
#include "drv_config.h"
#include "c_model.h"

// DDR type definitions
#define DDR_III_16_x1   0
#define DDR_III_8_x2    1
#define DDR_III_8_x1    2


//#if CONFIG_DRV_DRAM_SETTING_VERSION == 0
#include "ac8317_m1v1_v00.h"
//#endif
#include "drv_default.h"

#define DRVCUST_InitGet(x)          (x)
#define eDdrClock                   (DEFAULT_DDR_CLOCK)
#define eDramType                   (DEFAULT_DRAM_TYPE)
#define eDramColAddr                (DEFAULT_DRAM_COLADDR)
#define eDdrCL                      (DEFAULT_DDR_CL)
#define eDdrBusX8                   (DEFAULT_DDR_BUS_X8)
#define eFlagDDRQfp                 (FLAG_DDR_QFP)
#define eFlagDDR16BitSwap           (FLAG_DDR_16BITSWAP)
#define eFlagDDRDCBalance			(FLAG_DDR_DCBALANCE)
#define eFlag1GBitSupport           (DEFAULT_DRAM_8_BANKS)
#define eFlagReadODT                (DEFAULT_DRAM_RODT)
#define eFlagWriteODT               (DEFAULT_DRAM_WODT)
#define eDmpllSpectrumPermillage    (DMPLL_SPECTRUM_PERMILLAGE)
#define eDmpllSpectrumFrequency     (DMPLL_SPECTRUM_FREQUENCY)

#endif /*DRVCUST_IF_H */

