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


# === Custom ArgumentParser ===
class CustomArgumentParser(argparse.ArgumentParser):
    def error(self, message):
        sys.stderr.write(f"\n[ERROR] {message}\n")
        sys.stderr.write("请指定 --mode 参数，例如:\n")
        sys.stderr.write("  --mode copy_upg\n")
        sys.stderr.write("  --mode fastboot_upg\n\n")
        self.print_help()
        sys.exit(2)


# === Main Entry ===
def main():
    parser = CustomArgumentParser(description="auto upgrade")
    parser.add_argument("--device", choices=["emmc", "nand"], required=True,
                        help="device type: emmc / nand")
    parser.add_argument("--mode", choices=["copy_upg", "fastboot_upg"], required=True,
                        help="upgrade mode: copy_upg / fastboot_upg (must be specified)")
    args = parser.parse_args()

    setup_logging()

    if args.mode == "copy_upg":
        run_adb_command([ADB_PATH, "shell", "echo dword_write 0x10066 18 > /proc/mtz_debug"], True)
    elif args.mode == "fastboot_upg":
        run_adb_command([ADB_PATH, "shell", "echo dword_write 0x10066 17 > /proc/mtz_debug"], True)

    logging.info("Trigger upgrade ok")


if __name__ == "__main__":
    main()
