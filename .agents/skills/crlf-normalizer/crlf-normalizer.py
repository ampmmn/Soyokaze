#!/usr/bin/env python3

import argparse
import glob
import os
import sys
from typing import List


def normalize_to_crlf(path: str) -> bool:
    """
    指定ファイルの改行コードを CRLF に統一する。
    変換成功なら True、失敗なら False を返す。
    """
    try:
        # 元ファイルを読み込む（UTF-8 前提）
        with open(path, "r", encoding="utf-8") as f:
            original_text = f.read()

        # 改行コードを正規化：まず LF に統一
        normalized = original_text.replace("\r\n", "\n").replace("\r", "\n")

        # 最終的に CRLF に統一
        converted = normalized.replace("\n", "\r\n")

        # 一時ファイルに書き出し
        tmp_path = path + ".tmp_crlf"
        with open(tmp_path, "w", encoding="utf-8", newline="") as f:
            f.write(converted)

        # 上書き（atomic replace）
        os.replace(tmp_path, path)
        return True

    except Exception as e:
        # 失敗したら元ファイルはそのまま
        print(f"Error converting {path}: {e}", file=sys.stderr)
        return False


def expand_globs(patterns: List[str]) -> List[str]:
    """glob パターンを展開してファイル一覧を返す"""
    files = []
    for pat in patterns:
        files.extend(glob.glob(pat, recursive=True))
    return files


def main():
    parser = argparse.ArgumentParser(description="Convert newline to CRLF")
    parser.add_argument(
        "--input",
        action="append",
        required=True,
        help="Glob pattern for input files (can be specified multiple times)",
    )
    args = parser.parse_args()

    files = expand_globs(args.input)
    if not files:
        print("No files matched.", file=sys.stderr)
        sys.exit(1)

    for path in files:
        if os.path.isfile(path):
            print(f"Converting: {path}")
            normalize_to_crlf(path)
        else:
            print(f"Skipping non-file: {path}")


if __name__ == "__main__":
    main()

