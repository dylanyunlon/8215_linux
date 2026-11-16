/**
 * @file music_art.c
 */
#include "music_art.h"
#include "compat/awtk.h"
#include "music_app.h"

#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stb_image.h" /* declarations only: the implementation lives in libmusicplayer.so (img_blur.cpp) */

#define MAX_SRC_PIXELS (2048 * 2048)
#define PATH_MAX_LEN   MUSIC_MAX_PATH_LEN

/* ------------------------------------------------------------------ pixels */
static music_art_img_t *img_new(int px)
{
    music_art_img_t *img = (music_art_img_t *)calloc(1, sizeof *img);
    if (!img) return NULL;
    img->pixels = (uint8_t *)malloc((size_t)px * px * 4);
    if (!img->pixels) { free(img); return NULL; }
    img->dsc.header.magic = LV_IMAGE_HEADER_MAGIC;
    img->dsc.header.cf = LV_COLOR_FORMAT_ARGB8888;
    img->dsc.header.w = (uint16_t)px;
    img->dsc.header.h = (uint16_t)px;
    img->dsc.header.stride = (uint16_t)(px * 4);
    img->dsc.data_size = (uint32_t)px * px * 4;
    img->dsc.data = img->pixels;
    return img;
}

void music_art_free(music_art_img_t *img)
{
    if (!img) return;
    free(img->pixels);
    free(img);
}

static void put_px(uint8_t *p, uint32_t rgb, uint8_t a)      /* ARGB8888, little endian: B G R A */
{
    p[0] = (uint8_t)(rgb & 0xFF); p[1] = (uint8_t)((rgb >> 8) & 0xFF); p[2] = (uint8_t)((rgb >> 16) & 0xFF); p[3] = a;
}

/* anti-aliased rounded corners: scale alpha by the coverage of the corner circle */
static void round_corners(uint8_t *pixels, int px, int r)
{
    int x, y;
    if (r <= 0) return;
    if (r > px / 2) r = px / 2;
    for (y = 0; y < r; y++) {
        for (x = 0; x < r; x++) {
            float dx = (float)r - (x + 0.5f), dy = (float)r - (y + 0.5f);
            float cov = (float)r - sqrtf(dx * dx + dy * dy) + 0.5f;
            int xs[2] = {x, px - 1 - x}, ys[2] = {y, px - 1 - y}, i, j;
            if (cov >= 1.0f) continue;
            if (cov < 0.0f) cov = 0.0f;
            for (i = 0; i < 2; i++) for (j = 0; j < 2; j++) {
                uint8_t *a = pixels + ((size_t)ys[j] * px + xs[i]) * 4 + 3;
                *a = (uint8_t)(*a * cov);
            }
        }
    }
}

music_art_img_t *music_art_decode(const uint8_t *data, int size, int px, int radius)
{
    int w = 0, h = 0, comp = 0, side, ox, oy, x, y;
    uint8_t *rgba;
    music_art_img_t *img;

    if (!data || size <= 0 || px <= 0 || px > 1024) return NULL;
    if (!stbi_info_from_memory(data, size, &w, &h, &comp) || w <= 0 || h <= 0 || (int64_t)w * h > MAX_SRC_PIXELS) return NULL;
    rgba = stbi_load_from_memory(data, size, &w, &h, &comp, 4);
    if (!rgba) return NULL;
    img = img_new(px);
    if (!img) { stbi_image_free(rgba); return NULL; }

    side = w < h ? w : h; ox = (w - side) / 2; oy = (h - side) / 2;
    for (y = 0; y < px; y++) {
        int y0 = oy + (int)((int64_t)y * side / px), y1 = oy + (int)((int64_t)(y + 1) * side / px);
        uint8_t *out = img->pixels + (size_t)y * px * 4;
        if (y1 <= y0) y1 = y0 + 1;
        for (x = 0; x < px; x++) {
            int x0 = ox + (int)((int64_t)x * side / px), x1 = ox + (int)((int64_t)(x + 1) * side / px), xx, yy;
            unsigned r = 0, g = 0, b = 0, n = 0;
            if (x1 <= x0) x1 = x0 + 1;
            for (yy = y0; yy < y1; yy++) {
                const uint8_t *p = rgba + ((size_t)yy * w + x0) * 4;
                for (xx = x0; xx < x1; xx++, p += 4) { r += p[0]; g += p[1]; b += p[2]; n++; }
            }
            out[x * 4 + 0] = (uint8_t)(b / n); out[x * 4 + 1] = (uint8_t)(g / n); out[x * 4 + 2] = (uint8_t)(r / n); out[x * 4 + 3] = 255;
        }
    }
    stbi_image_free(rgba);
    round_corners(img->pixels, px, radius);
    return img;
}

music_art_img_t *music_art_make_placeholder(int px, int radius, uint32_t bg_rgb, uint32_t disc_rgb)
{
    music_art_img_t *img = img_new(px);
    int x, y;
    float c = px / 2.0f, rout = px * 0.34f, rin = px * 0.07f, rmid = px * 0.20f;
    if (!img) return NULL;
    for (y = 0; y < px; y++) for (x = 0; x < px; x++) {
        float d = sqrtf((x + 0.5f - c) * (x + 0.5f - c) + (y + 0.5f - c) * (y + 0.5f - c));
        uint32_t r = (bg_rgb >> 16) & 0xFF, g = (bg_rgb >> 8) & 0xFF, b = bg_rgb & 0xFF;
        uint32_t dr = (disc_rgb >> 16) & 0xFF, dg = (disc_rgb >> 8) & 0xFF, db = disc_rgb & 0xFF;
        float k = 0.0f;
        if (d <= rout && d >= rin) k = (d > rmid - 1.0f && d < rmid + 1.0f) ? 0.55f : 0.30f;    /* disc with a groove ring */
        r = (uint32_t)(r + (dr - (float)r) * k); g = (uint32_t)(g + (dg - (float)g) * k); b = (uint32_t)(b + (db - (float)b) * k);
        put_px(img->pixels + ((size_t)y * px + x) * 4, (r << 16) | (g << 8) | b, 255);
    }
    round_corners(img->pixels, px, radius);
    return img;
}

music_art_img_t *music_art_make_backdrop(const music_art_img_t *src, int w, int h)
{
    enum { GW = 8, GH = 5 };
    float grid[GH][GW][3];
    int sp, gx, gy, x, y;
    music_art_img_t *img;
    /* the backdrop is not square: allocate w*h ARGB and fix the header by hand */
    if (!src || w <= 0 || h <= 0 || w > 4096 || h > 4096) return NULL;
    img = (music_art_img_t *)calloc(1, sizeof *img);
    if (!img) return NULL;
    img->pixels = (uint8_t *)malloc((size_t)w * h * 4);
    if (!img->pixels) { free(img); return NULL; }
    img->dsc.header.magic = LV_IMAGE_HEADER_MAGIC; img->dsc.header.cf = LV_COLOR_FORMAT_ARGB8888;
    img->dsc.header.w = (uint16_t)w; img->dsc.header.h = (uint16_t)h; img->dsc.header.stride = (uint16_t)(w * 4);
    img->dsc.data_size = (uint32_t)w * h * 4; img->dsc.data = img->pixels;

    sp = src->dsc.header.w;
    for (gy = 0; gy < GH; gy++) for (gx = 0; gx < GW; gx++) {
        int x0 = gx * sp / GW, x1 = (gx + 1) * sp / GW, y0 = gy * sp / GH, y1 = (gy + 1) * sp / GH, xx, yy;
        float sum[3] = {0, 0, 0}; int n = 0;
        for (yy = y0; yy < y1; yy++) for (xx = x0; xx < x1; xx++) {
            const uint8_t *p = src->pixels + ((size_t)yy * sp + xx) * 4;
            if (p[3] < 128) continue;                     /* skip transparent rounded corners */
            sum[0] += p[0]; sum[1] += p[1]; sum[2] += p[2]; n++;
        }
        for (x = 0; x < 3; x++) grid[gy][gx][x] = n ? sum[x] / n : 0.0f;
    }
    for (y = 0; y < h; y++) {
        float fy = (y + 0.5f) * GH / h - 0.5f, ty;
        int y0 = (int)floorf(fy), y1;
        if (y0 < 0) y0 = 0;
        y1 = y0 + 1;
        if (y1 >= GH) y1 = GH - 1;
        ty = fy - y0;
        if (ty < 0) ty = 0;
        if (ty > 1) ty = 1;
        for (x = 0; x < w; x++) {
            float fx = (x + 0.5f) * GW / w - 0.5f, tx;
            int x0 = (int)floorf(fx), x1, c;
            uint8_t *o = img->pixels + ((size_t)y * w + x) * 4;
            if (x0 < 0) x0 = 0;
            x1 = x0 + 1;
            if (x1 >= GW) x1 = GW - 1;
            tx = fx - x0;
            if (tx < 0) tx = 0;
            if (tx > 1) tx = 1;
            for (c = 0; c < 3; c++) {
                float a = grid[y0][x0][c] * (1 - tx) + grid[y0][x1][c] * tx, b = grid[y1][x0][c] * (1 - tx) + grid[y1][x1][c] * tx;
                o[c] = (uint8_t)(a * (1 - ty) + b * ty + 0.5f);
            }
            o[3] = 255;
        }
    }
    return img;
}

/* ------------------------------------------------------------------ async loader + cache */
typedef struct entry {
    char path[PATH_MAX_LEN];
    int px, radius;
    music_art_img_t *img;                    /* NULL + used = "known to have no art" */
    uint32_t stamp;
    bool used;
} entry_t;

typedef struct req {
    char path[PATH_MAX_LEN];
    int px, radius;
    struct req *next;
} req_t;

typedef struct result {
    char path[PATH_MAX_LEN];
    int px, radius;
    music_art_img_t *img;
} result_t;

static entry_t s_cache[MUSIC_ART_CACHE_MAX];
static uint32_t s_stamp;
static pthread_mutex_t s_mu = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_cv = PTHREAD_COND_INITIALIZER;
static pthread_t s_thread;
static req_t *s_queue;                       /* LIFO: the newest request is the most relevant (user is scrolling) */
static char s_busy[PATH_MAX_LEN];
static bool s_running, s_inited;             /* s_running: worker should run; s_inited: GUI side accepts results */
static music_art_ready_cb s_ready; static void *s_ready_user;
static music_art_evict_cb s_evict; static void *s_evict_user;

void music_art_set_ready_cb(music_art_ready_cb cb, void *user) { s_ready = cb; s_ready_user = user; }
void music_art_set_evict_cb(music_art_evict_cb cb, void *user) { s_evict = cb; s_evict_user = user; }

static ret_t deliver(const idle_info_t *info)
{
    result_t *r = (result_t *)info->ctx;
    if (s_inited) {
        entry_t *e = NULL, *oldest = NULL;
        int i;
        for (i = 0; i < MUSIC_ART_CACHE_MAX; i++) {
            entry_t *c = &s_cache[i];
            if (c->used && c->px == r->px && c->radius == r->radius && strcmp(c->path, r->path) == 0) { e = c; break; }
            if (!c->used) { if (!e) e = c; }
            else if (!oldest || c->stamp < oldest->stamp) oldest = c;
        }
        if (e && e->used) {                                   /* same key: replace */
            if (e->img && s_evict) s_evict(e->img, s_evict_user);
            music_art_free(e->img);
        } else if (!e) {                                      /* full: evict the least recently used */
            e = oldest;
            if (e->img && s_evict) s_evict(e->img, s_evict_user);
            music_art_free(e->img);
        }
        snprintf(e->path, sizeof e->path, "%s", r->path);
        e->px = r->px; e->radius = r->radius; e->img = r->img; e->used = true; e->stamp = ++s_stamp;
        r->img = NULL;
        if (s_ready) s_ready(r->path, s_ready_user);
    }
    music_art_free(r->img);                                   /* not taken (deinit'ed meanwhile) */
    free(r);
    return RET_REMOVE;
}

static void *worker(void *arg)
{
    (void)arg;
    for (;;) {
        req_t *rq;
        result_t *res;
        uint8_t *data = NULL; int size = 0;
        music_art_img_t *img = NULL;

        pthread_mutex_lock(&s_mu);
        while (s_running && !s_queue) pthread_cond_wait(&s_cv, &s_mu);
        if (!s_running) { pthread_mutex_unlock(&s_mu); break; }
        rq = s_queue; s_queue = rq->next;
        snprintf(s_busy, sizeof s_busy, "%s", rq->path);
        pthread_mutex_unlock(&s_mu);

        if (music_app_extract_art(rq->path, &data, &size) == 0 && data && size > 0)
            img = music_art_decode(data, size, rq->px, rq->radius);
        free(data);

        res = (result_t *)calloc(1, sizeof *res);
        if (res) {
            snprintf(res->path, sizeof res->path, "%s", rq->path);
            res->px = rq->px; res->radius = rq->radius; res->img = img;
            if (idle_queue(deliver, res) != RET_OK) { music_art_free(res->img); free(res); }
        } else music_art_free(img);

        pthread_mutex_lock(&s_mu); s_busy[0] = 0; pthread_mutex_unlock(&s_mu);
        free(rq);
    }
    return NULL;
}

void music_art_init(void)
{
    if (s_inited) return;
    memset(s_cache, 0, sizeof s_cache);
    s_stamp = 0;
    pthread_mutex_lock(&s_mu); s_running = true; s_busy[0] = 0; pthread_mutex_unlock(&s_mu);
    if (pthread_create(&s_thread, NULL, worker, NULL) != 0) { s_running = false; return; }
    s_inited = true;
}

void music_art_deinit(void)
{
    req_t *q;
    int i;
    if (!s_inited) return;
    s_inited = false;
    pthread_mutex_lock(&s_mu);
    s_running = false;
    q = s_queue; s_queue = NULL;
    pthread_cond_broadcast(&s_cv);
    pthread_mutex_unlock(&s_mu);
    pthread_join(s_thread, NULL);
    while (q) { req_t *n = q->next; free(q); q = n; }
    for (i = 0; i < MUSIC_ART_CACHE_MAX; i++) {
        if (s_cache[i].used) { music_art_free(s_cache[i].img); s_cache[i].img = NULL; s_cache[i].used = false; }
    }
}

const music_art_img_t *music_art_get(const char *path, int px, int radius, bool *known)
{
    int i;
    req_t *rq, *it;
    bool dup = false;
    if (known) *known = false;
    if (!s_inited || !path || !path[0]) return NULL;
    for (i = 0; i < MUSIC_ART_CACHE_MAX; i++) {
        entry_t *c = &s_cache[i];
        if (c->used && c->px == px && c->radius == radius && strcmp(c->path, path) == 0) {
            c->stamp = ++s_stamp;
            if (known) *known = true;
            return c->img;
        }
    }
    pthread_mutex_lock(&s_mu);
    if (strcmp(s_busy, path) == 0) dup = true;
    for (it = s_queue; it && !dup; it = it->next) if (it->px == px && it->radius == radius && strcmp(it->path, path) == 0) dup = true;
    if (!dup) {
        rq = (req_t *)calloc(1, sizeof *rq);
        if (rq) {
            snprintf(rq->path, sizeof rq->path, "%s", path);
            rq->px = px; rq->radius = radius;
            rq->next = s_queue; s_queue = rq;
            pthread_cond_signal(&s_cv);
        }
    }
    pthread_mutex_unlock(&s_mu);
    return NULL;
}
