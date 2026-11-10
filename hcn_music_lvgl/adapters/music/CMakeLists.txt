# =====================================================================
#  adapters/music — 本地音乐应用层（HCN_APP_MUSIC）
#
#  music_app.c / favorite_manager.c / usb_monitor.c 从 musicplayer 包 **原样复用**（零改动，
#  那里是 U 盘热插拔/扫描/播放/收藏/歌词/封面的全部业务逻辑，已有大量问题修复）。
#  它们只用到 AWTK 的 idle_queue/timer_add/timer_remove 三个原语，由 include/compat/awtk.h
#  + src/music_mainloop.c 在主循环上重新实现（见 awtk.h 头注释：为什么不能定义 AWTK_STUB）。
#
#  源码位置：source/packages/application/musicplayer。application 在 buildroot 里是整树 rsync
#  到构建目录再编译的，拷贝树里相对路径失效，hcn-application.mk 必须注入绝对路径：
#      -DHCN_APP_MUSIC_SRC=$(TOPDIR)/../source/packages/application/musicplayer
#  并依赖 musicplayer 包（libmusicplayer.so：解码/ALSA 输出/扫描/DB/封面模糊/stb_image 实现）。
#
#  导出：target lvgl_music；父作用域变量 HCN_MUSIC_INC_DIRS（给 lvgl_app 用，顺序有意义：
#  compat 必须排在 awtk_app/src 之前）。
# =====================================================================
set(HCN_APP_MUSIC_SRC "${PROJECT_SOURCE_DIR}/../../../packages/application/musicplayer" CACHE PATH
    "musicplayer package source dir; injected by hcn-application.mk for buildroot builds")
set(_music_app_src "${HCN_APP_MUSIC_SRC}/awtk_app/src")

foreach(_f music_app.c music_app.h favorite_manager.c usb_monitor.c darray.h stb_image.h)
    if(NOT EXISTS "${_music_app_src}/${_f}")
        message(FATAL_ERROR
            "musicplayer source not found: ${_music_app_src}/${_f}\n"
            "buildroot 构建需在 hcn-application.mk 传 -DHCN_APP_MUSIC_SRC=<source>/packages/application/musicplayer；\n"
            "改过 CONF_OPTS 后必须 reconfigure 或 dirclean，-rebuild 不会重跑 configure。")
    endif()
endforeach()

add_library(lvgl_music STATIC
    src/music_mainloop.c
    src/music_adapter.c
    ${_music_app_src}/music_app.c
    ${_music_app_src}/favorite_manager.c
    ${_music_app_src}/usb_monitor.c)

set(_music_inc
    ${CMAKE_CURRENT_SOURCE_DIR}/include/compat      # 必须在 awtk_app/src 之前
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${_music_app_src}
    ${_music_app_src}/cmus
    ${HCN_APP_MUSIC_SRC})
target_include_directories(lvgl_music PUBLIC ${_music_inc})
set(HCN_MUSIC_INC_DIRS ${_music_inc} PARENT_SCOPE)

target_compile_definitions(lvgl_music PRIVATE _GNU_SOURCE)
# AWTK_STUB 绝不能定义：music_app.h 会改去包含上游基于线程的 awtk_stub.h（见 compat/awtk.h）
target_link_libraries(lvgl_music PUBLIC musicplayer pthread)

install(TARGETS lvgl_music ARCHIVE DESTINATION lib)
