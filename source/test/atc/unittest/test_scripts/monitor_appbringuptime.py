import re
import sys
import subprocess
import serial
import time
import os
from datetime import datetime
import json
import threading
import logging
import xml.etree.ElementTree as ET
sys.path.append('libs/')
sys.path.append('./')

sys.path.append(os.getcwd())
from data.perf_data.performance_common import save_perf_json

def get_bootprof_via_adb():
    try:
        result = subprocess.check_output(['adb', 'shell', 'cat', '/proc/bootprof'], encoding='utf-8')
        return result
    except subprocess.CalledProcessError as e:
        print("ADB 命令执行失败:", e)
        return None

def extract_time(label, bootprof_data):
    pattern = rf"^\s*(\d+)\s*:\s*{re.escape(label)}\s*$"
    for line in bootprof_data.splitlines():
        match = re.match(pattern, line)
        if match:
            return int(match.group(1))
    return None

def compare_times(reference_time):
    bootprof = get_bootprof_via_adb()
    if not bootprof:
        print("无法获取 bootprof 数据")
        return

    target_label = "first frame pan display"
    boot_time = extract_time(target_label, bootprof)

    if boot_time is None:
        print(f"未找到标签: {target_label}")
        return

    delta = boot_time - reference_time
    metrics_data = {
        "APPBringUpTime": boot_time
    }
    save_perf_json("monitor_appbringuptime", metrics_data)

    print(f"{target_label} 时间戳: {boot_time} ms")
    print(f"参考时间: {reference_time} ms")
    print(f"差值: {delta} ms")
    if boot_time > reference_time:
        logging.info(f'test fail')
        print("+++++++++++++Fail++++++++++++")
    else:
        print("+++++++++++++PASS++++++++++++")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("用法: python script.py <参考时间ms>")
        sys.exit(1)

    try:
        ref_time = int(sys.argv[1])
    except ValueError:
        print("错误：参考时间必须是整数（单位：毫秒）")
        sys.exit(1)

    compare_times(ref_time)
