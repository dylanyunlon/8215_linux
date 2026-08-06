#!/bin/sh

DEV=${1:-/dev/mtkd22}

echo "[E2FSCK] journal replay start: $DEV"

JOURNAL_OUT=$(/sbin/e2fsck -y -E journal_only "$DEV" 2>&1)
RET=$?

#
# NAND physical IO error
#
if echo "$JOURNAL_OUT" | grep -qiE \
    "read oob error|unable to read spare|I/O error|uncorrectable"
then
    echo "[E2FSCK] NAND physical error detected in journal replay"

    /usr/bin/storage_utils --device "$DEV" ext4 0

    exit 0
fi

echo "[E2FSCK] journal replay ret=$RET"

#
# e2fsck return:
# 0 clean
# 1 fixed
# 2 reboot needed
# 4 filesystem errors left uncorrected
# 8 operational error
#
if [ $RET -ge 2 ]; then
    echo "[E2FSCK] journal replay failed, start format"

    /usr/bin/storage_utils --device "$DEV" ext4 0

    if [ $? -ne 0 ]; then
        echo "[E2FSCK] format failed"
        exit 1
    fi

    exit 0
fi

echo "[E2FSCK] full fsck start"

FSCK_OUT=$(/sbin/e2fsck -y "$DEV" 2>&1)
RET=$?

#
# NAND physical IO error
#
if echo "$FSCK_OUT" | grep -qiE \
    "read oob error|unable to read spare|I/O error|uncorrectable"
then
    echo "[E2FSCK] NAND physical error detected in full fsck"

    /usr/bin/storage_utils --device "$DEV" ext4 0

    exit 0
fi

echo "[E2FSCK] full fsck ret=$RET"

#
# fatal fsck error
#
if [ $RET -ge 2 ]; then
    echo "[E2FSCK] fsck failed, start format"

    /usr/bin/storage_utils --device "$DEV" ext4 0

    if [ $? -ne 0 ]; then
        echo "[E2FSCK] format failed"
        exit 1
    fi

    exit 0
fi

echo "[E2FSCK] fsck success"

exit 0
