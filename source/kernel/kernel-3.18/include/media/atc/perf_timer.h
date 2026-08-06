#ifndef _PERF_TIMER_H_
#define _PERF_TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

#define REC_EACHSAMPLE_ENB 	0

#define PERF_PROFILING_ENB  0
#define PERF_HIGH_ACCURACY  0

typedef struct _PerfTimer PerfTimer; 

#define PERF_TIMER_SET_MEDIA_NAME	0X01

#define PERF_TIMER_DMUX_TIMER		0X02

#define PERF_TIMER_VDEC_TIMER		0X03

#define PERF_TIMER_DISP_TIMER		0X04

#define PERF_TIMER_STAT_TIMER		0X05

#define PERF_TIMER_SET_START_TIMER	0X06

typedef struct _Medis_Info_
{
	const char *lpFileName;
	double fps;
} Media_Info;

//Support these modules now
typedef enum {
	DMUX_MODULE,
	VDEC_MODULE,
	DISP_MODULE,
	TOTAL_MODULE
} PERF_MODULE;

void SetMedinfoForPerfTimer(const char *lpFileName, double fps);
PerfTimer *InitPerfTimer(PERF_MODULE name);
void StartPerfTimer(PerfTimer *pPerfTimer);
void StopPerfTimer(PerfTimer *pPerfTimer);
void DeinitPerfTimer(PerfTimer *pPerfTimer);

#ifdef __cplusplus
} 
#endif

#endif //_PERF_TIMER_H_
