#include "awtk.h"

BEGIN_C_DECLS
#ifdef AWTK_WEB
#include "assets.inc"
#else /*AWTK_WEB*/
#include "../res/assets.inc"
#endif /*AWTK_WEB*/
END_C_DECLS

extern ret_t application_init(void);

extern ret_t application_exit(void);

#define APP_LCD_ORIENTATION LCD_ORIENTATION_0
#define APP_TYPE APP_SIMULATOR
#define APP_NAME "awtk-demo5"

#if defined(LINUX) || defined(APPLE) || defined(HAS_STDIO) || defined(WINDOWS)
#define APP_RES_ROOT "/usr/lib/awtk/image/"
#endif

#include "awtk_main.inc"
