# DA 树 NAND-256 板：遗留问题与待办（2026-11-13，11-15 更新）

根因与过程见 [`DIAG_LOG_2026-11-12.md`](DIAG_LOG_2026-11-12.md)。下面是还没做完的事。

## 状态（2026-11-15）
正式修复已完成并上板：旧版 `atc_lib.a` + 关 `LVGL_HOME` + ffmpeg/musicplayer，allmake 整编，板上连续运行 2.7 天无重启，Home → Music 能进 LVGL 音乐界面。
插 U 盘崩溃已确诊、修复已提交，**待板上验证**（第 6 节）。

## 1. 闪屏——已确认并解决
adbfirst 包上 `ps` 同时有 `hcn_application` 和 `lvgl_home`；去掉 `S01lvgl_home` 后不再闪。正式做法：defconfig 关 `BR2_PACKAGE_LVGL_HOME`，并清掉 `out/target` 里的残留（见 `BUILD_ENV_NOTES.md` 第 10、13 条）。
`atcpicture_decoder` 也是 `lvgl_home` 包编出来的，随之消失，属预期。

## 2. 完整系统下 adb 进不去——原因未知，仍存在
- 正式包刷上后依旧进不去；`mk_adbfirst.sh` 做的开发包（adb 在 `rcS` 之前起）稳定可用，目前所有上板调试都靠它，**每次出包都要另做一份 adbfirst 包**。
- 待做：对照包去掉 `S00early_modutils` 和 `S70modutils`，看是不是模块加载影响了 USB gadget。
- 本板没法接串口，adb 起来之前的问题只能靠现象判断。

## 3. 正式修复——已完成
按原计划用旧版 `atc_lib.a`（MD5 `21c5372b…`）覆盖后 allmake 整编。核对：`vmlinux` 里有 `[MTKLib]Version 2026020331` 及只在旧版库出现的 `mtklib dettach mtd` 等串；板上 dmesg `FTL Lib Version` 同为 `2026020331`。
（`vmlinux` 里另有 `Version 2012112101`、`ADD MTKLib Time` 等串，两份库里都没有，来自内核自身源码，与换库无关。）

## 4. 要向同事确认
> DA 树 `prebuild/atc_ftl/atc_lib.a`（MD5 `129f1bfe…`，无版本串，9/16）用在 NAND 256MB 的板子上，内核启动约 18 秒后会重启（与 NAND 设备有无无关；挡掉 `mt33xx_mtklib_init` 后重启消失）。换成旧树里的版本（MD5 `21c5372b…`，`[MTKLib]Version 2026020331`，2026-06-10 提交）后，其余完全不变，内核稳定、adb 正常。想确认：DA 这份库是给哪种板子、哪个版本做的？NAND 板上应该用哪一份？
> 另外 `bootanidrv.c` 的 `rsv_free_thread` 里 `wait_event_interruptible(rsv_free_queue, 1)` 条件恒真，内核启动即释放开机动画保留内存（本次实测不是重启原因，但应该是 `g_arm2_vba_exit`）。

## 5. 音乐功能现状
- UI 变体：`HCN_APP_CFG_UI_VARIANT=auto`，板上因 template 素材缺失走 fallback（日志 `[ui] fallback UI selected`），Home 的 Music 图标调 `videofocusmanager_open_app(APP_MUSIC)` 返回 0，能打开本仓库的 LVGL 音乐界面。
- 解码：DA 树 `MediaPlayer::play()` 不带路径，编译期直接走软解（GaplessTransport + FFmpeg + ALSA），硬解没有尝试；启动日志里 `MediaPlayer setup failed, falling back` 这句有误导，硬解并未 setup。硬软结合需要先查清 DA 版 `atcmediaplayer.h` 设置路径的接口。
- 待修（只读代码得出，未上板验证）：`music_player.h` 里 `PLAYER_STATE_TRACK_FINISHED` 与 `PLAYER_STATE_ERROR` 都是 4，`music_app.c` 先判 ERROR，正常播完会走错误分支（发 `APP_EVENT_ERROR`、500 ms 后切歌、累计 `error_count`）。

## 6. 插 U 盘即崩溃——已确诊，修复待板上验证
现象：插 U 盘（Home 或音乐界面）后 `terminate called after throwing an instance of 'std::bad_alloc'`，进程退出，屏幕停在最后一帧。
原因：`music_app.c` 的 `add_device()`（以及缓存命中路径的临时列表）调用 `music_list_create(MUSIC_MAX_FILES)`，`music_scanner.cpp` 里据此 `vec.reserve(65536)`；`sizeof(MusicInfo)` = 2588 B，共约 162 MiB，本板 `MemTotal` 约 179 MB，分配失败抛异常且无人捕获。512 MB 的 AWTK 板上不出现。
修复（本次提交）：`music_list_create` 预留上限改为 `MUSIC_INIT_CAPACITY`（512），vector 按需增长；`reserve`/`emplace_back` 捕获 `std::bad_alloc` 返回空指针（调用方原本就处理）。只影响 `libmusicplayer.so`。
验证方法：重编 `musicplayer`，把新库推到板上 `/tmp/lib`，`LD_LIBRARY_PATH=/tmp/lib:/usr/lib/hcn_application /usr/bin/hcn_application` 前台跑，进 Music 插 U 盘。

## 7. 其他
- DA 树 `prebuild/` 里还有 `multimedia`（14 个文件）、`opengles`、`atcsurface`、`libsettings_atc`、`imgresz`、`audio` 与旧树不同，用户态库，**未验证，不要盲换**。
- `H2`/`H3`（旧 `atcftlcore.c` 配旧/DA 库）已编好未刷，只用于区分"库"和"`atcftlcore.c`"各自的责任，不影响修法。
- 126 上的备份与临时物（`~/dylan/` 即 `/work2/tanyunlong/dylan/`）：`atc_lib.a.DA_orig`、`bootanidrv.c.orig`、`ac83xx_kernel_defconfig.orig`、`displayinit.c.orig`、`metazone_drv.c.orig`、`aic8800_Kconfig.orig`；诊断包 `diag_*`、`DA_G12_full`、`DA_G12_adbfirst`、`DA_G12_hcn_only`、`release_images_DA_bootanifix_1112`；本轮 `release_images_DA_music_1113_nolibs`（缺 C++ 运行库的坏包，可删）、`DA_music_fbtest`（当前上板的 adbfirst 包）、`cmp_rootfs_1113/`（镜像对比解包，可删）、`ui_Home.c.orig_1115`、`music_scanner.cpp.orig_1115`。收尾时清理：`rm -rf $T/out/build/xt_* ~/dylan/root_G*`。
- 确认 DA 树里的 `bootanidrv.c` 已还原（曾被注释过一行）。
