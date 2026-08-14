# Knuth 式严格审查报告
# 对照 Android android_ref/autoapps/AutoMediaPlayer 参考实现

## 审查方法

逐模块对比 Android LocalService.java (4031行) + MediaService.java + MediaFilePathScan.java
+ FileStorageState.java + MusicProvider.java 与我们的 C/AWTK 实现。

按严重级别: **P0** = 必须立即修复(会导致崩溃/数据丢失)
             **P1** = 必须修复(功能缺失,用户可感知)
             **P2** = 应当修复(健壮性/边界情况)
             **P3** = 建议后续版本(增强功能)

---

## 一、USB/存储设备管理 (usb_monitor.c vs FileStorageState + BroadcastReceiver)

### 已正确对齐的部分
- ✅ uevent netlink 监听 ↔ Android ATCMountServiceListener (uevent-based)
- ✅ /proc/mounts 查询 ↔ Android HEnvironment.getStorageState()
- ✅ 启动时扫描已有设备 ↔ Android initStorageDevice() in onCreate
- ✅ USB/SD/Flash 路径分类 ↔ Android IConstant.PATH_USB_PREFIX / PATH_SD

### [P1-GAP-1] USB 多分区支持缺失
**Android**: FileStorageState.getUSBMountedList() 枚举 /storage/usb-otg/ 下所有子目录,
LocalService 对每个分区独立管理 StorageDeviceEx。USB Hub 场景下一个物理USB产生
多个 sda1/sda2 分区,各自独立挂载。

**当前实现**: usb_monitor 的 remove 事件中 `sda1→/mnt/usb0` 硬编码映射是**错误的**。
多分区时 sda1=/mnt/usb0, sda2=/mnt/usb0p2,甚至不同平台前缀不同。

**修复**: remove 时应查历史记录而非猜测 mount point。增加一个 `mount_point_cache[]`
在 add 时记录 dev→mount 映射,remove 时查询。

### [P2-GAP-2] unmount 时 USB Hub 部分拔出不应停止全部播放
**Android** (LocalService:1006-1030): `onMediaStorageDeviceUnmounted()` 检查
`mAppData.mCurrentMediaInfo.mFilePath.contains(strPath)`,只有当前播放文件在被拔出
的盘符上才停止播放,其他盘符的播放不受影响。

**当前实现**: `on_storage_event(UNMOUNTED)` 中 `idx == current_device_idx` 就无条件
`music_app_stop()`,但没检查当前正在播放的文件路径是否属于该设备。如果 USB Hub 拔出
一个分区而正在播放另一个分区的歌,会误停。

**修复**: 在 stop 前增加路径前缀匹配检查。

---

## 二、文件扫描 (music_scanner.c vs MediaFilePathScan.java)

### 已正确对齐的部分
- ✅ 后缀过滤 (.mp3/.wav/.flac/.aac/.ogg/.wma/.m4a) ↔ MUSIC_SUFFIX
- ✅ 递归扫描 ↔ scanMediaPathList 递归
- ✅ .nomedia 过滤 (已在本轮修复中增加)
- ✅ 隐藏文件过滤 (已在本轮修复中增加)
- ✅ 非扫描路径过滤 (/Android, /LOST.DIR 等) (已在本轮修复中增加)
- ✅ symlink 防护 (已在本轮修复中增加)

### [P1-GAP-3] 文件夹浏览模式缺失
**Android**: MediaFilePathScan 不仅收集所有音频文件到 mMusicOnlyList,还收集包含音频
的文件夹到 mMediaFolderList,支持用户按文件夹浏览。LocalService.tryPlayUsbFolderMusic()
可以播放某个文件夹下的歌。

**当前实现**: music_scanner 只扁平扫描所有文件,没有维护文件夹列表。
MusicInfo 结构体有 device_name 但没有 parent_folder 字段。

**修复**: 在 MusicInfo 中增加 `char folder_path[MUSIC_MAX_PATH_LEN]`,扫描时记录
`dirname(filepath)`。music_app 中增加按文件夹过滤播放列表的 API。

### [P1-GAP-4] ID3 分类(按专辑/艺术家)缺失
**Android**: MediaService.classifyMediaInfoList() 在 ID3 扫描完成后,将歌曲按
album→List<MusicInfo>, artist→List<MusicInfo> 分类存储到 StorageDeviceEx.mAlbumListMap
和 mArtistListMap。UI 上可以按专辑/艺术家浏览。

**当前实现**: music_scanner 解析了 artist/album 字段但 music_app 没有做分类。

**修复**: 在 music_app.c 中增加 `classify_music_list()` 函数,扫描完成后调用。
维护简单的链表或数组索引按 album/artist 分组。

### [P2-GAP-5] 扫描取消机制缺失
**Android**: MediaFilePathScan 使用 `state.mLoadingIndex.get() != nLoadingIndex` 来
支持扫描取消——如果新扫描请求到来,旧的扫描线程检查 index 不匹配会自动退出。

**当前实现**: scan_thread_func 没有检查取消标志。如果用户拔出USB再插入,旧线程继续
扫描已不存在的路径,虽然不会崩溃(opendir失败返回),但浪费资源。

**修复**: 在 storage_device_state_t 中增加 `volatile int scan_generation`,
scan_thread_func 每扫描100个文件检查一次。

---

## 三、播放控制 (music_player.cpp vs LocalService.onPlayControl + MediaPlayerModel)

### 已正确对齐的部分
- ✅ play/pause/stop/next/prev 基本控制
- ✅ 四种播放模式 (sequential/repeat-all/repeat-one/shuffle) ↔ IMusicState.REPEAT_MODE_*
- ✅ Fisher-Yates shuffle
- ✅ 自然播放结束自动切下一首

### [P0-GAP-6] 上下曲防抖缺失
**Android** (LocalService:1932-1968): `onPlayControl()` 中 PREV/NEXT 命令有 500ms
`ControlHandler.EVENT_KEY_PREV_NEXT_FILTER` 防抖。方向盘按键连续触发时,第一次有效,
500ms 内的重复触发被丢弃。没有这个防抖,旋钮快速拨动会跳过多首歌。

**当前实现**: `music_app_next()` / `music_app_prev()` 没有任何防抖,直接透传到
`music_player_next()`。车载旋钮/方向盘按键可能在 100ms 内连续触发 5-10 次。

**修复**: 在 music_app.c 中增加:
```c
static uint64_t s_last_next_prev_ms = 0;
#define PREV_NEXT_DEBOUNCE_MS 500

void music_app_next(void) {
    uint64_t now = timer_manager()->get_elapsed_ms();
    if (now - s_last_next_prev_ms < PREV_NEXT_DEBOUNCE_MS) return;
    s_last_next_prev_ms = now;
    if (s_app.player) music_player_next(s_app.player);
}
```

### [P0-GAP-7] 播放错误文件自动跳过+重新扫描缺失
**Android** (LocalService:787-810): EVENT_ERROR_FILE_NOT_EXIST 计数器
`mAppData.mFileNotExistCount`,连续3次文件不存在后触发 `requestScanStorageDevice()`
重新扫描。单次错误时自动跳到下一首。

**当前实现**: music_player.cpp 的 `media_state_callback(ErrorState)` 只设置
`PLAYER_STATE_ERROR`,没有自动跳下一首,也没有重试计数。用户看到 Error 必须手动
点 Next。

**修复**: 在 `on_player_state()` 回调中增加:
```c
if (state == PLAYER_STATE_ERROR) {
    s_app.error_count++;
    if (s_app.error_count >= 3) {
        music_app_rescan(); /* 触发重新扫描 */
        s_app.error_count = 0;
    } else {
        music_app_next(); /* 自动跳下一首 */
    }
}
```

### [P1-GAP-8] 播放完成时进度重置缺失
**Android** (LocalService:699-719): EVENT_MEDIA_COMPLETION 时
`writeMediaTime(type, path, 0, 102)` 将当前歌曲的记忆进度写为 0,
这样下次选中同一首歌时从头播放而非从尾部恢复。

**当前实现**: track 播放完成后自动 next,但没有将前一首的 last_position 清零。
如果用户记忆播放恢复到该歌曲,会从末尾位置恢复(几乎立即完成),导致
连续自动跳曲。

**修复**: 在 `on_player_state(STOPPED)` → auto-next 路径中,先清除
`s_app.state.last_position_ms = 0` 并写入 state file。

### [P1-GAP-9] 自动切曲延迟缺失
**Android**: 播放完成后 `H0.sendEmptyMessageDelayed(MSG_GOTO_NEXT_MEDIA, 1000)` ——
**1秒延迟**后才切下一首。这是为了:
1. 给 ALSA buffer drain 时间
2. 避免快速连续切歌的音频爆音
3. 让 UI 有时间显示 "播放完成" 状态

**当前实现**: `media_state_callback(StoppedState)` 中直接调用 `music_player_next()`,
无延迟。

**修复**: 使用 AWTK `timer_add()` 增加 1000ms 延迟:
```c
static ret_t delayed_next_timer(const timer_info_t* timer) {
    music_player_next(s_app.player);
    return RET_REMOVE;
}
// 在 StoppedState 回调中:
timer_add(delayed_next_timer, NULL, 1000);
```

---

## 四、音频焦点 (缺失模块 vs LocalService AudioFocusChangeListener)

### [P1-GAP-10] 完全缺失音频焦点管理
**Android** (LocalService:2233-2306 + 3800-3920):
完整的 AudioFocus 状态机:
- GAIN: 恢复播放 + 音量恢复到 1.0
- LOSS: 暂停 + 释放 MediaButton + 允许恢复标志
- LOSS_TRANSIENT: 暂停 + 允许恢复
- LOSS_TRANSIENT_CAN_DUCK: 音量降到 0.5(或系统配置值)

Linux 平台没有 Android AudioManager,但如果系统中有蓝牙电话、导航语音等其他音频
应用,同样需要协调。AC8215 平台通过 `audio_policy` 或自定义 IPC 实现类似功能。

**当前实现**: 完全没有音频焦点概念。如果蓝牙来电,音乐不会暂停;如果导航播报,
音量不会降低。

**修复方案(P1,需确认平台API)**:
1. 如果平台有 audio_policy 服务,注册监听回调
2. 如果没有,至少监听蓝牙HFP状态(通过 cluster-service 的 IClusterCallBack)
3. 增加 `audio_focus.h/c` 模块封装平台差异

---

## 五、状态持久化 (music_app.c save/restore vs Preferences + writeMediaTime)

### 已正确对齐的部分
- ✅ play_mode 持久化 ↔ Preferences.writePlayRepeatMode
- ✅ last_path 持久化 ↔ mRemoteService.readMediaPath

### [P1-GAP-11] 播放时间记忆粒度不足
**Android** (LocalService:2310-2330): `writeCurrentMediaTime()` 保存的是
`mPlayTimeInfo.mCurrentTime`(毫秒级进度),且每次暂停/停止/焦点丢失/ACC-OFF
都会调用。恢复时 `readMediaTime(type, path)` 精确到毫秒。

**当前实现**: `music_app_save_state()` 只保存 `current_position_ms`,但只在
`deinit()` 时调用——**正常运行中的意外掉电不会保存**。

**修复**: 增加定时保存(每 30 秒)+ 暂停/停止时立即保存:
```c
/* 在 on_player_state() 中: */
if (state == PLAYER_STATE_PAUSED || state == PLAYER_STATE_STOPPED) {
    music_app_save_state();
}
/* 另增加 30 秒定时器 */
```

### [P1-GAP-12] 恢复播放时未 seek 到记忆位置
**Android**: `readMediaTime()` 读取记忆时间,播放后 `seekToTime(nTime)` 恢复位置。

**当前实现**: `music_app_restore_state()` 读取了 `last_position_ms`,但没有在
自动播放时 seek 到该位置。用户重启后总是从头播放。

**修复**: 在 scan_complete → auto-play 路径中,找到 last_path 对应的 index 后:
```c
music_player_play(s_app.player, index);
if (s_app.state.last_position_ms > 0) {
    music_player_seek(s_app.player, s_app.state.last_position_ms);
}
```

---

## 六、ACC/电源管理 (缺失模块 vs LocalService power handling)

### [P2-GAP-13] ACC-OFF / 深度休眠处理缺失
**Android** (LocalService:1186-1230): `onEnterDeepSleepStatus()` 在 ACC-OFF 时:
1. 停止播放
2. 清空所有播放列表
3. 清空当前播放信息
4. 强制设置设备 loading=true(唤醒后等待重新扫描)

**当前实现**: 无 ACC/电源状态监听。AC8215 平台有 `CarStatus.getAccStatus()`,
如果 ACC-OFF 时不停止播放,继续消耗电池。

**修复**: 通过 cluster-service 的 IDC 通道或 /sys 节点监听 ACC 状态。

---

## 七、UI层 (music_ui.c vs MusicInfoLayout + MusicListLayout)

### [P1-GAP-14] 播放列表使用了 label 而非虚拟列表
**Android**: MusicListLayout 使用 RecyclerView + ViewHolder 模式,10000 首歌只创建
屏幕可见的 ~15 个 item view,滚动时复用。

**当前实现**: `rebuild_playlist_view()` 最多创建 100 个 label widget。超过 100 首的
歌无法显示。每次 track change 都全量重建,造成闪烁。

**修复**: 使用 AWTK 的 `list_view_create()` + `list_item_creator` 回调模式实现
虚拟滚动。这是 AWTK 原生支持的。

### [P2-GAP-15] 歌词显示缺失
**Android**: media-view 模块有 LyricsManager.java 支持 .lrc 歌词文件解析和同步显示。

**当前实现**: 无歌词支持。作为车载音乐播放器,歌词是重要功能。

**修复**: 后续版本增加 lrc_parser.c + 歌词同步显示 widget。

---

## 八、总结修复优先级

| ID | 级别 | 问题 | 涉及文件 | 工作量 |
|----|------|------|----------|--------|
| GAP-6  | **P0** | 上下曲防抖缺失 | music_app.c | 10行 |
| GAP-7  | **P0** | 错误文件无自动跳过/重扫 | music_app.c | 20行 |
| GAP-1  | P1 | USB多分区remove映射错误 | usb_monitor.c | 40行 |
| GAP-2  | P1 | unmount路径匹配不精确 | music_app.c | 10行 |
| GAP-3  | P1 | 文件夹浏览模式 | music_scanner.h/c, music_app | 60行 |
| GAP-4  | P1 | 按专辑/艺术家分类 | music_app.c | 80行 |
| GAP-8  | P1 | 播放完成进度不清零 | music_app.c | 5行 |
| GAP-9  | P1 | 自动切曲无1秒延迟 | music_player.cpp | 15行 |
| GAP-10 | P1 | 无音频焦点管理 | 新文件 audio_focus.h/c | 200行 |
| GAP-11 | P1 | 掉电不保存进度 | music_app.c | 20行 |
| GAP-12 | P1 | 恢复播放不seek | music_app.c | 10行 |
| GAP-5  | P2 | 扫描无取消机制 | music_scanner.c, music_app.c | 30行 |
| GAP-13 | P2 | 无ACC/电源管理 | 新文件 + 平台适配 | 100行 |
| GAP-14 | P1 | 播放列表非虚拟滚动 | music_ui.c | 80行 |
| GAP-15 | P3 | 无歌词支持 | 新文件 lrc_parser.h/c | 300行 |
