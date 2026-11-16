/**
 * @file music_search.c
 */
#include "music_search.h"
#include "music_pinyin.h"
#include "music_app.h"

#include <string.h>

int music_search_normalize(const char *in, char *out, size_t out_size)
{
    size_t n = 0;
    if (out_size == 0) return 0;
    for (; in && *in && n + 1 < out_size; in++) {
        unsigned char c = (unsigned char)*in;
        if (c >= 'A' && c <= 'Z') c = (unsigned char)(c - 'A' + 'a');
        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) out[n++] = (char)c;
    }
    out[n] = 0;
    return (int)n;
}

/* ASCII case-insensitive substring; bytes >= 0x80 compare exactly (UTF-8 safe because lead/continuation bytes never collide) */
static bool ci_contains(const char *hay, const char *needle)
{
    size_t nl = strlen(needle), i, j;
    if (nl == 0) return false;
    for (i = 0; hay[i]; i++) {
        for (j = 0; j < nl; j++) {
            unsigned char a = (unsigned char)hay[i + j], b = (unsigned char)needle[j];
            if (a == 0) return false;
            if (a >= 'A' && a <= 'Z') a = (unsigned char)(a - 'A' + 'a');
            if (b >= 'A' && b <= 'Z') b = (unsigned char)(b - 'A' + 'a');
            if (a != b) break;
        }
        if (j == nl) return true;
    }
    return false;
}

static bool field_matches(const char *field, const char *raw, const char *query)
{
    if (field == NULL || field[0] == 0) return false;
    if (ci_contains(field, raw)) return true;
    return query[0] != 0 && music_pinyin_match(field, query);
}

bool music_search_track_matches(const MusicInfo *mi, const char *raw, const char *query)
{
    return field_matches(mi->title, raw, query) || field_matches(mi->artist, raw, query) ||
           field_matches(mi->album, raw, query) || field_matches(mi->filename, raw, query);
}

int music_search_run(const char *raw_in, const MusicInfo **out, int max, bool *truncated)
{
    char raw[128], query[128];
    const music_app_state_t *st = music_app_get_state();
    const storage_device_state_t *dev = music_app_get_device(st->current_device_idx);
    const MusicList *list;
    size_t a = 0, b;
    int n = 0, i;

    if (truncated) *truncated = false;
    if (raw_in == NULL || dev == NULL || dev->music_list == NULL || max <= 0) return 0;
    list = dev->music_list;

    while (raw_in[a] == ' ') a++;                    /* trim */
    b = strlen(raw_in);
    while (b > a && raw_in[b - 1] == ' ') b--;
    if (b <= a) return 0;
    if (b - a >= sizeof raw) b = a + sizeof raw - 1;
    memcpy(raw, raw_in + a, b - a); raw[b - a] = 0;
    music_search_normalize(raw, query, sizeof query);

    for (i = 0; i < list->count; i++) {
        const MusicInfo *mi = &list->items[i];
        if (!music_search_track_matches(mi, raw, query)) continue;
        if (n >= max) { if (truncated) *truncated = true; break; }
        out[n++] = mi;
    }
    return n;
}
