# Knuth 式第四轮审查 — AWTK musicplayer vs Android AutoMediaPlayer (UI功能对齐)

> 审查人: 以 Knuth《计算机程序设计艺术》作者视角
> 审查日期: 2026-08-20
> 审查范围: `music_ui.c` vs Android `MusicUI.java` / `MusicInfoFragment` / `MusicListFragment`
> 编译命令: `./allmake.sh -a nand-512-ddr-512 -d userdebug -m false`
> 架构: 单进程纯C (参考 0314_ad008), 不需要跟 Android 映射

---

## 一、架构决策确认

**纯C还是C++？—— 必须纯C。**

| 维度 | Android | AWTK (本项目) |
|------|---------|---------------|
| 语言 | Java + C++ (JNI) | 纯 C (music_app.c/music_ui.c) |
| 进程模型 | 多进程 (UI + LocalService + RemoteService) | 单进程 (ref: 0314_ad008) |
| IPC | Binder | 无需 (同进程直接调用) |
| 对象模型 | class/interface/ViewModel/LiveData | struct + function pointer + callback |
| 线程通信 | Handler.post() | idle_queue() → AWTK main loop |
| 内存 | Android GC | malloc/free, 静态数组 |
| 目标硬件 | 通用 ARM + 1GB+ DDR | nand-512 + ddr-512 |

C++ 的 STL 容器、异常机制、虚表在 512MB DDR 上开销不可忽视。
现有 `music_player.cpp` 是 libatcmediaplayer 的 wrapper，通过 `extern "C"` 暴露纯 C 接口，这是正确的封装方式。

---

## 二、用户角度缺陷 (基于 Android AutoMediaPlayer 对比)

### Issue #41 — [P1] music_ui.c: 缺少存储设备切换 Tab/按钮

**Android 参照**: `MusicUI.java` 的 Fragment 切换 + `RadioButton rb_usb_bg`, `rb_sd_bg`
**AWTK 现状**: `music_app_switch_device()` API 已有，但 UI 中无触发入口

**用户症状**: 同时插入 USB 和 SD 卡时，无法切换设备

**需修改**: `music_ui.c` — 在状态栏区域增加设备切换按钮

---

### Issue #42 — [P1] music_ui.c: 缺少播放列表类型 Tab (全部/文件夹/收藏/专辑/艺术家)

**Android 参照**: `MusicViewPaperFragment.java` (ViewPager + TabLayout),
`MusicListFragment`, `IMusicPage.PAGE_INDEX_*`

**AWTK 现状**: `music_app.c` 已有文件夹、专辑、艺术家、收藏的 API，但 UI 只有扁平列表

**用户症状**: 用户只能看到全部歌曲列表

**需修改**: `music_ui.c` — 在列表区域上方增加 Tab 按钮行

---

### Issue #43 — [P2] music_ui.c: 缺少专辑封面显示区域

**Android 参照**: `MusicInfoLayout.updateMusicImage()`, ImageView 显示专辑封面

**AWTK 现状**: `music_app_get_album_art()` API 已有，但 UI 无图片显示区域

**需修改**: `music_ui.c` — 在播放信息区域增加 image widget

---

### Issue #44 — [P2] music_ui.c: 缺少歌词显示区域

**Android 参照**: `LyricsView`, `MusicInfoLayout.updateLrcRowList()`

**AWTK 现状**: `music_app_get_lyrics()` / `music_app_get_lyrics_line()` API 已有，但 UI 无歌词区域

**需修改**: `music_ui.c` — 在播放区域增加歌词滚动标签

---

### Issue #45 — [P1] music_ui.c: 缺少倒车事件处理

**Android 参照**: `MusicUI.AutoBroadcastReceiver` 处理 `CarStatus.ACTION_REVSTATUS`
停止频谱更新、暂停视频播放

**AWTK 现状**: 完全没有倒车状态处理

**需修改**: `main.c` 或 `music_app.c` — 接收倒车信号（通常通过 GPIO 或 CAN），暂停播放

---

### Issue #46 — [P2] music_ui.c: 缺少搜索 UI 入口

**Android 参照**: `MusicSearchFragment`, 搜索按钮 + 输入框 + 结果列表

**AWTK 现状**: `music_app_search()` API 已有，但 UI 无搜索入口

---

## 三、系统角度缺陷

### Issue #47 — [P1] music_app.c: music_app_play_folder() 中 folder_list 析构后 player 可能持有悬空指针

**分析**: 这是 Issue #31 的延续。`music_player_set_playlist()` 的头文件注释说 "copies data"，
但如果实现只是浅拷贝了 MusicList 的指针而非 items 数组，则 destroy 后 player 内部悬空。
同样的问题存在于 `music_app_play_group()`。

**修复方案**: 在 `music_player.h` 中明确 set_playlist 的语义。如果是浅拷贝，则不能在
set_playlist 后立即 destroy。可以改为 player 内部维护独立副本(深拷贝)。

---

### Issue #48 — [P1] music_app.c: build_folder_cache() 的 O(n²) 去重在大量文件时性能差

**问题**: 每个文件的 folder 都和已有 folder 列表做线性搜索，2000 首歌 × 50 个文件夹 = 10 万次比较

**修复**: 对 folder_paths 排序后二分查找，或使用简单哈希表

---

## 四、修复清单

| Issue | 级别 | 简述 | 涉及文件 | 类型 |
|-------|------|------|----------|------|
| #41 | **P1** | 设备切换 UI | music_ui.c | 功能 |
| #42 | **P1** | 播放列表 Tab UI | music_ui.c | 功能 |
| #43 | **P2** | 专辑封面显示 | music_ui.c | 功能 |
| #44 | **P2** | 歌词显示 | music_ui.c | 功能 |
| #45 | **P1** | 倒车处理 | music_app.c/main.c | 功能 |
| #46 | **P2** | 搜索 UI | music_ui.c | 功能 |
| #47 | **P1** | set_playlist 深浅拷贝 | music_player.cpp | Bug风险 |
| #48 | **P1** | folder去重性能 | music_app.c | 性能 |
