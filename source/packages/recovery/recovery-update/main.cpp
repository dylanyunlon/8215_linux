// main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "atc_safe_upgrade.h"

int main() {
    int result =  export_safeupg_upgrade();

    if (result < 0) {
        printf("export_emptycore_upgrade failed with error code: %d\n", result);
        return EXIT_FAILURE;
    } else {
        printf("export_emptycore_upgrade succeeded.\n");
        return EXIT_SUCCESS;
    }
}