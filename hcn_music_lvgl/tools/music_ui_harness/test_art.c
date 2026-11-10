/* music_art: decode, placeholder, backdrop, async loader, LRU cache, shutdown races.  Real stb_image, real idle_queue shim. */
#include "compat/awtk.h"
#include "music_app.h"
#include "music_art.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static int g_checks;
#define CHECK(c) do { g_checks++; if (!(c)) { printf("CHECK FAILED line %d: %s\n", __LINE__, #c); exit(1); } } while (0)

/* ---- fake engine side: music_app_extract_art ---- */
static uint8_t *JPG, *BIG; static int JPG_N, BIG_N;
static volatile int g_extract_calls;
int music_app_extract_art(const char *path, uint8_t **out, int *n)
{
    __sync_fetch_and_add(&g_extract_calls, 1);
    usleep(1500);
    if (!strncmp(path, "none", 4)) return -1;
    if (!strncmp(path, "bad", 3)) { *out = (uint8_t *)malloc(64); memset(*out, 0x5a, 64); *n = 64; return 0; }
    if (!strncmp(path, "big", 3)) { *out = (uint8_t *)malloc(BIG_N); memcpy(*out, BIG, BIG_N); *n = BIG_N; return 0; }
    *out = (uint8_t *)malloc(JPG_N); memcpy(*out, JPG, JPG_N); *n = JPG_N; return 0;
}

static uint8_t *slurp(const char *p, int *n) { FILE *f = fopen(p, "rb"); uint8_t *b; if (!f) { *n = 0; return NULL; } fseek(f, 0, SEEK_END); *n = (int)ftell(f); fseek(f, 0, SEEK_SET); b = malloc(*n); if (fread(b, 1, *n, f) != (size_t)*n) *n = 0; fclose(f); return b; }

static int g_ready, g_evicted; static char g_last_ready[512];
static void on_ready(const char *p, void *u) { (void)u; g_ready++; snprintf(g_last_ready, sizeof g_last_ready, "%s", p); }
static void on_evict(const music_art_img_t *i, void *u) { (void)u; CHECK(i != NULL); g_evicted++; }
static void pump_until(volatile int *what, int target, int ms) { for (int t = 0; t < ms && *what < target; t++) { music_mainloop_poll(); usleep(1000); } }
static uint8_t alpha(const music_art_img_t *i, int x, int y) { return i->pixels[((size_t)y * i->dsc.header.w + x) * 4 + 3]; }

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    JPG = slurp("cover.jpg", &JPG_N); BIG = slurp("cover_big.jpg", &BIG_N);
    CHECK(JPG && JPG_N > 1000); CHECK(BIG && BIG_N > 100);

    /* ---- synchronous decode ---- */
    music_art_img_t *a = music_art_decode(JPG, JPG_N, 64, 10);
    CHECK(a && a->dsc.header.w == 64 && a->dsc.header.h == 64 && a->dsc.header.stride == 256 && a->dsc.header.cf == LV_COLOR_FORMAT_ARGB8888);
    CHECK(a->dsc.data == a->pixels && a->dsc.data_size == 64 * 64 * 4 && a->dsc.header.magic == LV_IMAGE_HEADER_MAGIC);
    CHECK(alpha(a, 0, 0) == 0 && alpha(a, 63, 0) == 0 && alpha(a, 0, 63) == 0 && alpha(a, 63, 63) == 0);   /* rounded corners */
    CHECK(alpha(a, 32, 32) == 255 && alpha(a, 32, 0) == 255 && alpha(a, 0, 32) == 255);                       /* edges stay opaque */
    CHECK(alpha(a, 2, 2) < 255);                                                                              /* anti-aliased */
    { int nz = 0; for (int i = 0; i < 64 * 64; i++) if (a->pixels[i * 4] | a->pixels[i * 4 + 1] | a->pixels[i * 4 + 2]) nz++; CHECK(nz > 64 * 64 / 2); }
    music_art_img_t *a0 = music_art_decode(JPG, JPG_N, 32, 0);
    CHECK(a0 && alpha(a0, 0, 0) == 255);                                                                      /* radius 0: square */
    CHECK(music_art_decode(NULL, 10, 64, 4) == NULL);
    CHECK(music_art_decode(JPG, 0, 64, 4) == NULL);
    CHECK(music_art_decode(JPG, JPG_N, 0, 4) == NULL);
    CHECK(music_art_decode(JPG, JPG_N, 5000, 4) == NULL);
    { uint8_t junk[64]; memset(junk, 0x5a, sizeof junk); CHECK(music_art_decode(junk, 64, 64, 4) == NULL); }
    { uint8_t trunc[400]; memcpy(trunc, JPG, 400); music_art_img_t *t = music_art_decode(trunc, 400, 64, 4); music_art_free(t); }   /* truncated: NULL or partial, never a crash */
    CHECK(music_art_decode(BIG, BIG_N, 64, 4) == NULL);                                                       /* 2500x2500 > limit */

    /* ---- placeholder + backdrop ---- */
    music_art_img_t *ph = music_art_make_placeholder(100, 14, 0x1F2933, 0x8D9BA8);
    CHECK(ph && alpha(ph, 0, 0) == 0 && alpha(ph, 50, 50) == 255);
    { const uint8_t *c = ph->pixels + (50 * 100 + 5) * 4; CHECK(c[0] == 0x33 && c[1] == 0x29 && c[2] == 0x1F); }   /* plain bg at the edge */
    { const uint8_t *c = ph->pixels + (50 * 100 + 66) * 4; CHECK(c[0] != 0x33 || c[1] != 0x29); }                  /* disc ring differs */
    music_art_img_t *bd = music_art_make_backdrop(a, 256, 150);
    CHECK(bd && bd->dsc.header.w == 256 && bd->dsc.header.h == 150 && bd->dsc.header.stride == 1024 && alpha(bd, 0, 0) == 255);
    { /* the sunset cover is blue on top and darker/pinker at the bottom: the wash must keep that vertical difference and be smooth */
      const uint8_t *top = bd->pixels + (5 * 256 + 128) * 4, *bot = bd->pixels + (144 * 256 + 128) * 4;
      CHECK(abs((int)top[0] - (int)bot[0]) + abs((int)top[2] - (int)bot[2]) > 40);
      int maxstep = 0; for (int x = 1; x < 256; x++) { const uint8_t *p = bd->pixels + (75 * 256 + x) * 4, *q = p - 4; int d = abs(p[0] - q[0]) + abs(p[1] - q[1]) + abs(p[2] - q[2]); if (d > maxstep) maxstep = d; }
      CHECK(maxstep < 24); }
    CHECK(music_art_make_backdrop(NULL, 10, 10) == NULL); CHECK(music_art_make_backdrop(a, 0, 10) == NULL);
    music_art_free(a); music_art_free(a0); music_art_free(ph); music_art_free(bd); music_art_free(NULL);

    /* ---- async loader ---- */
    bool known; music_art_init(); music_art_set_ready_cb(on_ready, NULL); music_art_set_evict_cb(on_evict, NULL);
    CHECK(music_art_get("a.mp3", 64, 8, &known) == NULL && !known);                 /* miss -> queued */
    pump_until(&g_ready, 1, 2000); CHECK(g_ready == 1 && !strcmp(g_last_ready, "a.mp3"));
    const music_art_img_t *got = music_art_get("a.mp3", 64, 8, &known); CHECK(got && known && got->dsc.header.w == 64);
    CHECK(music_art_get("a.mp3", 64, 8, &known) == got);                             /* stable pointer */
    CHECK(music_art_get("none1.mp3", 64, 8, &known) == NULL && !known);
    pump_until(&g_ready, 2, 2000); CHECK(music_art_get("none1.mp3", 64, 8, &known) == NULL && known);   /* final "no art" */
    CHECK(music_art_get("bad1.mp3", 64, 8, &known) == NULL && !known);
    pump_until(&g_ready, 3, 2000); CHECK(music_art_get("bad1.mp3", 64, 8, &known) == NULL && known);    /* undecodable => no art, no crash */
    CHECK(music_art_get("", 64, 8, &known) == NULL && !known); CHECK(music_art_get(NULL, 64, 8, &known) == NULL);

    /* dedupe: 100 requests for one path = one extraction */
    { int before = g_extract_calls, r0 = g_ready;
      for (int i = 0; i < 100; i++) music_art_get("dup.mp3", 64, 8, &known);
      pump_until(&g_ready, r0 + 1, 2000); usleep(20000); music_mainloop_poll();
      CHECK(g_extract_calls - before == 1); CHECK(g_ready == r0 + 1); }

    /* LRU: 40 distinct covers through a 24-entry cache; the hot one must survive */
    { char p[64]; g_evicted = 0; int r0 = g_ready;
      for (int i = 0; i < 40; i++) {
          snprintf(p, sizeof p, "t%02d.mp3", i); music_art_get(p, 64, 8, &known);
          music_art_get("a.mp3", 64, 8, &known);                                    /* keep "a" hot */
          for (int t = 0; t < 30; t++) { music_mainloop_poll(); usleep(500); }
      }
      pump_until(&g_ready, r0 + 40, 5000);
      CHECK(g_ready - r0 >= 39); CHECK(g_evicted >= 40 + 4 - MUSIC_ART_CACHE_MAX - 2);   /* the two "no art" entries have no image, so no evict callback */
      CHECK(music_art_get("a.mp3", 64, 8, &known) != NULL && known);               /* survived */
      CHECK(music_art_get("t00.mp3", 64, 8, &known) == NULL && !known);           /* oldest was evicted, so it is a miss again */ }

    /* shutdown with work pending: no crash, no leak (ASan), late deliveries are discarded */
    { char p[64]; for (int i = 0; i < 60; i++) { snprintf(p, sizeof p, "late%02d.mp3", i); music_art_get(p, 64, 8, &known); }
      music_art_deinit();
      for (int i = 0; i < 20; i++) { music_mainloop_poll(); usleep(1000); }
      CHECK(music_art_get("a.mp3", 64, 8, &known) == NULL && !known);               /* not initialised: no queueing, no crash */ }
    /* init again works */
    music_art_init(); g_ready = 0; music_art_set_ready_cb(on_ready, NULL);
    CHECK(music_art_get("again.mp3", 64, 8, &known) == NULL); pump_until(&g_ready, 1, 2000); CHECK(g_ready == 1);
    music_art_deinit(); music_art_deinit();                                           /* idempotent */
    music_mainloop_deinit(); free(JPG); free(BIG);
    printf("ART TESTS PASS (%d checks)\n", g_checks);
    return 0;
}
