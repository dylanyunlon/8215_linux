
#ifndef __DEV_STATE_H__
#define __DEV_STATE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum {
    CHECK_SELF_STATE_INIT = 0,
    CHECK_SELF_STATE_START,
    CHECK_SELF_STATE_SUCCESS,
} check_self_state_e;

typedef enum {
    ANIMATION_STATE_IDLE = 0,
    ANIMATION_STATE_RUNNING,
    ANIMATION_STATE_END    ,
} animation_state_e;

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // __DEV_STATE_H__