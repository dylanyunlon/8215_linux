#ifndef _EC_LINK_H_
#define _EC_LINK_H_ 

#include "dev_config.h"

#if MW_CARLINK_EC_ENABLE
void start_wifi_ap();
void stop_carlink_video();
void restart_carlink_video();
#endif

#endif