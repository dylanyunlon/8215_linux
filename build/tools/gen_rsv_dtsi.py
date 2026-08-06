import csv
import os
import argparse

def parse_bool(s):
    return s.strip().lower() in ["yes", "no-map"]

def parse_int(s):
    try:
        return int(s.strip(), 0)
    except:
        return 0

def align_down(addr, align):
    if align == 0:
        return addr
    return addr & ~(align - 1)

def align_up(addr, align):
    if align == 0:
        return addr
    return (addr + align - 1) & ~(align - 1)

def generate_reserved_nodes_split(top_addr, nodes, forward_base=0x0, forward_limit=0x1000000):
    forward_nodes = [n for n in nodes if n.get("addr_flag") == 1]
    backward_nodes = [n for n in nodes if n.get("addr_flag") != 1]

    # 前向分配：从 forward_base 向上
    forward_addr = forward_base
    for node in forward_nodes:
        size = node["size"]
        align = node["alignment"]
        aligned_addr = align_up(forward_addr, align)
        node["base"] = aligned_addr
        forward_addr = aligned_addr + size

    if forward_addr > forward_limit:
        raise ValueError(
            f"[X] Error: addr_flag=1 的保留区总分配超过限制: 0x{forward_addr:x} > 0x{forward_limit:x}"
        )

    # 后向分配：从 top_addr 向下
    current = top_addr
    for node in backward_nodes:
        size = node["size"]
        align = node["alignment"]
        aligned_addr = align_down(current - size, align)
        node["base"] = aligned_addr
        current = aligned_addr

    return sorted(forward_nodes + backward_nodes, key=lambda x: x["base"])

def format_dts_node(node):
    map_flag = "            no-map;\n" if node["no-map"] else ""
    alignment_line = f"            alignment = <0x{node['alignment']:x}>;\n" if node["alignment"] else ""
    return f"""        {node['label']}: {node['name']}@0x{node['base']:x} {{
            compatible = "{node['compatible']}";
            reg = <0x{node['base']:x} 0x{node['size']:x}>;
{map_flag}{alignment_line}        }};
"""

def read_csv(filename, ddr_bit):
    nodes = []
    with open(filename, newline='') as csvfile:
        reader = csv.DictReader(csvfile)
        for idx, row in enumerate(reader, start=1):
            config_raw = row.get("Config", "0")
            config_str = str(config_raw).strip()
            try:
                config_val = int(config_str, 0)
            except ValueError:
                print(f"[!] Warning: Invalid Config in row {idx}: '{config_str}', skipping")
                continue

            # 根据 DDR bit 过滤不适用的节点
            if ((config_val >> ddr_bit) & 0x1) == 0:
                print(f"[i] Skipping '{row['Label'].strip()}' (DDR size incompatible)")
                continue

            size_str = row["Size(B)"].strip()
            if not size_str.isdigit():
                print(f"[!] Warning: Skipping row {idx} with invalid Size(B): '{size_str}'")
                continue

            nodes.append({
                "label": row["Label"].strip(),
                "name": row["Node Name"].strip(),
                "size": int(size_str),
                "compatible": row["Compatible"].strip().replace('"', ''),
                "no-map": parse_bool(row["Mapped"]),
                "alignment": parse_int(row["alignment (B)"]),
                "addr_flag": parse_int(row.get("addr_flag", "0")),
            })
    return nodes

def generate_dts_content(nodes):
    dts = "/ {\n"
    dts += '    compatible = "Autochips,ac83xx";\n'
    dts += "    reserved-memory {\n"
    dts += "        #address-cells = <1>;\n"
    dts += "        #size-cells = <1>;\n"
    dts += "        ranges;\n\n"
    for node in nodes:
        dts += format_dts_node(node)
    dts += "    };\n"
    dts += "};\n"
    return dts

def main():
    parser = argparse.ArgumentParser(description="Generate reserved-memory DTS from CSV.")
    parser.add_argument("--in", dest="input_csv", required=True, help="Input CSV file path")
    parser.add_argument("--out", dest="output_dtsi", required=True, help="Output DTSI file path")
    parser.add_argument("--top", dest="top_addr", default="0x8000000", help="Top address (e.g. 0x8000000)")

    args = parser.parse_args()

    try:
        top_addr = int(args.top_addr, 0)
    except ValueError:
        print(f"[X] Invalid top address: {args.top_addr}")
        return

    # 将 top_addr 转为 DDR bit index
    ddr_size_mb = top_addr >> 20
    ddr_bit_map = {
        128: 0,
        256: 1,
        512: 2,
        1024: 3,
        2048: 4,
        4096: 5,
        8192: 6,
        16384: 7,
    }

    if ddr_size_mb not in ddr_bit_map:
        print(f"[X] Unsupported DDR size: {ddr_size_mb} MB")
        return

    ddr_bit = ddr_bit_map[ddr_size_mb]

    out_dir = os.path.dirname(args.output_dtsi)
    if out_dir:
        os.makedirs(out_dir, exist_ok=True)

    nodes = read_csv(args.input_csv, ddr_bit)
    if not nodes:
        print("[!] No valid reserved-memory nodes after filtering.")
        return

    reserved = generate_reserved_nodes_split(top_addr, nodes)
    dts_content = generate_dts_content(reserved)

    with open(args.output_dtsi, "w") as f:
        f.write(dts_content)

    print(f"[✓] DTS written to: {args.output_dtsi}")

if __name__ == "__main__":
    main()
