# cmus_port 集成指南

## 一、文件部署

将 `cmus_port/` 目录整体复制到 AWTK musicplayer 源码中：

```bash
cp -r cmus_port/ source/packages/application/musicplayer/awtk_app/src/cmus/
```

### 文件清单（26 个文件）

| 文件 | 类型 | 说明 |
|------|------|------|
| `cmus_compat.h` | 新建 | 适配层（替换 cmus 的 options/debug/utils/path/ui_curses） |
| `cmus_compat.c` | 新建 | id3_default_charset 全局变量 |
| `compiler.h` | 搬运 | GCC 属性宏 |
| `xmalloc.h` / `xmalloc.c` | 搬运 | 安全内存分配 |
| `list.h` | 搬运 | Linux 内核双链表 |
| `uchar.h` / `uchar.c` | 搬运+修改 | UTF-8 编解码 |
| `unidecomp.h` | 搬运 | Unicode 分解表 |
| `wcwidth_uchar.h` | 搬运 | wcwidth 适配 |
| `gbuf.h` / `gbuf.c` | 搬运+修改 | 动态字符串缓冲 |
| `convert.h` / `convert.c` | 搬运+修改 | iconv 包装 |
| `file.h` / `file.c` | 搬运+修改 | EINTR 安全 I/O |
| `id3.h` / `id3.c` | 搬运+修改 | ID3v1/v2 解析器 |
| `cue.h` / `cue.c` | 搬运+修改 | CUE sheet 解析器 |
| `cue_utils.h` / `cue_utils.c` | 搬运+修改 | CUE 工具函数 |
| `keyval.h` / `keyval.c` | 搬运+修改 | keyval 存储 |
| `comment.h` / `comment.c` | 搬运+修改 | 元数据高级操作 |
| `Makefile` | 新建 | 独立编译验证用 |
| `test_cmus_port.c` | 新建 | 10 个功能测试 |

## 二、Makefile 集成

在 `musicplayer_awtk.mk` 中添加：

```makefile
# cmus ported modules
CMUS_DIR := $(SRC_DIR)/cmus
CMUS_SRCS := $(wildcard $(CMUS_DIR)/*.c)
CMUS_OBJS := $(CMUS_SRCS:.c=.o)

# 添加 cmus 目录到 include 路径
CFLAGS += -I$(CMUS_DIR)

# 添加到目标的依赖
$(TARGET): $(OBJS) $(CMUS_OBJS)
```

## 三、在 music_app.c 中集成 ID3 解析

### 替换现有 tag 提取

当前 `music_app.c` 里只有极简的 tag 提取。替换方式：

```c
#include "cmus/id3.h"
#include "cmus/cmus_compat.h"

/* 在 scan 线程中调用 */
static int fill_music_info_from_id3(const char *filepath, music_info_t *info)
{
    int fd = open(filepath, O_RDONLY);
    if (fd < 0) return -1;

    struct id3tag id3;
    id3_init(&id3);

    int rc = id3_read_tags(&id3, fd, ID3_V1 | ID3_V2);
    close(fd);

    if (rc != 0) {
        id3_free(&id3);
        return -1;
    }

    /* 提取 tag 到 music_info_t */
    char *val;

    val = id3_get_comment(&id3, ID3_TITLE);
    if (val) { strncpy(info->title, val, sizeof(info->title)-1); free(val); }

    val = id3_get_comment(&id3, ID3_ARTIST);
    if (val) { strncpy(info->artist, val, sizeof(info->artist)-1); free(val); }

    val = id3_get_comment(&id3, ID3_ALBUM);
    if (val) { strncpy(info->album, val, sizeof(info->album)-1); free(val); }

    val = id3_get_comment(&id3, ID3_GENRE);
    if (val) { strncpy(info->genre, val, sizeof(info->genre)-1); free(val); }

    val = id3_get_comment(&id3, ID3_TRACK);
    if (val) {
        info->track_number = atoi(val);
        free(val);
    }

    val = id3_get_comment(&id3, ID3_ALBUMARTIST);
    if (val) { strncpy(info->album_artist, val, sizeof(info->album_artist)-1); free(val); }

    id3_free(&id3);
    return 0;
}
```

## 四、在 music_scanner.c 中集成 CUE sheet 支持

```c
#include "cmus/cue.h"
#include "cmus/cue_utils.h"

/* 在 scan_directory() 中添加 CUE 检测 */
static void scan_cue_file(const char *cue_path, const char *dir,
                          music_list_t *list)
{
    struct cue_sheet *sheet = cue_from_file(cue_path);
    if (!sheet) return;

    for (size_t i = 0; i < sheet->num_tracks; i++) {
        struct cue_track *t = &sheet->tracks[i];
        music_info_t info;
        memset(&info, 0, sizeof(info));

        /* 构建实际音频文件路径 */
        snprintf(info.filepath, sizeof(info.filepath),
                 "%s/%s", dir, t->file);

        /* CUE 元数据 */
        if (t->meta.title)
            strncpy(info.title, t->meta.title, sizeof(info.title)-1);
        if (t->meta.performer)
            strncpy(info.artist, t->meta.performer, sizeof(info.artist)-1);

        /* 继承 sheet 级别元数据 */
        if (sheet->meta.title && !info.album[0])
            strncpy(info.album, sheet->meta.title, sizeof(info.album)-1);
        if (sheet->meta.genre && !info.genre[0])
            strncpy(info.genre, sheet->meta.genre, sizeof(info.genre)-1);

        info.track_number = t->number;
        info.cue_offset_sec = t->offset;
        info.cue_length_sec = t->length; /* -1 表示最后一轨 */

        music_list_add(list, &info);
    }

    cue_free(sheet);
}

/* 在 scan_directory 的文件遍历中 */
if (is_cue(filename)) {
    char cue_path[PATH_MAX];
    snprintf(cue_path, sizeof(cue_path), "%s/%s", dir, filename);
    scan_cue_file(cue_path, dir, list);
    continue; /* 不把 .cue 文件本身加到列表 */
}
```

**注意**：`music_info_t` 需要新增两个字段来支持 CUE 分轨播放：

```c
/* 在 music_app.h 的 music_info_t 中添加 */
double cue_offset_sec;   /* CUE 轨道起始秒数, 0 表示非 CUE */
double cue_length_sec;   /* CUE 轨道时长秒数, -1 表示到文件结尾 */
```

## 五、许可证

在项目根目录创建 `NOTICE` 文件：

```
This software includes code from cmus (C* Music Player)
https://github.com/cmus/cmus

Copyright 2004-2016 Various Authors (Timo Hirvonen et al.)
Licensed under the GNU General Public License v2.0 or later (GPL-2.0+)

The following files in src/cmus/ are derived from cmus:
  id3.c/h, cue.c/h, cue_utils.c/h, comment.c/h, keyval.c/h,
  convert.c/h, uchar.c/h, file.c/h, gbuf.c/h, xmalloc.c/h,
  compiler.h, list.h, unidecomp.h, wcwidth_uchar.h

list.h is derived from the Linux kernel (GPL-2.0)
wcwidth_uchar.h is derived from musl libc (MIT)
```
