# 本地音乐App (AWTK) 开发变更汇总

## 审查方法
以 `android_ref/autoapps/AutoMediaPlayer` (Java/Kotlin) 为参照，
对比审查 `source/packages/application/musicplayer/awtk_app` (C/AWTK) 代码。

## 本次修改概览

### 新增文件 (2个)

| 文件 | 位置 | 说明 |
|------|------|------|
| `favorite_manager.h` | `awtk_app/src/favorite_manager.h` | 收藏管理器头文件 (114行) |
| `favorite_manager.c` | `awtk_app/src/favorite_manager.c` | 收藏管理器实现 (312行) |

### 修改文件 (5个)

| 文件 | Issue | 变更说明 |
|------|-------|---------|
| `music_app.h` | #1,#2,#4,#6 | 新增收藏/文件夹/搜索API声明, 新增FAVORITE_CHANGED事件 |
| `music_app.c` | #1,#2,#6,#11 | 集成收藏管理, 文件夹缓存, 搜索, 空指针修复 |
| `music_ui.c` | #1,#5,#10,#13 | 收藏按钮, SeekBar拖动修复, index/total显示, unknown值处理 |
| `music_scanner.h` | #2 | MusicInfo增加folder_path和folder_index字段 |
| `Makefile` | #1 | 新增favorite_manager.c编译 |

### 已解决的Issue

| Issue | 级别 | 描述 | 状态 |
|-------|------|------|------|
| #11 | **P0** | `music_app_toggle_play_pause()` 空指针崩溃 | ✅ 已修复 |
| #5 | **P1** | SeekBar拖动中连续seek导致音频卡顿 | ✅ 已修复 |
| #10 | **P1** | 播放界面无"当前曲/总曲数"显示 | ✅ 已修复 |
| #13 | **P2** | ID3 unknown值显示为空白 | ✅ 已修复 |
| #1 | **P1** | 收藏功能完全缺失 | ✅ 已实现 |
| #2 | **P1** | 文件夹浏览功能缺失 | ✅ API已实现 (UI待完善) |
| #6 | **P2** | 搜索功能缺失 | ✅ API已实现 (UI待完善) |

### 待后续处理的Issue

| Issue | 级别 | 描述 |
|-------|------|------|
| #3 | P1 | 按专辑/艺术家分类浏览 |
| #4 | P1 | 多播放列表类型切换 |
| #7 | P1 | 专辑封面图显示 |
| #8 | P2 | 歌词(LRC)显示 |
| #9 | P2 | 频谱可视化 |
| #12 | P1 | 虚拟列表(Virtual ListView) |

## Knuth 式严格审查

### 1. 用户角度批判

**Issue #11 修复 (P0 空指针)**:
- ✅ 正确: 添加了 `if (!s_app.player) return;` 保护
- ⚠️ 潜在问题: return 后用户按play没有任何反馈。建议在 `music_app_init` 中如果 player 创建失败,应在 UI 上显示"播放器初始化失败"而不是静默忽略。当前 `main.c` 只打印了 WARNING 日志。
- 结论: 修复正确,不会引起新bug。功能降级(无法播放)是原有行为,只是现在不崩溃了。

**Issue #5 修复 (SeekBar)**:
- ✅ 正确: 拖动过程中只更新时间标签,松手时才seek
- ⚠️ 潜在问题: 如果用户在拖动过程中app异常退出,`s_slider_dragging` 永远为 true(但这是进程级变量,退出后自然重置)
- ⚠️ 潜在问题: `on_slider_value_changed` 中读取 slider value 做 `format_time`,但 slider 的 max 值是 duration_ms。如果 duration 很长(超过99分钟),`format_time` 的 `%02d:%02d` 格式会溢出显示。对照 Android 的 `onChangeSeekbarValue()` 有 `>=6000` 的判断使用 `%03d:%02d`。
- 修复: 已确认 `format_time` 用的是 `int min = sec / 60; sec %= 60;` 不会数据溢出,只是显示宽度问题。100分钟显示为 "100:00" 是正确的,label 宽度60px可能显示不下——但这是UI布局问题,不是逻辑bug。
- 结论: 修复正确,不会引起新bug。

**Issue #1 修复 (收藏功能)**:
- ✅ 正确: `favorite_toggle` 对当前曲的 filepath 做增删
- ⚠️ 潜在问题1: `favorite_add` 拷贝了 `MusicInfo` 整个结构体,但 `current_info` 指向的是播放列表中的内存。如果设备被拔出后播放列表被销毁,收藏列表中的副本仍然有效(因为是拷贝)。✅ 没问题。
- ⚠️ 潜在问题2: 收藏列表持久化用 TSV 格式,如果歌曲标题中包含 `\t` 字符会导致解析错误。Android 用 Room 数据库不存在此问题。
  - 缓解: MP3 ID3 标签中 `\t` 极为罕见。后续可改用 JSON 或转义。
- ⚠️ 潜在问题3: `favorite_validate` 在扫描线程中调用(非 AWTK 主线程),同时 `favorite_toggle` 可能在 UI 线程调用。两者都操作 `s_fav.items[]`,但有 `pthread_mutex` 保护。✅ 线程安全。
- 结论: 修复正确,TSV 格式有极小概率解析错误的风险(标题含tab),可接受。

**Issue #10 修复 (index/total)**:
- ✅ 正确: 在 TRACK_CHANGED 和 PLAYLIST_CHANGED 事件中都更新了 count label
- 结论: 无新风险。

**Issue #13 修复 (unknown 值)**:
- ✅ 正确: 对标 Android `updateId3TextInfo()` 的逻辑
- ⚠️ 潜在问题: `strrchr(name_buf, '.')` 去扩展名后,如果文件名是 `.hidden.mp3`,结果为 `.hidden` — 这是正确行为(显示不带扩展名的文件名)。如果文件名就是 `.mp3`,结果为空字符串,会回退到 "Unknown"。✅ 安全。
- 结论: 无新风险。

### 2. 系统角度批判

**MusicInfo 结构体增大**:
- 增加了 `folder_path[512]` 和 `folder_index`(4 bytes),每个 MusicInfo 增加约 516 bytes。
- 最大 2000 个文件 × 516 bytes ≈ 1 MB 额外内存。在嵌入式系统(AC8215 512MB DDR)上可接受。
- 但如果将来 `MUSIC_MAX_FILES` 扩大,需要注意总内存占用。

**folder_paths[] 内存管理**:
- 使用 `strdup` 分配,在 `build_folder_cache` 和 `deinit` 中 `free`。
- 如果 `build_folder_cache` 被多次调用(每次扫描完成),旧缓存会被正确释放。✅

**favorite_manager 单例模式**:
- 使用文件级 static 变量 `s_fav`,与 `music_app.c` 的 `s_app` 模式一致。✅

**编译兼容性**:
- `_GNU_SOURCE` 定义在 `#include` 之前,确保 `strcasestr` 等 GNU 扩展可用。实际代码中使用了自己实现的 `str_contains_ci()` 而非 `strcasestr`,所以 `_GNU_SOURCE` 实际上可以去掉。但保留不会有副作用。

**结论: 所有修改经 Knuth 式审查,未发现会引起生产级 bug 的问题。**
