# main_lvgl 分支

本分支 = `main` + 一个新目录 **`hcn_music_lvgl/`**，是 AWTK 版本地音乐（`source/packages/application/musicplayer`）的 LVGL 9.3 版：

- 业务逻辑（`music_app.c` 等）**原样复用**，只重写界面；AWTK 的三个原语由 `adapters/music/include/compat/awtk.h` 在主循环上重新实现
- 界面 `ui_Music.c`（1024×600，列表/收藏/文件夹/专辑/歌手/歌词），音频焦点 `music_focus.c`
- 宿主机一键测试 `tools/music_ui_harness/run_tests.sh`（界面 33 项、焦点 224 项断言，ASan/UBSan/TSan/LSan）
- 说明见 `hcn_music_lvgl/README_MUSIC.md`；构建环境备忘见 `hcn_music_lvgl/docs/BUILD_ENV_NOTES.md`

**刻意没有放进来的**（公开仓库，含内部源码）：HCN `application` / `middleware` / `vehicle_services` 源码、DA 树、Autochips 的 LVGL 魔改源码、
以及含上下文行的 `integration.patch`（只放了 `docs/INTEGRATION_ADDED_LINES.md`，列出我新增的行）。
`tools/enable_music_in_hcn_application.sh` 需要的 `integration.patch` 在 `hcn_music_lvgl.tar.gz` 里。

本分支里的测试对着本仓库的 `source/packages/application/musicplayer` 能跑 UI 场景；焦点测试和 `ui_Music.c` 的编译还需要
`BUILD_ENV_NOTES.md` 末尾列的那几个 HCN 文件（放在 `hcn_music_lvgl/lvgl/lvgl_app/frameworks/`，不要提交）。
