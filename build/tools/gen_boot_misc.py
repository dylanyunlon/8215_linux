import re
import struct
import argparse

class RSVMemory:
    def __init__(self, name, start_addr, size):
        self.name = name.encode('ascii')[:24].ljust(24, b'\x00')
        self.start_addr = int(start_addr)
        self.size = int(size)

    def pack(self):
        packed = struct.pack('<24sQQ', self.name, self.start_addr, self.size)
        name_str = self.name.split(b'\x00', 1)[0].decode()
        print(f"[打包] name={name_str}, addr=0x{self.start_addr:X}, size=0x{self.size:X} => packed={packed.hex()}")
        return packed

    def checksum(self):
        return (
            sum(self.name) +
            (self.start_addr & 0xFFFFFFFF) + (self.start_addr >> 32) +
            (self.size & 0xFFFFFFFF) + (self.size >> 32)
        )

def parse_rsv_dtsi(path):
    with open(path, "r") as f:
        text = f.read()

    pattern = re.compile(
        r'\w+\s*:\s*(\w+)@0x[0-9a-fA-F]+\s*\{[^}]*?reg\s*=\s*<\s*0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)\s*>;',
        re.DOTALL
    )

    rsv_list = []
    for match in pattern.finditer(text):
        node_name, reg_addr, reg_size = match.groups()
        start = int(reg_addr, 16)
        size = int(reg_size, 16)

        print(f"[解析] name={node_name}, addr=0x{start:X}, size=0x{size:X}")
        rsv_list.append(RSVMemory(node_name, start, size))

    return rsv_list

def write_binary_with_header(rsv_list, output_path):
    rsv_mem_num = len(rsv_list)
    checksum = sum(r.checksum() for r in rsv_list) & 0xFFFFFFFF

    header = struct.pack(
        '<4sIIi',
        b'RSV\x00',
        rsv_mem_num,
        checksum & 0xFFFFFFFF,
        0
    )

    with open(output_path, "wb") as f:
        f.write(header)
        for r in rsv_list:
            f.write(r.pack())

    print(f"[✔] 写入 {output_path} 成功，共 {rsv_mem_num} 个结构体。")

def debug_read_binary(output_path):
    print(f"\n[🔍] 正在反解析 {output_path} 内容...\n")
    with open(output_path, 'rb') as f:
        data = f.read()

    header = struct.unpack('<4siii', data[:16])
    magic, count, checksum, reserved = header
    print(f"[Header] magic={magic}, count={count}, checksum=0x{checksum:X}, reserved={reserved}")

    entry_size = struct.calcsize('<24sQQ')
    offset = 16
    for i in range(count):
        chunk = data[offset:offset+entry_size]
        name_bytes, addr, size = struct.unpack('<24sQQ', chunk)
        name_str = name_bytes.split(b'\x00', 1)[0].decode()
        print(f"[Entry {i+1}] name={name_str}, addr=0x{addr:X}, size=0x{size:X}")
        offset += entry_size

def main():
    parser = argparse.ArgumentParser(description="从 rsv.dtsi 生成 boot_misc.img 并校验")
    parser.add_argument("-i", "--input", required=True, help="输入 rsv.dtsi 文件路径")
    parser.add_argument("-o", "--output", required=True, help="输出 boot_misc.img 文件路径")
    args = parser.parse_args()

    rsv_mems = parse_rsv_dtsi(args.input)
    write_binary_with_header(rsv_mems, args.output)
    debug_read_binary(args.output)

if __name__ == "__main__":
    main()
