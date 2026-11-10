# 本地音乐（LVGL 版）— hcn_application

AWTK 版 `musicplayer_awtk` 的 LVGL 替代：U 盘/SD 卡本地音乐，1024×600 触摸界面。
业务逻辑（热插拔、扫描、ID3、播放、收藏、歌词、封面）**原样复用** `music_app.c`，只重写了界面。

## 目录（相对 `source/vendor/hcn/application/`，解压覆盖即可）

```
adapters/music/                      适配层（纯 C，不含 LVGL；和 adapters/bt 同一套约定）
  include/music_adapter.h              init / poll / deinit + 事件钩子
  include/compat/awtk.h                AWTK 三个原语的替换（idle_queue / timer_add / timer_remove）
  src/music_mainloop.c                 上述原语在主循环上的实现（线程安全队列，poll 时在 GUI 线程派发）
  src/music_adapter.c
  CMakeLists.txt                       复用 musicplayer 包里 music_app.c / favorite_manager.c / usb_monitor.c
lvgl/lvgl_app/screens/ui_Music.[ch]  界面（只显示；数据与操作全部走 music_app.h）
lvgl/lvgl_app/frameworks/music_focus.[ch]   音频焦点桥（对应 bt_focus_adapter.c）
lvgl/lvgl_app/fonts/music_font_{20,32}.c    生成的 CJK 字库（GB2312 一级 3755 字 + ASCII + 常用符号；压缩 4bpp，20px 约 0.57 MB + 32px 约 1.01 MB flash）
tools/add_musicplayer_pkg.sh         第 1 步：musicplayer 包（工具链无关版 mk）装进 DA 树
tools/enable_music_in_hcn_application.sh   第 2 步：音乐界面 + hcn_application.mk 改动
tools/gen_music_fonts.sh             字库生成脚本（python3+fonttools、node+npm、Noto Sans CJK）
tools/music_ui_harness/              宿主机无头测试与截图（见下）
integration.patch                    对你已有 6 个文件的改动：+79 / -1 行
screenshots/                         15 个界面状态
```

## 接入（DA 树，musl 工具链）

**推荐用脚本，分两步，每步之后整编一次再进下一步。** 前提：这棵树已经原样整编过（`hcn_application` 能编过）。

```sh
# 第 1 步：musicplayer 包（libmusicplayer.so + 命令行测试工具 music_app_cli / musicplayer_test）
tools/add_musicplayer_pkg.sh <树> <musicplayer 源码备份.tgz>
#   末尾会列出 ffmpeg / alsa / atcmediaplayer 在 sysroot 里是否齐全；MISSING 的要先在 defconfig 里打开
#   整编后确认 out/staging/usr/lib/libmusicplayer.so

# 第 2 步：把音乐界面放进 hcn_application，并改 hcn_application.mk
tools/enable_music_in_hcn_application.sh <树> <本压缩包解开的目录>
rm -rf <树>/out/build/ac83xx/hcn_application*      # 这棵树的 allmake.sh 不支持 <pkg>-dirclean 透传，直接删构建目录
```

两个脚本都是幂等的，改过的文件会留 `*.orig`，补丁 dry-run 失败时不会动任何东西。
**为什么要重写 `musicplayer.mk`：** 原来的 mk 和 Makefile 默认值里写死了 `arm-buildroot-linux-gnueabi`（glibc），
这棵树是 `arm-buildroot-linux-musleabihf`（musl）；新的 mk 全部改用 `$(TARGET_CXX)` / `$(TARGET_CC)` / `$(STAGING_DIR)`。
`libmusicplayer.so` 链接 `libatcmediaplayer`、ffmpeg（avformat/avcodec/avutil/swresample）、`libasound`、`libatomic`。

下面是脚本做的事，需要手工做时照这个来：

1. 解压覆盖到 application 根目录，再 `patch -p1 < integration.patch`
   （改动：`ui_shared_apps.[ch]` 注册 `APP_MUSIC` 并接线、`main.c` 主循环加 `music_adapter_poll()`、
   顶层与 `lvgl_app` 的 CMakeLists、`hcn_app_config.cmake` 加 `HCN_APP_CFG_MUSIC`）。
2. **`configs/package/hcn_application/hcn_application.mk` 改两处**（build 仓库 HCN_AC8215 分支 b6e14e1 起有这个文件；
   已按真实文件核对）。这个 mk **不读** `hcn_app_config.cmake`，依赖与参数都是写死的，所以要手工加：
   ```make
   HCN_APPLICATION_DEPENDENCIES = lvgl lib_mw vehicle_services directrender musicplayer
   ...
   HCN_APPLICATION_CONF_OPTS = \
   	-DHCN_APPLICATION_VS_INC=... \                    # 原有五项不动，最后一项末尾补 " \"
   	-DHCN_APP_SETTINGS_INC=$(TOPDIR)/../source/packages/graphics/libsettings_atc/include \
   	-DHCN_APP_MUSIC_SRC=$(TOPDIR)/../source/packages/application/musicplayer
   ```
   原因：application 是整树 rsync 到构建目录再编的，拷贝树里指向 `source/packages/...` 的相对路径失效，必须注入绝对路径
   （和现有 `HCN_APP_SETTINGS_INC` 同理）。因为 mk 不看配置文件，`HCN_APP_CFG_MUSIC` 只控制 CMake 一侧：
   关掉音乐时请把 `musicplayer` 依赖一起去掉。改了 CONF_OPTS 后用 `hcn_application-reconfigure` 或 `-dirclean`，`-rebuild` 不会重跑 configure。
3. **`musicplayer` 包要在这棵树里存在**（`libmusicplayer.so`：解码/ALSA 输出/扫描/DB/stb_image 实现）：
   `configs/package/application/musicplayer/{Config.in,musicplayer.mk}` + `configs/Config.in` 里对应的 `source` 行 +
   defconfig 里 `BR2_PACKAGE_MUSICPLAYER=y`，源码放 `source/packages/application/musicplayer`。
   **不需要** `musicplayer_awtk`（它依赖 AWTK，LVGL 版里不会选中）。
4. 开关：`hcn_app_config.cmake` 里 `set(HCN_APP_CFG_MUSIC ON)`。`hcn_app_switch.sh` 想支持 `music on|off`，照 `carplay` 的写法加一个关键字即可。
5. 入口：fallback 变体的 Home「音乐」图标原来就调 `videofocusmanager_open_app(APP_MUSIC)`，注册后直接生效。
   template（仪表按键）变体用 `hcn_ui_shared_apps_open_music()`，**选哪个按键组合是产品决策**，我没有改 `home_page_key.c`。

## 设计要点

```
USB/扫描/播放线程 ─ idle_queue ─► 线程安全队列 ──(music_adapter_poll, 主循环)──► GUI 线程
                                                      ├─ music_focus_on_app_event  → audiofocusmanager
                                                      └─ ui_Music_on_app_event     → LVGL 控件
```
- `LV_USE_OS` 是 `LV_OS_NONE`，所有 LVGL 调用必须在 GUI 线程。`music_app.c` 只用了 `idle_queue`/`timer_add`/`timer_remove`，
  所以只替换这三个原语，业务代码一行没改。
- **不能定义 `AWTK_STUB`**：`music_app.h` 会改去包含上游基于线程的 `awtk_stub.h`（引号包含先搜索包含者自己的目录，
  `-I` 顺序压不过它），UI 回调就会跑到工作线程上。`music_adapter.c` 里有 `#error` 金丝雀专门防这个。
- 列表是虚拟列表：8 个行对象循环使用，6.5 万首也只占固定内存；扫描中只更新计数，不重建行。
- 封面：`music_app_get_album_art()` → stb_image（实现在 `libmusicplayer.so` 里，符号已核实导出）→ 240×240 ARGB8888。
  板级 `LV_CACHE_DEF_SIZE=0`（图片缓存关闭），所以预解码成缓冲，不能把 JPEG 当图源。
- 音频焦点规则：播放→申请 `FOCUS_MUSIC`；被瞬时抢占→暂停，焦点回来才恢复（用户手动暂停的不恢复）；被永久抢占→暂停且不自动恢复；
  停止后延迟 1 s 释放（切歌不抖动）。本地播放走 libatcmediaplayer，**默认不发** `send_policy_event(MM_HANDLE,…)`
  （蓝牙音乐要发是因为它不经媒体播放器）；上板后如果路由没切换，加 `-DMUSIC_FOCUS_SEND_POLICY=1`。

## 验证（宿主机，vanilla LVGL v9.3.0 + 板级 lv_conf，只关掉了宿主机没有的 GL/fbdev/evdev 驱动）

`tools/music_ui_harness/run_tests.sh` 一键跑（首次会自动 clone LVGL v9.3.0，约 1 分钟）：

| 项 | 内容 | 结果 |
|---|---|---|
| 1 | 替换层：8 个生产线程 ×2000 个 idle，定时器一次/重复/重入/自删除 | ASan+UBSan、TSan 通过 |
| 2 | 界面场景 33 项断言（滚动回收、各浏览模式与下钻、歌词跟随、空设备/扫描中/无收藏…）+ 25 轮创建销毁后 LVGL 堆差 **0 字节** | 通过 |
| 3 | 音频焦点 224 项断言，对着**你仓库里真实的 `audiofocusmanager.c`**（来电打断/恢复、手动暂停、蓝牙音乐抢占、通话中不抢声、50 次连续打断） | 通过 |
| 4 | 2、3 整体在 ASan+UBSan+LeakSanitizer 下重跑 | 0 错误 0 泄漏 |
| 5 | 未改动的 `music_app.c` / `favorite_manager.c` / `usb_monitor.c` 在 C99 下对着替换层编译；`adapters/music` 用真实 CMakeLists 配置并编译 | 通过 |

资源：界面开着时 LVGL 堆占用 277 KB（8 MB 池里，含 230 KB 封面缓冲）；字库 flash 约 1.6 MB。

**没有验证、需要上板确认的：** GLFW/EGL 实际显示与触摸（板级 `liblvgl.a` 是魔改版，`lv_font_source_han_sans_sc_16_full_cjk`
这类符号 vanilla 里就没有）、真实 libmusicplayer 播放与 U 盘热插拔、`lv_atcimage` 路径、启动耗时、整机 flash 余量。
测试用的 `lv_conf` 是 9/21 提交里的副本；已用真实板级文件核对过字体相关项（`LV_MEM_SIZE`=8 MB、`LV_CACHE_DEF_SIZE`=0、`LV_FONT_FMT_TXT_LARGE`=1、`LV_USE_FONT_COMPRESSED`=1、默认字体 `lv_font_source_han_sans_sc_16_full_cjk`）全部一致，其余项还没逐行比对。那份里 `LV_USE_PERF_MONITOR`/`LV_USE_MEM_MONITOR` 是开着的，
会在屏幕左下/右下叠加调试信息（我的截图里已关掉）。

## 已知限制与顺带发现

- **启动时机**：`music_adapter_init()` 在 `hcn_ui_shared_apps_init()` 里，开机就扫盘并恢复上次状态。在意启动耗时可改成首次进入时再初始化。
- **只做了触摸**，没有按键/旋钮焦点导航（template 变体是按键驱动的，要用的话还得做）。
- **ACC**：`music_app_on_acc_off/on` 没接，需要 vehicle_services 的 ACC 信号。
- **封面解码在主线程同步做**（有 2048×2048 的上限保护）；大封面建议后续挪到工作线程。
- **收藏页点歌**沿用 AWTK 版的做法：恢复完整列表后线性查找再播放，所以上一首/下一首不限定在收藏内（原版的 TODO）。
- **字符集**是 GB2312 一级，生僻字显示为空。字库要求编译时 `LV_FONT_FMT_TXT_LARGE=1` 且 `LV_USE_FONT_COMPRESSED=1`（板级 `cluster/lvgl/lv_conf.h` 已核对两项都是 1；换板子时用 `MUSIC_FONT_COMPRESS=0` 重新生成）。
- 界面文字是写死的中文，没接 `aic_lang` 之类的多语言。
- 上游代码里看到的问题（我没改）：`music_player.h` 的 `PLAYER_STATE_ERROR` 和 `PLAYER_STATE_TRACK_FINISHED` 值都是 4，无法区分；
  `music_app.c:1618,2077`、`favorite_manager.c:103,106,140` 的 `snprintf` 在 `-Wall` 下有截断警告（超长路径会被静默截断）。
