/*****************************************************************************

File Name        :  hmi_reset.h
Organization    :  Zhuli Electronics Co.Ltd in Shanghai (www.shzldz.com)
******************************************************************************/

#ifndef _HMI_RESET_H
#define _HMI_RESET_H

#define	HMI_ALLWAYS_COLOD_ENABLE		HMI_YES
void hmi_gfx_cold_init(void);
void hmi_gfx_warm_init(void);
#ifdef __cplusplus
extern "C"{
#endif
void hmi_init(void);
#ifdef __cplusplus
}
#endif

#endif


