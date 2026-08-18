# Knuth 式第二轮严格审查 — AWTK musicplayer vs Android AutoMediaPlayer

> 审查人: 以 Knuth《计算机程序设计艺术》作者视角
> 审查日期: 2026-08-17
> 审查范围: `source/packages/application/musicplayer/awtk_app/` 全部 C 代码
> 参考标准: `android_ref/autoapps/AutoMediaPlayer/` 全部 Java/C++ 代码
> 审查方法: 逐文件、逐函数对比; 从用户角度 + 系统角度双重批判

---

## 一、用户角度缺陷 (会直接影响用户体验)

### Issue #14 — [P0] music_app.c: scan_thread_func 中 build_folder_cache / build_classification_cache 在无锁情况下访问共享数据

**文件**: `awtk_app/src/music_app.c` 第 242-250 行

**问题**: `scan_thread_func()` 在后台线程中调用 `build_folder_cache()` 和 `build_classification_cache()`, 这两个函数直接读写 `s_app.folder_paths[]`, `s_app.album_groups[]`, `s_app.artist_groups[]` 等共享数据, 但**没有持有 `s_app.mutex`**。

如果用户在扫描过程中切换到文件夹/专辑/艺术家页面(调用 `music_app_get_folder_list()` / `music_app_get_album_list()` / `music_app_get_artist_list()`), UI线程会同时读取这些数据, 产生**数据竞争(data race)**。

**Android 对比**: Android 中 MediaService.classifyMediaInfoList() 运行在 HandlerThread 中, 通过 Handler 消息序列化访问。

**用户症状**: 插入USB时浏览文件夹列表可能显示乱码或崩溃。

**修复**: 在 `build_folder_cache()` 和 `build_classification_cache()` 中加锁, 或确保调用时持有 mutex。

---

### Issue #15 — [P0] music_app.c: on_player_state() 回调中调用 timer_add / timer_remove 不是线程安全的

**文件**: `awtk_app/src/music_app.c` 第 290-330 行

**问题**: `on_player_state()` 从 player 线程调用, 其中调用了 AWTK 的 `timer_add()` 和 `timer_remove()` — 但 AWTK 定时器 API **不是线程安全的**, 只能从主线程调用。

```c
static void on_player_state(PlayerState state, void* user_data) {
    ...
    s_app.auto_next_timer_id = timer_add(delayed_auto_next_cb, NULL, 500);
    ...
}
```

**Android 对比**: Android 通过 `Handler.sendEmptyMessageDelayed()` 保证延迟任务在 HandlerThread 中执行, 天然线程安全。

**用户症状**: 播放错误时随机崩溃, 特别是在快速切歌时。

**修复**: 改为使用 `idle_queue()` 将 timer 操作投递到主线程执行。

---

### Issue #16 — [P1] music_app.c: music_app_play_folder() 只播放第一首, 不切换播放列表

**文件**: `awtk_app/src/music_app.c` 第 500-520 行

**问题**: `music_app_play_folder()` 只是在全局播放列表中找到该文件夹的第一首歌并跳转播放, 但**不改变播放列表**。用户点"下一首"时会离开该文件夹, 播放其他文件夹的歌。

**Android 对比** (`ILocalzModel.requestPlayMusicInfo()`): Android 中进入文件夹播放时, 调用 `updatePlaylist(IPlaylistType.FOLDER_LIST, folderMusicList)` 将播放列表替换为该文件夹内的歌曲。

**用户症状**: 用户进入文件夹想只播放该文件夹的歌, 但下一首会跳到其他文件夹。

**修复**: `music_app_play_folder()` 应构建一个仅包含该文件夹歌曲的子列表, 并通过 `music_player_set_playlist()` 设置为当前播放列表。

---

### Issue #17 — [P1] music_app.c: music_app_play_group() 同理, 不切换播放列表

**文件**: `awtk_app/src/music_app.c` 第 553-570 行

**问题**: 同 Issue #16。`music_app_play_group()` 在全局列表中定位歌曲并播放, 但下一首仍是全局顺序, 不限于所选专辑/艺术家。

**Android 对比**: Android 中点击专辑中的歌曲时, 将该专辑的歌曲列表设为当前播放列表(`DEVICE_LIST` 类型), 然后从所选位置开始播放。

**用户症状**: 用户选了一张专辑, 但下一首播的是另一张专辑的歌。

**修复**: 类似 Issue #16, 构建子播放列表。

---

### Issue #18 — [P1] music_player.cpp: music_player_play() 中 shuffle 模式下 get_actual_index 错误

**文件**: `musicplayer/music_player.cpp` 第 160-168 行

**问题**: `music_player_play(ctx, index)` 中, `index` 参数是**逻辑索引**(用户在 UI 中看到的位置), 但当 mode==SHUFFLE 时, 通过 `get_actual_index(ctx, index)` 映射到 shuffle 后的实际索引。这意味着:
- 用户点击列表中第 5 首歌, 实际播放的是 shuffle_order[5], 可能是第 12 首。
- **用户的直接点击操作不应该经过 shuffle 映射**, shuffle 只应影响 next/prev 自动跳转。

**Android 对比** (`MusicPlaylistEx.adjustPlayPosition()`): Android 的 shuffle 只影响 `adjustPlayPosition(isNextSong)` 中的自动切换, 用户点击列表项直接播放该首, 不经过随机映射。

**用户症状**: 用户点击歌曲A, 实际播放的是歌曲B (在 shuffle 模式下)。

**修复**: `music_player_play()` 应直接使用 `index` 作为实际索引; shuffle 映射只在 `music_player_next()` 和 `music_player_prev()` 中使用。

---

### Issue #19 — [P1] music_player.cpp: shuffle 模式 prev/next 逻辑不正确

**文件**: `musicplayer/music_player.cpp` 第 230-260 行

**问题**: 当前的 shuffle 实现使用了预生成的 shuffle_order 数组, next 操作只是 `(next+1) % count`, 这实际上是一个**固定的随机序列**, 不是真正的随机:
1. `prev()` 在 shuffle 模式下只是 `index-1`, 不会回到上一首**实际播放过的**歌。
2. 播放完整个列表后 `generate_shuffle(ctx)` 重新洗牌, 但不保证不重复播放刚听过的歌。

**Android 对比** (`MusicPlaylistEx`): Android 使用 `mRandomPositionList` — 一个**逐步消耗的随机池**:
- `getNextRandomPosition()` 从池中随机取一个, 然后 `removeFromRandomPositionList()` 移除。
- 这保证每首歌恰好播放一次, 不会重复。
- 池用完后自动重置。

**用户症状**: shuffle 模式下有些歌反复播放, 有些从不播放。

**修复**: 改用 Android 的消耗池方式实现 shuffle。

---

### Issue #20 — [P1] music_ui.c: rebuild_playlist_view() 每次 TRACK_CHANGED 都全量重建 — 性能灾难

**文件**: `awtk_app/src/music_ui.c` 第 263-310 行

**问题**: 每次切歌都调用 `rebuild_playlist_view()` → `widget_destroy_children(list)` → 全量重建所有子 widget。即使只是更新高亮行, 也要销毁并重建最多 100 个 label。

**Android 对比**: RecyclerView 的 `notifyItemChanged()` 只更新变化的行。

**用户症状**: 切歌时列表闪烁, 低端设备上可能卡顿 200-500ms。

**修复**: 分两步:
1. 短期: `TRACK_CHANGED` 时只更新旧/新高亮行的颜色, 不全量重建。
2. 长期: 改用 AWTK `list_view` 虚拟滚动 (Issue #12)。

---

### Issue #21 — [P1] music_ui.c: 列表点击 on_list_item_click 中 event target 可能不是预期 widget

**文件**: `awtk_app/src/music_ui.c` 第 137-148 行

**问题**: `on_list_item_click()` 中使用 `WIDGET(e->target)` 获取被点击的 widget, 然后读取其 `item_index` 属性。但 AWTK 的 EVT_CLICK 事件冒泡机制下, `e->target` 可能是 label 的子元素(如果有的话)或者 label 本身。当 list_view 有自己的滚动事件处理时, 点击可能传递给 list_view 而非具体的 item。

**Android 对比**: RecyclerView.ViewHolder 通过 `getAdapterPosition()` 精确获取位置。

**用户症状**: 某些情况下点击播放列表中的歌曲无反应。

**修复**: 使用 `widget_get_prop_int(item, "item_index", -1)` 前先验证 item 确实有该属性, 或改用 AWTK 的 `list_view` + `list_item` 组合。

---

### Issue #22 — [P2] favorite_manager.c: favorite_toggle() 非原子操作, 有 TOCTOU 竞争

**文件**: `awtk_app/src/favorite_manager.c` 第 148-157 行

**问题**:
```c
bool favorite_toggle(const MusicInfo* info) {
    if (favorite_contains(info->filepath)) {  // 查询 (锁→解锁)
        favorite_remove(info->filepath);       // 删除 (锁→解锁)
        return false;
    } else {
        favorite_add(info);                    // 添加 (锁→解锁)
        return true;
    }
}
```
`contains` 和 `remove/add` 之间 mutex 已经释放, 如果两个线程同时 toggle 同一首歌, 可能导致:
- 两个线程都看到 `contains==true`, 都执行 `remove`, 但实际只有一条, 第二次 `remove` 是空操作 → 逻辑正确但返回值错误。
- 两个线程都看到 `contains==false`, 都执行 `add`, 添加了两条相同记录。

**Android 对比**: Android FavoriteManager 是单线程操作(在 HandlerThread 中), 天然原子。

**用户症状**: 快速双击收藏按钮可能导致收藏列表中出现重复条目。

**修复**: `favorite_toggle()` 内部持有锁做完 check + action:
```c
bool favorite_toggle(const MusicInfo* info) {
    if (!info) return false;
    pthread_mutex_lock(&s_fav.mutex);
    int idx = find_by_path(info->filepath);
    if (idx >= 0) {
        // 内联 remove 逻辑 (不释放锁)
        ...
        pthread_mutex_unlock(&s_fav.mutex);
        return false;
    } else {
        // 内联 add 逻辑 (不释放锁)
        ...
        pthread_mutex_unlock(&s_fav.mutex);
        return true;
    }
}
```

---

### Issue #23 — [P1] music_app.c: 缺少 "当前播放列表类型" 跟踪 — 收藏夹播放后无法恢复

**文件**: `awtk_app/src/music_app.c` / `music_app.h`

**问题**: 当前实现没有 `IPlaylistType` 的 C 语言等价物。所有播放都使用同一个全局播放列表(设备扫描结果)。Android 支持:
- `DEVICE_LIST` — 全设备播放
- `FOLDER_LIST` — 文件夹播放
- `FAVORITE_LIST` — 收藏夹播放

切换播放列表类型时, Android 会备份第一播放列表(`MusicPlaylistEx.mFirstPlaylistEx`), 收藏夹播放结束后可恢复。

**用户症状**: 从收藏夹开始播放后, 无法回到 "全部歌曲" 播放模式, 除非重新扫描。

**修复**: 在 `music_app_state_t` 中增加 `playlist_type` 字段和备份机制。

---

### Issue #24 — [P2] music_app.c: music_app_get_lyrics_line() 二分搜索边界条件

**文件**: `awtk_app/src/music_app.c` 第 710-725 行

**问题**: `music_app_get_lyrics_line()` 中, 当 `time_ms` 小于第一行歌词的时间戳时, `result` 初始化为 0, 函数返回 0。但正确行为应该返回 -1(表示还没到歌词开始), 或者 0 并由 UI 判断是否高亮。

**Android 对比**: `LyricsManager` 在当前时间早于所有歌词行时不更新显示。

**用户症状**: 歌曲前奏时歌词区域显示第一行歌词而非空白。

**修复**: 如果 `time_ms < s_app.lyrics.lines[0].time_ms`, 返回 -1。

---

## 二、系统角度缺陷 (稳定性、资源管理、安全性)

### Issue #25 — [P0] music_app.c: music_app_init() 中 memset(&s_app, 0) 覆盖了已初始化的 mutex

**文件**: `awtk_app/src/music_app.c` 第 350 行

**问题**:
```c
int music_app_init(music_app_ui_callback_t ui_cb) {
    if (s_app.inited) { ... return -1; }
    memset(&s_app, 0, sizeof(s_app));      // ← 清零整个结构
    pthread_mutex_init(&s_app.mutex, NULL); // ← 然后初始化 mutex
```

`s_app` 是静态全局变量, 如果 `music_app_deinit()` 后再次调用 `music_app_init()` (例如应用重启), `memset` 会在 `pthread_mutex_destroy()` 已调用后覆盖 mutex 内存 — 这是未定义行为, 某些 pthread 实现会泄漏资源或崩溃。

更严重的是: `memset` 在检查 `inited` 之后, 意味着第一次初始化没有问题, 但如果应用 deinit 后 reinit, `inited==false` → `memset` 清零 → 如果此时其他线程(如USB monitor 的遗留线程)还在访问 `s_app` → **use-after-free 或数据损坏**。

**修复**: 不要 memset 整个结构。逐字段初始化, 或确保 deinit 完全清理了所有线程后再 reinit。

---

### Issue #26 — [P1] music_player.cpp: media_state_callback(StoppedState) 中直接调用 music_player_next() 可能递归死锁

**文件**: `musicplayer/music_player.cpp` 第 30-50 行

**问题**:
```c
case MediaPlayer::StoppedState:
    if (ctx->state == PLAYER_STATE_PLAYING) {
        ctx->state = PLAYER_STATE_STOPPED;
        pthread_mutex_unlock(&ctx->mutex);
        music_player_next(ctx);  // ← next() 内部会 lock mutex
        return;
    }
```

`media_state_callback` 调用链: MediaPlayer 内部线程 → `media_state_callback()` → `music_player_next()` → `music_player_play()` → `ctx->player->play()` — 如果 `play()` 同步触发另一个 `media_state_callback`(某些 MediaPlayer 实现会这样), 就会再次尝试 lock 同一个 mutex, 导致**死锁**(非递归 mutex)。

**Android 对比**: Android 通过 Handler 消息将 auto-next 延迟到消息循环中执行, 完全避免了回调嵌套。

**修复**: `media_state_callback()` 中不直接调用 `music_player_next()`, 而是通过 `on_player_state` 回调交给 `music_app.c`, 由 music_app.c 使用 AWTK timer 延迟调用 next, 也就是 GAP-9 的修复方案。**检查确认**: 当前代码中 `on_player_state()` 已经在 STOPPED 状态下设置了延迟 timer — 但 `music_player.cpp` 中 `media_state_callback` 仍然直接调用 `music_player_next()`, 这意味着 auto-next 实际上被**执行了两次** — 一次在 player 线程 (music_player.cpp), 一次在 AWTK 主线程 (delayed_auto_next_cb)。**这是一个双跳 bug**。

**修复**: 从 `music_player.cpp` 中移除 StoppedState 下直接调用 `music_player_next()` 的逻辑, 完全由 `on_player_state` 回调通过 `music_app.c` 的延迟 timer 处理。

---

### Issue #27 — [P1] music_app.c: on_player_state(PLAYER_STATE_ERROR) 中 timer_add 从非 AWTK 线程调用

**文件**: `awtk_app/src/music_app.c` 第 290-310 行

**问题**: 同 Issue #15 的具体表现。`on_player_state()` 从 player 回调线程调用, 其中:
```c
s_app.auto_next_timer_id = timer_add(delayed_auto_next_cb, NULL, 500);
```
AWTK 的 `timer_add()` 不是线程安全的, 必须在主线程调用。

**修复**: 将整个 error/stopped 处理逻辑改为通过 `idle_queue()` 投递到主线程。

---

### Issue #28 — [P1] usb_monitor.c: nl_sock bind 可能因为 nl_pid=getpid() 与其他 uevent 监听者冲突

**文件**: `awtk_app/src/usb_monitor.c` 第 270 行

**问题**:
```c
addr.nl_pid = getpid();
```
如果同一进程中有其他组件也创建了 NETLINK_KOBJECT_UEVENT socket 并使用 `getpid()` 作为 `nl_pid`, `bind()` 会失败(EADDRINUSE)。正确做法是用 0 让内核自动分配。

**Android 对比**: Android 的 uevent 监听使用 UEventObserver, 内核分配 nl_pid。

**修复**: `addr.nl_pid = 0;`

---

### Issue #29 — [P2] music_scanner.c: ID3v2 解析缺少对 ID3v2.2 (frame ID 3字节) 的支持

**文件**: `musicplayer/music_scanner.c` 第 100-160 行

**问题**: 当前 ID3 解析只处理 v2.3/v2.4 (4字节 frame ID: TIT2, TPE1, TALB)。ID3v2.2 使用 3 字节 frame ID (TT2, TP1, TAL), 且 frame header 是 6 字节而非 10 字节。一些老旧的 MP3 文件仍使用 v2.2。

**Android 对比**: Android 的 MediaMetadataRetriever 支持所有 ID3 版本。

**用户症状**: 老 MP3 文件无法显示标题/艺术家。

**修复**: 增加 `version_major == 2` 的分支, 使用 3 字节 frame ID 和 3 字节 size (big-endian 24-bit)。

---

### Issue #30 — [P2] music_app.c: build_folder_cache 使用线性查找去重, O(n²) 复杂度

**文件**: `awtk_app/src/music_app.c` 第 455-490 行

**问题**: 对每个文件, 用线性搜索 `s_app.folder_paths[]` 检查重复。2000 个文件 × 100 个文件夹 = 200,000 次字符串比较。在低端 ARM 设备上可能耗时数秒。

**修复**: 先排序后去重, 或使用简单哈希表。

---

## 三、修复清单

| Issue | 级别 | 简述 | 涉及文件 | 类型 |
|-------|------|------|----------|------|
| #14 | **P0** | 扫描线程无锁访问共享分类数据 | music_app.c | Bug |
| #15 | **P0** | timer_add 从非 AWTK 线程调用 | music_app.c | Bug |
| #25 | **P0** | memset 覆盖已初始化 mutex | music_app.c | Bug |
| #26 | **P1** | auto-next 双跳: player.cpp + app.c 各执行一次 | music_player.cpp | Bug |
| #16 | **P1** | 文件夹播放不切换播放列表 | music_app.c | 功能 |
| #17 | **P1** | 分组播放不切换播放列表 | music_app.c | 功能 |
| #18 | **P1** | shuffle 模式用户点击被映射到错误歌曲 | music_player.cpp | Bug |
| #19 | **P1** | shuffle 消耗池未实现,会重复播放 | music_player.cpp | 功能 |
| #20 | **P1** | 每次切歌全量重建播放列表 UI | music_ui.c | 性能 |
| #22 | **P2** | favorite_toggle TOCTOU 竞争 | favorite_manager.c | Bug |
| #23 | **P1** | 无播放列表类型跟踪和备份 | music_app.h/c | 功能 |
| #24 | **P2** | 歌词二分搜索前奏边界 | music_app.c | Bug |
| #27 | **P1** | error 处理 timer 线程安全 | music_app.c | Bug |
| #28 | **P1** | netlink pid 冲突 | usb_monitor.c | Bug |
| #29 | **P2** | ID3v2.2 不支持 | music_scanner.c | 功能 |
| #30 | **P2** | folder cache O(n²) | music_app.c | 性能 |
