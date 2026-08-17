/**
 * test_scanner.c - 独立测试: 扫描指定目录打印音乐文件信息
 *
 * 编译: gcc -o test_scanner test_scanner.c source/packages/application/musicplayer/music_scanner.c \
 *       -I source/packages/application/musicplayer -lpthread
 *
 * 用法: ./test_scanner /media/tanyunlong/USB盘名
 *       ./test_scanner /mnt/usb0
 *       ./test_scanner .    (扫描当前目录)
 */

#include "music_scanner.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

int main(int argc, char *argv[])
{
    const char *scan_path = ".";
    if (argc > 1) {
        scan_path = argv[1];
    }

    printf("========================================\n");
    printf("  本地音乐扫描测试工具\n");
    printf("========================================\n");
    printf("扫描路径: %s\n", scan_path);
    printf("最大文件数: %d\n", MUSIC_MAX_FILES);
    printf("\n");

    /* 创建列表 */
    MusicList *list = music_list_create(MUSIC_MAX_FILES);
    if (!list) {
        fprintf(stderr, "错误: 无法分配内存\n");
        return 1;
    }

    /* 计时 */
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    /* 执行扫描 */
    printf("正在扫描...\n\n");
    int ret = music_scan_directory(list, scan_path);

    clock_gettime(CLOCK_MONOTONIC, &t1);
    double elapsed = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;

    if (ret < 0) {
        fprintf(stderr, "错误: 扫描失败 (ret=%d)\n", ret);
        music_list_destroy(list);
        return 1;
    }

    /* 打印结果 */
    printf("%-4s  %-40s  %-20s  %-20s  %-15s  %s\n",
           "序号", "文件名", "标题(ID3)", "艺术家", "专辑", "路径");
    printf("----  %-40s  %-20s  %-20s  %-15s  %s\n",
           "----------------------------------------",
           "--------------------",
           "--------------------",
           "---------------",
           "----");

    int i;
    int has_id3 = 0;
    int no_id3 = 0;
    int has_apic = 0;

    for (i = 0; i < list->count; i++) {
        const MusicInfo *info = &list->items[i];

        /* 统计ID3 */
        if (info->title[0] != '\0' && strcmp(info->title, info->filename) != 0) {
            has_id3++;
        } else {
            no_id3++;
        }

        /* 检查是否有APIC封面 */
        {
            FILE *fp = fopen(info->filepath, "rb");
            if (fp) {
                unsigned char hdr[10];
                int has_cover = 0;
                if (fread(hdr, 1, 10, fp) == 10 &&
                    hdr[0] == 'I' && hdr[1] == 'D' && hdr[2] == '3') {
                    /* Quick scan for "APIC" in first 64KB */
                    unsigned char buf[65536];
                    size_t n = fread(buf, 1, sizeof(buf), fp);
                    size_t j;
                    for (j = 0; j + 3 < n; j++) {
                        if (buf[j]=='A' && buf[j+1]=='P' &&
                            buf[j+2]=='I' && buf[j+3]=='C') {
                            has_cover = 1;
                            has_apic++;
                            break;
                        }
                    }
                }
                fclose(fp);

                /* 截断显示 */
                char fn[41], ti[21], ar[21], al[16];
                snprintf(fn, sizeof(fn), "%s", info->filename);
                snprintf(ti, sizeof(ti), "%s", info->title[0] ? info->title : "--");
                snprintf(ar, sizeof(ar), "%s", info->artist[0] ? info->artist : "--");
                snprintf(al, sizeof(al), "%s", info->album[0] ? info->album : "--");

                printf("%-4d  %-40s  %-20s  %-20s  %-15s  %s  %s\n",
                       i + 1, fn, ti, ar, al,
                       has_cover ? "[ART]" : "     ",
                       info->filepath);
            }
        }

        /* Check for .lrc file */
        {
            char lrc_path[512];
            snprintf(lrc_path, sizeof(lrc_path), "%s", info->filepath);
            char *dot = strrchr(lrc_path, '.');
            if (dot) strcpy(dot, ".lrc");
            FILE *lrc_fp = fopen(lrc_path, "r");
            if (lrc_fp) {
                fclose(lrc_fp);
                printf("      ^ LRC歌词文件存在: %s\n", lrc_path);
            }
        }
    }

    /* 汇总 */
    printf("\n========================================\n");
    printf("  扫描结果汇总\n");
    printf("========================================\n");
    printf("扫描路径:     %s\n", scan_path);
    printf("发现音频文件: %d 个\n", list->count);
    printf("有ID3标签:    %d 个\n", has_id3);
    printf("无ID3标签:    %d 个\n", no_id3);
    printf("有专辑封面:   %d 个\n", has_apic);
    printf("扫描耗时:     %.3f 秒\n", elapsed);
    printf("========================================\n");

    music_list_destroy(list);
    return 0;
}
