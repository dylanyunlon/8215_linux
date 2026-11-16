/**
 * @file awtk.h  --  COMPATIBILITY SHIM, this is NOT the real AWTK header.
 *
 * Purpose: let the unmodified music_app.c / favorite_manager.c (the music
 * application layer written against AWTK) build inside the LVGL-based
 * hcn_application without touching their source.  music_app.c only uses
 * these AWTK primitives:
 *
 *     idle_queue()    marshal a callback onto the GUI thread (any thread -> GUI)
 *     timer_add()     one-shot / repeating timer, fires on the GUI thread
 *     timer_remove()
 *     ret_t / RET_*   callback return codes
 *
 * They are re-implemented in music_mainloop.c on top of a mutex-protected
 * queue that is drained by music_mainloop_poll().  Call that once per main-loop
 * iteration (same place as bt_app_poll()).  LV_USE_OS is LV_OS_NONE in this
 * product, so every LVGL call must stay on the GUI thread; this shim is what
 * guarantees music_app's UI callbacks run there.
 *
 * How it is picked up: music_app.h does `#include "awtk.h"` (when AWTK_STUB is NOT defined).
 * Put this directory on the include path of lvgl_music and of everything that includes
 * music_app.h.  Do NOT define AWTK_STUB: then music_app.h asks for "awtk_stub.h", and a quoted
 * include searches the includer's own directory FIRST, where the upstream thread-based
 * awtk_stub.h sits, so no -I order can win.  That stub runs callbacks on worker threads, which
 * is fatal with LV_OS_NONE.  music_adapter.c has a canary (#error) against exactly this.
 */
#ifndef HCN_MUSIC_COMPAT_AWTK_H
#define HCN_MUSIC_COMPAT_AWTK_H

/* canary: music_adapter.c refuses to build if it did not get this header */
#define HCN_MUSIC_SHIM 1

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int ret_t;
#define RET_OK      0
#define RET_FAIL    1
#define RET_REMOVE  2   /* one-shot: remove after this call            */
#define RET_REPEAT  3   /* keep: idle -> run again next poll, timer -> re-arm */

#define TK_INVALID_ID 0
#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

typedef struct _idle_info_t  { void *ctx; } idle_info_t;
typedef struct _timer_info_t { void *ctx; uint32_t id; } timer_info_t;

typedef ret_t (*idle_func_t)(const idle_info_t *idle);
typedef ret_t (*timer_func_t)(const timer_info_t *timer);

/** Thread-safe.  Callback runs later on the GUI thread inside music_mainloop_poll(). */
ret_t    idle_queue(idle_func_t func, void *ctx);
/** Returns a non-zero id, or TK_INVALID_ID on failure.  Thread-safe. */
uint32_t timer_add(timer_func_t func, void *ctx, uint32_t interval_ms);
ret_t    timer_remove(uint32_t id);

/** Drain queued idles and fire due timers.  GUI thread only. */
void music_mainloop_poll(void);
/** Drop everything still queued (shutdown). */
void music_mainloop_deinit(void);
/** Number of idles still waiting (diagnostics / tests). */
int  music_mainloop_pending(void);

#ifdef __cplusplus
}
#endif
#endif /* HCN_MUSIC_COMPAT_AWTK_H */
