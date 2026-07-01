#!/usr/bin/env python3

import sys
import os

assert len(os.sys.argv) > 3, "Insufficient arguments"
input_path = os.sys.argv[1]
output_path = os.sys.argv[2]
cxx_prefix = os.sys.argv[3]
assert os.path.exists(input_path), f"Input file {input_path} does not exist"
assert os.path.isdir(os.path.dirname(output_path)), f"Output directory {os.path.dirname(output_path)} does not exist"

with open(input_path, 'rb') as input_file:
    data = input_file.read()
    size = len(data)

cxx_lines = []
cxx_lines.append("#include <cstdint>")
cxx_lines.append("#include <cstddef>")
cxx_lines.append("")

cxx_lines.append(f"extern const std::size_t {cxx_prefix}_size = {size};")
cxx_lines.append(f"extern const std::uint8_t {cxx_prefix}[] = {{")

hex_lines = []
row_bytes = []

for i, byte in enumerate(data):
    row_bytes.append(f"0x{byte:02x}")

    if i < size - 1:
        row_bytes[-1] += ","
    
    if len(row_bytes) >= 12:
        hex_lines.append("    " + " ".join(row_bytes))
        row_bytes = []

if row_bytes:
    hex_lines.append("    " + " ".join(row_bytes))

cxx_lines.extend(hex_lines)
cxx_lines.append("};")


with open(output_path, 'w', encoding='utf-8') as output_file:
    output_file.write("\n".join(cxx_lines) + "\n")

sys.exit(0)
