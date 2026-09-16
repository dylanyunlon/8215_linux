/**
 * @file img_blur.cpp
 * @brief Album art blur with preload cache.
 *
 * Cache keyed by MP3 filepath (stable key, same across temp file rotations).
 * On track change: if next track was preloaded → instant cache hit.
 */

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_NO_STDIO
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "fast_gaussian_blur.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <string>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <vector>

#define SCREEN_W  1024
#define SCREEN_H  600
#define BLUR_MAX  128
#define CACHE_MAX 4

/*--- Scale helpers ---*/

static void downscale(const uint8_t* src, int sw, int sh,
                      uint8_t* dst, int dw, int dh) {
    float xr = (float)sw / dw, yr = (float)sh / dh;
    for (int y = 0; y < dh; y++) {
        int sy = std::min((int)(y * yr), sh - 1);
        for (int x = 0; x < dw; x++) {
            int sx = std::min((int)(x * xr), sw - 1);
            memcpy(dst + (y * dw + x) * 3, src + (sy * sw + sx) * 3, 3);
        }
    }
}

static void upscale(const uint8_t* src, int sw, int sh,
                    uint8_t* dst, int dw, int dh) {
    float xr = (float)(sw - 1) / dw, yr = (float)(sh - 1) / dh;
    for (int y = 0; y < dh; y++) {
        float fy = y * yr; int iy = (int)fy; float dy = fy - iy;
        if (iy >= sh - 1) { iy = sh - 2; dy = 1.0f; }
        for (int x = 0; x < dw; x++) {
            float fx = x * xr; int ix = (int)fx; float dx = fx - ix;
            if (ix >= sw - 1) { ix = sw - 2; dx = 1.0f; }
            const uint8_t* p00 = src + (iy * sw + ix) * 3;
            const uint8_t* p10 = p00 + 3;
            const uint8_t* p01 = p00 + sw * 3;
            const uint8_t* p11 = p01 + 3;
            uint8_t* dp = dst + (y * dw + x) * 3;
            for (int c = 0; c < 3; c++) {
                float v = p00[c]*(1-dx)*(1-dy) + p10[c]*dx*(1-dy)
                        + p01[c]*(1-dx)*dy     + p11[c]*dx*dy;
                dp[c] = (uint8_t)(v + 0.5f);
            }
        }
    }
}

/*--- Core blur: image file → JPEG bytes in memory ---*/

struct JpegBuf { uint8_t* data; int size; int cap; };

static void jpeg_write_cb(void* ctx, void* data, int size) {
    JpegBuf* b = (JpegBuf*)ctx;
    if (b->size + size > b->cap) {
        b->cap = std::max(b->cap * 2, b->size + size + 4096);
        b->data = (uint8_t*)realloc(b->data, b->cap);
    }
    if (b->data) { memcpy(b->data + b->size, data, size); b->size += size; }
}

static uint8_t* blur_to_jpeg(const char* art_path, float sigma, int* out_size) {
    FILE* fp = fopen(art_path, "rb");
    if (!fp) return NULL;
    fseek(fp, 0, SEEK_END); long sz = ftell(fp); fseek(fp, 0, SEEK_SET);
    if (sz <= 0 || sz > 10*1024*1024) { fclose(fp); return NULL; }
    uint8_t* buf = (uint8_t*)malloc(sz);
    if (!buf) { fclose(fp); return NULL; }
    fread(buf, 1, sz, fp); fclose(fp);

    int ow, oh, ch;
    uint8_t* rgb = stbi_load_from_memory(buf, (int)sz, &ow, &oh, &ch, 3);
    free(buf);
    if (!rgb) return NULL;

    float ratio = (float)std::max(ow, oh) / BLUR_MAX;
    if (ratio < 1.0f) ratio = 1.0f;
    int bw = std::max(4, (int)(ow / ratio)), bh = std::max(4, (int)(oh / ratio));

    uint8_t* small = (uint8_t*)malloc(bw * bh * 3);
    if (!small) { stbi_image_free(rgb); return NULL; }
    downscale(rgb, ow, oh, small, bw, bh);
    stbi_image_free(rgb);

    uint8_t* tmp = (uint8_t*)malloc(bw * bh * 3);
    if (!tmp) { free(small); return NULL; }
    uint8_t* bi = small, *bo = tmp;
    fast_gaussian_blur(bi, bo, bw, bh, 3, std::max(1.0f, sigma / ratio));

    uint8_t* screen = (uint8_t*)malloc(SCREEN_W * SCREEN_H * 3);
    if (!screen) { free(bi); free(bo); return NULL; }
    upscale(bo, bw, bh, screen, SCREEN_W, SCREEN_H);
    free(bi); free(bo);

    JpegBuf jbuf = {NULL, 0, 0};
    stbi_write_jpg_to_func(jpeg_write_cb, &jbuf, SCREEN_W, SCREEN_H, 3, screen, 75);
    free(screen);

    if (jbuf.data && jbuf.size > 0) { *out_size = jbuf.size; return jbuf.data; }
    if (jbuf.data) free(jbuf.data);
    return NULL;
}

/*--- Cache (keyed by MP3 filepath) ---*/

static std::mutex s_mtx;
static std::unordered_map<std::string, std::vector<uint8_t>> s_cache;

static void cache_put(const std::string& key, const uint8_t* d, int sz) {
    std::lock_guard<std::mutex> lk(s_mtx);
    if (s_cache.size() >= CACHE_MAX && s_cache.find(key) == s_cache.end())
        s_cache.erase(s_cache.begin());
    s_cache[key].assign(d, d + sz);
}

static bool cache_write(const std::string& key, const char* dst) {
    std::lock_guard<std::mutex> lk(s_mtx);
    auto it = s_cache.find(key);
    if (it == s_cache.end()) return false;
    FILE* fp = fopen(dst, "wb");
    if (!fp) return false;
    fwrite(it->second.data(), 1, it->second.size(), fp);
    fclose(fp);
    return true;
}

static bool cache_has(const std::string& key) {
    std::lock_guard<std::mutex> lk(s_mtx);
    return s_cache.count(key) > 0;
}

/*--- Public C API ---*/

typedef void (*img_blur_done_cb)(const char* dst_path, void* user_data);

/**
 * Async blur with cache. cache_key = MP3 filepath (stable).
 * art_path = temp JPEG file with album art.
 * dst_path = where to write blurred JPEG.
 */
extern "C"
void img_blur_album_art_async(const char* cache_key, const char* art_path,
                               const char* dst_path, float sigma,
                               img_blur_done_cb cb, void* ud)
{
    std::string key(cache_key ? cache_key : art_path);
    std::string art(art_path), dst(dst_path);
    float s = sigma > 0 ? sigma : 8.0f;

    if (cache_write(key, dst_path)) {
        fprintf(stderr, "[img_blur] CACHE HIT: %s\n", cache_key);
        if (cb) cb(dst_path, ud);
        return;
    }

    std::thread([key, art, dst, s, cb, ud]() {
        int jsz = 0;
        uint8_t* jpg = blur_to_jpeg(art.c_str(), s, &jsz);
        if (jpg) {
            cache_put(key, jpg, jsz);
            FILE* fp = fopen(dst.c_str(), "wb");
            if (fp) { fwrite(jpg, 1, jsz, fp); fclose(fp); }
            free(jpg);
            fprintf(stderr, "[img_blur] COMPUTED: %s\n", key.c_str());
            if (cb) cb(dst.c_str(), ud);
        }
    }).detach();
}

/**
 * Preload: extract art from MP3, blur it, cache under mp3_path key.
 * art_file = temp file with extracted APIC data.
 */
extern "C"
void img_blur_preload(const char* mp3_path, const char* art_file, float sigma)
{
    if (!mp3_path || !art_file) return;
    std::string key(mp3_path), art(art_file);
    if (cache_has(key)) return;

    float s = sigma > 0 ? sigma : 8.0f;
    std::thread([key, art, s]() {
        int jsz = 0;
        uint8_t* jpg = blur_to_jpeg(art.c_str(), s, &jsz);
        if (jpg) {
            cache_put(key, jpg, jsz);
            free(jpg);
            fprintf(stderr, "[img_blur] PRELOADED: %s\n", key.c_str());
        }
    }).detach();
}

/* Legacy sync API (still works, no cache key) */
extern "C"
int img_blur_album_art(const char* src_path, const char* dst_path,
                       float sigma, int scale)
{
    (void)scale;
    if (!src_path || !dst_path) return -1;
    int jsz = 0;
    uint8_t* jpg = blur_to_jpeg(src_path, sigma > 0 ? sigma : 8.0f, &jsz);
    if (!jpg) return -1;
    FILE* fp = fopen(dst_path, "wb");
    if (!fp) { free(jpg); return -1; }
    fwrite(jpg, 1, jsz, fp);
    fclose(fp); free(jpg);
    return 0;
}