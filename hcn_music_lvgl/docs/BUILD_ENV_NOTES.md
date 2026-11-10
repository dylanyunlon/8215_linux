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
MemTotal ≈ 179 MB；rootfs ≈ 172 MB；`/data` 仅 12 MB（收藏/数据库要控制体积）。存储介质已确认见下一节「板子与构建参数」；刷机前仍要把 scatter 分区表与设备 `mtdparts` 逐项对上。

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

## 板子与构建参数（adb 实测 + 整编验证）
**设备**（`/proc/cmdline`、`/proc/mtd`、`df`）：NAND（控制器 `atcnand`），根分区 **ext4**（`root=/dev/mtkd13`），**没有 A/B**。
`mtdparts`：preloader 512K×2、datazone 2M×2、uboot 2M、trustzone 2M、arm2 4M、dtb 2M、logo 5M、boot_misc 2M、vba 3M、metazone 2M、
kernel 5M、**system 204M**、**usrdata 18M**（合计 254 MiB，即 256 MB 级 NAND）。`/` 约 172 MB（已用 39.5 MB），`/data` 12.3 MB；
`tmpfs` 各 89.4 MB（≈ 内存一半），MemTotal ≈ 179 MB → **DDR 256**。

**构建参数**：本板对应 `./allmake.sh -a nand-256-ddr-256 -d userdebug -m false`（`build/setup` 解析 `-a <介质>-<容量>-ddr-<内存>`；非 A/B 的 nand 走 `nand_ext4_256M`）。
- 不要用 `nor-64-ddr-256`：它生成 `scatter.norboot.squashfs.xml`（NOR 启动，**system 只有 30 MiB 的 squashfs**，usrdata 为 jffs2），和本板的 NAND/ext4 布局不是一回事。
  不带音乐的基线就已用掉 27.2 MiB；再加 ffmpeg + musicplayer 后 `mksquashfs` 报 `image size … exceeded given size`，`target-post-image` 失败。
- 也不要照搬 AWTK 版的 `nand-512-ddr-512`：容量和内存都和本板对不上（内存布局/保留区随 DDR 变）。
- 换 `-a` 后用全新的 `out/`（把旧的挪走）；源码/配置里的改动不受影响。
- `allmake.sh` 对 nand（非 128）会做 raw 分区保留检查（`[CHECK] … limit=…`，超出报 `[ERROR] … overflow! need reserve 1MB`）；本板 kernel 分区只有 5 MiB。

## 离线构建（整编机无公网）
- 缺源码包时 Buildroot 的下载会被网络过滤拦截：返回的是一个几 KB 的拦截页，表现为 `wrong sha256`（不是真的包损坏）。
- 做法：从别处（已编过的树的 `dl/`，或能上网的机器）取包，核对 sha256 后放到 `<下载目录>/<包名>/`。下载目录看 `out/.config` 里的 `BR2_DL_DIR`
  （默认 `$(TOPDIR)/dl`，即 `buildroot/dl`）。`ffmpeg-3.4.5.tar.xz` 的 sha256 是 `741cbd6394eaed370774ca4cc089eaafbc54d0824b9aa360d4b3b0cbcbc4a92c`，必须用官网发布包（GitHub tag 打包的内容不同，哈希对不上）。
- 新换 `-a` 从零编之前，先把已有的 `dl/` 同步进来：`rsync -a --ignore-existing <旧 dl>/ <新树>/buildroot/dl/`。

## musicplayer 包在 musl 树里的三处修复（已在本仓库 `source/packages/application/musicplayer` 落地）
1. `Makefile` 补 `-I.`：`awtk_app/src/cmus_bridge.h` 要用根目录的 `music_scanner.h`（`Makefile.cli` 本来就有，库的 Makefile 漏了）。
2. `AlsaOut.cpp`：`<execinfo.h>` 是 glibc 专有，musl 没有；只有 `#include`、没有调用，用 `__GLIBC__` 保护。
3. `music_player.cpp`：本树 `atcmediaplayer.h` 里 `MediaPlayer::play()` **不带路径**（AWTK 树里是 `play(const std::string&)`）。改成编译期探测：
   传不进路径时不创建硬件播放器，直接走软解（musikcube + FFmpeg + ALSA）；旧头文件下行为不变。
   若要在本树用硬件解码，需要先弄清 `atcmediaplayer.h` 里设置文件路径的接口。

`tools/add_musicplayer_pkg.sh` 现在会：写 `musicplayer.mk`（两个命令行小工具 `musicplayer_test`/`music_app_cli` 编译/安装失败不再拖垮整包）、
在 defconfig 里启用 ffmpeg（仅解码：`DECODERS/DEMUXERS/PARSERS` 限定列表）与 `musicplayer`。ffmpeg 其余项（编码器/复用器/滤镜）仍是全开，
ext4 的 204 MiB 分区放得下；若换到小分区，可再加 `BR2_PACKAGE_FFMPEG_ENCODERS=""`、`..._MUXERS=""`、`..._PROTOCOLS="file"`、`..._FILTERS="anull"` 并关掉 INDEVS/OUTDEVS（未实测体积）。

## 已踩过的坑（续）
6. 本树 `allmake.sh` 先于 buildroot 打印的 `Can't open perl script …/PartitionUtility_nor.pl` 只是噪音；真正的镜像失败看日志里的 `image size … exceeded` / `[ERROR]`。
7. `BR2_PACKAGE_LVGL_HOME=y` 与 `BR2_PACKAGE_HCN_APPLICATION=y` 同开 → 上板后两个 UI 进程抢屏幕；刷机前关掉前者。
8. `tools/enable_music_in_hcn_application.sh` 早期只复制 `ui_Music.[ch]`/`music_focus.[ch]`/字库，漏了后加的 `ui_Music_cf`/`music_art`/`music_search`/`music_pinyin`/`music_pinyin_table`/`ui_Music_style.h`，
   表现为 `hcn_application` 的 CMake 在 `lvgl/lvgl_app/CMakeLists.txt` 的 `add_library(lvgl_app …)` 处报 `Cannot find source file` + `No SOURCES given`。
   现在脚本每次都整体同步 `adapters/music` 与 `lvgl/lvgl_app`，并在末尾核对 CMake 音乐块列出的源文件是否都在；已打过补丁的树重跑即可补齐。

## 分区表观察（待同事确认，未在板上验证）
本树 `nand-256-ddr-256` 生成的 `scatter.nand.ext4.xml` **没有 `boot_misc` 分区**，logo 之后的分区（vba/metazone/kernel/system/usrdata）比设备上现有布局整体前移 2 MiB（大小不变）。
`allmake.sh` 里 `boot_misc.bin` 是用 `dd … of=ac83xx.dtb.bin seek=$PRELOADER_DTB_OFFSET` 合并进 dtb 镜像的（`release_images/ac83xx.dtb.bin` 比 `out/images/` 里的大），
所以这是本树的布局设计，不是漏项；但它和设备上较早那版（有独立 `boot_misc` 分区）不同。刷机应使用 `release_images/` 里的整包，按 ATCUpgradeTool 的整片流程，不要把本次产物单个分区地刷到旧布局的设备上。
9. 启用 ffmpeg 后 `alsa-plugins`（DA 树里的厂商包）的 `pcm_a52.c` 编译失败（`avcodec_alloc_context`/`CODEC_ID_AC3`/`CH_LAYOUT_*` 未声明）：configure 在 staging 里发现 libavcodec 就会编 AC3 插件，
   而它用的是早已移除的 FFmpeg 接口。音乐用不到这个插件，给该包加 `ALSA_PLUGINS_CONF_OPTS += --disable-avcodec`（`configure` 有 `--enable-avcodec` 开关）。`add_musicplayer_pkg.sh` 会自动加（原文件留 `.orig`）。
   判断是否生效看日志里有没有编译 `pcm_a52`；`config.log` 里仍会出现 `creating a52/Makefile`，那不代表没生效。
10. **关掉某个包后，它已装进 `out/target` 的文件不会被 Buildroot 删除。** 例：基线时 `BR2_PACKAGE_LVGL_HOME=y`，之后关掉，`out/target/ac83xx/etc/init.d/S01lvgl_home` 和 `usr/bin/lvgl_home` 仍在，
    镜像里会同时起两个界面进程。处理：把这两个文件移出 `out/target`，再整编一次（只重做目标目录整理和镜像）。`enable_music_in_hcn_application.sh` 发现残留会提示。
11. `nohup ./allmake.sh … &` 结束时 shell 显示的 `Done` 不代表成功（失败时 `allmake.sh` 也以 0 退出，只在日志里打印 `Build error, please check the log`）。
    判断成败：`grep -n 'Build error' <日志>`；镜像阶段看 `partition size is`、`image size … exceeded`、`[CHECK] … limit=…`、`[ERROR]`。`u-boot` 首次 `make` 打印的 `System not configured - see README` + `Error 1` 是正常的（它随后自己配置）。
12. 整编日志可能有近 20 万行：只 `grep -nE 'Error [0-9]+|error:|CMake Error'`，并看出错行之前约 30 行；编译输出里的 `PRINT_ERROR(...)` 之类是源码文本，不是错误。

## 核对镜像内容（不用刷机，不需要 root）
```
debugfs -R 'ls /etc/init.d'                    release_images/system.img.ext4   # 只应有 S01hcn_application，没有 S01lvgl_home
debugfs -R 'stat /usr/bin/hcn_application'      release_images/system.img.ext4   # Size 应与 out/target 里的一致
debugfs -R 'ls -l /usr/lib'                     release_images/system.img.ext4 | grep -E 'libmusicplayer|libavcodec|libavformat|libasound'
debugfs -R 'cat /etc/ld-musl-armhf.path'        release_images/system.img.ext4   # 无此文件 = 走 musl 默认搜索路径（含 /usr/lib），没问题
```
`S01hcn_application` 直接启动 `/usr/bin/hcn_application`，并给 `LD_LIBRARY_PATH` 前置 `/usr/lib/hcn_application`（镜像里没有这个目录，无害）。
刷机用 `release_images/` 整包（里面的 `ac83xx.dtb.bin` 带 boot_misc，比 `out/images/` 里的大），按 ATCUpgradeTool 的整片流程；不要把单个分区镜像刷到旧布局的设备上。

## 上板后检查（未验证，按序）
```
cat /proc/mtd | head -20                                   # 分区与 scatter 是否一致
ps | grep -E 'hcn_application|lvgl_home'                   # 只应有 hcn_application
LD_TRACE_LOADED_OBJECTS=1 /usr/bin/hcn_application 2>&1 | grep -iE 'not found|error'     # 缺库
logread 2>/dev/null | grep -iE 'hcn_application|music' | tail -20
```
- Home 界面的「音乐」图标调用 `videofocusmanager_open_app(APP_MUSIC)`，并写一行 syslog `music app open requested, ret=…`（0 = 成功）。`hcn_app_config.cmake` 的 UI 变体是 `auto`：
  若板上起来的是 template（仪表按键）变体，则没有这个图标，需要用 `hcn_ui_shared_apps_open_music()` 另行接按键（产品决策）。
- 界面起不来/崩溃：`killall hcn_application; LD_LIBRARY_PATH=/usr/lib/hcn_application /usr/bin/hcn_application 2>&1 | head -60`，看 `[ui]`、`music` 开头的行。
- 插 U 盘放 mp3：`ls /data/music` 应出现数据库/状态文件（程序自己 `mkdir`）。没有声音时可试 `-DMUSIC_FOCUS_SEND_POLICY=1`；全屏背景/封面流卡顿可试 `-DMUSIC_UI_BACKDROP=0`。

