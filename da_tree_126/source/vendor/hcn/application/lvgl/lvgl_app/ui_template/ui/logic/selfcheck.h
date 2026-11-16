#ifndef CHECKSELF_H_
#define CHECKSELF_H_

#include <stdbool.h>
#include <stdint.h>
typedef enum {
    CHECK_STATE_IDLE,
    CHECK_STATE_WAITING,
    CHECK_STATE_CHECKING,
    CHECK_STATE_FINISHED
}check_state_e;

void selfcheck_init() ;

int  checkself_get_state() ;

void checkself_set_state(check_state_e _state) ;

/// @brief 计算从自检开始，运行时长达到tick 毫秒
/// @param tick 毫秒
/// @return 
bool checkout_time_out(uint32_t tick);
#endif  // CHECKSELF_H_
