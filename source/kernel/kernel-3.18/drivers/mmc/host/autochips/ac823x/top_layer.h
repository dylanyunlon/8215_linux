#ifndef _TOP_LAYER_H_
#define _TOP_LAYER_H_
#include "mt_sd.h"

extern void msdc_dma_channel_sel(struct msdc_host *host,unsigned int channel_sel);
extern void msdc_clock_choose(struct msdc_host *host,unsigned int sclk,unsigned int hclk);

extern void msdc_clock_enable(struct msdc_host *host,unsigned int enable);
extern void msdc_sw_reset(struct msdc_host *host);

extern void msdc_multip_function(struct msdc_host *host,unsigned int share_nand_pin);
extern void msdc_init_pad(struct msdc_host *host);
extern void MSDC_Init_cmd_dat_pull_up_down(struct msdc_host *host, unsigned int mode);
void msdc_dump_pad(struct msdc_host *host);

#endif 


