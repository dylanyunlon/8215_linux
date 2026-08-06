#include "storage_utils.h"

int main(int argc, char **argv) {
    int force = 0;
    int rc;
    if (argc == 4) {
        if (strcmp(argv[3], "true") == 0 || strcmp(argv[3], "1") == 0) {
            force = 1;
        } else if (strcmp(argv[3], "false") == 0 || strcmp(argv[3], "0") == 0) {
            force = 0;
        } else {
            fprintf(stderr, "invalid force argument: %s\n", argv[3]);
            return EINVAL;
        }
        rc = format_partition(argv[1], argv[2], force);
        if (rc != 0) {
            fprintf(stderr, "format_partition failed, errno=%d (%s)\n", rc, strerror(rc));
        }
        return rc;
    }

    if (argc == 5 && strcmp(argv[1], "--device") == 0) {
        if (strcmp(argv[4], "true") == 0 || strcmp(argv[4], "1") == 0) {
            force = 1;
        } else if (strcmp(argv[4], "false") == 0 || strcmp(argv[4], "0") == 0) {
            force = 0;
        } else {
            fprintf(stderr, "invalid force argument: %s\n", argv[4]);
            return EINVAL;
        }
        rc = format_partition2(argv[2], argv[3], force);
        if (rc != 0) {
            fprintf(stderr, "format_partition2 failed, errno=%d (%s)\n", rc, strerror(rc));
        }
        return rc;
    }

    fprintf(stderr,
                    "usage:\n"
                    "  %s <mount_path> <fs_type> <force(true|false|1|0)>\n"
                    "  %s --device <device_node> <fs_type> <force(true|false|1|0)>\n",
                    argv[0], argv[0]);
    return EINVAL;
}
