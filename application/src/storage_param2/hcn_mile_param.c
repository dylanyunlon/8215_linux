/**
*
* @file hcn_mile_param.c
*
* @brief This message displayed in Doxygen Files index
*
* @ingroup PackageName
* (note: this needs exactly one @defgroup somewhere)
*
* @date	2025/09/01 12:23
* @author och
*
*/

#include <string.h>
#include "hcn_mile_param.h"

#define DEBUG_MILE_PARAM_ENABLE

#define MILEAGE_PARAM_START_ADDR (0x00)
#define OTHER_PRAM_START_ADDR  (0x280)
#define MILEAGE_PARAM_MAX_SIZE  (640)   ///< 里程参数最大区域为640个字节
#define MILEAGE_PARAM_PAGE_NUM  (40)    ///< 40页

static mile_param_t mile_param;
static mile_param_t mile_param_pre = {
    0
};

static bool is_recovery_mile_param = false;
static uint16_t g_mile_write_addr = MILEAGE_PARAM_START_ADDR;

bool get_recovery_mile_param(void) {
    return is_recovery_mile_param;
}

bool set_hcn_mile_param(mile_param_handle_e id, void *param) {
    

    return 0;
}

bool get_hcn_mile_param(mile_param_handle_e id, void *param) {
   
    return 0;
}

static uint8_t get_page_num(void) {
   

    return 0;
}

static void printf_mile_param_info(mile_param_t *param) {
  
}

int save_mile_param(void) {
   

    return 0;
}   

static int read_mile_param(void) {
 
    return 0;
}

static char check_mile_param(void) {
   return 1 ;
}

void mile_param_init(void) {
    return ;
}
