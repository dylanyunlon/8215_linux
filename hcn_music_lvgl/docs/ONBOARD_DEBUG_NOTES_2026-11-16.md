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

~~板上用 `lv_opengles_texture`，每帧用 `glTexImage2D` 上传整张纹理~~（**此说法对板上很可能不成立，见 §10**）。

待你读数（屏幕上的 LVGL 性能叠加层）：快速滑动时的 **FPS** 和 CPU。
- FPS 接近 60 且 CPU≈90%：帧率太高，降刷新周期到 33ms 应能近似减半。
- FPS 很低（<25）：单帧就超过 40ms，降帧率没用，要降单帧成本。

## 7. 仍未解决

1. 崩溃的 `pc`/`lr`（见 §4）。
2. `displayinit.c`、`metazone_drv.c` 与 `.orig` 的差异（是否残留诊断补丁）。
3. 扫描期间"应用以外的约 1 个核"是谁。
4. 平台层的窗口定时器（§8）：是否值得加设置周期的接口/只在脏时上屏，需要驱动维护者确认。
5. 无压缩字体上板后的实际效果。

## 8. 滑动 CPU：行内元素开销与刷新周期（2026-11-16 追加）

板子是 **2 核**（`/sys/devices/system/cpu/online` = `0-1`；仓库旧树设备树写的 4 核不适用），**没有 cpufreq 接口**（`scaling_*` 都不存在，频率无法从系统读到，"低频下满载"的假设无法验证）。

x86 基准（`BENCH_PROBE=1`，误差约 ±0.1 ms，看相对大小）：
- `S.vlist` 的 `clip_corner` 没有开销（半径为 0，基线 1.65 ms，关掉 1.68 ms），已排除。
- 逐个隐藏行内元素省下的时间合计约 1.26 ms，占一帧约 75%：副标题（歌手 · 专辑）约 0.37 ms、标题 0.28、序号 0.24、时长 0.16，小图标可忽略。**文字是大头**；要继续降单帧成本，减少每行可见文字量最有效（属于界面取舍，未改）。
- 板级 `lv_conf` 事实：`LV_USE_DRAW_SW_ASM`=NEON（板上混合比 harness 快）、`LV_FONT_FMT_TXT_LARGE`=1（不压缩字体可用）、UI 单线程、图片缓存关。主循环 `lv_timer_handler()` + `usleep(5ms)`。

~~曾经的改动~~（已撤回）：`ui_Music.c` 曾在 `S.vlist` 的 `SCROLL_BEGIN/SCROLL_END` 上把**显示的**刷新定时器周期放宽到 33 ms。**这在板上无效，已撤回**（`ui_Music.c` 恢复为 `71b57353033cfa40b1c4d9dc52d86cbc`）。原因：板上帧节奏不是由显示自己的刷新周期决定的，见下。

**板上渲染节奏的真实来源**（读了 vanilla LVGL v9.3.0 的 `lv_glfw_window.c`，并在 126 上用 `grep`/`sed` 核对了板上魔改版同一段，结构一致，只多了一句无条件的 `GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0))`）：
- `lv_glfw_timer_init()` 用 **`LV_DEF_REFR_PERIOD`（板上 = 16 ms）** 创建一个窗口定时器 `window_update_handler`，周期在创建时固定。
- 它**每个周期无条件**执行：`glfwPollEvents` → 清屏 → `lv_refr_now(显示)`（只有脏区才真的渲染）→ 绑定默认帧缓冲 → `lv_opengles_render_texture`（画整屏纹理）→ `glfwSwapBuffers`。
- `lv_refr_now()` 直接调用 `lv_display_refr_timer()`，**不看显示自己的刷新周期**；`LV_USE_PERF_MONITOR` 打开时显示的刷新定时器也不会暂停。所以改显示的刷新周期不会改变帧节奏。
- sysmon 的 FPS = 刷新函数每秒被调用的次数（`refr_cnt`），**不是**实际重画的帧数（实际重画是另一个计数 `redraw_cnt`）。界面静止时叠加层仍显示约 50 FPS，原因就在这里，不说明有东西在持续重绘。
- 推断（**未测量**）：静止时主线程约 10%~12% 的 CPU 基线，很可能就是这个每 16 ms 一次的"清屏 + 整屏纹理 + 交换缓冲"。要降低它或降低滑动时的帧数，需要改**平台层**：例如给 `lv_glfw_window.c` 加一个设置窗口定时器周期的函数，或让它只在 `lv_refr_now` 真的渲染了东西时才画纹理/交换缓冲。GLFW 窗口可能还在合成别的纹理（CarPlay、倒车画面），这个改动要由维护该驱动的人确认。

附带修正：`shots.c` 里"有封面时背景图可见"的断言在 880da8c6e 关闭背景后一直是红的（当时没跑这一项），已改为随 `MUSIC_UI_BACKDROP` 分支判断。

## 9. 过夜测试结论（2026-11-17）

**没有崩溃，也没有卡死。** 之前看到的"卡死"是 Ctrl+C 之后的假象：

- 在 adb 下用 `run_app.sh | Tee-Object` 前台跑，程序是 adb 会话的子进程。程序启动 pid 714（开机自动起的那份是 93）。
- 程序**连续稳定运行约 14.9 小时**（11/16 17:30 到 11/17 08:24），`[music_app] No lyrics` 出现 174 次（约 174 首），无 OOM（`dmesg` 从开机起完整，无 `Out of memory`/`Killed process`/`aee`），内存可用约 99 MB，无 `/tmp/aee`。
- 约 08:22:21 用户按了 Ctrl+C，adb 管道断开。程序**不知道**，继续正常播放；约 08:24:05 它往已断开的管道写数据，收到 SIGPIPE 被杀，帧缓冲留着死前最后一帧。
- 佐证：抓到的帧缓冲显示《同桌的你》**02:45 / 04:30**；日志里这首歌的主线程换歌处理（`[music_app] Album art`）在 53715.53 秒，即 08:21:20；08:21:20 + 2分45秒 = 08:24:05，比日志最后写入（08:22:21）晚 1 分 44 秒，说明 Ctrl+C 之后界面仍在正常更新，不是 08:22:21 卡住。
- 程序里没有处理 `SIGPIPE`（在仓库可见源码里 0 处 `SIGPIPE`/`MSG_NOSIGNAL`/`signal(`），任何往已关闭管道/套接字的写都会让它无声死掉。建议在 `lvgl/main.c` 的 `main()` 开头加 `signal(SIGPIPE, SIG_IGN);`（团队文件，未改）。
- 过夜测试不要用 `Tee-Object` 挂着，用开机自启的那份加日志模式 `monitor.sh`（`MON_LOG=… MON_INTERVAL=30`）。

其它发现：
- 字体只含 GB2312 **一级**汉字 3755 个，二级汉字（如「咻」U+54BB，区位 6361）缺字，每次重绘都会写一条 `lv_draw_letter: glyph dsc. not found` 警告（滑动/动画时约每 60 ms 两条）。可补二级汉字或设后备字体。
- U 盘 FAT 文件系统有错误：`FAT-fs (sda1): invalid cluster chain`（同一时刻出错的是 `那些年_219765.mp3` 这个拷贝）。建议 `chkdsk X: /f`。程序遇到后继续正常，未造成崩溃。
- harness 的 `shots`（UI 场景）里"封面流相邻封面异步加载"那一项（`shots.c:233/234`）**不稳定**：同一份代码 6 次里有 5 次失败，与 ui_Music.c 的改动无关，是等待 900 ms 的时序问题，需要另行修（可轮询等待而不是固定等待）。之前记录的"89 项通过"只是碰巧跑过。

## 10. 更正：板上走的是"GL 绘制单元"，不是"CPU 画整屏再上传"（2026-11-17）

板级 `lv_conf.h` 实测（126 上 `grep`）：`LV_USE_DRAW_OPENGLES 1`、`LV_USE_OPENGLES_DEBUG 0`（`GL_CALL` 就是普通调用，无 `glGetError` 开销）、`LV_USE_LOG 1`/`LV_LOG_LEVEL_WARN`、`LV_USE_SYSMON 1`、`LV_USE_PERF_MONITOR 1`、`LV_USE_MEM_MONITOR 1`（FPS/CPU 与内存叠加层在板上屏幕上一直显示，产品版应关）。包名 `lvgl-9.3.0`（`out/build/ac83xx/lvgl-9.3.0`）。BUILD_ENV_NOTES 里早就记了 `LV_USE_DRAW_OPENGLES = 1`，但我之前没意识到它会让整个渲染模型不同。

按 vanilla LVGL v9.3.0 的源码（**板上 `lv_draw_opengles.c`、`lv_opengles_texture.c` 是否被改过还没看到，需确认**）：
- `lv_opengles_texture.c` 里 `flush_cb` 的整屏 `glTexImage2D` 上传被 `#if !LV_USE_DRAW_OPENGLES` 包住，板上这一段**不会编进去**。
- `lv_draw_opengles.c` 的 `evaluate()` 把几乎所有绘制任务指派给 GL 绘制单元。只有"无圆角、无渐变的纯色填充"直接用 GL 画；**其余（文字、边框、圆角填充、阴影、图片…）走 `draw_to_texture()`**：在 CPU 上用软件绘制画进一块临时 ARGB8888 缓冲，`glGenTextures` + `glTexImage2D` 上传 + `glGenerateMipmap`，再用一个贴图四边形画到目标。结果按绘制描述符做 LRU 缓存（128 项）。
- 代码里的注释写明：**`text_static` 为假的文字不缓存，画完立刻丢弃**。我们的列表行文字都是 `lv_label_set_text`（复制，非静态）；`LV_LABEL_LONG_DOT` 溢出时 LVGL 还会用内部缓冲，同样不是静态。**推断（未在板上测量）**：滑动时每帧每个可见文字标签都要经历"CPU 光栅化 → 建纹理 → 上传 → 生成 mipmap → 画 → 丢弃"，这很可能是滑动时主线程用户态 60%~70% 的主要来源，比 §6 里 x86 纯软件基准显示的还重。
- 因此 §6 里 x86 基准（纯软件绘制）只能说明各元素的**相对**占比，不能代表板上的绝对开销。字体不压缩只省"光栅化"那一段，省多少取决于它在每个标签总开销里占多少。
- 窗口定时器（§8）每个周期无条件 `lv_refr_now` + 画整屏纹理 + `glfwSwapBuffers`，这部分在 GL 绘制单元模型下仍成立。
