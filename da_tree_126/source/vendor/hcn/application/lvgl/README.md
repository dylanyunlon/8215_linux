# hcn/application/lvgl — HCN lvgl UI 模块（迁移自 lvgl_home demo）

依据《DA_AC8215_lvgl程序流与代码迁移hcn统一编译梳理》方案 A 落地；
蓝牙/CarPlay 适配层迁入见《DA_AC8215_lvgl_home蓝牙模块分析与hcn迁移抽离设计》
（adapters/ 结构见 ../adapters/README.md）。

## 结构（2026-09-23 起适配层迁入后）

| 目录/文件 | 说明 |
|---|---|
| `../CMakeLists.txt` | **CMake 根在 application/**（SITE 覆盖整目录，adapters 才能进 buildroot 拷贝树） |
| `main.c` | 进程入口：lv_init + GLFW/EGL 显示 + ui_init + 主循环（`lv_timer_handler` 5ms 协作轮询）；full 模式加 `vs_init()` + `dashboard_vm_flush()`；`HCN_APP_HAVE_BT` 加 `bt_app_poll()` + `bt_focus_adapter_poll()`；`LVGL_BUILD_CARPLAY` 加 `carplayapp_poll()` |
| `lvgl_app/ui.c` | UI 装配：Home 屏注册 + `HCN_APP_HAVE_BT` 蓝牙屏注册/bt UI 钩子/bt_focus_adapter + `LVGL_BUILD_CARPLAY` CarPlay 双屏注册 |
| `lvgl_app/frameworks/` | `videofocusmanager`（屏幕导航栈）、`audiofocusmanager`、`app_common`、`audio_settings`、`policyEvent/streamTypePublic`（policyaux 客户端契约）、`bt_focus_adapter`（蓝牙焦点桥，HCN_APP_BT） |
| `lvgl_app/screens/ui_Home.c` | Home 屏（bt/aa/carplay 调用点用 `HCN_APP_HAVE_BT/AA`、`LVGL_BUILD_CARPLAY` 宏守卫） |
| `lvgl_app/screens/ui_Bluetooth.c` | 蓝牙屏 6 页面（拨号/联系人/通话记录/音乐/配对/设置，HCN_APP_BT） |
| `lvgl_app/screens/dashboard_view.c` | MVVM lvgl View（full 模式）：订阅 `dashboard_vm` emitter 刷新车速/转速/档位 label |
| `lvgl_app/components\|fonts\|images` | 组件/字体/图片资源 |
| `S01hcn_application` | init 脚本（迁自 S01lvgl_home） |

未迁入（保持原位，后续按需）：androidauto / rvc 适配层、
atcMusic/atcPicture/atcVideo 媒体源、atcpicture_decoder、WiFi 设置页。
原包 `source/packages/application/lvgl_home` 保留为 baseline。

## MVVM 与数据交互层（保留契约）

```
lib_mw vehicle_param 回调（CAN 线程）
  -> vehicle_services.c on_vehicle_change        [vs 门面，UI↔mw 唯一边界]
  -> dashboard_model_set()  emitter 置脏
  -> main 循环 dashboard_vm_flush()（GUI 线程 diff 派发）
  -> dashboard_view -> lvgl label 刷新
```

蓝牙数据并行不悖、各司其职：

```
BT SDK（回调线程）-> bt 事件队列 -> BtAppSnapshot -> ui_Bluetooth 拉取（主循环）
```

- `../src/MVVM/core/emitter.c`、`model/dashboard_model.c`：纯 C，UI 无关，原样复用
- `../src/MVVM/viewModel/dashboard_vm.c`：已去除 awtk.h 耦合（idle_queue → 置脏 +
  GUI 线程 flush），AWTK/lvgl 两种 View 共用同一实现
- application 不 include 任何 `<mw/xxx.h>`，只依赖 `vehicle_services.h` + MVVM

## 编译（buildroot 包 hcn-application）

```sh
# full 模式（默认，+ lib_mw + vehicle_services + MVVM）
make DEVICE=ac83xx VARIANT=userdebug hcn_application-rebuild

# 蓝牙适配层：menuconfig 打开 "bluetooth adapter"（默认桩后端）
#   BR2_PACKAGE_HCN_APPLICATION_BT=y（自动 select POLICYAUX）
#   BT 栈（BLUECLIENT/BLUECOMMON）入图后自动切厂商 SDK 后端
# CarPlay：menuconfig 打开 "carplay composite adapter"（需 CarPlay 组件）
#   BR2_PACKAGE_HCN_APPLICATION_CARPLAY=y（自动 select CARPLAY）

# 切开关后必须强制重配（-rebuild 不清 .stamp_configured）：
make DEVICE=ac83xx VARIANT=userdebug hcn_application-dirclean
make DEVICE=ac83xx VARIANT=userdebug hcn_application-rebuild
```

CMake 选项：`-DHCN_APP_MODE=demo|full`、`-DHCN_APP_BT=ON|OFF`、
`-DHCN_APP_BT_USE_SDK=ON|OFF`、`-DHCN_APP_CARPLAY=ON|OFF`
（由 Config.in 选项映射，一般不手动传）。

full 链接：`vehicle_services mw rt`（vehicle_services 在 lib_mw 前；cJSON 需 -lm）。
BT 链接：`lvgl_bt policyaux`（SDK 后端时 .so 由 lvgl_bt 目标自链）。

## buildroot 构建路径注入（重要）

buildroot local 站点包把 `application/` **整树** rsync 到
`out/build/ac83xx/<pkg>/` 构建，SITE 之外的源码树路径不可解析，
`hcn-application.mk` 注入绝对路径：

- `-DHCN_APP_APPSRC=source/vendor/hcn/application/src`（MVVM 头与源码；src 在 SITE 内，默认相对路径拷贝树同样成立，注入双保险）
- `-DHCN_APP_SETTINGS_INC=source/packages/graphics/libsettings_atc/include`（audio_settings.c 所需 AtcAudioSettings.h）
- 蓝牙/CarPlay 适配层的注入项见 `../adapters/README.md`

在源码树内直接跑 cmake 时使用各 CMakeLists 内默认相对路径，无需传参。
