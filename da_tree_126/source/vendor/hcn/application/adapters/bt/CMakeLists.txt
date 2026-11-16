# =====================================================================
#  lvgl_bt — 蓝牙适配层（SDK/桩双后端）
#
#  后端选择：LVGL_BT_USE_SDK（厂商 SDK，bt_platform_sdk.cpp）/ 桩（stub）
#   - hcn 顶层 CMakeLists 由 HCN_APP_BT / HCN_APP_BT_USE_SDK 透传（FORCE
#     覆盖下方默认 ON —— hcn 默认桩后端，UI 可无 SDK/硬件独立开发调试）
#   - SDK 头目录：源码树内由 lvgl_bt_find_connectivity_include 自动上溯
#     检索（8 级内可命中）；buildroot 拷贝树由 hcn-application.mk 注入
#     -DLVGL_BT_SDK_INCLUDE_DIR / -DLVGL_BT_UNIVERSAL_UTILS_INCLUDE_DIR
# =====================================================================
cmake_minimum_required(VERSION 3.8.2)

option(LVGL_BT_USE_SDK "Use platform Bluetooth SDK backend" ON)
option(LVGL_BT_LINK_SDK_LIBS "Link platform Bluetooth SDK libraries" ON)
set(LVGL_BT_SDK_INCLUDE_DIR "" CACHE PATH "Bluetooth SDK include directory")
set(LVGL_BT_UNIVERSAL_UTILS_INCLUDE_DIR "" CACHE PATH "Universal utils include directory")

function(lvgl_bt_find_connectivity_include OUT_VAR PACKAGE_NAME CHECK_FILE)
    set(search_dir "${CMAKE_CURRENT_SOURCE_DIR}")
    set(found_dir "")

    foreach(_ RANGE 0 8)
        set(candidate "${search_dir}/source/packages/connectivity/${PACKAGE_NAME}/include")
        if((CHECK_FILE AND EXISTS "${candidate}/${CHECK_FILE}") OR
           (NOT CHECK_FILE AND IS_DIRECTORY "${candidate}"))
            set(found_dir "${candidate}")
            break()
        endif()

        set(candidate "${search_dir}/packages/connectivity/${PACKAGE_NAME}/include")
        if((CHECK_FILE AND EXISTS "${candidate}/${CHECK_FILE}") OR
           (NOT CHECK_FILE AND IS_DIRECTORY "${candidate}"))
            set(found_dir "${candidate}")
            break()
        endif()

        get_filename_component(parent_dir "${search_dir}" DIRECTORY)
        if(parent_dir STREQUAL search_dir)
            break()
        endif()
        set(search_dir "${parent_dir}")
    endforeach()

    set(${OUT_VAR} "${found_dir}" PARENT_SCOPE)
endfunction()

lvgl_bt_find_connectivity_include(LVGL_BT_DEFAULT_SDK_INCLUDE_DIR bluetooth bluetoothapi.h)
lvgl_bt_find_connectivity_include(LVGL_BT_DEFAULT_UNIVERSAL_UTILS_INCLUDE_DIR universal_utils "")

set(LVGL_BT_PLATFORM_SOURCE platform/bt_platform_stub.c)
if(LVGL_BT_USE_SDK)
    set(LVGL_BT_PLATFORM_SOURCE platform/bt_platform_sdk.cpp)
    message(STATUS "lvgl_bt backend: SDK")
else()
    message(WARNING "lvgl_bt backend: STUB. PBAP, local name, and power state are simulated.")
endif()

add_library(lvgl_bt STATIC
    core/bt_app.c
    core/bt_config.c
    core/bt_core.c
    core/bt_event_queue.c
    core/bt_event_apply.c
    core/bt_records.c
    profile/bt_gap.c
    profile/bt_hfp.c
    profile/bt_pbap.c
    profile/bt_avrcp.c
    profile/bt_a2dp.c
    ${LVGL_BT_PLATFORM_SOURCE}
)

target_include_directories(lvgl_bt PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(lvgl_bt PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/profile
    ${CMAKE_CURRENT_SOURCE_DIR}/platform
)

if(LVGL_BT_SDK_INCLUDE_DIR AND EXISTS "${LVGL_BT_SDK_INCLUDE_DIR}/bluetoothapi.h")
    target_include_directories(lvgl_bt PRIVATE ${LVGL_BT_SDK_INCLUDE_DIR})
elseif(EXISTS "${LVGL_BT_DEFAULT_SDK_INCLUDE_DIR}/bluetoothapi.h")
    target_include_directories(lvgl_bt PRIVATE ${LVGL_BT_DEFAULT_SDK_INCLUDE_DIR})
elseif(LVGL_BT_USE_SDK)
    message(WARNING "Bluetooth SDK include directory not found; bluetoothapi.h may be missing")
endif()

if(LVGL_BT_UNIVERSAL_UTILS_INCLUDE_DIR AND IS_DIRECTORY "${LVGL_BT_UNIVERSAL_UTILS_INCLUDE_DIR}")
    target_include_directories(lvgl_bt PRIVATE ${LVGL_BT_UNIVERSAL_UTILS_INCLUDE_DIR})
elseif(IS_DIRECTORY "${LVGL_BT_DEFAULT_UNIVERSAL_UTILS_INCLUDE_DIR}")
    target_include_directories(lvgl_bt PRIVATE ${LVGL_BT_DEFAULT_UNIVERSAL_UTILS_INCLUDE_DIR})
endif()

if(LVGL_BT_USE_SDK)
    target_compile_features(lvgl_bt PUBLIC cxx_std_14)
endif()

target_link_libraries(lvgl_bt PUBLIC pthread)

if(LVGL_BT_LINK_SDK_LIBS AND LVGL_BT_USE_SDK)
    target_link_libraries(lvgl_bt PUBLIC
        bluetoothcommon
        bluetoothclient
        universal_utils
        tinyxml2
        sqlite3
        dl
    )
endif()
