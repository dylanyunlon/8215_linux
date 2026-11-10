# 对 HCN application 已有文件的改动（仅列出新增/删除的行）

完整补丁 `integration.patch` 含约 90 行 HCN 源码上下文，不放进公开仓库；它随 `hcn_music_lvgl.tar.gz` 分发。
下面只列我新增的行，供评审和对照；`<anchor>` 表示插入位置紧跟在对应原有代码之后，具体位置看补丁。


## `CMakeLists.txt`


```diff
+#   HCN_APP_MUSIC    本地音乐（adapters/music + screens/ui_Music + frameworks/music_focus）
```
```diff
+option(HCN_APP_MUSIC "Enable local music (adapters/music + ui_Music + music_focus; needs musicplayer package)" "${HCN_APP_CFG_MUSIC}")
```
```diff
+message(STATUS "HCN_APP_MUSIC=${HCN_APP_MUSIC}")
```
```diff
+# ---- 本地音乐：music_app 业务层原样复用 + AWTK 原语替换层（顺序：compat 在前）----
+if(HCN_APP_MUSIC)
+    add_subdirectory(adapters/music)
+    # ui_Music.c / music_focus.c 在 lvgl_app 里编译，需要 music_app.h 及 compat/awtk.h
+    target_include_directories(lvgl_app PUBLIC ${HCN_MUSIC_INC_DIRS})
+    target_compile_definitions(lvgl_app PUBLIC HCN_APP_HAVE_MUSIC)
+endif()
+
```
```diff
-foreach(_hcn_optional_target lvgl_ui_template lvgl_bt lvgl_carplay)
+foreach(_hcn_optional_target lvgl_ui_template lvgl_bt lvgl_carplay lvgl_music)
```
```diff
+if(TARGET lvgl_music)
+    list(APPEND _hcn_ui_link_libs lvgl_music)   # 带出 musicplayer(libmusicplayer.so) + pthread
+endif()

## `hcn_app_config.cmake`
```
```diff
+# ---- 本地音乐（adapters/music + ui_Music + music_focus） ----
+# ON : 编入本地音乐（mk 自动给包加 musicplayer 依赖并注入 HCN_APP_MUSIC_SRC；
+#      字体 music_font_20/30 约 1.5 MB flash —— nor-64 空间敏感，按需开启）
+# OFF: 全部剔除
+set(HCN_APP_CFG_MUSIC ON)
+

## `lvgl/lvgl_app/CMakeLists.txt`
```
```diff
+# 本地音乐屏 + 音频焦点桥 + 生成的 CJK 字体（HCN_APP_MUSIC 顶层开关，cache 变量在父作用域声明）：
+# 适配层本体在 application/adapters/music，include 路径由顶层 CMakeLists 统一注入 lvgl_app
+if(HCN_APP_MUSIC)
+    list(APPEND SOURCES
+        screens/ui_Music.c
+        frameworks/music_focus.c
+        fonts/music_font_20.c
+        fonts/music_font_32.c)
+endif()
+

## `lvgl/lvgl_app/frameworks/ui_shared_apps.c`
```
```diff
+#ifdef HCN_APP_HAVE_MUSIC
+#include "music_adapter.h"
+#include "frameworks/music_focus.h"
+#include "screens/ui_Music.h"
+#include <stdio.h>
+#endif
+
```
```diff
+#ifdef HCN_APP_HAVE_MUSIC
+    const app_desc_t music_desc = {
+        .app_id = APP_MUSIC,
+        .priority = APP_PRIORITY_LOW,
+        .name = "Music",
+        .init_cb = ui_Music_screen_init,
+        .destroy_cb = ui_Music_screen_destroy,
+        .screen_policy = APP_SCREEN_PERSISTENT,
+        .on_enter = ui_Music_refresh,
+    };
+    ret = displayfocusmanager_register_app(&music_desc);
+    if (ret != APP_OK) {
+        return ret;
+    }
+#endif
+
```
```diff
+#ifdef HCN_APP_HAVE_MUSIC
+    /* hooks run on the GUI thread, in this order: audio focus first, then the screen */
+    music_focus_init();
+    music_adapter_add_event_hook(music_focus_on_app_event);
+    music_adapter_add_event_hook(ui_Music_on_app_event);
+    if (music_adapter_init() != 0) {
+        fprintf(stderr, "[ui] music adapter init failed, music screen stays empty\n");
+    }
+#endif
```
```diff
+void hcn_ui_shared_apps_open_music(void)
+{
+#ifdef HCN_APP_HAVE_MUSIC
+    displayfocusmanager_open_app(APP_MUSIC);
+#endif
+}
+
+#ifdef HCN_APP_HAVE_MUSIC
+    music_adapter_deinit();
+    music_adapter_clear_event_hooks();
+    music_focus_deinit();
+#endif

## `lvgl/lvgl_app/frameworks/ui_shared_apps.h`
```
```diff
+void hcn_ui_shared_apps_open_music(void);   /* no-op unless built with HCN_APP_MUSIC */

## `lvgl/main.c`
```
```diff
+/* 本地音乐适配层（HCN_APP_HAVE_MUSIC 由顶层 CMake HCN_APP_MUSIC 注入）：
+ * 主循环协作轮询 music_adapter_poll —— music_app 的 idle_queue/timer 回调都在这里的 GUI 线程执行 */
+#ifdef HCN_APP_HAVE_MUSIC
+#include "music_adapter.h"
+#endif
+
```
```diff
+#ifdef HCN_APP_HAVE_MUSIC
+    /* 本地音乐：USB/扫描/播放线程投递的事件与定时器在 GUI 线程派发 */
+    music_adapter_poll();
+#endif
```
