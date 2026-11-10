/**
 * @file music_pinyin.h
 * @brief Pinyin-initial matching for the music search (board lv_conf has LV_USE_IME_PINYIN=0, and the LVGL
 *        keyboard is Latin-only: typing "zjl" must find 周杰伦).  Pure C, no LVGL, no music_app.
 */
#ifndef MUSIC_PINYIN_H
#define MUSIC_PINYIN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Bit i set => the hanzi can start a syllable with letter 'a'+i (all readings).  0 = not a known hanzi. */
uint32_t music_pinyin_initial_mask(uint32_t codepoint);

/** Decode one UTF-8 character.  Returns bytes consumed (>=1); *cp = U+FFFD for malformed input. */
int music_utf8_next(const char *s, uint32_t *cp);

/** `query` = lowercase [a-z0-9] only.  True if it matches a run of consecutive characters of `text`, where a hanzi
 *  matches any of its pinyin initials, ASCII letters/digits match themselves (case-insensitive), and spaces and
 *  punctuation are skipped.  Empty query never matches. */
bool music_pinyin_match(const char *text, const char *query);

#ifdef __cplusplus
}
#endif
#endif /* MUSIC_PINYIN_H */
