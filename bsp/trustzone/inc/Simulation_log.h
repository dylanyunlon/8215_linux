#ifndef _SIM_LOG_H_
#define _SIM_LOG_H_

#include "x_typedef.h"

#define SI_RLOG_TYPE_DWRD                       (0x01)
#define SI_RLOG_TYPE_STR                        (0x02)

#if (SIMULATION_LOG)
#define vSimTraceDWRD(dVal)                 bSendLog_ROREG(SI_RLOG_TYPE_DWRD, (DWRD)(dVal))
#define vSimTraceStr(pStr)                  bSendLog_ROREG(SI_RLOG_TYPE_STR, (DWRD)(pStr))  
extern void SIM_Printf(const CHAR *format,...);
extern BOOL bSendLog_ROREG(UINT8 bType, UINT32 dPara);
#define SIM_PRINTF(fmt...)      SIM_Printf(fmt)

#else
#define vSimTraceDWRD(dVal)
#define vSimTraceStr(pStr)
#define SIM_PRINTF(fmt...)
#endif


#endif
