/* Pinyin-initial matching + search runner (pure C, no LVGL). */
#include "compat/awtk.h"
#include "music_app.h"
#include "../../../lvgl/lvgl_app/screens/music_pinyin.h"
#include "../../../lvgl/lvgl_app/screens/music_search.h"
#include <stdio.h>
#include <string.h>

/* stand-ins for the two music_app calls music_search_run() makes */
static MusicInfo T[300]; static MusicList LIST; static storage_device_state_t DEV; static music_app_state_t ST;
const music_app_state_t *music_app_get_state(void) { return &ST; }
const storage_device_state_t *music_app_get_device(int i) { return i == 0 ? &DEV : NULL; }

static int g_checks;
#define YES(t, q) do { g_checks++; if (!music_pinyin_match((t), (q))) { printf("FAIL expected match: '%s' ~ '%s' (line %d)\n", t, q, __LINE__); return 1; } } while (0)
#define NO(t, q)  do { g_checks++; if (music_pinyin_match((t), (q)))  { printf("FAIL expected NO match: '%s' ~ '%s' (line %d)\n", t, q, __LINE__); return 1; } } while (0)
#define EQ(a, b)  do { g_checks++; long _a = (long)(a), _b = (long)(b); if (_a != _b) { printf("FAIL %s=%ld != %s=%ld (line %d)\n", #a, _a, #b, _b, __LINE__); return 1; } } while (0)

static void set(int i, const char *title, const char *artist, const char *album, const char *file)
{
    snprintf(T[i].title, sizeof T[i].title, "%s", title); snprintf(T[i].artist, sizeof T[i].artist, "%s", artist);
    snprintf(T[i].album, sizeof T[i].album, "%s", album); snprintf(T[i].filename, sizeof T[i].filename, "%s", file);
}

int main(void)
{
    char buf[64]; const MusicInfo *res[MUSIC_SEARCH_MAX]; bool trunc;

    /* ---- initials ---- */
    YES("周杰伦", "zjl"); YES("周杰伦", "jl"); YES("周杰伦", "z"); YES("周杰伦", "l");
    NO("周杰伦", "zl");               /* not consecutive */
    NO("周杰伦", "zjlx");             /* longer than the text */
    NO("周杰伦", "zhoujielun");       /* full pinyin is not supported */
    YES("晴天", "qt"); YES("七里香", "qlx"); YES("稻香", "dx"); YES("成都", "cd"); YES("夜曲", "yq");
    /* heteronyms: any reading counts */
    YES("音乐", "yy"); YES("音乐", "yl"); YES("乐", "y"); YES("乐", "l"); YES("银行", "yh"); YES("银行", "yx");
    YES("重庆", "zq"); YES("重庆", "cq"); YES("长大", "cd"); YES("长大", "zd");
    /* mixed text, digits, case, punctuation */
    NO("Hello 世界", "hsj"); YES("Hello 世界", "osj"); YES("Hello 世界", "hello"); YES("HELLO", "hello");
    YES("1989 世界巡回", "1989s"); YES("A-B_C", "abc"); YES("Blinding Lights", "blind");
    NO("Blinding Lights", "BLIND");   /* queries must already be normalised (lowercase) */
    NO("周杰伦", "z j");              /* spaces are invalid in a query */
    NO("周杰伦", ""); NO("", "a"); NO("abc", "abcd");
    /* robustness */
    NO("\xff\xfe\xe4\xb8", "a");                           /* malformed UTF-8 must not crash or loop */
    YES("\xe4\xb8\xad" "abc", "abc");                      /* 中abc */
    { char longtext[2000]; memset(longtext, 'a', sizeof longtext - 1); longtext[sizeof longtext - 1] = 0; YES(longtext, "aaaa"); NO(longtext, "b"); }
    { char q[400]; memset(q, 'a', sizeof q - 1); q[sizeof q - 1] = 0; NO("aaa", q); }   /* query longer than the unit limit */
    EQ(music_pinyin_initial_mask(0x5468) != 0, 1);          /* 周 */
    EQ(music_pinyin_initial_mask('a'), 0);                  /* ASCII is not a hanzi */
    EQ(music_pinyin_initial_mask(0x10000), 0);

    /* ---- normalisation ---- */
    EQ(music_search_normalize(" Z j-L ", buf, sizeof buf), 3); EQ(strcmp(buf, "zjl"), 0);
    EQ(music_search_normalize("周杰伦", buf, sizeof buf), 0);
    EQ(music_search_normalize("abcdef", buf, 4), 3);        /* truncates, always NUL-terminated */
    EQ(strcmp(buf, "abc"), 0);

    /* ---- search runner over a device list ---- */
    memset(T, 0, sizeof T);
    for (int i = 0; i < 300; i++) set(i, "Track", "Various", "Mixed", "x.mp3");
    set(0, "晴天", "周杰伦", "叶惠美", "01.mp3");
    set(1, "七里香", "周杰伦", "七里香", "02.mp3");
    set(2, "Blinding Lights", "The Weeknd", "After Hours", "03.mp3");
    set(3, "成都", "赵雷", "无法长大", "04.mp3");
    set(4, "音乐盒", "未知", "", "ylh_demo.mp3");
    LIST.items = T; LIST.count = 300; LIST.capacity = 300; DEV.music_list = &LIST; DEV.mounted = true;
    EQ(music_search_run("zjl", res, MUSIC_SEARCH_MAX, &trunc), 2); EQ(trunc, 0);          /* artist 周杰伦 x2 */
    EQ(res[0] == &T[0] && res[1] == &T[1], 1);
    EQ(music_search_run("  ZJL  ", res, MUSIC_SEARCH_MAX, &trunc), 2);                    /* trim + case */
    EQ(music_search_run("blinding", res, MUSIC_SEARCH_MAX, &trunc), 1);                   /* plain substring */
    EQ(music_search_run("after hours", res, MUSIC_SEARCH_MAX, &trunc), 1);                /* space kept for substring */
    EQ(music_search_run("cd", res, MUSIC_SEARCH_MAX, &trunc), 1);                         /* 成都 */
    EQ(music_search_run("ylh", res, MUSIC_SEARCH_MAX, &trunc), 1);                        /* initials OR filename */
    EQ(music_search_run("晴天", res, MUSIC_SEARCH_MAX, &trunc), 1);                       /* Chinese substring (pasted / future IME) */
    EQ(music_search_run("", res, MUSIC_SEARCH_MAX, &trunc), 0);
    EQ(music_search_run("   ", res, MUSIC_SEARCH_MAX, &trunc), 0);
    EQ(music_search_run("zzzzzz", res, MUSIC_SEARCH_MAX, &trunc), 0);
    EQ(music_search_run("track", res, 10, &trunc), 10); EQ(trunc, 1);                    /* cap + truncated flag */
    EQ(music_search_run("track", res, MUSIC_SEARCH_MAX, &trunc), 200); EQ(trunc, 1);
    DEV.music_list = NULL; EQ(music_search_run("zjl", res, MUSIC_SEARCH_MAX, &trunc), 0); /* no list */
    printf("SEARCH TESTS PASS (%d checks)\n", g_checks);
    return 0;
}
