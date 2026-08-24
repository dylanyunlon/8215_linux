# cmus / MPD 代码搬运可行性分析报告

## 结论摘要

| 项目 | 可搬性 | 推荐 | 理由 |
|------|--------|------|------|
| **cmus** | ✅ 纯C，高度可行 | **优先搬运** | Linux内核风格，零外部依赖，GPL-2.0 |
| **MPD** | ⚠️ 选择性提取 | 作为参考 | C++，需翻译为C |
| **rhythmbox** | ❌ 不推荐 | 跳过 | GLib/GTK/GStreamer/D-Bus全家桶，512MB跑不动 |

---

## 一、cmus 搬运方案

### 搬运模块清单

#### 模块 A：ID3 tag 完整解析器（替换我们的简陋 APIC 提取）

**核心文件：**
| 文件 | 行数 | 功能 |
|------|------|------|
| `id3.c` | 1294 | ID3v1/v2.2/v2.3/v2.4 完整解析器 |
| `id3.h` | 81 | 公共接口：`id3_read_tags()`, `id3_get_comment()` |

**能力增益：**
- ID3v2.2/v2.3/v2.4 全版本支持（我们目前只有 v2.3 部分帧）
- UTF-16 LE/BE ↔ UTF-8 编解码（中日韩歌名乱码修复）
- 148 种 genre 完整映射表
- TXXX 自定义帧（ReplayGain 等）
- COMM 注释帧
- RVA2 相对音量调整
- UFID MusicBrainz Track ID
- ID3v1 回退（向后兼容古老 MP3）

#### 模块 B：CUE sheet 解析器（我们完全没有）

**核心文件：**
| 文件 | 行数 | 功能 |
|------|------|------|
| `cue.c` | 554 | CUE sheet 完整解析器 |
| `cue.h` | 66 | 公共接口：`cue_parse()`, `cue_from_file()`, `cue_free()` |
| `cue_utils.c` | 81 | 工具函数：`is_cue()`, `cue_get_track_nums()` |
| `cue_utils.h` | 30 | 工具接口 |

**能力增益：**
- PERFORMER / TITLE / GENRE / DATE 等元数据
- INDEX 00/01 时间偏移解析（精度 1/75 秒）
- PREGAP / POSTGAP 处理
- REM 扩展字段（GENRE/DATE/COMPILATION/REPLAYGAIN）
- 多 FILE 支持
- UTF-8 BOM 自动跳过

#### 模块 C：keyval 元数据存取（通用基础设施）

**核心文件：**
| 文件 | 行数 | 功能 |
|------|------|------|
| `keyval.c` | 126 | keyval 数组操作：new/dup/free/get/add |
| `keyval.h` | 44 | `struct keyval`, `struct growing_keyvals` |
| `comment.c` | 296 | 高级元数据：compilation 检测、artistsort、日期解析 |
| `comment.h` | 38 | 高级接口 |

**能力增益：**
- 统一的 key=value 元数据存储
- 动态增长的 keyvals（growing_keyvals）
- compilation 智能检测（Various Artists / VA / V/A）
- key 别名映射（album_artist→albumartist, disc→discnumber 等）
- 日期解析为可排序整数 YYYYMMDD

---

### AST 依赖链完整分析

以下是搬运这三个模块所需的**全部**传递性依赖，已按文件级别追踪到底：

```
id3.c
├── id3.h                    (81行, 无外部依赖)
├── xmalloc.h                (99行, inline 函数)
│   └── compiler.h           (95行, GCC 属性宏)
├── convert.c/h              (129/33行, iconv 包装)
│   ├── xmalloc.h            (同上)
│   └── uchar.h              (267行, 含 inline 函数)
├── uchar.c/h                (673/267行, UTF-8 操作)
│   ├── compiler.h           (同上)
│   ├── gbuf.c/h             (160/66行, 动态字符串缓冲)  ← 仅 u_casefold 需要
│   ├── unidecomp.h          (869行, Unicode 分解表)     ← 仅 u_casefold 需要
│   └── wcwidth_uchar.h      (仅 u_char_width 需要)     ← 我们不需要
├── file.c/h                 (185/45行, EINTR 安全 read/write/mmap)
│   └── xmalloc.h            (同上)
├── options.h                (仅需 extern char *id3_default_charset)
├── debug.h                  (仅需 d_print 宏)
└── utils.h                  (仅需 N_ELEMENTS, min_i, str_to_int, is_freeform_true)

cue.c
├── cue.h                    (66行)
│   └── list.h               (348行, Linux 内核双链表)    ← 核心依赖
├── xmalloc.h                (同上)
└── file.c/h                 (同上, 需要 mmap_file)

cue_utils.c
├── cue.h                    (同上)
├── cue_utils.h              (30行)
├── xmalloc.h                (同上)
├── path.h                   (仅需 get_extension)
└── utils.h                  (同上)

comment.c
├── comment.h                (38行)
│   └── keyval.h             (44行)
├── keyval.c                 (126行)
│   ├── debug.h              (同上, 仅 BUG_ON 宏)
│   └── xmalloc.h            (同上)
├── xmalloc.h                (同上)
├── utils.h                  (同上, 需 is_freeform_true, str_to_int)
└── uchar.h                  (同上, 需 u_strcase_equal)
    └── uchar.c              (同上, u_strcase_equal → u_casefold_char → 需 wctype.h)
```

### 传递性依赖去重后的完整文件清单

| 文件 | 行数 | 搬运策略 | 备注 |
|------|------|----------|------|
| `id3.c` | 1294 | **整文件搬运** | 核心 |
| `id3.h` | 81 | **整文件搬运** | 核心 |
| `cue.c` | 554 | **整文件搬运** | 核心 |
| `cue.h` | 66 | **整文件搬运** | 核心 |
| `cue_utils.c` | 81 | **整文件搬运** | 核心 |
| `cue_utils.h` | 30 | **整文件搬运** | 核心 |
| `comment.c` | 296 | **整文件搬运** | 核心 |
| `comment.h` | 38 | **整文件搬运** | 核心 |
| `keyval.c` | 126 | **整文件搬运** | 核心 |
| `keyval.h` | 44 | **整文件搬运** | 核心 |
| `convert.c` | 129 | **整文件搬运** | 依赖 |
| `convert.h` | 33 | **整文件搬运** | 依赖 |
| `uchar.c` | 673 | **整文件搬运** | 依赖（剥离 ui_curses） |
| `uchar.h` | 267 | **整文件搬运** | 依赖 |
| `file.c` | 185 | **整文件搬运** | 依赖 |
| `file.h` | 45 | **整文件搬运** | 依赖 |
| `list.h` | 348 | **整文件搬运** | 依赖（Linux 内核链表） |
| `xmalloc.h` | 99 | **整文件搬运** | 依赖 |
| `xmalloc.c` | 47 | **整文件搬运** | 依赖（malloc_fail 实现） |
| `compiler.h` | 95 | **整文件搬运** | 依赖 |
| `gbuf.c` | 160 | **整文件搬运** | 依赖（u_casefold 需要） |
| `gbuf.h` | 66 | **整文件搬运** | 依赖 |
| `unidecomp.h` | 869 | **整文件搬运** | 依赖（Unicode 分解表） |
| **cmus_compat.h** | ~80 | **新建适配层** | 替换 options.h/debug.h/utils.h/path.h |

**搬运总量：约 5,800 行代码（含 869 行 Unicode 表）**

---

### 适配层设计：cmus_compat.h

需要创建一个适配头文件，替换以下来自 cmus 其他子系统的依赖：

```c
/* cmus_compat.h — 适配层，替换 cmus 的 options.h/debug.h/utils.h/path.h/ui_curses.h */
#ifndef CMUS_COMPAT_H
#define CMUS_COMPAT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* === 来自 options.h === */
/* 默认 ID3v1 字符集，id3.c 的 decode_str() 用于 ISO-8859-1 → UTF-8 转码 */
extern char *id3_default_charset;

/* === 来自 debug.h === */
#ifdef CMUS_DEBUG
  #define d_print(...) fprintf(stderr, "[cmus] " __VA_ARGS__)
#else
  #define d_print(...) do {} while(0)
#endif
#define BUG_ON(a) do { if (a) { fprintf(stderr, "BUG: %s:%d\n", __FILE__, __LINE__); abort(); } } while(0)

/* === 来自 utils.h === */
#define N_ELEMENTS(array) (sizeof(array) / sizeof((array)[0]))

static inline long min_i(long a, long b) { return a < b ? a : b; }

static inline int str_to_int(const char *str, long int *val) {
    char *end;
    *val = strtol(str, &end, 10);
    if (*str == 0 || *end != 0) return -1;
    return 0;
}

static inline int is_freeform_true(const char *c) {
    return c[0] == '1' || c[0] == 'y' || c[0] == 'Y' || c[0] == 't' || c[0] == 'T';
}

/* === 来自 path.h === */
static inline const char *get_extension(const char *filename) {
    const char *ext = filename + strlen(filename) - 1;
    while (ext >= filename && *ext != '/') {
        if (*ext == '.') return ext + 1;
        ext--;
    }
    return NULL;
}

/* === 来自 ui_curses.h === */
/* uchar.c 引用了 using_utf8 和 charset，但只在 to_utf8 中使用 */
/* 嵌入式系统固定 UTF-8，所以硬编码 */
#define using_utf8 1
static const char *charset = "UTF-8";

/* === 来自 config/ === */
/* AC8215 Linux 有 iconv、strdup、strndup */
#define HAVE_ICONV 1
#define HAVE_STRDUP 1
#define HAVE_STRNDUP 1
/* 不使用 cmus 的 config.h 系统 */
/* #define HAVE_CONFIG 1 — 不定义，让 xmalloc.h/convert.c 走 else 分支 */

#endif /* CMUS_COMPAT_H */
```

### uchar.c 搬运时需要的修改

`uchar.c` 原始 `#include "ui_curses.h"` 引用了 `using_utf8` 和 `charset`。
搬运时改为 `#include "cmus_compat.h"`，因为嵌入式环境固定 UTF-8。

同时删除 `#include "wcwidth_uchar.h"` 这个依赖，因为我们不需要
`u_char_width()` 这个终端显示宽度函数（用于 curses UI 对齐的）。
如果编译报错 `u_char_width` 未定义，可以把它 stub 为 `return 1;`。

---

## 二、对应我们 musicplayer 的 Issue 映射

| cmus 模块 | 解决的 Issue | 当前痛点 |
|-----------|-------------|----------|
| id3.c | #57 专辑封面 | 我们的 APIC 提取极简，不处理 UTF-16 |
| id3.c | (新增) ID3v2.4 | 我们只硬编码了 v2.3 的几个帧 ID |
| id3.c | (新增) Genre 解析 | 我们不解析 `(13)` → "Pop" 这种格式 |
| cue.c | (新增) CUE sheet | 完全没有支持，整张 CD 镜像无法分轨播放 |
| comment.c | #54 Album/Artist | 缺少 albumartist/artistsort 规范化 |
| keyval.c | (改进) 元数据管理 | 当前 music_info_t 字段硬编码，不可扩展 |

---

## 三、MPD 选择性提取清单

MPD 是 C++，需逐模块判断能否提取纯逻辑为 C：

| MPD 文件 | 行数 | 可提取性 | 用途 |
|----------|------|----------|------|
| `src/tag/Id3Load.cxx` | ~300 | ⚠️ 参考 | ID3 加载（依赖 id3lib/id3v2lib） |
| `src/tag/ApeTag.cxx` | ~200 | ⚠️ 可提取逻辑 | APE tag 解析 |
| `src/playlist/cue/CueParser.cxx` | ~400 | ⚠️ 参考 | CUE 解析（cmus 的更好搬） |
| `src/db/` 整个目录 | ~5000 | ❌ 太重 | sqlite+C++ OOP |

**建议**：MPD 代码仅作为参考，不直接搬运。cmus 的实现更适合嵌入式（纯 C，小巧）。

---

## 四、搬运步骤计划

### Phase 1：基础设施搬运（~2000 行）

1. `cp` 以下文件到 `awtk_app/src/cmus/` 子目录：
   - `compiler.h`, `xmalloc.h`, `xmalloc.c`
   - `list.h`
   - `gbuf.h`, `gbuf.c`
   - `uchar.h`, `uchar.c`, `unidecomp.h`
   - `convert.h`, `convert.c`
   - `file.h`, `file.c`
   - `keyval.h`, `keyval.c`

2. 新建 `cmus_compat.h`（适配层）

3. 修改 `uchar.c`：
   - `#include "ui_curses.h"` → `#include "cmus_compat.h"`
   - 删除 `#include "wcwidth_uchar.h"`（或 stub）

4. 修改 `convert.c`：
   - 删除 `#ifdef HAVE_CONFIG` / `#include "config/iconv.h"` 分支
   - 直接 `#include <iconv.h>`（AC8215 BSP 的 glibc 有 iconv）

5. 编译验证基础设施

### Phase 2：ID3 解析器搬运（~1400 行）

1. `cp id3.h id3.c` 到 `awtk_app/src/cmus/`
2. 修改 `id3.c`：
   - `#include "options.h"` → `#include "cmus_compat.h"`
   - `#include "debug.h"` → 删除（cmus_compat.h 已提供 d_print）
   - `#include "utils.h"` → 删除（cmus_compat.h 已提供 N_ELEMENTS/min_i）
3. 在 `cmus_compat.h` 或新建 `cmus_globals.c` 中定义：
   ```c
   char *id3_default_charset = "ISO-8859-1";
   ```
4. 编写 `music_app.c` 的集成代码：调用 `id3_read_tags()` 替换现有 tag 提取

### Phase 3：CUE sheet 搬运（~730 行）

1. `cp cue.h cue.c cue_utils.h cue_utils.c` 到 `awtk_app/src/cmus/`
2. `cue_utils.c` 修改：
   - `#include "path.h"` → `#include "cmus_compat.h"`（get_extension 已内联）
   - `#include "utils.h"` → 删除
3. 编写 `music_scanner.c` 的集成：扫描时检测 `.cue` 文件并展开轨道

### Phase 4：comment/keyval 搬运（~500 行）

1. `cp comment.h comment.c` 到 `awtk_app/src/cmus/`
2. 修改 `comment.c`：
   - `#include "utils.h"` → `#include "cmus_compat.h"`
3. 编写集成层，将 cmus 的 keyval 元数据桥接到我们的 `music_info_t`

### Phase 5：Makefile 集成

在 `musicplayer_awtk.mk` 中添加 cmus 子目录的编译规则：
```makefile
CMUS_SRCS := $(wildcard $(SRC_DIR)/cmus/*.c)
CMUS_OBJS := $(CMUS_SRCS:.c=.o)
$(CMUS_OBJS): CFLAGS += -I$(SRC_DIR)/cmus
```

---

## 五、许可证合规

cmus 使用 **GPL-2.0-or-later**。我们的 AWTK musicplayer 是车载嵌入式固件：
- 如果最终产品以 GPL 兼容方式发布源代码 → 无问题
- 如果需要闭源 → 需要法务确认 GPL 在固件中的适用范围
- cmus 的 `list.h` 来自 Linux 内核，同为 GPL-2.0

**建议**：将搬运的 cmus 代码放在独立子目录 `cmus/`，保留原始版权头，
在项目根目录 NOTICE 文件中声明使用了 cmus 代码及其许可证。
