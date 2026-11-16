# =====================================================================
#  hcn_app_config.cmake — HCN application 模块本地配置（唯一真相源）
#
#  团队在此调整适配层开关，无需 buildroot menuconfig / 改 out/.config。
#  buildroot 侧（hcn-application.mk）会 grep 本文件决定包依赖
#  （BT→policyaux，CarPlay→carplay 组件），CMake 侧 include 本文件
#  取开关值。HCN_APP_CFG_* 为唯一真相源；切换后请 dirclean 重配。
#
#  ⚠ 改动本文件后必须强制重配（CMakeCache 会缓存旧值）：
#    ./allmake.sh -a nor-64-ddr-256 -d userdebug -m false hcn_application-dirclean
#    ./allmake.sh -a nor-64-ddr-256 -d userdebug -m false hcn_application-rebuild
#  （demo/full 模式也由本文件 HCN_APP_CFG_MODE 决定）
# =====================================================================

# ---- 构建模式：demo / full ----
# demo: 纯 LVGL UI 基线，不接 vehicle_services / MVVM
# full: UI + vehicle_services + lib_mw + MVVM（生产/联调）
set(HCN_APP_CFG_MODE full)

# ---- 蓝牙适配层（adapters/bt + ui_Bluetooth + bt_focus_adapter） ----
# ON : 编入蓝牙适配层与蓝牙屏（6 页面）
# OFF: 全部剔除（ui_Home.c 等守卫段不编译，回到最小 UI 模块）
set(HCN_APP_CFG_BT ON)

# ---- 蓝牙后端：厂商 SDK / 桩 ----
# OFF: 桩后端（bt_platform_stub.c，模拟扫描/电话簿，用于无 SDK/硬件环境）
# ON : 厂商 SDK 后端（dlopen bluetoothclient/bluetoothcommon —— 需 BT 栈
#      已入图，即 BR2_PACKAGE_BLUECLIENT/BLUECOMMON=y，否则链接失败）
set(HCN_APP_CFG_BT_USE_SDK ON)

# ---- CarPlay 复合适配层（adapters/carplay，适配核+专用屏） ----
# ON : 编入 CarPlay（mk 自动给包加 carplay 依赖，把 carlink CarPlay 组件
#      拉入镜像 —— nor-64 空间敏感，按需开启）
# OFF: 全部剔除
set(HCN_APP_CFG_CARPLAY ON)

# ---- 本地音乐（adapters/music + ui_Music + music_focus） ----
# ON : 编入本地音乐（mk 自动给包加 musicplayer 依赖并注入 HCN_APP_MUSIC_SRC；
#      字体 music_font_20/30 约 1.5 MB flash —— nor-64 空间敏感，按需开启）
# OFF: 全部剔除
set(HCN_APP_CFG_MUSIC ON)

# ---- UI 变体：auto / template / fallback ----
# auto     : 模板资源存在则启动模板 UI，否则回退当前 UI
# template : 强制模板 UI；初始化失败时仍回退一次
# fallback : 只编入当前 UI，不安装模板资源
set(HCN_APP_CFG_UI_VARIANT auto)
