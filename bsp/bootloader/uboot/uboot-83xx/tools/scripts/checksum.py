#!/usr/bin/env python3
import argparse
import hashlib
import zlib
import sys
from pathlib import Path
import struct

BUF_SIZE = 1024 * 1024  # 1MB


def calc_crc32(path: Path) -> int:
    crc = 0
    with path.open("rb") as f:
        while True:
            data = f.read(BUF_SIZE)
            if not data:
                break
            crc = zlib.crc32(data, crc)
    return crc & 0xffffffff


def calc_hash(path: Path, algo: str) -> str:
    h = hashlib.new(algo)
    with path.open("rb") as f:
        while True:
            data = f.read(BUF_SIZE)
            if not data:
                break
            h.update(data)
    return h.hexdigest()

def calc_u32sum(path: Path) -> int:
    total = 0

    with path.open("rb") as f:
        while True:
            data = f.read(BUF_SIZE)
            if not data:
                break

            # pad to 4 bytes
            if len(data) % 4 != 0:
                data += b"\x00" * (4 - len(data) % 4)

            for i in range(0, len(data), 4):
                (val,) = struct.unpack_from("<I", data, i)
                total = (total + val) & 0xFFFFFFFF  # ignore overflow

    return total
    
def calc_u8sum(path: Path) -> int:
    total = 0

    with path.open("rb") as f:
        while True:
            data = f.read(BUF_SIZE)
            if not data:
                break

            for b in data:
                total = (total + b) & 0xFFFFFFFF

    return total


def main():
    parser = argparse.ArgumentParser(description="Binary checksum calculator")
    parser.add_argument("file", help="binary file path")
    parser.add_argument(
        "--algo",
        default="u8sum",
        choices=["crc32", "md5", "sha1", "sha256", "u32sum", "u8sum"],
        help="checksum algorithm",
    )
    args = parser.parse_args()

    path = Path(args.file)
    if not path.is_file():
        print(f"Error: {path} not found", file=sys.stderr)
        sys.exit(1)

    if args.algo == "crc32":
        value = calc_crc32(path)
        print(f"CRC32   : 0x{value:08X}")
    elif args.algo == "u8sum":
        value = calc_u8sum(path)
        print(f"U8SUM  : 0x{value:08X}")
    elif args.algo == "u32sum":
        value = calc_u32sum(path)
        print(f"U32SUM  : 0x{value:08X}")        
    else:
        value = calc_hash(path, args.algo)
        print(f"{args.algo.upper():7}: {value}")


if __name__ == "__main__":
    main()
