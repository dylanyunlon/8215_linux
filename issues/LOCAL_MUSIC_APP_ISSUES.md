# 本地音乐 App 功能审查 — AWTK awtk_app vs Android AutoMediaPlayer

审查方法: 以 `android_ref/autoapps/AutoMediaPlayer` 为标准，逐功能对比
`source/packages/application/musicplayer/awtk_app` 的 C 代码实现。

AUDIT_REPORT.md 中 GAP-1~12 的代码层修复已确认落地。
以下为**仍然缺失**的功能点。

---

## Issue #1 — [P1] 收藏(Favorite)管理功能完全缺失

**Android 参照**: `FavoriteManager.java`(358行), `IMediaEvent.EVENT_MUSIC_FAVORITE_OPERATE`,
`MusicViewModel.mFavoriteListOperateListener`

**Android 行为**:
- 用户点击收藏按钮 → `addFavoriteMusic(info)` 加入收藏列表
- 收藏列表持久化到 Room 数据库 (`FavoriteMusic.java` / `FavoriteMusicDao.java`)
- 收藏列表可作为独立播放列表切换播放
- 上限128首，超出提示
- 收藏/取消收藏有事件回调通知 UI 刷新图标状态

**AWTK 现状**: 无收藏数据结构、无收藏 API、无收藏按钮、无收藏持久化

**需新增**: `favorite_manager.h/c`
**需修改**: `music_app.h/c` (集成收藏API), `music_ui.c` (收藏按钮+列表)

---

## Issue #2 — [P1] 文件夹浏览(Folder Browse)功能缺失

**Android 参照**: `FolderListLayout.java`(533行), `MediaFilePathScan.java`,
`FilePathScanManager.java`, `MusicFolderListAdapter.java`

**Android 行为**:
- 用户可进入文件夹列表页面
- 显示包含音频文件的文件夹 (`mMediaFolderList`) + 当前目录下的音频文件 (`mMusicOnlyList`)
- 点击文件夹进入子目录 (递归浏览)
- 点击返回按钮回到上级目录
- 点击音频文件播放当前文件夹下的所有音频 (以文件夹为播放列表)
- 路径记忆: 下次进入文件夹页面恢复上次浏览位置

**AWTK 现状**: `music_scanner.c` 只做扁平扫描全部文件, 无文件夹层级浏览

**MusicInfo 缺失**: 无 `folder_path` 字段, 无 `mIndex=-1` 文件夹标记约定

**需修改**: `music_scanner.h` (MusicInfo增字段), `music_app.h/c` (文件夹过滤API), `music_ui.c` (文件夹页面)

---

## Issue #3 — [P1] 按专辑/艺术家分类浏览缺失

**Android 参照**: `AlbumListLayout.java`(239行), `ArtistListLayout.java`(231行),
`IMusicState.PAGE_INDEX_ALBUM/ARTIST`

**Android 行为**:
- 全部歌曲按 album 字段分组, 用户选择专辑 → 展示该专辑歌曲列表
- 全部歌曲按 artist 字段分组, 同理
- 点击歌曲以该分组为播放列表开始播放

**AWTK 现状**: `music_scanner.c` 解析了 artist/album 字段, 但 `music_app.c` 无分组索引

**需新增**: 分类索引数据结构 + 分类浏览 API
**需修改**: `music_app.h/c`, `music_ui.c`

---

## Issue #4 — [P1] 多播放列表类型(PlaylistType)切换缺失

**Android 参照**: `MusicPlaylistEx.java`, `IPlaylistType.java`, `BaseMediaData.mMusicPlaylistEx`

**Android 行为**:
- 支持播放列表类型: 全部/USB/SD/Flash/文件夹/收藏/专辑/艺术家
- 切换播放列表类型时记忆旧位置
- 不同列表类型可各自独立播放位置

**AWTK 现状**: 只有一个全局 MusicList, 切设备直接替换

---

## Issue #5 — [P1] SeekBar 拖动时应暂停时间更新(EVENT_SCROLL_SEEKBAR)

**Android 参照**: `MusicInfoLayout.initSeekBarCtrl()` →
`onStartTrackingTouch` 发送 `EVENT_SCROLL_SEEKBAR`,
`LocalService` 收到后停止 `mTimeRunnable` 定时器

**Android 行为**:
- 用户开始拖动进度条 → 暂停定时器更新进度 → 避免进度条跳动
- 用户松手 → `EVENT_STOP_SCROLL_SEEKBAR` → 恢复定时器 + 发送 seek 命令

**AWTK 现状**: `music_ui.c` 有 `s_slider_dragging` 标志位, `on_slider_pointer_down/up` 设置它,
`APP_EVENT_POSITION_CHANGED` 事件处理中检查 `s_slider_dragging` 跳过更新 — **基本对齐但不完整**:
- 问题: `on_slider_value_changed` 在拖动过程中每次变化都 seek, 会导致卡顿
- Android: 只在 `onStopTrackingTouch` 时发一次 seek

**需修改**: `music_ui.c` — `on_slider_value_changed` 中拖动过程不应 seek, 只在 pointer_up 时 seek

---

## Issue #6 — [P2] 音乐搜索功能缺失

**Android 参照**: `MusicSearchFragment.java`, `MusicSearchFragmentEx.java`,
`FolderListLayout.search()`, `BaseMediaData.mSearchList`

**Android 行为**:
- 音乐列表页面有搜索按钮
- 输入关键词 → 按 title/artist/album/filename 模糊匹配
- 搜索结果可直接点击播放

**AWTK 现状**: 无搜索功能

---

## Issue #7 — [P1] ID3 专辑封面图(AlbumArt)显示缺失

**Android 参照**: `MusicInfoLayout.updateMusicImage()`, `BitmapCache.loadNativeImage()`,
`MusicInfo.mID3Type`

**Android 行为**:
- 播放界面显示专辑封面图
- ID3v2 中嵌入的 APIC 帧提取为 Bitmap 显示
- 无封面时显示默认图 `default_thumbnails_bg`
- 有 LRU 缓存避免重复解析

**AWTK 现状**: `music_scanner.c` 的 `music_parse_id3v2` 只解析 title/artist/album 文字,
未提取 APIC 图片帧。`music_ui.c` 无专辑封面显示区域。

**需修改**: `music_scanner.h/c` (ID3 APIC 提取), `music_ui.c` (图片显示)

---

## Issue #8 — [P2] 歌词(LRC)显示缺失

**Android 参照**: `LyricsManager.java`(媒体-view模块), `LyricsView.java`, `LyricsRow.java`,
`MusicInfoLayout.updateLrcRowList()`, `onChangeLyricsView()`

**Android 行为**:
- 查找同名 `.lrc` 歌词文件
- 解析 LRC 时间标签
- 同步滚动显示歌词 (当前行高亮)
- 支持歌词/频谱视图切换
- 无歌词时显示 "暂无歌词" 提示

**AWTK 现状**: 无歌词功能

---

## Issue #9 — [P2] 频谱可视化(Visualizer)缺失

**Android 参照**: `MusicInfoLayout.initVisualizer()`, `PlayFlashView.java`,
`Visualizer.OnDataCaptureListener`

**Android 行为**:
- 播放时显示 FFT 频谱动画
- 用户可切换歌词/频谱视图
- 倒车时停止更新频谱

**AWTK 现状**: 无频谱功能

---

## Issue #10 — [P1] 播放界面"当前曲/总曲数"(index/total)显示缺失

**Android 参照**: `MusicInfoLayout.changeTotalValue()` → `"%d/%d" format(index+1, total)`

**Android 行为**: 播放界面显示 "3/128" 表示当前第3首/共128首

**AWTK 现状**: `music_ui.c` 显示了 "N tracks" 总数, 但无 "当前第几首" 信息

**需修改**: `music_ui.c` — `APP_EVENT_TRACK_CHANGED` 处理中更新 index/total

---

## Issue #11 — [P0] music_app_toggle_play_pause 空指针风险

**代码审查**: `music_app.c` 第380行:
```c
void music_app_toggle_play_pause(void) {
    PlayerState st = music_player_get_state(s_app.player);  // ← s_app.player 可能为 NULL
```

如果 `music_player_create()` 失败(如 libatcmediaplayer.so 缺失), `s_app.player==NULL`,
此处直接传 NULL 进 `music_player_get_state()` 会段错误。

其他 `music_app_*` 函数都有 `if (s_app.player)` 保护, 唯独这个没有。

**需修改**: `music_app.c`

---

## Issue #12 — [P1] 虚拟列表(Virtual ListView)未实现

**Android 参照**: RecyclerView + ViewHolder 模式

**AWTK 现状**: `rebuild_playlist_view()` 每次全量创建 label widget (最多100个),
超过100首歌的无法显示, 每次 track change 都全量重建导致闪烁。

AWTK 原生支持 `list_view + list_item_creator` 虚拟滚动模式。

**需修改**: `music_ui.c` — 改用 AWTK list_view 虚拟滚动

---

## Issue #13 — [P2] ID3 unknown 值显示不友好

**Android 参照**: `MusicInfoLayout.updateId3TextInfo()` →
`"<Unknown>".equals(info.mTitle)` 时回退显示文件名(去扩展名)

**AWTK 现状**: ID3 解析失败时 title 字段填的是 filename, 但 artist/album 为空字符串 `""`
直接显示为空 — 应显示 "未知" 或 "--"

**需修改**: `music_ui.c`

---

## 优先级总结

| Issue | 级别 | 简述 | 状态 |
|-------|------|------|------|
| #11 | **P0** | toggle_play_pause 空指针 | ✅ 已修复 |
| #5 | **P1** | SeekBar拖动中不应连续seek | ✅ 已修复 |
| #10 | **P1** | 当前曲/总曲数显示 | ✅ 已修复 |
| #13 | **P2** | unknown值显示优化 | ✅ 已修复 |
| #1 | **P1** | 收藏功能 | ✅ 已实现 |
| #2 | **P1** | 文件夹浏览 | ✅ API已实现 |
| #6 | **P2** | 搜索功能 | ✅ API已实现 |
| #3 | **P1** | 专辑/艺术家分类 | ✅ API已实现 |
| #7 | **P1** | 专辑封面APIC | ✅ API已实现 |
| #8 | **P2** | LRC歌词解析 | ✅ API已实现 |
| #12 | **P1** | 虚拟列表 | PENDING |
| #4 | **P1** | 多播放列表类型 | PENDING |
| #9 | **P2** | 频谱可视化 | PENDING |
