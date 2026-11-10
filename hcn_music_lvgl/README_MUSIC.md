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
lvgl/lvgl_app/screens/ui_Music_cf.[ch]     封面流
lvgl/lvgl_app/screens/music_art.[ch]       封面解码 / 异步加载 + LRU 缓存 / 背景晕染（工作线程不碰 LVGL）
lvgl/lvgl_app/screens/music_search.[ch]    搜索（子串 + 拼音首字母，扫描当前设备全部歌曲）
lvgl/lvgl_app/screens/music_pinyin.[ch]    拼音首字母匹配；music_pinyin_table.c 由 tools/gen_pinyin_table.py 生成
lvgl/lvgl_app/screens/ui_Music_style.h     配色/字体常量，MUSIC_UI_BACKDROP 开关
lvgl/lvgl_app/frameworks/music_focus.[ch]   音频焦点桥（对应 bt_focus_adapter.c）
lvgl/lvgl_app/fonts/music_font_{20,32}.c    生成的 CJK 字库（GB2312 一级 3755 字 + ASCII + 常用符号；压缩 4bpp，20px 约 0.57 MB + 32px 约 1.01 MB flash）
tools/add_musicplayer_pkg.sh         第 1 步：musicplayer 包（工具链无关版 mk）装进 DA 树
tools/enable_music_in_hcn_application.sh   第 2 步：音乐界面 + hcn_application.mk 改动
tools/gen_pinyin_table.py            拼音首字母表生成（pip install pypinyin）
tools/check_font_coverage.py         界面字符串是否都在字库里
tools/gen_music_fonts.sh             字库生成脚本（python3+fonttools、node+npm、Noto Sans CJK）
tools/music_ui_harness/              宿主机无头测试与截图（见下）
integration.patch                    对 6 个已有文件的改动（零上下文格式，只含新增行；`patch -p1` 照常应用，不带任何周围的 HCN 源码）
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

## 功能（与原 AWTK 界面对照）

| 功能 | 做法 | 与原版的区别 |
|---|---|---|
| 多存储设备切换 | 顶栏设备胶囊（`U盘 · 48 首` / `SD卡 · 12 首`），≥2 个设备时出现，点按 `music_app_switch_device`，当前设备高亮 | 原版是侧栏 |
| 搜索 | 「搜索」标签 + 输入框 + 键盘；结果复用虚拟列表，点结果播放（先 `restore_full_playlist` 再按设备列表下标播放） | 板级 `LV_USE_IME_PINYIN=0`，键盘只能打拉丁字母，所以**自带拼音首字母匹配**：`zjl` → 周杰伦；多音字任一读音都算（乐 y/l）。`music_pinyin_table.c` 由 `tools/gen_pinyin_table.py` 生成（GB2312 6763 字，约 40 KB flash） |
| 快进/快退 | 上一首/下一首**长按**，每 350 ms 跳 `MUSIC_APP_SEEK_STEP_MS`（5 s）；短按仍是切歌，长按不会连带切歌 | 原版是两个独立按钮，控制行放不下 |
| 封面流 | 封面上的「封面流」胶囊把右侧面板切成斜轴卡片流（中心 1.12×，远处 0.55× 并渐隐，±4 张）。上下拖动浏览，点侧边卡拉到中心，点中心卡播放；封面由 `music_art` 工作线程异步解码，24 张 LRU 缓存；播放曲目变化时自动跟随 | 原版是 NanoVG 绘制；这里是 LVGL 图片缩放，观感接近、渲染不同 |
| 模糊封面背景 | 封面取 8×5 色块后放大成 256×150 的色彩晕染，4 倍显示在最底层，左侧卡片半透明让它透出来 | 是色彩晕染，不是高斯模糊；上板若全屏缩放绘制偏慢，编译加 `-DMUSIC_UI_BACKDROP=0` 关掉，其余不受影响 |
| 歌词 | 当前行超长时横向循环滚动，其余行单行省略；上下边缘渐隐 | 垂直滚动仍是 LVGL 的整行动画 |

内存：封面缓存最多 24×100 KB + 背景 150 KB + 当前封面 230 KB。工作线程不碰 LVGL 堆（`LV_USE_OS` 为 NONE），
只用 malloc + stb_image，结果经 `idle_queue` 交回 GUI 线程。

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
| 2 | 界面场景 88 项断言，用**脚本化的真实指针**（按下/拖动/长按/抬起，含点击键盘 q、w 键的实际位置）：滚动回收、各浏览模式与下钻、歌词跟随与跑马灯、设备切换、长按快进快退（且不切歌）、搜索全流程、封面流拖动/点按/快速翻到头；创建销毁 25 轮（含封面流、搜索、设备切换） | 通过 |
| 2b | 拼音首字母 + 搜索 64 项（多音字、畸形 UTF-8、超长文本/查询、结果截断）；封面模块 60 项（解码、圆角、背景、异步、去重、LRU、关闭时 60 个未完成请求）；字库覆盖检查 | ASan+UBSan，封面另加 TSan，通过 |
| 3 | 音频焦点 224 项断言，对着**你仓库里真实的 `audiofocusmanager.c`**（来电打断/恢复、手动暂停、蓝牙音乐抢占、通话中不抢声、50 次连续打断） | 通过 |
| 4 | 2、3 整体在 ASan+UBSan+LeakSanitizer 下重跑 | 0 错误 0 泄漏 |
| 5 | 未改动的 `music_app.c` / `favorite_manager.c` / `usb_monitor.c` 在 C99 下对着替换层编译；`adapters/music` 用真实 CMakeLists 配置并编译 | 通过 |

**测试用的 LVGL 与配置（已对着真实环境跑过）：** `run_tests.sh` 设 `LVGL_DIR=<树>/source/packages/cluster`（板级 LVGL 魔改源码）和
`BOARD_LV_CONF=<树>/source/packages/cluster/lvgl/lv_conf.h`（板级真实配置）即可；脚本只关掉宿主机用不了的开关
（OpenGLES/GLFW、fbdev、evdev、`ATCIMAGE`、PNG/JPEG、NEON 汇编、性能/内存监视器、日志）。
不设这两个变量则用 vanilla LVGL v9.3.0 + 目录里自带的 9/21 版 lv_conf 副本。两种都全绿。
（对着魔改源码需要包里带 `source/packages/cluster/lvgl/lvgl_private.h`；我这次用的是 vanilla 的同名文件补上的。）

板级配置里与我早先那份副本不同、影响判断的几处：
- **`LV_USE_STDLIB_MALLOC = LV_STDLIB_CLIB`**：LVGL 用 libc `malloc`，**没有内存池**（`LV_MEM_SIZE` 8 MB 不生效，我早先“8 MB 池”的说法不对）。
  好处是 LeakSanitizer 能看到 LVGL 的每次分配，上面第 4 项因此比之前更严格；`lv_mem_monitor` 在此模式下无数据，所以那条“堆差 0 字节”的数值检查被跳过。
  界面的主要常驻内存是 240×240 ARGB8888 封面缓冲（230 KB）加控件对象（内置分配器下测得 LVGL 内部占用约 277 KB）；板上 RSS 还没测，请上板后看 `/proc/<pid>/status` 的 `VmRSS`。
- `LV_USE_DRAW_SW_ASM = NEON`、`LV_USE_DRAW_OPENGLES = 1`（GPU 绘制单元开着）、`LV_USE_LIBPNG` / `LV_USE_LIBJPEG_TURBO` / `LV_USE_ATCIMAGE` 都开着：
  对我的代码没有接口影响（封面走 stb_image 预解码），但板上渲染走的是另一条绘制路径，**界面性能要上板实测**。
- **`LV_USE_PERF_MONITOR` / `LV_USE_MEM_MONITOR` 仍是 1**：真机屏幕左下/右下会叠加调试信息（我的截图里已关掉）。
  左下角那块会压到收藏按钮下沿，正式版建议关掉。

字库 flash 约 1.6 MB。

**没有验证、需要上板确认的：** GLFW/EGL 实际显示与触摸、真实 libmusicplayer 播放与 U 盘热插拔、`lv_atcimage` 与 GPU 绘制路径的实际表现、启动耗时、整机 flash 余量。

## 已知限制与顺带发现

- **新增功能的上板风险**：全屏 4 倍缩放的背景、封面流的图片缩放，在板级 GL 绘制单元下的流畅度没测过（背景可用 `-DMUSIC_UI_BACKDROP=0` 关掉）；A7 上封面解码耗时、键盘按键手感同样待上板确认。
- 搜索只做**首字母**（`zjl`），不支持全拼（`zhoujielun`）；键盘是 LVGL 自带拉丁键盘。
- 封面流点中心卡才播放，滑动只浏览；播放曲目变化时自动回到当前曲目。

- **启动时机**：`music_adapter_init()` 在 `hcn_ui_shared_apps_init()` 里，开机就扫盘并恢复上次状态。在意启动耗时可改成首次进入时再初始化。
- **只做了触摸**，没有按键/旋钮焦点导航（template 变体是按键驱动的，要用的话还得做）。
- **ACC**：`music_app_on_acc_off/on` 没接，需要 vehicle_services 的 ACC 信号。
- **封面解码在主线程同步做**（有 2048×2048 的上限保护）；大封面建议后续挪到工作线程。
- **收藏页点歌**沿用 AWTK 版的做法：恢复完整列表后线性查找再播放，所以上一首/下一首不限定在收藏内（原版的 TODO）。
- **字符集**是 GB2312 一级，生僻字显示为空。字库要求编译时 `LV_FONT_FMT_TXT_LARGE=1` 且 `LV_USE_FONT_COMPRESSED=1`（板级 `cluster/lvgl/lv_conf.h` 已核对两项都是 1；换板子时用 `MUSIC_FONT_COMPRESS=0` 重新生成）。
- 界面文字是写死的中文，没接 `aic_lang` 之类的多语言。
- 上游代码里看到的问题（我没改）：`music_player.h` 的 `PLAYER_STATE_ERROR` 和 `PLAYER_STATE_TRACK_FINISHED` 值都是 4，无法区分；
  `music_app.c:1618,2077`、`favorite_manager.c:103,106,140` 的 `snprintf` 在 `-Wall` 下有截断警告（超长路径会被静默截断）。
