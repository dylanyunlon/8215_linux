/* Test double for music_app.h: same signatures, canned data, manual clock. Harness only. */
#include "music_app.h"
#include "favorite_manager.h"
#include "displayfocusmanager.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define NTRK 48
static MusicInfo T[NTRK];
static music_app_state_t ST;
static storage_device_state_t DEV;
static MusicList LIST;
#define NTRK2 12
static MusicInfo T2[NTRK2]; static MusicList LIST2; static storage_device_state_t DEV2; static bool have_dev2;
static int g_seek_fwd, g_seek_back, g_next, g_prev, g_last_play = -1;
static uint8_t *COVERS[6]; static int COVER_N[6];
static bool FAV[NTRK];
static bool have_device = true;
static lrc_data_t LRC;
static lrc_line_t LRCL[24];
static uint8_t *ART; static int ART_N;
static bool art_on = true, lrc_on = true;
static void set_state(PlayerState s); static void fpost(music_app_event_t e, int p);
static const MusicInfo *SUB[NTRK]; static int SUBN; static bool sub_active;
static music_group_t ALBUMS[NTRK], ARTISTS[NTRK]; static int NALB, NART;
static const char *FOLDERS[] = {"/mnt/usb0/周杰伦", "/mnt/usb0/Western Hits", "/mnt/usb0/Live 现场"};
static int folder_of(int i) { return i >= 32 ? 2 : ((i % 16) == 5 || (i % 16) == 8 || (i % 16) == 12 || (i % 16) == 15) ? 1 : 0; }
static void group_add(music_group_t *arr, int *n, const char *key, const MusicInfo *mi) {
    int g; for (g = 0; g < *n; g++) if (!strcmp(arr[g].key, key)) break;
    if (g == *n) { memset(&arr[g], 0, sizeof arr[g]); snprintf(arr[g].key, sizeof arr[g].key, "%s", key); arr[g].items.items = calloc(NTRK, sizeof(const MusicInfo *)); arr[g].items.capacity = NTRK; (*n)++; }
    arr[g].items.items[arr[g].items.count++] = mi;
}

static const char *TIT[] = {"晴天","七里香","稻香","夜曲","后来","Blinding Lights","起风了","平凡之路","Shape of You","光年之外","成都","小幸运","Let It Be","海阔天空","突然的自我","Hotel California"};
static const char *ART_[] = {"周杰伦","周杰伦","周杰伦","周杰伦","刘若英","The Weeknd","买辣椒也用券","朴树","Ed Sheeran","G.E.M. 邓紫棋","赵雷","田馥甄","The Beatles","Beyond","伍佰 & China Blue","Eagles"};
static const char *ALB[] = {"叶惠美","七里香","魔杰座","十一月的萧邦","我等你","After Hours","起风了","猎户星座","÷","光年之外","无法长大","小幸运","Let It Be","海阔天空","Space Man","Hotel California"};

static void free_groups(void) {
    for (int g = 0; g < NALB; g++) free(ALBUMS[g].items.items);
    for (int g = 0; g < NART; g++) free(ARTISTS[g].items.items);
    NALB = NART = 0; free(ART); ART = NULL; ART_N = 0;
    for (int k = 0; k < 6; k++) { free(COVERS[k]); COVERS[k] = NULL; COVER_N[k] = 0; }
}
void fake_shutdown(void) { free_groups(); }

void fake_init(void)
{
    free_groups();
    memset(&ST, 0, sizeof ST); memset(&DEV, 0, sizeof DEV);
    for (int i = 0; i < NTRK; i++) {
        MusicInfo *m = &T[i]; memset(m, 0, sizeof *m);
        m->uid = i + 1; snprintf(m->title, sizeof m->title, "%s%s", TIT[i % 16], i >= 16 ? (i >= 32 ? " (Live)" : " (Remix)") : "");
        snprintf(m->artist, sizeof m->artist, "%s", ART_[i % 16]); snprintf(m->album, sizeof m->album, "%s", ALB[i % 16]);
        m->duration_ms = (170 + (i * 37) % 120) * 1000; snprintf(m->filepath, sizeof m->filepath, "/mnt/usb0/m%02d.mp3", i);
        snprintf(m->filename, sizeof m->filename, "m%02d.mp3", i);
    }
    /* a track with a very long title and one with no tags, to exercise truncation / fallbacks */
    snprintf(T[5].title, sizeof T[5].title, "这是一首标题非常非常非常长的歌曲 The Quick Brown Fox Jumps Over The Lazy Dog (Extended Mix)");
    T[7].title[0] = 0; T[7].artist[0] = 0; T[7].album[0] = 0;
    DEV.mounted = true; DEV.scan_done = true; DEV.type = STORAGE_TYPE_USB; strcpy(DEV.mount_point, "/mnt/usb0");
    LIST.items = T; LIST.count = NTRK; LIST.capacity = NTRK; DEV.music_list = &LIST;
    ST.current_device_idx = 0; ST.play_mode = PLAY_MODE_REPEAT_ALL; ST.playlist_type = PLAYLIST_TYPE_DEVICE;
    ST.current_info = &T[3]; ST.current_position_ms = 83000; ST.current_duration_ms = T[3].duration_ms; ST.player_state = PLAYER_STATE_PLAYING;
    for (int i = 0; i < NTRK2; i++) {
        MusicInfo *m = &T2[i]; memset(m, 0, sizeof *m); m->uid = 1000 + i;
        snprintf(m->title, sizeof m->title, "SD Track %02d", i + 1); snprintf(m->artist, sizeof m->artist, "SD Artist"); snprintf(m->album, sizeof m->album, "SD Album");
        m->duration_ms = (150 + i * 11) * 1000; snprintf(m->filepath, sizeof m->filepath, "/mnt/sd0/s%02d.mp3", i); snprintf(m->filename, sizeof m->filename, "s%02d.mp3", i);
    }
    memset(&DEV2, 0, sizeof DEV2); DEV2.mounted = true; DEV2.scan_done = true; DEV2.type = STORAGE_TYPE_SD; strcpy(DEV2.mount_point, "/mnt/sd0");
    LIST2.items = T2; LIST2.count = NTRK2; LIST2.capacity = NTRK2; DEV2.music_list = &LIST2; have_dev2 = false;
    g_seek_fwd = g_seek_back = g_next = g_prev = 0; g_last_play = -1;
    for (int k = 0; k < 6; k++) { char fn[32]; snprintf(fn, sizeof fn, "cover%d.jpg", k); FILE *cf = fopen(fn, "rb");
        if (cf) { fseek(cf, 0, SEEK_END); COVER_N[k] = (int)ftell(cf); fseek(cf, 0, SEEK_SET); COVERS[k] = malloc(COVER_N[k]); if (fread(COVERS[k], 1, COVER_N[k], cf) != (size_t)COVER_N[k]) COVER_N[k] = 0; fclose(cf); } }
    FAV[3] = FAV[0] = FAV[9] = FAV[12] = true; have_device = true; art_on = lrc_on = true;
    sub_active = false; NALB = NART = 0; ST.playlist_type = PLAYLIST_TYPE_DEVICE;
    for (int i = 0; i < NTRK; i++) { group_add(ALBUMS, &NALB, T[i].album[0] ? T[i].album : "未知专辑", &T[i]); group_add(ARTISTS, &NART, T[i].artist[0] ? T[i].artist : "未知艺术家", &T[i]); }
    static const char *L[] = {"作词：方文山  作曲：周杰伦","一定在夜里哭","嘴巴没有说","你的泪水在手心里流","我们曾经快乐过","就像一场梦","为了你我变成了另一个我","时光不能倒流","爱到最后只剩下沉默","(间奏)","如果还能再爱一次","我会紧紧地抱住你","不再让你离开","夜曲就这样慢慢结束"};
    LRC.lines = LRCL; LRC.count = 14; LRC.capacity = 24;
    for (int i = 0; i < 14; i++) { LRCL[i].time_ms = 20000 + i * 9000; snprintf(LRCL[i].text, sizeof LRCL[i].text, "%s", L[i]); }
    FILE *f = fopen("cover.jpg", "rb"); if (f) { fseek(f, 0, SEEK_END); ART_N = (int)ftell(f); fseek(f, 0, SEEK_SET); ART = malloc(ART_N); if (fread(ART, 1, ART_N, f) != (size_t)ART_N) ART_N = 0; fclose(f); }
}
void fake_set_device(bool mounted, bool scanning, bool done) { have_device = mounted; DEV.mounted = mounted; DEV.scanning = scanning; DEV.scan_done = done; if (!done) { LIST.count = scanning ? 0 : 0; } else LIST.count = NTRK; }
void fake_set_count(int n) { LIST.count = n; }
void fake_set_current(int idx, int pos_ms) { ST.current_info = idx >= 0 ? &T[idx] : NULL; ST.current_position_ms = pos_ms; ST.current_duration_ms = idx >= 0 ? T[idx].duration_ms : 0; }
void fake_set_state(PlayerState s) { ST.player_state = s; }
void fake_set_mode(PlayMode m) { ST.play_mode = m; }
void fake_set_art(bool on) { art_on = on; }
void fake_set_lyrics(bool on) { lrc_on = on; }
void fake_long_lyric(bool on) { snprintf(LRCL[8].text, sizeof LRCL[8].text, "%s", on ? "这是一句特别特别长的歌词，用来测试当前行是否会横向滚动而不是被截断成省略号 It keeps going on and on" : "爱到最后只剩下沉默"); }
void fake_set_second_device(bool mounted) { have_dev2 = mounted; }
int fake_seek_fwd(void) { return g_seek_fwd; } int fake_seek_back(void) { return g_seek_back; }
int fake_next_calls(void) { return g_next; } int fake_prev_calls(void) { return g_prev; } int fake_last_play(void) { return g_last_play; }
void fake_reset_counters(void) { g_seek_fwd = g_seek_back = g_next = g_prev = 0; g_last_play = -1; }
void fake_clear_favs(void) { memset(FAV, 0, sizeof FAV); }
void fake_set_playlist_type(playlist_type_t t) { ST.playlist_type = t; if (t == PLAYLIST_TYPE_DEVICE) sub_active = false; }

const music_app_state_t *music_app_get_state(void) { return &ST; }
const storage_device_state_t *music_app_get_device(int i) { return (i == 0 && have_device) ? &DEV : (i == 1 && have_dev2) ? &DEV2 : NULL; }
int music_app_get_device_count(void) { return have_dev2 ? 2 : (have_device ? 1 : 0); }
static MusicList *cur_list(void) { return ST.current_device_idx == 1 ? &LIST2 : &LIST; }
int music_app_get_playlist_count(void) { return (ST.current_device_idx == 0 && !have_device) ? 0 : sub_active ? SUBN : cur_list()->count; }
const MusicInfo *music_app_get_track_info(int i) { if ((ST.current_device_idx == 0 && !have_device) || i < 0) return NULL; if (sub_active) return i < SUBN ? SUB[i] : NULL; return i < cur_list()->count ? &cur_list()->items[i] : NULL; }
int music_app_get_current_index(void) { int n = music_app_get_playlist_count(); for (int i = 0; i < n; i++) if (music_app_get_track_info(i) == ST.current_info) return i; return -1; }
void music_app_restore_full_playlist(void) { sub_active = false; ST.playlist_type = PLAYLIST_TYPE_DEVICE; }
void music_app_get_folder_list(const char ***o, int *n) { *o = (const char **)FOLDERS; *n = have_device ? 3 : 0; }
void music_app_play_folder(const char *path) { SUBN = 0; for (int i = 0; i < NTRK; i++) if (!strcmp(FOLDERS[folder_of(i)], path)) SUB[SUBN++] = &T[i]; sub_active = true; ST.playlist_type = PLAYLIST_TYPE_FOLDER; ST.current_info = SUB[0]; ST.current_position_ms = 0; printf("[fake] play_folder %s (%d)\n", path, SUBN); }
void music_app_get_album_list(const music_group_t **g, int *n) { *g = ALBUMS; *n = have_device ? NALB : 0; }
void music_app_get_artist_list(const music_group_t **g, int *n) { *g = ARTISTS; *n = have_device ? NART : 0; }
void music_app_play_group(const music_group_t *g, int idx) { bool art = (g >= ARTISTS && g < ARTISTS + NTRK); SUBN = g->items.count; for (int i = 0; i < SUBN; i++) SUB[i] = g->items.items[i]; sub_active = true; ST.playlist_type = art ? PLAYLIST_TYPE_ARTIST : PLAYLIST_TYPE_ALBUM; ST.current_info = SUB[idx]; ST.current_position_ms = 0; printf("[fake] play_group %s #%d\n", g->key, idx); }
int music_app_safe_play(int i) { ST.current_info = music_app_get_track_info(i); if (!ST.current_info) return -1; g_last_play = i; ST.current_position_ms = 0; ST.current_duration_ms = ST.current_info->duration_ms; ST.player_state = PLAYER_STATE_PLAYING; set_state(PLAYER_STATE_PLAYING); fpost(APP_EVENT_TRACK_CHANGED, i); return 0; }
int music_app_safe_next(void) { g_next++; return 0; } int music_app_safe_prev(void) { g_prev++; return 0; }
void music_app_seek_forward(int ms) { g_seek_fwd++; ST.current_position_ms += ms; fpost(APP_EVENT_POSITION_CHANGED, ST.current_position_ms); }
void music_app_seek_backward(int ms) { g_seek_back++; ST.current_position_ms = ST.current_position_ms > ms ? ST.current_position_ms - ms : 0; fpost(APP_EVENT_POSITION_CHANGED, ST.current_position_ms); }
void music_app_switch_device(int idx) { ST.current_device_idx = idx; sub_active = false; ST.playlist_type = PLAYLIST_TYPE_DEVICE; ST.current_info = NULL; ST.current_position_ms = 0; ST.current_duration_ms = 0; fpost(APP_EVENT_PLAYLIST_CHANGED, idx); }
void music_app_cycle_play_mode(void) { ST.play_mode = (PlayMode)((ST.play_mode + 1) & 3); }
static int usb_index(const MusicInfo *m) { return (m >= T && m < T + NTRK) ? (int)(m - T) : -1; }
bool music_app_is_favorite(void) { int i = usb_index(ST.current_info); return i >= 0 && FAV[i]; }
bool music_app_toggle_favorite(void) { int i = usb_index(ST.current_info); if (i < 0) return false; FAV[i] = !FAV[i]; return FAV[i]; }
/* --- emulate the real asynchronous engine: state changes are announced through idle_queue (-> GUI thread) --- */
static music_app_ui_callback_t g_cb;
typedef struct { music_app_event_t e; int p; } fev_t;
static ret_t fdispatch(const idle_info_t *i) { fev_t *d = (fev_t *)i->ctx; if (g_cb) g_cb(d->e, &d->p); free(d); return RET_REMOVE; }
static void fpost(music_app_event_t e, int p) { fev_t *d = malloc(sizeof *d); d->e = e; d->p = p; idle_queue(fdispatch, d); }
int music_app_init(music_app_ui_callback_t cb) { g_cb = cb; return 0; }
void music_app_deinit(void) { g_cb = NULL; }
void music_app_save_state(void) {}
static void set_state(PlayerState s) { if (ST.player_state != s) { ST.player_state = s; fpost(APP_EVENT_STATE_CHANGED, (int)s); } }
void music_app_pause(void) { if (ST.player_state == PLAYER_STATE_PLAYING) set_state(PLAYER_STATE_PAUSED); }
void music_app_resume(void) { set_state(PLAYER_STATE_PLAYING); }
void music_app_stop(void) { set_state(PLAYER_STATE_STOPPED); }
void music_app_toggle_play_pause(void) { if (ST.player_state == PLAYER_STATE_PLAYING) music_app_pause(); else music_app_resume(); }
void music_app_seek(int ms) { ST.current_position_ms = ms; printf("[fake] seek %d\n", ms); }
static MusicInfo FAVL[NTRK];
const MusicInfo *music_app_get_favorite_list(int *n) { int c = 0; for (int i = 0; i < NTRK; i++) if (FAV[i]) FAVL[c++] = T[i]; if (n) *n = c; return FAVL; }
const lrc_data_t *music_app_get_lyrics(void) { return (lrc_on && ST.current_info) ? &LRC : NULL; }
int music_app_get_lyrics_line(int t) { int r = -1; for (int i = 0; i < LRC.count; i++) if (LRCL[i].time_ms <= t) r = i; return r; }
static int cover_pick(const char *path) { unsigned h = 5381; for (; *path; path++) h = h * 33 + (unsigned char)*path; return (int)(h % 6); }
int music_app_get_album_art(const uint8_t **d, int *s) {
    int k;
    if (!art_on || !ST.current_info) return -1;
    k = cover_pick(ST.current_info->filepath);
    if (COVERS[k]) { *d = COVERS[k]; *s = COVER_N[k]; return 0; }
    if (!ART) return -1;
    *d = ART; *s = ART_N; return 0;
}
/* async-safe (called from the art worker thread): reads only immutable cover buffers */
int music_app_extract_art(const char *path, uint8_t **out, int *n) {
    int k = cover_pick(path);
    usleep(1000);
    if (strstr(path, "NOART") || !COVERS[k]) return -1;
    *out = (uint8_t *)malloc(COVER_N[k]); memcpy(*out, COVERS[k], COVER_N[k]); *n = COVER_N[k]; return 0;
}
bool favorite_contains(const char *path) { for (int i = 0; i < NTRK; i++) if (FAV[i] && !strcmp(T[i].filepath, path)) return true; return false; }
int displayfocusmanager_go_home(void) { printf("[fake] go_home\n"); return 0; }
