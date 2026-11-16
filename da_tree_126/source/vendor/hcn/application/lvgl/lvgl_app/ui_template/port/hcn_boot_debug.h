#ifndef HCN_BOOT_DEBUG_H
#define HCN_BOOT_DEBUG_H

/* Boot-flow switches. This is the single definition; main.c, the PC entry,
 * selfcheck.c and custom.c all include this header. */

/* 1 = enter the DVR page after startup; 0 = normal boot flow. */
#ifndef HCN_BOOT_INTO_DVR
#define HCN_BOOT_INTO_DVR 0
#endif

/* DVR entry timing (only when HCN_BOOT_INTO_DVR=1):
 *   0 = after selfcheck completes;
 *   1 = immediately after ui_backend_init(). */
#ifndef HCN_BOOT_DVR_ENTRY_EARLY
#define HCN_BOOT_DVR_ENTRY_EARLY 0
#endif

/* 1 = skip the boot animation; custom.c must force ANIMATION_STATE_END. */
#ifndef HCN_BOOT_SKIP_BOOT_ANIM
#define HCN_BOOT_SKIP_BOOT_ANIM 0
#endif

/* 1 = set selfcheck to FINISHED immediately; custom.c must also move the
 * cartoon to CARTOON_STATE_READY. */
#ifndef HCN_BOOT_SKIP_SELFCHECK
#define HCN_BOOT_SKIP_SELFCHECK 0
#endif

#endif /* HCN_BOOT_DEBUG_H */
