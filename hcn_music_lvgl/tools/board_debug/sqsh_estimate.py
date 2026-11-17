#!/usr/bin/env python3
"""估算一个目录做成 squashfs 之后的大小（没有 mksquashfs 时用）。
用法: python3 sqsh_estimate.py <目录> [块大小KB，默认128] [--top N] [--by-dir]
做法与 squashfs 一致：每个普通文件切成 128KB 一块，每块单独压缩，压不动的块原样存放。
输出 gzip(zlib -9) 与 xz 两种估算，外加元数据开销估计。多进程并行，只读，不改任何文件。"""
import os, sys, zlib, lzma, multiprocessing as mp
BS = 128 * 1024
def comp_file(args):
    path, bs = args
    try:
        raw = gz = xz = 0
        with open(path, 'rb') as f:
            while True:
                b = f.read(bs)
                if not b: break
                raw += len(b)
                gz += min(len(b), len(zlib.compress(b, 9)))
                xz += min(len(b), len(lzma.compress(b, format=lzma.FORMAT_XZ, preset=9 | lzma.PRESET_EXTREME)))
        return path, raw, gz, xz
    except Exception:
        return path, 0, 0, 0
def main():
    a = [x for x in sys.argv[1:] if not x.startswith('--')]
    root = a[0]; bs = (int(a[1]) if len(a) > 1 else 128) * 1024
    top = int(sys.argv[sys.argv.index('--top') + 1]) if '--top' in sys.argv else 15
    files = []; ndir = nlink = 0
    for dp, dn, fn in os.walk(root):
        ndir += 1
        for f in fn:
            p = os.path.join(dp, f)
            if os.path.islink(p): nlink += 1
            elif os.path.isfile(p): files.append((p, bs))
    n = min(32, os.cpu_count() or 4)
    with mp.Pool(n) as pool:
        res = pool.map(comp_file, files, chunksize=4)
    raw = sum(r[1] for r in res); gz = sum(r[2] for r in res); xz = sum(r[3] for r in res)
    meta = len(files) * 120 + ndir * 80 + nlink * 100           # inode + 目录项的粗略估计
    M = 1048576
    print(f"目录 {root}\n文件 {len(files)} 个，目录 {ndir} 个，符号链接 {nlink} 个，块大小 {bs//1024}KB")
    print(f"未压缩            : {raw/M:7.1f} MiB")
    print(f"squashfs(gzip)估算: {(gz+meta)/M:7.1f} MiB   (文件数据 {gz/M:.1f} + 元数据约 {meta/M:.1f})")
    print(f"squashfs(xz)  估算: {(xz+meta)/M:7.1f} MiB   (文件数据 {xz/M:.1f} + 元数据约 {meta/M:.1f})")
    print(f"\n压缩后占用最大的 {top} 个文件 (xz 口径):")
    for p, r, g, x in sorted(res, key=lambda t: -t[3])[:top]:
        print(f"  {x/1024:8.0f} KB (原 {r/1024:8.0f} KB)  {os.path.relpath(p, root)}")
    if '--by-dir' in sys.argv:
        d = {}
        for p, r, g, x in res:
            k = os.sep.join(os.path.relpath(p, root).split(os.sep)[:2]); d[k] = d.get(k, 0) + x
        print("\n按目录(前两级)汇总 xz 口径:")
        for k, v in sorted(d.items(), key=lambda t: -t[1])[:12]: print(f"  {v/M:6.1f} MiB  {k}")
if __name__ == '__main__': main()
