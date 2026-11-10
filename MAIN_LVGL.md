# main_lvgl 分支

本分支 = `main` + 一个新目录 **`hcn_music_lvgl/`**，是 AWTK 版本地音乐（`source/packages/application/musicplayer`）的 LVGL 9.3 版：

- 业务逻辑（`music_app.c` 等）**原样复用**，只重写界面；AWTK 的三个原语由 `adapters/music/include/compat/awtk.h` 在主循环上重新实现
- 界面 `ui_Music.c`（1024×600，列表/收藏/文件夹/专辑/歌手/歌词），音频焦点 `music_focus.c`
- 宿主机一键测试 `tools/music_ui_harness/run_tests.sh`（界面 33 项、焦点 224 项断言，ASan/UBSan/TSan/LSan）
- 说明见 `hcn_music_lvgl/README_MUSIC.md`；构建环境备忘见 `hcn_music_lvgl/docs/BUILD_ENV_NOTES.md`

**刻意没有放进来的**（公开仓库，含内部源码）：HCN `application` / `middleware` / `vehicle_services` 源码、DA 树、Autochips 的 LVGL 魔改源码。
`hcn_music_lvgl/integration.patch` 是**零上下文**补丁（只含新增行，不带周围的 HCN 代码；唯一被改的原有行是 CMake 里的一行 `foreach`），
`patch -p1` 照常应用。

**直接下载整包：** `hcn_music_lvgl/dist/hcn_music_lvgl.tar.gz`（内容与 `hcn_music_lvgl/` 目录一致）。

本分支里的测试对着本仓库的 `source/packages/application/musicplayer` 能跑 UI 场景；焦点测试和 `ui_Music.c` 的编译还需要
`BUILD_ENV_NOTES.md` 末尾列的那几个 HCN 文件（放在 `hcn_music_lvgl/lvgl/lvgl_app/frameworks/`，不要提交）。
