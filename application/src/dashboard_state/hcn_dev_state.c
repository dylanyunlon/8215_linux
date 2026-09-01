/**
*
* @file hcn_dev_state.c
*
* @brief This message displayed in Doxygen Files index
*
* @ingroup PackageName
* (note: this needs exactly one @defgroup somewhere)
*
* @date	2025/09/25 09:06
* @author och
*
*/

#include "hcn_dev_state.h"

typedef struct hcn_dev_state
{
    int text ;
}TimerHandle_t;



static check_self_state_e current_state = CHECK_SELF_STATE_INIT;

void set_check_self_state(check_self_state_e state) {
    current_state = state;
}

check_self_state_e get_check_self_state(void) {
    return current_state;
}

animation_state_e get_boot_animation_status(void) {
    return 0 ;
}


static void usr_timer_entry(TimerHandle_t xTimer) {
    return ;
}

void dev_state_init(void) {

}