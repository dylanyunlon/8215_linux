#!/usr/bin/env python3
import sys
import subprocess
import struct
from collections import OrderedDict

# ---------------- 配置 ----------------

CODE_TAGS = set("TtWwNn")
MAGIC = 0x46554E43  # 'FUNC'
VERSION = 1
FLAG_INLINE = 1 << 0

# ---------------- System.map 解析 ----------------

def parse_system_map(path):
    """
    返回:
      addrs:   所有代码符号地址（排序、去重）
      symbols: 所有真实函数名集合（用于 inline 判定）
    """
    addrs = []
    symbols = set()

    with open(path) as f:
        for line in f:
            parts = line.strip().split()
            if len(parts) < 3:
                continue
            addr, tag, name = parts[0], parts[1], parts[2]
            if tag in CODE_TAGS:
                try:
                    addrs.append(int(addr, 16))
                    symbols.add(name)
                except ValueError:
                    pass

    return sorted(set(addrs)), symbols

# ---------------- 批量 addr2line ----------------

def batch_addr2line(addr2line_bin, elf, probes):
    """
    probes: list of probe addresses
    返回: list of function name chains

    说明:
    - GNU addr2line 不提供地址间的明确分隔符，批量解析时需要人为插入哨兵地址，
      否则无法可靠地区分不同地址的输出，导致所有符号被拼到第一个地址上。
    - 这里在每个真实地址后追加一个 0 哨兵（会返回 ??/??:0），
      以此作为该地址函数链的终止标记。
    """
    if not probes:
        return []

    # 构造带哨兵的地址序列： [addr0, 0, addr1, 0, ...]
    SENTINEL = 0
    batched = []
    for a in probes:
        batched.append(a)
        batched.append(SENTINEL)

    cmd = [addr2line_bin, "-f", "-C", "-i", "-s", "-e", elf]
    try:
        proc = subprocess.run(
            cmd,
            input="\n".join(hex(a) for a in batched) + "\n",
            capture_output=True,
            text=True,
            timeout=120,
            check=False,
        )
    except subprocess.TimeoutExpired:
        raise SystemExit("addr2line execution timed out. Try smaller inputs or check elf/symbols.")

    out = proc.stdout.splitlines()

    chains = []
    idx = 0

    for _ in probes:
        chain = []
        # 逐对读取：name, loc
        while idx + 1 < len(out):
            name = out[idx].strip()
            loc = out[idx + 1].strip()
            idx += 2

            # 哨兵命中（?? / ??:0），结束当前地址链
            if name == "??" and loc.startswith("??"):
                break

            if name and name != "??":
                chain.append(name)

        chains.append(chain)

    return chains

# ---------------- 主流程 ----------------

def main():
    if len(sys.argv) != 6:
        print(f"Usage: {sys.argv[0]} <System.map> <addr2line> <elf> <out.bin> <out.txt>")
        sys.exit(1)

    system_map, addr2line_bin, elf, out_bin, out_txt = sys.argv[1:]

    print("[*] Parsing System.map...")
    addrs, real_funcs = parse_system_map(system_map)

    if len(addrs) < 2:
        print("Not enough symbols")
        sys.exit(1)

    # 构造地址区间
    ranges = [(addrs[i], addrs[i+1]) for i in range(len(addrs)-1)]

    # 构造探针：按每 4 字节递增采样，但保证每个区间至少 1 个探针
    STEP = 4
    probes = []                 # 所有探针地址（全局扁平）
    probe_endcaps = []          # 与探针一一对应的“符号区间结束地址”上界（半开区间）
    for start, end in ranges:
        addrs = list(range(start, end, STEP))
        if not addrs:
            addrs = [start]
        probes.extend(addrs)
        probe_endcaps.extend([end] * len(addrs))

    print(f"[*] Resolving {len(probes)} probes via addr2line...")
    chains_all = batch_addr2line(addr2line_bin, elf, probes)

    # 将探针结果压缩为“同链连续地址段”的稠密区间
    dense_ranges = []  # (start, end, chain)
    if probes:
        cur_start = probes[0]
        cur_chain = chains_all[0]
        cur_cap = probe_endcaps[0]
        for i in range(1, len(probes)):
            addr = probes[i]
            chain = chains_all[i]
            cap = probe_endcaps[i]
            prev_addr = probes[i-1]
            contiguous = (addr == prev_addr + STEP)
            same_chain = (chain == cur_chain)
            same_cap = (cap == cur_cap)
            if contiguous and same_chain and same_cap:
                continue
            # 关闭前一段，结束位置不能超过其符号区间上界，也不能超过当前地址
            end_pos = min(cur_cap, addr)
            if cur_chain:
                dense_ranges.append((cur_start, end_pos, cur_chain))
            # 开启新段
            cur_start = addr
            cur_chain = chain
            cur_cap = cap
        # 收尾：最后一段到 min(cap, last_addr+STEP)
        last_end = min(cur_cap, probes[-1] + STEP)
        if cur_chain:
            dense_ranges.append((cur_start, last_end, cur_chain))

    entries = []
    strings = OrderedDict()

    def str_offset(s):
        if s not in strings:
            strings[s] = sum(len(k) + 1 for k in strings)
        return strings[s]

    # 生成 func_info 表
    for start, end, chain in dense_ranges:
        for name in chain:
            flags = FLAG_INLINE if name not in real_funcs else 0
            entries.append((
                str_offset(name),
                start,
                end,
                flags
            ))

    # ---------------- 写 binary ----------------

    with open(out_bin, "wb") as f:
        header = struct.pack(
            "<IHHI",
            MAGIC,
            VERSION,
            len(entries),
            sum(len(s) + 1 for s in strings)
        )
        f.write(header)

        # func_info: name_off, start, end, flags
        for name_off, start, end, flags in entries:
            f.write(struct.pack("<IIIb", name_off, start, end, flags))

        for s in strings:
            f.write(s.encode("utf-8") + b"\0")

    # ---------------- 写 txt ----------------

    with open(out_txt, "w") as f:
        f.write("# Functions (address ranges)\n")
        for name_off, start, end, flags in entries:
            name = list(strings.keys())[list(strings.values()).index(name_off)]
            tag = " [inline]" if flags & FLAG_INLINE else ""
            f.write(f"0x{start:08x} - 0x{end:08x}  {name}{tag}\n")

    print(f"[+] Generated:")
    print(f"    {out_bin}")
    print(f"    {out_txt}")

# ---------------- 入口 ----------------

if __name__ == "__main__":
    main()
