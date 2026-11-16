# hcn/application/adapters — 应用侧设备互联适配层

依据《DA_AC8215_lvgl_home蓝牙模块分析与hcn迁移抽离设计》落地。
定位：**既不属于中间件（middleware/vehicle_services 域），也不属于 UI
（lvgl/ 域）** 的第三极 —— 把外部设备 SDK / 互联协议栈适配成应用可用的
门面，供 UI 进程直接消费。

## 归属判定规则（防腐烂约定）

满足以下全部条件的模块放 `adapters/`：

1. 包一个**外部 SDK / 协议栈**（蓝牙 SDK、carlink、亿连 SDK…）；
2. 对 UI 只暴露**纯 C 门面 + 快照/事件 + UI 钩子**（回调反转）；
3. 自身**不含 lvgl 控件**、不 include `<mw/xxx.h>`（车辆数据一律走
   vehicle_services）。

不满足的：有 lvgl 控件 → `lvgl/lvgl_app/screens`；车辆数据/实时控制 →
`hcn/middleware`；跨 UI↔中间件边界 → `hcn/vehicle_services`。

## 依赖方向约定

```
lvgl/ ──依赖──► adapters/ ──dlopen/链接──► 外部 SDK（.so / carlink 库）
adapters/ 不依赖 lvgl（UI 钩子反转）
adapters/ 不 include 任何 <mw/xxx.h>（车辆数据走 vehicle_services）
```

## 模块清单

| 模块 | 迁自 | 形态 | 开关（hcn_app_config.cmake） |
|---|---|---|---|
| `bt/` | `lvgl_home/bt`（~5549 行，源码零改动） | 纯 C 门面（bt_app.h）+ 事件队列/快照 + GAP/HFP/PBAP/AVRCP/A2DP profile + **SDK/桩双后端**（platform/） | `HCN_APP_CFG_BT`（当前 ON）；后端 `HCN_APP_CFG_BT_USE_SDK`（当前 OFF=桩） |
| `carplay/` | `lvgl_home/lvgl_carplay`（11 文件） | **复合适配层**：C++ 适配核（carplayapp_if 门面）+ 专用屏（ui/）。与 bt 不同，核↔屏双向耦合（核直接调 carplaysetting_* UI 函数），保持原模块边界为单一静态库，不再强行拆分 | `HCN_APP_CFG_CARPLAY`（当前 OFF，需 carlink CarPlay 组件入图） |

## 开关配置（2026-09-23 起本地化，不经 buildroot kconfig）

适配层开关的**唯一真相源**是 `application/hcn_app_config.cmake`：

```cmake
set(HCN_APP_CFG_BT        ON)   # 蓝牙适配层 + 蓝牙屏 + 焦点桥
set(HCN_APP_CFG_BT_USE_SDK OFF) # 桩后端（ON=厂商 SDK，需 BT 栈入图）
set(HCN_APP_CFG_CARPLAY   OFF)  # CarPlay 复合适配层
```

机制：CMake 根 `include` 该文件作为 option 默认值；`hcn-application.mk`
grep 该文件跟随包依赖（BT=ON → +policyaux；CARPLAY=ON → +carplay 组件）。
优先级：命令行 `-D` > 配置文件 > 内置默认。

```sh
# 改配置文件后（CMakeCache 缓存旧值，必须强制重配）：
./allmake.sh -a nor-64-ddr-256 -d userdebug -m false hcn_application-dirclean
./allmake.sh -a nor-64-ddr-256 -d userdebug -m false hcn_application-rebuild
```

判据：日志出现 `Configuring` 步骤与 `Building C object bt_*.c` /
`Building CXX object carplay/carplayapp*.cpp` 行才算真正生效。

bt 的 UI 侧配套（同批迁入，在 lvgl/ 域内）：

- `lvgl/lvgl_app/screens/ui_Bluetooth.c`（6 页面：拨号/联系人/通话记录/音乐/配对/设置）
- `lvgl/lvgl_app/frameworks/bt_focus_adapter.c`（音频焦点桥：进程内
  audiofocusmanager 仲裁 → `send_policy_event` 系统级上报，链 libpolicyaux）

## buildroot 路径注入（重要）

buildroot local 站点包把 `application/` 整树 rsync 到
`out/build/ac83xx/hcn-application/` 拷贝构建，SITE 之外的路径在拷贝树中
不可解析。`hcn-application.mk` 注入绝对路径：

- `-DLVGL_BT_SDK_INCLUDE_DIR` / `-DLVGL_BT_UNIVERSAL_UTILS_INCLUDE_DIR`
  （bt 的 SDK 头；源码树内 bt/CMakeLists 会自动上溯检索，拷贝树必须注入）
- `-DHCN_APP_CARLINK_INTERFACES` / `-DHCN_APP_BT_SDK_INC` /
  `-DHCN_APP_AUDIO_POLICY_INC`（carplay 的 carlink/蓝牙/策略头）

## 编译

```sh
# 蓝牙（默认桩后端，UI 可独立验证）
make DEVICE=ac83xx VARIANT=userdebug hcn_application-dirclean
make DEVICE=ac83xx VARIANT=userdebug hcn_application-rebuild
# menuconfig: hcn application -> bluetooth adapter 打开
# SDK 后端在 BT 栈（BLUECLIENT/BLUECOMMON）入图后自动切换

# CarPlay：menuconfig 打开 carplay composite adapter（需 CarPlay 组件）
```

经验：kconfig 开关切换后必须 `dirclean` + `rebuild`（`-rebuild` 不清
`.stamp_configured`，CMake 缓存会沿用旧开关），日志需出现 `Configuring`
步骤与 `Building C object bt_*.c` / `carplayapp_*.cpp` 行才算真正生效。
