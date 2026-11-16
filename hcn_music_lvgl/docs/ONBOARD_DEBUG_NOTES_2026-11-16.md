# 上板调试笔记（2026-11-16）

只记已验证的事实、明确标注的推断、以及还没解决的事。文中没有任何凭据。

## 1. 三个程序版本（MD5 对照，板上 `md5sum /usr/bin/hcn_application`）

| MD5 | 编译时间 | 内容 |
|---|---|---|
| `aba05ee0c4adb2028610d29ba9f5513a` | 14:19 | 音乐入口键被 `#ifdef HCN_APP_HAVE_MUSIC` 整段编译掉，**无效** |
| `5b87f8e3cedd269dde6b83fe58d9b756` | 14:42 | 音乐入口可用，背景仍是模糊封面 |
| `d6a1e09fdbe0346abd6f0a7ba28ecb33` | 16:19 | 关闭模糊背景（`MUSIC_UI_BACKDROP 0`），板上确认背景为纯色 |

教训：`adb push … /tmp/hcn_application_new` 再运行，会把烧录的程序顶掉。曾因此把旧程序当成新版测了几轮，看到"背景还在"。测试前先 `adb shell md5sum` 确认，或者直接用 `run_app.sh /usr/bin/hcn_application` 跑烧录版。

## 2. 音乐入口（template 变体）

- 板上 `UI_VARIANT=auto` 选中了 template（仪表）变体，柯基动画是它的素材。template 的按键里没有音乐。
- `home_page_key.c` 里加了 `CLICK_SHORT_BACK -> hcn_ui_shared_apps_open_music()`；屏幕上的虚拟 BACK 键在左侧中间（`custom.c` 里 `TOUCH_SCREEN` 已定义）。
- 不能再用 `#ifdef HCN_APP_HAVE_MUSIC` 守卫：`lvgl_ui_template` 是**独立静态库**，该宏只传给了 `lvgl_app`。`hcn_ui_shared_apps_open_music()` 在 `ui_shared_apps.c` 里无条件定义，内部自己判断。

## 3. 板上收集日志（没有串口）

脚本在 `hcn_music_lvgl/tools/board_debug/`：

```
adb push run_app.sh /tmp/run_app.sh ; adb push monitor.sh /tmp/monitor.sh
adb exec-out sh /tmp/run_app.sh /usr/bin/hcn_application | Tee-Object -FilePath app.log   # 窗口 1
adb exec-out sh /tmp/monitor.sh | Tee-Object -FilePath mem.log                            # 窗口 2
```

- `run_app.sh`：每行加开机秒数（读 `/proc/uptime`，不 fork），退出后打印退出码（137=SIGKILL，139=段错误）、meminfo、dmesg 末尾。
- `monitor.sh`：每 2 秒一行，带同样的开机秒数，可与 app.log 对齐。含系统/进程 CPU、最忙三个线程及用户态/内核态拆分（`u9+k3`）、**应用以外最忙三个进程 `oth=[...]`**、内存、`/tmp` 占用。输出只含 ASCII。
- 程序的 `printf` 不进 `syslog.log`，grep syslog 找不到。`/tmp` 是内存盘，重启清空，脚本和程序要重新 push。
- Windows PowerShell 默认 GBK，中文显示乱码是显示问题，不是程序问题。

## 4. 崩溃：段错误（退出码 139）

事实：
- 崩溃点 `libc strcmp+0x8`，转储只有一层调用栈，看不到调用者。
- 发生在 U 盘**扫描尚未完成**（约 507/1111）并且已经在播放、切歌时；转储里 `xiaoban_308245.mp3` 仍处于打开状态，即扫描线程在 `id3_read_tags` 内部。同一首歌的其它拷贝刚刚已成功解析过 3 次。
- 另一次抓日志（等到 `Scan complete` 之后才播放）没有崩，跑了约 3 分钟。
- 转储目录 `/tmp/aee/tombstone_hcn_application_new_<pid>_<日期>_<时间>/crash-stack.bin`，重启后丢失。

推断（未证实）：扫描线程与播放/界面并发造成的竞争，或内存紧张时某次分配失败。`id3.c`、`convert.c` 里有几处 `strcmp` 可能拿到无效指针，但没有证据指向具体某一处。

还缺：`dmesg` 里崩溃块开头的 `pc`、`lr`、`fault addr`。崩溃后立刻 `adb shell dmesg > dmesg_full.txt`。

## 5. 扫描期间的负载

- 1111 首约 22 秒。
- 扫描线程几乎全是内核态（`u0 + k9~16`），系统整体 CPU 68%~93%，而应用只占约 0.7 个核；板子至少 2 个核，约 1 个核花在应用以外（U 盘中断、FAT、内核线程，**具体是谁还没定位**，新版 `monitor.sh` 的 `oth=` 就是为此加的）。
- 扫描一结束，CPU 立刻回到 18%，与静置基线（Home 约 12%）相当。

## 6. 滑动列表时 CPU 90%

现象：关闭背景后，快速滑动列表主线程仍有 60%~70%（几乎全是用户态），扫描完成后也一样。**所以背景不是主因（之前的假设被否定）。**

x86 剖析（`tools/music_ui_harness/da_bench.sh`，真实 `ui_Music.c`，连续滑动）。绝对耗时不能对应 A7，看比例：

| 部分 | 占比 |
|---|---|
| 纯色块混合/填充 `lv_draw_sw_blend_color_to_argb8888` | 约 33%~38% |
| `lv_memset` | 约 12%~16% |
| 字体：RLE 解压 `rle_next` 17.6% + 取字模 5% + 字形二分/字距约 7% | 约 30% |

字体实验（同一份 Noto Sans CJK SC，1500 帧，各跑两次）：

| 版本 | 每帧渲染 |
|---|---|
| 压缩 + 字距（原） | 1.62 ms |
| `--no-compress` | 1.19 ms（约 -27%） |
| `--no-compress --no-kerning` | 1.25 ms（无额外收益） |

已将 `music_font_20.c` / `music_font_32.c` 换成不压缩版，`gen_music_fonts.sh` 默认 `MUSIC_FONT_COMPRESS=0`。代价：二进制约 +0.9MB（20px +176KB，32px +712KB）。位图超过 1MB，板上 `lv_conf.h` 需要 `LV_FONT_FMT_TXT_LARGE=1`（脚本注释记载板上已开）。**这个字体版本还没上板验证。**

板上用 `lv_opengles_texture`（`lvgl/main.c`），源码里 `flush_cb` 在每帧最后一次 flush 时用 `glTexImage2D` 上传**整张** 1024×600 纹理，与变化面积无关。这一部分在宿主机上测不了，可能占很大比例。

待你读数（屏幕上的 LVGL 性能叠加层）：快速滑动时的 **FPS** 和 CPU。
- FPS 接近 60 且 CPU≈90%：帧率太高，降刷新周期到 33ms 应能近似减半。
- FPS 很低（<25）：单帧就超过 40ms，降帧率没用，要降单帧成本。

## 7. 仍未解决

1. 崩溃的 `pc`/`lr`（见 §4）。
2. `displayinit.c`、`metazone_drv.c` 与 `.orig` 的差异（是否残留诊断补丁）。
3. 扫描期间"应用以外的约 1 个核"是谁。
4. 板上 `LV_DEF_REFR_PERIOD` 的实际值；滑动时的 FPS；§8 的刷新周期改动上板后的实际效果。
5. 无压缩字体上板后的实际效果。

## 8. 滑动 CPU：行内元素开销与刷新周期（2026-11-16 追加）

板子是 **2 核**（`/sys/devices/system/cpu/online` = `0-1`；仓库旧树设备树写的 4 核不适用），**没有 cpufreq 接口**（`scaling_*` 都不存在，频率无法从系统读到，"低频下满载"的假设无法验证）。

x86 基准（`BENCH_PROBE=1`，误差约 ±0.1 ms，看相对大小）：
- `S.vlist` 的 `clip_corner` 没有开销（半径为 0，基线 1.65 ms，关掉 1.68 ms），已排除。
- 逐个隐藏行内元素省下的时间合计约 1.26 ms，占一帧约 75%：副标题（歌手 · 专辑）约 0.37 ms、标题 0.28、序号 0.24、时长 0.16，小图标可忽略。**文字是大头**；要继续降单帧成本，减少每行可见文字量最有效（属于界面取舍，未改）。
- 板级 `lv_conf` 事实：`LV_USE_DRAW_SW_ASM`=NEON（板上混合比 harness 快）、`LV_FONT_FMT_TXT_LARGE`=1（不压缩字体可用）、UI 单线程、图片缓存关。主循环 `lv_timer_handler()` + `usleep(5ms)`。

改动：`ui_Music.c` 在 `S.vlist` 的 `SCROLL_BEGIN/SCROLL_END` 上切换 LVGL 刷新周期：滑动期间放宽到 `MUSIC_SCROLL_REFR_MS`（默认 33 ms，约 30 FPS），结束或离开本屏恢复 `LV_DEF_REFR_PERIOD`；`-DMUSIC_SCROLL_REFR_MS=0` 关闭。harness 里验证了行为（16 → 33 → 16，滑动途中离开本屏也恢复）和 89 项 UI 场景断言。

**前提尚未确认**：harness 自带的 `lv_conf.h` 是 9/21 版副本，其中 `LV_DEF_REFR_PERIOD` 为 16（60 FPS），**板上现在的值要看** `source/packages/cluster/lvgl/lv_conf.h`。若板上已经是 33，这个改动不起作用（无害）。预期（假设）：滑动期间主线程再除去约 12% 的基础开销后大致减半。**这个预期没有板上数据支持。** 代价：滑动时帧率降到约 30 FPS，观感比 60 FPS 略粗。

附带修正：`shots.c` 里"有封面时背景图可见"的断言在 880da8c6e 关闭背景后一直是红的（当时没跑这一项），已改为随 `MUSIC_UI_BACKDROP` 分支判断。
