# -*- coding: utf-8 -*-  # 在文件的开头添加这行

import re
import sys
import os

DEFAULT_MAX_END_ADDR = 0x80000000
ALIGN_SIZE_32K = 0x8000  # 32KB 对齐
ALIGN_SIZE_16M = 0x1000000  # 16MB 对齐

def extract_reserved_memory_block(dts_text):
    match = re.search(r'reserved-memory\s*\{', dts_text)
    if not match:
        return None

    start = match.end()
    brace_level = 1
    index = start

    while index < len(dts_text):
        if dts_text[index] == '{':
            brace_level += 1
        elif dts_text[index] == '}':
            brace_level -= 1
            if brace_level == 0:
                return dts_text[match.start():index + 1]
        index += 1

    return None

def remove_comments(text):
    text = re.sub(r'/\*.*?\*/', '', text, flags=re.DOTALL)
    text = re.sub(r'//.*', '', text)
    return text

def parse_reserved_entries(block):
    return re.findall(
        r'(\w+)\s*:\s*([\w@]+)\s*\{[^}]*?reg\s*=\s*<\s*(0x[0-9a-fA-F]+)\s+(0x[0-9a-fA-F]+)\s*>;',
        block,
        re.DOTALL
    )

def align_size(address, align_size):
    """按 2MB 对齐"""
    if address % align_size == 0:
        return address
    return ((address // align_size) + 1) * align_size

def parse_dts(dts_path, out_dir=".", filename="rsv.mk", max_end_addr=DEFAULT_MAX_END_ADDR):
    if not os.path.isfile(dts_path):
        print(f"❌ 找不到文件: {dts_path}")
        return

    with open(dts_path, "r", encoding="utf-8") as f:  # 使用 UTF-8 编码读取文件
        dts_text = f.read()

    reserved_block = extract_reserved_memory_block(dts_text)
    if not reserved_block:
        print("❌ 未找到 reserved-memory 区块")
        return

    clean_block = remove_comments(reserved_block)
    entries = parse_reserved_entries(clean_block)
    if not entries:
        print("⚠️ 没有找到任何有效 reg 节点")
        return

    last_addr = int(entries[-1][2], 16)
    last_size = int(entries[-1][3], 16)
   # last_end = last_addr + last_size
#    print(f"last_end is 0x{last_end:X}")
   # last_end = align_size(last_end, ALIGN_SIZE_32K)
  #  kernel_start = align_size(last_end, ALIGN_SIZE_16M)
  #  kernel_start += 0x8000
#    print(f"after align last_end is 0x{last_end:X},kernel_start is 0x{kernel_start:X}")
 #   print(f"max_end_addr is 0x{max_end_addr:X}")
#    if kernel_start >= max_end_addr:
 #       print(f"❌ 错误: 最后一个 reg 范围超出限制")
 #       print(f"   addr: 0x{last_addr:X}, size: 0x{last_size:X}, 结束地址: 0x{last_end:X}")
 #       print(f"   限制最大地址: 0x{max_end_addr:X}")
 #       return

    # 确保输出目录存在
    os.makedirs(out_dir, exist_ok=True)
    output_path = os.path.join(out_dir, filename)

    with open(output_path, "w", encoding="utf-8") as out:  # 使用 UTF-8 编码写入文件
        for label, name, addr, size in entries:
            macro_name = f"CFG_{label.upper()}"
            out.write(f"{macro_name}_ADDR := {addr}\n")
            out.write(f"{macro_name}_SIZE := {size}\n\n")
    #    out.write(f"CONFIG_RSV_END := 0x{last_end:X}\n")
    #    out.write(f"CONFIG_KERNEL_START := 0x{kernel_start:X}\n")

#    print(f"✅ 已生成: {output_path}")
#    print(f"✅ CONFIG_RSV_END := 0x{last_end:X}")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("用法: python parse_rsv_dts.py <your.dts> [output_dir] [max_size]")
        sys.exit(1)

    dts_path = sys.argv[1]
    out_dir = sys.argv[2] if len(sys.argv) > 2 else "."
    max_end_addr = int(sys.argv[3], 16) if len(sys.argv) > 3 else DEFAULT_MAX_END_ADDR
    parse_dts(dts_path, out_dir, max_end_addr=max_end_addr)
