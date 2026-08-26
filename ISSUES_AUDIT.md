# MusicPlayer AWTK 审查 Issue 清单

> 审查方法：用 `android_ref/autoapps/AutoMediaPlayer` 的 Java/C++ 代码  
> 对比 `source/packages/application/musicplayer/awtk_app` 的现有 C 代码  
> 编译命令：`./allmake.sh -a nand-512-ddr-512 -d userdebug -m false`  
> 参考：`middleware/module/bt/bt_cxx.cpp` C/C++混编范式（extern "C" wrapper + STL内部实现）

---

## 架构级 Issue

### Issue #A1: C→C++ 重构 — 采用 bt_cxx 混编范式

**现状：** 现有 `music_app.c` 用纯 C 手写数据结构（`darray.h` 宏模板、手动 malloc/free、手写哈希），约1745行。  
**Android 对标：** `LocalService.java` 4031行，大量使用 `ArrayList`、`HashMap`、`MusicInfo` 对象。  
**bt_cxx 范式已证明可行：** `middleware/module/bt/` 已经建立了完整的 C/C++ 混编模式：
- `bt_device_manager.h`：C++类，使用 `std::vector`、`std::unordered_map`、`std::string`、`std::mutex`、`osal_cxx.h`
- `bt_cxx.h/cpp`：`extern "C"` wrapper，try/catch 全兜底，只暴露 C ABI

**方案：** 新建 `music_app_cxx.cpp` + `music_app_cxx.h`（C ABI wrapper）+ `music_app_impl.h/cpp`（C++业务类），  
保持 `music_app.h` 现有 C API 签名不变（`music_ui.c` / `main.c` 零改动），内部用 STL 重写。  
**收益：**
- `darray.h` 宏 → `std::vector<T>`：类型安全，无手写扩容
- 手动 `strdup`/`free` → `std::string`：消灭内存泄漏类 bug
- 手写 `qsort` + 线性去重 → `std::unordered_set` / `std::sort` + `std::unique`
- `pthread_mutex` 手动 lock/unlock → `osal::MutexGuard` RAII
- 分散的 `find_or_create_group` → `std::unordered_map<std::string, std::vector<MusicInfo*>>`

**优先级：** P0（架构决策，影响后续所有 Issue）

---

### Issue #A2: 音频焦点管理 — Android AudioFocusChangeListener 对标缺失

**Android：** `LocalService.java` 的 `AudioFocusChangeListener` 处理四种焦点状态：
- `AUDIOFOCUS_GAIN`：恢复播放 + 音量恢复1.0
- `AUDIOFOCUS_LOSS`：永久丢失 → 暂停 + 释放 MediaButton + 取消通知
- `AUDIOFOCUS_LOSS_TRANSIENT`：短暂丢失 → 暂停（记忆状态 `mAllowResumePlay`）
- `AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK`：混音 → 音量降到0.5

**AWTK 现状：** 完全缺失。无音频焦点概念。  
**影响：** 在 8215 Linux 上，如果其他进程（蓝牙通话、导航语音、TTS）播放音频，musicplayer 不知道让步，会出现混音/冲突。  
**方案：** 通过 middleware 的 IPC 机制（或 PulseAudio/ALSA mixer 事件监听）实现简化版焦点仲裁：
- 收到"其他音源播放"信号 → 暂停 + 记忆 `mAllowResumePlay`
- 收到"其他音源结束"信号 → 恢复播放

**优先级：** P1（多进程架构下必须，单进程可降级）

---

### Issue #A3: 快进/快退 — Android onSeekRewind/onFastForward 缺失

**Android：** `LocalService.java` lines 3881-3910：
```java
private void onSeekRewind() {
    int nTime = mAppData.mPlayTimeInfo.mCurrentTime - SEEK_STEP; // SEEK_STEP=5000ms
    if (nTime < 0) nTime = 0;
    trySeekToTime(nTime);
}
private void onFastForward() {
    int nTime = mAppData.mPlayTimeInfo.mCurrentTime + SEEK_STEP;
    if (nTime > mAppData.mPlayTimeInfo.mTotalTime) nTime = mAppData.mPlayTimeInfo.mTotalTime;
    trySeekToTime(nTime);
}
```
**AWTK 现状：** `music_app.h` 无 `music_app_seek_forward()` / `music_app_seek_backward()` API。  
`music_ui.c` 无对应按钮/手势。  
**影响：** 用户无法快进快退，长音频（有声书、播客）不可用。  
**方案：** 添加 API + UI 长按 prev/next 按钮触发快进快退（Android KeyEvent.KEYCODE_MEDIA_REWIND 模式）。

**优先级：** P1

---

### Issue #A4: 播放锁 mIsMediaPlayerLocked — 防并发操作缺失

**Android：** `LocalService.java` 在 `onLocalMusicPlayControl` 开头检查：
```java
if (mAppData.mIsMediaPlayerLocked) return false;
```
这在 `requestPlayDataSource` 设置 DataSource 到 `onPrepared` 之间为 true，防止用户疯狂点击导致 MediaPlayer 并发调用崩溃。

**AWTK 现状：** 无此机制。`music_app_play()`/`music_app_next()` 可以在 player 还在 preparing 时被调用。  
**影响：** 快速连续点击"下一曲"可能导致 `libatcmediaplayer` 崩溃（MediaPlayer 在 preparing 状态收到新的 setDataSource）。  
**方案：** 在 `music_app_state_t` 增加 `bool player_locked`，在 play 命令发出到 state callback 返回 PLAYING 之间置 true。

**优先级：** P0（崩溃级别）

---

### Issue #A5: 高优先级事件屏蔽 — existsHighPriorityEvent 缺失

**Android：** `LocalService` 的 `existsHighPriorityEvent()` 检查：蓝牙通话中/ACC OFF/倒车中/...  
这些场景下禁止播放控制操作。

**AWTK 现状：** 仅有 ACC OFF 处理，无蓝牙通话/倒车检测。  
**影响：** 倒车时视频不暂停（安全隐患），蓝牙通话时音乐不自动暂停。  
**方案：** 通过 middleware 的 `vehicle_param` 获取车辆状态（参照 `bt_device_manager.cpp` 的 `vehicle_speed_kmh()` 调用方式），在播放控制入口增加门控。

**优先级：** P1

---

## 功能级 Issue

### Issue #F1: ID3 同步更新 — 扫描后 playlist 的 ID3 信息更新

**Android：** `AsyncHandler.MSG_SYNC_ID3INFO_2_MUSIC_PLAYLIST` — 文件扫描先快速建列表（文件名），ID3 解析完成后异步同步 title/artist/album 到已有 playlist 条目。

**AWTK 现状：** `music_scan_directory()` 同步完成文件扫描+ID3解析（阻塞式）。  
**影响：** 大量文件时首次扫描慢（10000文件可能耗时30+秒），用户看到空列表时间过长。  
**方案（C++重构后）：**
1. Phase1：快速扫描文件名（只 `stat`+扩展名过滤），立即显示列表
2. Phase2：后台线程逐文件解析 ID3，通过 `osal::Worker` 批量更新
3. UI 通过 `APP_EVENT_ID3_UPDATED` 增量刷新

**优先级：** P2

---

### Issue #F2: 收藏列表播放模式 — Android IPlaylistType.FAVORITE_LIST 播放

**Android：** 点击收藏列表中的歌曲时，`tryUpdateMusicPlaylist(IPlaylistType.FAVORITE_LIST, ...)` 将收藏列表设为当前 playlist，next/prev 在收藏列表内循环。

**AWTK 现状：** `on_fav_item_click` 调用 `music_app_restore_full_playlist()` 后按全局索引播放，next/prev 会跳出收藏列表。  
**影响：** 用户体验不符合预期——点了收藏列表的歌，下一曲却不是收藏列表里的。  
**方案：** 在 `on_fav_item_click` 中构建收藏子列表并 `music_player_set_playlist()`，设 `playlist_type = PLAYLIST_TYPE_FAVORITE`。

**优先级：** P1

---

### Issue #F3: 外部触发播放 — mSingleMusicFilePath 机制缺失

**Android：** 支持"从文件管理器打开音频文件"的 Intent：`mSingleMusicFilePath` → 定位设备 → 定位索引 → 播放。  
支持"语音识别结果触发播放"：`MusicRegInfo` → 按 title/artist/album 模糊匹配。

**AWTK 现状：** 不支持外部触发播放。  
**影响：** 无法从其他应用启动播放特定歌曲。  
**方案（单进程架构）：** 通过 Unix domain socket 或共享内存接收外部播放请求。  
**方案（多进程架构）：** 通过 middleware IPC / D-Bus 接收。

**优先级：** P2（当前单独进程运行，无外部交互需求；转单进程架构后 P1）

---

### Issue #F4: classifyMediaInfoList 分类后的 HashMap 缺失

**Android：** `StorageDeviceEx` 维护：
- `mAlbumListMap: HashMap<String, List<MusicInfo>>` — 按专辑分组
- `mArtistListMap: HashMap<String, List<MusicInfo>>` — 按艺术家分组
- `mPathListMap` — 按路径分组

**AWTK 现状：** `build_classification_cache()` 用 `GroupArray`（`darray` 宏展开的线性数组）+ 线性查找 `find_or_create_group()`，O(n*m) 复杂度。

**C++重构方案：**
```cpp
std::unordered_map<std::string, std::vector<const MusicInfo*>> album_map_;
std::unordered_map<std::string, std::vector<const MusicInfo*>> artist_map_;
```
查找从 O(n) 降到 O(1)。

**优先级：** P1（C++重构时自然解决）

---

### Issue #F5: Notification 通知栏 — Android 通知栏播放控制

**Android：** `initMusicPlayerNotification()` — RemoteViews 显示当前歌曲 + prev/play/next 按钮。  
**AWTK 现状：** 不适用（嵌入式无通知栏）。  
**结论：** 不需要实现。标记为 Won't Fix。

---

### Issue #F6: 语音控制接口 — VoiceControlReceiver

**Android：** `VoiceControlReceiver` 接收语音命令广播：
- `ACTION_MUSIC_PLAY` / `ACTION_MUSIC_PAUSE`
- `ACTION_MUSIC_SINGLE_MODEL` / `ACTION_MUSIC_RANDOM_MODEL` / `ACTION_MUSIC_ALL_LOOP_MODEL`

**AWTK 现状：** 无语音控制接口。  
**方案：** 通过 middleware 的 `callback_config.h` 注册回调，接收语音模块事件。  
**优先级：** P2

---

### Issue #F7: 播放恢复机制 — mAllowResumePlay 状态机缺失

**Android：** `doShouldPlayEvent()` / `doShouldPauseEvent()` 形成完整的暂停-恢复状态机：
- 暂停时记忆 `mAllowResumePlay = (当前是播放状态)`
- 恢复条件满足时检查 `mAllowResumePlay` 决定是否自动播放

**AWTK 现状：** ACC ON/OFF 有简化处理，但无通用的暂停-恢复机制。  
蓝牙通话结束/焦点恢复时不会自动恢复播放。  
**方案：** 在 `music_app_state_t` 增加 `bool allow_resume_play`，在所有暂停路径（焦点丢失/倒车/蓝牙通话）中设置。

**优先级：** P1

---

### Issue #F8: 视频播放支持 — Android MEDIA_TYPE_VIDEO 完整链路

**Android：** `LocalService` 同时管理音频和视频：
- `mMediaType` 切换
- 视频 Surface 管理
- 行车中禁止视频播放（安全法规）
- 视频后台播放策略

**AWTK 现状：** 仅音频播放器，不支持视频。  
**结论：** 当前是"本地音乐 app"，视频支持为 Phase2。标记为 Deferred。

**优先级：** P3 (Deferred)

---

### Issue #F9: 媒体数据库持久化 — Android Room/SQLite 对标

**Android：** 使用 ContentProvider + Room 数据库持久化媒体列表和 ID3 信息。  
**AWTK 现状：** `music_db_save()` 使用 TAB 分隔的文本文件。  
**影响：** 文本文件无索引、无事务、无完整性保证。10000+ 条目时 load 慢。  
**方案（可选）：** 嵌入 SQLite（buildroot 已有 libsqlite3），用 prepared statement 存取。  
**优先级：** P3

---

### Issue #F10: 行车安全门控 — 倒车暂停/刹车状态

**Android：** `onActionReverseStatus()` + `onActionParkingStatus()`：
- 倒车中：视频暂停（音乐不暂停）
- 行车中（非刹车）：视频播放界面显示安全警告

**AWTK 现状：** 完全缺失。  
**方案：** 通过 middleware 的 `vehicle_param` 模块获取倒车/刹车状态。  
**优先级：** P1（安全法规要求）

---

## Bug / 健壮性 Issue

### Issue #B1: folder_list 内存泄漏 — music_app_play_folder 的 sub-list

**现状：** `music_app_play_folder()` 创建 `folder_list`，调用 `music_player_set_playlist()` 后立即 `music_list_destroy()`。  
**问题：** 注释说"folder_list data was copied by set_playlist; we can destroy the container"——但如果 `music_player_set_playlist` 不做深拷贝（只存指针），则 destroy 后 player 持有悬垂指针。需要验证 `music_player.cpp` 的 `set_playlist` 实现。

**Knuth 审查：** 如果 `set_playlist` 是浅拷贝 → Use-After-Free 崩溃。如果是深拷贝 → OK 但浪费内存（两份 MusicInfo 数组）。C++ 重构用 `std::vector` + move 语义彻底解决。

**优先级：** P0（潜在崩溃）

---

### Issue #B2: build_folder_cache 的 all_dirs malloc 失败路径

**现状：** line 720:
```c
char** all_dirs = (char**)malloc(list->count * sizeof(char*));
```
如果 `list->count` 很大（65536）且内存紧张（512MB DDR），malloc 可能返回 NULL。  
后续 `all_dirs[dir_count] = strdup(dir)` 会 segfault。

**方案：** 加 NULL 检查。C++ 重构后用 `std::vector<std::string>` 自动管理。

**优先级：** P1

---

### Issue #B3: extract_apic_from_file 的 ID3v2.2 兼容性

**现状：** APIC 解析只处理 4 字节 frame ID（ID3v2.3/v2.4）。  
**问题：** ID3v2.2 使用 3 字节 frame ID（"PIC" 而非 "APIC"），frame size 也是 3 字节。老旧 MP3 文件可能使用 v2.2。  
**影响：** 部分老文件无法显示封面。  
**方案：** 增加 `version_major == 2` 的特殊处理分支。

**优先级：** P2

---

### Issue #B4: LRC 解析的 UTF-8 BOM 处理

**现状：** `parse_lrc_file()` 直接 `fgets` 读取。  
**问题：** 很多 Windows 编辑器生成的 LRC 文件带 UTF-8 BOM（`\xEF\xBB\xBF`），第一行 `[ti:...]` 变成 `\xEF\xBB\xBF[ti:...]`，time tag 解析失败。  
**方案：** 在读取第一行时检测并跳过 BOM。

**优先级：** P2

---

### Issue #B5: wchar_t → UTF-8 手写转换不完整

**现状：** `execute_search()` 中手写了 wchar → UTF-8 转换（line 339-345），只处理到 3 字节（BMP 范围）。  
**问题：** 4 字节 Unicode（emoji、部分 CJK 扩展）被截断。  
**方案：** 使用 AWTK 的 `tk_utf8_from_utf16()` 或标准 `wcstombs()`。

**优先级：** P2

---

## 编译/部署 Issue

### Issue #D1: Makefile 添加 C++ 编译支持

**现状：** `awtk_app/Makefile` 的 `CPP_SRCS` 为空，但 `$(CXX)` 已配置交叉编译 g++。  
**方案：** 添加 `.cpp` 源文件到 `CPP_SRCS`，添加 `-std=c++14` 到 `CXXFLAGS`，添加 `-I$(TOPDIR)/middleware/hal/osal` 到 `INC_FLAGS`（引入 `osal_cxx.h`）。

**优先级：** P0（C++重构前置条件）

---

### Issue #D2: SCP 部署脚本 — 推送到 192.168.0.126

**编译环境：** 192.168.0.126（无公网）通过 192.168.70.17 中转  
**登录信息：** `tanyunlong` / `hcn@2026`  
**方案：** 编写 `deploy.sh`：
```bash
scp music_player tanyunlong@192.168.70.17:/tmp/
ssh tanyunlong@192.168.70.17 "scp /tmp/music_player tanyunlong@192.168.0.126:~/out/target/..."
```

**优先级：** P2

---

## 总结优先级排序

| 优先级 | Issue | 描述 |
|--------|-------|------|
| P0 | #A1 | C++重构（bt_cxx范式） |
| P0 | #A4 | 播放锁 mIsMediaPlayerLocked |
| P0 | #B1 | folder_list 内存安全 |
| P0 | #D1 | Makefile C++支持 |
| P1 | #A3 | 快进/快退 |
| P1 | #A5 | 高优先级事件屏蔽 |
| P1 | #F2 | 收藏列表播放模式 |
| P1 | #F4 | 分类HashMap重构 |
| P1 | #F7 | 播放恢复状态机 |
| P1 | #F10 | 行车安全门控 |
| P1 | #B2 | malloc失败路径 |
| P1 | #A2 | 音频焦点管理 |
| P2 | #F1 | ID3异步解析 |
| P2 | #F3 | 外部触发播放 |
| P2 | #F6 | 语音控制接口 |
| P2 | #B3 | ID3v2.2兼容 |
| P2 | #B4 | LRC UTF-8 BOM |
| P2 | #B5 | wchar转换不完整 |
| P2 | #D2 | SCP部署脚本 |
| P3 | #F8 | 视频播放(Deferred) |
| P3 | #F9 | SQLite持久化 |
| N/A | #F5 | 通知栏(Won't Fix) |
