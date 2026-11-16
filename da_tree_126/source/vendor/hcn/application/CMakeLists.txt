# =====================================================================
#  hcn_application — HCN 应用根
#
#  目录结构：
#   lvgl/        UI 模块（main.c + lvgl_app：装配/屏/框架/资源）
#   adapters/    应用侧设备互联适配层（非 UI、非中间件）
#     bt/        蓝牙适配层（SDK/桩双后端）
#     carplay/   CarPlay 复合适配层（适配核 + 专用屏）
#   src/         MVVM 共用层（dashboard_model/emitter/...）
#
#  buildroot local site 包 SITE 覆盖整个 application/，确保 adapters/ 被
#  rsync 到构建目录。
#
#  开关（唯一真相源 = hcn_app_config.cmake）：
#   HCN_APP_MODE     demo|full（full = + vehicle_services + lib_mw + MVVM）
#   HCN_APP_BT       蓝牙适配层（adapters/bt + ui_Bluetooth + bt_focus_adapter）
#   HCN_APP_BT_USE_SDK  蓝牙后端用厂商 SDK（默认桩；需 blueclient/common 入图）
#   HCN_APP_CARPLAY  CarPlay 适配层（需 carlink 系库入图，mk 跟随配置加依赖）
#   HCN_APP_MUSIC    本地音乐（adapters/music + screens/ui_Music + frameworks/music_focus）
#
#  构建入口：buildroot cmake 包 hcn-application
#    make DEVICE=ac83xx VARIANT=userdebug hcn_application-rebuild
# =====================================================================
cmake_minimum_required(VERSION 3.8.2)
project(hcn_application C CXX)

# 模块本地配置（HCN_APP_CFG_*）：唯一真相源；改后须 dirclean 重配
include(${CMAKE_CURRENT_SOURCE_DIR}/hcn_app_config.cmake)

# HCN_APP_MODE 是字符串，不能用 option()：option 只按 BOOL 解释，
# 会把 "full" 归一化成 OFF。这里强制从唯一真相源同步，避免 CMakeCache 残留。
set(HCN_APP_MODE "${HCN_APP_CFG_MODE}" CACHE STRING "demo|full" FORCE)
if(NOT HCN_APP_MODE MATCHES "^(demo|full)$")
    message(FATAL_ERROR "HCN_APP_MODE must be 'demo' or 'full', got '${HCN_APP_MODE}'")
endif()

option(HCN_APP_BT "Enable bluetooth adapter (adapters/bt + ui_Bluetooth + focus bridge)" "${HCN_APP_CFG_BT}")
option(HCN_APP_BT_USE_SDK "Bluetooth adapter vendor SDK backend (else pure C stub)" "${HCN_APP_CFG_BT_USE_SDK}")
option(HCN_APP_CARPLAY "Enable CarPlay composite adapter (needs carlink libs in image)" "${HCN_APP_CFG_CARPLAY}")
option(HCN_APP_MUSIC "Enable local music (adapters/music + ui_Music + music_focus; needs musicplayer package)" "${HCN_APP_CFG_MUSIC}")
option(HCN_APP_BUNDLE_SDK_LIBS "Install matching vendor runtime libraries under lib/hcn_application" ON)

set(HCN_APPLICATION_SDK_LIB_DIR "" CACHE PATH
    "Vendor/shared library directory used for app-private runtime libraries")

set(HCN_APP_UI_VARIANT "${HCN_APP_CFG_UI_VARIANT}" CACHE STRING
    "UI variant: auto|template|fallback" FORCE)
if(NOT HCN_APP_UI_VARIANT MATCHES "^(auto|template|fallback)$")
    message(FATAL_ERROR
        "HCN_APP_CFG_UI_VARIANT must be auto, template or fallback; got '${HCN_APP_UI_VARIANT}'")
endif()
if(HCN_APP_UI_VARIANT STREQUAL "auto")
    set(HCN_APP_UI_VARIANT_MODE 0)
elseif(HCN_APP_UI_VARIANT STREQUAL "template")
    set(HCN_APP_UI_VARIANT_MODE 1)
else()
    set(HCN_APP_UI_VARIANT_MODE 2)
endif()

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "HCN_APP_MODE=${HCN_APP_MODE}")
message(STATUS "HCN_APP_BT=${HCN_APP_BT} (use_sdk=${HCN_APP_BT_USE_SDK})")
message(STATUS "HCN_APP_CARPLAY=${HCN_APP_CARPLAY}")
message(STATUS "HCN_APP_MUSIC=${HCN_APP_MUSIC}")
message(STATUS "HCN_APP_UI_VARIANT=${HCN_APP_UI_VARIANT} (mode=${HCN_APP_UI_VARIANT_MODE})")
message(STATUS "HCN_APP_BUNDLE_SDK_LIBS=${HCN_APP_BUNDLE_SDK_LIBS} (dir=${HCN_APPLICATION_SDK_LIB_DIR})")
message(STATUS "AC83XX_BOOT_DEVICE=${AC83XX_BOOT_DEVICE}")

set(HCN_APPLICATION_VS_INC "${PROJECT_SOURCE_DIR}/../vehicle_services" CACHE PATH
    "vehicle_services public header directory")
set(HCN_APPLICATION_VS_LIB "${PROJECT_SOURCE_DIR}/../vehicle_services/libvehicle_services.a" CACHE FILEPATH
    "vehicle_services static library")

# 中间件静态库目录：buildroot 传 -DHCN_APPLICATION_MW_LIB=$(STAGING_DIR)/usr/lib；
# 源码树内默认 hcn/middleware。链接时直接用绝对路径，不依赖 -L 搜索顺序。
set(HCN_APPLICATION_MW_LIB "${PROJECT_SOURCE_DIR}/../middleware" CACHE PATH
    "lib_mw.a / lib_mw_vendor_hal.a 所在目录")

# LVGL 头文件目录：buildroot 注入 HCN_APPLICATION_MW_INC=<staging>/usr/include，
# HCN_APPLICATION_LVGL_INC 指向的 lvgl/ 同时包含 lvgl.h、lv_conf.h 与 src/。
if(NOT DEFINED HCN_APPLICATION_LVGL_INC)
    if(DEFINED HCN_APPLICATION_MW_INC AND EXISTS "${HCN_APPLICATION_MW_INC}/lvgl/lvgl.h")
        set(_hcn_default_lvgl_inc "${HCN_APPLICATION_MW_INC}/lvgl")
    else()
        set(_hcn_default_lvgl_inc "${PROJECT_SOURCE_DIR}/../../../packages/cluster/lvgl")
    endif()
    set(HCN_APPLICATION_LVGL_INC "${_hcn_default_lvgl_inc}" CACHE PATH
        "LVGL headers directory (lvgl.h, lv_conf.h, src/)")
endif()

# 应用必须与 liblvgl.a 使用同一份 lv_conf.h。显式指定绝对路径，
# 避免构建拷贝树中的残留头文件因 include 顺序而覆盖板级配置。
set(HCN_APPLICATION_LV_CONF "${HCN_APPLICATION_LVGL_INC}/lv_conf.h")
get_filename_component(HCN_APPLICATION_LV_CONF "${HCN_APPLICATION_LV_CONF}" ABSOLUTE)
if(NOT EXISTS "${HCN_APPLICATION_LV_CONF}")
    message(FATAL_ERROR
        "lv_conf.h not found: ${HCN_APPLICATION_LV_CONF}\n"
        "HCN_APPLICATION_LVGL_INC must point to the same LVGL headers used to build liblvgl.a.")
endif()
message(STATUS "HCN_APPLICATION_LV_CONF=${HCN_APPLICATION_LV_CONF}")

add_subdirectory(lvgl/lvgl_app)

# ---- 蓝牙适配层（adapters/bt + ui_Bluetooth + bt_focus_adapter） ----
if(HCN_APP_BT)
    # 透传后端选择给 bt/CMakeLists 的 LVGL_BT_USE_SDK/LINK_SDK_LIBS（cache
    # 选项，FORCE 覆盖其默认 ON —— hcn 默认桩后端，UI 可无 SDK 独立开发）
    set(LVGL_BT_USE_SDK "${HCN_APP_BT_USE_SDK}" CACHE BOOL
        "lvgl_bt backend: vendor SDK" FORCE)
    set(LVGL_BT_LINK_SDK_LIBS "${HCN_APP_BT_USE_SDK}" CACHE BOOL
        "lvgl_bt link vendor SDK libraries" FORCE)
    add_subdirectory(adapters/bt)
    # ui_Home.c/ui.c 中 HCN_APP_HAVE_BT 守卫段的编译开关（PUBLIC：可执行
    # 目标及依赖方均可见）
    target_compile_definitions(lvgl_app PUBLIC HCN_APP_HAVE_BT)
endif()

# ---- CarPlay 适配层（复合适配层：适配核 + 专用屏，保持原模块边界） ----
if(HCN_APP_CARPLAY)
    add_subdirectory(adapters/carplay)
    # ui_Home.c/ui.c/bt_focus_adapter.c 中 LVGL_BUILD_CARPLAY 守卫段
    target_compile_definitions(lvgl_app PUBLIC LVGL_BUILD_CARPLAY)
endif()

# ---- 本地音乐：music_app 业务层原样复用 + AWTK 原语替换层（顺序：compat 在前）----
if(HCN_APP_MUSIC)
    add_subdirectory(adapters/music)
    # ui_Music.c / music_focus.c 在 lvgl_app 里编译，需要 music_app.h 及 compat/awtk.h
    target_include_directories(lvgl_app PUBLIC ${HCN_MUSIC_INC_DIRS})
    target_compile_definitions(lvgl_app PUBLIC HCN_APP_HAVE_MUSIC)
endif()

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Os")
set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -Os")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-s")

target_include_directories(lvgl_app PUBLIC ${PROJECT_SOURCE_DIR}/lvgl/lvgl_app)
target_compile_definitions(lvgl_app PUBLIC
        LV_CONF_INCLUDE_SIMPLE
        LV_FONT_SOURCE_HAN_SANS_SC_16_CJK=1)

if(HCN_APP_MODE STREQUAL "full")
    target_compile_definitions(lvgl_app PUBLIC HCN_APP_FULL)
endif()

if(TARGET lvgl)
    target_compile_definitions(lvgl PUBLIC
            LV_CONF_INCLUDE_SIMPLE
            LV_FONT_SOURCE_HAN_SANS_SC_16_CJK=1)
    target_include_directories(lvgl PUBLIC ${PROJECT_SOURCE_DIR})
endif()

# application/src（MVVM "MVVM/..." 头前缀）：默认适配源码树内构建
# （src 在 SITE 内，buildroot 拷贝树中相对路径同样成立）；
# hcn-application.mk 仍注入绝对路径以双保险
set(HCN_APP_APPSRC "${PROJECT_SOURCE_DIR}/src" CACHE PATH
    "hcn application src dir (MVVM), injected by hcn-application.mk for buildroot builds")

target_include_directories(lvgl_app PUBLIC ${HCN_APP_APPSRC})

# 蓝牙适配层 include（ui_Bluetooth.c / bt_focus_adapter.c / ui_Home.c 守卫段）
if(HCN_APP_BT)
    target_include_directories(lvgl_app PUBLIC
        ${PROJECT_SOURCE_DIR}/adapters/bt/include)
endif()
# CarPlay 适配层 include（ui_Home.c / ui.c 守卫段的 carplayapp_*.h）
if(HCN_APP_CARPLAY)
    target_include_directories(lvgl_app PUBLIC
        ${PROJECT_SOURCE_DIR}/adapters/carplay/include)
endif()

add_executable(hcn_application
        ${PROJECT_SOURCE_DIR}/lvgl/main.c
        ${PROJECT_SOURCE_DIR}/lvgl/mouse_cursor_icon.c)

set(_hcn_lv_conf_targets lvgl_app hcn_application)
foreach(_hcn_optional_target lvgl_ui_template lvgl_bt lvgl_carplay lvgl_music)
    if(TARGET ${_hcn_optional_target})
        list(APPEND _hcn_lv_conf_targets ${_hcn_optional_target})
    endif()
endforeach()
foreach(_hcn_target IN LISTS _hcn_lv_conf_targets)
    target_compile_definitions(${_hcn_target} PUBLIC
        "LV_CONF_PATH=\"${HCN_APPLICATION_LV_CONF}\"")
endforeach()

target_compile_definitions(hcn_application PRIVATE LV_CONF_INCLUDE_SIMPLE)
target_include_directories(hcn_application PRIVATE ${HCN_APP_APPSRC})
if(HCN_APP_BT)
    target_include_directories(hcn_application PRIVATE
        ${PROJECT_SOURCE_DIR}/adapters/bt/include)
endif()

if(HCN_APP_MODE STREQUAL "full")
    target_compile_definitions(hcn_application PRIVATE HCN_APP_FULL)
endif()

# jpeg/png/image：liblvgl.a 自带解码器依赖（lv_libjpeg_turbo/lv_libpng/lv_atcimage
# 的 imgdec_*），与 cluster/lvgl_demo 链接清单一致 —— 不只是媒体应用需要
set(_hcn_ui_link_libs lvgl_app)
if(TARGET lvgl_ui_template)
    list(APPEND _hcn_ui_link_libs lvgl_ui_template)
endif()
if(TARGET lvgl_bt)
    list(APPEND _hcn_ui_link_libs lvgl_bt)
endif()
if(TARGET lvgl_carplay)
    list(APPEND _hcn_ui_link_libs lvgl_carplay)
endif()
if(TARGET lvgl_music)
    list(APPEND _hcn_ui_link_libs lvgl_music)   # 带出 musicplayer(libmusicplayer.so) + pthread
endif()
if(TARGET lvgl_bt OR TARGET lvgl_carplay)
    list(APPEND _hcn_ui_link_libs policyaux)
endif()

# 静态库必须放在同一扫描组：CarPlay/BT 适配库会引用 LVGL 的字体、图像等符号，
# 若 lvgl 在此之前被单独扫描，后续库新增的未解析符号不会触发回头扫描。
target_link_libraries(hcn_application
    -Wl,--start-group ${_hcn_ui_link_libs} lvgl -Wl,--end-group
    m pthread glfw GLESv2 EGL settings_atc
    jpeg png image)

if(HCN_APP_MODE STREQUAL "full")
    # vehicle_services -> lib_mw.a -> lib_mw_vendor_hal.a。
    # 顺序即链接顺序：静态库单遍扫描，C++ HAL 之后接 directrender 系库。
    if(NOT EXISTS "${HCN_APPLICATION_VS_INC}/vehicle_services.h")
        message(FATAL_ERROR "vehicle_services.h not found in ${HCN_APPLICATION_VS_INC}")
    endif()
    if(NOT EXISTS "${HCN_APPLICATION_VS_LIB}")
        message(FATAL_ERROR "libvehicle_services.a not found at ${HCN_APPLICATION_VS_LIB}")
    endif()
    foreach(_mw lib_mw.a lib_mw_vendor_hal.a)
        if(NOT EXISTS "${HCN_APPLICATION_MW_LIB}/${_mw}")
            message(FATAL_ERROR
                "${_mw} not found in ${HCN_APPLICATION_MW_LIB}（buildroot 需传 -DHCN_APPLICATION_MW_LIB=$(STAGING_DIR)/usr/lib）")
        endif()
    endforeach()
    add_library(vehicle_services STATIC IMPORTED GLOBAL)
    set_target_properties(vehicle_services PROPERTIES
        IMPORTED_LOCATION "${HCN_APPLICATION_VS_LIB}"
        INTERFACE_INCLUDE_DIRECTORIES "${HCN_APPLICATION_VS_INC}")

    # 用绝对路径而不是 -l:lib_mw.a：不依赖 -L 搜索顺序，也避开 -lmw 命名坑。
    target_link_libraries(hcn_application
        vehicle_services
        "${HCN_APPLICATION_MW_LIB}/lib_mw.a"
        "${HCN_APPLICATION_MW_LIB}/lib_mw_vendor_hal.a"
        directrender surface_atc osal rt)
endif()

# BT/CarPlay 的适配库已加入上方静态扫描组；其 PUBLIC 依赖由各自 CMakeLists 继续透传。

# 输出固定到 <build>/bin：避免可执行文件名与构建目录中的同名目录冲突
# （ld 报 "cannot open output file hcn_application: Is a directory"）。
set_target_properties(hcn_application PROPERTIES
    LINKER_LANGUAGE CXX
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# 应用私有运行库：可执行文件位于 /usr/bin，RPATH 解析到 /usr/lib/hcn_application。
# 厂商连接类库必须与编译期头文件保持同一版本，不能回落到系统旧版本。
set(_hcn_private_runtime_libs
    libsettings_atc.so
    libpolicyaux.so)
if(HCN_APP_BT AND HCN_APP_BT_USE_SDK)
    list(APPEND _hcn_private_runtime_libs
        libbluetoothclient.so
        libbluetoothcommon.so)
endif()
if(HCN_APP_CARPLAY)
    list(APPEND _hcn_private_runtime_libs
        libaccessoryinfo.so)
endif()

if(HCN_APP_BUNDLE_SDK_LIBS)
    if(NOT HCN_APPLICATION_SDK_LIB_DIR)
        message(WARNING
            "HCN_APP_BUNDLE_SDK_LIBS=ON but HCN_APPLICATION_SDK_LIB_DIR is empty; "
            "vendor runtime libraries will not be installed")
    else()
        set(_hcn_private_runtime_lib_paths)
        foreach(_lib IN LISTS _hcn_private_runtime_libs)
            set(_lib_path "${HCN_APPLICATION_SDK_LIB_DIR}/${_lib}")
            if(NOT EXISTS "${_lib_path}")
                message(FATAL_ERROR
                    "Runtime library ${_lib} not found in ${HCN_APPLICATION_SDK_LIB_DIR}; "
                    "rebuild its buildroot package or set HCN_APPLICATION_SDK_LIB_DIR")
            endif()
            list(APPEND _hcn_private_runtime_lib_paths "${_lib_path}")
        endforeach()

        install(FILES ${_hcn_private_runtime_lib_paths}
            DESTINATION lib/hcn_application)
        set_target_properties(hcn_application PROPERTIES
            INSTALL_RPATH "$ORIGIN/../lib/hcn_application:$ORIGIN/lib/hcn_application"
            BUILD_WITH_INSTALL_RPATH TRUE)
    endif()
endif()

install(TARGETS hcn_application DESTINATION bin)
install(PROGRAMS  ${PROJECT_SOURCE_DIR}/lvgl/S01hcn_application DESTINATION /etc/init.d/)
