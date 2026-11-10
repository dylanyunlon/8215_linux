/**
 * @file music_pinyin.c
 */
#include "music_pinyin.h"

#include <string.h>

extern const uint16_t music_pinyin_cp[];
extern const uint32_t music_pinyin_mask[];
extern const size_t music_pinyin_count;

#define MAX_UNITS 256

typedef struct {
    uint32_t mask;      /* letters this unit can stand for */
    char digit;         /* '0'..'9' or 0 */
} unit_t;

int music_utf8_next(const char *s, uint32_t *cp)
{
    const unsigned char *p = (const unsigned char *)s;
    if (p[0] == 0) { *cp = 0; return 0; }
    if (p[0] < 0x80) { *cp = p[0]; return 1; }
    if ((p[0] & 0xE0) == 0xC0 && (p[1] & 0xC0) == 0x80) { *cp = ((p[0] & 0x1Fu) << 6) | (p[1] & 0x3Fu); return 2; }
    if ((p[0] & 0xF0) == 0xE0 && (p[1] & 0xC0) == 0x80 && (p[2] & 0xC0) == 0x80) {
        *cp = ((p[0] & 0x0Fu) << 12) | ((p[1] & 0x3Fu) << 6) | (p[2] & 0x3Fu); return 3;
    }
    if ((p[0] & 0xF8) == 0xF0 && (p[1] & 0xC0) == 0x80 && (p[2] & 0xC0) == 0x80 && (p[3] & 0xC0) == 0x80) {
        *cp = ((p[0] & 0x07u) << 18) | ((p[1] & 0x3Fu) << 12) | ((p[2] & 0x3Fu) << 6) | (p[3] & 0x3Fu); return 4;
    }
    *cp = 0xFFFD;
    return 1;
}

uint32_t music_pinyin_initial_mask(uint32_t cp)
{
    size_t lo = 0, hi = music_pinyin_count;
    if (cp > 0xFFFF) return 0;
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        if (music_pinyin_cp[mid] == cp) return music_pinyin_mask[mid];
        if (music_pinyin_cp[mid] < cp) lo = mid + 1; else hi = mid;
    }
    return 0;
}

bool music_pinyin_match(const char *text, const char *query)
{
    unit_t u[MAX_UNITS];
    size_t n = 0, m = strlen(query), s, j;

    if (text == NULL || m == 0 || m > MAX_UNITS) return false;
    for (j = 0; j < m; j++) {
        char c = query[j];
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))) return false;
    }
    while (*text && n < MAX_UNITS) {
        uint32_t cp;
        int len = music_utf8_next(text, &cp);
        text += len;
        if (cp < 0x80) {
            if (cp >= 'A' && cp <= 'Z') cp = cp - 'A' + 'a';
            if (cp >= 'a' && cp <= 'z') { u[n].mask = 1u << (cp - 'a'); u[n].digit = 0; n++; }
            else if (cp >= '0' && cp <= '9') { u[n].mask = 0; u[n].digit = (char)cp; n++; }
            /* spaces / punctuation: skipped */
        } else {
            uint32_t mask = music_pinyin_initial_mask(cp);
            u[n].mask = mask; u[n].digit = 0; n++;    /* unknown non-ASCII: a unit that matches nothing, breaks runs */
        }
    }
    if (n < m) return false;
    for (s = 0; s + m <= n; s++) {
        for (j = 0; j < m; j++) {
            char q = query[j];
            bool ok = (q >= 'a' && q <= 'z') ? ((u[s + j].mask >> (q - 'a')) & 1u) : (u[s + j].digit == q);
            if (!ok) break;
        }
        if (j == m) return true;
    }
    return false;
}
