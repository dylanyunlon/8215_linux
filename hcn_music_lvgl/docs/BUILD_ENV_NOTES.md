# 构建环境备忘（DA / LVGL 树）

> 已脱敏：不含内网地址、账号、密码、人名。具体的内网仓库地址与权限请向管理员确认。

## 两棵树，不要混
| | 本仓库 `main` 里的快照 | DA / LVGL 树 |
|---|---|---|
| 工具链 | `arm-buildroot-linux-gnueabi`（glibc） | **`arm-buildroot-linux-musleabihf`（musl，hard-float）** |
| UI | AWTK / Qt | **LVGL 9.3.0**（源码在 `source/packages/cluster/lvgl`，CMake 编） |
| 应用 | `musicplayer_awtk` | `hcn_application`（cmake 包），应用源码在 `source/vendor/hcn/{application,middleware,vehicle_services}` |

## DA 树怎么拼出来
- 由一份 manifest（分支 `HCN`，清单 `hcn_manifest.xml`）同步，它 include 了 `hcn_common_app.xml`，后者定义三个应用仓库，
  分支都是 **`AC8215E`**，落点是 `source/vendor/hcn/application`、`.../middleware`、`.../vehicle_services`（注意**下划线**）。
- 这三个应用仓库的默认分支 `master` 只有一个 "Initial empty repository" 提交：**直接 clone 不切分支，拿到的是空目录**，
  拷进树里后 `lib_mw` 报 `No targets specified and no makefile found`。
- 基线 manifest 同步下来的 `build` / `cluster` 可能落后于服务器上 `HCN_AC8215` 分支；`hcn_application.mk`、`lib_mw.mk`、`vehicle_services.mk`
  是后来加的，同步前先确认 `configs/package/hcn_application/` 存在。

## 构建要点
- 入口：`./allmake.sh -a <介质>-<容量>-ddr-<大小> -d userdebug -m false`；首次整编需要完整跑一遍（内核/工具链/全部包）。
- **这棵树的 `allmake.sh` 不支持 `<pkg>-dirclean` / `<pkg>-rebuild` 透传**：多余参数被忽略，等于又启动了一次完整构建。
  要强制重编某个包，直接删 `out/build/ac83xx/<pkg>*` 再重新整编（已编好的包靠 stamp 跳过）。改了 `.mk` 的 `CONF_OPTS` 之后同理，`-rebuild` 不会重跑 cmake configure。
- 日志里这些是已有噪音，不用处理：`ignoring unsupported character`（Config.in）、`BR2_PACKAGE_LIBGLFW_ATC unmet direct dependencies`、
  开头的 `Can't open perl script .../PartitionUtility_nor.pl`（路径多一层 `..`，整编仍能跑到出镜像）。
- `hcn_application.mk`：依赖 `lvgl lib_mw vehicle_services directrender`；`CONF_OPTS` 传 `HCN_APPLICATION_VS_INC / VS_LIB / MW_INC / MW_LIB` 与 `HCN_APP_SETTINGS_INC`。
  **它不读 `hcn_app_config.cmake`**，要加依赖或参数只能改 mk 本身。
- 产物：`out/target/ac83xx/usr/bin/hcn_application`（ARM、musl 动态链接），`out/staging/usr/lib/{liblvgl.a,lib_mw.a,lib_mw_vendor_hal.a}`。

## LVGL / lv_conf 事实（来自板级 `cluster/lvgl/lv_conf.h`）
`LV_USE_STDLIB_MALLOC` = **CLIB**（用 libc malloc，没有内存池，`LV_MEM_SIZE` 不生效）、`LV_CACHE_DEF_SIZE` 0（图片缓存关）、`LV_FONT_FMT_TXT_LARGE` 1、`LV_USE_FONT_COMPRESSED` 1、
默认字体 `lv_font_source_han_sans_sc_16_full_cjk`（vanilla LVGL 里没有这个符号，说明是魔改版）、`LV_USE_OS` 为 NONE（整个 UI 单线程）。
显示：GLFW + EGL/GLES 纹理（Mali，fbdev 后端，无 DRM/wayland），输入 evdev 自动发现。`LV_USE_DRAW_SW_ASM` = NEON、`LV_USE_DRAW_OPENGLES` = 1、PNG/JPEG/`ATCIMAGE`/POSIX 文件系统（盘符 A）开着、`LV_USE_LOG` = 1，`LV_USE_PERF_MONITOR`/`LV_USE_MEM_MONITOR` 仍是 1（真机屏幕会叠加调试信息）。

## 设备事实（adb）
内核 3.18.49 armv7l；`/dev/fb0` 1024×600 32bpp（mtk_fb，无 `/dev/dri`）；触摸 `/dev/input/event0`（`ac83xx_tsadcc`，protocol B 多点）；
MemTotal ≈ 179 MB；rootfs ≈ 172 MB；`/data` 仅 12 MB（收藏/数据库要控制体积）。存储介质（nor/nand/emmc）**待确认**，刷机前必须与构建参数对上。

## 已踩过的坑
1. `musicplayer.mk` 原版写死 `arm-buildroot-linux-gnueabi`：在 musl 树里找不到编译器 → 用 `tools/add_musicplayer_pkg.sh` 里的工具链无关版。
2. defconfig 里 `BR2_PACKAGE_LVGL_HOME=y` 与 `BR2_PACKAGE_HCN_APPLICATION=y` 同时开 → 上板后两个 UI 进程抢屏幕；刷机前关掉前者。
3. `BR2_PACKAGE_MUSICPLAYER_AWTK` 依赖 AWTK，LVGL 版里不要选。
4. 引号包含 `#include "awtk_stub.h"` 先搜索包含者自己的目录：用 `-I` 顺序压不过上游那份基于线程的 stub，见 `adapters/music/include/compat/awtk.h` 头注释。
5. 压缩字库和 `LV_FONT_FMT_TXT_LARGE` 必须与 `liblvgl.a` 的 lv_conf 一致，编字库用错 lv_conf 会得到假的“1 MB 上限”错误。

## 想在 CI/本机把测试跑得更真，还缺这些（**含内部源码，请放私有仓库，不要放公开仓库**）
1. 板级 `source/packages/cluster/lvgl/` 下的 `lv_conf.h`、`lvgl.h`、`lv_version.h`、**`lvgl_private.h`**、`src/`（LVGL 魔改源码），用 `LVGL_DIR` + `BOARD_LV_CONF` 替换 vanilla v9.3.0（上次的包漏了 `lvgl_private.h`）。
2. HCN 应用里的这几个文件（`ui_Music.c` / `music_focus.c` 编译与测试要用）：
   `lvgl/lvgl_app/frameworks/{audiofocusmanager.c,audiofocusmanager.h,displayfocusmanager.h,app_common.h,videofocusmanager.h}`。
