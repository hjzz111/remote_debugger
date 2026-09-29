# SPDX-License-Identifier: MIT
# Copyright (c) 2026 Remote Debugger contributors

import argparse
import struct
from pathlib import Path

METADATA_PAGE_SIZE = 1024
APP1_MAX_SIZE = 0x7400

IOS_READY = 0x00000001


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("app_bin", help="链接地址为0x08001400的APP bin文件")
    parser.add_argument("output", help="输出的元数据bin文件")
    parser.add_argument(
        "--version",
        type=lambda value: int(value, 0),
        default=1,
        help="APP1版本号，默认为1",
    )
    args = parser.parse_args()

    app_path = Path(args.app_bin)
    output_path = Path(args.output)

    app_size = app_path.stat().st_size

    if app_size == 0:
        raise ValueError("APP bin文件为空")

    if app_size > APP1_MAX_SIZE:
        raise ValueError(
            f"APP文件过大：{app_size}字节，最大允许{APP1_MAX_SIZE}字节"
        )

    if not 0 <= args.version <= 0xFFFFFFFF:
        raise ValueError("版本号必须在uint32_t范围内")

    # 整个元数据页面默认为擦除状态
    metadata = bytearray([0xFF] * METADATA_PAGE_SIZE)

    # STM32为小端，写入APP1版本、状态和固件大小
    struct.pack_into(
        "<III",
        metadata,
        0,
        args.version,
        IOS_READY,
        app_size,
    )

    output_path.write_bytes(metadata)

    print(f"APP文件：{app_path}")
    print(f"APP版本：{args.version}")
    print(f"APP大小：{app_size} 字节，0x{app_size:08X}")
    print(f"元数据：{output_path}，{len(metadata)} 字节")


if __name__ == "__main__":
    main()
    
