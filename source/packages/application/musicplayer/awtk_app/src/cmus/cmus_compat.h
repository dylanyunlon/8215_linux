/*
 * cmus_compat.h — Adaptation layer for porting cmus code to AWTK musicplayer
 *
 * Replaces the following cmus headers that depend on the full cmus build system:
 *   options.h   — global configuration (id3_default_charset)
 *   debug.h     — debug logging (d_print, BUG_ON)
 *   utils.h     — utility macros (N_ELEMENTS, min_i, str_to_int, is_freeform_true)
 *   path.h      — path utilities (get_extension)
 *   ui_curses.h — terminal UI globals (using_utf8, charset)
 *
 * Copyright 2026 — placed alongside cmus GPL-2.0 code
 */

#ifndef CMUS_COMPAT_H
#define CMUS_COMPAT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* ===================================================================
 * From options.h
 * id3.c uses this as the fallback charset for ISO-8859-1 encoded tags.
 * Defined in cmus_compat.c
 * =================================================================== */
extern char *id3_default_charset;

/* ===================================================================
 * From debug.h
 * =================================================================== */
#ifdef CMUS_DEBUG
  #define d_print(...) \
      do { fprintf(stderr, "[cmus:%s] ", __FUNCTION__); \
           fprintf(stderr, __VA_ARGS__); } while (0)
  #define _debug_print(func, ...) \
      do { fprintf(stderr, "[cmus:%s] ", func); \
           fprintf(stderr, __VA_ARGS__); } while (0)
#else
  #define d_print(...)            do { } while (0)
  #define _debug_print(func, ...) do { } while (0)
#endif

#define BUG_ON(a) \
    do { if (a) { \
        fprintf(stderr, "BUG: %s:%d: %s\n", __FILE__, __LINE__, #a); \
        abort(); \
    } } while (0)

/* ===================================================================
 * From utils.h
 * =================================================================== */
#ifndef N_ELEMENTS
#define N_ELEMENTS(array) (sizeof(array) / sizeof((array)[0]))
#endif

static inline long min_i(long a, long b)
{
	return a < b ? a : b;
}

static inline long max_i(long a, long b)
{
	return a > b ? a : b;
}

static inline int str_to_int(const char *str, long int *val)
{
	char *end;
	*val = strtol(str, &end, 10);
	if (*str == 0 || *end != 0)
		return -1;
	return 0;
}

static inline int is_freeform_true(const char *c)
{
	return c[0] == '1' ||
	       c[0] == 'y' || c[0] == 'Y' ||
	       c[0] == 't' || c[0] == 'T';
}

/* ===================================================================
 * From path.h
 * =================================================================== */
static inline const char *get_extension(const char *filename)
{
	const char *ext;

	ext = filename + strlen(filename) - 1;
	while (ext >= filename && *ext != '/') {
		if (*ext == '.') {
			ext++;
			return ext;
		}
		ext--;
	}
	return NULL;
}

/* ===================================================================
 * From ui_curses.h
 * uchar.c references using_utf8 and charset for to_utf8().
 * Embedded system is always UTF-8.
 * =================================================================== */
#ifndef using_utf8
#define using_utf8 1
#endif

/* charset is declared extern in the original; we provide a local definition */
static const char *const cmus_charset = "UTF-8";
/* Redirect the symbol uchar.c expects */
#define charset cmus_charset

/* ===================================================================
 * Build configuration
 * AC8215 Linux BSP glibc has iconv, strdup, strndup.
 * We do NOT define HAVE_CONFIG so xmalloc.h/convert.c skip config/ includes.
 * =================================================================== */
#ifndef HAVE_ICONV
#define HAVE_ICONV 1
#endif

/* strdup/strndup: xmalloc.h checks these */
#ifndef HAVE_STRDUP
#define HAVE_STRDUP 1
#endif

#ifndef HAVE_STRNDUP
#define HAVE_STRNDUP 1
#endif

/* wcwidth: wcwidth_uchar.h checks this */
#ifndef HAVE_WCWIDTH
#define HAVE_WCWIDTH 1
#endif

#endif /* CMUS_COMPAT_H */
