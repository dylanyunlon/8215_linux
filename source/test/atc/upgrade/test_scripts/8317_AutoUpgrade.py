#!/usr/bin/env python3

import subprocess
import xml.etree.ElementTree as ET
from pathlib import Path
import argparse
import sys
import logging
import time
import struct
import shutil
import os
import re
from datetime import datetime

# === Constants ===
ADB_PATH = "adb"


# === Logging Setup ===
def setup_logging():
    log_dir = "log"
    os.makedirs(log_dir, exist_ok=True)
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    log_file = os.path.join(log_dir, f"upgrade_log_{timestamp}.txt")
    logging.basicConfig(
        level=logging.DEBUG,
        format='[%(asctime)s] %(message)s',
        handlers=[
            logging.FileHandler(log_file, encoding='utf-8'),
            logging.StreamHandler(sys.stdout)
        ]
    )
    logging.info(f"log print to {log_file}")


# === Utility ===
def check_tool_exists(tool):
    if shutil.which(tool) is None:
        logging.error(f"{tool} is not in path")
        logging.error("Download fail.")
        sys.exit(1)


def run_adb_command(cmd, debug=True):
    if debug:
        logging.info(f"[DEBUG] adb cmd : {' '.join(cmd)}")
    try:
        result = subprocess.run(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            encoding='utf-8'
        )
    except Exception as e:
        logging.error(f"[ERROR] cmd exec failed: {e}")
        return subprocess.CompletedProcess(cmd, 1, '', str(e))

    stdout = result.stdout.strip() if result.stdout else "<No Output>"
    stderr = result.stderr.strip() if result.stderr else "<No Err>"
    if debug:
        logging.info(f"[DEBUG] stdout: {stdout}")
        logging.info(f"[DEBUG] stderr: {stderr}")
    return result


# === ADB operations ===
def adb_pull(remote, local):
    logging.info(f"[ADB] Pull {remote} -> {local}")
    try:
        subprocess.run([ADB_PATH, "pull", remote, str(local)], check=True)
    except subprocess.CalledProcessError as e:
        logging.error(f"[ERROR] adb pull failed: {e}")
        logging.error("Download fail.")
        sys.exit(1)


# === Push all partitions ===
def push_all_partitions_images(img_dir, device_type, debug=True):
    """
    parse scatter file, push image files to /media/ext_sdcard2
    """
    if device_type == "emmc":
        scatter_file = img_dir / "scatter.mmcboot.ext4.xml"
    elif device_type == "nand":
        scatter_file = img_dir / "scatter.nand.ext4.xml"
    else:
        logging.error(f"[ERROR] Unknown device type : {device_type}")
        logging.error("Download fail.")
        sys.exit(1)

    if not scatter_file.exists():
        logging.error(f"[ERROR] scatter file not exist: {scatter_file}")
        logging.error("Download fail.")
        sys.exit(1)

    logging.info(f"[INFO] Using scatter file : {scatter_file}")
    tree = ET.parse(scatter_file)
    root = tree.getroot()

    # push files set
    pushed_files = set()

    for part in root.findall("partition"):
        name = part.get("name")
        imagename = part.get("imagename", "")
        part_type = part.get("type")

        if not imagename or part_type not in ("raw", "ext4"):
            continue

        if imagename in pushed_files:
            logging.info(f"[SKIP] {imagename} has pushed, skip (partition={name})")
            continue

        img_path = Path(img_dir) / imagename
        if not img_path.exists():
            logging.error(f"img no exsits: {img_path}, skip {name}")
            continue

        target_path = f"/media/ext_sdcard2/{imagename}"
        logging.info(f"[ADB] Pushing {imagename} -> {target_path}")
        result = run_adb_command([ADB_PATH, "push", str(img_path), target_path], debug=debug)

        if result.returncode != 0:
            logging.error(f"adb push {imagename} fail: {result.stderr.strip()}")
            raise RuntimeError(f"adb push {imagename} fail")
        else:
            logging.info(f"adb push {imagename} success")
            pushed_files.add(imagename)

    logging.info("[ADB] all image file push to /media/ext_sdcard2")

    # list all pushed files
    if pushed_files:
        logging.info("[SUMMARY] pushed file lists: ")
        for fname in sorted(pushed_files):
            logging.info(f"    - {fname}")
    else:
        logging.warning("[SUMMARY] no file pushed")


# === Main Entry ===
def main():
    parser = argparse.ArgumentParser(description="auto upgrade")
    parser.add_argument("--img", type=Path, required=True, help="image path")
    parser.add_argument("--device", choices=["emmc", "nand"], required=True, help="device type:emmc/nand")
    args = parser.parse_args()

    setup_logging()
    check_tool_exists(ADB_PATH)

    logging.info("\n=== Step 0: adb wait-for-device ===")
    run_adb_command([ADB_PATH, "wait-for-device"], True)    

    logging.info("\n=== Step 1: reboot platform for upgrade test ===")
    run_adb_command([ADB_PATH, "shell", "reboot"], True)

    logging.info("\n=== Step 2: wait for devie and storage ready ===")
    run_adb_command([ADB_PATH, "wait-for-device"], True)    

    time.sleep(20)

    logging.info("\n=== Step 3: Push all partition images ===")
    push_all_partitions_images(args.img, args.device, True)
    
    logging.info("\n=== Step 4: ready to write upgrade mode flag ===")
    run_adb_command([ADB_PATH, "shell", "echo dword_write 0x10066 18 > /proc/mtz_debug"], True)

    run_adb_command([ADB_PATH, "shell", "echo dword_write 0x10060 20 > /proc/mtz_debug"], True)
    time.sleep(1)

    logging.info("\n=== Step 5: Reboot device ===")
    run_adb_command([ADB_PATH, "shell", "reboot"], True)

    time.sleep(60)

    logging.info("\n=== Step 6: Wait for platform bootup ===")
    run_adb_command([ADB_PATH, "wait-for-device"], True)

    logging.info("\n=== Step 7: Check metazone upgrade flag ===")

    # 执行echo + cat组合命令，读取/解析结果
    result = run_adb_command([
        ADB_PATH, "shell",
        "echo dword_read 0x10060 > /proc/mtz_debug && cat /proc/mtz_debug"
    ], True)

    if result.returncode != 0:
        logging.error("Failed to read /proc/mtz_debug")
        sys.exit(1)

    output = result.stdout.strip()
    logging.info(f"[INFO] mtz_debug output:\n{output}")

    match = re.search(r"Dword data\s*:(\d+)", output)
    if not match:
        logging.error("[ERROR] Can't parse dword data from /proc/mtz_debug output.")
        sys.exit(1)

    value = int(match.group(1))
    if value == 0:
        logging.info("[PASS] Upgrade success, Dword data = 0")
    elif value == 20:
        logging.error("[FAIL] Upgrade failed, Dword data = 20")
        sys.exit(1)
    else:
        logging.warning(f"Unexpected Dword data value: {value}")
        sys.exit(1)

    logging.info("Download Success")

if __name__ == "__main__":
    main()
