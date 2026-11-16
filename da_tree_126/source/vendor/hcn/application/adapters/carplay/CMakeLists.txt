# =====================================================================
#  lvgl_carplay — CarPlay 复合适配层（适配核 + 专用屏）
#
#  构成：carplay/ 适配核（C++：carplayapp.cpp / carplayapp_if.cpp / 事件队列）
#        ui/      专用屏（carplayapp_ui.c / carplaysetting_ui.c，与适配核
#                 双向耦合，故整体为单一静态库，不再拆分 —— 与 bt 适配层的
#                 纯门面形态不同，见 adapters/README.md）
#
#  ⚠ 相对路径只适配源码树内构建（adapters/carplay 上溯 6 级到工程根）；
#    buildroot 拷贝树中由 hcn-application.mk 注入绝对 CACHE 变量：
#      HCN_APP_CARLINK_INTERFACES / HCN_APP_BT_SDK_INC / HCN_APP_AUDIO_POLICY_INC
#    （frameworks 路径在 SITE 内，相对路径两种场景均成立）
# =====================================================================
project(lvgl_carplay)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# carlink 互联协议栈接口（carplay/accessory/iap）
set(HCN_APP_CARLINK_INTERFACES "${PROJECT_SOURCE_DIR}/../../../../../../source/packages/connectivity/carlink/interfaces" CACHE PATH
    "carlink interfaces dir, injected by hcn-application.mk for buildroot builds")
# 蓝牙 SDK 头（carplay 配对设备枚举依赖 bluetoothapi.h 族）
set(HCN_APP_BT_SDK_INC "${PROJECT_SOURCE_DIR}/../../../../../../source/packages/connectivity/bluetooth/include" CACHE PATH
    "bluetooth SDK include dir, injected by hcn-application.mk for buildroot builds")
# audiopolicy 头（policyEvent.h 上游定义）
set(HCN_APP_AUDIO_POLICY_INC "${PROJECT_SOURCE_DIR}/../../../../../../source/packages/audio/audiopolicy/include" CACHE PATH
    "audiopolicy include dir, injected by hcn-application.mk for buildroot builds")
# hcn UI 框架头（app_common/videofocusmanager/audiofocusmanager/policyEvent 副本）
set(HCN_APP_FRAMEWORKS_INC "${PROJECT_SOURCE_DIR}/../../lvgl/lvgl_app/frameworks" CACHE PATH
    "hcn lvgl_app frameworks include dir (inside SITE, relative works in both trees)")

SET(SOURCES
    carplay/carplayapp.cpp
    carplay/carplayapp_if.cpp
    carplay/cp_event_queue.cpp
    ui/carplayapp_ui.c
    ui/carplaysetting_ui.c)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    include
    ${HCN_APP_CARLINK_INTERFACES}/carplay
    ${HCN_APP_CARLINK_INTERFACES}/accessory
    ${HCN_APP_CARLINK_INTERFACES}/../utils/include
    ${HCN_APP_BT_SDK_INC}
    ${HCN_APP_FRAMEWORKS_INC}
    ${HCN_APP_AUDIO_POLICY_INC}
    )

# carlink 系库 + surface + 音频策略（需 CarPlay 组件入图，HCN_APP_CARPLAY
# 默认 OFF 的原因：nor-64 DA 基线无 carlink）
target_link_libraries(${PROJECT_NAME} PUBLIC
    carplayclient
    accessoryinfo
    surface_atc
    carlinkutils
    protobuf-lite
    audiopolicy
    policyaux
    )
