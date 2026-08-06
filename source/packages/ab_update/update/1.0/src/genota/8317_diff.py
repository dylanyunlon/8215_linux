#!/usr/bin/env python3
import os
import hashlib
import json
import subprocess
import sys
import shutil
from pathlib import Path

def calculate_md5(file_path):
    md5 = hashlib.md5()
    count = 0
    with open(file_path, "rb") as file:
        while True:
            data = file.read(512)
            if not data:
                break
            if len(data) < 512:
                data +=b'\x00'*(512 -len(data))
            count=count+1
            md5.update(data)
    md5_hex = md5.hexdigest()
    return md5_hex


def run_bsdiff(old_file, new_file, patch_file):
    current_directory = sys.path[0]
    bsdiff_path = os.path.join(current_directory, "bsdiff")
    cmd = "chmod 777 {default_path}/bsdiff".format(default_path=current_directory)
    try:
        subprocess.run(
            [bsdiff_path, old_file, new_file, patch_file],
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        return True
    except subprocess.CalledProcessError as e:
        print(f"bsdiff failed for {old_file} -> {new_file}: {e.stderr.decode()}")
        return False
    except Exception as e:
        print(f"Error running bsdiff: {str(e)}")
        return False

def save_version_info(dir3, pre_version, post_version):
    version_content = f"""pre-version:{pre_version}
post-verison:{post_version}
package:diff"""

    with open (os.path.join(dir3, "version"), 'w') as f:
        f.write(version_content)

def getImgesize(imge_floder, out_file):
    with open(out_file, 'w') as f:
        for root, dirs, files in os.walk(imge_floder):
            for file_name in files:
                file_path = os.path.join(root, file_name)
                file_size = os.path.getsize(file_path)
                f.write(f"{file_name}: {file_size}\n")

def process_files(dir1, dir2, dir3):
    Path(dir3).mkdir(parents=True, exist_ok=True)
    md5_list = []
    md5_list_diff = []
    version_info = {"pre-version": "", "post-version": ""}

    for filename in os.listdir(dir1):
        file1 = os.path.join(dir1, filename)
        file2 = os.path.join(dir2, filename)
        patch_file = os.path.join(dir3, f"{filename}")

        if filename == "version":
            with open(file1, 'r') as f:
                pre_version = f.read().strip()
            with open(file2, 'r') as f:
                post_version = f.read().strip()
            save_version_info(dir3, pre_version, post_version)
            continue

        if filename.endswith('.xml'):
            shutil.copy2(file1, os.path.join(dir3, filename))
            continue

        md5_hash= calculate_md5(file1)
        if md5_hash:
            md5_list.append(f"{md5_hash} {filename}")

        if os.path.exists(file2):
            print(f"Processing {filename}...")
            if not run_bsdiff(file1, file2, patch_file):
                print(f"Failed to generate patch for {filename}")
                continue

        md5_hash_diff= calculate_md5(patch_file)
        if md5_hash_diff:
            md5_list_diff.append(f"{md5_hash_diff} {filename}")

    with open(os.path.join(dir3, "oldpackage"), 'w') as f:
        #json.dump(md5_dict, f, indent=2)
        for line in md5_list:
            f.write(f"{line}\n")

    with open(os.path.join(dir3, "image.md5"), 'w') as f:
        #json.dump(md5_dict, f, indent=2)
        for line in md5_list_diff:
            f.write(f"{line}\n")

    out_file = os.path.join(dir3, "updatesize")
    getImgesize(dir1, out_file)

    output_iso = "linux_diff.iso"
    # create iso file
    mkisofs_cmd = [
        "mkisofs",
        "-o", output_iso,
        "-R", "-J",
        dir3
    ]
    try:
        subprocess.run(mkisofs_cmd, check=True)
        print(f"ISO file '{output_iso}' created successfully.")
    except subprocess.CalledProcessError as e:
        print(f"Error creating ISO file: {e}")
        sys.exit(1)
    print("Processing complete!")
    print(f"Results saved to: {dir3}")

if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description='Generate binary diffs between two directories using local bsdiff')
    parser.add_argument('dir1', help='Original directory (old version)')
    parser.add_argument('dir2', help='New directory (new version)')
    parser.add_argument('dir3', help='Output directory for patches')

    args = parser.parse_args()
    if not os.path.isdir(args.dir1):
        print(f"Error: bsdiff executable not found at {args.dir1}")
        exit(1)
    if not os.path.isdir(args.dir2):
        print(f"Error: bsdiff executable not found at {args.dir2}")
        exit(1)
    process_files(args.dir1, args.dir2, args.dir3)