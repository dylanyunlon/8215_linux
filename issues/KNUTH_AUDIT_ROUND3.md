# Knuth 式第三轮严格审查 — AWTK musicplayer vs Android AutoMediaPlayer

> 审查人: 以 Knuth《计算机程序设计艺术》作者视角
> 审查日期: 2026-08-20
> 审查范围: `source/packages/application/musicplayer/` 全部代码
> 参考标准: `android_ref/autoapps/AutoMediaPlayer/` 全部 Java/C++ 代码
> 前置: 第一轮 (GAP-1~12), 第二轮 (Issue #14~#30) 已修复
> 编译命令: `./allmake.sh -a nand-512-ddr-512 -d userdebug -m false`
> 架构: 单进程 (参考 dylanyunlon/0314_ad008)

---

## 一、用户角度缺陷

### Issue #31 — [P0] music_app.c: music_app_play_folder() 内存泄漏 — folder_list 的 items 被 set_playlist 复制后未正确释放

**文件**: `awtk_app/src/music_app.c` `music_app_play_folder()` 函数

**问题**: 代码注释说 "folder_list data was copied by set_playlist; we can destroy the container"，
但 `music_list_create()` 分配了 `items` 数组，`music_list_destroy()` 会释放它。
然而 `music_player_set_playlist()` 的语义需要确认：如果它是浅拷贝（只拷贝指针），
那么 destroy 之后 player 持有的指针就是悬空的。如果它是深拷贝，则没问题。

**关键**: `music_player_set_playlist()` 在 `music_player.h` 中注释说 "copies data"，但我们
没有看到 `.cpp` 实现。如果实现中有 bug（例如只拷贝了 list 指针没拷贝 items 数组），
那么 `music_app_play_folder()` 和 `music_app_play_group()` 中调用
`music_list_destroy(folder_list)` 后，player 内部的 playlist 就指向已释放的内存。

**用户症状**: 进入文件夹播放后，切歌时崩溃或播放错误文件。

**修复**: 审查 `music_player.cpp` 中 `music_player_set_playlist()` 的实现，确保是深拷贝。
如果不是，则不要在 set_playlist 之后 destroy folder_list，改为让 player 在
下次 set_playlist 时释放旧列表。

---

### Issue #32 — [P1] music_app.c: 缺少 ACC OFF/ON 生命周期处理 — Android LocalService.onAccOffBroadcastEvent()

**文件**: `awtk_app/src/music_app.c`, `music_app.h`

**问题**: Android `LocalService` 中有完整的 ACC OFF/ON 处理:
- ACC OFF → `writeCurrentMediaTime(true, ...)` 保存当前播放位置 → 暂停播放
- ACC ON → 恢复播放 (如果之前在播放状态)

当前 AWTK 实现完全没有 ACC 状态处理。虽然当前是单独进程，但车机在 ACC OFF 时如果
进程还在运行，应当保存状态并停止播放（省电、避免继续驱动功放）。

**用户症状**: 用户关闭车辆点火后，音乐可能继续播放，或者重新上电后不能恢复到上次位置。

**修复**: 增加 `music_app_on_acc_off()` / `music_app_on_acc_on()` API，在 main.c 中
通过 signal 或外部 IPC 触发。

---

### Issue #33 — [P1] music_ui.c: 缺少存储设备切换 UI — Android 支持多设备 Tab 切换

**文件**: `awtk_app/src/music_ui.c`

**问题**: Android `MusicUI` / `MusicInfoFragment` 中有 USB/SD/Flash 设备切换按钮
（对应 `RadioButton rb_usb_bg`, `rb_sd_bg`, `rb_flash_bg`），用户可以在多个
存储设备之间切换。当前 AWTK 实现在 `music_app.c` 中有 `music_app_switch_device()`
API，但 UI 中没有任何触发设备切换的按钮或交互元素。

**用户症状**: 如果同时插入 USB 和 SD，用户无法手动切换到另一个设备。

**修复**: 在 `music_ui.c` 中增加设备切换按钮行。

---

### Issue #34 — [P1] music_ui.c: 缺少播放列表类型切换 Tab — Android 支持 全部/文件夹/收藏/专辑/艺术家 Tab

**文件**: `awtk_app/src/music_ui.c`

**问题**: Android `MusicListFragment` / `MusicViewPaperFragment` 中有播放列表类型
Tab 栏（全部歌曲 / 文件夹 / 收藏 / 专辑 / 艺术家），用户可以点击 Tab 切换不同的
列表视图。当前 AWTK 实现在 `music_app.c` 中已有文件夹、专辑、艺术家、收藏的 API，
但 UI 中只有一个扁平的播放列表，没有 Tab 切换。

**用户症状**: 用户只能看到全部歌曲列表，无法浏览文件夹/专辑/艺术家分类。

**修复**: 在列表区域上方增加 Tab 按钮行，点击切换不同的列表视图。

---

### Issue #35 — [P2] music_app.c: music_app_play_folder() 只过滤直接子文件, 不支持递归子文件夹

**文件**: `awtk_app/src/music_app.c` `music_app_play_folder()` 中

**问题**:
```c
if (strchr(rest, '/') == NULL) {
    /* Direct child of this folder */
```
这只匹配文件夹的直接子文件。但 Android `FolderListLayout` 支持递归浏览子文件夹，
用户可以进入子文件夹再播放。当前实现中如果用户的音乐在 `/mnt/usb0/Music/Rock/` 下，
而用户选择 `/mnt/usb0/Music/` 文件夹，则 Rock 子文件夹下的文件不会出现在播放列表中。

**Android 对比**: Android `MediaFilePathScan` 递归扫描，`FolderListLayout` 分层显示。

**修复**: 移除 `strchr(rest, '/') == NULL` 过滤，或提供选项让用户选择是否递归。

---

### Issue #36 — [P1] music_ui.c: 缺少 "回到全部歌曲" 播放列表恢复功能

**文件**: `awtk_app/src/music_app.c`, `music_ui.c`

**问题**: 用户进入文件夹或专辑播放后(`music_app_play_folder` / `music_app_play_group`)，
播放列表被替换为子集。但没有 API 或 UI 可以恢复回 "全部歌曲" 播放列表。

**Android 对比**: Android `MusicPlaylistEx` 中有 `mFirstPlaylistEx` 备份机制，
点击 "全部歌曲" Tab 时恢复。`IPlaylistType.DEVICE_LIST` 始终保留完整列表。

**修复**: 增加 `music_app_restore_full_playlist()` API，将当前设备的完整 music_list
重新设置为播放列表。在 UI 的 Tab 切换中，点击 "全部" 时调用。

---

### Issue #37 — [P2] music_app.c: load_album_art_for_current() 在 on_player_track 回调中被调用，但此时在 player 线程

**文件**: `awtk_app/src/music_app.c` `on_player_track()` 回调

**问题**:
```c
static void on_player_track(int index, const MusicInfo* info, void* user_data) {
    ...
    load_album_art_for_current();  // ← 在 player 线程中调用
    load_lyrics_for_current();     // ← 在 player 线程中调用
    post_ui_event(APP_EVENT_TRACK_CHANGED, index);
}
```
`load_album_art_for_current()` 和 `load_lyrics_for_current()` 都执行文件 I/O
（`fopen`, `fread`），这在 player 线程中会阻塞播放回调，导致播放器状态更新延迟。
虽然功能上没有线程安全问题（它们操作的是独立缓存），但会影响切歌响应速度。

**Android 对比**: Android `BitmapCache.loadNativeImage()` 在独立线程中异步加载。

**修复**: 将 `load_album_art_for_current()` 和 `load_lyrics_for_current()` 移到
`post_ui_event` 之后在主线程执行，或者在扫描线程中预加载。

---

## 二、系统角度缺陷

### Issue #38 — [P0] music_app.c: scan_thread_func 中 music_player_set_playlist 和 music_player_play 从非主线程调用

**文件**: `awtk_app/src/music_app.c` `scan_thread_func()` 第 230-260 行

**问题**:
```c
if (is_current && list->count > 0 && s_app.player) {
    music_player_set_playlist(s_app.player, list);  // ← scan thread
    ...
    music_player_play(s_app.player, resume_idx);    // ← scan thread
    music_player_seek(s_app.player, ...);            // ← scan thread
}
```
`music_player_set_playlist()` 和 `music_player_play()` 操作 player 内部数据结构，
如果 player 内部没有做线程保护（从 `.h` 接口看没有明确说明），这些操作在 scan 线程
中执行时，如果 player 正在进行播放回调或位置更新，就会产生数据竞争。

**修复**: 通过 `idle_queue()` 将 set_playlist/play/seek 投递到 AWTK 主线程执行。

---

### Issue #39 — [P1] main.c: 没有 SIGCHLD 处理 — 如果 fork 了子进程(如 mount helper)可能产生僵尸进程

**文件**: `awtk_app/src/main.c`

**问题**: `signal_handler()` 只处理 SIGINT 和 SIGTERM。如果系统中任何组件（如 mount
自动挂载）fork 了子进程并且该进程退出，而没有 SIGCHLD 处理，会产生僵尸进程。
在单进程架构中这可能不是问题，但如果未来集成更多功能（如参考 0314_ad008 的单进程设计），
就需要注意。

**修复**: 增加 `signal(SIGCHLD, SIG_IGN)` 或在 signal_handler 中 waitpid。

---

### Issue #40 — [P1] music_app.c: build_folder_cache 中 folder_paths 数组大小为 MUSIC_MAX_FILES(2000) 但每个元素是 strdup 分配

**文件**: `awtk_app/src/music_app.c` s_app 结构体定义

**问题**:
```c
char* folder_paths[MUSIC_MAX_FILES]; /* unique folder strings */
```
`MUSIC_MAX_FILES = 2000`，每个元素是 `strdup()` 分配的字符串指针。但唯一文件夹数量
通常远小于文件数量（可能只有 20-50 个）。这浪费了 `2000 * sizeof(char*) = 16KB`
栈空间（如果在栈上）或静态数据段空间。更严重的是，这个数组大小限制了文件夹数量
不能超过 2000 个（虽然实际不太可能）。

**修复**: 使用独立的 `#define MUSIC_MAX_FOLDERS 512` 或动态分配。

---

## 三、修复清单

| Issue | 级别 | 简述 | 涉及文件 | 类型 |
|-------|------|------|----------|------|
| #31 | **P0** | play_folder/play_group 后 destroy 可能导致悬空指针 | music_app.c | Bug |
| #38 | **P0** | scan_thread 中调用 player API 数据竞争 | music_app.c | Bug |
| #32 | **P1** | 缺少 ACC OFF/ON 生命周期处理 | music_app.c/h | 功能 |
| #33 | **P1** | 缺少存储设备切换 UI | music_ui.c | 功能 |
| #34 | **P1** | 缺少列表类型 Tab 切换 UI | music_ui.c | 功能 |
| #36 | **P1** | 缺少恢复全部歌曲播放列表 API | music_app.c/h | 功能 |
| #37 | **P2** | 专辑封面/歌词在 player 线程加载阻塞回调 | music_app.c | 性能 |
| #35 | **P2** | 文件夹播放不递归子文件夹 | music_app.c | 功能 |
| #39 | **P1** | 缺少 SIGCHLD 处理 | main.c | 系统 |
| #40 | **P1** | folder_paths 数组大小不合理 | music_app.c | 优化 |
