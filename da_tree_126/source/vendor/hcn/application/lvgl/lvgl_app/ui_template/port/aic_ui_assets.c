/**
 * @file    aic_ui_assets.c
 * @brief   资源根目录的**运行时**解析（图片 / 字体的路径前缀）
 *
 * 为什么需要它
 * ------------
 * AiUIBuilder 生成代码用 LVGL_IMAGE_PATH(x) 拼资源路径。若只在编译期拼死一个绝对路径前缀，
 * 把可执行文件和 assets/ 拷到别的目录就整屏找不到图：板级是 LV_USE_FS_POSIX +
 * LV_FS_POSIX_PATH ""，按绝对路径 open()，找不到直接返回错误，不会回退到当前目录。
 *
 * 解析顺序（只做一次，谁先命中用谁）
 * --------------------------------
 *   1) 环境变量 HCN_APPLICATION_ASSETS          —— 现场 / 调试显式覆盖，最高优先级
 *   2) <可执行文件目录>/assets          —— "拷到哪都能跑"（Linux 用 /proc/self/exe）
 *   3) <当前工作目录>/assets            —— 兼容从当前目录启动的场景
 *   4) 编译期默认值 LVGL_DIR            —— rootfs 的 /usr/share/hcn_application/assets，行为与改前一致
 *
 * "命中"怎么判定：<dir>/image/start_animation/000.png 能用普通 fopen 打开。
 *   不用 stat / access / GetFileAttributes 是为了省掉两端平台差异；
 *   这个文件是模板首页启动动画的首帧，回退 UI 不含该资源，可避免两种资源布局误判。
 *
 * 两个容易踩的点，这里都处理了
 * ----------------------------
 *   1) 盘符：lv_fs 只认 "<letter>:<path>"，letter 由板级 lv_conf.h 决定（两端都是 'A'）。
 *      所以候选目录统一拼 HCN_APPLICATION_FS_LETTER（不是 'A' 的情况 aic_ui.h 里已经编译期报错）。
 *      Windows 的 "E:/xxx" 也照拼成 "A:E:/xxx" —— 同一套逻辑，不做平台分支。
 *
 *   2) 字符串生命周期：**LVGL 的 style 属性不复制字符串**
 *      （src/misc/lv_style.c 的 lv_style_set_prop 直接存指针；只有
 *        src/widgets/image/lv_image.c:220 的 lv_image_set_src 会 lv_strdup）。
 *      而 ui/ 里有大量 lv_obj_set_style_bg_image_src(..., LVGL_IMAGE_PATH(...), ...)，
 *      存进去的指针要长期有效。所以 ui_asset_path_site() 按**调用点**(__FILE__,__LINE__)
 *      各分配一次、永不释放：47 个调用点 ≈ 47 条小字符串，换来"被 LVGL 存多久都不悬空"。
 */

#include "aic_ui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__linux__)
#  include <unistd.h>        /* readlink */
#elif defined(_WIN32)
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>       /* GetModuleFileNameA */
#endif

#define HCN_APPLICATION_ASSET_MAX_PATH 512
#define HCN_APPLICATION_ASSET_SLOT_MAX 128
#define HCN_APPLICATION_ASSET_PROBE    "start_animation/000.png"   /* 探测锚点：<dir>/image/<probe> */

typedef struct {
    const char *file;      /* __FILE__ 字面量地址，天然稳定 */
    int         line;
    const char *full;      /* 拼好的完整路径，永不过期 */
} ui_asset_slot_t;

static ui_asset_slot_t s_slots[HCN_APPLICATION_ASSET_SLOT_MAX];
static int             s_slot_cnt;
static char            s_root[HCN_APPLICATION_ASSET_MAX_PATH];   /* "A:<dir>"，无尾斜杠 */
static int             s_root_ready;

/** 去掉尾部斜杠，但保住 "A:" 本身 */
static void trim_tail_slash(char *s)
{
    size_t n = strlen(s);
    while (n > 2 && (s[n - 1] == '/' || s[n - 1] == '\\'))
        s[--n] = '\0';
}

/** <dir>/image/<probe> 能打开 -> 这个目录就是资源根 */
static int asset_dir_ok(const char *dir)
{
    char   probe[HCN_APPLICATION_ASSET_MAX_PATH + 64];
    FILE  *fp;

    snprintf(probe, sizeof(probe), "%s/image/%s", dir, HCN_APPLICATION_ASSET_PROBE);
    fp = fopen(probe, "rb");
    if (fp == NULL)
        return 0;
    fclose(fp);
    return 1;
}

/** 由 native 目录得到最终 root："A:<dir>" */
static void root_from_dir(const char *dir)
{
    snprintf(s_root, sizeof(s_root), "%c:%s", HCN_APPLICATION_FS_LETTER, dir);
    trim_tail_slash(s_root);
}

/** 1) 环境变量：显式给的路径直接用（哪怕探测不到，也尊重使用者的显式指定，只告警） */
static int root_from_env(void)
{
    const char *v      = getenv("HCN_APPLICATION_ASSETS");
    const char *native;

    if (v == NULL || v[0] == '\0')
        return 0;

    if (v[0] == HCN_APPLICATION_FS_LETTER && v[1] == ':') {
        snprintf(s_root, sizeof(s_root), "%s", v);
        trim_tail_slash(s_root);
        native = v + 2;                 /* "A:/x" -> "/x"，探测要用 native 路径 */
    }
    else {
        root_from_dir(v);
        native = v;
    }

    if (!asset_dir_ok(native)) {
        fprintf(stderr, "[hcn_application] WARN: HCN_APPLICATION_ASSETS=%s 下没看到 image/%s，仍按它执行\n",
                v, HCN_APPLICATION_ASSET_PROBE);
    }
    return 1;
}

/** 2) 可执行文件所在目录 + /assets */
static int root_from_exedir(void)
{
    char  exe[HCN_APPLICATION_ASSET_MAX_PATH];
    char  cand[HCN_APPLICATION_ASSET_MAX_PATH + 16];
    char *slash;

#if defined(__linux__)
    {
        ssize_t n = readlink("/proc/self/exe", exe, sizeof(exe) - 1);
        if (n <= 0)
            return 0;
        exe[n] = '\0';
    }
#elif defined(_WIN32)
    {
        DWORD n = GetModuleFileNameA(NULL, exe, (DWORD)(sizeof(exe) - 1));
        if (n == 0)
            return 0;
        exe[n] = '\0';
    }
#else
    return 0;
#endif

    slash = strrchr(exe, '/');
#if defined(_WIN32)
    {
        char *bs = strrchr(exe, '\\');
        if (bs != NULL && (slash == NULL || bs > slash))
            slash = bs;
    }
#endif
    if (slash == NULL)
        return 0;
    *slash = '\0';                                  /* exe 目录 */

    snprintf(cand, sizeof(cand), "%s/assets", exe);
    if (!asset_dir_ok(cand))
        return 0;

    root_from_dir(cand);
    return 1;
}

/** 3) 当前工作目录 + /assets */
static int root_from_cwd(void)
{
    static const char *cands[] = { "./assets", "assets" };
    size_t i;

    for (i = 0; i < sizeof(cands) / sizeof(cands[0]); i++) {
        if (asset_dir_ok(cands[i])) {
            root_from_dir(cands[i]);
            return 1;
        }
    }
    return 0;
}

/** 4) 编译期默认值（CMake 注入的 LVGL_DIR，已带盘符与尾斜杠） */
static void root_from_default(void)
{
    snprintf(s_root, sizeof(s_root), "%s", LVGL_DIR);
    trim_tail_slash(s_root);
}

const char *ui_asset_root(void)
{
    const char *how;

    if (s_root_ready)
        return s_root;
    s_root_ready = 1;                    /* 先置位，避免任何递归可能 */

    if (root_from_env()) {
        how = "env HCN_APPLICATION_ASSETS";
    }
    else if (root_from_exedir()) {
        how = "exe dir";
    }
    else if (root_from_cwd()) {
        how = "cwd";
    }
    else {
        root_from_default();
        how = "compile-time default";
    }

    /* 用 fprintf 而不是 LV_LOG_USER：LV_LOG_LEVEL 一般是 WARN，USER 级别的日志看不到。
     * 这行必须始终可见。 */
    fprintf(stderr, "[hcn_application] assets root = %s  (via %s)\n", s_root, how);
    return s_root;
}

const char *ui_asset_path_site(const char *rel, const char *file, int line)
{
    const char *root;
    char       *buf;
    size_t      n;
    int         i;

    /* 同一调用点必然同一个字符串：直接复用，指针永久有效 */
    for (i = 0; i < s_slot_cnt; i++) {
        if (s_slots[i].file == file && s_slots[i].line == line)
            return s_slots[i].full;
    }

    root = ui_asset_root();
    n    = strlen(root) + strlen(rel) + 2;

    buf = (char *)malloc(n);
    if (buf == NULL)
        return root;                     /* 兜底：宁可指到根目录也不要返回 NULL */

    snprintf(buf, n, "%s/%s", root, rel);

    if (s_slot_cnt < HCN_APPLICATION_ASSET_SLOT_MAX) {
        s_slots[s_slot_cnt].file = file;
        s_slots[s_slot_cnt].line = line;
        s_slots[s_slot_cnt].full = buf;
        s_slot_cnt++;
    }
    /* 槽位用尽：buf 不登记但依然有效（只多占一点常驻内存，绝不会悬空） */

    return buf;
}
