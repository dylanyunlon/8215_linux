/*
 * darray.h — 纯C泛型动态数组 (替代 std::vector)
 *
 * 用法:
 *   DARRAY_DEFINE(MusicInfoArray, MusicInfo);    // 定义类型
 *
 *   MusicInfoArray arr;
 *   darray_init(&arr, 64);                       // 初始容量64
 *   darray_push(&arr, &some_info);               // 追加元素 (自动扩容)
 *   MusicInfo *p = darray_get(&arr, 3);          // O(1) 随机访问
 *   darray_clear(&arr);                          // 清空 (不释放内存)
 *   darray_destroy(&arr);                        // 释放内存
 *
 * 设计原则:
 *   - POD 类型专用 (MusicInfo, char*, int 等无析构函数的类型)
 *   - realloc 2x 扩容, 与 std::vector 策略一致
 *   - 所有操作返回 0=成功, -1=失败 (OOM)
 *   - 线程不安全 (调用者自行加锁, 与 ad010 的 Mutex::Autolock 对齐)
 *
 * 对应关系:
 *   ad010: std::vector<std::string>    →  DARRAY_DEFINE(StrArray, char*)
 *   ad010: std::vector<MusicInfo>      →  DARRAY_DEFINE(MusicInfoArray, MusicInfo)
 *   ad010: std::set<callback_fn>       →  DARRAY_DEFINE(CallbackArray, callback_fn)
 *
 * Copyright (c) 2026. All rights reserved.
 */

#ifndef DARRAY_H
#define DARRAY_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * DARRAY_DEFINE(TypeName, ElemType)
 *
 * 展开后生成:
 *   typedef struct { ElemType *items; int count; int capacity; } TypeName;
 *
 * 以及一组 static inline 函数:
 *   TypeName_init, TypeName_destroy, TypeName_push, TypeName_get,
 *   TypeName_remove, TypeName_clear, TypeName_ensure_capacity
 *
 * 全部 static inline, 头文件 only, 零链接开销。
 */

#define DARRAY_DEFINE(TypeName, ElemType)                                      \
                                                                               \
typedef struct {                                                               \
    ElemType *items;                                                           \
    int       count;                                                           \
    int       capacity;                                                        \
} TypeName;                                                                    \
                                                                               \
/* 初始化, init_cap=0 则默认 16 */                                               \
static inline int TypeName##_init(TypeName *a, int init_cap) {                 \
    if (init_cap <= 0) init_cap = 16;                                          \
    a->items = (ElemType *)calloc(init_cap, sizeof(ElemType));                 \
    if (!a->items) return -1;                                                  \
    a->count = 0;                                                              \
    a->capacity = init_cap;                                                    \
    return 0;                                                                  \
}                                                                              \
                                                                               \
/* 释放内存 */                                                                   \
static inline void TypeName##_destroy(TypeName *a) {                           \
    if (a->items) { free(a->items); a->items = NULL; }                         \
    a->count = 0;                                                              \
    a->capacity = 0;                                                           \
}                                                                              \
                                                                               \
/* 确保至少还有 1 个空位, 不够则 2x 扩容 */                                          \
static inline int TypeName##_ensure(TypeName *a) {                             \
    if (a->count < a->capacity) return 0;                                      \
    int new_cap = (a->capacity == 0) ? 16 : a->capacity * 2;                  \
    ElemType *p = (ElemType *)realloc(a->items,                                \
                                      (size_t)new_cap * sizeof(ElemType));     \
    if (!p) {                                                                  \
        fprintf(stderr, "[darray] realloc failed: %d->%d\n",                   \
                a->capacity, new_cap);                                         \
        return -1;                                                             \
    }                                                                          \
    memset(&p[a->capacity], 0,                                                 \
           (size_t)(new_cap - a->capacity) * sizeof(ElemType));                \
    a->items = p;                                                              \
    a->capacity = new_cap;                                                     \
    return 0;                                                                  \
}                                                                              \
                                                                               \
/* 追加元素 (拷贝 *elem 到末尾) */                                                 \
static inline int TypeName##_push(TypeName *a, const ElemType *elem) {         \
    if (TypeName##_ensure(a) != 0) return -1;                                  \
    memcpy(&a->items[a->count], elem, sizeof(ElemType));                       \
    a->count++;                                                                \
    return 0;                                                                  \
}                                                                              \
                                                                               \
/* O(1) 随机访问, 越界返回 NULL */                                                 \
static inline ElemType* TypeName##_get(TypeName *a, int idx) {                 \
    if (idx < 0 || idx >= a->count) return NULL;                               \
    return &a->items[idx];                                                     \
}                                                                              \
                                                                               \
/* 删除指定索引, 后续元素前移 O(n). 不缩容. */                                       \
static inline int TypeName##_remove(TypeName *a, int idx) {                    \
    if (idx < 0 || idx >= a->count) return -1;                                 \
    if (idx < a->count - 1) {                                                  \
        memmove(&a->items[idx], &a->items[idx + 1],                            \
                (size_t)(a->count - idx - 1) * sizeof(ElemType));              \
    }                                                                          \
    a->count--;                                                                \
    memset(&a->items[a->count], 0, sizeof(ElemType));                          \
    return 0;                                                                  \
}                                                                              \
                                                                               \
/* O(1) 末尾删除 */                                                              \
static inline int TypeName##_pop(TypeName *a) {                                \
    if (a->count <= 0) return -1;                                              \
    a->count--;                                                                \
    memset(&a->items[a->count], 0, sizeof(ElemType));                          \
    return 0;                                                                  \
}                                                                              \
                                                                               \
/* 清空 (不释放内存, 下次 push 不需要 malloc) */                                     \
static inline void TypeName##_clear(TypeName *a) {                             \
    if (a->count > 0) {                                                        \
        memset(a->items, 0, (size_t)a->count * sizeof(ElemType));              \
    }                                                                          \
    a->count = 0;                                                              \
}                                                                              \
                                                                               \
/* 线性查找, 用自定义比较函数. 返回索引, -1=未找到 */                                   \
static inline int TypeName##_find(TypeName *a,                                 \
    int (*cmp)(const ElemType *a, const void *key), const void *key) {         \
    int i;                                                                     \
    for (i = 0; i < a->count; i++) {                                           \
        if (cmp(&a->items[i], key) == 0) return i;                             \
    }                                                                          \
    return -1;                                                                 \
}                                                                              \
                                                                               \
/* 排序, 用 qsort */                                                            \
static inline void TypeName##_sort(TypeName *a,                                \
    int (*cmp)(const void *, const void *)) {                                  \
    if (a->count > 1) {                                                        \
        qsort(a->items, (size_t)a->count, sizeof(ElemType), cmp);              \
    }                                                                          \
}

/*
 * 便捷宏 — 遍历
 *   DARRAY_FOREACH(arr, MusicInfo, item) {
 *       printf("%s\n", item->title);
 *   }
 */
#define DARRAY_FOREACH(arr, ElemType, var)                                      \
    for (ElemType *var = (arr).items,                                           \
         *_end_##var = (arr).items + (arr).count;                              \
         var < _end_##var; var++)

#endif /* DARRAY_H */
