#!/usr/bin/env python3
"""Build a small read-only FAT12 image and emit it as a C header."""

from __future__ import annotations

import argparse
import struct
from pathlib import Path

SECTOR_SIZE = 512
SECTOR_COUNT = 128
FAT_COUNT = 2
SECTORS_PER_FAT = 1
ROOT_ENTRIES = 32
ROOT_SECTORS = 2
DATA_START_SECTOR = 1 + FAT_COUNT * SECTORS_PER_FAT + ROOT_SECTORS


def set_fat12_entry(fat: bytearray, cluster: int, value: int) -> None:
    offset = cluster + cluster // 2
    if cluster & 1:
        fat[offset] = (fat[offset] & 0x0F) | ((value << 4) & 0xF0)
        fat[offset + 1] = (value >> 4) & 0xFF
    else:
        fat[offset] = value & 0xFF
        fat[offset + 1] = (fat[offset + 1] & 0xF0) | ((value >> 8) & 0x0F)


def directory_entry(name: bytes, first_cluster: int, size: int) -> bytes:
    if len(name) != 11:
        raise ValueError("FAT 8.3 names must contain exactly 11 bytes")
    entry = bytearray(32)
    entry[:11] = name
    entry[11] = 0x01  # read-only
    struct.pack_into("<H", entry, 26, first_cluster)
    struct.pack_into("<I", entry, 28, size)
    return bytes(entry)


def build_image(files: list[tuple[bytes, bytes]]) -> bytes:
    image = bytearray(SECTOR_SIZE * SECTOR_COUNT)
    boot = memoryview(image)[:SECTOR_SIZE]
    boot[0:3] = b"\xEB\x3C\x90"
    boot[3:11] = b"MSDOS5.0"
    struct.pack_into("<H", boot, 11, SECTOR_SIZE)
    boot[13] = 1
    struct.pack_into("<H", boot, 14, 1)
    boot[16] = FAT_COUNT
    struct.pack_into("<H", boot, 17, ROOT_ENTRIES)
    struct.pack_into("<H", boot, 19, SECTOR_COUNT)
    boot[21] = 0xF8
    struct.pack_into("<H", boot, 22, SECTORS_PER_FAT)
    struct.pack_into("<H", boot, 24, 1)
    struct.pack_into("<H", boot, 26, 1)
    struct.pack_into("<I", boot, 28, 0)
    struct.pack_into("<I", boot, 32, 0)
    boot[36] = 0x80
    boot[38] = 0x29
    struct.pack_into("<I", boot, 39, 0x5049434F)
    boot[43:54] = b"PICO-PAIR  "
    boot[54:62] = b"FAT12   "
    boot[510:512] = b"\x55\xAA"

    fat = bytearray(SECTOR_SIZE)
    fat[:3] = b"\xF8\xFF\xFF"
    root = bytearray(ROOT_SECTORS * SECTOR_SIZE)
    root[:11] = b"PICO-PAIR  "
    root[11] = 0x08

    cluster = 2
    root_offset = 32
    data_region = DATA_START_SECTOR * SECTOR_SIZE
    for name, content in files:
        clusters_needed = max(1, (len(content) + SECTOR_SIZE - 1) // SECTOR_SIZE)
        last_cluster = cluster + clusters_needed - 1
        if data_region + (last_cluster - 1) * SECTOR_SIZE > len(image):
            raise ValueError("Files do not fit in the FAT12 image")

        root[root_offset:root_offset + 32] = directory_entry(name, cluster, len(content))
        root_offset += 32
        for current in range(cluster, last_cluster + 1):
            set_fat12_entry(fat, current, 0xFFF if current == last_cluster else current + 1)

        start = data_region + (cluster - 2) * SECTOR_SIZE
        image[start:start + len(content)] = content
        cluster = last_cluster + 1

    fat1 = SECTOR_SIZE
    fat2 = fat1 + SECTOR_SIZE
    root_start = fat2 + SECTOR_SIZE
    image[fat1:fat1 + SECTOR_SIZE] = fat
    image[fat2:fat2 + SECTOR_SIZE] = fat
    image[root_start:root_start + len(root)] = root
    return bytes(image)


def emit_header(image: bytes, output: Path) -> None:
    rows = []
    for offset in range(0, len(image), 16):
        values = ", ".join(f"0x{value:02x}" for value in image[offset:offset + 16])
        rows.append(f"    {values},")
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(
        "#ifndef MSC_DISK_IMAGE_H\n"
        "#define MSC_DISK_IMAGE_H\n\n"
        "#include <stdint.h>\n\n"
        f"#define MSC_DISK_BLOCK_SIZE {SECTOR_SIZE}u\n"
        f"#define MSC_DISK_BLOCK_COUNT {SECTOR_COUNT}u\n\n"
        "static const uint8_t msc_disk_image"
        "[MSC_DISK_BLOCK_SIZE * MSC_DISK_BLOCK_COUNT] = {\n"
        + "\n".join(rows)
        + "\n};\n\n#endif\n",
        encoding="ascii",
    )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--html", type=Path, required=True)
    parser.add_argument("--readme", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    html = args.html.read_bytes()
    readme = args.readme.read_bytes()
    image = build_image([(b"INDEX   HTM", html), (b"README  TXT", readme)])
    emit_header(image, args.output)


if __name__ == "__main__":
    main()
