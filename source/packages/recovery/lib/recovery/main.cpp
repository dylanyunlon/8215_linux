// main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "atc_emptycore_upgrade.h"  // 包含函数声明

int main(int argc, char *argv[]) {
    // 创建并初始化 RecoveryUpdateModule 结构体实例
    RecoveryUpdateModule rum = {0};  // 使用零初始化作为默认值

    // 根据需要设置 rum 的成员变量
    // 示例：rum.mMcu = 1; // 如果需要升级 MCU
    // 示例：rum.mNavi = 1; // 如果需要升级导航
    // 示例：rum.mFromudisk = 1; // 如果从U盘升级

    // 调用 export_emptycore_upgrade 函数
    int result = export_emptycore_upgrade(&rum);

    if (result < 0) {
        printf("export_emptycore_upgrade failed with error code: %d\n", result);
        return EXIT_FAILURE;
    } else {
        printf("export_emptycore_upgrade succeeded.\n");
        return EXIT_SUCCESS;
    }
}